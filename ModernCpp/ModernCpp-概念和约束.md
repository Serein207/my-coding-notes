# Chapter13. Modern C++ 概念和约束

> C++20引入了 **概念(concepts)**，命名需求来约束类模板和函数模板的模板类型和非类型参数。概念的主要目标是使与模板相关的编译器错误更具有可读性。
>
> 概念允许编译器在不满足某些类型约束时输出可读的错误消息。因此，为了得到有意义的语义错误，建议编写代码时使用概念来建模语义需求。避免只验证没有任何语义意义的语法方面的概念。例如只检查类型是否支持 `operator+`，这样的概念将只检查语法，而不是语义。 `std::string` 支持 `operator+`，但它与整数的 `operator+` 具有完全不同的含义。另一方面，诸如可排序和可交换的概念是使用概念对某些语义进行建模的好例子。

- [Chapter13. Modern C++ 概念和约束](#chapter13-modern-c-概念和约束)
  - [13.1 语法](#131-语法)
  - [13.2 约束表达式](#132-约束表达式)
    - [13.2.1 `requires` 表达式](#1321-requires-表达式)
      - [简单requirement](#简单requirement)
      - [类型requirement](#类型requirement)
      - [复合requirement](#复合requirement)
      - [嵌套requirement](#嵌套requirement)
    - [13.2.2 组合概念表达式](#1322-组合概念表达式)
  - [13.3 预定义的标准概念](#133-预定义的标准概念)
  - [13.4 类型约束的 `auto`](#134-类型约束的-auto)
  - [13.5 类型约束和函数模板](#135-类型约束和函数模板)
    - [约束包含](#约束包含)
  - [13.6 类型约束和类模板](#136-类型约束和类模板)
  - [13.7 类型约束和类方法](#137-类型约束和类方法)
  - [13.8 类型约束和模板特化](#138-类型约束和模板特化)

## 13.1 语法

概念定义的泛型语法如下：

```cpp
template <parameter_list>
concept concept_name = constrains_expression;
```

这里从熟悉的模板 `<>` 说明符开始，但与类模板和函数模板不同，概念从不会被实例化。接下来，使用一个新关键字 `concept`，然后是概念的名称。名称可以是任意的。constraints_expression 可以是任意的常量表达式。约束表达式必须要产生一个布尔值；但约束永远不会在运行期计算。下一节将详细讨论约束表达式。

概念表达式的语法如下：

```cpp
concept_name<argument_list>
```

概念表达式的计算结果为真或假。如果它的计算结果为真，那么使用给定的类型模板实参为概念建模。

## 13.2 约束表达式

计算结果为布尔值的常量表达式可以直接用作概念定义的约束。但他的结果必须精确计算为一个布尔值，并且没有任何类型转换。下面是一个示例：

```cpp
template <typename T>
concept C = sizeof(T) == 4;
```

随着概念的引入，还引入了一种新的常量表达式，称为 `require` 表达式，接下来对此进行解释

### 13.2.1 `requires` 表达式

`requires` 表达式的语法如下：

```cpp
requires (parameter_list) { requirements; }
```

parameter_list为可选参数。每个requirement必须以分号作为结尾。

有4种类型的requirement：简单requirement、类型requirement、复合requirement和嵌套requirement，所有这些将在接下来的部分中讨论。

#### 简单requirement

一个简单的requirement是一个任意的表达式语句，而不是以 `requires` 开头。不允许使用变量声明、循环、条件语句等并且这个表达式语句永远不会被计算；编译器也只是用于验证它是否已通过编译。

例如，下面的概念定义指定某种类型T必须是可递增的；也就是说，类型T必须支持后缀和前缀 `++` 运算符：

```cpp
template <typename T>
concept Incrementable = requires(T x) { x++; ++x; };
```

`requires` 表达式的参数列表引入位于 `requires` 表达式主体中的命名变量。并且 `requires` 表达式的主体不能有常规变量的声明。

#### 类型requirement

类型requirement用于验证特定类型是否有效。例如，下面的概念要求特定类型T有 `value_type` 成员：

```cpp
template <typename T>
concept C = requires { typename T::value_type; };
```

类型需求可以用来验证某个模板是否可以使用给定的类型进行实例化。下面是一个示例：

```cpp
template <typename T>
concept C = requires { typename SomeTemplate<T>; };
```

#### 复合requirement

符合requirement可以用于验证某些东西不会抛出任何异常和/或验证某个方法是否返回某个类型。语法如下：

```cpp
{ expression } noexcept -> type_constraint;
```

`noexcept` 和 `-> type_constraint` 都是可选的。例如，下面的概念验证给定类型是否具有标记为 `noexcept()` 的 `swap()` 方法：

```cpp
template <typename T>
concept C = requires (T x, T y) {
  { x.swap(y) } noexcept;
}
```

type_constraint可以是任何的类型约束。**类型约束(type constraint)** 只是一个概念的名称，它包含0个或多个模板类型参数。箭头左边表达式的类型自动作为类型约束的第一个模板参数进行传递。因此，类型约束的实参总是比对应概念定义的模板参数的数目少一个。例如，具有单一模板类型的概念定义的类型不需要任何模板实参；可以指定空参列表 `<>`，或者省略它们。

以下概念验证给定类型具有一个名为 `size()` 的方法，该方法返回的类型可转换为 `size_t` 的类型：

```cpp
template <typename T>
concept C = requires (const T x) {
  { x.size() } -> std::convertible_to<size_t>;
};
```

`std::convertible_to<From, To>` 是标准库在 `<concepts>` 中预定义的概念，它有两个模板类型参数。箭头左边的表达式的类型自动作为第一个模板类型参数传递给 `convertible_to` 的类型约束。因此，在这种情况下，只需要指定 `To` 模板参数类型实参（本例中为size_t)。

一个 `requires` 表达式可以有多个参数，并且可以由一系列需求组成。例如，下面的概念要求类型T的示例是可比较的：

```cpp
template <typename T>
concept Comparable = requires (const T a, const T b) {
  { a == b } -> std::convertible_to<bool>;
  { a < b} -> std::convertible_to<bool>;
  //...
};
```

#### 嵌套requirement

`requires` 表达式可以有嵌套的需求。例如，这里有一个概念，要求类型的大小为4个字节，并且该类型支持前缀和后缀的自增和自减操作：

```cpp
template <typename T>
concept C = requires (T t) {
  requires sizeof(t) == 4;
  ++t; --t; t++; t--;
};
```

### 13.2.2 组合概念表达式

现有的概念表达式可以使用 `&&` 和 `||` 逻辑运算符进行组合。例如，假设有一个递减的概念Decrementable。下面演示了一个概念，它要求一个类型既可以是自增的，也可以是自减的：

```cpp
template <typename T>
concept IncrementableAndDecrementable = Incrementable<T> && Decrementable<T>;
```

## 13.3 预定义的标准概念

标准库定义了一系列预定义的概念，分为若干类别。下面的列表给出了每个类别的一些示例概念，它们都定义在 `<concepts>` 和std命名空间中。

- 核心语言概念： `same_as`、`derived_from`、`convertible_to`、`integral`、`floating_point`、`copy_constructible` 等
- 比较概念： `equality_comparable`、`totally_ordered` 等
- 对象概念： `movable`、 `copyable` 等
- 可调用的概念： `invocable`、`predicate` 等

> 此外， `<iterator>` 定义了与迭代器相关的概念，如 `random_access_iterator`、`bidirectional_iterator`、`forward_iterator` 等。它还定义了算法 `requires` 表达式，如 `std::swap`、`std::move` 等。C++20范围程序库还是提供了许多标准概念。它们会在别的章节具体介绍。

如果需要这些标准概念中的任何一个，那么可以直接使用它们，而不必实现自己的。例如，下面的概念要求类型T派生自类Foo：

```cpp
template <typename T>
concept IsDerivedFromFoo = std::derived_from<T, Foo>;
```

下面的概念要求类型T可以转换为bool类型：

```cpp
template <typename T>
concept IsConvertibleToBool = std::convertible_to<T, bool>;
```

当然，这些标准概念也可以组合成更具体的概念。例如，下面的概念要求类型T既是默认的也是可复制的：

```cpp
template <typename T>
concept DefaultAndCopyConstructible = 
 std::default_initializable<T> && std::copy_constructible<T>;
```

> **注意**
>
> 编写完整且正确的概念并不总是那么容易的。如果可能，尝试使用可用的标准概念或它们的组合来约束类型。

## 13.4 类型约束的 `auto`

类型约束可用于约束用自动类型推导定义的变量，在使用函数返回类型推导时约束其返回类型，约束在简化函数模板和泛型lambda表达式中的参数，等等。

例如，下面的代码编译通过，类型被推导为 `int`，它模拟了Incrementable概念：

```cpp
Incrementable auto value1 { 1 };
```

但是，下面的操作导致编译错误。该类型被推导为 `std::string`，并且string不建模Incrementable：

```cpp
Incrementable auto value2 { "abc"s };
```

## 13.5 类型约束和函数模板

在函数模板中使用类型约束有几种不同的语法方式。第一种时使用熟悉的 `template<>` 语法，但不是使用 `typename` （或 `class`），而是使用类型约束。示例如下：

```cpp
template <std::convertible_to<bool> T>
void handle(const T& t);

template <Incrementable T>
void process(const T& t);
```

使用整型参数调用 `process()` 就可以按预期工作。用 `std::string` 调用它就会导致一个错误，编译器会报错不满足约束，报错内容具有极强的可读性。

另一种语法是使用 `requires` 表达子句，示例如下：

```cpp
template <typename T> requires constant_expression
void process(const T& t);
```

constant_expression可以是任何产生布尔类型的常量表达式。例如，常量表达式可以是一个概念表达式：

```cpp
template <typename T> requires Incrementable<T>
void process(const T& t);
```

或者一个预定义的标准概念：

```cpp
template <typename T> requires std::convertible_to<T, bool>
void process(const T& t);
```

或者一个 `requires` 表达式（注意两个 `requires` 关键字）：

```cpp
template <typename T> requires requires(T x) { x++; ++x; }
void process(const T& t);
```

或者任何产生布尔值的常量表达式：

```cpp
template <typename T> requires (sizeof(T) == 4)
void process(const T& t);
```

或者是且、或运算的组合：

```cpp
template <typename T> requires Incrementable<T> && Decrementable<T>
void process(const T& t);
```

或者是类型萃取：

```cpp
template <typename T> requires std::is_arithmetic_v<T>
void process(const T& t);
```

也可以在函数头之后指定 `requires` 子句，即后置 `requires` 子句：

```cpp
template <typename T>
void process(const T& t) requires Incrementable<T>;
```

使用类型约束的一种优雅的方式是将本章前面讨论过的简化函数模板的语法和类型约束结合起来，从而产生以下漂亮而因凑的语法。请注意，即使没有模板说明符，`process()` 仍然是一个函数模板：

```cpp
void process(const Incrementable auto& t);
```

> **注意**
> 
> 随着类型约束的引入，函数模板和类模板不受约束的模板参数应该成为过去。对于每个模板类型，都不可避免地需要满足实现中与该类型直接相关的某些约束。因此，应该对其施加类型约束，使编译器在编译时对它进行验证。

### 约束包含

可以使用不同的类型约束重载函数模板。编译器总是使用具有最具体约束的模板；更具体的约束包含/暗示更少的约束。下面是一个实例：

```cpp
template <typename T> requires std::integral<T>
void process(const T& t) { std::cout << "integral<T>" << std::endl; }

template <typename T> requires (std::integral<T> && sizeof(T) == 4)
void process(const T& t) { std::cout << "integral<T> && sizeof(T) == 4" << std::endl; }
```

假设对 `process()` 有以下调用：

```cpp
process(int { 1 });
process(short { 2 });
```

输出如下：

```
integral<T> && sizeof(T) == 4
integral<T>
```

编译器首先通过规范化约束表达式来解析任何包含。在约束表达式的规范化过程中，所有概念表达式都会被递归拓展它们的定义，知道结果是一个由常量布尔表达式的且/或运算组成的常量表达式。如果编译器可以证明一个规范化的约束表达式可以包含另一个约束表达式，那么它就包含另一个约束表达式。只考虑使用且和或来证明任何包含，不考虑非。

这种包含推理只在语法层面完成，而不是语义层面。例如，`sizeof(T)>4` 在语义上比 `sizeof(T)>=4` 更具体，但在语法上前者并不会包含后者。

但是，需要注意的是，类型萃取在规范化期间不会被拓展。因此，如果有一个预定义的概念和一个类型萃取可用，那么应该使用这个概念而不是这个萃取。例如，使用 `std::integral` 概念来代替 `std::is_integral` 类型萃取。

## 13.6 类型约束和类模板

类型约束也可以和类模板一起使用，并使用和函数模板类似的语法。

> 详细示例参考code/grid/game_board.ixx

## 13.7 类型约束和类方法

也可以对类模板的特定方法添加额外的约束。

> 详细示例参考code/grid/game_board.ixx

请注意，基于类模板的选择性实例化，仍然可以使用非移动类型的GameBoard类模板，只要不调用它的 `move()` 方法。

## 13.8 类型约束和模板特化

可以为类模板编写特化，为函数模板编写重载，从而为特定类型编写不同的实现。也可以为满足特定约束的类型的结合编写特化。

回顾一下[函数模板](ModernCpp/ModernCpp-函数模板-变量模板.md)中提到的 `Find()` 函数模板：

```cpp
template <typename T>
size_t Find(const T& value, const T* arr. size_t size) {
  for (size_t i { 0 }; i < size; ++i) {
    if (srr[i] == value) {
      return i;
    }
  }
  return NOT_FOUND;
}
```

该实现使用 `==` 运算符比较值。通常不建议使用 `==` 比较浮点类型是否相等，而是使用所谓的epsilon检测。下面针对浮点类型的 `Find()` 特化使用了在 `AreEqual()` 辅助函数中实现epsilon检测，而不是 `operator==`：

```cpp
template <std::floating_point T>
size_t Find(const T& value, const T* arr, size_t size) {
  for (size_t i { 0 }; i < size; ++i) {
    if (AreEqual(arr[i], value)) {
      return i;
    }
  }
  return NOT_FOUND;
}
```

`AreEqual()` 的定义如下，同样也使用类型约束。关于epsilon检测的数学推导这里不做讨论。

```cpp
template <std::floating_point T>
bool AreEqual(T x, T y, int precision = 2) {
  // scale the machine epsilon to the magnitude of the given values and
  // multiply by required precision.
  return fabs(x - y) <= std::numeric_limits<T>::epsilon() * fabs(x + y) * precision
    || fabs(x - y) < std::numeric_limits<T>::min();
}
```
