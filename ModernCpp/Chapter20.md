# Chapter20. Modern C++ 高级模板

- [Chapter20. Modern C++ 高级模板](#chapter20-modern-c-高级模板)
  - [20.1 深入了解模板参数](#201-深入了解模板参数)
    - [20.1.1 深入了解模板类型参数](#2011-深入了解模板类型参数)
    - [20.1.2 template template参数介绍](#2012-template-template参数介绍)
    - [20.1.3 深入了解非类型模板参数](#2013-深入了解非类型模板参数)
  - [20.2 类模板部分特化](#202-类模板部分特化)
  - [20.3 通过重载模拟函数部分特化](#203-通过重载模拟函数部分特化)
  - [20.4 模板递归](#204-模板递归)
    - [20.4.1 N维网格：初次尝试](#2041-n维网格初次尝试)
    - [20.4.2 真正的N维网格](#2042-真正的n维网格)
  - [20.5 可变参数模板](#205-可变参数模板)
    - [20.5.1 类型的安全的变长参数列表](#2051-类型的安全的变长参数列表)
    - [20.5.2 可变数目的混入类](#2052-可变数目的混入类)
    - [20.5.3 折叠表达式](#2053-折叠表达式)

## 20.1 深入了解模板参数

实际上有3中模板参数：类型参数、非类型参数和template template参数。下面深入探讨这三类模板参数。

### 20.1.1 深入了解模板类型参数

可声明任意数目的类型参数。例如，可以给Chapter11网格模板添加第二个参数类型，以表示这个网格构建于另一个模板化的类容器之上。原始的网格类使用vector的vector存储网格的元素，Grid类的用户可能想使用deque的vector。通过另一个模板的类型参数，可以让用户指定底层容器是vector还是deque。该实现使用容器的resize()方法和容器的value_type类型别名。因此使用concept强制给定的容器这些。下面是带有额外模板参数的concept和类定义：

```cpp
template <typename Container>
concept ContainerType = requires(Container c) {
  c.resize(1);
  typename Container::value_type;
};

export template <typename T, ContainerType Container>
class Grid {
public:
  explicit Grid(size_t width = DefaultWidth,
    size_t height = DefaultHeight);
  virtual ~Grid() = default;

  // explicit default a copy constructor and assignment operator
  Grid(const Grid& src) = default;
  Grid<T>& operator=(const Grid<T>& rhs) = default;

  // explicit default a move constructor and assignment operator
  Grid(Grid&& src) = default;
  Grid<T>& operator=(Grid<T>&& src) = default;

  typename Container::value_type& at(size_t x, size_t y);
  const typename Container::value_type& at(size_t x, size_t y) const;

  size_t getHeight() const { return m_height; }
  size_t getWidth() const { return m_width; }

  static const size_t DefaultWidth { 10 };
  static const size_t DefaultHeight { 10 };

private:
  void verifyCoordinate(size_t x, size_t y) const;

  std::vector<Container> m_cells;
  size_t m_width { 0 }, m_height { 0 };
};
```

现在这个模板有两个参数：T和Container。因此，所有引用了 `Grid<T>` 的地方现在都必须指定 `Grid<T, Container>` 以表示两个模板参数。注意m_cells现在是Container的vector。at()方法的返回类型是存储在给定容器类型中元素的类型。可通过 `typename Container::value_type` 来访问这个类型。

下面是at()方法的实现：

```cpp
template <typename T, typename Container>
const typename Container::value_type& 
Grid<T, Container>::at(size_t x, size_t y) const {
  verifyCoordinate(x, y); 	
  return m_cells[x][y];
}

template <typename T, typename Container>
typename Grid<T, Container>::value_type&
Grid<T, Container>::at(size_t x, size_t y) {
  return const_cast<typename Container::value_type&>(
    std::as_const(*this).at(x, y));
}
```

尝试使用double作为容器模板类型参数类实例化Grid类模板：

```cpp
Grid<int, double> test;   // compile error
```

此代码无法通过编译，编译器会指出double类型不满足模板类型参数相关concept的约束。

与函数参数一样，可以给模板参数指定默认值。例如，可能想表示Grid的默认容器是vector。这个模板类定义如下：

```cpp
export template <typename T,
  ContainerType Container = std::vector<std::optional<T>>>
class Grid {...};
```

stack, queue, priority_queue类模板都使用了Container模板类型参数，包含默认值，并指定底层容器。

### 20.1.2 template template参数介绍

上面讨论的Container参数还存在一个问题。当实例化类模板时，这样编写代码：

```cpp
Grid<int, vector<optional<int>>> myIntGrid;
```

请注意int类型的重复。必须在vector中同时为Grid和vector指定元素类型。如果编写了下面的代码，会怎么样？

```cpp
Grid<int, vector<optional<SpreadsheetCell>>> myIntGrid;
```

这不能很好的工作。如果能编写以下代码就好了：

```cpp
Grid<int, vector> myIntGrid;
```

Grid类应该能够判断除需要一个元素类型为int的optional vector。不过编译器不会允许传递这样的参数给普通的类型参数，因为vector本身并不是类型，而是模板。

如果想要接收模板作为模板的参数，那么就必须使用template template参数。指定template template参数，有点像在普通函数中指定函数指针参数。template template参数的完整规范包括该模板的参数。

例如，vector和deque等容器有一个模板参数列表，如下所示。E是元素类型。

```cpp
template <typename E, typename Allocator = std::allocator<E>>
class vector {...};
```

要把这样的容器传递为template template参数，只能复制并粘贴类模板的声明，用参数名(Container)替代类名(vector)，并把它用作另一个模板声明的template template参数，而不是简单的类型名。下面是接收一个容器模板作为第二个模板参数的Grid类的类模板定义：

```cpp
export template <typename T,
  template <typename E, typename Allocator = std::allocator<E>> class Container
    = std::vector>
class Grid {
public:
  std::optional<T>& at(size_t x, size_t y);
  const std::optional<T>& at(size_t x, size_t y) const;
private:
  void verifyCoordinate(size_t x, size_t y) const;
  
  std::vector<Container<std::optional<T>>> m_cells;
  size_t m_width { 0 }, m_height { 0 };
};
```

第一个模板参数与之前一样：元素类型T。第二个模板参数现在本身就是容器的模板，如vector或deque。如前所述，这种模板类型必须接收两个参数：元素类型E和分配器类型。注意嵌套模板参数列表后面重复的单词class。这个参数在Gird模板中的名称是Container。默认值现在是vector而不是vector\<T\>，因为Container是模板而是不是实际类型。

template template参数更通用的语法规则是：

```cpp
template <..., template <TemplateTypeParams> class ParameterName, ...>
```

> **注意**
>
> 从C++17起，也可以用typename关键字代替class，如下：
>
> ```cpp
> template <..., template <TemplateTypeParams> typename ParameterName, ...>;
> ```

在代码汇总不要使用Container本身，而必须把 `Container<std::optional<T>>` 指定为容器类型。例如，现在m_cells的声明如下：

```cpp
std::vector<Container<std::optional<T>>> m_cells;
```

不需要更改方法定义，但必须更改模板行，例如：

```cpp
export template <typename T,
  template <typename E, typename Allocator = std::allocator<E>> class Container>
void Grid<T, Container>::verifyCoordinate(size_t x, size_t y) const {...}
```

可以这样使用Grid模板：

```cpp
Grid<int, vector> myGrid;
myGird.at(1, 2) = 3;
cout << myGird.at(1, 2).value_or(0) << endl;
Grid<int, vector> myGrid2 { myGrid };
Grid<int, deque> myDequeGrid;
```

### 20.1.3 深入了解非类型模板参数

有时可能想让用户指定一个默认元素，用来初始化网格中的每一个单元格。下面是实现这个目标的一种完全合理的方法，它使用零初始化语法T{}，作为第二个模板参数的默认值：

```cpp
export template <typename T, const T DEFAULT = T{}>
class Grid {...}; 
```

非类型参数可以是const，就像函数参数一样。可使用T的初始化值来初始化网格中的每个单元格：

```cpp
template <typename T, const T DEFAULT>
Grid<T, DEFAULT>::GRid(size_t width, size_t height)
  : m_width { width }, m_height { height } {
  m_cells.resize(m_width);
  for (auto& column : m_cells) {
    column.resize(m_height);            
    for (auto& element : column) {
      element = DEFAULT;
    }
  }
}
```

其他的方法定义保持不变，只是必须向模板行添加第二个模板参数，所有的Grid\<T\>实例要变为Grid\<T, DEFAULT\>。完成这些修改后，可实例化一个int网格，并为所有元素这是初始值：

```cpp
Grid<int> myIntGrid;      // Initial value is 0
Grid<int, 10> myIntGrid2; // Initial value is 10
```

初始值可以是任何整数。但是，假设创建一个SpreadsheetCell网格：

```cpp
SpreadsheetCell defaultCell;
Grid<SpreadsheetCell, defaultCell> mySpreadsheet; // compile error
```

这会导致编译错误，因为不能向非类型参数传递对象作为参数。

这个例子展示了模板类的一种奇怪行为：可正常用于一种类型，但另一种类型却会编译失败。

允许用户指定网格初始值的一种更详尽方式是使用T引用作为非类型模板参数。下面是新的类定义：

```cpp
export template <typename T, const T& DEFAULT>
class Grid {...};
```

现在可以用任何类型实例化这个类模板。然而，作为第二个模板参数传递的引用必须具有链接。下面的示例使用内部链接的初始值声明了int和SpreadsheetCell网格：

```cpp
namespace {
  int defaultInt { 11 };
  SpreadsheetCell defaultCell { 1.2 };
}

int main() {
  Grid<int, defaultInt> myIntGrid;     
  Grid<SpreadsheetCell, defaultCell> mySpreadsheet;
}
```

## 20.2 类模板部分特化

Chapter11中的const char*类的特化被称为完全类模板特化，因为它对Gird模板中的每个模板参数进行的特化。在这个特化中没有剩下任何模板参数。这并不是特例化类的唯一方式；还可编写部分特化的类，这个类允许特化部分模板参数，而不处理其他参数。例如，基本版本的Gird模板带有宽度和高度的非类型参数：

```cpp
export template <typename T, size_t WIDTH, size_t HEIGHT> 
class Grid {...};
```

可采用这种方式为const char*字符串特化这个类模板：

```cpp
export template <size_t WIDTH, size_t HEIGHT>
class Grid<const char*, WIDTH, HEIGHT> {...};
```

注意，这个模板只有两个参数：WIDTH，HEIGHT。然而，这个Grid类带有3个参数：T, WIDTH, HEIGHT。因此，模板参数列表包含两个参数，而显式的 `Grid<const char*, WIDTH, HEIGHT>` 包含3个参数。实例化模板时仍须指定3个参数：

```cpp
Grid<int, 2, 2> myIntGrid;            // original Grid
Grid<const char*, 2, 2> myStringGrid; // partial specialization
// Grid<2, 3> test;                   // compile error!
```

上述的语法的确很乱。更糟糕的是，在部分特化中，与完全特化不同，在每个方法定义前都要包含模板代码行，如下所示：

```cpp
template <size_t WIDTH, size_t HEIGHT>
const std::optional<std::string>&
Grid<const char*, WIDTH, HEIGHT>::at(size_t x, size_t y) const {...}
```

需要这一带有两个参数的模板行，以表示这个方法针对这两个参数做了参数化处理。注意，需要表示完整类名时，都要使用 `Grid<const char*, WIDTH, HEIGHT>`。

可为可能的类型子集编写特化的实现，而不需要为每种类型特化。例如，可为所有的指针类型编写特化的Grid类。这种特化的拷贝构造函数和赋值运算符可对指针所指的对象执行深拷贝。

下面是类的定义，假设只用一个参数特化最早版本的Grid。在这个实现中，Grid称为所提供指针的拥有者，因此它在需要时自动释放内存。

```cpp
export template <typename T>
class Grid<T*> {
public:
  ...
  Grid(const Grid& src);
  Grid& operator=(const Grid& rhs);

  Grid(Grid&& src) = default;
  Grid& operator=(Grid&& rhs) = default;  
  
  void swap(Grid& other) noexcept;

  std::unique_ptr<T>& at(size_t x, size_t y);
  const std::unique_ptr<T>& ar(size_t x, size_t y) const;

  ...
private:
  ...
  std::vector<std::vector<std::unique_ptr<T>>> m_cells;
  size_t m_width { 0 }, m_height { 0 };  
};
```

下面这两行代码是关键所在：

```cpp
export template <typename T>
class Grid<T*>
```

上述语法表明这个类是Grid类模板对所有指针类型的特化。只有T是指针类型的情况下才提供实现。请注意，如果像下面这样实例化网格：`Grid<int*>mtIntGrid`，那么T实际上是int而非int*。

## 20.3 通过重载模拟函数部分特化

C++标准不允许函数的模板部分特化。但是可用另一个模板重载函数。假设要编写一个特化的Find()函数模板，这个特化对指针解引用，对指向的对象直接调用 `operator==`。实现此行为的正确方式是为Find()函数编写第二个函数模板：

```cpp
template <typename T>
size_t Find(T* value, T* const* arr, size_t size) {
  for (size_t i { 0 }; i < size; ++i) {
    if (*arr[i] == *value) {
      return i;
    }
  }
  return NOT_FOUND;
}
```

可在一个程序中定义原始的Find()模板、针对指针类型的重载Find()，针对const char*的重载Find()。编译期会根据推导规则选择合适的版本来调用。

> **注意**
>
> 在所有重载的版本之间，编译器总是选择最具体的函数版本。如果非模板化的版本与函数模板实例化等价，编译器更偏向非模板化的版本。

## 20.4 模板递归

模板递归类似于函数递归，意思是一个函数的定义是通过调用自身实现。这一节首先讲解模板递归的动机，然后讲述如何实现模板递归。

### 20.4.1 N维网格：初次尝试

前面的Grid模板示例到现在为止只支持两个维度，这限制了它的实用性。当然，可以为每个维度写一个模板类或非模板类。然而，这会重复很多代码。另一种方法是只编写一个一维网格，然后利用另一个网格作为元素类型实例化Grid，可创建任意维度的网格。这种Grid元素类型本身可以用网格作为元素类型进行实例化。下面是OneDGrid类模板的实现。这只是前面例子中Grid模板的一维版本，添加了resize()方法，并用 `operator[]` 替换at()。与诸如vector的标准库容器类似，`operator[]` 实现不执行边界检查。另外，在这个示例中，m_elements存储T的实例而非 `std::optional<T>` 的实例。

```cpp
export template <typename T>
class OneDGrid {
public: 
  explicit OneDGrid(size_t size = DefaultSize) { resize(size); }
  virtual ~OneDGrid() = default;

  T& operator[](size_t x) { return m_elements[x]; }
  const T& operator[](size_t x) const { return m_elements[x]; }

  void resize(size_t newSize) { m_elements.resize(newSize); }
  size_t getSize() const { return m_elements.size(); }

  static const size_t DefaultSize { 10 };
private:
  std::vector<T> m_elements;
};
```

有了OneDGrid的这个实现，就可通过如下方式创建多维网格：

```cpp
OneDGrid<int> singleDGrid;
OneDGrid<OneDGrid<int>> twoDGrid;
OneDGrid<OneDGrid<OneDGrid<int>>> threeDGrid;
singleDGrid[3] = 5;
twoDGrid[3][3] = 5;
threeDGrid[3][3][3] = 5;
```

此代码可以正常执行，但声明看上去有点乱。下面对其加以改进。

### 20.4.2 真正的N维网格

可使用模板递归编写真正的N维网格，因为网格的维度在本质上是递归的。从如下声明中可以看出：

```cpp
OneDGrid<OneDGrid<OneDGrid<int>>> threeDGrid;
```

可将嵌套的每层OneDGrid想象成一个递归步骤，int的OneDGrid是递归的基本情形。三维网格是int一维网格的一维网格的一维网格。用户不需要自己进行递归，可以编写一个类模板来自动进行递归。然后，可创建如下N维网格：

```cpp
NDGrid<int, 1> singleDGrid;
NDGrid<int, 2> twoDGrid;
NDGrid<int, 3> threeDGrid;
```

NDGrid类模板需要元素类型和表示维度的整数作为参数。这里的关键问题在于，NDGrid的元素类型不是模板参数表中指定的元素类型，而是上一层递归的维度中指定的另一个NDGrid。换句话说，三维网格是二维网格的矢量，二维网格是一维网格的矢量。

使用递归时，需要处理基本情形。可编写维度为1的部分特化的NDGrid，其中元素类型不是另一个NDGrid，而是模板参数指定的元素类型。

下面是NDGrid模板定义和实现。模板递归实现最棘手的部分不是模板递归本身，而是网格中每个维度的正确大小。这个实现创建了N维网格，每个维度都一样大。维每个维度指定不同的大小要困难得多。然而，即使做了这样的简化，也仍然存在一个问题：用户应该有能力创建指定大小的数组。因此，构造函数接收一个整数作为大小参数。然而，当动态重设子网格的vector时，不能将这个大小参数传递给子网格元素，因为vector使用默认的构造函数创建对象。因此，必须对vector的每个网格元素显式调用resize()。注意m_elements是 `NDGrid<T, N-1>` 的vector，这是递归步骤。此外，`operator[]` 返回类型的引用，同样是 `NDGrid<T, N-1>` ，而不是T。

```cpp
export template <typename T, size_t N>
class NDGrid {
public:
  explicit NDGrid(size_t size = DefaultSize) { resize(size); }
  virtual ~NDGrid() = default;
  
  NDGrid<T, N-1>& operator[](size_t x) { return m_elements[x]; }  
  const NDGrid<T, N-1>& operator[](size_t x) const { return m_elements[x]; }
  void resize(size_t newSize) {
    m_elements.resize(newSize);
    for (auto& element : m_elements) {
      element.resize(newSize);
    }
  }

  size_t getSize() const { return m_elements.size(); }

  static const size_t DefaultSize { 10 };
private:
  std::vector<NDGrid<T, N-1>> m_elements;
};
```
  
基本用例的模板定义是维度为1的部分特化。请注意，必须重写很多代码，因为不能在特化中继承任何实现。

```cpp
export template <typename T>
class NDGrid<T, 1> {
  explicit NDGrid(size_t size = DefaultSize) { resize(size); }
  virtual ~NDGrid() = default;
  
  T& operator[](size_t x) { return m_elements[x]; }  
  const T& operator[](size_t x) const { return m_elements[x]; }
  void resize(size_t newSize) { m_elements.resize(newSize); }

  size_t getSize() const { return m_elements.size(); }

  static const size_t DefaultSize { 10 };
private:
  std::vector<T> m_elements;
};
```

至此，递归结束；元素类型是T，而不是另外的模板实例化。

现在，可编写下面的代码：

```cpp
Grid<int, 3> my3DGird { 4 };
my3DGrid[2][1][2] = 5;
my3DGrid[1][1][1] = 5;
cout << my3DGrid[2][1][2] << endl;
```

## 20.5 可变参数模板

可变参数模板(variadic template)可接收可变数目的模板参数。例如，下面的代码定义了一个模板，它可以接收任何数目的模板参数，使用称为Types的参数包(parameter pack)：

```cpp
template <typename... Types>
class MyVariadicTemplate {};
```

> **注意**
>
> typename之后的3个点并非错误。这是为可变参数模板定义参数包的语法。参数包可以接收可变数目的参数。在3个点前后允许添加空格。

可用任意数量的类型实例化MyVariadicTemplate。例如：

```cpp
MyVariadicTemplate<int> instance1;
MyVariadicTemplate<string, double, list<int>> instance2;
```

甚至可用零个模板参数实例化MyVariadicTemplate。

```cpp
MyVariadicTemplate<> instance3;
```

为阻止用零个模板参数实例化可变参数模板，可像下面这样编写模板：

```cpp
template <typename T1, typename... Types>
class MyVariadicTemplate {}; 
```

有了这个定义后，试图通过零个模板参数实例化MyVariadicTemplate会导致编译错误。

不能直接遍历传给可变参数模板的不同参数。唯一的方法是借助模板递归或折叠表达式的帮助。下面将展示这两种方法的实例。

### 20.5.1 类型的安全的变长参数列表

### 20.5.2 可变数目的混入类

### 20.5.3 折叠表达式
