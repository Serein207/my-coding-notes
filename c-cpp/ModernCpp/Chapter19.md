# Chapter19. variant, any, tuple

- [Chapter19. variant, any, tuple](#chapter19-variant-any-tuple)
  - [19.1 variant](#191-variant)
  - [19.2 any](#192-any)
  - [19.3 tuple](#193-tuple)
    - [19.3.1 分解元组](#1931-分解元组)
      - [1. 结构化绑定](#1-结构化绑定)
      - [2. tie()](#2-tie)
    - [19.3.2 串联](#1932-串联)
    - [19.3.3 比较](#1933-比较)
    - [19.3.4 make\_from\_tuple()](#1934-make_from_tuple)
    - [19.3.5 apply()](#1935-apply)

## 19.1 variant

`std::variant` 在 `<variant>` 中定义，可以保存给定类型集合的一个值。定义variant时，必须指定它可能包含的类型。例如，以下代码定义variant一次可以包含整数、字符串或浮点值：

```cpp
variant<int, string, float> v;
``` 

variant的模板类型参数必须是唯一的；例如，`variant<int, int>` 无效。这里，默认构造的variant包含第一个类型（此处是int）的默认构造值。要默认构造variant，务必确保variant的第一个类型是默认可构造的。例如，下面的代码无法编译，因为Foo不是默认可构造的。

```cpp
class Foo { public: Foo() = delete; Foo(int) {} };
class Bar { public: Bar() = delete; Bar(int) {} };
...
variant<Foo, Bar> v;
```

Foo和Bar都不是默认可构造的。如果仍需要默认构造variant，可使用 `std::monostate`（一个表现好的空替代）作为variant的第一个类型：

```cpp
variant<monostate, Foo, Bar> v;
```

可使用赋值运算符，在variant中存储内容：

```cpp
variant<int, string, float> v;
v = 12;
v = 12.5f;
v = "An std::string"s;
```

variant在任何时候只能包含一个值。因此，对于这三个赋值语句，首先将整数12存储在variant中，然后将variant改为包含浮点值，最后将variant改为包含字符串。

可使用index()方法来获取当前存储在variant中的值类型的索引（以0开始）。可以使用 `std::holds_alternative()` 函数模板来确定variant当前是否包含特定类型的值：

```cpp
cout << "Type index: " << v.index() << endl;
cout << "Contains an int: " << holds_alternative<int>(v) << endl; 
```

**output**

```
Type index: 1
Contains an int: 0
```

使用 `std::get<index>()` 或 `get<T>()` 从variant检索值，其中index是想要检索类型的索引，T是想要检索的类型。如果使用类型的索引，或使用与variant的当前值不匹配的类型，这些函数抛出bad_variant_access异常。

```cpp
cout << get<string>(v) << endl;
try {
  cout << get<0>(v) << endl;
} catch (const bad_variant_access& ex) {
  cout << "Exception: " << ex.what() << endl;
}
```

**output**

```
An std::string
Exception: bad variant access
```

为避免异常，可使用 `std::get_if<index>()` 或 `get_if<T>()` 辅助函数。这些函数接收指向variant的指针，返回指向请求值的指针；如果遇到错误，返回nullptr。

```cpp
string* theString { std::ger_if<string>(&v) };
int* theInt { std::get_if<int>(&v) };
cout << "retrieved string: " << (theString ? *theString : "null") << endl;
cout << "retrieved int: " << (theInt? *theInt : 0) << endl;
```

**output**

```
retrieved string: An std::string
retrieved int: 0
```

可使用 `std::visit()` 辅助函数，将visitor模式应用于variant。visitor必须是可调用对象，可接受可能存储在variant中的任何类型。假设以下类定义了多个重载的函数调用运算符，variant中的每个可能类型对应一个。

```cpp
class MyVisitor {
public:
  void operator()(int i) { cout << "int " << i << endl; }
  void operator()(const string& s) { cout << "string " << s << endl; }
  void operator()(float f) { cout << "float " << f << endl; }
};
```

可将其与 `std::visit()` 一起使用，如下所示：

```cpp
visit(MyVisitor{}, v);
```

这样就会根据variant中当前存储的值，调用适当的重载的函数调用运算符。这个示例的输出如下：

```
string An std::string
```

variant不能存储数组，就像optional一样，不能再variant中存储引用。可以存储指针，`reference_wrapper<const T>` 或 `reference_wrapper<T>` 的实例。

## 19.2 any

`std::any` 在 `<any>` 中定义，是一个可包含任意类型值得类。可以使用任意构造函数或 `std::make_any()` 辅助函数创建实例。一旦构建，可确认any实例中是否包含值，以及所包含值的类型。要访问包含的值，需要使用 `any_cast()` ，如果失败，会抛出bad_any_cast类型的异常。下面是一个示例：

```cpp
any empty;
any anInt { 3 };
any aString { "An std::string"s };

cout << "empty.has_value = " << empty.has_value() << endl;
cout << "anInt.has_value = " << anInt.has_value() << endl << end;

cout << "anInt wrapped type = " << anInt.type().name() << endl;
cout << "aString wrapped type = " << aString.type().name() << endl << endl;

int theInt { any_cast<int>(anInt) };
cout << theInt << endl;
try {
  int test { any_cast<int>(aString) };
  cout << test << endl;
} catch (const bad_any_cast& e) { 
  cout << "Exception: " << e.what() << endl;
}
```

**output**

```
empty.has_value = 0
anInt.has_value = 1

anInt wrapped type = int
aString wrapped type = class std::basic_string<char, struct std::char_traits<char>, class std::allocator<char>>

3
Exception: Bad any_cast
```

可将新值赋值给any实例，甚至是不同类型的值：

```cpp
any something { 3 };    // Now it contains an integer
something = "An std::string"s; // Now it contains a string 
```

any的实例可存储在标准库容器中，这样就可在单个容器中存放异构数据。这么做的唯一缺点在于，只能通过显式执行any_cast来检索特定值，如下所示：

```cpp
vector<any> v;
v.push_back(42);
v.push_back("An std::string"s); 

cout << any_cast<string>(v[1]) << endl;
```

与optional和variant一样，无法存储any实例的引用。可存储指针，也可存储`reference_wrapper<const T>` 或 `reference_wrapper<T>` 的实例。

## 19.3 tuple

`std::pair` 可以保存两个值，每个值都有特定的类型。每个值的类型都应该在编译期确定。下面是一个简单的例子：

```cpp
pair<int, string> p1 { 16, "Hello World." };
pair p2 { true, 0.123f }; // CTAD
cout << format("p1 = {{}, {})", p1.first, p1.second) << endl;
cout << format("p2 = {{}, {}}}\n", p2.first, p2.second) << endl;
```

**output**

```
p1 = {16, Hello World.} 	
p2 = {true, 0.123}
```

还有 `std::tuple` 类，这个类定义在 `<tuple>` 中。tuple（元组）是pair的泛化，允许存储任意数量的值，每个值都有自己的特定类型。和pair一样，tuple的大小和值类型都是编译期确定的，都是固定的。

tuple可通过构造函数创建，需要指定模板类型和实际值。例如，下面的代码创建了一个tuple，其中第一个元素是一个整数，第二个元素是一个字符串，最后一个元素是一个布尔值：

```cpp
using MyTuple = tuple<int, string, bool>; 
MyTuple t1 { 16, "Test", true };
```

`std::get<i>()` 从tuple中获得第i个元素，i是从0开始的索引。返回值的类型是tuple中那个索引位置的正确类型：

```cpp
cout << format("t1 = ({}, {}, {})", get<0>(t1), get<1>(t1), get<2>(t1)) << endl; 
// out: t1 = (16, Test, 1)
```

可通过 `<typeinfo>` 中的typeid()检查 `get<i>` 是否返回了正确的类型。下面这段代码的输出表明， `get<1>(t1)` 的返回值确实是std::string。

```cpp
cout << "Type of get<1>(t1) = " << typeid(get<1>(t1)).name() << endl;
// out: Type of get<1>(t1) = class std::basic_string<char, struct std::char_traits<char>, class std::allocator<char>>
```

可以使用 `std::tuple_element` 类模板在编译期根据元素的索引获取元素的类型。tuple_element要求指定元组的类型，而不是像t1那样的实际元组实例。下面是示例：

```cpp
cout << "Type of element with index 2 = "
     << typeid(tuple_element<2, MyTuple>::type).name() << endl; 		
// out: Type of element with index 2 = bool
```

也可以根据类型使用 `std::get<T>()` 从tuple中提取元素，其中T是要提取的元素（而不是索引）的类型。如果tuple有多个所需类型的元素，编译器会发生错误。例如，可从t1中提取字符串元素：

```cpp
cout << "String = " << get<string>(t1) << endl;
// out: String = Test
```

迭代tuple的值并不简单。无法编写简单循环或调用 `get<i>(MyTuple)` 等，因为i的值在编译期必须确定。一种可能的解决方案是使用模板元编程，这在以后会讲到。

可通过 `std::tuple_size` 模板查询tuple的大小。和tuple_element一样，tuple_size要求指定tuple的类型而不是实例：

```cpp
cout << "Tuple size = " << tuple_size<MyTuple>::value << endl;
// out: Tuple size = 3
```

如果不知道准确的tuple类型，可以使用decltype()来查询类型，如下所示：

```cpp
cout << "Tuple size = " << tuple_size<decltype(t1)>::value << endl;
// out: Tuple size = 3
```

通过模板参数推导(CTAD)，在构造tuple时，可忽略模板参数类型，让编译期根据传递给构造函数的实参类型自动进行推导。例如，下面定义同样的t1元组，它包含一个整数、一个字符串和一个布尔值：

```cpp
std::tuple t1 { 16, "Test"s, true };
```

因为类型自动推导，不能通过&指定阴影。如果需要通过构造函数的CTAD方式生成一个包含引用或常量引用的tuple，那么需要分别使用ref()和cref()。例如，下面的构造会生成一个类型为 `tuple<int, const double&, string&>` 的tuple：

```cpp
double d { 3.14 };
string str1 { "Test" };
std::tuple t2 { 16, ref(d), cref(d), ref(str1) };
```

为测试元组t2中的double引用，下面的代码首先将double的值输出，然后调用 `get<1>(t2)`，这个函数实际上返回的是对d的引用，因为第二个tuple（索引1）元素使用了ref(d)。第二行修改引用的变量的值，最后一行展示了d的值的确通过保存在tuple中的引用修改了。注意，第三行不能通过编译，因为cref(d)用于第三个tuple元素，也就是说，它是d的常量引用。

```cpp
cout << "d = " << d << endl;
get<1>(t2) *= 2;
// get<1>(t2) *= 2; // Error: cerf()
cout << "d = " << get<1>(t2) << endl; 
// out: d = 3.14;
//      d = 6.28
```

如果不适用构造函数的CTAD方法，可以使用 `std::make_tuple()` 工具函数创建一个tuple。由于它是函数模板，支持模板参数推导，因此允许通过仅指定实际值来创建元组。在编译期自动推导类型，例如：

```cpp
auto t2 { std::make_tuple(16, ref(d). cref(d), ref(str1)) };
```

### 19.3.1 分解元组

可采用两种方法，将一个元组分解为单独的元素：结构化绑定以及std::tie()。

#### 1. 结构化绑定

结构化绑定在C++17中可用，允许方便地将一个元组分解为多个变量。例如，下面的代码定义了一个tuple，这个tuple包括一个整数，一个字符串和一个布尔值；此后，使用结构化绑定，将这个tuple分解为3个独立的变量：

```cpp
tuple t1 { 16, "Test"s, true };
auto [i, str, b] { t1 };
cout << format("Decomposed: i = {}, str = \"{}\", b = {}", i, str, b) << endl;
```

还可以将元组分解为引用，允许通过引用修改元组的内容，示例如下：

```cpp
auto& [i2, str2, b2] { t1 };
i2 *= 2;
str2 = "Hello world";
b2 = !b2;
```

使用结构化绑定，无法在分解元组时忽略特定元素。如果元组包含3个元素，则结构化绑定需要3个变量。如果想忽略元素，则必须使用tie()。

#### 2. tie()

如果在分解元组时不使用结构化绑定，可以使用 `std::tie()` 工具函数，它生成一个引用的tuple。下例首先创建一个tuple，然后创建3个变量，将这些变量出书。`tie(i, str, b)` 会创建一个tuple，其中包含对i, str和b的引用。使用赋值运算符，将tuple t1赋给tie()的结果。由于tie()的结果是一个引用的tuple，赋值实际上更改了三个变量的值：

```cpp
tuple t1 { 16, "Test", true };
int i { 0 };
string str;
bool b { false }; 
cout << format("Before: i = {}, str = \"{}\", b = {}\n", i, str, b) << endl;
tie(i, str, b) = t1; 	
cout << format("After: i = {}, str = \"{}\", b = {}\n", i, str, b) << endl;
```

**output**

```
Before: i = 0, str = "", b = 0
After: i = 16, str = "Test", b = 1
```

有了tie()，可忽略一些不想分解的元素。并非使用分解值的变量名，而使用特殊的std::ignore值。例如，t1元组的string元素可以被忽略，用下面的语句替换前面的tie()语句：

```cpp
tie(i, ignore, b) = t1;
```

### 19.3.2 串联

通过 `std::tuple_cat()` 可将两个tuple串联为一个。在下面的例子中，t3的类型为 `tuple<int, string, bool, double, string>`：

```cpp
tuple t1 { 16, "Test"s, true };
tuple t2 { 3.14, "String 2"s };
auto t3 { tuple_cat(t1, t2) }; 	
```

### 19.3.3 比较

tuple支持所有比较运算符。为了能使用这些比较运算符，tuple中存储的元素类型也应该支持这些操作。例如：

```cpp
tuple t1 { 123, "def" };
tuple t2 { 123, "abc" };
if (t1 < t2) {
  cout << "t1 < t2" << endl;
} else {
  cout 	<< "t1 >= t2" << endl;
}
```

**output**

```
t1 >= t2
```

对于包含多个数据成员的自定义类型，tuple比较可用于方便地实现这些类型的按词典比较运算符。例如，如下的类包含3个数据成员：

```cpp
class Foo {
public:
  Foo(int i, string s, bool b) : m_int { i }, m_str { move(s) }, m_bool { b } {}
private:
  int m_int; 	
  string 	m_str;
  bool m_bool;
};
```

正确实现该类的完整比较运算符并不简单。通过 `std::tie()` 和C++20三向比较运算符，它就变成了简单的一行程序。下面是Foo类运算符 `<=>` 方法的实现：

```cpp
auto operator<=>(const Foo& rhs) {
  return tie(m_int, m_str, m_bool) <=>
   tie(rhs.m_int, rhs.m_str, rhs.m_bool);
}
```

### 19.3.4 make_from_tuple()

使用 `std::make_from_tuple<T>()` 可以构建一个T类型的对象，将给定tuple的元素作为参数传递给T的构造函数。例如，有以下类：

```cpp
class Foo {
public:
  Foo(string str, int i) : m_str { move(str) }, m_int { i } {}
private:
  string m_str;
  int m_int;
};
```

可按如下方式使用make_from_tuple()：

```cpp
auto myTuple { "Hello world.", 42 };
auto foo { make_from_tuple<Foo>(myTuple) };
```

从技术上讲，提供给make_from_tuple()的实参未必是一个tuple，但必须支持 `std::get<>()` 和 `tuple_size`。std::array和pair也满足这些要求。

### 19.3.5 apply()

`std::apply()` 调用给定的函数、lambda表达式和函数对象等，将给定tuple的元素作为实参传递，下面是一个例子：

```cpp
int add(int a, int b) { return a + b; }
...
cout << apply(add, tuple { 39, 3 }) << endl;
```