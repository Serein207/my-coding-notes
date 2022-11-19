# C++11：类型

---
***Contents***
- [C++11：类型](#c11类型)
  - [`dynamic_cast` 运行时类型识别 *(RTTI)*](#dynamic_cast-运行时类型识别-rtti)
  - [`typeid` 运算符和 `type_info` 类](#typeid-运算符和-type_info-类)
  - [自动类型推导 `auto`](#自动类型推导-auto)
  - [自动类型推导 `decltype`](#自动类型推导-decltype)

---

## `dynamic_cast` 运行时类型识别 *(RTTI)*
构建多态时，如果需要调用派生类对象的非虚函数，则需要强转父类指针类型为目标对象派生类指针类型，但必须保证目标对象正确
```cpp
class Base {
  public:
   virtual void func1() = 0;
};

class Derived1 : public Base {
  public:
   void func1() { std::cout << "derived1" << std::endl; }
};

class Derived2 : public Base {
  public:
   void func1() { std::cout << "derived2" << std::endl; }
   void func2() { std::cout << "hello" << std::endl; }
};

class Derived3 : public Base {
  public:
   void func1() { std::cout << "derived3" << std::endl; }
};

int main() {
   Base* d2 = new Derived2;
   d2->func1();
   // d1->func2();                   // error，func2不是虚函数
   Derived2* d22 = (Derived2*)d2;    // 将基类指针转化为派生类指针
   d22->func2();
   delete d2;
}
```

`dynamic_cast`运算符用指向基类的指针来生成派生类的指针，它不能回答指针指向的是什么类对象的问题，但能回答**是否可以安全地将指针的地址复制给特定指针类型**的问题
```cpp
Derived* derived = dynamic_cast<Derived*>(base)
```
如果转换成功，`dynamic_cast`返回对象地址，否则返回`nullptr`

上述代码可以改为：
```cpp
Derived2* d22 = dynamic_cast<Derived2*>(d2); 
if(d22 != nullptr){
  d22->func2();
}
delete d2;
```

* `dynamic_cast`只适用于包含虚函数的类；
* `dynamic_cast`可以将派生类指针转化为基类指针，但这种做法没有意义，派生类指针可以直接赋值给基类指针；
* `dynamic_cast`可以用于引用，但是，没有与控制很对应的引用值，如果转换请求不正确，会出现`bad_cast`异常（C++11取消异常）；

---

## `typeid` 运算符和 `type_info` 类

```cpp
typeid(Type);
typeid(expr);
```

* `typeid`运算符返回`type_info`类（在头文件`<typeinfo>`中定义）的对象的引用

```cpp
class A {
 public:
   A() {}
};

int main() {
  int i = 3;
  int* pi = &i;
  int& ri = i;

  // 内置数据类型
  std::cout << "typeid(int)=" << typeid(int).name() << std::endl;
  std::cout << "typeid(i)=" << typeid(i).name() << std::endl;
  std::cout << "typeid(int*)=" << typeid(int*).name() << std::endl;
  std::cout << "typeid(pi)=" << typeid(pi).name() << std::endl;
  std::cout << "typeid(int&)=" << typeid(int&).name() << std::endl;
  std::cout << "typeid(ri)=" << typeid(ri).name() << std::endl;

  A a;
  A* pa = &a;
  A& ra = a;

  // 自定义数据类型
  std::cout << "typeid(A)=" << typeid(A).name() << std::endl;
  std::cout << "typeid(a)=" << typeid(a).name() << std::endl;
  std::cout << "typeid(A*)=" << typeid(A*).name() << std::endl;
  std::cout << "typeid(pa)=" << typeid(pa).name() << std::endl;
  std::cout << "typeid(A&)=" << typeid(A&).name() << std::endl;
  std::cout << "typeid(ra)=" << typeid(ra).name() << std::endl;
}
```
**output**
*gcc编译器*
```
typeid(int)=i
typeid(i)=i
typeid(int*)=Pi
typeid(pi)=Pi
typeid(int&)=i
typeid(ri)=i
typeid(A)=1A
typeid(a)=1A
typeid(A*)=P1A
typeid(pa)=P1A
typeid(A&)=1A
typeid(ra)=1A
```

`typeid`在实际开发中仅用于调试，不同编译器的输出结果不同

</br>

* `type_info`类的实现随编译器而异，但至少有`name()`成员函数，该函数返回一个字符串，通常是类名
* `type_info`重载了`==`和`!=`运算符，用于进行类型比较

```cpp
if(typeid(A) == typeid(a))
  std::cout << "a type is A" << std::endl;
if(typeid(A) == typeid(*pa))
  std::cout << "*pa type is A" << std::endl;
if(typeid(A) == typeid(ra))
  std::cout << "ra type is A" << std::endl;
```
**output**
```
a type is A
*pa type is A
ra type is A
```

***注意***
* `type_info`类的构造函数是private属性，没有拷贝构造函数，无法实例化，只能由编译器在内部实例化；
* 不建议用`name()`成员函数返回的字符串作为判断类型的依据（编译器可能会转换类型名）；
* `typeid`运算符可以用于多态场景，在运行阶段识别对象类型；
  ```cpp
  Base* derived = new Derived;
  if(typeid(Derived*) == typeid(derived)) // derived是Base*类型
    std::cout << "derived type is Derived*" <<std::endl;
  if(typeid(Derived) == typeid(*derived)) // 对derived解引用是Derived类型
    std::cout << "*derived type is Derived" << std::endl;
  ```
  **output**
  ```
  *derived type is Derived
  ```
* 假设有表达式`typeid(*ptr)`，当`ptr`是空指针时，如果`ptr`是多态类型，会引发`bad_typeid`异常。

---

## 自动类型推导 `auto`

* 在C和C++98中，`auto`关键字用于修饰变量（自动存储的局部变量）
* 在C++11中，赋予了`auto`新的含义，作为类型指示符，只是编译器在编译时推导`auto`声明的变量的数据类型
* 在Linux平台下，编译需要加`-std=c++11`参数

```cpp
auto a = 3;               // int
std::cout << "a=" << a << std::endl;
auto b = 3.3;             // double
std::cout << "b=" << b << std::endl;
auto c = "hello";         // const char*
std::cout << "c=" << c << std::endl;
```
***注意***
* `auto` 声明的变量必须在定义时初始化；
* 初始化的右值可以是具体的数值，也可以是表达式和函数的返回值
* `auto` 不能作为函数的形参类型
* `auto` 不能直接声明数组
* `auto` 不能定义类的非静态成员变量

```cpp
std::string func() {
  return "hello";
}

auto d = func();
std::cout << "d=" << d << std::endl;
```

***不要滥用 `auto`！！！！***
什么叫滥用呢？~~上面所有的用法都叫滥用~~ \^_\^

**实际开发时真正用途如下：**
* 代替冗长复杂的变量声明
* 在模板中用于声明以来模板参数的变量
* 函数模板以来模板参数的返回值
* 用于lambda表达式中

这里仅演示第一种用途，其他用途会在以后说到
```cpp
double func(int a, double b, const char* c, float d, short e, long f) {
  std::cout << " a=" << a << ",b=" << b << ",c=" << c 
            << ",d=" << d << ",e=" << e << ",f=" << f << std::endl;
  return 5.5;
}

int main() {
  double (*pf)(int, double, const char*, float, short, long);
  pf = func;
  pf(1, 2, "hello", 3, 4, 5);

  auto pf1 = func;
  pf1(1, 2, "hello", 3, 4, 5);
}
```

---

## 自动类型推导 `decltype`

* 在C++11中，**`decltype` 操作符用于查询表达式的数据类型**
```cpp
decltype(expr) var;
```
`decltype`分析表达式并得到它的类型，**不会计算表达式或执行函数**。

**`decltype` 的推导规则**
* 如果expr是没用用括号括起来的标识符，则var的类型与该标识符的类型相同，包括`const`等限定符；
* 如果expr是函数调用，则var的类型与函数返回值相同（函数不能返回`void`，但可以返回`void*`）；
* 如果expr是左值（能取地址），或者括号括起来的标识符，那么var的类型是expr的引用；
* 如果上面的条件都不满足，则var的类型与expr的类型相同。

```cpp
short a = 5;
decltype(a) da;         // short
short* pa = &a;
decltype(pa) dpa;       // short*
short& ra = a;
short b = 0;
decltype(ra) dra = b;   // short&
```

```cpp
int func() {
  std::cout << "func" << std::endl;
  return 3;
}

int main() {
  decltype(func()) da;              // int
  decltype(func) dfa;               // int dfa()
  decltype(func)* dpfa = func;      // int (*dpfa)()
  dpfa();                           // 执行func()
}
```
```cpp
decltype((a)) dra = b;               // short&
decltype((func)) drfa = func;        // int (&da)()
drfa();                              // 执行func()
```

* **`decltype`&`auto`**
两者都可以完成类型推导，但是有本质区别
```cpp
decltype(func()) da;        // 不需要初始化，不执行函数
auto aa = func();           // 需要初始化，执行函数
```

* 如果需要多次使用`decltype`，可以使用`typedef`和`using`

---

**edit & arrange**  Serein