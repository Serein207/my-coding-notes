# Modern C++ 函数模板&变量模板

- [Modern C++ 函数模板\&变量模板](#modern-c-函数模板变量模板)
  - [函数模板](#函数模板)
    - [函数模板重载](#函数模板重载)
    - [对类模板参数推导的更多介绍](#对类模板参数推导的更多介绍)
    - [函数模板的返回类型](#函数模板的返回类型)
  - [简化函数模板的语法](#简化函数模板的语法)
  - [变量模板](#变量模板)

## 函数模板

可以为独立函数编写模板。例如，可以编写一个通用函数，该函数在数组中查找一个值并返回这个值的索引：

```cpp
static const size_t NOT_FOUND = static_cast<size_t>(-1);

template <typename T>
size_t Find(const T& value, const T* array, size_t size) {
  for(size_t i = 0; i < size; ++i) {
    if(array[i] == value) {
      return i;
    }
  }
  return NOT_FOUND;
}
```

这个 `Find()` 函数可用于任何的类型的数组。

可通过两种方式调用这个函数：一种是尖括号显式指定类型，另一种是忽略类型，让编译器根据实参自动推导类型。下面举一些例子：

```cpp
int myInt = 3, intArray[] = {1, 2, 3, 4};
const size_t sizeIntArray = std::size(intArray);

size_t res;
res = Find(myInt, intArray, sizeIntArray);      // calls Find<int> by deduction
res = Find<int>(myInt, intArray, sizeIntArray); // calls Find<int> explicitly
if(res != NOT_FOUND) 
  std::cout << res << std::endl;
else
  std::cout << "Not found" << std::endl;

double myDouble = 3.14, doubleArray[] = {1.1, 2.2, 3.3, 4.4};
const size_t sizeDoubleArray = std::size(doubleArray);

res = Find(myDouble, doubleArray, sizeDoubleArray);         // calls Find<double> by deduction
res = Find<double>(myDouble, doubleArray, sizeDoubleArray); // calls Find<double> explicitly
if(res != NOT_FOUND) 
  std::cout << res << std::endl;
else
  std::cout << "Not found" << std::endl;

// res = Find(myInt, doubleArray, sizeDoubleArray); // compile error
// calls Find<double> explicitly, even with myInt
res = Find<double>(myInt, doubleArray, sizeDoubleArray);
```

前面的 `Find()` 函数的实现需要把数组的大小作为一个参数。有时编译器知道数组的确切大小，例如，基于堆栈的数组。用这种数组调用 `Find()` 函数，就不需要传递数组的大小。为此，可添加如下函数模板。该实现仅把调用传递给前面的 `Find()` 函数模板。这也说明函数模板可接收非类型的参数，与类模板一样。

```cpp
template <typename T, size_t N>
size_t Find(const T& value, constT(&arr)[N]) {
  return Find(value, arr, N);
}
```

这个版本的 `Find()` 函数语法有些特殊，但其用法相当直接，如下所示：

```cpp
int myInt { 3 }, intArray[] = {1, 2, 3, 4};
size_t res { Find(myInt, intArray) };
```

函数模板定义（不只是原型）必须对所有使用它们的源文件可用。因此，如果多个源文件使用函数模板，或使用显式实例化，就应把其定义放在头文件中。

函数模板的模板参数可以具有默认值，这与类模板一样。

> **注意**
>
> C++标准库提供了功能功能比这里的 `Find()` 函数模板更强大的模板函数 `std::find()`。

### 函数模板重载

C++语言允许编写函数模板特化，就像类模板特化一样。但是，很少会这么做，由于这样的函数模板特化不参与重载决议，因此可能会出现意外的行为。

可以使用非模板函数来重载函数模板。例如，加入想为 `const char*` 类型的C风格字符串编写一个 `Find()` 重载，以便与 `strcmp()` 进行比较，而不是使用 `operator==`，因为 `==` 只会比较地址，而不是实际的字符串。下面是一个这样的重载：

```cpp
size_t Find(const char* value, const char** arr, size_t size) {
  for(size_t i { 0 }; i < size; i++) {
    return i;
  }
  return NOT_FOUND;
}
```

这个函数重载的使用示例如下：

```cpp
const char* word { "two" };
const char* words[] { "one", "three", "four" };
const size_t sizeWords { std::size(words) };
size_t res { Find(word, words, sizeWords) };    // calls non-template function
```

如果确实像下面这样显式地指定模板参数类型，那么函数模板将被 `T = const char*` 调用，而不是 `const char*` 的重载。

```cpp
res = Find<const char*>(word, words, sizeWords);
```

### 对类模板参数推导的更多介绍

编译器根据传递给函数模板的实参来推导函数模板的参数类型；而对于无法推导的模板参数，则需要显式指定。例如，如下 `add()` 函数模板需要3个模板参数：返回值类型以及两个操作数类型。

```cpp
template <typename RetType, typename T1, typename T2>
RetType add(const T1& t1, const T2& t2) { return t1 + t2; }
```

调用这个模板函数时，可指定如下所有3个参数：

```cpp
auto result { add<long long, int, int>(1, 2) };
```

但由于模板参数T1和T2是函数的参数，编译器可以推导这两个参数，因此调用 `add()` 时可以仅指定返回值类型：

```cpp
auto result { add<long long>(1, 2) };
```

当然，仅在要推导的参数位于参数列表的最后时，这才可行。假设以如下方式定义函数模板：

```cpp
template <typename T1, typename RetType, typename T2>
RetType add(const T1& t1, const T2& t2) { return t1 + t2; }
```

必须指定 `RetType` 的模板参数类型，编译器无法推导该类型。但由于 `RetType` 是第二个参数，因此必须显式指定T1：

```cpp
auto result { add<int, long long>(1, 2) };
```

也可以提供返回类型模板参数的默认值，这样调用 `add()` 时可不指定任何类型。

```cpp
template <typename RetType = long long, typename T1, typename T2>
RetType add(const T1& t1, const T2& t2) { return t1 + t2; }
...
auto result { add(1, 2) };
```

### 函数模板的返回类型

继续分析 `add()` 函数模板的示例，使编译器推导返回值类型岂不是更好？但返回类型取决于模板参数类型，如何才能做到这一点？例如，考虑如下模板函数：

```cpp
template <typename T1, typename T2>
RetType add(const T1& t1, const T2& t2) { return t1 + t2; }
```

在这个示例中， `RetType` 应当是表达式t1+t2的类型，但由于不知道T1和T2是什么类型，因此并不知道这一点。

从C++14起，可要求编译器自动推导函数的返回类型。因此，只需编写如下 `add()` 模板函数：

```cpp
template <typename T1, typename T2>
auto add(const T1& t1, const T2& t2) { return t1 + t2; }
```

但是，使用 `auto` 推导表达式类型是去掉了引用和const限定符；但是 `decltype` 没有去除这些限定符。这种剥离对于其他的函数模板可能并不理想，所以需要考虑如何避免这种剥离。

再继续函数模板示例之前，首先使用一个非模板示例，来看看 `auto` 和 `decltype` 的区别。

```cpp
const std::string message { "Test" };
const std::string& getString() { return message; }
```

可以调用 `getString()`，并且将结果存储在 `auto` 类型的变量中：

```cpp
auto s1 { getString() };
```

由于 `auto` 会去掉引用和const限定符，因此s1的类型将会是string，并产生一个副本。如果需要一个const引用，可将其显式地设置为引用，并标记为const，如下所示：

```cpp
const auto s2 { getString() };
```

另一个解决方案是 `decltype` ，它不会去掉引用和const限定符。

```cpp
decltype(getString()) s3 { getString() };
```

这里，s3的类型将会是 `const string&`，但存在代码冗余，因为需要指定 `getString()` 两次。如果 `getString()` 是更复杂的表达式，这就会很麻烦。

为了解决这个问题，可以使用 `decltype(auto)` ：

```cpp
decltype(auto) s4 { getString() };
```

s4的类型也是 `const string&`。

了解到这些以后，可以使用 `decltype(auto)` 编写 `add()` 函数，以避免去掉任何const和引用限定符：

```cpp
template <typename T1, typename T2>
decltype(auto) add(const T1& t1, const T2& t2) { return t1 + t2; }
```

在C++14之前，不支持推导函数的返回类型和 `decltype(auto)` 。C++11引入的 decltype(expression) 解决了这个问题。例如，或许会编写如下代码：

```cpp
template <typename T1, typename T2>
decltype(t1 + t2) add(const T1& t1, const T2& t2) { return t1 + t2; } // compile error
```

但这是错误的。在开头使用了t1和t2，但这些尚且不知。在语义分析器到达参数列表的末尾时，才能知道t1和t2。

通常使用 **替换函数语法(alternative function syntax)** 来解决这个问题。注意，在这种语法中，`auto` 被用于原型行的开头，而实际返回类型是在参数列表之后指定的（后置返回类型）；因此，在解析时参数的名称（以及参数的而理性，因此也包括类型t1+t2）是已知的。

```cpp
template <typename T1, typename T2>
auto add(const T1& t1, const T2& t2) -> decltype(t1 + t2) { return t1 + t2; }
```

> **注意**
>
> C++支持自动返回类型推导和 `decltype(auto)` ，建议使用其中的一种机制，而不要使用替换函数语法。

## 简化函数模板的语法

C++20引入了一个简化函数模板的语法。对于前一节的 `add()` 函数，下面是推荐的版本：

```cpp
template <typename T1, typename T2>
decltype(auto) add(T1& t1, T2& t2) { return t1 + t2; }
```

在这里，指定一个简单的函数模板是一种相当冗长的语法。使用简化函数模板的语法，可以优雅地编写如下代码：

```cpp
decltype(auto) add(const auto& t1, const auto& t2) { return t1 + t2; }
```

使用这种语法，不再有 `template<>` 规范来指定模板参数。这种简化的语法只是语法糖：编译器会自动将这个简化的实现转换为更长的原始代码。基本上，每个被指定为 `auto` 的函数参数都称为模板类型参数。

但必须牢记两个注意事项：

```cpp
template <typename T>
decltype(auto) add(const T& t1, const T& t2) { return t1 + t2; }
```

这个版本只有一个模板参数类型，函数的两个参数t1和t2都是 `const T&` 类型。对于这样的函数模板，不能使用简化的语法，因为这将转换为具有2个不同模板参数类型的函数模板。

第二个问题是，不能在函数模板的实现中显式使用推导的类型，因为这些自动推导的类型没有名称。如果需要这样做，那么可以继续使用普通的模板函数语法，或者使用 `decltype(auto)` 。

## 变量模板

除了类模板、类方法模板和函数模板外，C++还支持变量模板。语法如下：

```cpp
template <typename T>
constexpr T pi { T { 3.141592653589793238462643383279502884 } };
```

这是pi值的可变模板。为了在某种类型中获得pi值，可以使用如下语法：

```cpp
float piFloat { pi<float> };
auto piLongDouble { pi<long double> };
```

这样总会得到在所有请求的类型中可表示的pi的近似值。与其他类型的模板一样，模板变量也可以特化。

> **注意**
>
> C++20引入了 `<numbers>`，它定义了一组常用的数字常量，包括pi:`std::numbers::pi`。
