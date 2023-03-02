# Modern C++ 概念和约束

> C++20引入了 **概念（concepts)**，命名需求来约束类模板和函数模板的模板类型和非类型参数。概念的主要目标是使与模板相关的编译器错误更具有可读性。
>
> 概念允许编译器在不满足某些类型约束时输出可读的错误消息。因此，为了得到有意义的语义错误，建议编写代码时使用概念来建模语义需求。避免只验证没有任何语义意义的语法方面的概念。例如只检查类型是否支持 `operator+`，这样的概念将只检查语法，而不是语义。 `std::string` 支持 `operator+`，但它与整数的 `operator+` 具有完全不同的含义。另一方面，诸如可排序和可交换的概念是使用概念对某些语义进行建模的好例子。

- [Modern C++ 概念和约束](#modern-c-概念和约束)
  - [语法](#语法)
  - [约束表达式](#约束表达式)
    - [`requires` 表达式](#requires-表达式)
      - [简单requirement](#简单requirement)
      - [类型requirement](#类型requirement)
      - [复合requirement](#复合requirement)
      - [嵌套requirement](#嵌套requirement)
    - [组合概念表达式](#组合概念表达式)
  - [预定义的标准概念](#预定义的标准概念)

## 语法

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

## 约束表达式

计算结果为布尔值的常量表达式可以直接用作概念定义的约束。但他的结果必须精确计算为一个布尔值，并且没有任何类型转换。下面是一个示例：

```cpp
template <typename T>
concept C = sizeof(T) == 4;
```

随着概念的引入，还引入了一种新的常量表达式，称为 `require` 表达式，接下来对此进行解释

### `requires` 表达式

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

### 组合概念表达式

现有的概念表达式可以使用 `&&` 和 `||` 逻辑运算符进行组合。例如，假设有一个递减的概念Decrementable。下面演示了一个概念，它要求一个类型既可以是自增的，也可以是自减的：

```cpp
template <typename T>
concept IncrementableAndDecrementable = Incrementable<T> && Decrementable<T>;
```

## 预定义的标准概念

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

当然，这些标准概念也可以组合成更具体地概念。例如，下面的概念要求类型T既是默认的也是可复制的：

```cpp
template <typename T>
concept DefaultAndCopyConstructible = 
 std::default_initializable<T> && std::copy_constructible<T>;
```

> **注意**
>
> 编写完整且正确地概念并不总是那么容易地。如果可能，尝试使用可用地标准概念或它们地组合来约束类型。