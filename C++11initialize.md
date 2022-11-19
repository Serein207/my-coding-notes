# C++11：统一初始化列表 *(initializer_list)*

---
***Contents***
- [C++11：统一初始化列表 *(initializer_list)*](#c11统一初始化列表-initializer_list)

---

C++11丰富了`{}`的使用范围，用`{}`括起来的列表可以用于所有内置类型和自定义类型；使用初始化列表时，可以添加`=`，也可以不添加


* **用于内置数据类型**
```cpp
int x = {5};
double y[5]{2.75};
short arr[5]{4, 5, 6, 7};
```
* **统一初始化列表也可以用于`new`表达式中**
```cpp
int* arr = new int[4]{2,3,6,7};
```
* **创建对象时，也可以使用大括号（不是圆括号）来调用构造函数**
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

* **STL容器提供了`std::initializer_list`模板类作为参数的构造函数**
```cpp
std::vector<int> v1(10);        // v1初始化为10个元素
std::vector<int> v2{10};        // v2初始化为1个元素，该元素为10
std::vector<int> v3{3, 5, 8};   // v3初始化为3个元素，分别为3, 5, 8
```

* **将`std::initializer_list`用于常规函数的参数**
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
**edit&arrange**  Serein
