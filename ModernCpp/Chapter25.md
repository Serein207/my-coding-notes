# Chapter25. Modern C++ 协程

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

## 24.4 semaphore

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