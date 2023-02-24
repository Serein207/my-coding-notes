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

## 赋值和移动

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

#### 实现移动语义

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

#### 使用移动语义实现交换函数

考虑交换两个对象的 `swap()` 函数模板，这是另一个使用移动语义提高性能的实例。下面的 `swapCopy()` 实现没有使用移动语义：

```cpp
template <typename T>
void swapCopy(T& lhs, T& rhs) {
  T temp { lhs };
  lhs = rhs;
  rhs = temp;
}
```

对于这种实现方式，如果类型T的复制开销很大，这个交换将严重影响性能。使用移动语义，`swap()` 函数可避免所有复制：

```cpp
template <typename T>
void swap(T& lhs, T& rhs) {
  T temp { std::move(lhs) };
  lhs = std::move(rhs);
  rhs = std::move(temp);
}
```

这正是标准库的 `std::swap()` 的实现方式。

#### 在返回语句中使用 `std::move()`

对于 `return object;` 形式的语句，如果object是局部变量、函数的参数或临时值，则它们被视为右值表达式，并触发返回值优化（RVO）。此外，若object是一个局部变量，则会启动命名返回值优化（NRVO）。RVO和NRVO都是复制消除的形式，使得从函数返回对象非常高效。使用复制消除，编译器可以避免复制和移动函数返回的对象。这导致了所谓的零拷贝值传递语义。

但是，通过使用 `std::move()`，编译器无法再应用RVO和NRVO，因为这只是用于形式为 `return object;` 的语句。编译器会继续选择移动语义，如果不支持，则使用拷贝语义，这会对性能有很大的影响！因此， **当从函数中返回一个局部变量或参数时，只要写 `return object;` 就可以了，不要使用 `std::move()`。**

#### 向函数传递参数的最佳方法

到目前为止，建议对非基本类型的函数参数使用const引用参数，以避免对传递给函数的实参进行不必要的昂贵复制。但是，如果混合使用右值，情况会略有变化。假设一个函数复制了作为其参数之一传递的实参（这种情况经常在类方法中出现）。下面是一个简单的例子：

```cpp
class DataHolder {
 public:
   void setData(const std::vector<int>& data) { m_data = data; }
 private:
   std::vector<int> m_data;
};
```

`setData()` 方法生成一份传入数据的副本。既然你对右值和右值引用都很熟悉，那么你可能需要添加一个重载来优化 `setData()` 方法，以避免对传入的数据进行不必要的昂贵复制。例如：

```cpp
class DataHolder {
 public:
   void setData(const std::vector<int>& data) { m_data = data; }
   void setData(std::vector<int>&& data) { m_data = std::move(data); }
 private:
   std::vector<int> m_data;
};
```

当以临时调用 `setData()` 时，不会产生任何复制，数据将被移动。

以下代码触发对 `setData()` 的const引用重载版本的调用，从而生成数据的副本：

```cpp
DataHolder wrapper;
std::vector myData { 11, 22, 33 };
wrapper.setData(myData);
```

另一方面，下面的代码段使用临时变量调用 `setData()`，这回触发对 `setData()` 的右值引用重载版本的调用。随后将移动数据，而不是复制数据。

```cpp
wrapper.setData({ 11, 22, 33 });
```

但是，这种作为左值和右值优化的 `setData()` 方法需要实现两个重载。幸运的是，对于单个方法来说有一种更好的方法。是的，值传递！到目前为止，建议对非基本类型的函数参数使用const引用参数，以避免对传递给函数的实参进行不必要的昂贵复制。但是现在我们建议使用值传递，这只适用于函数无论如何都要被复制的参数。在这种情况下，通过使用值传递语义，代码对于左值和右值都是最优的。如果传入一个左值，它只复制一次，就像const引用参数一样。如果传入一个右值，则不会进行复制，就像右值引用参数一样。让我们看一些代码：

```cpp
class DataHolder {
 public:
   void setData(const std::vector<int> data) { m_data = std::move(data); }
 private:
   std::vector<int> m_data;
};
```

如果将左值传递给 `setData()` 方法，则会将其复制到data参数中，然后移动到m_data。如果将右值传递给 `setData()` 方法，则会将其移动到data参数中，然后再次移动到m_data。

> **注意**
>
> 对于函数本身将复制的参数，更倾向于值传递，但仅当该参数属于支持移动语义的类型时。否则，请使用const引用参数。

## 与方法有关的更多内容

C++为方法提供了许多选择，本节将详细介绍这些细节。

### 方法重载

#### 1. 基于 `const` 的重载

可以编写两个名称相同、参数也相同的方法，其中一个是 `const` 的，另一个不是。如果是const对象，就调用const方法；如果是非const对象，就调用非const方法。

通常情况下，const版本和非const版本的实现是一样的。为避免代码重复，可使用 `const_cast()` 转换。例如，Spreadsheet类有一个 `getCellAt()` 方法，该方法返回SpreadsheetCell的非const引用。可添加const重载版本，它返回SpreadsheetCell的const引用。

```cpp
export class Spreadsheet {
 public:
   SpreadsheetCell& getCellAt(size_t x, size_t y);
   const SpreadsheetCell& getCellAt(size_t x, size_t y) const;
   // ...
};
```

对于 `const_cast()` 转换，你可以像往常一样实现const版本，此后通过适当转换，传递对const版本的调用，以实现非const版本。如下所示：

```cpp
const SpreadsheetCell& Spreadsheet::getCellAt(size_t x, size_t y) const{
  verifyCoordinate(x, y);
  return m_cells[x][y];
}

SpreadsheetCell& Spreadsheet::getCellAt(size_t x, size_t y) {
  return const_cast<SpreadsheetCell&>(std::as_const(*this).getCellAt(x, y));
}
```

基本上，你使用 `std::as_const()` 将 `*this` 转换为 `const Spreadsheet&` ，调用 `getCellAt()` 方法，并将结果返回为 `const SpreadsheetCell&` ，然后使用 `const_cast()` 转换为非const的 `SpreadsheetCell&` 。

有了这两个重载的 `getCellAt()` ，现在可以在const和非const的Spreadsheet对象上调用 `getCellAt()` 方法：

```cpp
Spreadsheet sheet1 { 5, 6 };
SpreadsheetCell& cell { sheet1.getCellAt(1, 1) };


const Spreadsheet sheet2 { 5, 6 };
const SpreadsheetCell cell2 { sheet2.getCellAt(1, 1) };
```

这里，`getCellAt()` 的const版本做的事情不多，因此使用 `const_cast()` 转换的优势不明显。但如果它能做更多工作，那么通过非const版本转递给const版本，可省区很多代码。

#### 2. 显式弃置重载

重载方法可以被显式弃置，可以通过这种方法禁止调用具有特定参数的成员函数。例如，SpreadsheetCell类有一个方法叫 `setValue(double)`，可按如下方式调用：

```cpp
SpreadsheetCell cell;
cell.setValue(3.14);
cell.setValue(314);
```

在第三行，编译器将整型值（314）转换为double，然后调用 `setValue(double)`。如果出去某些原因，你不希望以整型调用 `setValue()`，可以显式弃置 `setValue()` 的整型重载版本：

```cpp
export class SpreadsheetCell {
 public:
   void setValue(double value);
   void setValue(int) = delete;
};
```

通过这一改动，以整型为参数调用 `setValue()` 时，编译器会报错。

#### 3. 引用限定方法

可以对类的非临时和临时对象调用普通类方法。假设有以下类：

```cpp
class TextHolder {
 public:
   TextHolder(std::string text) : m_text { std::move(text) } {}
   const std::string& getText() const { return m_text; }
 private:
   std::string m_text;
};
```

毫无疑问，可以在TextHolder的非临时对象上调用 `getText()` 方法，如下所示：

```cpp
TextHolder textHolder { "Hello World" };
std::cout << textHolder.getText() << std::endl;
```

`getText()` 也可以被临时调用：

```cpp
std::cout << TextHolder{ "Hello world!" }.getText() << std::endl;
std::cout << std::move(textHolder).getText() << std::endl;
```

可以显式指定能够调用某个方法的类型实例，无论是临时实例还是非临时实例。这是通过向方法添加一个 *引用限定符(ref-qualifier)* 来实现的。如果只应在非临时实例上调用方法，则在方法头之后添加一个 `&` 限定符。类似地，如果只应在临时实例上调用方法，则要添加一个 `&&` 限定符。

```cpp
class TextHolder {
 public:
   TextHolder(std::string text) : m_text { std::move(text) } {}
   const std::string& getText() const & { return m_text; }
   std::string&& getText() const && { return std::move(m_text); }
 private:
   std::string m_text;
};
```

假设你有以下调用：

```cpp
TextHolder textHolder { "Hello World" };
std::cout << textHolder.getText() << std::endl;
std::cout << TextHolder{ " Hello world!" }.getText() << std::endl;
std::cout << std::move(textHolder).getText() << std::endl;
```

第一个对 `getText()` 的调用使用了 `&` 限定符重载，第二个和第三个使用了 `&&` 限定符重载。

### 内联方法

C++可以建议函数或方法的调用不在生成的代码中实现，就像调用独立的代码块那样。相反，编译器应将方法体直接插入调用方法的位置。这个过程称为内联(inline)，具有这一行为的方法称为内敛方法。

注意，`inline` 关键字只是提示编译器，如果编译器认为这会降低性能，则会忽略关键字。

在所有调用了内联函数或内联方法的源文件中，内联方法或内联函数的定义必须有效。如果编写了内联方法，应该将该方法定义与其所在的类的定义放在同一文件中。

> 高级C++编译器不要求把内联方法和定义放在同一文件中。MSVC, GCC, Clang具有此特性。

不使用C++20模块时，如果方法的定义直接放在类定义中，则该方法会隐式标记为 `inline`，即使不使用 `inline` 关键字。对于从模块导出的类，情况不再如此。如果希望这些方法时内联的，则需要使用 `inline` 关键字标记它们。

## 不同的数据成员类型

### `static` 数据成员

不仅要在类定义中列出static成员，还需要再源文件中为其分配内存，通常是定义类方法的那个源文件。在此可以初始化static成员，但注意与普通变量和数据成员不同，默认情况下它们会做零初始化。

#### 内联变量

从C++17开始，可将静态数据成员声明为 `inline`。这样做的好处是不必在源文件中为它们分配空间。下面是一个示例：

```cpp
export class Spreadsheet {
  // ...
 private:
   static inline size_t ms_counter { 0 };
};
```

注意其中的 `inline` 关键字。有了这个类定义，可从源文件中删除下面代码行：

```cpp
size_t Spreadsheet::ms_counter;
```

### `const static` 数据成员

如果某个常量只适用于类，应该使用static const（或const static）数据成员，也称为类常量，而不是全局常量。可以在类定义中定义和初始化整型或枚举类型的const static数据成员，而不需要将其指定为内联变量。

例如，你可能想指定电子表格的最大高度和宽度。如果用户想要创建电子表格的高度或宽度大于最大值，就改用最大值。可将最大高度和宽度设置为Spreadsheet类的static const成员：

```cpp
export class Spreadsheet {
 public:
   //...
   static const size_t MaxHeight { 100 };
   static const size_t MaxWidth { 100 };
};
```

可以在构造函数中使用这些新常量，如下面的代码片段所示：

```cpp
Spreadsheet::Spreadsheet(size_t width, size_t height)
  : m_id { ms_counter++ }
  , m_width { std::min(width, MaxWidth) }
  , m_height { std::min(height, MaxHeight) }
{
  //...
}
```

这些常量也可用作构造函数参数的默认值：

```cpp
export class Spreadsheet {
public:
  Spreadsheet(size_t width = MaxWidth, size_t height = MaxHeight);
  //...
};
```

### 引用数据成员

Spreadsheet和SpreadsheetCell这两个类本身并不能组成非常有用的应用程序。为了用代码控制整个电子表格程序，可将这两个类一起放入SpreadsheetApplication类。假设将在需要在每一个Spreadsheet类中存储一个应用程序对象的引用。SpreadsheetApplication类的实现再次并不重要，所以下面的代码简单地将其定义为空类。Spreadsheet类被修改了，以容纳一个名为 `m_theApp` 的新的引用数据类型。

```cpp
export class SpreadsheetApplication {};

export class Spreadsheet {
 public:
   Spreadsheet(size_t width, size_t height, 
    SpreadsheetApplication& theApp);
   //...
 private:
   //...
   SpreadsheetApplication& m_theApp;
};
```

这个定义将一个SpreadsheetApplication引用作为数据成员添加进来。在此情况下建议使用引用而不是指针，因为Spreadsheet总会指向一个SpreadsheetApplication，而指针无法保证这一点。

> 请注意，存储对应应用程序的引用，仅是为了演示把引用作为数据成员的用法。不建议以这种方式把Spreadsheet类和SpreadsheetApplication类组合在一起。

在构造函数中每个Spreadsheet都得到一个应用程序引用。如果不指向某些事物，引用将无法存在，因此在构造函数的初始化器中必须给 `m_theApp` 指定一个值。

```cpp
Spreadsheet::Spreadsheet(size_t width, size_t height, SpreadsheetApplication& theApp)
	: m_id { ms_counter++ }
	, m_width { std::min(width, MaxWidth) }
	, m_height { std::min(height, MaxHeight) }
  , m_theApp { theApp }
{
  //...
}
```

在拷贝构造函数中也必须初始化这个引用成员。由于Spreadsheet拷贝构造函数委托给非拷贝构造函数，因此这将自动处理。

在初始化引用后，不能更改它的指向。因此无法在赋值运算符中对引用赋值。如果属于这种情况，通常将赋值运算符标记为删除。

最后，引用数据成员也可以被标记为const。例如，你可能决定让Spreadsheet类只包含应用程序对象的const引用，只需要在类定义中将 `m_theApp` 指定为const引用。

```cpp
export class Spreadsheet {
 public:
   Spreadsheet(size_t width, size_t height, 
    const SpreadsheetApplication& theApp);
   //...
 private:
 	 //...
   const SpreadsheetApplication& m_theApp;
};
```

## 嵌套类

类定义不仅可包含成员函数和数据成员，还可编写嵌套类和嵌套的结构体、声明类型别名或者创建枚举类型。类中声明的一切内容都具有类作用域。如果声明的内容是public的，那么可在类外使用 `ClassName::` 作用域解析语法访问。

可在类的定义中提供另一个类的定义。例如，假定SpreadsheetCell类实际上是Spreadsheet类的一部分，因此不妨将SpreadsCell类重命名为Cell。可将二者定义为：

```cpp
export class Spreadsheet {
 public:
   class Cell {
    public:
      Cell() = default;
      Cell(double initialValue);
      //...
   };

   Spreadsheet(size_t width, size_t height, 
     const SpreadsheetApplication& theApp);
   //...
};
```

现在Cell类定义位于Spreadsheet类内部，因此在Spreadsheet类外引用Cell必须用 `Spreadsheet::` 作用域限定名称，即使在方法定义时也是如此。例如，Cell的double构造函数应如下所示：

```cpp
Spreadsheet::Cell::Cell(double initialValue)
  : m_value { initialValue } {}
```

甚至在Spreadsheet类方法的返回类型也必须使用这一语法：

```cpp
Spreadsheet::Cell& Spreadsheet::getCellAt(size_t x, size_t y) {
  verifyCoordinate(x, y);
  return m_cells[x][y];
}
```

如果在Spreadsheet类中直接完整定义嵌套的Cell类，将使Spreadsheet类的定义略显臃肿。为缓解这一点，只需要在Spreadsheet类中为Cell添加前置声明，然后独立地定义Cell类，如下所示：

```cpp
export class Spreadsheet {
 public:
   class Cell;
   
   Spreadsheet(size_t width, size_t height,
     const SpreadsheetApplication& theApp);
   //...
};

class Spreadsheet::Cell {
 public:
   Cell() = default;
   Cell(double initialValue);
   //...
};
```

普通的访问控制也适用于嵌套类定义。如果声明了一个private或protected嵌套类，这个类只能在外围类中使用。嵌套类有权访问外围类中的所有private或protected成员，而外围类却只能访问嵌套类中的public成员。

## 运算符重载

### 使用全局函数重载可交换的运算符

```cpp
SpreadsheetCell SpreadsheetCell::operator+(const SpreadsheetCell& cell) const {
  return SpreadsheetCell { getValue() + cell.getValue() };
}
```

隐式转换允许 `operator+` 成员方法将SpreadsheetCell对象与int和double值相加。然而，这个运算符不具有互换性。如下所示：

```cpp
SpreadsheetCell myCell { 4 }, aCell;
aCell = myCell + 2;     // ok
aCell = myCell + 4.0;   // ok
aCell = 2 + myCell;     // fail
aCell = 4.0 + myCell;   // fail
```

当SpreadsCell对象在运算符左边时，隐式转换正常进行。但在右边时无法进行。加法必须是可交换的，因此这里存在错误。

但是，如果不局限于某个特定对象的全局 `operator+` 替换类内的 `operator+` 方法，上面的代码就可以运行，如下所示：

```cpp
SpreadsheetCell operator+(const SpreadsheetCell& lhs,
	const SpreadsheetCell& rhs) {
	return SpreadsheetCell { lhs.getValue() + rhs.getValue() };
}
```

需要在模块接口文件中声明运算符并将其导出：

```cpp
export SpreadsheetCell operator+(const SpreadsheetCell& lhs, 
	const SpreadsheetCell& rhs);
```

这样，下面的4个加法运算都可以按预期运行：

```cpp
SpreadsheetCell myCell { 4 }, aCell;
aCell = myCell + 2;     // ok
aCell = myCell + 4.0;   // ok
aCell = 2 + myCell;     // ok
aCell = 4.0 + myCell;   // ok
```

### 重载比较运算符

C++20添加了三向比较运算符，也为其他比较运算符(>,<,<=,>=,==,!=)带来了很多变化。

与基本算数运算符一样，6个C++20之前的比较运算符应该是全局函数，这样就可以在运算符的左侧和右侧参数上使用隐式转换。比较运算符都返回bool，当然也可以更改返回类型，但不建议这样做。

当类的数据成员较多时，比较每个数据成员可能比较痛苦。然而，当实现了 `==` 和 `<` 后，可以根据这两个运算符编写其他比较运算符。例如，下面的 `operator>=` 定义使用了 `operator<` 。

```cpp
bool operator>=(const SpreadsheetCell& lhs, const SpreadsheetCell& rhs) {
  return !(lhs < rhs);
}
```

> 大多数时候，最好不要对浮点数执行相等或不相等测试。

如你所见，需要编写6个单独的函数来支持6个比较运算符。使用当前实现的6个比较函数，可以将SpreadsheetCell与double进行比较，因为double隐式参数转换为SpreadsheetCell。这种隐式转换可能效率低下，因为必须创建临时对象。可以通过显式实现与double进行比较的函数来避免这种情况。对于每个操作符 `<op>`，将需要以下三个重载：

```cpp
bool operator<op>(const SpreadsheetCell& lhs, const SpreadsheetCell& rhs);
bool operator<op>(const SpreadsheetCell& lhs, const double& rhs);
bool operator<op>(const double& lhs, const SpreadsheetCell& rhs);
```

如果你需要支持所有的比较运算符，需要写很多重复的代码！

现在让我们看看C++20带来了什么。C++20简化了向类中添加比较运算符的支持。首先，对于C++20，建议将 `operator==` 实现为类的成员函数，而不是全局函数。还要注意，添加 `[[nodiscard]]` 属性是个好主意，这样操作符结果就不能被忽略。例如：

```cpp
[[nodiscard]] bool operator==(const SpreadsheetCell& rhs) const;
```

在C++20中，单个的 `operator==` 重载就可以使下面的比较生效：

```cpp
if(myCell == 10) std::cout << "MyCell == 10\n";
if(10 == myCell) std::cout << "10 == MyCell\n";
```

编译器会将 `10 == myCell` 重写为 `myCell == 10` ，然后调用其成员函数。此外，通过实现 `operator==`，C++20会自动添加对 `!=` 的支持。

接下来，要实现对全套比较运算符的子好吃，在C++20中，只需要实现一个额外的重载运算符，`operator<=>`。一旦类有运算符 `==` 和 `<=>` 的重载，C++20就会自动为所有6个比较运算符提供支持！对于Spreadsheet类，运算符 `<=>` 如下所示：

```cpp
[[nodiscard]] std::partial_ordering operator<=>(const SpreadsheetCell& rhs) const;
```

存储在SpreadsheetCell类中的值是double类型的。浮点类型只有偏序，这就是重载返回 `std::partial_ordering` 的原因。实现非常简单：

```cpp
std::partial_ordering operator<=>(const SpreadsheetCell& rhs) const {
  return getValue() <=> rhs.getValue();
}
```

通过实现 `operator<=>`，C++20会自动添加对 >,<,<=,>=的支持。所以，只要实现 `operator==` 和 `operator<=>`，SpreadsheetCell类就可以支持所有的比较运算符。

与前面一样，如果希望避免隐式转换对性能的轻微影响，可以为double提供特定的重载。只需要提供以下两个额外的重载运算符作为方法：

```cpp
[[nodiscard]] std::partial_ordering operator<=>(const double rhs) const;
[[nodiscard]] bool operator==(const double rhs) const;
```

#### 编译器生成的比较运算符

注意SpreadsheetCell的 `operator==` 和 `operator<=>` 的实现，它们只是简单比较所有数据车官员。在这种欧冠情况下，可以将它们设为默认。此外，如果是显式默认的 `operator<=>` ，编译器也会自动包含一个默认的 `operator==`。因此，对于SpreadsheetCell类，如果没有显式的double版本的`operator==` 和 `operator<=>` ，只需编写以下单行代码，即可添加所有6个比较运算符的完全支持：

```cpp
[[nodiscard]] std::partial_ordering operator<=>(
  const SpreadsheetCell&) const = default;
```

此外，还可以使用 `auto` 作为 `operator<=>` 的返回类型，编译器将根据数据成员的 `<=>` 运算符的返回类型推断返回类型。如果数据成员不支持 `operator<=>` ，那么返回类型推断将不起作用，需要显式指定返回类型。

显式默认的 `operator<=>` 适用于没有显式的double版本的 `operator==` 和 `operator<=>` 。如果确实添加了这些显式double版本，则添加的是用户声明的 `operator==(double)`。因此，编译器不再自动生成 `operator==(const SpreadsheetCell&)`，因此必须将其显式默认，如下所示：

```cpp
[[nodiscard]] std::partial_ordering operator<=>(
  const SpreadsheetCell&) const = default;
[[nodiscard]] bool operator==(
  const SpreadsheetCell&) const = default;

[[nodiscard]] bool operator==(double rhs) const;
[[nodiscard]] std::partial_ordering operator<=>(double rhs) const;
```

尽量将 `operator<=>` 设置为显式默认，它将于新添加或修改地数据成员保持同步。

只有当 `operator==` 和 `operator<=>` 使用定义操作符的类类型的const引用作为参数时，才可能将 `operator==` 和 `operator<=>` 设置为显式默认。例如，以下操作不起作用：

```cpp
[[nodiscard]] std::partial_ordering operator<=>(double)const = default;
```

## 继承技术

### `final` 和 `override`

#### `final` 说明符

当在虚函数声明或定义中使用时，`final` 说明符确保函数为虚并指定它不能被派生类覆盖，否则程序非良构（生成编译时错误）。

当在类定义中使用时，`final` 指定此类不能派生子类，否则程序非良构（生成编译时错误）。

#### `override` 关键字

在成员函数的声明或定义中，`override` 说明符确保该函数为虚函数并覆盖某个基类中的虚函数。如果不是这样，那么程序非良构（生成编译错误）。

#### 语法

用于类

```cpp
class Foo final {};

class Derived : public Base {};   // compile error
```

用于方法

```cpp
class Base {
 public:
   virtual void foo();
};

class Derived : public Base {
 public:
   void foo() override final;
};

class Derived1: public Derived {
 public:
   void foo() override;   // compile error
};
```

### 类型转换

#### `std::bit_cast()`

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