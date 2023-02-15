# Modern C++ 常量

C++11中，引入常量表达式关键字 `constexpr`，它可以修饰函数，变量，类等数据。

在了解 `constexpr` 的特点之前，我们先来了解下我们的老朋友 `const`

### 常值变量修饰符 `const`

正如标题那样，`const` 与 `#define` 的不同之处正是在于， `const` 修饰的仍然是在内存中的变量，在整个程序中只有一份数据，只是编译器让它不可以被修改，**在编译期仅分配内存，运行期赋值** ，所以被其修饰的变量也被称为“常值变量”。  
而 `#define` 宏定义则是将常量放在内存中的只读区域，在预编译期执行替换操作，编译期确定数值，并且在整个程序中存在多份数据（如果被调用多次的话）。

举个例子
```cpp
#define N 10
const int n = 10;

int arr[N];
int arr[n];   // error
```
尽管在最新的编译器中，定义数组时数组长度为变量 `n` 的行为被优化，成为可执行语句，但它仍然是一个非法行为。

即使数组长度是一个常值变量，仍然是非法行为。但是使用宏常量定义时是合法的，因为它只是完成替换，与内存中的变量无关。

当然，数组长度也可以是一些常量表达式
```cpp
int arr[10 + 5];
```
但是，我们仍然无法这样使用
```cpp
const int n = 10 + 5;
int arr[n];           // error
```

### 常量表达式修饰符 `constexpr`

constexpr（常量表达式）：是指值不会改变并且在编译过程就能得到计算结果的表达式。

常量表达式的优点是将计算过程转移到编译时期，那么运行期就不再需要计算了，程序性能也就提升了。

#### 修饰变量

那么我们将上述代码改为
```cpp
constexpr int n = 10;
int arr[n];
```
这样就合法了

引用变量可声明为 `constexpr`
```cpp
static constexpr const int& x = 42;// 到 const int 对象的 constexpr 引用
                                   // （该对象拥有静态存储期，因为静态引用延长了生存期）
```

#### 修饰函数

注意，这里修饰的是函数的返回值

```cpp
constexpr int Length_Constexpr() {
    return 5;
}
​
char arr_2[Length_Constexpr() + 1];
```

当然，为了保证函数能够产生一个常量表达式，函数必须满足以下条件：

1. 修饰的函数 **只能包括 `return` 语句**（允许出现 `using`,`typedef`,`static_assert`）
2. 修饰的函数 **只能引用全局不变常量**
3. 修饰的函数 **只能调用其他 `constexpr` 修饰的函数**（C++23开始允许）
4. 函数必须有返回值且 **不能为 `void` 类型**

`constexpr` 修饰的函数是可以实现递归的，同时它本身自带 `inline` 属性

如求斐波那契数列的第n项

```cpp
constexpr int fibonacci(const int n) {
    return n == 1 || n == 2 ? 1 : fibonacci(n - 1) + fibonacci(n - 2);
}
```

在C++11中，被 `constexpr` 修饰的函数 **有且只能有一个 `return` 语句** 。而在C++14中，这个要求被放宽，我们可以这样写

```cpp
constexpr int fibonacci(const int n) {
    if (n == 1) return 1;
    if (n == 2) return 1;
    return fibonacci_2(n - 1) + fibonacci_2(n - 2);
}
```
注意，C++11中 `constexpr` 不支持修饰被 `virtual` 修饰的成员函数

#### 修饰构造函数

对象会在编译器被初始化，同时构造函数需要满足如下条件：

1. 对于类或结构体的构造函数，**每个子对象和每个非变体非静态数据成员必须被初始化**。如果类是联合体式的类，那么对于它的每个非空匿名联合体成员，必须恰好有一个变体成员被初始化
2. 对于非空联合体的构造函数，恰好有一个非静态数据成员被初始化

通过定义 `constexpr` 构造函数，可以创建用户自定义的类型的常量表达式变量。下面是一个示例，以下Rect类定义了 `constexpr` 构造函数。它还定义了执行一些计算的 `constexpr getArea()` 方法。 

```cpp
class Rect {
public:
	constexpr Rect(size_t width, size_t height)
		: m_width{ width }, m_height{ height } {}

	constexpr size_t getArea()const {
		return m_width * m_height;
	}
private:
	size_t m_width{}, m_height{};
};
```

使用这个类声明 `constexpr` 对象是非常容易的。

```cpp
constexpr Rect r{ 8, 2 };
int myArray[r.getArea()];
```

#### 修饰析构函数

析构函数不能是 `constexpr` 的，但能在常量表达式中隐式调用平凡析构函数。(C++20 前)

函数体非 `=delete`; 的 `constexpr` 析构函数必须满足下列额外要求：

- 每个用于销毁非静态数据成员与基类的析构函数必须是 constexpr 析构函数。(C++20 起)

最后，对于 `constexpr` 是否成功修饰，仍然要看编译器是否允许（就像 `inline` 一样），**并不是加上就一定是常量表达式的**。

### `consteval` 常量值修饰符

`constexpr` 关键字可以指定函数在编译期执行，但不能保证一定在编译期执行。采用以下 `constexpr` 函数：

```cpp
constexpr double inchToMm(double inch) { return inch * 25.4; }
```

如果按以下方式调用，则会在编译时满足需要对函数求值：

```cpp
constexpr double const_inch {6.0};
constexpr double mm1 {inchToNm(const_inch)};    // at compile time
```

如果按照以下方式调用，函数将不会在编译期被求值，而是在运行时：

```cpp
double dynamic_inch {8.0};
double mm2 {inchToMm(dynamic_inch)};    // at run time
```

如果确实希望保证始终在编译时对函数进行求值，则需要使用C++20的 `consteval` 关键字将函数转换为 **立即函数(immediate function)** 。可以按照如下方式更改 `inchToMm()` 函数：

```cpp
consteval double inchToMm(double inch) { return inch * 25.4; }
```

现在，对 `inchToMm()(double inch)` 的第一次调用仍然可以正常编译，并且可以在编译期进行求值。但是，第二个调用现在会导致编译错误，因为无法在编译期对其进行求值。