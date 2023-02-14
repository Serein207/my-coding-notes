# Modern C++ 初始化

### 语法

变量的初始化会在构造时提供变量的初始值。

初始值可以由声明符或 `new` 表达式的初始化器部分提供。在函数调用时也会发生：函数形参及函数返回值也会被初始化。

对于每个声明符，初始化器必须是下列之一：

- `( 表达式列表 )` 括号中的以逗号分隔的含有任意表达式和花括号初始化器列表的列表
- `= 表达式` 等号后面跟着一个表达式
- `{ 初始化器列表 }` 花括号初始化器列表：以逗号分隔且可以为空的含有表达式和其他花括号初始化器列表的列表

根据上下文，初始化器可以调用：

- 值初始化，例如 `std::string s{};`
- 直接初始化，例如 `std::string s("hello");`
- 复制初始化，例如 `std::string s = "hello";`
- 列表初始化，例如 `std::string s{'a', 'b', 'c'};`
- 聚合初始化，例如 `char a[3] = {'a', 'b'};`
- 引用初始化，例如 `char& c = a[0];`

如果不提供初始化器，那么就会应用默认初始化的规则。

初始化包括了对初始化器中的子表达式进行求值以及为函数实参和返回值创建临时对象。

### 统一初始化

在C++11前，各类型的初始化并非是统一的。例如，考虑下面两个定义，其中一个作为结构体，另一个作为类。

```cpp
struct CircleStruct {
  int x, y;
  double radius;
};

class CircleClass {
 public:
   CircleClass(int x, int y, double radius)
    : m_x{x}, m_y{y}, m_radius{radius}; {}

 private:
   int m_x, m_y;
   double m_radius;
};
```

在C++11之前，CircleStruct类型变量和CircleClass类型变量是不同的：

```cpp
CircleStruct myCircle1 = {10, 10, 2.5};
CircleClass myCircle2(10, 10, 2.5);
```

对于结构体版本，可使用 `{...}` 语法。然而，对于类版本，需要使用函数符号 `(...)` 调用构造函数。

自C++11以后，允许一律使用 `{...}` 语法初始化类型，如下所示：

```cpp
CircleStruct myCircle3 = {10, 20, 2.5};
CircleClass myCircle4 = {10, 20, 2.5};
```

定义myCircle4时将自动第哦啊用CircleClass的构造函数。甚至等号也是可选的，因此下面的代码与前面的代码等价：

```cpp
CircleStruct myCircle5{10, 20, 2.5};
CircleClass myCircle6{10, 20, 2.5};
```

使用统一初始化并不局限于结构体和类，它还可以用于初始化C++中的任何内容。例如，下面的代码吧所有4个变量都初始化为3：
```cpp
int a = 3;
int b{3};
int c = {3};
int d{3};
```

统一初始化还可用于对变量进行零初始化，只需要指定一对空白的大括号：

```cpp
int e{};
```

使用统一初始化的一个优点是可以阻止它窄化(narrowing)。当使旧风格的赋值语法初始化变量时，C++隐式地执行窄化：

```cpp
void func(int i) {...}

int main() {
  int x = 3.14;
  func(3.14);
}
```

在main函数的两行代码中，C++在对x赋值或调用func之前，会自动将3.14截断为3。而使用统一初始化，x的赋值和func的调用都会发生编译错误：

```cpp
int x{3.14};  // error
func({3.14}); // error
```

统一初始化还可以用来初始化动态分配的数组：

```cpp
int* mtArray = new int[4] { 0, 1, 2, 3 };
```

从C++20开始，可以省略数组的大小4：

```cpp
int* mtArray = new int[] { 0, 1, 2, 3 };
```

统一初始化还可以在构造函数初始化器中初始化成员数组：

```cpp
class MyClass {
 public:
   MyClass() : m_array { 0, 1, 2, 3 } {}
 private:
   int m_array[4];
};
```

统一初始化还可用于标准库容器：

```cpp
std::vector<int> v{ 1, 2, 3, 4 };
```

### 指派初始化器

C++20引入指派初始化器，以使它们的名称初始化所谓聚合的数据成员。

**聚合体** 是以下类型之一：

- 数组类型
- 符合以下条件的类类型（通常是 `struct` 或 `union` ）
  - 没有用户提供、继承或 `explicit` 的构造函数（C++11起 C++20前）
  - 没有用户声明或继承的构造函数（C++20起）
  - 没有私有或受保护的直接非静态数据成员
  - 没有基类（C++17前）
  - 没有虚基类（C++17起）
  - 没有私有或受保护的直接基类（C++17起）
  - 没有虚成员函数
  - 没有默认成员初始化器（C++11起 C++14前）

指派初始化器以 `.` 开头，后跟数据成员的名称。指派初始化的顺序必须与数据成员的声明顺序相同。不允许混合使用指派初始化器和非指派初始化器。未使用指派初始化器初始化的任何数据成员都将使用其默认值进行初始化，这意味着：

- 拥有类内初始化器的数据成员会先得到该值
- 没有类内初始化器的数据成员会被零初始化

让我们来看一下略微修改的Employee结构，这次，salary数据成员的默认值75000

```cpp
struct Employee {
  char firstInitial;
  char lastInitial;
  int employeeNumber;
  int salary{75'000};
};
```

`Employee` 结构可以使用如下的统一初始化语法初始化：

```cpp
Employee anEmployee{'J', 'D', 42, 80'000};
```

使用指派初始化器，可以被写成这样：

```cpp
Employee anEmployee{.firstInitial = 'J',
                    .lastInitial = 'D',
                    .employeeNumber = 42,
                    .salary = 80'000};
```

使用指派初始化器的好处是，与使用统一初始化依法相比，它更容易理解指派初始化器正在初始化的内容。

使用指派初始化器，如果对某些成员的默认值感到满意，则可以跳过它们的初始化。例如，在创建员工时，可以跳过初始化 `employeeNumber` ，在这种情况下，`employeeNumber` 初始化为零，因为它没有类内初始化器。

```cpp
Employee anEmployee{.firstInitial = 'J',
                    .lastInitial = 'D',
                    .salary = 80'000};
```

如果使用统一初始化器，这是不可以的，必须像下面这样指定 `employeeNumber` 为0。

```cpp
Employee anEmployee{'J', 'D', 0, 80'000};
```

如果你像下面这样跳过了初始化 `salary` 数据成员，它就会得到它的默认值，即它的类内初始化值75000。

```cpp
Employee anEmployee{.firstInitial = 'J',
                    .lastInitial = 'D',};
```

使用指派初始化器的最后一个好处是，当新成员被添加到数据结构时，使用指派初始化器的现有代码将继续起作用。新的数据成员将使用其默认值进行初始化。
