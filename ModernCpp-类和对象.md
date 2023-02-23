# Modern C++ 类和对象

## 构造函数

### 显式默认的默认构造函数

在C++11前，如果类需要一些接收参数的显式构造函数，还需要一个说明都不做的默认构造函数，就必须显式地编写空的默认构造函数，如前所述。

为了避免手动编写空的默认构造函数，C++现在支持显式默认的默认构造函数(explicitly defaulted default constructor)。可按如下方法编写类的定义，而不需要在实现中定义默认构造函数：

```cpp
export class SpreadsheetCell {
 public:
   SpreadsheetCell() = default;
   SpreadsheetCell(double initialValue);
   SpreadsheetCell(std::string_view initialValue);
   // Remainder of the class definition omitted for brevity
};
```

SpreadsheetCell定义了两个定制的构造函数。然而，由于使用了default关键字，编译器仍然会生成一个标准的由编译器生成的默认构造函数。

注意，既可以将 `=default` 直接放在类定义中，也可以放在实现文件中。在同一文件中，已声明的函数不能再被定义成默认函数。

### 显式弃置的默认构造函数

C++还支持显式弃置的默认构造函数(explicitly deleted default constructor)。例如，可定义一个只有静态方法的类，这个类没有任何构造函数，也不想让编译器生成默认构造函数。在此情况下可以显式弃置默认构造函数：

```cpp
export class MyClass {
 public:
   MyClass() = delete;
};
```

> **注意**
>
> 使用 `=delete` 弃置的函数属于显式弃置，会参与重载决议，弃置函数被使用后程序非良构。
>
> 基类中被弃置的函数（包括构造和析构函数）在派生类中被隐式弃置，不会参与重载决议。

### 显示默认或显示弃置的拷贝构造函数

正如可以将编译器生成的默认构造函数设置为显式默认或显式弃置，同样可以将编译器生成的拷贝构造函数设置为默认或将其弃置：

```cpp
SpreadsheetCell(const SpreadsheetCell& src) = default;
```

或者

```cpp
SpreadsheetCell(const SpreadsheetCell& src) = delete;
```

通过弃置拷贝构造函数，对象将无法复制，这可用于禁止按值传递对象。

### 转换构造函数

在C++11以前，转换构造函数只能有一个参数。自C++11以来，由于支持列表初始化，转换构造函数可以有多个参数。让我们看一个例子：

```cpp
class MyClass {
 public:
   MyClass(int) {}
   MyClass(int, int) {}
};
```

这个类有两个构造函数，从C++11开始，他们都是转换构造函数。以下示例展示了编译器使用这些转换构造函数自动将给定参数`1,{1},{1, 2}`转换为MyClass的实例：

```cpp
void process(const MyClass& c) {}

int main() {
  process(1);
  process({1});
  process({1, 2});
}
```

为了避免编译器执行此类隐式转换，两个转换构造函数都可以标记为 `explicit`：

```cpp
class MyClass {
 public:
   explicit MyClass(int) {}
   explicit MyClass(int, int) {}
};
```

有了这个变更，就必须显式地执行这些转换，例如：

```cpp
process(MyClass{1});
process(MyClass{1, 2});
```

从C++20起，可以将布尔参数传递给 `explicit`，已将其转换为条件explicit。语法如下：

```cpp
explicit(true) MyClass(int);
```

当然，仅仅编写 `explicit(true)` 就等价于 `explicit`，但在它使用所谓类型萃取的泛型模板代码中更加有用。使用类型萃取，可以查询给定类型的某些属性，例如某个类型是否可以转换为另一个类型。类型萃取的结果可用作 `explicit()` 的参数。类型萃取允许编写高级泛型代码。

## 拷贝和赋值

> 本部分代码为截取代码，源码见 code/spreadsheet

### 异常安全的赋值运算

下面包含赋值运算符的Spreadsheet类定义：

```cpp
export class Spreadsheet{
 public:
   Spreadsheet& operator=(const Spreadsheet&rhs);
   // Code omitted for brevity
};
```

下面是一个不成熟的实现：

```cpp
Spreadsheet& Spreadsheet::operator=(const Spreadsheet& rhs) {
  // Check for self-assignment
  if(this == &rhs)  return *this;

  // Free the old memory
  for(size_t i { 0 }; i < m_width; i++) {
    delete[] m_cells[i];
  }
  delete[] m_cells;
  m_cells = nullptr;

  // Allocate new memory
  m_width = rhs.m_width;
  m_height = rhs.m_height;
  
  m_cells = new SpreadsheetCell*[m_width];
  for(size_t i { 0 }; i < m_width; i++) {
    m_cells[i] = new SpreadsheetCell[m_height];
  }

  // Copy the data
  for(size_t i { 0 }; i < m_width; i++) {
    for(size_t j { 0 }; j < m_height; j++) {
      m_cells[i][j] = rhs.m_cells[i][j];
    }
  }

  return *this;
}
```

代码首先检查自我复制，然后释放this对象当前内存，然后重新分配内存，最后复制各个元素。然而，这个代码不是异常安全的，程序可能发生内存泄漏。

我们需要一种全有或全无的机制：要么全部成功，要么该对象保持不变。为了实现这样一个异常安全的赋值运算，要使用“复制和交换”惯用方法。下面式包含 `operator=` `swap()` 方法以及非成员函数Spreadsheet类的定义：

```cpp
export class Spreadsheet {
 public:
   Spreadsheet& operator=(const Spreadsheet& rhs);
   void swap(Spreadsheet& other) noexcept;
   // ...
};
export void swap(Spreadsheet& first, Spreadsheet& second) noexcept;
```

要实现异常安全的“复制和交换”惯用方法，要求 `swap()` 函数永不抛异常，因此将其标记为 `noexcept`。

交换每个数据成员可以使用标准库中提供的 `<utility>` 中的 `std::swap()` 工具函数，它可以高效的交换两个值。

```cpp
void Spreadsheet::swap(Spreadsheet& other) noexcept {
  std::swap(m_height, other.m_height);
  std::swap(m_width, other.m_width);
  std::swap(m_cells, other.m_cells);
}
```
非成员的 `swap()` 只是简单地调用了 `swap()` 方法：

```cpp
void swap(Spreadsheet& first, Spreadsheet& second) noexcept {
  first.swap(second);
}
```

现在就有了异常安全的 `swap()` 函数，它可以用来实现赋值运算符：

```cpp
Spreadsheet& Spreadsheet::operator=(const Spreadsheet& rhs) {
  Spreadsheet temp { rhs };
  swap(temp);
  return *this;
}
```

首先，先创建一份右边的副本，名为temp。然后用当前对象与这个副本交换。这个模式式实现赋值运算符的推荐方法，因为它保证强大的异常安全性。

如果不使用“复制和交换”惯用方法实现赋值运算符，那么为了提高效率，有时也为了确保正确性，赋值运算符中第一行代码通常会检查自我赋值。例如：

```cpp
Spreadsheet& Spreadsheet::operator=(const Spreadsheet& rhs) {
  if(this == rhs) return *this;
  // ...
  return *this;
}
```

使用“复制和交换”惯用方法的情况下，就不再需要自我赋值检查了。

### 移动语义

对象的移动语义(move semantics)需要实现移动构造函数(move constructor)和移动赋值运算符(move assignment operators)。如果源对象是操作结束后会被临时销毁的对象，或是显式使用 `std::move()` 时，编译器就会使用这两个方法。移动将内存和其他资源的所有权从一个对象移动到另一个对象。这两个方法基本上只对成员变量进行浅拷贝，然后转换已分配内存和其他资源的所有权，从而阻止悬空指针和内存泄漏。

移动构造函数和移动赋值运算符将数据成员从源对象移动到新对象，然后使源对象处于有效但不确定的状态。通常，源对象的数据成员被置空，但这不是必须的。不过，不要使用任何已经移动的对象，这会触发未定义行为。 `std::unique_ptr` 和 `std::shared_ptr` 是例外情况，这些智能指针在移动时必须将其内部指针重置为 `nullptr`，这使得移动后可以安全地重用这些智能指针。

### 实现移动语义

为了对类增加移动语义，需要实现移动构造函数和移动赋值运算符。移动构造函数和移动赋值运算符应使用 `noexcept` 限定符标记。这对于与标准库兼容非常重要，因为如果实现了移动语义，标准库容器会移动存储的对象，且确保不抛出异常。

下面的Spreadsheet类定义包含一个移动构造函数和一个移动赋值运算符。也引入了两个辅助方法 `cleanup()` 和 `moveFrom()`。前者在析构函数和移动赋值运算符中调用。后者用于把成员变量从源对象移动到目标对象，接着重置源对象。

```cpp
export class Spreadsheet {
public:
  Spreadsheet(Spreadsheet&& src) noexcept;	          // move constructor
  Spreadsheet& operator=(Spreadsheet&& rhs) noexcept;	// move assign
  // ...
private:
  void cleanup() noexcept;
  void moveFrom(Spreadsheet& src) noexcept;
  // ...
};
```

实现代码如下：

```cpp
void Spreadsheet::cleanup() noexcept {
  for (size_t i { 0 }; i < m_width; ++i) {
    delete[] m_cells[i];
  }
  delete[] m_cells;
  m_cells = nullptr;
  m_width = m_height = 0;
}

void Spreadsheet::moveFrom(Spreadsheet& src) noexcept {
  // shallow copy of data
  m_width = src.m_width;
  m_height = src.m_height;
  m_cells = src.m_cells;

  // reset the source object, because ownership has been moved!
  src.m_width = 0;
  src.m_height = 0;
  src.m_cells = nullptr;
}

// move constructor
Spreadsheet::Spreadsheet(Spreadsheet&& src) noexcept {
  moveFrom(src);
}

// move assign
Spreadsheet& Spreadsheet::operator=(Spreadsheet&& rhs) noexcept {
  if (this == &rhs)	return *this;

  // free the old memory and move ownership
  cleanup();
  moveFrom(rhs);
  return *this;
}
```

移动构造函数和移动赋值运算符都将m_cells的内存所有权从源对象移动到新对象。移动完成后将源对象资源置空，以防止源对象的析构函数释放这块内存，因为新的对象现在拥有了这块内存。

注意，此实现在移动赋值运算符中包含一个自我赋值检查。取决于你的类以及将类的一个实例移动到另一个实例的方法，此自我赋值检查可能不是必要的。最好将其包括在内，以确保以下代码不会出现问题：

```cpp
sheet1 = std::move(sheet1);
```

就像普通的构造函数或拷贝赋值运算符一样，可显式地将移动构造函数和移动赋值运算符设置为默认或将其弃置。

> 仅当类没有用户声明的拷贝构造函数、拷贝赋值运算符、移动赋值运算符或析构函数时，编译器才会为类自动生成默认的移动构造函数。仅当类没有用户声明的拷贝构造函数、移动构造函数、拷贝赋值运算符或析构函数时，才会为类生成默认的移动赋值运算符。

#### 使用 `std::exchange()`

定义在 `<utility>` 中的 `std::exchange()` ，可以用一个新的值替换原来的值，并返回原来的值。例如：

```cpp
int a { 11 };
int b { 22 };
std::cout << std::format("Before exchange(): a = {}, b = {}", a, b) << std::endl;
int returnValue { std::exchange(a, b) };
std::cout << std::format("After exchange(): a = {}, b = {}", a, b) << std::endl;
std::cout << std::format("After exchange(): returnValue = {}", returnValue) << std::endl;
```

输出如下：
```
Before exchange(): a = 11, b = 22
After exchange(): a = 22, b = 11
After exchange(): returnValue = 22
```

在实现移动赋值运算符后，`exchange()` 十分有用。移动赋值运算符需要将数据从源对象移动到目标对象，之后源对象的数据通常为空。使用 `exchange()` 可以更简洁得编写，如下所示：

```cpp
void Spreadsheet::moveFrom(Spreadsheet& src) noexcept {
  m_width = std::exchange(src.m_width, 0);
  m_height = std::exchange(src.m_height, 0);
  m_cells = std::exchange(src.m_cells, nullptr);
}
```
  
#### 移动对象数据成员

`moveFrom()` 方法对3个数据成员直接赋值，因为这些成员都是基本类型。如果对象还有其他对象作为数据成员，则应当使用 `std::move()` 将其移动到新的对象。

假设Spreadsheet类有一个名为m_name的 `std::string` 数据成员。接着采用以下方式实现 `moveFrom()` 方法：

```cpp
void Spreadsheet::moveFrom(Spreadsheet& src) noexcept {
  // move object data members
  m_name = std::move(src.m_name);

  // move primitives
  m_width = std::exchange(src.m_width, 0);
  m_height = std::exchange(src.m_height, 0);
  m_cells = std::exchange(src.m_cells, nullptr);
}
```

#### 使用 `std::swap()` 实现移动构造函数和移动赋值运算符

前面的移动构造函数和移动赋值运算符的实现都使用了 `moveFrom()` 辅助方法。在此实现中，如果给Spreadsheet类添加了其他对象作为数据成员，则必须修改 `swap()` 函数和 `moveFrom()` 方法。为了避免忘记修改引发的bug，可使用 `std::swap()` 函数，编写移动构造函数和移动赋值运算符。

首先删除 `cleanup()` 和 `moveFrom()` 辅助方法，将 `cleanup()` 方法中的代码移入析构函数。此后，可以按照如下实现移动构造函数和移动赋值运算符：

```cpp
Spreadsheet::Spreadsheet(Spreadsheet&& src) noexcept {
  std::swap(*this, src);
}

Spreadsheet& Spreadsheet::operator=(Spreadsheet&& rhs) noexcept {
  std::swap(*this, rhs);
  return *this;
}
```

移动构造函数只是简单地将默认构造的 `*this` 与给定的源对象进行交换。同样，移动赋值运算符也是简单地将默认赋值的 `*this` 与给定的源对象进行交换。

> **注意**
>
> 用 `std::swap()` 函数实现移动构造函数和移动赋值运算符所需的代码更少，当加入新的数据成员时出现bug的可能性也更低。因为你只需要升级 `swap()` 函数，使其包括新的成员。




* 如果想对一个lvalue使用移动语义，C++11提供了 `std::move()` ，将lvalue转化为rvalue
* lvalue被move后，不会立刻析构，只有离开自己作用域时才会析构，继续使用左值中的资源会报错
* 如果没有提供移动构造/赋值函数，编译器会调用拷贝构造/赋值函数
* C++11中所有容器都已经实现移动语义
* 移动语义对于拥有资源的对象有效，对于基本类型没有意义
```cpp
A a2 = std::move(a1);                  // 调用移动构造函数
std::cout << "a2.m_data = " << *a2.m_data <<std::endl;
```
两个代码块不可以在同一作用域进行
```cpp
A a3;
a3 = std::move(a1);                    // 调用移动赋值函数
std::cout << "a3.m_data = " << *a3.m_data <<std::endl;
```