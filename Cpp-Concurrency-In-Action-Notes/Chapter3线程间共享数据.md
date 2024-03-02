# Chapter3 线程间共享数据

## 共享数据带来的问题

如果共享数据时只读的，那么操作不会影响到数据，更不会涉及对数据的修改，所以所有线程都会获得同样的数据。但是，当一个或多个线程要修改共享数据时，就会产生很多麻烦。

线程间的问题在于修改共享数据，致使不变量遭到破坏。当在修改过程中不确定是否有其他线程能够进行访问的话，可能就有线程访问到正在修改的节点；这样的话，线程就读取到要未修改的数据，这样不变量就被破坏了。

## 条件竞争

并发中竞争条件的形成，取决于一个以上线程的相对执行顺序，每个线程都抢着完成自己的任务。大多数情况下，即使改变执行顺序，也是良性竞争，其结果可以接受。例如，有两个线程同时向一个处理队列中添加任务，因为系统提供的不变量保持不变，所以谁先谁后都不会有什么影响。当不变量遭到破坏时，才会产生条件竞争。

恶性条件竞争通常发生于完成对多于一个的数据块的修改时。因为操作要访问两个独立的数据块，独立的指令将会对数据块将进行修改，并且其中一个线程可能正在进行时，另一个线程就对数据块进行了访问。因为出现的概率太低，条件竞争很难查找，也很难复现。

> [!NOTE] 数据竞争
>  C++ 标准中也定义了数据竞争这个术语，一种特殊的条件竞争：并发的去修改一个独立对象，数据竞争是(可怕的)未定义行为的起因

## 使用互斥量保护共享数据

C++通过实例化 `std::mutex` 创建互斥量实例，通过成员函数 `lock()` 对互斥量上锁，`unlock()` 进行解锁。实践中推荐采用RAII技法的模板类 `std::lock_guard`，在构造时对互斥量上锁，析构时解锁。

```cpp
std::list<int> some_list;
std::mutex some_mutex;

void add_to_list(int new_value)
{
    std::lock_guard<std::mutex> guard(some_mutex);
    some_list.push_back(new_value);
}

bool list_contains(int value_to_find)
{
    std::lock_guard<std::mutex> guard(some_mutex)
    return 
        std::find(some_list.begin(), some_list.end(), value_to_find) !=
            some_list.end();
}
```

C++17引入类模板实参推导，构造时可以简化为：

```cpp
std::lock_guard guard(some_mutex);
```

C++17还添加了另一种加强版数据保护机制——`std::lock_guard`：

```cpp
std::scoped_lock guard(some_mutex);
```

使用互斥量来保护数据，并不是仅仅在每一个成员函数中都加入一个 `std::lock_guard` 对象那么简单。考虑下面的例子：

```cpp
class some_data
{
    int a;
    std::string b;
public:
    void do_something();
};

class data_wrapper
{
    some_data data;
    std::mutex m;
public:
    template<typename Function>
    void process_data(Function func)
    {
        std::lock_guard<std::mutex> l(m);
        func(data); // 传递“保护”数据给用户函数
    }
};

some_data* unprotected;

void malicious_function(some_data& protected_data)
{
    unprotected = &protected_data;
}

data_wrapper x;
void foo()
{
    x.process_data(malicious_function); // 传递一个恶意函数
    unprotected->do_something();        // 在无保护的情况下访问保护数据
}
```

> 切勿将受保护数据的指针或引用传递到互斥锁作用域之外，无论是函数返回值，还是存储在外部可见内存，亦或是以参数的形式传递到用户提供的函数中去。

## 死锁

线程有对锁的竞争：一对线程需要对他们所有的互斥量做一些操作，其中每个线程都有一个互斥量，且等待另一个解锁。这样没有线程能工作，因为他们都在等待对方释放互斥量。这种情况就是死锁，它的最大问题就是由两个或两个以上的互斥量来锁定一个操作。

避免死锁的一般建议，就是让两个互斥量总以相同的顺序上锁：总在互斥量B之前锁住互斥量A，就永远不会死锁。某些情况下是可以这样用，因为不同的互斥量用于不同的地方。不过，事情没那么简单，比如：当有多个互斥量保护同一个类的独立实例时，一个操作对同一个类
的两个不同实例进行数据的交换操作，为了保证数据交换操作的正确性，就要避免数据被并发修改，并确保每个实例上的互斥量都能锁住自己要保护的区域。不过，选择一个固定的顺序(例如，实例提供的第一互斥量作为第一个参数，提供的第二个互斥量为第二个参数)，可能
会适得其反：在参数交换了之后，两个线程试图在相同的两个实例间进行数据交换时，程序又死锁了！

很幸运，C++标准库有办法解决这个问题，`std::lock` ——可以一次性锁住多个(两个以上)的互斥量，并且没有副作用(死锁风险)。下面的程序清单中，就来看一下怎么在一个简单的交换操作中使用 `std::lock`。

```cpp
class some_big_object;
void swap(some_big_object& lhs,some_big_object& rhs);
class X
{
private:
    some_big_object some_detail;
    std::mutex m;
public:
    X(some_big_object const& sd) : some_detail(sd) {}
    friend void swap(X& lhs, X& rhs)
    {
        if(&lhs == &rhs)
            return;
        std::lock(lhs.m, rhs.m); 
        std::lock_guard<std::mutex> lock_a(lhs.m, std::adopt_lock);
        std::lock_guard<std::mutex> lock_b(rhs.m, std::adopt_lock);
        swap(lhs.some_detail, rhs.some_detail);
    }
};
```

C++17对这种情况提供了支持， `std::scoped_lock<>` 一种新的RAII模板类型，与 `std::lock_guard<>` 的功能等价，这个新类型能接受不定数量的互斥量类型作为模板参数，以及相应的互斥量(数量和类型)作为构造参数。互斥量支持构造即上锁，与 `std::lock` 的用法相同，其解锁阶段是在析构中进行。`swap()`操作可以重写如下：

```cpp
void swap(X& lhs, X& rhs)
{
    if(&lhs==&rhs)
        return;
    std::scoped_lock guard(lhs.m, rhs.m);
    swap(lhs.some_detail, rhs.some_detail);
}
```

## 避免死锁

- 避免嵌套锁
- 避免在持有锁时调用用户提供的代码
- 使用固定顺序获取锁
- 使用层次锁

## `std::unique_lock` ——灵活的锁

可将 `std::adopt_lock` 作为第二个参数传入构造函数，对互斥量进行管理；也可以将 `std::defer_lock` 作为第二个参数传递进
去，表明互斥量应保持解锁状态。这样，就可以被 `std::unique_lock` 对象(不是互斥量)的 `lock()` 函数所获取，或传递  `std::unique_lock` 对象到 `std::lock()` 中。相比于 `std::lock_guard` ，会有性能损失。

```cpp
class some_big_object;
void swap(some_big_object& lhs,some_big_object& rhs);
class X
{
private:
    some_big_object some_detail;
    std::mutex m;
public:
    X(some_big_object const& sd) : some_detail(sd) {}
    friend void swap(X& lhs, X& rhs)
    {
        if(&lhs==&rhs)
            return;
        std::unique_lock<std::mutex> lock_a(lhs.m, std::defer_lock);
        std::unique_lock<std::mutex> lock_b(rhs.m, std::defer_lock);
        // 1 std::defer_lock 留下未上锁的互斥量
        std::lock(lock_a,lock_b); // 2 互斥量在这里上锁
        swap(lhs.some_detail, rhs.some_detail);
    }
};
```

当 `std::lock_guard` 已经能够满足你的需求时，还是建议你继续使用它。当需要更加灵活的锁时，最好选择 `std::unique_lock` ，因为它更适合于你的任务。

## 不同域中互斥量所有权的传递

`std::unique_lock` 实例没有与自身相关的互斥量，一个互斥量的所有权可以通过移动操作，在不同的实例中进行传递。 `std::unique_lock` 是可移动，但不可复制的类型。

一种使用可能是允许一个函数去锁住一个互斥量，并且将所有权移到调用者上，所以调用者可以在这个锁保护的范围内执行额外的动作。

```cpp
std::unique_lock<std::mutex> get_lock()
{
    extern std::mutex some_mutex;      // 注意这里是extern，不是定义
    std::unique_lock<std::mutex> lk(some_mutex);
    prepare_data();
    return lk; 
}
void process_data()
{
    std::unique_lock<std::mutex> lk(get_lock());
    do_something();
}
```

## 锁的粒度

> [!NOTE] 锁的粒度
> 
> 锁的粒度是一个摆手术语(hand-waving term)，用来描述通过一个锁保护着的数据量大小。一个细粒度锁(a fine-grained lock)能够保护较小的数据量，一个粗粒度锁(a coarse-grained lock)能够保护较多的数据量。选择粒度对于锁来说很重要，为了保护对应的数据，保证锁有能力保护这些数据也很重要。

`std::unique_lock` 在这种情况下工作正常，调用 `unlock()` 时，代码不需要再访问共享数据；而后当再次需要对共享数据进行访问时，就可以再调用 `lock()` 了。下面代码就是这样的一种情况：

```cpp
void get_and_process_data()
{
    std::unique_lock<std::mutex> my_lock(the_mutex);
    some_class data_to_process=get_next_data_chunk();
    my_lock.unlock(); // 1 不要让锁住的互斥量越过process()函数的调用
    result_type result=process(data_to_process);
    my_lock.lock(); // 2 为了写入数据，对互斥量再次上锁
    write_result(data_to_process, result);
}
```

一般情况下，执行必要的操作时，尽可能将持有锁的时间缩减到最小。这也就意味有一些浪费时间的操作，比如：获取另外一个锁(即使你知道这不会造成死锁)，或等待输入/输出操作完成时没有必要持有一个锁(除非绝对需要)。

```cpp
class Y
{
private:
    int some_detail;
    mutable std::mutex m;
    int get_detail() const
    {
        std::lock_guard<std::mutex> lock_a(m);
        return some_detail;
    }
public:
    Y(int sd) : some_detail(sd) {}
    friend bool operator==(Y const& lhs, Y const& rhs)
    {
        if(&lhs==&rhs)
            return true;
        int const lhs_value = lhs.get_detail();
        int const rhs_value = rhs.get_detail();
        return lhs_value == rhs_value;
    }
};
```

## 保护共享数据的初始化过程

**延迟初始化(Lazy initialization)** 在单线程代码很常见——每一个操作都需要先对源进行检查，为了了解数据是否被初始化，然后在其使用前决定，数据是否需要初始化：

### 单线程

```cpp
std::shared_ptr<some_resource> resource_ptr;
void foo() {
    if(!resource_ptr)
        resource_ptr.reset(new some_resource);
    resource_ptr->do_something();
}
```

### 没必要的线程化

```cpp
std::shared_ptr<some_resource> resource_ptr;
std::mutex resource_mutex;
void foo() {
    std::unique_lock<std::mutex> lk(resource_mutex); // 所有线程在此序列化
    if(!resource_ptr)
        resource_ptr.reset(new some_resource); // 只有初始化过程需要保护
    lk.unlock();
    resource_ptr->do_something();
}
```

### 双重检查锁模式

> [!IMPORTANT] 双重检查锁是线程不安全的
> 
> 千万别用！千万别用！千万别用!

```cpp
void undefined_behavior_with_double_checked_locking()
{
    if(!resource_ptr) {
        std::lock_guard<std::mutex> lk(resource_mutex);
        if(!resource_ptr) 
            resource_ptr.reset(new some_resource); 
    }
    resource_ptr->do_something();
}
```

### 使用 `std::call_once` 作为类成员的延迟初始化(线程安全)

```cpp
class X
{
private:
    connection_info connection_details;
    connection_handle connection;
    std::once_flag connection_init_flag;

    void open_connection() {
        connection=connection_manager.open(connection_details);
    }

public:
    X(connection_info const& connection_details_):
        connection_details(connection_details_) {}

    void send_data(data_packet const& data) {
        std::call_once(connection_init_flag, &X::open_connection, this);
        connection.send_data(data);
    }

    data_packet receive_data() {
        std::call_once(connection_init_flag, &X::open_connection, this);
        return connection.receive_data();
    }
};
```

### magic `static`

在C++11标准中，初始化及定义完全在一个线程中发生，并且没有其他线程可在初始化完成前对其进行处理，条件竞争终止于初始化阶段，这样比在之后再去处理好的多。在只需要一个全局实例情况下，这里提供一个 `std::call_once` 的替代方案：

```cpp
class my_class;
my_class& get_my_class_instance() {
    static my_class instance; // 线程安全的初始化过程
    return instance;
}
```

多线程可以安全的调用 `get_my_class_instance()` 函数，不用为数据竞争而担心。

## 读写锁

读写锁允许两种不同的使用方式：一个“writer”线程独占访问和共享访问，让多个“reader”线程并发访问。

C++17标准库提供了两种非常好的互斥量—— `std::shared_mutex` 和 `std::shared_timed_mutex` 。C++14只提供了 `std::shared_timed_mutex` ，并且在C++11中并未提供任何互斥量类型。

对于写操作，可以使用 `std::lock_guard<std::shared_mutex>` 和 `std::unique_lock<std::shared_mutex>` 上锁。
作为 `std::mutex` 的替代方案，与 `std::mutex` 所做的一样，这就能保证写线程的独占访问。因为其他线程不需要去修改数据结构，所以其可以使用 `std::shared_lock<std::shared_mutex>` （C++14）获取读权限。

```cpp
class dns_entry;

class dns_cache {
    std::map<std::string,dns_entry> entries;
    mutable std::shared_mutex entry_mutex;

public:
    dns_entry find_entry(std::string const& domain) const {
        std::shared_lock<std::shared_mutex> lk(entry_mutex);
        std::map<std::string, dns_entry>::const_iterator const it=
            entries.find(domain);
        return (it == entries.end()) ? dns_entry() : it->second;
    }

    void update_or_add_entry(std::string const& domain,
                             dns_entry const& dns_details) {
        std::lock_guard<std::shared_mutex> lk(entry_mutex);
        entries[domain] = dns_details;
    }
};
```

## 嵌套锁

当一个线程已经获取一个 `std::mutex` 时(已经上锁)，并对其再次上锁，这个操作就是错误的，并且继续尝试这样做的话，就会产生未定义行为。C++标准库提供了 `std::recursive_mutex` 类。除了可以对同一线程的单个实例上获取多个锁，其他功能与 `std::mutex` 相同。互斥量锁住其他线程前，必须释放拥有的所有锁，所以当调用 `lock()` 三次后，也必须调用 `unlock()` 三次。正确使用 `std::lock_guard<std::recursive_mutex>` 和 `std::unique_lock<std::recursive_mutex>` 可以帮你处理这些问题。

大多数情况下，当需要嵌套锁时，就要对代码设计进行改动。嵌套锁一般用在可并发访问的类上，所以使用互斥量保护其成员数据。每个公共成员函数都会对互斥量上锁，然后完成对应的操作后再解锁互斥量。不过，有时成员函数会调用另一个成员函数，这种情况下，第二个成员函数也会试图锁住互斥量，这就会导致未定义行为的发生。“变通的”解决方案会将互斥量转为嵌套锁，第二个成员函数就能成功的进行上锁，并且函数能继续执行。

但是，不推荐这样的使用方式，因为过于草率，并且不合理。特别是，当锁被持有时，对应类的不变量通常正在被修改。这意味着，当不变量正在改变的时候，第二个成员函数还需要继续执行。一个比较好的方式是，从中提取出一个函数作为类的私有成员，并且让其他成员函数都对其进行调用，这个私有成员函数不会对互斥量进行上锁(在调用前必须获得锁)。然后，你仔细考虑一下，在这种情况调用新函数时，数据的状态。