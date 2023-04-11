# Chapter24. Modern C++ 条件变量，闩与屏障，信号量

- [Chapter24. Modern C++ 条件变量，闩与屏障，信号量](#chapter24-modern-c-条件变量闩与屏障信号量)
  - [24.1 条件变量](#241-条件变量)
    - [24.1.1 虚假唤醒](#2411-虚假唤醒)
    - [24.1.2 使用条件变量](#2412-使用条件变量)
  - [24.2 latch](#242-latch)
  - [24.3 barrier](#243-barrier)
  - [24.4 semaphore](#244-semaphore)

## 24.1 条件变量

条件变量允许一个线程阻塞，直到另一个线程设置某个条件或系统时间道道某个指定时间。条件变量允许显式的线程间通信。

有两类条件变量，它们都定义在 `<condition_variable>` 中。

- `std::condition_variable`：只能等待 `unique_lock<mutex>` 上的条件变量；根据C++标准的描述，这个条件变量可在特定平台上达到最高效率。
- `std::condition_variable_any`：可等待任何对象的条件变量，包括自定义的锁类型。


condition_variable类支持以下方法：

- `notify_one()`：唤醒等待这个条件变量的线程之一。
- `notify_all()`：唤醒等待这个条件变量的所有线程。
- `wait(unique_lock<mutex>& lk)`：
  
  调用wait()的线程应该已经获得lk上的锁。调用wait()的效果是以原子方式调用 `lk.unlock()` 并阻塞线程，等待通知。当线程被另一个线程中的notify_one()或notify_all()调用解除阻塞时，这个函数会再次调用 `lk.lock()`，可能被这个锁阻塞，然后返回。

- `wait_for(unique_lock<mutex>& lk, const chrono::duration<Rep,Period>& rel_time)`：
  
  类似于此前的wait()方法，区别在于这个线程会被notify_one()或notify_all()调用解除阻塞，也可能在给定超时时间到达后解除阻塞。

- `wait_until(unique_lock<mutex>& lk, const chrono::time_point<Clock,Duration>& abs_time)`：

  类似于此前的wait()方法，区别在于这个线程会被notify_one()或notify_all()调用解除阻塞，也可能在系统时间超过给定的绝对时间时解除阻塞。

也有一些其他版本的wait(), wait_for()和wait_until()接收一个额外的谓词参数。例如，接收一个额外谓词的wait()等同于：

```cpp
while (!predicate())
  wait(lk);
```

condition_variable_any类支持的方法和condition_variable类相同，区别在于condition_variable_any可接受任何类型的锁类，而不只是 `unique_lock<mutex>`。锁类应提供lock()和unlock()方法。

### 24.1.1 虚假唤醒

等待条件变量的线程可在另一个线程调用notify_one()或notify_all()时醒过来，或在系统时间超过给定时间时醒过来，也可能不合时宜地醒过来。这意味着，即使没有其他线程调用任何通知方法，线程也会醒过来。因此，当线程等待一个条件变量并醒过来时，就需要检查它是否因为获得通知而醒过来。一种检查方法是使用接收谓词参数版本的wait()方法。

### 24.1.2 使用条件变量

条件变量可用于处理队列项的后台线程。可定义队列，在队列中插入要处理的项。后台线程等待队列中出现项。把一项插入队列中时，线程就会醒过来，处理项，然后继续休眠，等待下一项。假设有以下队列：

```cpp
std::queue<std::string> m_queue;
```

需要确保在任何适合只有一个线程修改这个队列。可通过互斥体实现这一点：

```cpp
std::mutex m_mutex;
```

为了能在添加一项时通知后台线程，需要一个条件变量：

```cpp
std::condition_variable m_condVar;
```

需要向队列中添加项的线程首先要获得这个互斥体上的锁，然后向队列中添加项，最后通知后台线程。无论当前是否拥有锁，都可以调用notify_one()或notify_all()，它们都会正常工作：

```cpp
// lock mutex and add entry to the queue
std::unique_lock lock { m_mutex };
m_queue.push(entry);
// notify condition variable to wake up thread
m_condVar.notify_all();
```

后台线程会在一个无限循环中等待通知。注意这里使用接收谓词参数的wait()方法正确处理线程虚假唤醒。谓词检查队列中是否有队列项。对wait()的调用返回时，就可以肯定队列中有队列项了。

```cpp
std::unique_lock lock { m_mutex };
while (true) {
  // wait for a notification
  m_condVar.wait(lock, [this]{ return !m_queue.empty(); });
  // condition variable is notified, so something is in the queue
  // process queue item...
}
```

C++标准还定义了辅助函数 `std::notify_all_at_thread_exit(cond, lk)`，其中cond是一个条件变量，lk是一个 `unique_lock<mutex>` 实例。调用这个函数的线程应该已经获得了锁lk。当线程退出时，会自动执行以下代码：

```cpp
lk.unlock();
cond.notify_all();
```

> **注意**
>
> 将锁lk保持锁定，直到该线程退出为止。因此，一定要确保这不会在代码中造成任何死锁，例如由于错误的锁顺序而产生的死锁。

## 24.2 latch

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

## 24.3 barrier

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