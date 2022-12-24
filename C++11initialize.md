# C++11：初始化

---
***Contents***
- [C++11：初始化](#c11初始化)
  - [初始化列表](#初始化列表)
    - [用于内置数据类型](#用于内置数据类型)
    - [`new`表达式](#new表达式)
    - [创建对象](#创建对象)
    - [STL容器](#stl容器)
    - [函数参数](#函数参数)
  - [结构化绑定](#结构化绑定)

---
## 初始化列表
C++11丰富了`{}`的使用范围，用`{}`括起来的列表可以用于所有内置类型和自定义类型；使用初始化列表时，可以添加`=`，也可以不添加

### 用于内置数据类型
```cpp
int x = {5};
double y[5]{2.75};
short arr[5]{4, 5, 6, 7};
```
### `new`表达式
统一初始化列表也可以用于 `new` 表达式中
```cpp
int* arr = new int[4]{2,3,6,7};
```

### 创建对象
创建对象时，也可以使用大括号（不是圆括号）来调用构造函数
```cpp
class A {
 public:
   A(int a, int b) : m_a(a), m_b(b) {}
 private:
   int m_a;
   int m_b;
};

int main() {
  A a1 = {1, 2};
  A a2{3, 4};
}
```
### STL容器
**STL容器提供了`std::initializer_list`模板类作为参数的构造函数**
```cpp
std::vector<int> v1(10);        // v1初始化为10个元素
std::vector<int> v2{10};        // v2初始化为1个元素，该元素为10
std::vector<int> v3{3, 5, 8};   // v3初始化为3个元素，分别为3, 5, 8
```

### 函数参数
**将`std::initializer_list`用于常规函数的参数**

使用时包含头文件`<initializer_list>`
  
```cpp
double sum(std::initializer_list<double> i) {
  double total = 0;
  for (std::vector<int>::iterator it = i.begin(); it != i.end(); it++) {
    total += *it;
  }
  return total;
}

int main () {
  double total = sum({1.2, 3.4, 5.6});
  std::cout << total << std::endl;
}
```

---
## 结构化绑定
结构化绑定允许声明多个变量，这些变量使用数组、结构体、pair、或元组中的元素以初始化。

例如，假设有下面的数组
```cpp
std::array values{11, 22, 33};
```
可声明3个变量x、y、z，像下面这样使用数组中的3个值进行初始化。注意，必须使用 `auto` 关键字。例如，不能用 `int` 代替 `auto` 
```cpp
auto [x, y, z]{values};
```

使用结构化绑定声的变量数量必须与右侧表达式的值数量匹配

如果所有非静态成员都是公有的，也可以将结构化绑定用于结构体
```cpp
struct Point {double m_x, m_y, m_z;};
Point point;
point.m_x = 1.0;  point.m_y = 2.0;  point.m_z = 3.0;
auto [x, y, z]{point};  
```
正如最后一个例子，以下代码中将pair中的元素分解为单独的变量
```cpp
pair myPair{"hello", 5};
auto [theString, theInt]{myPair};
std::cout << theString << std::endl;
std::cout << theInt << std::endl;
```
通过 `auto&` 或 `const auto&` 代替 `auto` 还可以使用结构化绑定语法创建一组对非const的引用或const引用。

---
**edit&arrange**  Serein
