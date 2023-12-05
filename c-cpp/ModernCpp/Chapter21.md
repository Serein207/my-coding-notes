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
    - [21.4.2 使用类型关系](#2142-使用类型关系)
    - [21.4.3 使用条件类型trait](#2143-使用条件类型trait)
    - [21.4.4 使用enable\_if](#2144-使用enable_if)
    - [21.4.5 使用constexpr if简化enable\_if结构](#2145-使用constexpr-if简化enable_if结构)
    - [21.4.6 逻辑运算符trait](#2146-逻辑运算符trait)
    - [21.4.7 静态断言](#2147-静态断言)

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

当然，你可能永远都不会采用这种方式使用类型trait。只有结合模板根据类型的某些属性生成代码时，类型trait才更有用。下面的模板示例演示了这一点。代码动议了函数模板processHelper()的两个重载版本，这个函数模板接收一种类型作为模板参数。第一个参数是一个值，第二个参数是true_type或false_type实例。process()函数模板接收一个参数，并调用processHelper()函数：

```cpp
template <typename T>
void processHelper(const T& t, true_type) {
  cout << t << " is an integral type." << endl;
}

template <typename T>
void processHelper(const T& t, false_type) {
  cout << t << " is not an integral type." << endl;
}

template <typename T>
void process(const T& t) {
  processHelper(t, typename is_integral<T>::type{});
}
```

processHelper()函数调用的第二个参数定义如下：

```cpp
typename is_integral<T>::type{}
```

该参数使用is_integral判断T是否为整数类型。使用 `::type` 访问结果integral_constant类型，可以是true_type或false_type。processHelper()函数需要true_type或false_type的一个实例作为第二个参数，这也是 `::type` 后面有 `{}` 的原因。注意，processHelper()函数的两个重载版本使用了类型为true_type或false_type的匿名参数，因为在函数体内没有使用这些参数，这些参数仅用于函数重载解析。

前面的例子只是使用单个函数模板来编写，但没有说明如何使用类型trait，以基于类型选择不同的重载。

```cpp
template <typename T>
void process(const T& t) {
  if constexpr (is_integral_v<T>) {
    cout << t << " is an integral type." << endl;
  } else {
    cout << t << " is not an integral type." << endl;
  }
}
```

### 21.4.2 使用类型关系

有3中类型关系：is_same, is_base_of和is_convertible。下面给出一个例子来展示如何使用is_same。其余类型关系的工作原理类似。

下面的same()函数模板通过is_same类型trait特性判断两个给定参数是否类型相同，然后输出相应的信息。

```cpp
template <typename T1, typename T2>
void same(const T1& t1, const T2& t2) {
  bool areTypesTheSame { is_same_v<T1, T2> };
  cout << format("'{}' and '{}' are {} types.", t1, t2,
    (areTypesTheSame ? "the same" : "difference")) << endl;
}
int main() {
  same(1, 32);
  same(1, 3.01);
  same(3.01, "test"s);
}
```

**output**

```
'1' and '32' are the same types. 
'3.01' and 'test' are difference types.
'3.01' and 'test' are difference types.
```

### 21.4.3 使用条件类型trait

标准库辅助函数模板 `std::move_is_noexcept()` 可以根据移动构造函数是否标记为noexcept，来有条件地调用移动构造函数还是拷贝构造函数。标准库没有提供类似的辅助函数模板，根据移动赋值运算符是否标记为noexcept，选择调用移动赋值运算符还是拷贝赋值运算符。现在已经了解了模板元编程和类型trait，来看看如何实现自己的 `move_assign_if_noexcept()`。

如果移动构造函数标记为noexcept，move_if_noexcept()只会将给定的引用转换为右值引用，否则将转换为const引用。move_assign_if_noexcept()需要做类似的事情，如果移动赋值运算符标记为noexcept，则将给定的引用转换为右值引用，否则将转换为const引用。

`std::conditional` 类型trait可用于实现条件，而 `is_nothrow_move_assignable` 类型trait可用于判断某个类型是否有标记为noexcept的移动赋值运算符。条件类型trait有3个模板参数：一个布尔型，一个布尔型标记为true的类型以及一个布尔型为false的类型。下面是整个函数模板：

```cpp
template <typename T>
constexpr std::conditional<std::is_nothrow_move_assignable_v<T>, 
                           T&&, const T&>::type
move_assign_if_noexcept(T& t) noexcept {
  return std::move(t);
}
```

C++标准为具有类型成员（比如conditional）的trait定义了别名模板。它们与trait具有相同的名称，但是附加了 `_t`。对于如下写法：

```cpp
std::conditional<std::is_no_throw_move_assignable_v<T>, T&&, const T&>::type
```

可以改为这样写：

```cpp
std::conditional_t<std::is_nothrow_move_assignment_v<T>, T&&, const T&>
```

可以对move_assign_if_noexcept()函数模板进行以下测试：

```cpp
class MoveAssignable {
public:
  MoveAssignable& operator=(const MoveAssignable&) {
    cout << "copy assign" << endl; return *this;
  }
  MoveAssignable& operator=(MoveAssignable&&) {
    cout << "move assign" << endl; return *this;
  } 
};

class MoveAssignableNoexcept {
public:
  MoveAssignableNoexcept& operator=(const MoveAssignableNoexcept&) {
    cout << "copy assign" << endl; return *this;
  }
  MoveAssignableNoexcept& operator=(MoveAssignableNoexcept&&) {
    cout << "move assign" << endl; return *this;
  } 
};

int main() {
  MoveAssignable a, b;
  a = move_assign_if_noexcept(b); 
  MoveAssignableNoexcept c, d;
  c = move_assign_if_noexcept(d);
}
```

**output**

```
copy assign
move assign
```

### 21.4.4 使用enable_if

使用enable_if需要了解 **“替换失败不是错误”(Substitution Failure Is Not An Error, SFINAE)** 特性。它规定，为一组给定的模板参数特化函数模板失败不会被视为编译错误。相反，这样的特化应该从函数重载集合中移除。下面仅讲解SFINAE的的基础知识。

如果有一组重载函数，就可以使用enable_if根据某些类型特性有选择地仅有某些重载。enable_if通常用于重载函数的返回类型。enable_if接收两个模板类型参数。第一个参数是布尔值，第二个参数是默认为void的类型。如果布尔值是true，enable_if类就有一种可使用 `::type` 访问的嵌套类型，这种嵌套类型由第二个模板类型参数给定。如果布尔值是false，就没有嵌套类型。

通过enable_if，可将前面使用same()函数模板的例子重写为一个重载的checkType()函数模板。在这个版本中，checkType()函数根据给定值的类型是否相同，返回true或false。如果不希望checkType()返回任何内容，可删除return语句，可删除enable_if的第二个模板类型参数，或用void替换。

```cpp
template <typename T1, typename T2>
enable_if_t<is_same_v<T1, T2>, bool>
checkType(const T1& t1, const T2& t2) {
  cout << format("'{}' and '{}' are the same types.", t1, t2) << end;
  return true;
}

template <typename T1, typename T2>
enable_if_t<!is_same_v<T1, T2>, bool>
checkType(const T1& t1, const T2& t2) {
  cout << format("'{}' and '{}' are different types.", t1, t2) << endl;
  return false;
}

int main() {
  checkType(1, 32);
  checkType(1, 3.01);
  checkType(3.01, "test"s);
}
```

**output**

```
'1' and '32' are the same types. 
'3.01' and 'test' are difference types.
'3.01' and 'test' are difference types.
```

上述代码定义了两个重载的checkType()，它们的返回类型都是enable_if的嵌套类型bool。首先，通过is_same_v检查给定的值的类型是否相同，然后通过enable_if_t获得结果。当enable_if_t的第一个参数为true时，enable_if_t的类型就是bool；当第一个参数为false时，将不会有返回类型。这就是SFINAE发挥作用的地方。

当编译器编译main()函数的第一行时，它试图找到接收两个整型值的checkType()函数。编译器会在源码中找到第一个重载的checkType()函数模板，并将T1和T2都设置为整数，以推断可使用这个模板的实例。然后，编译器会尝试确定返回类型。由于这两个参数是整数，因此是相同的类型，`is_same_v<T1, T2>` 将返回true，这导致 `enable_if_t<true, bool>` 返回类型bool。这样实例化时一切都很好，编译器可使用该版本的checkType()。

当编译器尝试编译main()函数的第二行时，编译器会再次尝试找到合适的checkType()函数。编译器从第一个checkType()开始，判断出可将T1设置为int类型，将T2设置为double类型。然后，编译器会尝试确定返回类型。由于这两个参数都是整数，这一次，T1和T2是不同的类型，checkType()函数不会有返回类型。编译器会注意到这个错误，但由于SFINAE，还不会产生真正的编译错误。编译器将正常回溯，并试图找到另一个checkType()函数。在这种情况下，第二个checkType()可以正常工作，因为 `!is_same_v<T1, T2>` 为true，此时 `enable_if_t<true, bool>` 返回类型bool。

如果希望在一组构造函数上使用enable_if，就不能将它用于返回类型，因为构造函数没有返回类型。此时可在带默认值的额外构造函数参数上使用enable_if。

建议慎用enable_if，仅在需要解析重载歧义时使用，即无法使用其他技术（例如特化、concepts等）解析重载歧义时使用。例如，如果只希望在对模板使用了错误类型时编译失败，应使用concepts，或者静态断言，而不是SFINAE。当然，enable_if有合法的用例。一个例子是为类似于自定义矢量的类特化复制函数，使用enable_if和is_trivially_copyable类型trait对普通的可复制类型执行按位复制（例如C函数memcpy()）。

> **警告**
>
> 依赖于SFINAE是一件很棘手和复杂的事。如果有选择地使用SFINAE和enable_if禁用重载集中地错误重载，就会得到奇怪的编译错误，这些错误很难跟踪。

### 21.4.5 使用constexpr if简化enable_if结构

某些情况下，C++17引入的constexpr if特性有助于极大地简化enable_if。

例如，假设有以下两个类：

```cpp
class IsDoable {
public:
  void doit() const { cout << "IsDoable::doit()" << endl; }
};

class Derived : public IsDoable {};
```

可创建一个函数模板callDoit()。如果方法可用，它调用doit方法；否则输出错误消息。为此，可使用enable_if，检查给定类型是否从IsDoable派生：

```cpp
template <typename T>
enable_if_t<is_base_of_v<IsDoable, T>, void> callDoit(T& t) { t.doit(); }

template <typename T>
enable_if_t<!is_base_of_v<IsDoable, T>, void> callDoit(T& t) { 
  cout << "Cannot call doit()!" << endl;
}
```

对该实现进行测试：

```cpp
Derived d;
callDoit(d); 
callDoit(123);
```

**output**

```
IsDoable::doit()
Cannot call doit()!
```

使用constexpr if可极大地简化enable_if实现：

```cpp
template <typename T>
void callDoit(const T& [[maybe_unused]] t) {
  if constexpr (is_base_of_v<IsDoable, T>) {
    t.doit();
  } else {
    cout << "Cannot call doit()!" << endl;
  }
}
```

使用constexpr if语句，如果提供了并非从IsDoable派生的类型，`t.doit()` 一行甚至不会编译！

不使用is_base_of类型trait，也可使用is_invocable trait，这个trait可用于确定在调用给定函数时是否可以使用一组给定的参数。下面是is_invocable trait的callDoit()实现：

```cpp
template <typename T>
void callDoit(const T& [[maybe_unused]] t) {
  if constexpr(is_invocable_v<decltype(&IsDoable::doit), T>) {
    t.doit();
  } else {
    cout << "Cannot call doit()!" << endl;
  }
}
```

### 21.4.6 逻辑运算符trait

在3种逻辑运算符trait： **串联(conjunction)**、**分离(disjunction)** 与 **否定(negation)**。以 `_v` 结尾的可变模板也可供使用。这些trait接收可变数量的模板类型参数，可用于在类型trait上执行逻辑操作，如下所示：

```cpp
cout << conjunction_v<is_integral<int>, is_integral<short>> << " ";
cout << conjunction_v<is_integral<int>, is_integral<double>> << " ";

cout << disjunction_v<is_integral<int>, is_integral<double>, 
        is_integral<short>> << " "; 	

cout << negation_v<is_integral<int>> << " ";
```

**output**

```
1 0 1 0
```

### 21.4.7 静态断言

static_assert允许在编译期对断言求值。断言需要是true，如果断言是false，编译器就会报错。static_assert调用接收两个参数：编译期求值的表达式和字符串。当表达式为false时，编译期将给出包含指定字符串的错误提示。下例核实是否在使用64位编译器进行编译：

```cpp
static_assert(sizeof(void*) == 8, "Requires 64-bit complication.");
```

如果编译器使用32位编译器，指针是4B，编译器将给出错误提示，如下所示：

```
test.cpp(3): error C2338: Requires 64-bit complication.
```

从C++17开始，字符串参数变为可选的，如下所示：

```cpp
static_assert(sizeof(void*) == 8);
```

此时，如果表达式的计算结果是false，将得到与编译器相关的错误信息。

static_assert可以和类型trait结合使用。示例如下：

```cpp
template <typename T>
void foo(const T& t) {
  static_assert(is_integral_v<T>, "T should be an integral type.");
}
```

推荐使用C++20的concepts替代带有类型trait的static_assert()。例如：

```cpp
template <std::integral T>
void foo(const T& t) {}
```

或者使用C++20简化的函数模板语法：

```cpp
void foo(const std::integral auto& t) {}
```