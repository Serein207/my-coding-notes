关于引用的更详细说明 [知乎链接](https://zhuanlan.zhihu.com/p/99524127)，本文部分参考该文

# C++11：引用 

---
***Contents***
- [C++11：引用](#c11引用)
  - [左值&右值](#左值右值)
  - [右值引用 *(rvalue reference)*](#右值引用-rvalue-reference)
  - [常量左值引用 *(const rvalue reference)*](#常量左值引用-const-rvalue-reference)
  - [万能引用 *(universal reference)*](#万能引用-universal-reference)
    - [来点有意思的东西](#来点有意思的东西)
  - [移动语义 `std::move`](#移动语义-stdmove)
  - [完美转发 `std::forward`](#完美转发-stdforward)
  - [引用折叠 *(reference collapsing)*](#引用折叠-reference-collapsing)
    - [再看点有意思的](#再看点有意思的)

---

## 左值&右值
* C++11中将右值拓展为 **纯右值 *(prvalue)*** 和 **将亡值 *(xvalue)***
  * **纯右值**：非引用返回的临时变量，运算表达式的结果，字面常量
  * **字符串字面量**是左值，而且是不可被更改的左值。字符串字面量并不具名，但是可以用&取地址所以也是左值。
  * **将亡值**：与右值引用相关的表达式 如：将要被移动的对象，`T&&`函数返回的值，`std::move()`的返回值，转换成`T&&`的类型的转换函数的返回值
    ```cpp
    int&& f() {
      return 3;
    }

    int main() {
      f(); 
      // The expression f() belongs to the xvalue category,
      // because f() return type is an rvalue reference to object type.
    }
    ```
    ```cpp
    static_cast<int&&>(7); 
    // The expression static_cast<int&&>(7) belongs to the xvalue 
    // category, because it is a cast to an rvalue reference to object type.
    std::move(7); 
    // std::move(7) is equivalent to static_cast<int&&>(7).
    ```

左值右值判断方法精简：
* 如果你可以对一个表达式取地址，那这个表达式就是个lvalue。
* 如果一个表达式的类型是一个lvalue reference (例如, `T&` 或 `const T&`, 等.)，那这个表达式就是一个lvalue。
* 其它情况，这个表达式就是一个rvalue。从概念上来讲(通常实际上也是这样)，rvalue对应于临时对象，例如函数返回值或者通过隐式类型转换得到的对象，大部分字面值(e.g., `10` and `5.3`)也是rvalue。

```cpp
class A() {
  public:
   int m_a;
};

A&& getTemp() {
  return A();
}

int i = 3;        // 纯右值
int j = i + 8;    // 纯右值
A aa = getTemp(); // 将亡值
getTemp().m_a;    // 将亡值
```

---

## 右值引用 *(rvalue reference)*
* C++11中增加右值引用，在C++98中的引用都称为**左值引用 *(lvalue reference)***

右值引用就是给右值取别名，新名字就是左值。如果一个prvalue被绑定到一个引用上，它的生命周期则会延长到跟这个引用变量一样长。

C++11的右值引用为了实现[移动语义](#移动语义-stdmove)

语法：`Type&& varName = right_value;`
  ```cpp
  class A() {
    public:
     int m_a = 9;
  };

  A getTemp() {
    return A();
  }

  int main() {
    int&& a = 3;     // 3是右值
    int b = 8;       // b是左值
    int&& c = b + 5; // b + 5 是右值

    A&& aa = getTemp(); // getTemp()返回值是右值

    std::cout << "a = " << a << std::endl; // 3
    std::cout << "c = " << c << std::endl; // 8
    std::cout << "aa.m_a = " << aa.m_a << std::endl; // 9

    a++;
    c++;
    aa.m_a++;

    std::cout << "a = " << a << std::endl; // 4
    std::cout << "c = " << c << std::endl; // 9
    std::cout << "aa.m_a = " << aa.m_a << std::endl; // 10
  }
  ```

* 生命周期延长可以被应用在析构函数上，当我们想要去继承某个基类的时候，这个基类往往会被声明为virtual，当不声明的话，子类便不会得到析构。如果想让这个子类对象的析构仍然是完全正常，你可以把一个没有虚析构函数的子类对象绑定到基类的引用变量上。
```cpp
class shape {
 public:
  shape() { std::cout << "shape" << std::endl; }

  // 基类没有虚析构
  ~shape() { std::cout << "~shape" << std::endl; }
};

class circle : public shape {
 public:
  circle() { std::cout << "circle" << std::endl; }

  ~circle() { std::cout << "~circle" << std::endl; }
};

class triangle : public shape {
 public:
  triangle() { std::cout << "triangle" << std::endl; }

  ~triangle() { std::cout << "~triangle" << std::endl; }
};

class rectangle : public shape {
 public:
  rectangle() { std::cout << "rectangle" << std::endl; }

  ~rectangle() { std::cout << "~rectangle" << std::endl; }
};

class result {
 public:
  result() { puts("result()"); }

  ~result() { puts("~result()"); }
};

result process_shape(const shape& shape1, const shape& shape2) {
  puts("process_shape()");
  return result();
}

int main() {
  result&& r = process_shape(circle(), triangle());
}
```
**output:**
```
shape
triangle
shape
circle
process_shape()
result()
~circle
~shape
~triangle
~shape
~result()
```

---

## 常量左值引用 *(const rvalue reference)*
* 常量左值引用，可以绑定左值和右值，但不能更改引用的值
* 非常量左值引用只能绑定左值，右值引用只能绑定右值
```cpp
int a = 0;
const int& b = a; // 绑定左值

const int& c = 1; // 绑定右值
```

---

## 万能引用 *(universal reference)* 
也称转发引用或通用引用

> If a variable or parameter is declared to have type T&& for some deduced type T, that variable or parameter is a universal reference.
>如果一个变量或者参数被声明为**T&&**，其中**T是被推导的类型**，那这个变量或者参数就是一个universal reference。

在实践当中，几乎所有的universal references都是函数模板的参数。因为`auto`声明的变量的类型推导规则本质上和模板是一样的，所以使用`auto`的时候你也可能得到一个universal references。
* 如果用来初始化universal reference的表达式是一个左值，那么universal reference就变成lvalue reference。
* 如果用来初始化universal reference的表达式是一个右值，那么universal reference就变成rvalue reference。

```cpp
template<typename T>
void f(T&& param); 
int a;
f(a);   // 传入左值,那么上述的T&& 就是lvalue reference,也就是左值引用绑定到了左值
f(1);   // 传入右值,那么上述的T&& 就是rvalue reference,也就是右值引用绑定到了左值
```
```cpp
std::vector<int> v;
...
auto&& val = v[0];    // val becomes an lvalue reference
```
声明引用的时候必须用 `T&&` 的形式才能获得一个universal reference。这个一个很重要的信息。再看看这段代码:
```cpp
template<typename T>
void f(std::vector<T>&& param);       // “&&” means rvalue reference
```
这里，我们同时有类型推导和一个带 `&&` 的参数，但是参数确不具有 `T&&` 的形式，而是 `std::vector<T>&&`。其结果就是，参数就只是一个普通的rvalue reference，而不是universal reference。 Universal references只以 `T&&` 的形式出现！即便是仅仅加一个`const`限定符都会使得 `&&` 不再被解释为universal reference:
```cpp
template<typename T>
void f(const T&& param);               // “&&” means rvalue reference
```

</br>

### 来点有意思的东西

有的时候你可以在函数模板的声明中看到`T&&`，但却没有发生类型推导。来看下`std::vector`的 `push_back` 函数
```cpp
template <class T, class Allocator = allocator<T> >
class vector {
 public:
  ...
  void push_back(T&& x);       // fully specified parameter type ⇒ no type deduction;
  ...                          // && ≡ rvalue reference
};
```
这里, `T` 是模板参数, 并且`push_back`接受一个 `T&&`, 但是这个参数却不是universal reference! 这怎么可能?

如果我们看看`push_back`在类外部是如何声明的，这个问题的答案就很清楚了。我会假装`std::vector`的 `Allocator` 参数不存在，因为它和我们的讨论无关。我们来看看没`Allocator`参数的`std::vector::push_back`:
```cpp
template <class T>
void vector<T>::push_back(T&& x);
push_back不能离开std::vector<T>这个类而独立存在。但如果我们有了一个叫做std::vector<T>的类，我们就已经知道了T是什么东西，那就没必要推导T。
```
举个例子可能会更好。如果我这么写:

```cpp
Widget makeWidget();             // factory function for Widget
std::vector<Widget> vw;
...
Widget w;
vw.push_back(makeWidget());      // create Widget from factory, add it to vw
```

代码中对 `push_back` 的使用会让编译器实例化类 `std::vector<Widget>` 相应的函数。这个 `push_back` 的声明看起来像这样:
```cpp
void std::vector<Widget>::push_back(Widget&& x);
```
看到了没? 一旦我们知道了类是 `std::vector<Widget>`，`push_back`的参数类型就完全确定了: 就是`Widget&&`。这里完全不需要进行任何的类型推导。

对比下 `std::vector` 的`emplace_back`，它看起来是这样的:
```cpp
template <class T, class Allocator = allocator<T> >
class vector {
public:
    ...
    template <class... Args>
    void emplace_back(Args&&... args); // deduced parameter types ⇒ type deduction;
    ...                                // && ≡ universal references
};
```
`emplace_back` 看起来需要多个参数(`Args`和`args`的声明当中都有...)，但重点是每一个参数的类型都需要进行推导。函数的模板参数 `Args` 和类的模板参数`T`无关，所以即使我知道这个类具体是什么，比如说，`std::vector<Widget>`，但我们还是不知道`emplace_back`的参数类型是什么。

我们看下在类`std::vector<Widget>`外面声明的 `emplace_back` 会更清楚的表明这一点 (我会继续忽略 `Allocator` 参数):
```cpp
template<class... Args>
void std::vector<Widget>::emplace_back(Args&&... args);
```

---

## 移动语义 `std::move`
* 拷贝堆区对象需要重写拷贝构造函数和赋值函数，实现深拷贝
* 如果堆区源对象是临时对象（右值），深拷贝会造成无意义的内存申请和释放操作
* C++11的移动语义可以直接使用源对象，可以提高效率。
  
移动语义需要的两个函数
* 移动构造函数
  `className(className&& object) {}`
* 移动赋值函数
  `className& operator=(className&& object) {}`

普通深拷贝
```cpp
class A {
  public:
   int* m_data = nullptr;   // 数据成员指向堆区指针

   A() = default;

   void alloc() {
   try {
     this->m_data = new int();   // 分配内存
   } catch (bad_alloc) {
     alloc();
   }
     memset(this->m_data, 0, sizeof(int));   //初始化已分配内存
   }

   A(const A& a) {
     std::cout << "copy construction" << std::endl;
     if(this->m_data == nullptr) alloc();
     memcpy(this->m_data, a.m_data, sizeof(int));   // 拷贝源对象数据
   }

   A& operator=(const A& a) {
     std::cout << "operator=" << std::endl;
     if(this == &a) return *this;   // 避免自我赋值
     if(this->m_data == nullptr) alloc();
     memcpy(this->m_data, a.m_data, sizeof(int));
     return *this;
   }

   ~A() {
      if(this->m_data != nullptr) {
        delete m_data;
        m_data = nullptr;
      }
   }
}

int main() {
    A a1;
    a1.malloc();
    *a1.m_data = 3;
    std::cout << "a1.m_data = " << *a1.m_data << std::endl;

    A a2 = a1;      //调用拷贝构造函数
    std::cout << "a2.m_data = " << *a2.m_data << std::endl;

    A a3;
    a3 = a1;        //调用赋值函数
    std::cout << "a3.m_data = " << *a3.m_data << std::endl;

}
```
C++11中将拷贝数据称为拷贝语义，移动语义则是不拷贝语义而修改指针指向

增加构造函数和赋值函数的重载版本，使得传入对象为右值时优先调用移动构造和移动赋值函数

* 移动构造函数
```cpp
A(A&& a) {   //需要操作被转移对象的指针，所以不能使用const
  std::cout << "copy construction" << std::endl;
  if(this == &a) return *this;   // 避免自我赋值
  if(this->m_data != nullptr) delete this->m_data;  // 如果已分配内存，则先释放
  this->m_data = a.m_data;                          // 把资源从对象转移过来
  a.m_data = nullptr;                               // 把源对象指针置空
}
```
* 移动赋值函数
```cpp
A& operator=(A&& a) {
  std::cout << "operator=" << std::endl;
  if(this == &a) return *this;
  if(this->m_data != nullptr) delete this->m_data;
  this->m_data = a.m_data;
  a.m_data = nullptr;
}
```
* 调用移动语义
```cpp
auto f = [] { A a; a.alloc(); *a.m_data; return a; }; //返回A类对象的lambda函数
A a1 = f();                                           // lambda函数返回prvalue，
                                                      // 将调用移动构造函数
std::cout << "a2.m_data = " << *a2.m_data <<std::endl;

A a2 = f();
A a3 = a2;                                            // 将调用移动构造函数
std::cout << "a3.m_data = " << *a3.m_data <<std::endl;
```

</br>

* 如果想对一个lvalue使用移动语义，C++11提供了 `std::move()` ，将lvalue转化为rvalue
* lvalue被move后，不会立刻析构，只有离开自己作用域时才会析构，继续使用左值中的资源会报错
* 如果没有提供移动构造/赋值函数，编译器会调用拷贝构造/赋值函数
* C++11中所有容器都已经实现移动语义
* 移动语义对于拥有资源的对象有效，对于基本类型没有意义
```cpp
A a2 = std::move(a1);                  // 调用移动构造函数
std::cout << "a1.m_data = " << *a1.m_data <<std::endl;
```
两个代码块不可以在同一作用域进行
```cpp
A a3;
a3 = std::move(a1);                    // 调用移动赋值函数
std::cout << "a3.m_data = " << *a3.m_data <<std::endl;
```

</br>

* 一个表达式的 **左值性 *(lvalueness)*** 或者 **右值性 *(rvalueness)*** 和它的类型无关。

来看下 `int`。可以有lvalue的`int` (e.g., 声明为`int`的变量)，还有rvalue的`int` (e.g., 字面值`10`)。用户定义类型`Widget`等等也是一样的。

一个`Widget`对象可以是lvalue(e.g., `Widget` 变量) 或者是rvalue (e.g., 创建`Widget`的工程函数的返回值)。

表达式的类型不会告诉你它到底是个lvalue还是rvalue。因为表达式的 lvalueness 或 rvalueness 独立于它的类型，我们就可以有一个 lvalue，但它的类型确是 rvalue reference，也可以有一个 rvalue reference 类型的 rvalue :

```cpp
Widget makeWidget();                       
// factory function for Widget
 
Widget&& var1 = makeWidget()               
// var1 is an lvalue, but its type is rvalue reference (to Widget)
 
Widget var2 = static_cast<Widget&&>(var1); 
// the cast expression yields an rvalue, but its type is rvalue reference  (to Widget)
```

`var1`类别是左值，但它的类型是右值引用。`static_cast<Widget&&>(var1)` 表达式是个右值，但它的类型是右值引用。

把 lvalue (例如 `var1`) 转换成 rvalue 比较常规的方式是对它们调用`std::move`，所以 `var2` 可以像这样定义:

```cpp
Widget var2 = std::move(var1);             // equivalent to above
```

我最初的代码里使用 `static_cast` 仅仅是为了显示的说明这个表达式的类型是个rvalue reference (`Widget&&`)。rvalue reference 类型的具名变量和参数是 lvalue。(你可以对他们取地址。)

我们再来看下前面提到的 `Widget` 和 `Gadget` 模板:
```cpp
template<typename T>
class Widget {
    ...
    Widget(Widget&& rhs);        // rhs’s type is rvalue reference,
    ...                          // but rhs itself is an lvalue
};
 
template<typename T1>
class Gadget {
    ...
    template <typename T2>
    Gadget(T2&& rhs);            
    // rhs is a universal reference whose type will ventually become an
    // rvalue reference or an lvalue reference, but rhs itself is an lvalue
    ...
};
```

在 Widget 的构造函数当中, `rhs` 是一个rvalue reference，前面提到，右值引用只能被绑定到右值上，所以我们知道它被绑定到了一个rvalue上面(i.e., 因此我们需要传递了一个rvalue给它)， 但是 `rhs` 本身是一个 lvalue，所以，当我们想要用到这个被绑定在 `rhs` 上的rvalue 的 rvalueness 的时候，我们就需要把 `rhs` 转换回一个rvalue。之所以我们想要这么做，是因为我们想将它作为一个移动操作的source，这就是为什么我们用 `std::move` 将它转换回一个 rvalue。

类似地，`Gadget` 构造函数当中的`rhs` 是一个 universal reference,，所以它可能绑定到一个 lvalue 或者 rvalue 上，但是无论它被绑定到什么东西上，`rhs` 本身还是一个 lvalue。

如果它被绑定到一个 rvalue 并且我们想利用这个rvalue 的 rvalueness， 我们就要重新将 `rhs` 转换回一个rvalue。如果它被绑定到一个lvalue上，当然我们就不想把它当做 rvalue。

一个绑定到universal reference上的对象可能具有 lvalueness 或者 rvalueness，正是因为有这种二义性，所以催生了`std::forward`: 如果一个本身是 lvalue 的 universal reference 如果绑定在了一个 rvalue 上面，就把它重新转换为rvalue。函数的名字 (“forward”) 的意思就是，我们希望在传递参数的时候，可以保存参数原来的lvalueness 或 rvalueness，即是说把参数转发给另一个函数。

---

## 完美转发 `std::forward`

在函数模板中，可以将自己的参数“完美地”转发给其他函数，即准确转发参数的值和左右值属性
* 能否实现完美转发，决定了该参数在传递过程中用的是拷贝语义还是移动语义

以下实现方式中，`func2()`可以调用两个重载版本，`func1()`无法调用rvalue重载版本
```cpp
void func1(int& i) {         // 参数为lvalue
  std::cout << "lvalue" << i << std::endl;
}

void func1(int&& i) {         // 参数为rvalue
  std::cout << "rvalue" << i << std::endl;
}

void func2(int& i) {          // 参数为lvalue
  func1(i);
}

void func2(int&& i) {         // 参数为rvalue
  func1(i);
}

int main() {
  int i = 3;
  func2(i);                   // 调用lvalue
  func2(8);                   // 调用rvalue
}
```
**output**
```
lvalue
lvalue
```
怎么解决`func2`无法调用重载版本的问题呢

* 在`func2(int&& i)`中添加`std::move()`
```cpp
void func2(int&& i) {         // 参数为rvalue
  func1(std::move(i));
}
```

* 将`func2()`改成模板参数写法
```cpp
template <typename T>
void func2(T& i) {            // 参数为lvalue
  func1(i);
}

template <typename T>
void func2(T&& i) {           // 参数为rvalue
  func1(std::move(i));
}
```

* C++11支持完美转发，提供以下方案
  * 如果类模板中（包括类模板和函数模板）函数的参数为`T&&`类型，则为万能引用（既可以接受左值引用，又可以接受右值引用）
  * 提供模板函数`std::forward<T>()`，用于转发参数，转发后保留参数的左右值类型
```cpp
template<typename T>
void func(T&& i) {
  func1(std::forward<T>(i));
}

int main() {
  int i = 3;
  func(i);
  func(8);
}
```
**output**
```
lvalue
rvalue
```

## 引用折叠 *(reference collapsing)*

*我们即将深入探讨 universal reference 的实现原理*

C++11中一些构造会弄出引用的引用，而C++不允许引用的引用。
```cpp
Widget w1;
...
Widget& & w2 = w1;      // error! No such thing as "reference to reference"
```

在对一个 universal reference 的模板参数进行类型推导时候，同一个类型的 lvalue 和 rvalue 被推导为稍微有些不同的类型。具体来说，类型`T`的lvalue被推导为`T&`(i.e., lvalue reference to `T`)，而类型T的 rvalue 被推导为 `T`。(**注意，虽然 lvalue 会被推导为lvalue reference，但 rvalue 却不会被推导为 rvalue references!**) 

我们来看下分别用rvalue和lvalue来调用一个接受universal reference的模板函数时会发生什么:
```cpp
template<typename T>
void f(T&& param);

...
 
int x;
 
...
 
f(10);                           // invoke f on rvalue
f(x);                            // invoke f on lvalue
```
当用rvalue `10`调用 `f` 的时候, `T` 被推导为 `int`，实例化的 `f` 看起来像这样:
```cpp
void f(int&& param);             // f instantiated from rvalue
```

这里一切都OK。但是当我们用lvalue `x` 来调用 `f` 的时候，`T` 被推导为 `int&`，而实例化的 `f` 就包含了一个引用的引用:
```cpp
void f(int& && param);           // initial instantiation of f with lvalue
```

因为这里出现了引用的引用，这实例化的代码乍一看好像不合法，但是像– `f(x)` –这么写代码是完全合理的。为了避免编译器对这个代码报错，C++11引入了一个叫做 **引用折叠 *(reference collapsing)*** 的规则来处理某些像模板实例化这种情况下带来的"引用的引用"的问题。

因为有两种类型的引用 (lvalue references 和 rvalue references)，那"引用的引用"就有四种可能的组合: 
  1. lvalue reference to lvalue reference
  2. lvalue reference to rvalue reference
  3. rvalue reference to lvalue reference
  4. rvalue reference to rvalue reference

**引用折叠只有两条规则:**

> * 一个 rvalue reference to an rvalue reference 会变成 (“折叠为”) 一个 rvalue reference.
> * 所有其他种类的"引用的引用" (i.e., 组合当中含有lvalue reference) 都会折叠为 lvalue reference.

在用lvalue实例化 f 时，应用这两条规则，会生成下面的合法代码，编译器就是这样处理这个函数调用的:
```cpp
void f(int& param);              // instantiation of f with lvalue after reference collapsing
```
上面的内容精确的说明了一个 universal reference 是如何在经过类型推导和引用折叠之后，可以变为一个 lvalue reference的。
实际上，**universal reference 其实只是一个身处于引用折叠背景下的rvalue reference**

</br>

### 再看点有意思的

当一个变量本身的类型是引用类型的时候，这里就有点难搞了。这种情况下，类型当中所带的引用就被忽略了。例如:

```cpp
int x;
 
...
 
int&& r1 = 10;                   // r1’s type is int&&
 
int& r2 = x;                     // r2’s type is int&
```

* 在调用模板函数 `f` 的时候 `r1` 和 `r2` 的类型都被当做 int。这个扒掉引用的行为，和"universal references 在类型推导期间，lvalue 被推导为 `T&` ，rvalue 被推导为`T` 这条规则无关。所以，这么调用模板函数的时候:

```cpp
template<typename T>
void f(T &&param) {
  static_assert(std::is_lvalue_reference<T>::value, "T& is not lvalue reference");
  std::cout << "T& is lvalue reference" << std::endl;
}

int main() {
  int x;
  int&& r1 = 10;
  int& r2 = x;
  f(r1);
  f(r2);
}
```
**output:**
```
T& is lvalue reference
T& is lvalue reference
```

`r1` 和`r2` 的类型都被推导为 `int&`。这是为啥呢?

首先，`r1` 和 `r2` 的引用部分被去掉了(留下的只是 `int`)。然后，因为它们都是 lvalue 所以当调用 `f`，对 universal reference 参数进行类型推导的时候，得到的类型都是`int&`。

我前面已经说过，引用折叠只发生在“像是模板实例化这样的场景当中”。 声明`auto`变量是另一个这样的场景。推导一个universal reference的 `auto` 变量的类型，在本质上和推导universal reference的函数模板参数是一样的，所以 **类型 `T` 的lvalue被推导为 `T&`，类型 `T` 的rvalue被推导为 `T`** 。我们再来看一下本文开头的实例代码:
```cpp
Widget&& var1 = someWidget;      // var1 is of type Widget&& (no use of auto here)
 
auto&& var2 = var1;              // var2 is of type Widget& (see below)
```
`var1` 的类型是 `Widget&&`，但是它的 reference-ness 在推导 `var2` 类型的时候被忽略了;`var1` 这时候就被当做 `Widget`。

因为它是个lvalue，所以初始化一个universal reference(`var2`)的时候，`var1` 的类型就被推导成`Widget&`。在 var2 的定义当中将 `auto` 替换成`Widget&` 会生成下面的非法代码:

```cpp
Widget& && var2 = var1;          // note reference-to-reference
```

而在引用折叠之后，就变成了:

```cpp
Widget& var2 = var1;             // var2 is of type Widget&
```

* 还有第三种发生引用折叠的场景，就是形成和使用 `typedef` 的时候。看一下这样一个类模板，
```cpp
template<typename T>
class Widget {
    typedef T& LvalueRefType;
    ...
};
int main() {
    Widget<int&> w;
}
```

根据引用折叠的规则：
> * 一个 rvalue reference to an rvalue reference 会变成 (“折叠为”) 一个 rvalue reference.
> * 所有其他种类的"引用的引用" (i.e., 组合当中含有lvalue reference) 都会折叠为 lvalue reference.

我们知道T会被推导为lvalue reference，因此结果肯定是lvalue reference,对应于上述规则，我们来通过代码验证。
```cpp
template<typename T>
class Widget {
  typedef T& LvalueRefType;
  typedef T&& RvalueRefType;
 public:
  void judge() {
    static_assert(std::is_lvalue_reference<LvalueRefType>::value, "LvalueRefType& is lvalue reference");
    static_assert(std::is_lvalue_reference<RvalueRefType>::value, "RvalueRefType& is lvalue reference");
    std::cout << "LvalueRefType and RvalueRefType is lvalue reference" << std::endl;
  }
};
int main() {
  Widget<int&> w;
}
```
**output:**
```
LvalueRefType and RvalueRefType is lvalue reference
```

如果我们在应用引用的上下文中使用这个typedef，例如:
```cpp
void f(Widget<int&>::LvalueRefType&& param);
```
在对 `typedef` 扩展之后会产生非法代码:
```cpp
void f(int& && param);
```
但引用折叠这时候又插了一脚进来，所以最终的声明会是这样:
```cpp
void f(int& param);
```

* 最后还有一种场景会有引用折叠发生，就是使用 `decltype`。和模板和 `auto` 一样，`decltype `对表达式进行类型推导时候可能会返回 `T` 或者 `T&`，然后`decltype` 会应用 C++11 的引用折叠规则。

好吧， `decltype` 的类型推导规则其实和模板或者 `auto` 的类型推导不一样,后面我会具体说明他的用法。但是我们需要注意这样一个区别，即 `decltype` 对一个具名的、非引用类型的变量，会推导为类型 `T` (i.e., 一个非引用类型)，在相同条件下，模板和 `auto` 却会推导出 `T&`。

还有一个重要的区别就是`decltype` 进行类型推导只依赖于 `decltype` 的表达式; 用来对变量进行初始化的表达式的类型(如果有的话)会被忽略。因此:
```cpp
Widget w1, w2;
 
auto&& v1 = w1;         
 
decltype(w1)&& v2 = w2; 
```

`v1`本身是左值，根据`auto&&`知道为万能引用，因此`v1`被推导为指向`w1`的左值引用。

`w2`是左值，`decltype(w1)`推导为`Widget`而不发生引用折叠，因此`v2`为右值引用，根据右值引用只能绑定到右值，这里却给了一个左值，因此不能编译！

* 对于` template <typename T> foo(T&&)`这样的代码。

如果传递过去的参数是左值，`T` 的推导结果是左值引用，那 `T&&` 的结果仍然是左值引用——即 `T& && `坍缩成了`T&`
如果传递过去的参数是右值，T 的推导结果是参数的类型本身。那 `T&&` 的结果自然就是一个右值引用。
例如：
```cpp
void foo(const shape&)
{
	puts("foo(const shape&)");
}
void foo(shape&&)
{
	puts("foo(shape&&)");
}
void bar(const shape& s)
{
	puts("bar(const shape&)");
	foo(s);
}
void bar(shape&& s)
{
	puts("bar(shape&&)");
	foo(s);
}
int main()
{
	bar(circle());
}
```
**output**
```
bar(shape&&)
foo(const shape&）
```

`bar`中传入的是右值，调用`bar`的`&&`重载函数，但是对于`void bar(shape&& s)`来说，`s`本身是一个lvalue，所以在`foo(s)`后，仍旧调用的是`&`重载函数。

如果想要调用`foo(shape&&)`，可以：
```cpp
foo(std::move(s))
```
或者：
```cpp
foo(static_cast<shape&&>(s))
```
再考虑下面这个例子：
```cpp
void foo(const shape&) {
	puts("foo(const shape&)");
}
void foo(shape&&) {
	puts("foo(shape&&)");
}
template <typename T>
void bar(T&& s) {
	foo(std::forward<T>(s));
}
int main() {
    circle temp;
    bar(temp);
    bar(circle());
}
```
**output**
```
foo(const shape&)
foo(shape&&)
```


上面提到过一个绑定到universal reference上的对象可能具有 lvalueness 或者 rvalueness，正是因为有这种二义性,所以催生了`std::forward`: 如果一个本身是 lvalue 的 universal reference 如果绑定在了一个 rvalue 上面，就把它重新转换为rvalue。函数的名字 (“forward”) 的意思就是，我们希望在传递参数的时候，可以保存参数原来的lvalueness 或 rvalueness，即是说把参数转发给另一个函数。

因为在 `T` 是模板参数时，`T&&` 的作用主要是保持值类别进行转发，它有个名字就叫“转发引用”（forwarding reference）。因为既可以是左值引用，也可以是右值引用，它也曾经被叫做“万能引用”（universal reference）。

---

**edit & arrange**  Serein

