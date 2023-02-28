# Modern C++ 类模板

 > 类模板定义了一个类，其中，将一些变量的类型、方法和参数类型和方法的参数类型指定为参数。本节使用了一个Grid容器作为示例。不同小节会向Grid容器添加一些特性。源码见code/grid

- [Modern C++ 类模板](#modern-c-类模板)
  - [编译器处理模板的原理](#编译器处理模板的原理)
    - [1. 选择性实例化](#1-选择性实例化)
    - [2. 模板对类型的要求](#2-模板对类型的要求)
  - [将模板代码分布到多个文件中](#将模板代码分布到多个文件中)
    - [1. 将方法定义与模板定义放在相同模块接口文件中](#1-将方法定义与模板定义放在相同模块接口文件中)
    - [2. 将方法定义放在单独的文件中](#2-将方法定义放在单独的文件中)
  - [模板参数](#模板参数)
    - [1. 非类型的模板参数](#1-非类型的模板参数)
    - [2. 类型参数的默认值](#2-类型参数的默认值)
    - [3. 类模板实参推导（CTAD)](#3-类模板实参推导ctad)
      - [用户定义的推导规则](#用户定义的推导规则)
  - [方法模板](#方法模板)

## 编译器处理模板的原理

编译器遇到模板方法时，会进行语法检查，但是并不编译模板。编译器无法编译模板定义，因为不知道它要使用什么类型。如果不知道x和y的类型，那么编译器就无法为x=y这样的语句生成代码。

编译器遇到一个实例化的模板时，例如 `Grid<int>`，就会将模板类定义的每一个T替换为int，从而生成Grid模板的int版本代码。当编译器遇到这个模板的另一个示例时，就会生成另一个版本的Grid类。

### 1. 选择性实例化

类模板的隐式实例化如下所示：

```cpp
Grid<int> myIntGrid;
```

编译器总是为类模板的所有虚方法生成代码。但是对于非虚方法，编译器只会为那些实际为某种类型调用的非虚方法生成代码。例如，给定前面定义的Grid模板，假设在 `main()` 函数中编写这段代码：

```cpp
Grid<int> myIntGrid;
myIntGrid.at(0, 0) = 10;
```

编译器只会为int版本的Grid类生成无参构造函数、析构函数和非常量 `at()` 方法的代码。它不会为其他方法生成代码。编译器的以上行为称为 **选择性实例化(selective instantiation)**。

存在这样的危险：在类模板方法中存在编译错误，而这些错误会被忽略。类模板中未使用的方法可能包含语法错误，而这些错误不会被编译。通过显式的模板实例化，可以强制编译器为所有方法生成代码，包括虚方法和非虚方法。示例如下：

```cpp
template class Grid<int>;
```

当使用显式模板实例化时，不要只尝试使用基本数据类型来实例化类模板，而应该尝试使用更复杂的类型来实例化模板。

### 2. 模板对类型的要求

在编写与类型无关的代码时，必须对这些类型有一些假设。例如，在Grid类模板中，假设元素类型T时可以被析构的、可复制/移动的。

如果试图用一种不支持模板使用的所有操作的类型对模板进行实例化，那么这段代码无法编译。C++20引入 **概念(concepts)** ，允许编写编译器可以解释和验证的模板参数要求。如果传递给实例化模板的模板参数不满足这些要求，编译器会生成更多可读的错误。该部分内容会在之后讨论。

## 将模板代码分布到多个文件中

对于类模板，编译器必须可以从使用它们的任何源文件中获取类模板定义和方法定义。有几种机制可以实现这一点。

### 1. 将方法定义与模板定义放在相同模块接口文件中

可以将方法定义直接放在定义类模板的相同模块接口文件中。当使用模板的另一个源文件中导入此模块时，编译器能访问所需的所有代码。

### 2. 将方法定义放在单独的文件中

或者，可将类模板的方法定义在单独的模块接口分区文件中。然后，还需要将类模板的定义放在它自己的分区中。例如，Grid类模板的基本模块接口文件如下：

```cpp
export import grid;

export import :definition
export import :implementation
```

这里导入和导出两份模块分区：definition和implementation。类模板定义在definition分区中：

```cpp
export module grid:definition

import <vector>;
import <optional>;

export template <typename T> class Grid {...};
```

如果方法的实现在implementation分区中，那么implementation分区也需要导入definition分区，因为它需要Grid类模板定义：

```cpp
export module gird:implementation;

import :definition;
import <vector>;

export template <typename T> 
Grid<T>::Grid(size_t width, size_t height) : m_width { width }, m_height { height } {...};
```

## 模板参数

编写类模板时，需要在尖括号中指定参数列表，示例如下：

```cpp
template <typename T>
```

这个参数列表类似于函数或方法中的参数列表。与函数或方法一样，可使用任意多个模板参数来编写类。此外，这些参数可以不是类型，并且可以具有默认值。

### 1. 非类型的模板参数

非类型的模板参数是“普通”参数，如int和指针。然而，非类型的模板参数只能是整数类型（char、int、long等）、枚举类型、指针、引用、`std::null_ptr_t`、`auto`、`auto&`、`auto*`。此外，C++20起允许浮点类型（甚至类类型）的非类型模板参数，不过有许多限制，本处不做讨论。

在Grid类模板中，可以使用非类型模板参数指定网格的高度和宽度，而不是在构造函数中指定它们。在模板列表中指定非类型参数可以在编译期确定值。因此，在这个实现中，可以使用普通的二维数组而不是动态二维数组。下面是新的类定义：

```cpp
export template <typename T, size_t WIDTH, size_t HEIGHT>
class Grid {
 public:
   Grid() = default;
   virtual ~Grid() = default;

   //...

   size_t getHeight() const { return HEIGHT; }
   size_t getWidth() const { return WIDTH; }

 private:
   //...
   std::optional<T> m_cells[WIDTH][HEIGHT];
};
```

注意，模板参数列表需要3个参数：网格中保存的对象类型以及网格的宽度和高度。宽度和高度用于创建保存对象的二维数组。下面是类方法定义：

```cpp
export template <typename T, size_t WIDTH, size_t HEIGHT>
void Grid<T, WIDTH, HEIGHT>::verifyCoordinate(size_t x, size_t y) const {
  if(x >= WIDTH) {
    throw std::out_of_range{...};
  }
  if(y >= HEIGHT) {
    throw std:out_of_range{...};
  }
}

//...
```

注意之前所有指定 `Grid<T>` 的地方，现在都必须指定 `Grid<T, WIDTH, HEIGHT>` 来表示这三个模板参数。

可以通过以下方式实例化这个模板：

```cpp
Grid<int, 10, 10> myGrid;
Grid<int, 10, 10> anotherGrid;
myGrid.at(2, 3) = 42;
anotherGrid = myGrid;
std::cout << anotherGrid.at(2, 3).value_or(0) << std::endl;
```

这段代码看上去很棒。但遗憾的是，实际的限制比想象中的要多。首先，不能通过非常量的整数指定高度和宽度。下面的代码无法编译：

```cpp
size_t height { 10 };
Grid<int, 10, height> testGrid; // compile error
```

然而，如果把height声明为const，这段代码可以编译通过：

```cpp
const size_t height { 10 };
Grid<int, 10, height> testGrid; // compile success
```

带有正确返回类型的constexpr函数也可以编译。例如，如果有一个返回size_t的constexpr函数，那么可以使用它初始化height模板参数：

```cpp
constexpr size_t getHeight() { return 10; }
...
Grid<double, 2, getHeight()> myDoubleGrid;
```

另一个限制可能更明显。既然宽度和高度都是模板参数，那么它们也是每种网格类型的一部分。这意味着 `Grid<int, 10, 10>` 和 `Grid<int, 10, 11>` 是两种不同的类型。不能将一种类型的对象赋给另一种类型的对象，而且一种类型的变量不能传递给接收另一种类型的变量的函数或方法。

> **注意**
>
> 非类型模板参数是实例化的对象的类型规范中的一部分。

### 2. 类型参数的默认值

可能你需要为高度和宽度（它们是非类型模板参数）提供默认值。C+++允许使用类似函数参数默认值的语法向模板参数提供默认值。这里也可以给T类型参数提供默认值。下面是类定义：

```cpp
export template <typename T = int, size_t WIDTH = 10, size_t HEIGHT = 10>
class Grid {
  //...
}
```

不需要在方法定义的模板规范中指定T, WIDTH, HEIGHT的默认值。例如，下面是 `at()` 方法的实现：

```cpp
template <typename T, size_t WIDTH, size_t HEIGHT>
const std::optional<T>& Grid<T, WIDTH, HEIGHT>::at(size_t x, size_t y) const {
  verifyCoordinate(x, y);
  return m_cells[x][y];
}
```

现在，实例化Grid时，可以不指定模板参数，只指定元素类型，或者指定元素类型或宽度，或指定元素类型、宽度和高度。

```cpp
Grid<> myIntGrid;
Grid<int> myGrid;
Grid<int, 5> anotherGrid;
Grid<int, 5, 5> aFourthGrid;
```

注意，如果没有指定任何类模板参数，那么仍需指定一组空尖括号。以下代码无法编译：

```cpp
Grid myIntGrid;
```

模板参数列表中默认参数与函数一样，可以从右向左提供默认值。也可以不用遵循这个规定，但无法在指定参数类型时省略该默认值，此时默认参数没有任何意义。

### 3. 类模板实参推导（CTAD)

通过类模板实参推导，编译器可以自动从传递给类模板构造函数的实参推导出模板参数。

例如，标准库有一个 `std::pair` 类模板，一个pair存储两种不同类型的两个值，必须为其指定模板参数。示例如下：

```cpp
std::pair<int, double> pair1 { 1, 2.3 };
```

为避免写模板参数，可以使用一个辅助的函数模板 `std::make_pair()`。函数模板始终支持基于传递给函数模板的实参自动推导模板参数。因此，`std::make_pair()` 能根据传递给它的值自动推导模板类型参数。对于pair类模板，只需要编写以下代码：

```cpp
auto pair2 { std::make_pair(1, 2.3) };
```

使用类模板实参推导（CTAD），这样的辅助函数模板就不再需要了。现在，编译器可以根据传递给构造函数的实参自动推导模板类型参数。对于pair类模板，只需要编写以下代码：

```cpp
std::pair pair3 {1, 2.3 };  // type pair<int, double>
```

当然，推导的前提时类模板的所有模板参数要么有默认值，要么用作构造函数中的参数。

注意，CTAD需要一个初始化式才能工作。以下是非法的：

```cpp
std::pair pair4;
```

许多标准库类都支持CTAD，例如 `std::vector` 、`std::array` 等。

> **注意**
> `std::unique_ptr` 和 `std::shared_ptr` 会禁用类型推导。给它们的构造函数传递 `T*` ，可能推导出 `<T>` 或是 `<T[]>`，这是一个可能出错的危险选择。

#### 用户定义的推导规则

可以编写自己的推导原则来帮助编译器。它允许编写如何推导模板参数的规则。这会在 *模板高级* 中详细叙述，这里举一个例子来演示其功能。

假设具有以下SpreadsheetCell类模板：

```cpp
template <typename T> 
class SpreadsheetCell {
 public:
   SpreadsheetCell(const T& t) : m_content(t) {}
   const T& getContent() const { return m_content; }
 private:
   T m_content;
};
```

通过CTAD，可使用 `std::string` 类型创建SpreadsheetCell：

```cpp
std::string myString { "hello world" };
SpreadsheetCell cell { myString };
```

番薯，如果给SpreadsheetCell构造函数传递一个 `const char*`，那么会将类型T推导为 `const char*`，则是不需要的结果。可以创建以下用户定义的推导规则，在将 `const char*` 作为实参传递给构造函数时，将T推导为 `std::string`。

```cpp
SpreadsheetCell(const char* t) -> SpreadsheetCell<std::string>;
```

在与SpreadsheetCell类相同的命名空间中，在类定义之外定义该规则。

通用语法如下。 `explicit` 关键字是可选的，它的行为与构造函数的 `explicit` 相同。通常，这些推导规则也是模板。

```cpp
explicit TemplateName(param) -> DeducedTemplateName;
```

## 方法模板

C++允许模板化类中的单个方法。这些方法称为 **方法模板(method template)** ，它们可以在类模板中，也可以在非模板化的类中。在编写方法模板时，实际上实在为很多不同的类型编写很多不同版本的方法。在类模板中，方法模板对赋值运算符和拷贝构造函数非常有用。

> **警告**
>
> 不能用方法模板编写虚方法和析构函数

考虑最早只有一个模板参数的Grid模板。可实例化很多不同类型的网格，例如int网格和double网格。

```cpp
Grid<int> myIntGrid;
Grid<double> myDoubleGrid;
```

然而，`Grid<int>` 和 `Grid<double>` 的类型是不同的。如果编写的函数接收类型为 `Grid<double>` 的对象，就不能传入 `Grid<int>`。即使int网格中的元素可以赋值到double网格中。也不能将类型为 `Grid<int>` 的对象赋值给 `Grid<double>` 的对象，也不能构造不同类的对象。下面两行代码无法编译：

```cpp
myDoubleGrid = myIntGrid;                   // compile error
Grid<double> newDoubleGrid { myIntGrid };   // compile error
```

问题在于Grid模板的拷贝构造函数和赋值运算符如下所示：

```cpp
Grid(const Grid& src);
Grid& operator=(const Grid& src);
```

它们等同于：

```cpp
Grid(const Grid<T>& src);
Grid<T>& operator=(const Grid<T>& src);
```

