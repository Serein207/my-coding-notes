# Chapter25. Modern C++ 协程

- [Chapter25. Modern C++ 协程](#chapter25-modern-c-协程)
  - [25.1 latch](#251-latch)
  - [25.2 barrier](#252-barrier)
  - [25.4 semaphore](#254-semaphore)
  - [25.5 future](#255-future)
    - [25.5.1 `std::promise` 和 `std::future`](#2551-stdpromise-和-stdfuture)
    - [25.5.2 `std::packaged_task`](#2552-stdpackaged_task)
    - [25.5.3 `std::async`](#2553-stdasync)
    - [25.5.4 异常处理](#2554-异常处理)
    - [25.5.5 `std::shared_future`](#2555-stdshared_future)
  - [25.6 示例：多线程的Logger类](#256-示例多线程的logger类)
  - [25.7 协程](#257-协程)

## 25.1 latch

latch是一次性使用的线程协调点。一旦给定数量的线程达到latch点时，所有线程都会解除阻塞并继续执行。基本上它是个计数器，在每个线程达到latch点时倒数。一旦计数器达到零，latch将无限期保持在一个有信号的状态，所有阻塞线程都将解除阻塞，随后到达latch点的任何线程会立刻被允许继续执行。

latch由 `std::latch` 实现，在 `<latch>` 中定义。构造函数接收需要到达latch点的所需线程数。到达latch点的线程可以调用arrive_and_wait()，它递减latch计数器并阻塞，直到latch有信号为止。线程也可以通过调用wait()在不减少计数器的情况下阻塞在latch点上。try_wait()方法可用于检查计数器是否达到零。最后，如果需要，还可以通过调用count_down()来减少计数器，而不会阻塞。

下面演示了一个latch点的用例，其中一些数据需要加载到内存(I/O bound)中，然后在多个线程中并行处理这些数据。进一步假设线程在启动时和开始处理数据之前需要执行一些CPU绑定的初始化，并行加载数据(I/O bound)，性能得到了提高。代码使用计数器1初始化一个latch对象，并启动10个线程，这些线程都进行一些初始化，然后阻塞latch，直到latch计数器达到零。在启动10个线程后，代码加载一些数据。一旦加载了所有数据，latch计数器将减为零，这10个线程都将解除阻塞。

```cpp
std::latch startLatch { 1 };
std::vector<std::jthread> threads;

for (int i { 0 }; i < 10; ++i) {
  threads.push_back(std::jthread { [&startLatch] {
    // do some initialization...(CPU bound)

    // wait until the latch counter reaches zero
    startLatch.wait();

    //process data...
  } });
}

// load data...(I/O bound)

// once all data has been loaded, decrement the latch counter
// which when reaches zero and unblocks all waiting threads.
startLatch.count_down();
```

## 25.2 barrier

barrier是由一些列阶段组成的可重用线程协调机制。许多线程在barrier点阻塞。当给定数量的线程到达barrier时，将执行完成阶段的回调，解除所有阻塞线程的阻塞，重置线程计数器，并开始下一个阶段。在每个阶段中，可以调整下一阶段的预期线程数。barrier对于在循环之间执行同步非常有用。例如，假设由很多线程并发执行，并在一个循环中执行一些i计算。进一步假设一旦这些计算完成，需要在线程中开始其循环的新迭代之前对结果进行一些处理。对于这种情况，设置barrier是完美的，所有的线程都会阻塞在barrier处。当它们全部到达时，完成阶段的回调将处理线程的结果，然后解除所有线程的阻塞，以开始它们的下一次迭代。

barrier由 `std::barrier` 实现，在 `<barrier>` 中定义。barrier最重要的方法是arrive_and_wait()，它减少计数器，然后阻塞线程，直到当前阶段完成.

下面的代码演示了barrier的使用。它启动4个线程，在循环中连续执行某些操作。在每次迭代中，所有线程都是用barrier进行同步：

```cpp
void completionFunction() noexcept {/*...*/}

int main() {
  const size_t numberOfThreads { 4 };
  std::barrier barrierPoint { numberOfThreads, completionFunction };
  std::vector<std::jthread> threads;

  for (int i { 0 }; i < numberOfThreads; ++i) {
    threads.push_back(std::jthread { [&barrierPoint] (std::stop_token token) {
      while (!token.stop_requested()) {
        // ... do some calculations ...
        
        // synchronize with other threads
        barrierPoint.arrive_and_wait();
      }
    } });
  }
}
```

## 25.4 semaphore

semaphore（信号量）是轻量级同步原语，可用作其他同步机制（如mutex, latch, barrier）的构建块。基本上一个semaphore由一个表示很多插槽的计数器组成。计数器在构造函数中初始化。如果获得了一个插槽，计数器将减少，而释放插槽将增加计数器。在 `<semaphore>` 中定义了两个semaphore类：`std::counting_semaphore` 和 `std::binary_semaphore`。前一种模型是非负资源计数。后者只有一个插槽，该槽要么是空的，要么不是空的，完美适合作为互斥的构建块。两者都提供下表所示的方法：

| 方法                | 描述                                                                                              |
| ------------------- | ------------------------------------------------------------------------------------------------- |
| acquire()           | 递减计数器。当计数器为零时阻塞，直到计数器再次递增                                                |
| try_acquire()       | 尝试递减计数器，但如果计数器已经为零时不会阻塞。如果计数器可以可以递减，则返回true，否则返回false |
| try_acquire_for()   | 与try_acquire相同，但会在给定时间段内尝试                                                         |
| try_acquire_until() | 与try_acquire相同，但会一直尝试直到系统到达给定时间                                               |
| release()           | 计数器增加一个给定的数，并解除在acquire调用中线程的阻塞                                           |

计数semaphore允许精确地控制希望允许并发允许地线程数量。例如，下面的代码允许最多4个线程并行运行：

```cpp
std::counting_semaphore semaphore { 4 };
std::vector<std::jthread> threads;
for (int i { 0 }; i < 10; ++i) {
  threads.push_back(std::jthread { [&semaphore] {
    semaphore.acquire();
    // ... slot acquired ... (at most 4 threads concurrently)
    semaphore.release();
  } });
}
```

semaphore的另一个用例是为线程而不是为条件变量实现通知机制。例如，可以在其构造函数中将semaphore的计数器初始化为0，任何调用acquire()的线程都将阻塞，直到其他线程对semaphore调用release()。

## 25.5 future

与std::thread相关的另一个问题是处理像异常这样的错误。如果一个线程抛出一个异常，而这个异常没有被线程本身处理，C++运行时将调用std::terminate()，这通常会终止整个应用程序。

使用future更方便地获得线程的结果，并将异常转移到另一个线程中，然后另一个线程可以任意处理这个异常。当然，应该总是尝试在线程本身处理异常，不要让异常离开线程。

future在promise中存储结果。可通过future获取promise中存储的结果。也就是说，promise是结果的输入端，future是输出端。一旦在同一线程或另一线程中运行的函数计算出希望返回的值，就把这个值放在promise中。然后可以通过future获取这个值。可将future/promise对想象为线程之间传递结果的信息通道。

C++提供标准的future名为 `std::future`。可从 `std::future` 检索结果。T是计算结果的类型。

```cpp
std::future<T> myFuture {...}; 
T result { myFuture.get() };
```

调用get()以取出结果，并保存在变量result中。如果另一个线程尚未计算完结果，对get()的调用将阻塞，直到结果值可用。只能在future上调用一次get()。按照标准，第二次调用的行为是不确定的。

可首先通过向future询问结果是否可用的方式来避免阻塞：

```cpp
if (myFuture.wait_for(0)) {
  T result { myFuture.get() };
} else {
  // value is not yet available
}
```

### 25.5.1 `std::promise` 和 `std::future`

C++提供了 `std::promise` 类，作为promise概念的一种方式。可在promise上调用set_value()来存储结果，也可调用set_exception()，在promise中存储异常。注意，只能在特定的promise上调用set_value()或set_exception()一次。如果多次调用它，抛出 `std::future_error` 异常。

如果线程A启动线程B以执行计算，则线程A可创建一个 `std::promise`，将其传给已启动的线程。注意，无法复制promise，但可将其移到线程中！线程B使用promise存储结果。将promise移入线程B之前，线程A在创建的promise上调用get_future()，这样，线程B完成后就能访问结果。

下面是一个简单示例：

```cpp
void doWork(std::promise<int> thePromise) {
  // ... do some work ...
  // and ultimately store the result in the promise
  thePromise.set_value(42);
}

int main() {
  // create a promise to pass to the thread
  std::promise<int> myPromise;

  // get the future of the promise
  auto theFuture { myPromise.get_future() };
  // create a thread and move promise into it
  std::thread theThread { doWork, std::move(myPromise) };

  // do some work ...

  // get the result
  int result { theFuture.get() };
  std::cout << "Result: " << result << std::endl;

  // make sure to join the thread
  theThread.join();
}
```

> **注意**
>
> 在实际应用程序中使用future模型时，可定期检查future中是否游客用的结果，或使用条件变量等同步机制。当结果还不可用时，可做其他事情，而不是阻塞。

### 25.5.2 `std::packaged_task`

有了 `std::packaged_task`，将可以更方便地使用promise，而不需要显式地使用 `std::promise`。下面的代码创建了一个packaged_task来执行calculateSum()。通过调用get_future()，从packaged_task检索future。启动一个线程，并将packaged_task移入其中。无法复制packaged_task！启动线程后，在检索到的future上调用get()来获得结果。在结果可用之前，将一直阻塞。

calculateSum()不需要任何类型的promise显式存储任何数据。packaged_task自动创建promise，自动在promise中存储被调用函数的结果，并自动在promise中存储函数抛出的任何异常。

```cpp
int calculateSum(int a, int b) { return a + b; }

int main() {
  // create a packaged_task to tun calculateSum
  std::packaged_task<int(int, int)> task { calculateSum };
  // get the future for the result if the packaged_task
  auto theFuture { task.get_future() };
  // create a thread, move the packaged task into it, and
  // execute the packaged_task with the given arguments
  std::thread theThread { std::move(task), 39, 3 };
  // do some more work...

  // get the result
  int result { theFuture.get() };
  std::cout << result << std::endl;

  // make sure to join the thread
  theThread.join();
}
```

### 25.5.3 `std::async`

如果向让C++运行时更多地控制是否创建一个线程以进行某种计算，可使用 `std::async()`。它接收一个想要执行地函数，并返回可用于检索的future。async()可通过两种方法运行函数：

- 创建一个新线程，异步运行提供的函数。
- 在返回的future上调用get()方法时，在主线程上同步地运行函数。

如果没有通过额外参数来调用async()，C++运行时会根据一些因素（如系统处理器地数目）从两种方法中自动选择一种方法。也可指定策略参数，从而调整C++运行时的行为。

- `launch::async`：强制C++运行时在一个不同线程上异步地执行函数。
- `launch::deferred`：强制C++运行时在调用get()方法时，在主线程上同步地执行函数。
- `launch::async | launch::deferred`：允许C++允许时进行选择(=默认行为)。

下面演示了async()的用法：

```cpp
int calculate() { return 42; }
int main() {
  auto myFuture { std::async(calculate) };
  // auto myFuture { std::async(std::launch::async, calculate) };
  // auto myFuture { std::async(std::launch::deferred, calculate) };

  // do some more work...

  // get the result
  int result { myFuture.get() };
  std::cout << result << std::endl;
} 
```

从这个例子可以看出 `std::async()` 是以异步方式（在不同线程中）或同步方式（在同一线程中）执行一些计算并在随后获取结果的最简单方式之一。

> **警告**
>
> 调用async()锁返回的future会在其析构函数中阻塞，直到结果可用为止。这意味着如果调用async()时未捕获返回的future，async()调用会成为阻塞调用！
> 
> ```cpp
> std::async(calculate);
> ```
>
> 这条语句中，未捕获async()返回的future，于是产生临时的future，在该语句完成前调用其析构函数，在结果可用前，该析构函数将一直阻塞。

### 25.5.4 异常处理

使用future的一大优点是它们会自动在线程之间传递异常。在future上调用get()时，要么返回计算结果，要么重新抛出与future关联的promise中存储的任何异常。使用packaged_task或async()时，从已启动的函数抛出的任何异常将自动存储在promise中。如果 `std::promise` 用作promise，可调用set_exception()存储异常。下面是一个使用async()的示例：

```cpp
int calculate() {
  throw std::runtime_error { "Exception thrown from calculate()." };
}

int main() {
  // use the launch::async policy to force asynchronous execution.
  auto myFuture { std::async(std::launch::async, calculate) };

  // do some more work...

  // get the result
  try {
    int result { myFuture.get() };
    std::cout << result << std::endl;
  } catch (const std::exception& ex) {
    std::cout << "Caught exception: " << ex.what() << std::endl;
  }
}
```

### 25.5.5 `std::shared_future`

`std::future<T>` 只要求T可移动。在future\<T>上调用get()时，结果将移出future，并返回。这意味着只能在 future\<T>上调用get()一次。

如果要多次调用get()，甚至从多个线程多次调用，则需要使用 `std::shared_future<T>`，此时，T需要可复制。可使用 `std::future::share()`，或给shared_future构造函数传递future，以创建shared_future。注意，future不可复制，因此需要将其移入shared_future构造函数。

shared_future可用于同时唤醒多个线程。下面的代码创建了两个 `std::promise` 对象 thread1Started 和 thread2Started 和一个 `std::promise<int>` 对象 signalPromise，分别表示两个线程已经启动和信号已经被设置。它还创建了一个 `std::shared_future<int>` 对象 signalFuture，表示一个共享的、单独的期望（expectation）。两个线程都分别与该对象关联，以等待特定的信号。 

在每个线程的 lambda 表达式中，它首先通知线程已经开始运行（通过调用 set_value）并等待设置信号，以便在 `signalFuture.get()` 处返回时恢复执行并进行下一步操作。
在主线程中，它等待两个线程都已经启动后才设置信号变量。这样，当 signalFuture 被设置为 42 时，两个线程将同时恢复执行。

由于两个线程在本例中使用独立的 `std::shared_future` 对象等待信号的到来，因此它们不必共享同一个 `std::promise` 对象。这允许两个线程分别等待各自的条件，而不会被其他线程的信号唤醒。此外，使用 `std::shared_future` 而不是 `std::future` 可以在需要在多个线程之间共享期望时非常有用。

```cpp
std::promise<void> thread1Started, thread2Started;

std::promise<int> signalPromise;
auto signalFuture { signalPromise.get_future().share() };
// std::shared_future<int> signalFuture { signalPromise.get_future() };

auto function1 { [&thread1Started, signalFuture] {
  thread1Started.set_value();
  // wait until parameter is set
  int parameter { signalFuture.get() };
  // ...
} };

auto function2 { [&thread2Started, signalFuture] { 
  thread2Started.set_value(); 
  // wait until parameter is set 
  int parameter { signalFuture.get() }; 
  //... 
} }

// run both lambda expressions asynchronously
// remember to capture the future returned by async()!
auto result1 { std::async(std::launch::async, function1) };
auto result2 { std::async(std::launch::async, function2) };

// wait until both threads have started
thread1Started.get_future().wait();
thread2Started.get_future().wait();

// both threads are now waiting for the parameter
// set the parameter to wake up both of them
signalPromise.set_value(42);
```

## 25.6 示例：多线程的Logger类

本节演示如何使用线程、互斥体对象、锁和条件变量编写一个多线程的Logger类，这个类允许不同宣传向队列中添加日志消息。Logger类本身会在另一个后台线程中处理这个队列，将日志信息串行写入一个文件。

详见 [Logger](/code/Loggeer)

## 25.7 协程

协程是一个可以在执行过程中挂起并在稍后的时间点恢复的函数。任何函数体包含以下任一项都是协程：

- co_wait：在等待一个计算完成时挂起一个协程的执行。当计算完成后，继续执行。
- co_return：从协程返回。在此之后，协程无法恢复。
- co_yield：从协程返回一个值给调用者，并挂起协程，随后再次调用协程，在它被挂起的地方继续执行。

通常由两种类型的协程：有栈协程和无栈协程。有栈协程可以从嵌套调用内部的任何地方挂起。无栈协程只能从顶层栈帧挂起。当无栈协程挂起时，只保存函数体中具有自动存储时间的变量和临时变量，不保存调用栈。因此，无栈协程的内存使用非常少，允许数百万甚至数十亿的协程同步运行。C++只支持无栈协程的变体。

协程可以使用同步编程风格来实现异步操作，用例包括以下内容：

- 生成器(Generators)
- 异步I/O(Asynchronous I/O)
- 延迟计算(Lazy computations)
- 事件驱动程序(Event-driven applications)

C++20标准只提供了协程构建块，也就是语言的补充。

C+20标准库没有提供任何标准化的高级协程，比如生成器。有一些第三方的库确实提供了这样的协程。msvc还提供了一些更高层次的结构，比如一个实验性生成器。下面代码演示了msvc `std::experimental::generator` 协程的使用：

```cpp
#include <experimental/generator>
#include <iostream>
#include <coroutine>
#include <chrono>

std::experimental::generator<int> getSequenceGenerator(
  int startValue, int numOfValues) {
  for (int i { startValue }; i < startValue + numOfValues; ++i) {
    // print the local time to standard out
    time_t tt { std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) };
    tm t;
    localtime_s(&t, &tt);
    std::cout << std::put_time(&t, "%H:%M:%S") << ": ";
    // yield a value to the caller, and suspend the coroutine
    co_yield i;
  }
}

int main() {
  auto gen { getSequenceGenerator(10, 5) };
  for (const auto& value : gen) {
    std::cout << value << " (Press enter for next value)";
    std::cin.ignore();
  }
}
```

运行程序会得到以下输出：

```
20:51:18: 10 (Press enter for next value)
```

每按下一次Enter，生成器都会请求一个新值。这回导致协程继续执行。

```
20:51:20: 11 (Press enter for next value)
20:51:21: 12 (Press enter for next value)
20:51:21: 13 (Press enter for next value)
20:51:22: 14 (Press enter for next value)
```

这是几乎关于协程的所有内容。也许将来的C++会引入标准化协程。