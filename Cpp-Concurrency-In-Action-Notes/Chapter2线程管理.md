# Chapter2 线程管理

## 启动线程

`std::thread` 可以用可调用对象（functor）构造：

```cpp
class background_task
{
public:
    void operator()() const {
        do_something();
        do_something_else();
    }
};

background_task f;
std::thread my_thread(f);
```

需要注意，当把函数对象传入到构造函数中时，需要避免C++语法解析优先解析为函数声明而非类型对象定义：

```cpp
std::thread my_thread(background_task());
```

编译器会将它理解为一个 **一个返回值为 `std::thread`，参数为函数指针，*指向空参并返回 `background_task` 对象的函数* 的函数声明）**，而非启动了一个线程。

可以使用统一初始化语法或使用多组括号表明参数是一个表达式：

```cpp
std::thread my_thread((background_task()));
std::thread my_thread { background_task() };
```

## 结束线程

启动线程后，需要明确是等待线程结束还是让线程分离自主运行。如果 `std::thread` 对象在销毁前还没有做出决定，析构函数会调用 `std::terminate()` 终止程序。

如果选择后者，需要保证对象的声明周期应大于等于线程执行时间。否则会造成对非法内存的访问：

```cpp
struct func 
{
    int& i;
    func(int& i_) : i(_i) {}
    void operator()() 
    {
        for (unsigned j = 0; j < 100000; ++j)
        {
            do_something(i);
        }
    }
};

void oops()
{
    int some_local_state = 0;
    func my_func(some_local_state);
    std::thread m_thread(my_func);
    my_thread.detach();
}
```

如果需要等待线程，需要使用 `join` 。被join的线程不能再次被join，使用 `joinable()` 可以返回线程对象是否可以被join。没有执行线程的 `std::thread` 不能使用 `join()`。

一个异常安全的线程类，可以使用RAII技法：

```cpp
class thread_guard
{
    std::thread& t;
public:
    explicit thread_guard(std::thread& t_):
        t(_t)
    {}
    ~thread_guard()
    {
        if (j.joinable())
        {
            t.join();
        }
    }
    thread_guard(thread_guard const&) = delete;
    thread_guard& operator=(thread_guard const&) = delete;
};

struct func;
void f()
{
    int some_local_state = 0;
    func my_func(some_local_state);
    std::thread t(my_func);
    thread_guard g(t);
    do_something_in_current_thread();
}
```

如果不想等待线程结束，可以detach线程，从而避免异常安全问题。

## 守护线程

使用 `detach()` 会让线程在后台运行。线程的生命周期可能会从某一个应用起始到结束，可能在后台监视文件系统，还有可能对缓存进行清理。发后即忘（fire and forget）的任务就用到线程的这种方式。

> 在UNIX中，守护线程是指没有任何显式的用户接口，并在后台运行的线程。

对线程使用 `detach()` 的条件和使用 `join()` 的条件相同。如果 `joinable()` 返回 `true`，就可以使用 `detach()` 。

## 向线程函数传递参数

默认参数要拷贝到线程独立内存中，即使参数是引用的形式，也可以在线程中进行访问：

```cpp
void f(int i, std::string const& s);
std::thread t(f, 3, "hello");
```

需要特别注意，当指向动态变量的指针作为参数传递给线程的情况，代码如下：

```cpp
void f(int i, std::string const& s);
void oops(int some_param)
{
    char buffer[1024];
    sprintf(buffer, "%i", some_param);
    std::thread t(f, 3, buffer);
    t.detach();
}
```

函数可能在字符串转换为 `std::thread` 对象前崩溃，从而导致未定义行为。解决方案是在传递到构造函数前就转换为 `std::thread` 对象：

```cpp
void f(int i, std::string const& s);
void not_oops(int some_param)
{
    char buffer[1024];
    sprintf(buffer, "%1", some_param);
    std::thread t(f, 3, std::string(buffer));
    t.detach();
}
```

如果要以引用的形式传递参数，需要包装一层 `std::ref()` ：

```cpp
void update_data_for_widget(widget_id w, widget_data& data);
std::thread t(update_data_for_widget, w, std::ref(data));
```

`const` 引用则需包装 `std::cref()`。

对于成员函数，则需要多传递一个对象指针：

```cpp
class X 
{
public:
    void do_lengthy_work(int);
};
X my_x;
int num(0);
std::thread t(&X::do_lengthy_work, &my_xm, num);
```

对于只移对象，可以使用 `std::move()` 显式移动至线程内：

```cpp
void process_big_object(std::unique_ptr<big_object>);

std::unique_ptr<big_object> p(new big_object);
p->prepare_data(42)
std::thread t(process_big_object, std::move(p));
```

## 转移线程多有权

`std::thread` 是一个资源占有类型，与其他C++标准库中的资源类型相同，都是可移动，但不可拷贝：

```cpp
void some_function();
void some_other_function();
std::thread t1(some_function);
std::thread t2 = std::move(t1);         // t1 -> t2
t1 = std::thread(some_other_function);  // 临时对象 -> t1
std::thread t3;                         
t3 = std::move(t2);                     // t2 -> t3
t1 = std::move(t3);                     // 赋值操作将调用 std::terminate()
```

`std::thread` 实例可作为参数进行传递：

```cpp
void f(std::thread(some_function));
void g()
{
    void some_function();
    f(std::thread(some_function));
    std::thread t(some_function);
    f(std::move(t));
}
```

当 `thread_guard` 对象持有线程的引用时，移动操作会带来一些麻烦。当某个对象转移了线程所有权后，它就不能对线程进行join或detach。为了确保线程能够正确结束，下面的代码定义了`scoped_thread` 类：

```cpp
class scoped_thread
{
    std::thread t;
public:
    explicit scoped_thread(std::thread t_):
        t(std::move(t_))
    {
        if (!t.joinable())
            throw std::logic_error("No thread");
    }
    ~scoped_thread()
    {
        t.join();
    }
    scoped_thread(scoped_thread const&) = delete;
    scoped_thread& operator=(scoped_thread const&) = delete;
};

struct func;

void f() 
{
    int some_local_state;
    scoped_thread t(std::thread(func(some_local_state)));
    do_something_in_current_thread();
}
```

C++17标准的提案包括添加一个 `joining_thread` 类型，这个类型使用RAII技法管理 `std::thread`。不过标准委员会对此没有达成统一共识，不过在C++20中新增了 `std::jthread` ，对这种方式进行了探索。下面对这个类进行实现：

```cpp
class joining_thread 
{
    std::thread t;
public:
    joining_thread() noexcept = default;

    template<typename Callable,typename ... Args>
    explicit joining_thread(Callable&& func,Args&& ... args):
        t(std::forward<Callable>(func),std::forward<Args>(args)...) {}

    explicit joining_thread(std::thread t_) noexcept: t(std::move(t_)) {}

    joining_thread(joining_thread&& other) noexcept: t(std::move(other.t)) {}

    joining_thread& operator=(joining_thread&& other) noexcept {
        if (joinable())
            join();      
        t = std::move(other.t);
        return *this;
    }

    joining_thread& operator=(std::thread other) noexcept {
        if(joinable()) join();
        t=std::move(other);
        return *this;
    }

    ~joining_thread() noexcept { if(joinable()) join(); }

    void swap(joining_thread& other) noexcept { t.swap(other.t); }

    std::thread::id get_id() const noexcept { return t.get_id(); }

    bool joinable() const noexcept { return t.joinable(); }

    void join() { t.join(); }

    void detach() { t.detach(); }

    std::thread& as_thread() noexcept { return t; }

    const std::thread& as_thread() const noexcept { return t; }
};
```

关于 `std::thread` 对象的容器，如果这个容器是支持移动的，那么移动操作同样适用于这些容器：

```cpp
void do_work(unsigned id);

void f()
{
    std::vector<std::thread> threads;
    for (unsigned i = 0; i < 20; ++i)
    {
        threads.push_back(std::thread(do_work, i));
    }
    std::for_each(threads.begin(), threads.end(),
                    std::mem_fn(&std::thread::join));
}
```

## 运行时决定线程数量

`std::thread::hardware_concurrency()` 返回当前操作系统支持的能并发在一个程序中的线程数量。

## 标识线程

线程标识类型为 `std::thread_id`，可通过 `std::thread` 的成员函数 `get_id()` 来直接获取。如果 `std::thread` 对象没有与任何执行线程相关联，`get_id()` 将返回 `std::thread::type` 默认构造值，这个值表示 **无线程** 。也可以在当前线程调用 `std::this_thread::get_id()` 获得线程标识。

`std::thread::id` 提供对比操作，允许为不同的值进行排序，这就允许将其作为有序容器的key值。标准库也提供 `std::hash<std::thread::id>` 容器，所以它也可以作为无序容器的key值。

`std::thread::id` 可作为一个线程的唯一标识符，当标识符只与语义相关（如数组索引）时，就需要这个方案了。也可以使用输出流来记录一个id值：

```cpp
std::cout << std::this::thread::get_id();
```

> C++标准只规定保证ID比较结果相等的线程必须有相同的输出，具体输出依赖于编译器实现。