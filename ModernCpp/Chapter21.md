# Chapter21. Modern C++ 模板元编程

> 模板元编程的目标是在编译期而不是运行时执行一些计算。模板元编程基本上是介于另一种语言的一种小型编程语言。下面首先介绍一个简单示例，这个例子在编译期计算一个数的阶乘，并在运行时能将计算结果用作简单的常数。

- [Chapter21. Modern C++ 模板元编程](#chapter21-modern-c-模板元编程)
  - [21.1 编译期阶乘](#211-编译期阶乘)
  - [21.2 循环展开](#212-循环展开)
  - [21.3 打印元组](#213-打印元组)
    - [21.3.1 `constexpr if`](#2131-constexpr-if)
    - [21.3.2 使用编译期整数序列和折叠](#2132-使用编译期整数序列和折叠)
  - [21.4 类型trait](#214-类型trait)
    - [21.4.1 使用类型类别](#2141-使用类型类别)

## 21.1 编译期阶乘

下面的代码演示了在编译期如何计算一个数的阶乘。代码使用了模板递归，我们需要一个递归模板和用于停止递归的基本模板。根据数学定义，0的阶乘是1，所以用作基本情形：

```cpp
template <unsigned char f>
class Factorial {
public:
  static const unsigned long long value = f * Factorial<f - 1>::value; 	
};

template <>
class  Factorial<0> {
public:
  static const unsigned long long value { 1 };
};

int main() {
  cout << Factorial<6>::value << endl;
}
```

这将计算6的阶乘，数学表达式为 `6!`。

> **注意**
>
> 在编译期计算阶乘。在运行时，可通过 `::value` 访问编译期计算出来的值，这是一个静态常量值。

上面这个具体实例在编译期计算一个数的阶乘，但未必要使用模板元编程。可不使用模板，写成C++20的consteval immediate函数形式。不过，模板实现仍然是实现递归模板的优秀示例。

```cpp
constexpr unsigned long long factorial(unsigned char f) {
  if (f == 0) { return 1; }
  else { return f * factorial(f - 1); }
}
```

可以像调用其他函数一样调用factorial()，不同指出在于consteval函数保证在编译期执行。

## 21.2 循环展开

模板元编程的第二个例子是在编译期循环展开，而不是在运行时执行循环。注意 **循环展开(loop unrolling)** 应尽在需要时使用，因为编译器通常会自动展开可展开的循环。

这个例子再次使用了模板递归，因为需要在编译期在循环中完成一些事情。在每次递归中，Loop类模板都会通过i-1实例化自身。当到达0时，停止递归。

```cpp
template <int i>
class Loop {
public:
  template <typename FuncType>
  static inline void run(FuncType func) {
    Loop<i - 1>::run(func);
    func(i);
  }
};

template <>
class Loop<0> {
public:
  template <typename FuncType>
  static inline void run(FuncType /* func */) { }
};
```

可以像下面这样使用Loop模板：

```cpp
void doWork(int i) { cout << "doWork(" << i << ")" << endl; }

int main() {
  Loop<3>::run(doWork);
}
```

这段代码将导致编译器循环展开，并连续3次调用doWork()函数。这个程序的输出如下所示：

```cpp
doWork(1)
doWork(2) 
doWork(3)
```

## 21.3 打印元组

这个例子通过模板元编程来打印 `std::tuple` 的各个元素。与模板元编程的大部分情况一样，这个例子也使用了模板递归。tuple_print类模板接收两个模板参数：tuple类型和初始化为元组大小的整数。然后再构造函数中递归地实例化自身，每一次调用都将大小减小。当大小变成0时，tuple_print的一个部分特化停止递归。

```cpp
template <typename TupleType, int n>
class TuplePrint {
public:
  TuplePrint(const TupleType& t) {
    TuplePrint<TupleType, n - 1> tp { t };
    cout << get<n - 1>(t) << endl;
  }
};

template <typename TupleType>
class TuplePrint<TupleType, 0> {
public:
  TuplePrint(const TupleType& t) {}
};

int main() {
  using MyTuple = tuple<int, string, bool>;
  MyTuple t1 { 16, "Test", true};
  TuplePrint<MyTuple, tuple_size<MyTuple>::value> tp { t1 };
}
```

引入自动推导模板参数的辅助函数模板可以简化这段代码。简化的实现如下：

```cpp
template <typename TupleType, int n>
class TuplePrintHelper {
public:
  TuplePrintHelper(const TupleType& t) {
    TuplePrintHelper<TupleType, n - 1> tp { t };
    cout << get<n - 1>(t) << endl;
  }
};

template <typename TupleType>
class TuplePrintHelper<TupleType, 0> {
public:
  TuplePrintHelper(const TupleType&) {}
};

template <typename T>
void tuplePrint(const T& t) {
  tuplePrintHelper<T, tuple_size<T>::value> tph { t };
}

int main() {
  tuple t1 { 167, "Testing"s, false, 2.3 };
  tuplePrint(t1);
}
```

### 21.3.1 `constexpr if`

C++17引入了constexpr if。这些是在编译期执行的if语句。如果constexpr if语句的分支从未到达，就不会进行编译。这可用于简化大量的模板元变成技术。例如，可按如下方式使用constexpr if，简化前面的打印元组元素的代码。注意，不再需要模板递归基本情形，原因在于可通过constexpr if停止递归。

```cpp
template <typename TupleType, int n>
class TuplePrintHelper {
public:
  TuplePrintHelper(const TupleType& t) { 	
    if constexpr (n > 1) {
      TuplePrintHelper<TupleType, n - 1> { t };
    }
    cout << get<n - 1>(t) << endl;
  } 
};

template <typename T>
void tuplePrint(const T& t) { 	
  tuplePrintHelper<T, tuple_size<T>::value> tph { t }; 
}
```

现在，甚至可以丢弃类模板本身，替换为简单的函数模板tuplePrintHelper()：

```cpp
template <typename TupleType, int n>
void tuplePrintHelper(const TupleType& t) {
  if constexpr (n > 1) {
    tuplePrintHelper<TupleType, n - 1>(t);
  }
  cout << get<n - 1>(t) << endl;
}

template <typename T>
void tuplePrint(const T& t) {
  tuplePrintHelper<T, tuple_size<T>::value>(t);
}
```

可对其进一步简化，将两个函数合为一个：

```cpp
template <typename TupleType, int n = tuple_size<TupleType>::value>
void tuplePrint(const TupleType& t) {
  if constexpr (n > 1) {
    tuplePrintHelper<TupleType, n - 1>(t);
  }
  cout << get<n - 1>(t) << endl;
}
```

仍然像前面那样进行调用：

```cpp
tuple t1 { 167, "Testing"s, false, 2.3 };
tuplePrint(t1); 
```

### 21.3.2 使用编译期整数序列和折叠

C++使用 `std::integer_sequence` 支持编译期整数序列。模板元编程的一个常见用例是生成编译期索引序列，即size_t类型的整数序列。此处，可使用辅助用的 `std::index_sequence`。可使用 `std::index_sequence_for()` ，生成与给定的参数包等长的索引序列。

下面使用可变参数模板、编译期索引序列和折叠表达式，实现元组打印程序：

```cpp
template <typename Tuple, size_t... Indices>
void tuplePrintHelper(const Tuple& t, std::index_sequence<Indices...>) {
  ((std::cout << get<Indices>(t) << endl), ...);
}

template <typename... Args>
void tuplePrint(const std::tuple<Args...>& t) {
  tuplePrintHelper(t, std::index_sequence_for<Args...>{});
}
```

可按与前面相同的方式调用：

```cpp
tuple t1 { 167, "Testing"s, false, 2.3 };
tuplePrint(t1); 
```

调用时，tuplePrintHelper()函数模板的一元右折叠表达式为如下形式：

```cpp
(((cout << get<0>(t) << endl;),
((cout << get<1>(t) << endl),
((cout << get<0>(t) << endl),
(cout << get<3>(t) << endl)))));
```

## 21.4 类型trait

通过类型trait可在编译时根据类型做出决策。例如，可验证一个类型是否从另一个类型派生而来、是否可以转换为另一个类型、是否是整型等。C++标准提供了大量可供选择的类型trait类。所有与类型trait相关的功能都定义在 `<type_traits>` 中。

类型trait是一个非常高级的C++功能，这里不可能解释类型trait的所有细节。下面列举几个例子，展示如何使用类型trait。

### 21.4.1 使用类型类别

在给出使用类trait的模板示例前，首先要了解一下诸如 `std::is_integral` 的类的工作方式。C++标准对integral_constant类的定义如下所示：

```cpp
template <class T, T v>
struct integral_constant {
  static constexpr T value { v };
  using value_type = T;
  using type = integral_constant<T, v>;
  constexpr operator value_type() const noexcept { return value; }
  constexpr value_type operator()() const noexcept { return value; }
};
```

这也定义了bool_constant, true_type和false_type类型别名：

```cpp
template <bool B>
using bool_constant = integral_constant<bool, B>;

using true_type = bool_constant<true>; 	
using false_type = bool_constant<false>;
```

当调用 `true_type::value` 时，得到的值是true；调用 `false_type::value` 时，得到的值是false。还可调用 `true_type::type` 返回true_type类型。这同样适用于false_type。像is_integral这样检查类型是否为整型和is_class这样检查类型是否为类类型的类，都继承自true_type或者false_type。例如，is_integral为类型bool特化，如下：

```cpp
template <> struct is_integral<bool> : public true_type {}; 
```

这样就可以编写 `std::is_integral<bool>::value` ，并返回true。注意，不需要编写这些特化，这些是标准库的一部分。

下面的代码演示了使用类型类别的最简单例子：

```cpp
if (is_integral<int>::value) { cout << "int is integral" << endl; }
else { cout << "int is not integral" << endl; }

if (is_class<string>::value) { cout << "string is a class" << endl; }
else { cout << "string is not integral" << endl; }
```

**output**

```
int is integral
string is a class
```

对于每一个具有value的成员trait，C++添加了一个变量模板，它与trait同名，后跟_v。编写 `some_trait_v<T>`，例如 `is_integral_v<T>` 和 `is_const_v<T>` 等。下面用变量模板重写了前面的例子：

```cpp
if (is_integral_v<int>) { cout << "int is integral" << endl; }
else { cout << "int is not integral" << endl; }

if (is_class_v<string>) { cout << "string is a class" << endl; }
else { cout << "string is not integral" << endl; }
```

