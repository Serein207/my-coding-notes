# Chapter6. Modern C++ 类型

> 本节将介绍C++类型转换和自动类型推导的用法。

- [Chapter6. Modern C++ 类型](#chapter6-modern-c-类型)
  - [6.1 `const_cast`](#61-const_cast)
  - [6.2 `std::bit_cast()`](#62-stdbit_cast)
  - [6.3 `dynamic_cast` 运行时类型识别 *(RTTI)*](#63-dynamic_cast-运行时类型识别-rtti)
  - [6.4 `typeid` 运算符和 `type_info` 类](#64-typeid-运算符和-type_info-类)
  - [6.5 自动类型推导 `auto`](#65-自动类型推导-auto)
    - [6.5.1 `auto&` 语法](#651-auto-语法)
    - [6.5.2 `auto*` 语法](#652-auto-语法)
  - [6.6 自动类型推导 `decltype`](#66-自动类型推导-decltype)
    - [6.6.1 `decltype` 的推导规则](#661-decltype-的推导规则)
    - [6.6.2 `decltype` \& `auto`](#662-decltype--auto)

## 6.1 `const_cast`

可以使用它为变量添加或取消const属性。这是一个例子：

```cpp
void ThirdPartyLibraryMethod(char* str);

void f(const char* str) {
  ThirdPartyLibraryMethod(const_cast<char*>(str));
}
```

此外，标准库提供了一个名为 `std::as_const()` 的辅助方法，该方法定义在 `utility` 中，该方法接收一个引用参数，返回它的const引用版本。基本上，`std::as_const(obj)` 等价于 `const_cast<const T&>(obj)` ，其中T是obj的类型。与使用 `const_cast` 相比，使用 `std::as_const()` 可以使代码更短更易读。其基本用法如下：

```cpp
std::string str{"C++"};
const std::string& constStr{as_const(str)};
```

## 6.2 `std::bit_cast()`

C++20引入 `std::bit_cast()`，它定义在 `<bit>` 中。这是标准库唯一的强制类型转换，其他的强制转换是C++语言本身的一部分。 `bit_cast()` 与 `reinterpret_cast()` 类似，但它会创建一个指定目标类型的新对象，并按位从源对象复制到此新对象。它有效地将源对象的位解释为目标对象的位。 `bit_cast` 要求源对象与目标对象的大小相同，并且两者都是可复制的。示例如下：

```cpp
float asFloat { 1.23f };
auto asUnit { std::bit_cast<unsigned int>(asFloat) };
if (std::bit_cast<float>(asUnit) == asFloat) { 
  std::cout << "Roundtrip success." << std::endl; 
}
```

> 普通可复制类型是，组成对象的底层字节可以复制到一个数组中（比如char）。如果数组的数据随后被复制回对象，则该对象保持其原始值。

`bit_cast()` 的一个用例是可复制类型的二进制I/O。比如，可以将此类型的各个字节写入文件。当文件读回到内存时，可以使用 `bit_cast()` 正确地解释从文件中读取的字节。

## 6.3 `dynamic_cast` 运行时类型识别 *(RTTI)*

构建多态时，如果需要调用派生类对象的非虚函数，则需要强转父类指针类型为目标对象派生类指针类型，但必须保证目标对象正确
```cpp
class Base {
  public:
   virtual void func1() = 0;
};

class Derived1 : public Base {
  public:
   void func1() { std::cout << "derived1" << std::endl; }
};

class Derived2 : public Base {
  public:
   void func1() { std::cout << "derived2" << std::endl; }
   void func2() { std::cout << "hello" << std::endl; }
};

class Derived3 : public Base {
  public:
   void func1() { std::cout << "derived3" << std::endl; }
};

int main() {
   Base* d2 = new Derived2;
   d2->func1();
   // d1->func2();                   // error，func2不是虚函数
   Derived2* d22 = (Derived2*)d2;    // 将基类指针转化为派生类指针
   d22->func2();
   delete d2;
}
```

`dynamic_cast` 运算符用指向基类的指针来生成派生类的指针，它不能回答指针指向的是什么类对象的问题，但能回答 **是否可以安全地将指针的地址复制给特定指针类型** 的问题

```cpp
Derived* derived = dynamic_cast<Derived*>(base)
```

如果转换成功，`dynamic_cast` 返回对象地址，否则返回 `nullptr`

上述代码可以改为：

```cpp
Derived2* d22 = dynamic_cast<Derived2*>(d2); 
if(d22 != nullptr){
  d22->func2();
}
delete d2;
```

* `dynamic_cast`只适用于包含虚函数的类；
* `dynamic_cast`可以将派生类指针转化为基类指针，但这种做法没有意义，派生类指针可以直接赋值给基类指针；
* `dynamic_cast`可以用于引用，但是，没有与控制很对应的引用值，如果转换请求不正确，会出现`bad_cast`异常（C++11取消异常）；


## 6.4 `typeid` 运算符和 `type_info` 类

```cpp
typeid(Type);
typeid(expr);
```

* `typeid`运算符返回`type_info`类（在头文件`<typeinfo>`中定义）的对象的引用

```cpp
class A {
 public:
   A() {}
};

int main() {
  int i = 3;
  int* pi = &i;
  int& ri = i;

  // 内置数据类型
  std::cout << "typeid(int)=" << typeid(int).name() << std::endl;
  std::cout << "typeid(i)=" << typeid(i).name() << std::endl;
  std::cout << "typeid(int*)=" << typeid(int*).name() << std::endl;
  std::cout << "typeid(pi)=" << typeid(pi).name() << std::endl;
  std::cout << "typeid(int&)=" << typeid(int&).name() << std::endl;
  std::cout << "typeid(ri)=" << typeid(ri).name() << std::endl;

  A a;
  A* pa = &a;
  A& ra = a;

  // 自定义数据类型
  std::cout << "typeid(A)=" << typeid(A).name() << std::endl;
  std::cout << "typeid(a)=" << typeid(a).name() << std::endl;
  std::cout << "typeid(A*)=" << typeid(A*).name() << std::endl;
  std::cout << "typeid(pa)=" << typeid(pa).name() << std::endl;
  std::cout << "typeid(A&)=" << typeid(A&).name() << std::endl;
  std::cout << "typeid(ra)=" << typeid(ra).name() << std::endl;
}
```
**output**
*g++编译器*
```
typeid(int)=i
typeid(i)=i
typeid(int*)=Pi
typeid(pi)=Pi
typeid(int&)=i
typeid(ri)=i
typeid(A)=1A
typeid(a)=1A
typeid(A*)=P1A
typeid(pa)=P1A
typeid(A&)=1A
typeid(ra)=1A
```

`typeid`在实际开发中仅用于调试，不同编译器的输出结果不同

* `type_info`类的实现随编译器而异，但至少有`name()`成员函数，该函数返回一个字符串，通常是类名
* `type_info`重载了`==`和`!=`运算符，用于进行类型比较

```cpp
if(typeid(A) == typeid(a))
  std::cout << "a type is A" << std::endl;
if(typeid(A) == typeid(*pa))
  std::cout << "*pa type is A" << std::endl;
if(typeid(A) == typeid(ra))
  std::cout << "ra type is A" << std::endl;
```
**output**
```
a type is A
*pa type is A
ra type is A
```

***注意***
* `type_info`类的构造函数是private属性，没有拷贝构造函数，无法实例化，只能由编译器在内部实例化；
* 不建议用`name()`成员函数返回的字符串作为判断类型的依据（编译器可能会转换类型名）；
* `typeid`运算符可以用于多态场景，在运行阶段识别对象类型；
  ```cpp
  Base* derived = new Derived;
  if(typeid(Derived*) == typeid(derived)) // derived是Base*类型
    std::cout << "derived type is Derived*" <<std::endl;
  if(typeid(Derived) == typeid(*derived)) // 对derived解引用是Derived类型
    std::cout << "*derived type is Derived" << std::endl;
  ```
  **output**
  ```
  *derived type is Derived
  ```
* 假设有表达式`typeid(*ptr)`，当`ptr`是空指针时，如果`ptr`是多态类型，会引发`bad_typeid`异常。

## 6.5 自动类型推导 `auto`

* 在C和C++98中，`auto`关键字用于修饰变量（自动存储的局部变量）
* 在C++11中，赋予了`auto`新的含义，作为类型指示符，只是编译器在编译时推导`auto`声明的变量的数据类型
* 在Linux平台下，编译需要加`-std=c++11`参数

关键字 `auto` 有多种不同的用法：

- 推断函数的返回类型
- 结构化绑定
- 推断表达式的类型
- 推断非类型模板参数的类型
- 简写函数模板的语法
- `decltype(auto)`
- 其他函数语法
- 泛型lambda表达式

`auto` 可用于告诉编译期，在编译时自动推断变量的类型。下面的代码演示了在这种情况下关键字 `auto` 最简单的用法：

```cpp
auto x{123}; // x is type of int
```

假定 `getFoo()` 函数有一个复杂的返回类型，也可以简单地使用 `auto` ，让编译器推断出该类型。

```cpp
auto result{getFoo()};
```

这样，可方便地更该函数的返回类型，而不需要更新代码中调用该函数的所有位置。

### 6.5.1 `auto&` 语法

使用 `auto` 推断类型时去除了引用和 `const` 限定符。假设有以下函数：

```cpp
const std::string message{"Test"};
const std::string& foo() { return message; }
```

可以调用foo()，把结果存储在一个变量中，将该变量的类型指定为 `auto` ，如下所示：

```cpp
auto f1{foo()};
```

工具函数 `std::as_const()` 返回其引用参数的 `const T&` 版本，将 `as_count()` 与 `auto` 结合使用时要小心。由于自动去除了引用和const限定符，因此以下结果的变量类型为string，而不是const string&类型，因此将进行复制：

```cpp
std::string str {"C++"};
auto result {std::as_count(str)};
```

### 6.5.2 `auto*` 语法

`auto` 关键字也可用于指针，下面是一个例子：

```cpp
int i{123};
auto p{&i};
```

p的类型是 `int*`。与上一节中讨论的引用不同，此处不存在意外复制的危险。但是，在使用指针时，建议使用 `auto*` 语法，因为它可以更清楚地指出涉及指针。例如：

```cpp
auto* p{&i};
```

此外，使用 `auto*` 代替 `auto` 确实可以将 `auto` `const` 和指针一起使用的奇怪行为。假设你编写以下内容：

```cpp
const auto p1{&i};
```

大多数情况下，不会发生你期待的事情。

通常，当使用const时，你想保护指针所指向的东西。你可能认为p1的类型为 `const int*`，但实际上，该类型为 `int* const`，因此它是指向非const整数的const指针。按如下所示将const放在auto后面无济于事，类型仍然是 `int* const`。

```cpp
auto const p2{&i};
```

当 `auto*` 与 `const` 结合使用时，它的行为就会与期望的一样。这是个例子：

```cpp
auto* const p4{&i};
```

最后，使用这个语法可以令指针和整数都是const：

```cpp
const auto* const p5{&i};
```

p5的类型是 `const int* const`。如果省略了 `*`，将不能得到这个结果。

## 6.6 自动类型推导 `decltype`

在C++11中，**`decltype` 操作符用于查询表达式的数据类型**

```cpp
decltype(expr) var;
```

`decltype`分析表达式并得到它的类型，**不会计算表达式或执行函数**。

### 6.6.1 `decltype` 的推导规则

* 如果expr是没用用括号括起来的标识符，则var的类型与该标识符的类型相同，包括`const`等限定符；
* 如果expr是函数调用，则var的类型与函数返回值相同（函数不能返回`void`，但可以返回`void*`）；
* 如果expr是左值（能取地址），或者括号括起来的标识符，那么var的类型是expr的引用；
* 如果上面的条件都不满足，则var的类型与expr的类型相同。

```cpp
short a = 5;
decltype(a) da;         // short
short* pa = &a;
decltype(pa) dpa;       // short*
short& ra = a;
short b = 0;
decltype(ra) dra = b;   // short&
```

```cpp
int func() {
  std::cout << "func" << std::endl;
  return 3;
}

int main() {
  decltype(func()) da;              // int
  decltype(func) dfa;               // int dfa()
  decltype(func)* dpfa = func;      // int (*dpfa)()
  dpfa();                           // 执行func()
}
```

```cpp
decltype((a)) dra = b;               // short&
decltype((func)) drfa = func;        // int (&da)()
drfa();                              // 执行func()
```

### 6.6.2 `decltype` & `auto`
两者都可以完成类型推导，但是有本质区别
```cpp
decltype(func()) da;        // 不需要初始化，不执行函数
auto aa = func();           // 需要初始化，执行函数
```

如果需要多次使用`decltype`，可以使用`typedef`和`using`