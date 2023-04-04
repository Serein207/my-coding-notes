# Modern C++ 函数指针，函数对象，lambda表达式

> C++中的函数被称为一级函数，因为函数可以像普通变量一样使用，例如将它们作为参数传递给其他函数，从其他函数返回它们，将它们赋值给变量。这叫做回调，他表示可以调用的东西，它可以是函数指针或任何行为类似于函数指针的东西。重载 `operator()` 的类称为函数对象，简称functor。标准库提供了一组可用于创建回调对象和调整现有回调对象的类，这很方便。

- [Modern C++ 函数指针，函数对象，lambda表达式](#modern-c-函数指针函数对象lambda表达式)
  - [函数指针](#函数指针)
  - [指向方法（和数据成员）的指针](#指向方法和数据成员的指针)
    - [`std::function`](#stdfunction)
  - [函数对象](#函数对象)
    - [编写第一个函数对象](#编写第一个函数对象)
    - [标准库中的函数对象](#标准库中的函数对象)
      - [1. 算数函数对象](#1-算数函数对象)
      - [2. 透明运算符仿函数](#2-透明运算符仿函数)
      - [3. 比较函数对象](#3-比较函数对象)
      - [4. 逻辑函数对象](#4-逻辑函数对象)
      - [5. 位函数对象](#5-位函数对象)
      - [6. 适配器函数对象](#6-适配器函数对象)
        - [绑定器](#绑定器)
        - [否定器](#否定器)
        - [调用成员函数](#调用成员函数)
  - [lambda表达式](#lambda表达式)
    - [语法](#语法)
    - [lambda表达式作为参数](#lambda表达式作为参数)
    - [泛型lambda表达式](#泛型lambda表达式)
    - [lambda捕获表达式](#lambda捕获表达式)
    - [模板化lambda表达式](#模板化lambda表达式)
    - [lambda表达式作为返回类型](#lambda表达式作为返回类型)
    - [不求值表达式中的lambda表达式](#不求值表达式中的lambda表达式)
    - [默认构造、拷贝和赋值](#默认构造拷贝和赋值)
    - [lambda函数递归](#lambda函数递归)
  - [调用](#调用)

## 函数指针

函数指针根据参数类型和兼容函数的返回类型进行类型划分，使用函数指针的一种方法是使用类型别名，类型别名允许将类型名称赋值给具有给定特征的函数族。距离，下面定义了一种名为Matcher的类型，它表示指向任何具有两个int形参并返回bool的函数的指针：

```cpp
using Matcher = bool(*)(int, int);
```

下面的类型别名定义了一种名为MatchHandler的类型，用于接收一个size_t和两个int作为参数和空返回的函数：

```cpp
using MatchHandler = void(*)(size_t, int, int);
```

可以编写一个接收两个回调（Matcher和MatchHandler）作为参数的函数。接收其他函数作为参数的函数，或返回函数的函数称为高阶函数。例如，下面的函数接收两个int型的span，以及一个Matcher和一个MatchHandler，它并行地遍历这个span，并在两个span的对应元素上调用Matcher。如果Matcher返回true，MatchHandler会被调用，第一个参数是匹配的位置，第二个和第三个参数是使得Matcher返回true的值。

```cpp
void findMatches(span<const int> values1, span<const int> values2,
  Matcher matcher, MatchHandler handler) {
  if (values1.size() != values2.size()) { return; } // must be same size
  
  for (size_t i { 0 }; i < values1.size(); ++i) {
    if (matcher(values1[i], values2[i])) {
      handler(i, values1[i], values2[i]);
    }
  }
}
```

注意，这个实现要求两个span具有相同的元素数量。要调用 `findMatches()` 函数，需要的是任何遵循一定义的Matcher类型的函数，以及一个遵顼MatchHandler类型的函数。下面是一个可能为Matcher的示例，如果两个参数相等则返回true：

```cpp
bool intEqual(int item1, int item2) { return item1 == item2; }
```

下面是一个简单打印匹配的MatchHandler的示例：

```cpp
void printMatch(size_t position, int value1, int value2) {
  cout << format("Match found at position {}({}, {})",
    position, value1, value2) << endl;
}
```

`intEqual()` 和 `printMatch()` 函数可以作为参数传递给 `findMatches()`：

```cpp
vector values1 {2, 5, 6, 9, 10, 1, 1};
vector values2 {4, 4, 2, 9, 0, 3, 1};
cout << "Calling findMatches() using intEqual():" << endl;
findMatches(values1, values2, intEqual, printMatch);
```

**output**

```
Calling findMatches() using intEqual():
Match found at position 3(9, 9)
Match found at position 6(1, 1)
```

函数指针的好处在于，findMatches是个泛型函数，它比较两个vector中并行的值。正如前面的示例所示，它基于相等来比较值。但因为它接收一个函数指针，所以它可以根据其他条件比较值。

使用函数指针的一个常见用例是获取指向动态链接库中的函数的指针。下面的示例获取指向DLL中的函数的指针。

考虑一个DLL(hardware.dll)，它有一个名为 `Connect()` 的函数，只有在需要调用 `Connect()` 时才会加载这个库，在运行时加载库时通过Windows的 `LoadLibrary()` 函数实现的：

```cpp
HMODULE lib { ::LoadLibrary("hardware.dll") };
```

调用的结果称为库句柄，如果出现错误，句柄为NULL。在加载库中的函数之前，需要知道函数的原型。假设下面是 `Connect()` 函数的原型，它返回一个整数并接收三个参数：布尔型、整型和C风格字符串：

```cpp
int __stdcall Connect(bool b, int n, const char* p);
```
 
`__stdcall` 是一个Microsoft特定的指令，用于指定函数调用约定，规定如何将函数传递给函数以及如何清理函数。

可以使用类型别名定义函数指针的名称（ConnectFunction），该函数具有要求的原型：

```cpp
using ConnectFunction = int(__stdcall*)(bool, int, const char*);
```

成功加载库并且定义了函数指针的名称后，就可以得到一个指向库函数的指针，如下所示：

```cpp
ConnectFunction connect { (ConnectFunction)::GetProcAddress(lib, "Connect") };
```

如果失败，connect会为nullptr；如果成功，可以调用加载的函数：

```cpp
connect(true, 3, "Hello world");
```

## 指向方法（和数据成员）的指针

现在考虑指向类数据成员和方法的指针。在C++中，拿到类数据成员和方法的地址获得指向它们的指针是完全合法的，但是，没有对象就不能访问非静态数据成员或调用非静态方法。类数据成员和方法的全部意义在于它们以每个对象为基础存在，因此，当想要通过指针调用方法或者访问数据成员时，必须在对象的上下文中解引用该指针。

```cpp
int (Employee::*methodPtr) () const { &Employee::getSalary };
Employee employee { "John", "Doe" };
cout << (employee.*methodPtr)() << endl;
```

第一行声明了一个名为methodPtr的变量，它的类型是指向Employee的指针的一个非静态const方法的指针，该方法不接受任何参数并返回int类型。同时，它初始化这个变量，使其指向Employee类的 `getSalary()` 方法。这种语法非常类似于声明一个简单的函数指针，除了在 `*methodPtr` 之前添加了 `Employee::` 。注意在本例中必须使用 `&`。

第三行通过methodPtr指针调用了employee对象上的 `getSalary()` 方法，注意在 `employee.*methodPtr` 周围括号使用。

如果有一个指向对象的指针，需要使用 `->*` 而不是 `.*` ，如下面的代码说明：

```cpp
int (Employee::*methodPtr) () const { &Employee::getSalary };
Employee* employee { new Employee { "John", "Doe" } };
cout << (employee->*methodPtr)() << endl;
```

使用类型别名可以让methodPtr的定义更易阅读：

```cpp
using PtrToGet = int (Employee::*) () const;
PtrToGet methodPtr { &Employee::getSalary };
Employee* employee { new Employee { "John", "Doe" } };
cout << (employee->*methodPtr)() << endl;
```

最后，可以使用auto进一步简化：

```cpp
auto methodPtr { &Employee::getSalary };
Employee* employee { new Employee { "John", "Doe" } };
cout << (employee->*methodPtr)() << endl;
```

> **注意**
>
> 可以使用 `std::men_fn()` 避免 `.*` 或 `->*` 语法，本章后面将会解释这点。

如果没有对象，就不能解引用指向非静态方法和数据成员的指针。

> **注意**
>
> C++确实允许在不使用对象的情况下解引用指向静态数据成员和静态方法的指针。

### `std::function`

在 `<functional>` 中定义的 `std::function` 函数模板是一个多态函数包装器，可以用来创建指向任何可调用对象的类型，如函数、函数对象、或者lambda表达式。std::function的实例可用作函数指针，也可用作函数实现回调函数，可以存储、赋值、移动，当然也可以执行。

函数模板的模板参数看起来和大多数模板参数有些不同，语法如下：

```cpp
std::function<R(ArgTypes...)>
```

R是返回值类型，ArgTypes是函数参数类型的逗号分隔列表。

下面演示了如何使用 `std::function` 实现函数指针，它创建一个指向函数 `func()` 的函数指针f1。一旦定义了f1，就可以用它调用 `func()` ：

```cpp
void func(int num, string_view str) {
  cout << format("func({}, {})", num, str) << endl;
}

int main() {
  function<void(int, string_view)> f1 { func };
  f1(1, "test");
}
```

由于类模板参数推导，可以简化f1的创建，如下所示：

```cpp
function f1 { func };
```

当然，在前面的实例中可以只是用auto关键字，这样就不需要指定f1的类型，下面对f1的定义同样有效，而且更短，但是编译器推导出来的f1类型是个函数指针，即 `void(*f1)(int, string_view)`，而不是 `std::function`：

```cpp
auto f1 { func };
```

因为std::function的行为像函数指针，它可以传递给接收回调的函数。前面的findMatched()示例可以使用std::function重写，唯一需要更改的是下面两个类型别名：

```cpp
// a type alias for a function accepting two integer values,
// returning true if both values are matching, false otherwise
using Matcher = function<bool(int, int)>;

// a type alias for a function to handle a match. the first
// parameter is the position of the match,
// the second and third are the values that matched
using MatchHandler = function<void(size_t, int, int)>;
```

然而从技术上讲，findMatches()不需要std::function参数来接收回调参数，相反，可以将findMatches()转换为函数模板，唯一需要更改的是删除Matcher和MatchHandler类型别名，使其成为函数模板。

```cpp
template <typename Matcher, typename MatchHandler>
void findMatcher(span<const int> values1, span<const int> values2, 
  Matcher matcher, MatchHandler handler) {...}
```

使用C++20的简化函数模板语法，findMatches()函数模板可以这样写：

```cpp
void findMatches(span<const int> values1, span<const int> values2,
  auto matcher, auto handler) {...}
```

findMatches()函数模板或简化函数模板实际上是它的推荐方法，因此在所有这些例子中，看起来 std::function不是真的有用，然而当需要将回调函数存储为类的数据成员时，std::function真的很有用。

## 函数对象

可以重载类中的函数调用运算符，以便类的对象代替函数指针。这些对象成为函数对象，或简称仿函数。使用函数对象而不是使用简单函数的好处是，函数对象可以在调用之间保持状态。

### 编写第一个函数对象

要使任何类称为函数对象，只需要重载函数调用运算符，下面是一个简单示例：

```cpp
class IsLagerThan {
public:
  explicit IsLargerThan(int value) : m_value { value } {}
  bool operator()(int value1, int value2) const {
    return value1 > m_value && value2 > m_value;
  }
private:
  int m_value;
};

int main() {
  vector values1 { 2, 500, 6, 9, 10, 101, 1 };
  vector values2 { 4, 5, 2, 9, 0, 300, 1 };
  findMatches(values1, values2, IsLargerThan { 100 }, printMatch);
}
```

注意，类中的重载函数调用运算符标记为const，在本例中这并不是严格要求的。但是对于大多数标准库算法，谓词的函数调用运算符必须是const。

### 标准库中的函数对象

C++在 `<functional>` 中提供了几个预定义的函数类，它们执行最常用的回调操作，本节会概述这些预定义仿函数。

#### 1. 算数函数对象

C++为5中二元运算符提供了仿函数类模板：plus, minus, multiplies, divides, modulus，此外还提供了一元运算符negate。这些类根据操作数的类型模板化，是实际运算符的包装器。它们接收模板类型的一或两个参数，执行运算并返回结果。下面是一个使用plus类模板的示例：

```cpp
plus<int> myPlus;
int res { myPlus(4, 5) };
cout << res << endl;
```

下面的代码片段定义了接收Operation作为参数的函数模板 `accumulateData()` 。`geometricMean()` 函数的实现调用了一个预定义的multiplies函数对象实例的 `accumulateData()` 方法：

```cpp
template <typename Iter, typename StartValue, typename Operation>
auto accumulateData(Iter begin, Iter end, StartValue startValue, Operation op) {
  auto accumulated { startValue };
  for (Iter iter { begin }; iter != end; ++iter) {
    accumulate = op(accumulate, *iter);
  }
  return accumulated;
}

double geometricMean(span<const int> values) {
  auto mult {accumulateData(cbegin(values), cend(values), 1, multiplies<int>{})};
  return pow(mult, 1.0 / values.size());  // pow() requires <cmath>
}
```

表达式 `multiplies<int>()` 创建了multiplies仿函数类模板的新对象，用int类型来实例化。

其他算数函数对象的行为类似。

#### 2. 透明运算符仿函数

C++支持透明运算符仿函数，允许你忽略模板类型参数。例如可以指定 `multiplies<>()` 为 `multiplies<void>()` 而不是 `multiplies<int>` 的缩写：

```cpp
double geometricMeanTransparent(span<const int> values) {
  auto mult { accumulateData(cbegin(values), cend(values), 1, multiplies<>{}) };
  return pow(mult, 1.0 / values.size()); 
}
```

这些透明运算符的一个重要特征是异构。也就是说，它们不仅比非透明运算符更简洁，而且具有真正的功能性优势。举例来说，下面的代码使用了透明运算符仿函数 `multiplies<>{}` ，使用了double类型的 `1.1` 作为初值，而vector中包含整数。accumulateData()计算结果也是double类型，结果会是 `6.6`。

```cpp
vector<int> values { 1, 2, 3 };
double result {accumulateData(cbegin(values), 
                              cend(values), 
                              1.1, 
                              multiplies<>{})};
```                          
如果这段代码使用了非透明运算符仿函数 `multiplies<int>()`，那么accumulateData()计算的结果就会是int型，结果会是 `6`。

```cpp
vector<int> values { 1, 2, 3 };
double result {accumulateData(cbegin(values), 
                              cend(values), 
                              1.1, 
                              multiplies<int>{}())};
```

最后，使用透明运算符而不是非透明运算符可以提高性能，下一节会讲到。

> **注意**
>
> 推荐经常使用透明运算符仿函数

#### 3. 比较函数对象

除了算数函数对象类之外，所有标准比较运算符也可作为仿函数：equal_to, not_equal_to, less, less_equal, greater, greater_equal。下面是 `priority_queue` 使用默认比较运算符 `std::less` 的示例：

```cpp
priority_queue<int> myQueue;
myQueue.push(3);
myQueue.push(4);
myQueue.push(2);
myQueue.push(1);

while(!myQueue.empty()) {
  cout << myQueue.top() << " ";
  myQueue.pop();
}
```

**output**

```
4 3 2 1
```

如上所见，队列中的元素根据less比较运算符按降序被移除，通过执行比较模板参数，可以将更改为greater。priority_queue模板的定义如下：

```cpp
template <class T, class Container = vector<T>, class Compare = less<T>>;
```

如果想使用一个greater进行升序排列的priority_queue，需要将前面例子中的priority_queue的定义修改如下：

```cpp
priority_queue<int, vector<int>, greater<>> myQueue;
```

**output**

```
1 2 3 4 
```

注意myQueue使用透明运算符 `greater<>` 定义。事实上，对于接收比较器类型的标准库容器来说，建议经常使用透明运算符，使用透明运算符相对于不透明运算符可以获得更好的性能。例如，如果 `set<string>` 使用了不透明比较器，对于给定的字符串字面量执行查询会导致产生一次没必要的拷贝，因为字符串实例必须从字符串字面量构造：

```cpp
set<string> mySet;
auto i1 { mySet.find("Key") };  // string constructed, allocates memory!
// auto i2 { mySet.find("Key"sv) }; // compilation error!
``` 

当使用透明比较器时，可以避免这种复制，这称为异构查找，示例如下：

```cpp
set<string, less<>> mySet;
auto i1 { mySet.find("Key") };  // no string constructed, no allocates memory
auto i2 { mySet.find("Key"sv) };// no string constructed, no allocates memory
```

C++20增加了对无序关联容器的透明运算符的支持。比如unordered_map和unordered_set。与有序关联容器使用透明运算符相比，对无需关联容器使用透明运算符更复杂一些。基本上，需要实现一个自定义哈希函数，包含一个定义为void的is_transparent类型别名。

```cpp
class Hasher {
public:
  using is_transparent = void;
  size_t operator()(string_view sv) const { return hash<string_view>{}(sv); }
};
```

当使用自定义哈希时，需要指定透明的 `equal_to<>` 仿函数作为键相等模板类型参数的类型，如下例：

```cpp
unordered_ser<string, Hasher, equal_to<>> mySet;
auto i1 { mySet.find("Key") };  // no string constructed, no allocates memory
auto i2 { mySet.find("Key"sv) };// no string constructed, no allocates memory
```

#### 4. 逻辑函数对象

对于3种逻辑运算，运算符!, &&, ||提供了下面的函数对象类：logic_not, logic_and和logic_or。这些逻辑操作只处理值true和false。

例如，逻辑仿函数可以用于实现allTrue()函数，该函数检查容器中的所有布尔值标志是否为true，则返回true：

```cpp
bool allTrue(const vector<bool>& flags) {
  return accumulateData(begin(flags), end(flags), true, logical_and<>{});
}
```

类似地，logic_or仿函数可以用来实现anyTrue()函数，如果容器中至少有一个布尔值为true，则返回true：

```cpp
bool allTrue(const vector<bool>& flags) {
  return accumulateData(begin(flags), end(flags), true, logical_or<>{});
}
```

> **注意**
>
> 标准库提供了 `std::all_of()` 和 `std::any_of()` 算法，它们执行和上面两个函数相同的操作，但是具有短路的邮电，所有性能更好。

#### 5. 位函数对象

C++具有与位操作运算符&, |, ^, ~相对应的函数对象bit_and, bit_or, bit_xor, bit_not。例如，这些位仿函数可以和transform()算法一起使用，对容器中所有元素执行位操作。

#### 6. 适配器函数对象

适配器函数对象允许修改函数对象、函数指针和任何本质上可调用的对象。适配器提供了对组合功能的少量的支持，即将功能组合在一起创建需要的确切行为。

##### 绑定器

绑定器可用于将可调用对象的参数绑定到某些值。为此可以使用在 `<functional>` 中的 `std::bind()` ，它允许以灵活的方式绑定可调用对象的参数，可以将参数绑定到固定值，或以不同的顺序重新排列参数。假设一个函数func()接收两个参数：

```cpp
void func(int num, string_view str) {
  cout << format("func({}, {})", num, str) << endl;
}
```

下面演示了如何使用bind()将func()的第二个参数绑定到一个固定值myString，结果存储在f1()中。这里使用auto关键字是因为C++标准没有指定bind()的返回类型，因此是特定于实现的。没有绑定到特定值的参数应指定为 `_1`, `_2`, `_3` 等，这些在 `std::placeholders` 命名空间中定义。在f1()的定义中，`_1` 指定了在调用func()时f1()的第一个参数需要放在哪里，结果f1()可以只用一个整数参数来调用：

```cpp
string myString { "abc" };
auto f1 { bind(func, placeholders::_1, myString) };
f1(16);
```

**output**

```
func(16, abc)
```

如下面的代码所示，bind()还可以用于重新排列参数。`_2` 指定在调用func()时f2()的第二个参数需要放在哪里，换句话说，f2()的绑定意味着f2()的第一个参数将成为func()的第二个参数，而f2()的第二个参数将成为func()的第一个参数：

```cpp
auto f2 { bind(func, placeholders::_2, placeholders::_1) };
f2("Test", 32);
```

**output**

```
func(32, Test)
```

`<functional>` 定义了 `std::ref()` 和 `std::cref()` 辅助函数模板，它们可用于绑定非const引用和const引用。举个例子，假设有下面函数：

```cpp
void increment(int& value) { ++value; }
```

如果这样调用这个函数，index的值会变为1：

```cpp
int index { 0 };
increment(index);
```

如果使用bind()像下面这样调用它，index的值不会增加，因为生成的是index的拷贝，这个拷贝的引用被绑定到increment()函数的第一个参数中：

```cpp
auto incr { bind(increment, index) };
incr();
```

使用 `std::ref()` 传递正确的引用就会增加index：

```cpp
auto incr { bind(increment, ref(index)) };
incr();
```

绑定参数与重载函数结合使用有一个小问题，假设虾米那有两个重载函数，一个接收整型，一个接受浮点型：

```cpp
void overloaded(int num) {}
void overloaded(float f) {}
```

如果想在这些重载函数上使用bind()，需要显示执行要绑定两个重载中的哪一个，下面代码将无法哦通过编译：

```cpp
auto f3 { bind(overloaded, placeholders::_1) };   // error
```

如果想接收浮点参数的重载函数的形参，需要使用以下语法：

```cpp
auto f4 { bind((void(*)(float))overloaded, placeholders::_1) }; // ok
```

bind()的另一个示例是使用前面定义过的findMatches()，作为MatchHandler类的方法。例如假设有下面的Handler类：

```cpp
class Handler {
public:
  void handleMatch(size_t position, int value1, int value2) {
    cout << format("Match found at position {} ({}, {})",
      position, value1, value2) << endl;
  }
};
```

如何传递handleMatch()方法作为findMatches()的最后一个参数？这里的问题在于方法总是在对象的上下文中被调用。从技术上讲，类的每个方法都有隐式的第一个参数this，因此我们的MatchHandler类型只接受3个参数，会导致签名不匹配问题。解决办法是绑定这个隐式的第一个参数，如下所示：

```cpp
Handler handler;
findMatches(values1, values2, intEqual, 
            bind(&Handler::handleMatch, &handler,
                 placeholders::_1, placeholders::_2, placeholders::_3));
```

可以使用bind()绑定标准函数对象的参数，例如可以将greater_equal的第二个参数绑定为经常与固定值进行比较：

```cpp
auto greaterEqualTo100 { bind(greater_equal<>{}, placeholders::_1, 100) };
```

> **注意**
>
> C++11之前有bind2nd()和bind1st()，这两个在C++17标准中被移除。可以使用lambda表达式或bind()来代替。

##### 否定器

`not_fn()` 是 **negator（否定器）**，类似于 **binder（绑定器）**，但它补充了可调用对象的结果。例如，如果想使用findMatches()查找不相等的值对时，可以像下面这样对intEqual()的结果应用not_fn()的negator适配器。

```cpp
findMatches(values1, values2, not_fn(intEqual), printMatch);
```

not_fn()仿函数对作为参数的可调用对象的每次调用结果进行补充。使用仿函数和适配器可能变得复杂，建议使用lambda表达式。

> **注意**
>
> `std::not_fn()` 适配器在C++17中被以内，在C++17前可以使用 `std::not1()` 和 `not2()` 适配器，然而它们在C++17中被弃用并在C++20中被移除。应该避免使用它们。

##### 调用成员函数

可能希望将指向类方法得指针当作算法得回溯，例如，假设有以下算法，它从匹配特定条件的容器中输出字符串s：

```cpp
template <typename Matcher>
void printMatchingStings(const vector<string>& strings, Matcher matcher) {
  for (const auto& string : strings) {
    if (match(string)) { cout << string << " "; }
  }
}
```

通过string的empty()方法，可以使用此算法打印所有的非空字符串s，然而如果只是把指向 `string::empty()` 的指针作为printMatchingStrings()的第二个参数传递，算法就无法知道它接收到的是指向方法的指针，而不是普通的函数指针或仿函数。调用方法指针的代码和调用普通指针的代码不同，因为前者必须在对象上调用。

C++提供了名为 `mem_fn()` 的转换函数，在将其传递给算法前，可以使用方法指针调用该函数，下面的示例演示了这一点，并将其与not_fn()结合来反转mem_fn()的结果没注意必须指定方法指针为 `&string::empty` 。

```cpp
vector <string> values { "Hello", "", "", "World", "!" };
printMatchingStrings(values, not_fn(mem_fn(&string::empty)));
```

`not_fn(mem_fn())` 生成一个函数对象，作为printMatchingStrings()的回调函数。每次调用它时，会对其参数调用empty()方法，并反转结果。

> **注意**
>
> mem_fn()并不是实现所需行为的最直观方法，建议使用lamb发表达式，以一种更具可读性方式实现它。

## lambda表达式

lambda表达式可以接收参数，可返回值，可模板化，可通过值或引用从其闭包范围内访问变量，很灵活。

### 语法

从一个简单的lambda表达式开始，下面的示例定义了一个lambda表达式，只将字符串写入控制台。lambda表达式以捕获列表 `[]` 方括号开始，后面跟着包含lambda表达式主体的花括号 `{}`，lambda表达式赋值给auto类型变量basicLambda，第二行使用普通函数调用语法执行lambda表达式。

```cpp
auto basicLambda { []{ cout << "Hello from lambda" << endl; } };
basicLambda();
```

**output**

```
Hello from lambda
```

编译器自动将任何lambda表达式转换为函数对象，也成为lambda闭包，它忽具有唯一的编译器生成的命名。在本例中，lambda表达式被转换成行为类似于下面函数对象的函数对象，注意函数调用运算符是一个const方法，返回类型是auto，方便编译器根据方法体自动推导出返回类型。

```cpp
class CompilerGeneratedName {
public:
  auto operator()() const { cout << "Hello from lambda" << endl; }
};
```

编译器生成的lambda闭包名字可以是一些奇特的名字，例如 `__Lambda_17Za`，无法知道这个名字，但幸运的是不需要知道它的名字。

lambda表达式可以接收参数，就像普通函数一样，下面是一个名为value的参数的示例：

```cpp
auto parametersLambda {
  [](int value){ cout << "The value is " << value << endl; } };
parameterLambda(42);
```

如果lambda表达式不接受任何参数，可以指定空括号或者直接省略括号。

在编译器为此lambda表达式生成的函数对象中，参数被简单地转换为重载函数调用运算符的参数：

```cpp
class ComplierGeneratedName {
public:
  auto operator()(int value) const {
    cout << "The value is " << value << endl;
  }
};
```

lambda表达式可以返回值，返回类型在箭头后面指定，称为尾返回类型。下面的例子定义了lambda表达式，它接收两个参数并返回它们的和：

```cpp
auto returningLambda { [](int a, int b) -> int { return a + b; } };
int sum { returningLambda(11, 22) };
```

返回类型可以忽略，在这种情况下，编译器根据与函数返回类型推导相同的规则来推导lambda表达式的返回类型。在前面的示例中，返回类型可以省略，如下所示：

```cpp
auto returningLambda { [](int a, int b){ return a + b; } };
int sum { returningLambda(11, 22) };
```

lambda表达式的闭包行为如下：

```cpp
class CompilerGeneratedName {
public:
  auto operator()(int a, int b) const { return a + b; }
};
```

返回类型推导会移除任何引用和const限定，例如，假设下面的Person类：

```cpp
class Person {
public:
  Person(const std::string name) : m_name { std::move(name) } {}
  const std::string& getName() const { return m_name; }
private:
  std::string m_name;
};
```

下面lambda表达式的返回类型会被推到为string，因此会生成person的名字的拷贝，即使getName()返回的是 `const string&`：

```cpp
[](const Person& person) { return person.getName(); }
```

可以将尾返回类型与 `decltype(auto)` 结合使用，使推导出来的类型与getName()的返回类型匹配：

```cpp
[](const Person& person) -> decltype(auto) { return person.getName(); }
```

以上全部的lambda表达式被称为无状态表达式，因为它们没有从闭包作用域捕获任何内容。lambda表达式可以通过从闭包作用域捕获变量而具有状态，例如，下面的lambda表达式捕获变量数据，以便在其主体中使用：

```cpp
double data { 1,23 };
auto capturingLambda { [data]{ cout << "Data = " << data << endl; } };
```

方括号部分称为lambda捕获列表，捕获变量意味着该变量在lambda表达式主体中可用，指定一个空的捕获列表 `[]` 意味着不从闭包作用域中捕获任何变量，当向前面的示例一样在捕获块中写入变量的名称时，那就是通过值捕获该变量。

捕获的变量变为lambda闭包的数据成员，值捕获的变量被复制到仿函数的数据成员中，这些数据成员与捕获的变量具有相同的const属性。在前面的capturingLambda示例中，仿函数得到一个名为data的非const数据成员，因为捕获的data是非const，编译器生成的仿函数行为如下：

```cpp
class CompilerGeneratedName {
public:
  CompilerGeneratedName(const double& d) : data { d } {}
  auto operator()() const { cout << "Data = " << data << endl; }
private:
  double data;
};
```

在下例中，仿函数获得一个名为data的const数据成员，因为捕获的变量是const。

```cpp
const double data { 1.23 };
auto capturingLambda { [data]{ cout << "Data = " << data << endl; } };
```

如前所述，lambda闭包有个重载的函数调用运算符，它被默认标记为const。在lambda表达式中按值捕获非const变量，lambda表达式中也不能修改该拷贝。如果lambda表达式指定为mutable，可以将函数调用运算符标记为非const，如下所示：

```cpp
double data { 1.23 };
auto capturingLambda {
  [data]() mutable { data *= 2; cout << "Data = " << data << endl; }
};
```

注意，如果指定了mutable，就必须要为参数指定圆括号，即使它是空参的。

可以在变量的名称前面加一个 `&` ，表示通过引用捕获它，下面的示例通过引用捕获变量data，这样lambda表达式可以在闭包作用域内直接修改data：

```cpp
double data { 1.23 };
auto capturingLambda { [&data]{ data *= 2; } };
```

当通过引用捕获变量时，必须确保引用在lambda表达式执行期间是合法的。

有两种方法从闭包作用域捕获所有的变量，称为默认捕获：

- `[=]` 按值捕获所有变量
- `[&]` 按引用捕获所有变量

> **注意**
>
> 当使用引用捕获时，通过值或引用。只有那些在lambda表达式中真正使用的变量才会被捕获，未使用的变量不会被捕获。

下面是一些捕获块的示例：

- `[&x]` 只通过引用捕获x，没有其他内容
- `[x]` 只通过值捕获x，没有其他内容
- `[=, &x, &y]` 默认通过值捕获，除了变量x和y通过引用捕获
- `[&, x]` 默认通过引用捕获，除了变量x通过值捕获
- `[&x, &x]` 非法，标识符不能重复
- `[this]` 捕获当前兑现给，在lambda表达式主体综合功能，即使不使用 `this->`，也可以访问该对象，需要确保所指向的对象在lambda表达式最后一次执行时始终存活
- `[*this]` 捕获当前对象的拷贝，这在执行lambda表达式且原始对象不再存活时非常有用
- `[=, this]` 按值捕获所有内容，并显式捕获this指针。在C++20之前， `[=]` 会隐式捕获this指针，这个在C++20中已被弃用，如果需要，请显式捕获它。

下面是一些关于捕获列表的注意事项：

- 如果默认指定了按值捕获或者按引用捕获，则不允许额外通过值或引用捕获特定的变量，例如 `[=, x]` 和 `[&, &x]` 都无效。
- 对象的数据成员不能被捕获，除非使用lambda捕获表达式。
- 当通过拷贝this指针 `[this]` 或拷贝当前对象 `[*this]` 来捕获this时，lambda表达式可以访问对象的所有公有、保护和私有数据成员和方法。

> **警告**
>
> 全局变量总是通过引用捕获，即便要求值捕获！例如，在下面的代码中，默认捕获用于按值捕获的所有内容，然而，全局变量global是按引用捕获的，在执行lambda后它的值被更改。
>
> ```cpp
> int global { 42 };
> int main() {
> auto lambda { [=] { global = 2; } };
> lambda();
> // global now has the value 2!
> }
>
> 此外，不允许像下面这样显式捕获全局变量，这回导致编译错误：
>
> ```cpp
> auto lambda { [global] { global = 2; } };
> ```
>
> 即使没有这些问题，也不推荐使用全局变量。

lambda表达式的完整语法如下：

```cpp
[capture_params] <template_params> (parameters) mutable constexpr
  noexcept_specifier attributes
  -> return_type requires {body};
```

除了捕获列表和主体，其他都是可选的。

- 捕获列表：也成为lambda导入期，可以指定如何从闭包作用域总捕获变量，并使其在lambda主体中可用。
- 模板参数（C++20）：允许编写模板化lambda表达式。
- 参数：lambda表达式的参数列表，只有当不需要任何参数且不指定mutable, constexpr, noexcept说明符、属性、返回类型或可选条款时才可以省略此列表。
- mutable：将lambda表达式标记为mutable。
- constexpr：将lambda表达式标记为constexpr，它就可以在编译期计算。
- noexcept说明符：可用于指定noexcept条款。
- 属性：可用于指定lambda表达式的属性。
- 返回类型：返回值的类型。如果省略，编译器会根据与函数返回类型推导相同的规则推导lambda表达式的返回类型。
- requires（C++20）：为lambda闭包的函数调用运算符添加模板类型约束。

> **注意**
>
> 对于不捕获任何内容的lambda表达式，编译期自动提供转换匀速那副，将lambda表达式转换为函数指针。例如，这样的lambda表达式可用于传递给接收函数指针作为参数的函数。

### lambda表达式作为参数

lambda表达式可以通过两种方式作为参数传递给函数，一种是与lambda表达式签名匹配的 `std::function` 类型的函数参数，另一种是使用模板类型参数。

例如，lambda表达式可以传递给本章签名的findMatches()函数：

```cpp
vector values1 { 2, 5, 6, 9, 10, 1, 1 };
vector values2 { 4, 4, 2, 9, 0, 3, 1 };
findMatches(values1, values2,
  [](int value1, int value2) { return value1 == value2; },
  printMatch);
```

### 泛型lambda表达式

可以为lambda表达式的参数使用自动类型推导，而不是显式地为它们指定具体类型。若要为参数指定自动类型推导，只需要简单地将类型指定为auto，类型推到规则与模板参数推导规则相同。

下面的示例定义了名为areEqual的泛型lambda表达式，这个lambda表达式用作本章前面提到过的findMatches()函数的回调：

```cpp
// define a generic lambda expression to find equal values.
auto areEqual { [](const auto& value1, const auto& value2) {
  return value1 == value2; } };
// use the generic lambda expression in a call to findMatches()
vector values1 { 2, 5, 6, 9, 10, 1, 1 };
vector values2 { 4, 4, 2, 9, 0, 3, 1 };
findMatches(values1, values2, areEqual, printMatch);
```

编译器为泛型lambda表达式生成的仿函数如下：

```cpp
class CompilerGeneratedName {
public:
  template <typename T1, typename T2>
  auto operator()(const T1& value1, const T2& value2) const 
  { return value1 == value2; }
};
```

如果findMatches()函数被更改为不仅支持整型区间，还支持其他类型，那么areEqual泛型表达式不需要做任何修改也可继续使用。

### lambda捕获表达式

### 模板化lambda表达式

### lambda表达式作为返回类型

### 不求值表达式中的lambda表达式

### 默认构造、拷贝和赋值

### lambda函数递归

因为lambda函数是匿名函数，无法简单地直接调用自身，但是我们还是可以通过一些办法实现递归

```cpp
auto gcd = [&gcd](const int& small, const int& big) {
             return x == 0 ? x : gcd(small, big % small);
           };
gcd(4, 6);
```

**output**

```
2
```

严格来说，这并不是lambda调用自身的递归，毕竟是通过调用一个包装器实现的。那有没有更加完美的办法呢？

* 以下内容仅作拓展，摘自[从零开始的简单函数式C++（四）lambda 函数](https://zhuanlan.zhihu.com/p/45430715)

想要实现 lambda 函数的递归调用，必须首先对 Y-组合子有一定的了解。简单的讲，虽然 lambda 是没有名字的，但是我们可以把他作为一个参数传递给一个更上层的函数，然后让他再调用。这时候，熟悉 Haskell 或者其他函数式语言的大家就会想起不动点组合子。也就是我们构造一个函数，返回他的不动点。

```Haskell
fix :: (a -> a) -> a
fix f = let {x = f x} in x
```

在使用这个不动点组合子之前，我们先看一下正常的函数递归计算是什么样的
```Haskell
fact :: Int -> Int
fact 0 = 1
fact n = n * fact (n - 1)
main = putStrLn $ show $ fact 5 -- => 120
```

而当我们使用 lambda 时，则需要借助不动点组合子，写成这样fix :: (a -> a) -> a
```Haskell
fix f = let {x = f x} in x
main = putStrLn $ show $ fix (\f n -> if n == 0 then 1 else n * f (n-1)) 5 -- => 120
```

简单的讲，我们需要把自己的 lambda 函数增加一个参数 f 表示这个函数自己，然后利用 Y-组合子将其自动展开到不动点，从而完成计算。

道理我都懂，C++11 能不能实现 Y-组合子呢？当然是可以的，而且非常的简单。
```CPP
template <typename F>
struct Y {
  Y(F f) : _f(f) {}
  template <typename... Args>
  auto operator()(Args&&... t) const {
    return _f(*this, std::forward<Args>(t)...);
  }
  F _f;
};

template <typename F>
Y<F> fix(F&& f) {
  return Y<F>(forward<F>(f));
}

int main() {
  auto gcd = fix(
    [](auto g, int a, int b) -> int{ return b == 0 ? a : g(b, a % b); }
  );
  std::cout << gcd(63, 105) << std::endl; // => 21
}
```

首先我们定义了一个组合子 Y 用来“拼接”函数，然后外部公开了方法 fix 来转发 lambda 函数，而内部则用变长模板来转发 lambda 函数的参数。这里注意第 27 行里，需要显式的声明 lambda 函数的返回类型，否则会导致编译报错，原因是函数参数 g 并没有一个明确的返回值。

## 调用
