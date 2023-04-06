# Chapter19. variant, any, tuple

- [Chapter19. variant, any, tuple](#chapter19-variant-any-tuple)
  - [19.1 variant](#191-variant)
  - [19.2 any](#192-any)
  - [19.3 tuple](#193-tuple)

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

为避免异常，可使用 `std::ger_if<index>()` 或 `get_if<T>()` 辅助函数。这些函数接收指向variant的指针，返回指向请求值的指针；如果遇到错误，返回nullptr。

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

