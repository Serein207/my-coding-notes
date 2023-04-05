# Chapter11. Modern C++ 类模板

 > 类模板定义了一个类，其中，将一些变量的类型、方法和参数类型和方法的参数类型指定为参数。本节使用了一个Grid容器作为示例。不同小节会向Grid容器添加一些特性。源码见code/grid

- [Chapter11. Modern C++ 类模板](#chapter11-modern-c-类模板)
  - [11.1 编译器处理模板的原理](#111-编译器处理模板的原理)
    - [11.1.1 选择性实例化](#1111-选择性实例化)
    - [11.1.2  模板对类型的要求](#1112--模板对类型的要求)
  - [11.2 将模板代码分布到多个文件中](#112-将模板代码分布到多个文件中)
    - [11.2.1 将方法定义与模板定义放在相同模块接口文件中](#1121-将方法定义与模板定义放在相同模块接口文件中)
    - [11.2.2 将方法定义放在单独的文件中](#1122-将方法定义放在单独的文件中)
  - [11.3 模板参数](#113-模板参数)
    - [11.3.1 非类型的模板参数](#1131-非类型的模板参数)
    - [11.3.2 类型参数的默认值](#1132-类型参数的默认值)
    - [11.3.3 类模板实参推导(CTAD)](#1133-类模板实参推导ctad)
      - [用户定义的推导规则](#用户定义的推导规则)
  - [11.4 方法模板](#114-方法模板)
    - [带有非类型参数的方法模板](#带有非类型参数的方法模板)
  - [11.5 类模板特化](#115-类模板特化)
  - [11.6 从类模板派生](#116-从类模板派生)
  - [11.7 继承还是特化](#117-继承还是特化)
  - [11.8 模板别名](#118-模板别名)

## 11.1 编译器处理模板的原理

编译器遇到模板方法时，会进行语法检查，但是并不编译模板。编译器无法编译模板定义，因为不知道它要使用什么类型。如果不知道x和y的类型，那么编译器就无法为x=y这样的语句生成代码。

编译器遇到一个实例化的模板时，例如 `Grid<int>`，就会将模板类定义的每一个T替换为int，从而生成Grid模板的int版本代码。当编译器遇到这个模板的另一个示例时，就会生成另一个版本的Grid类。

### 11.1.1 选择性实例化

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

### 11.1.2  模板对类型的要求

在编写与类型无关的代码时，必须对这些类型有一些假设。例如，在Grid类模板中，假设元素类型T时可以被析构的、可复制/移动的。

如果试图用一种不支持模板使用的所有操作的类型对模板进行实例化，那么这段代码无法编译。C++20引入 **概念(concepts)** ，允许编写编译器可以解释和验证的模板参数要求。如果传递给实例化模板的模板参数不满足这些要求，编译器会生成更多可读的错误。该部分内容会在之后讨论。

## 11.2 将模板代码分布到多个文件中

对于类模板，编译器必须可以从使用它们的任何源文件中获取类模板定义和方法定义。有几种机制可以实现这一点。

### 11.2.1 将方法定义与模板定义放在相同模块接口文件中

可以将方法定义直接放在定义类模板的相同模块接口文件中。当使用模板的另一个源文件中导入此模块时，编译器能访问所需的所有代码。

### 11.2.2 将方法定义放在单独的文件中

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

## 11.3 模板参数

编写类模板时，需要在尖括号中指定参数列表，示例如下：

```cpp
template <typename T>
```

这个参数列表类似于函数或方法中的参数列表。与函数或方法一样，可使用任意多个模板参数来编写类。此外，这些参数可以不是类型，并且可以具有默认值。

### 11.3.1 非类型的模板参数

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

### 11.3.2 类型参数的默认值

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

### 11.3.3 类模板实参推导(CTAD)

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

## 11.4 方法模板

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

Grid拷贝构造函数和 `operator=` 都不会接收 `Grid<int>` 作为参数。

幸运的是，在Grid类中添加模板化的拷贝构造函数和赋值运算符，可生成一种将网格类型转换成另一种网格类型的方法，从而修复这个bug。下面是新的Grid类定义：

```cpp
export template<typename T>
class Grid {
public:
  template <typename E>
  Grid(const Grid<E>& src);

  template <typename E>
  Grid<T>& operator=(const Grid<E>& rhs);

  //...
};
```

> 不能删除原始拷贝构造函数和拷贝赋值运算符。如果E等于T，那么编译器将不会调用这些新的模板化拷贝构造函数和模板化拷贝复制运算符。

首先检查新的模板化的复制构造函数：

```cpp
template <typename E>
Grid(const Grid<E>& src);
```

看到另一个具有不同类型名称E的模板声明。这个类在类型T上被模板化，这个新的拷贝构造函数又在另一个不同的类型E上被模板化。通过这种双重模板化可将一种类型的网格复制到另一种类型的网格。下面是新的拷贝构造函数的定义：

```cpp
template <typename T>
template <typename E>
Grid<T>::Grid(const Grid<E>& src)
  :Grid { src.getWidth(),src.getHeight()} {
  for (size_t i { 0 }; m_width; i++) {
    for (size_t j { 0 }; j < m_height; j++) {
      m_cells[i][j] = src.at(i, j);
    }
  }
}
```

可以看出，必须将声明类模板（T参数）的那一行放在成员模板（E参数）的那一行声明之前。不能像下面这样合并两者：

```cpp
template <typename T, typename E>
Grid<T>::Grid(const Grid<E>& src) {
  ...
}
```

注意必须通过公共访问方法 `getWidth()`、`getHeight()`、`at()` 访问src的元素。这是因为复制目标对象与复制来源对象不是同一类型，因此必须使用公共方法。

`swap()` 方法的实现如下所示：

```cpp
template <typename T>
void Grid<T>::swap(Grid& other) noexcept {
  std::swap(m_width, other.m_width);
  std::swap(m_height, other.m_height);
  std::swap(m_cells, other.m_cells);
}
```

模板化的赋值运算符接收 `const Grid<E>&` 作为参数，但返回 `Grid<T>&`：

```cpp
template <typename T>
template <typename E>
Grid<T>& Grid<T>::operator=(const Grid<E>& rhs) {
  Grid<T> temp { rhs };
  swap(temp);
  return *this;
}
```

`swap()` 方法只能交换同类网格，但这是可行的。因为模板化的赋值运算符首先使用模板化的拷贝构造函数，将给定的 `Grid<E>` 转换为 `Grid<T>`（temp），然后使用 `swap()` 方法将 `temp` 变为 `*this`。


### 带有非类型参数的方法模板

在之前用于HEIGHT和WIDTH整数模板参数的例子中，一个主要的问题是告诉和宽度称为类型的一部分。因为存在这个限制，所以不能将某个高度和宽度的网格赋值给另一个高度和宽度的网格。如果源数组在任何一个维度上都比目标数组小，那么可以用默认值填充目标数组。有了赋值运算符和拷贝构造函数的方法模板后，完全可以实现这个操作，从而允许对不同大小的网格进行赋值和复制。下面是类定义：

```cpp
export template <typename T, size_t WIDTH = 10, size_t HEIGHT = 10>
class Grid {
public:
  Grid() = default;
  virtual ~Grid() = default;

  // explicit default a copy constructor and assignment operator
  Grid(const Grid& src) = default;
  Grid<T>& operator=(const Grid<T>& rhs) = default;

  template <typename E, size_t WIDTH2, size_t HEIGHT2>
  Grid(const Grid<E, WIDTH2, HEIGHT2>& src);

  template <typename E, size_t WIDTH2, size_t HEIGHT2>
  Grid<T>& operator=(const Grid<E, WIDTH2, HEIGHT2>& rhs);

  void swap(Grid& other) noexcept;
  
  //...
};
```

这个新定义包含拷贝构造函数符赋值运算符的方法模板，还包含辅助方法 `swap()`。注意，将非模板化的拷贝构造函数和赋值运算符显式设置为默认，语义和大小一样的网格完全一致。

下面是模板化的拷贝构造函数：

```cpp
template <typename T, size_t WIDTH, size_t HEIGHT>
template <typename E, size_t WIDTH2, size_t HEIGHT2>
Grid(const Grid<E, WIDTH2, HEIGHT2>& src) {
  for (size_t i { 0 }; m_width; i++) {
    for (size_t j { 0 }; j < m_height; j++) {
      if(i < WIDTH2 && j < HEIGHT2) {
        m_cells[i][j] = src.at(i, j);
      } else {
        m_cells[i][j].reset();
      }
    }
  }
}
```

注意，该拷贝构造函数只从src在x维度和y维度上分别复制WIDTH和HEIGHT个元素，即使src比WIDTH和HEIGHT大。如果sec在任何一个维度上都比这个指定值小，那么可以使用 `reset()` 方法重置多余的 `std::optional` 对象。

下面是 `swap()` 和 `operator=` 的实现：

```cpp
template <typename T, size_t WIDTH, size_t HEIGHT>
void Grid<T, WIDTH, HEIGHT>::swap(Grid& other) noexcept {
  std::swap(m_cells, other.m_cells);
}

template <typename T, size_t WIDTH, size_t HEIGHT>
template <typename E, size_t WIDTH2, size_t HEIGHT2>
Grid<T>& Grid<T, WIDTH, HEIGHT>::operator=(
        const Grid<E, WIDTH2, HEIGHT2>& rhs) {
  Grid<T, WIDTH, HEIGHT> temp {rhs};
  swap(temp);
  return *this;
}
```

## 11.5 类模板特化

对于特定类型，可以给类模板提供不同的实现。模板的另一个实现称为 **模板特化(template specialization)** 。
编写一个类模板特化时，必须指明这是一个模板，以及正在为哪种类型编写这个模板。下面是为 `const char*` 特化的Grid的语法。对于这个实现，原始的Grid类模板被移动到一个名为main的模块接口分区中，并且在一个名为string的模块接口分区中。

```cpp
export module grid:string;

import :main;

export template <>
class Grid<const char*> {
public:
  explicit Grid(size_t width = DefaultWidth,
    size_t height = DefaultHeight);
  virtual ~Grid() = default;

  // explicit default a copy constructor and assignment operator
  Grid(const Grid& src) = default;
  Grid& operator=(const Grid& rhs) = default;

  // explicit default a move constructor and assignment operator
  Grid(Grid&& src) = default;
  Grid& operator=(Grid&& src) = default;

  std::optional<std::string>& at(size_t x, size_t y);
  const std::optional<std::string>& at(size_t x, size_t y) const;

  size_t getHeight() const { return m_height; }
  size_t getWidth() const { return m_width; }

  static const size_t DefaultWidth { 10 };
  static const size_t DefaultHeight { 10 };

  void swap(Grid& other) noexcept;

private:
  void verifyCoordinate(size_t x, size_t y) const;

  std::vector<std::vector<std::optional<std::string>>> m_cells;
  size_t m_width { 0 }, m_height { 0 };
};
```

注意，在这个特化中不要指定任何类型变量T，而是直接处理 `const char*` 和 `std::string`。

```cpp
template <>
class Grid<const char*>
```

上述语法告诉编译器，这个类是Grid类的 `const char*` 的特化版本。假设没有提供这种语法，那么尝试编写下面的代码：

```cpp
class Grid
```

编译器会报错，因为已经有一个名为Grid的类（原始的类模板）。只能通过特化重用这个名字。特化的好处是可以对用户隐藏。当用户创建 `const char*` 类型的Grid时，编译器会使用 `const char*` 特化版本。这些全都由编译器自动完成。

主模块接口文件导入和导出两个模块的接口分区：

```cpp
export module grid;

export import :main;
export import :string;
```

特化一个模板时，并没有“继承”任何代码：特化和派生不同。必须重写类的实现。不要求提供具有相同名称或行为的方法。事实上，可以编写一个和原来的类无关的、完全不同的类。当然，没有正当理由，不应该这么做。下面是 `const char*` 特化版本的方法的实现。与模板定义不同，不必在每个方法定义之前重复 `template<>` 语法：

```cpp
Grid<const char*>::Grid(size_t width, size_t height) 
  : m_width { width }, m_height { height } {
  m_cells.resize(m_width);
  for (auto& column : m_cells) {
    column.resize(m_height);
  }
}

void Grid<const char*>::verifyCoordinate(size_t x, size_t y) const {
  if (x >= m_width) {
    throw std::out_of_range {
      std::format("{} must be less than {}.", x, m_width) };
  }
  if (y >= m_height) {
    throw std::out_of_range {
      std::format("{} must be less than {}.", y, m_height) };
  }
}

const std::optional<std::string>& Grid<const char*>::at(
  size_t x, size_t y) const {
  verifyCoordinate(x, y);
  return m_cells[x][y];
}

std::optional<std::string>& Grid<const char*>::at(size_t x, size_t y) {
  return const_cast<std::optional<std::string>&>(
    std::as_const(*this).at(x, y));
}
```

> *模板高级* 中将继续讨论部分特化等高级特性。

## 11.6 从类模板派生

可以从类模板派生。如果一个派生类从模板本身继承，那么这个类也必须是模板。此外，还可以从类模板派生某个特定实例，在这种情况下，派生类不需要是模板。

假设Grid类没有提供足够的棋盘功能，要给棋盘添加 `move()` 方法，允许棋盘上的棋子从一个位置移动到另一个位置。下面是这个GameBoard模板类的定义：

```cpp
import grid;

export template <typename T>
class GameBoard :public Grid<T> {
public:
  explicit GameBoard(size_t width = Grid<T>::DefaultWidth,
    size_t height = Grid<T>::DefaultHeight);
  void move(size_t xSrc, size_t ySrc, size_t xDest, size_t yDest);
};
```

这个GameBoard模板派生自Grid模板，因此继承了Grid的所有功能。不需要重写基类方法，也不需要添加拷贝构造、`operator=` 或析构函数，因为在GameBoard中没有任何动态分配的内存。

继承的语法和普通继承一样，区别在于基类是 `Grid<T>` ，而不是 `Grid`。`:public Grid<T>` 语法表明，这个类继承了Grid实例化对类型参数T有意义的所有内容。C++的名称查找规则要求使用 `this` 指针或 `Grid<T>::` 表示出基类模板中的数据成员和方法。

## 11.7 继承还是特化

|              | 继承 | 特化 |
| ------------ | ---- | ---- |
| 是否重用代码 | 是   | 否   |
| 是否重用名称 | 否   | 是   |
| 是否支持多态 | 是   | 否   |

> 通过继承拓展实现和使用多态。通过特化自定义特定的类型实现。

## 11.8 模板别名

通过 `typedef` 可以给特定类型赋予另一个名称。例如可以编写以下类型别名，给类型int指定另一个名称：

```cpp
using MyInt = int;
```

类似地，可使用类型别名给模板化的类赋予另一个名称。假定有如下类模板：

```cpp
template <typename T1, typename T2>
class MyTemplateClass {...};
```

可以定义如下类型别名，给定两个类模板的类型参数：

```cpp
using OtherName = MyTemplateClass<int, double>;
```

也可以用 `typedef` 替代类型别名。

还可以指定一些类型，其他类型则保持为模板类型参数，这称为 **别名模板(alias template)**。例如：

```cpp
template <typename T1>
using otherName = MyTemplateClass<T1, int>;
```

这无法用 `typedef` 完成。