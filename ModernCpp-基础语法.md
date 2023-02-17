# Modern C++ 基础语法

### 变量

下面的代码给出了两种风格的变量声明方式：

```cpp
int uninitializedInt;
int initializedInt {7};
std::cout << std::format("{} is a random value", uninitializedInt) << std::endl;
std::cout << std::format("{} was assigned an initial value", initializedInt) << std::endl;
```

`initializedInt` 变量使用统一初始化语法进行初始化。也可以使用下面的赋值语法来初始化：

```cpp
initializedInt = 7;
```

`<cstddef>` 提供了 `std::byte` 类型用来表示单个字节。在C++17之前，`char` 或者 `unsigned char` 用来表示一个字节，但是那些类型使得像在处理字符。

`std::byte` 却能指明意图，即内存中的单个字节。一个byte可以用如下的方式初始化：

```cpp
std::byte b {42};
```

### 数值极限

C++提供了一种获取数值极限信息的标准方式，使用定义在 `<limits>` 中的类模板 `std::numeric_limits` 。下面是一些例子：

```cpp
std::cout << "int:\n";
std::cout << std::format("Max int value: {}\n", std::numeric_limits<int>::max());
std::cout << std::format("Min int value: {}\n", std::numeric_limits<int>::min());
std::cout << std::format("Lowest int value: {}\n", std::numeric_limits<int>::lowest());

std::cout << "\ndouble:\n";
std::cout << std::format("Max double value: {}\n", std::numeric_limits<double>::max());
std::cout << std::format("Min double value: {}\n", std::numeric_limits<double>::min());
std::cout << std::format("Lowest double value: {}\n", std::numeric_limits<double>::lowest());
```

output *运行结果与环境有关*
```
int:
Max int value: 2147483647
Min int value: -2147483648
Lowest int value: -2147483648

double:
Max double value: 1.7976931348623157e+308
Min double value: 2.2250738585072014e-308
Lowest double value: -1.7976931348623157e+308
```

注意 `min()` 和 `lowest()` 之间的区别。对于一个证书，最小值等于最低值。然而对于浮点类型来说，最小是表示该类型能表示的最小正数，最低值表示该类型能表示的最小负数，即 `-max()`。

这里有几个特殊的浮点数：
- `+infinity` `-infinity`：表示正无穷和负无穷。
- `NaN`：非数字的缩写，如0除以0的结果。
  
可以用 `std::isnan()` 判断一个给定的浮点数是否为非数字，用 `std::isinf()` 判断是否为无穷，这两个函数都定义在 `<cmath>` 中。

可以用 `std::numeric_limits` 获取这些特殊的浮点数，例如 `std::numeric_limits<double>::infinity` 。

### 零初始化

可以用一个 `{0}` 的统一初始化将变量初始化为0，0在这里是可选的。一对空的花括号组成的统一初始化器 `{}` 称为零初始化器。零初始化会将原始的整数类型初始化为`0`，将怨怒是的浮点类型初始化为`0.0`，将指针类型初始化为 `nullptr`，将对象用默认构造函数初始化。

下面是 `float` 和 `int` 零初始化的例子：

```cpp
float myFloat{};
int mtInt{};
```

### 枚举类型

整数代表某个数字序列中的值。枚举类型允许你定义自己的序列，这样你就能使用这个数列中的值声明变量。例如，在一个国际象棋程序中，可以用 `int` 代表所有棋子，用常量代表棋子的类型，代码如下：

```cpp
const int PieceTypeKing{0};
const int PieceTypeQueen{1};
const int PieceTypeRook{2};
const int PIeceTypePawn{3};
// etc.
int myPiece{PieceTypeKing};
```

这种做法存在一定的风险，因为棋子只是一个 `int` ，如果另一个程序增加棋子的值，或是将某个棋子的值设置为-1，这会导致不可控行为发生。

强类型的枚举类型通过定义变量的取值范围解决了上述问题。下面的代码声明了一个新类型 `PieceType` ，这个类型具有4个可能的值，分别代表4中国际象棋棋子：

```cpp
enum class PieceType {King, Queen, Rook, Pawn};
```

这种新类型可以像下面这样使用

```cpp
PieceType piece{PieceType::King};
```

事实上，枚举类型只是一个整型值。King、Queen、Rook、Pawn的实际值分别是0、1、2、3，还可以为枚举成员指定整型值，其语法如下：

```cpp
enum class PieceType {
  King = 1,
  Queen,
  Rook = 10,
  Pawn
};
```

如果你没有为当前枚举成员赋值，编译器会将上一个枚举成员的值递增1，再赋予当前枚举成员。如果没有给第一个枚举成员赋值，编译器就它赋值0。

尽管枚举值内部是由整型值表示的，它却不会自动转换为整数。因此，下面的代码是不合法的：

```cpp
if (PieceType::Queen == 2) {...}
```

默认情况下，枚举值的基本类型是整型，但可采用以下方式加以改变：

```cpp
enum class PieceType : unsigned long {
  King = 1;
  Queen,
  Rook = 10;
  Pawn
};
```

对于 `enum class` ，枚举值不会自动超出封闭的作用域，这意味着它们不会与定义再父作用域的其他名字重提。所以，不同的强类型枚举可以拥有同名的枚举值。例如，以下两个枚举类型是完全合法的：

```cpp
enum class State {Unknown, Started, Finished};
enum class Error {None, BadInput, DiskFull, Unknown};
```

然而，这意味着必须使用枚举值的全名，或者使用 `using enum` 或 `using` 声明，像下文描述的那样。

从C++20起，可以用 `using enum` 声明来避免使用枚举值全名。这是一个例子：

```cpp
using enum PieceType;
PieceType piece{King};
```

另外，可以使用 `using` 声明避免使用某个特定枚举值的全名。例如，在下面的代码片段中，King可以不用全名就被使用，但是其他枚举值仍要使用全名。

```cpp
using PiceType::King;
PieceType piece{King};
piece = PieceType::Queen;
```

> **警告**
>
> C语言中 `enum XXX` 仍允许使用，但它是类型不安全的。

### `if` 初始化器

C++允许在if语句中包括一个初始化器，语法如下：

```cpp
if (<initializer>; <conditional_expression>) {
  <if_body>
} else if (<else_if_expression>) {
  <else_if_body>
} else {
  <else_body>
}
```

`<initializer>` 中引用的任何变量只在 `<conditional_expression>` `<if_body>` `<else_if_expression>` `<else_if_body>` `<else_body>` 中可用，之外不可用。

示例：

```cpp
if (Employee employee{getEmployee()}; employee.salary > 1000) {...}
```

### `switch` 初始化器

与if语句一样，可以在switch语句中使用初始化器。语法如下：

```cpp
switch (<initializer>; <expression>) {<body>}
```

`<initializer>` 中引用的任何变量只在 `<expression>` `<body>` 中可用，之外不可用。

### 三向比较运算符

`<=>` 三向比较运算符可用于确定两个值的大小顺序。它也被称为太空飞船运算符。使用单个表达式，它可以告诉你一个值是否等于、小于或大于另一个值。它不能返回bool类型，它返回枚举(enumeration-like) ^[不是真正的枚举类型。这些排序不能用在 `switch` 语句中，也不能用 `using enum` 声明] 类型，定义在 `<compare>` 中。

对于 `a <=> b` ，如果操作数是整型，则结果是所谓的强排序，并且可以是以下之一：

- `std::strong_ordering::less` ：a < b
- `std::strong_ordering::greater` ：a > b
- `std::strong_ordering::equal` ：a = b

如果操作数是浮点型，结果是一个偏序(partial ordering)
- `std::partial_ordering::less` ：a < b
- `std::partial_ordering::greater` ：a > b
- `std::partial_ordering::equivalent` ：a = b
- `std::partial_ordering::unordered` ：操作数中存在非数字

以下是它的示例用法：

```cpp
int i{11};
std::strong_order result{1 <=> 0};
if (result == std::strong_ordering::less) std::cout << "less" << std::endl;
if (result == std::strong_ordering::greater) std::cout << "greater" << std::endl;
if (result == std::strong_ordering::equal) std::cout << "equal" << std::endl;
```

还有一种弱排序，这是可以选择的另一种排序类型，以针对你自己的类型实现三向比较：
- `std::weak_ordering::less` ：a < b
- `std::weak_ordering::greater` ：a > b
- `std::weak_ordering::equivalent` ：a = b

对于原始类型，与仅使用 `==` `>` `<` 运算符进行单个比较相比，使用三向比较运算符不会带来太多收益。但是，它对于比较昂贵的对象很有用。使用三向比较运算符，可以使用单个运算符对此类对象进行排序，而不用潜在地调用两个独立地比较运算符。

最后， `<compare>` 提供命名的比较函数来解释排序结果。这些函数是 `std::is_eq()` `std::is_neq()` `std::is_lt()` `std::is_lteq()` `std::is_gt()` `std::is_gteq()` 。分别表示==、!=、<、<=、>、>=，返回bool类型。下面是一个例子：

```cpp
int i{11};
std::strong_ordering result{i <=> 0};
if (std::is_lt(result)) std::cout << "less" << std::endl;
if (std::is_gt(result)) std::cout << "greater" << std::endl;
if (std::is_eq(result)) std::cout << "equal" << std::endl;
```

### 属性

属性是一种将可选的和/或特定于编译器厂商的信息添加到源代码中的机制。从C++11开始，通过使用双括号语法 `[[attribute]]` 对属性进行标准化的支持。

1. `[[nodiscard]]`

可用于一个有返回值的函数，使编译器在该函数被调用却没有对返回的值进行任何处理时发出警告，以下是一个例子：

```cpp
[[nodiscard]] int func() {
  return 42;
}

int main() {
  func();
}
```

例如，此特性可用于返回错误代码的函数。通过 `[[nodiscard]]` 属性，错误代码就无法被忽视。

更笼统地说，`[[nodiscard]]` 可用与类，函数和枚举。

C++20起，可以字符串形式为 `[[nodiscard]]` 提供一个原因，例如：

```cpp
[[nodiscard("Some explanation")]] int func();
```

2. `[[maybe_unused]]`

可用于禁止编译器在未使用某些内容时发出警告，如下所示：

```cpp
int func([[maybe_used]] int param) {
  return 42;
}
```

`[[maybe_unused]]` 属性可用于类和结构体，非静态数据成员，联合，typedef，类型别名，变量，函数，枚举以及枚举值。

3. `[[noreturn]]`
  
向函数添加 `[[noreturn]]` 属性意味着它永远不会将控制权返回给调用点。通常，函数要么导致某种终止，要么引发异常。使用此属性，编译器可以避免发出警告或错误，因为它现在可以更多地了解该函数的用途。这是一个例子：

```cpp
[[noreturn]] void forceProgramTermination() {
  std::exit(1);
}

bool isDongleAvailable() {
  bool isAvailable{false};
  // check whether a licensing dongle is available ...
  return isAvailable;
}

bool isFeatureLicensed(int featuredId) {
  if (!isDongleAvailable) {
    // No licensing dongle found, abort program execution!
    forceProgramTermination();
  } else {
    bool isLicensed{featured == 42};
    // Dongle is available, perform license check of the given feature ...
    return isLicensed;
  }
}

int main() {
  bool isLicensed{isFeatureLicensed(42)};
}
```

此段代码可以正常编译，没有任何警告或错误。

4. `[[deprecated]]`

`[[deprecated]]` 可用于某些内容标记为已弃用。这意味着可以使用它，但不鼓励使用。此属性接受一个可选参数，该参数可用于解释弃用的原因，如以下示例所示：

```cpp
[[deprecated("Unsafe method, please use xyz")]] void func();
```

如果使用了已经弃用的函数，你将会收到编译器错误或警告。

5. `[[likely]]` 和 `[[unlikely]]`

这些可能性属性可用于帮助编译器优化代码。例如，这些属性可用于某个分支被采用的可能性来标记if和switch语句的分支。请注意，很少需要这些属性。如今，编译器和硬件具有强大的分支预测功能，可以自行解决。但在某些情况下，例如对于性能至关重要的代码，可能需要帮助编译器。语法如下：

```cpp
int value{...};
if (value > 11) [[unlikely]] {...}
else {...}

switch (value) {
  [[likely]] case 1:
  ...
  break;
  case2:
  ...
  break;
  [[unlikely]] case 3:
  ...
  break;
}
```

### `std::pair`

`std::pair` 类模板定义在 `<utility>` 中。它将两个可能不同类型的值组合在一起。可通过 `first` 和 `second` 公共数据成员访问这些值。这是一个例子：

```cpp
std::pair<double, int> myPair{1.23, 5};
std::cout << std::format("{} {}", myPair.first, mtPair.second) << std::endl;
```

pair也支持CTAD，所以你可以按下列方式定义myPair：

```cpp
std::pair myPair{1.23, 5};
```

### `std::optional`

在 `<optional>` 中定义的 `std::optional` 保留特定的值类型，或者不包含任何值。

基本上，如果想要允许值是可选的，可以将optional用于函数的参数。如果函数可能返回也可能不返回某些内容，则通常也将optional作为函数的返回类型。

`std::optional` 类型是一个类模板，因此必须要在尖括号间指定所需的实际类型，如 `std::optional<int>` 。

这是一个返回 `std::optional` 的例子：

```cpp
std::optional<int> getData(bool giveIt) {
  if (giveIt) {
    return 42;
  }
  return std::nullopt; // or simply return {};
}
```

可以按下列方式调用这个函数：

```cpp
std::optional<int> data1{ getData(true) };
std::optional<int> data2{ getData(false) };
```

可以用 `has_value()` 方法判断一个 `std::optional` 是否有值，或简单地将 `std::optional` 用在if语句中：

```cpp
std::optional<int> data1{ getData(true) };
std::optional<int> data2{ getData(false) };

std::cout << "data1.has_value() = " << data1.has_value() << std::endl;
if (data2) {
	std::cout << "data2 has a value." << std::endl;
}
```

如果optional有值，可以使用 `value()` 或解引用运算符访问它。

```cpp
std::cout << "data1.value = " << data1.value() << std::endl;
std::cout << "data1.value = " << *data1 << std::endl;
```

如果你对一个空的optional使用 `value()` ，将会抛出 `std::bad_optional_access` 异常。

`value_or()` 可以用来返回optional的值，如果optional为空，则返回指定的值。

```cpp
std::cout << "data2.value = " << data2.value_or(0) << std::endl;
```

请注意，不能将引用保存在optional中，所以 `std::optional<T&>` 是无效的。但是，可以将指针保存在optional中。

### 基于范围的 `for` 循环

这种循环允许方便地迭代容器中的元素。这种循环允许方便地迭代容器中地元素。这种循环类型可用于C风格的数组、初始化列表，也可用于任何具有返回迭代器的 `begin()` 和 `end()` 方法的类型，例如 `std::array` `std::vector` 以及其他容器。

下面定义了一个包含四个整数的数组，此后“基于范围的for循环”遍历数组中的每个元素的副本，输出每个值。为在迭代元素时不创建副本，应使用引用变量。

```cpp
std::array arr{ 1, 2, 3, 4 };
for(int ele : arr) {
  std::cout << ele << std::endl;
}
```

#### 基于范围的 `for` 循环初始化器 

C++20起，可以在基于范围的for循环中使用初始化器，与if和switch语句中的用法类似，语法如下：

```cpp
for (std::array{1, 2, 3, 4}; int i : arr) { std::cout << i << std::endl; }
```

### 结构化绑定

结构化绑定允许声明多个变量，这些变量使用数组、结构体、pair、或元组中的元素以初始化。

例如，假设有下面的数组

```cpp
std::array values{11, 22, 33};
```

可声明3个变量x、y、z，像下面这样使用数组中的3个值进行初始化。注意，必须使用 `auto` 关键字。例如，不能用 `int` 代替 `auto` 

```cpp
auto [x, y, z]{values};
```

使用结构化绑定声的变量数量必须与右侧表达式的值数量匹配

如果所有非静态成员都是公有的，也可以将结构化绑定用于结构体

```cpp
struct Point {double m_x, m_y, m_z;};
Point point;
point.m_x = 1.0;  point.m_y = 2.0;  point.m_z = 3.0;
auto [x, y, z]{point};  
```

正如最后一个例子，以下代码中将pair中的元素分解为单独的变量

```cpp
pair myPair{"hello", 5};
auto [theString, theInt]{myPair};
std::cout << theString << std::endl;
std::cout << theInt << std::endl;
```

通过 `auto&` 或 `const auto&` 代替 `auto` 还可以使用结构化绑定语法创建一组对非const的引用或const引用。

### 初始化列表

初始化列表在 `<initializer_list>` 头文件中定义；利用初始化列表，可轻松地编写能接受可变数量参数的函数。

`std::initializer_list` 是一个模板，要求在尖括号间指定列表中的元素类型。下面演示如何使用初始化列表：

```cpp
import <initializer_list>;

int makeSum(std::initializer_list<int> values) {
	int total{ 0 };
	for (int value : values) {
		total += value;
	}
	return total;
}
```

`makeSum()` 函数接收一个整数类型的初始化列表作为参数。函数体累加总数。可按如下方式使用该函数：

```cpp
int a{ makeSum({1, 2, 3}) };
int b{ makeSum({10, 20, 30, 40, 50, 60}) };
```

初始化列表是类型安全的，列表中所有元素必须为同一类型。尝试用double数值进行调用，将导致编译器报错：

```cpp
int c { makeSum(1, 2, 3.0) };
```
