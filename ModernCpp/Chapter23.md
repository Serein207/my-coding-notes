# Chapter23. Modern C++ 互斥

> 如果编写的是多线程应用程序，那么必须分外留意操作顺序。如果线程读写共享数据，就可能发生问题。如果不能避免数据共享，那么必须采用同步机制，使一次只有一个线程能更改数据。
>
> 布尔值和整数等标量经常使用上述原子操作来实现同步，当数据更复杂且必须在多个线程中使用这些数据时，就必须提供显式的同步机制。
>
> 标准库支持互斥的形式包括互斥体(mutex)类和锁类。这些类都可以用来实现线程之间的同步

- [Chapter23. Modern C++ 互斥](#chapter23-modern-c-互斥)
  - [23.1 互斥体类](#231-互斥体类)
    - [23.1.1 自旋锁](#2311-自旋锁)
    - [23.1.2 非定时的互斥体类](#2312-非定时的互斥体类)

## 23.1 互斥体类

互斥体(mutex, mutual exclusion)的基本使用机制如下：

- 希望与其他线程共享内存读写的一个线程试图锁定互斥体对象。如果另一个线程正在持有这个锁，希望获得访问的线程将被阻塞，直到锁被释放，或直到超时。
- 一旦线程获得锁，这个线程就可以随意使用共享的内存，因为这要假定希望使用共享数据的所有线程都正确获得了互斥体对象上的锁。
- 线程读写完共享的内存后，线程将锁释放，使其他线程有机会获得访问共享内存的锁。如果两个或多个线程正在等待锁，没有机制能保证哪个线程优先获得锁，并且继续访问数据。

C++标准提供了非定时的互斥体类和定时的互斥体类。有递归和非递归的两种风格。在讨论这些之前，先看看自旋锁的概念。

### 23.1.1 自旋锁

自旋锁是互斥锁的一种形式，其中线程使用忙碌循环（自旋）方式来尝试获取锁，执行工作，并释放锁。在旋转时，线程保持活跃，但不做任何有用的工作。即便如此，自旋锁在某些情况下还是有很多用，因为它们完全可以在自己的代码中实现，不需要对操作系统进行任何昂贵的调用，也不会造成线程切换的任何开销。如下面的代码所示，自旋锁可以使用单个原子类型实现：atomic_flag：

```cpp
std::atomic_flag spinlock = ATOMIC_FLAG_INIT; 	// Uniform initialization is not allowed
static const size_t NumberOfThreads { 50 };
static const size_t LoopsPerThread { 100 };

void doWork(size_t threadNumber, std::vector<size_t>& data) {
  for (size_t i{ 0 }; i < LoopsPerThread; ++i) {
    while (spinlock.test_and_set()) {}// spins until lock is acquired
    // save to handle shared data...
    data.push_back(threadNumber);
    spinlock.clear(); // release the acquired lock
  }
}

int main() {
  std::vector<size_t> data;
  std::vector<std::thread> threads;
  for (size_t i{ 0 }; i < NumberOfThreads; ++i) {
    threads.push_back(std::thread { doWork, i, ref(data) });
  }
  for (auto& t : threads) {
    t.join();
  }
  std::cout << std::format("data contains {} elements, excepted {}.\n", data.size(),
    NumberOfThreads * LoopsPerThread);
}
```

在这段代码中，每个线程都试图通过反复调用atomic_flag上的test_and_set()方法来获取一个锁，直到成功。这是忙碌循环。

> **警告**
>
> 由于自旋锁使用忙碌等待循环，因此只有在确定线程只会在短时间内锁定自旋锁时，才应该考虑这种方式。

### 23.1.2 非定时的互斥体类

标准库有3个非定时的互斥体类：`std::mutex`、`std::recursive_mutex`和`std::shared_mutex`。前两个类在 `<mutex>` 中定义，最后一个类在 `<shared_mutex>` 中定义。每个类都支持以下方法：

- `lock()`：调用线程将尝试获取锁，并阻塞直到获得锁。这个方法会无限期阻塞。如果希望设置线程阻塞最长时间，应该使用定时的互斥体类。
- `try_lock()`：调用线程将尝试获得锁。如果当前锁被其他线程持有，这个调用会立即返回。如果成功获得锁，try_lock()返回true，否则返回false。
- `unlock()`：释放由调用线程持有的锁，使另一个线程能获取这个锁。

`std::mutex` 使一个标准的具有独占所有权语义的互斥体类。只能有一个线程拥有互斥体。如果另一个线程想要获得互斥体的所有权，那么这个线程既可通过lock()阻塞，也可通过try_lock()尝试失败。已经拥有 `std::mutex` 所有权的线程不能再这个互斥体上再次调用lock()和try_lock()，否则可能导致死锁。

