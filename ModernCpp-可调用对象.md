# Modern C++ 可调用对象

- [Modern C++ 可调用对象](#modern-c-可调用对象)
  - [函数返回值类型后置](#函数返回值类型后置)
  - [C++可调用对象](#c可调用对象)
  - [可调用对象包装器 `std::function`](#可调用对象包装器-stdfunction)
    - [来看点有意思的](#来看点有意思的)
  - [可调用对象适配器(绑定器) `std::bind`](#可调用对象适配器绑定器-stdbind)
  - [lambda函数](#lambda函数)
    - [lambda函数递归](#lambda函数递归)
    - [lambda 能否返回一个 lambda](#lambda-能否返回一个-lambda)

## 函数返回值类型后置

在如下场景中，如果我们想返回临时变量`tmp`，该怎么办呢
```cpp
template <typename T1, typename T2>
void func(T1 x, T2 y) {
    auto tmp = x + y;
}
```
我们不知道tmp的类型到底是什么
```cpp
template <typename T1, typename T2>
??? func(T1 x, T2 y) {
    return x + y;
}
```
我们知道， `decltype`可以返回表达式类型而不执行表达式，先尝试将返回值换为 `decltype`
```cpp
template <typename T1, typename T2>
decltype(x + y) func(T1 x, T2 y) {
    return x + y;
}
```
然而，上面的代码报错了，因为在执行`decltype`时，`x`和`y`都是未定义的变量

**下面，隆重介绍，C++11后置返回值类型**
```cpp
template <typename T1, typename T2>
auto func(T1 x, T2 y) -> decltype(x + y) {
    return x + y;
}
```
语法：函数返回值定义为`auto`，后面跟箭头指向`decltype`推导的返回值类型

也许，这种写法还是不够优雅，于是C++14对`auto`进行了优化
```cpp
template <typename T1, typename T2>
auto func(T1 x, T2 y) {
    return x + y;
}
```
**优雅！**

---

## C++可调用对象

声明一个函数可以这样写
```cpp
void show(int i, const std::string& message);
```
在C++11中，我们可以用 `using` 写得更优雅
```cpp
using FUNC = void(int, const std::string&);  // 普通函数类型别名
FUNC show;                              // 声明普通函数

int main() {
  show(1, "hello");                     // 调用普通函数
}

void show(int i, const std::string& message) {
  std::cout << i << message << std::endl;
}
```
注意，不能用函数类型名直接定义函数的实体
***错误写法***
```cpp
FUNC show {
  std::cout << i << message << std::endl;
}
```
定义函数指针或引用
```cpp
void(*f_ptr)(int, const std::string&) = show;
void(&f_ref)(int, const std::string&) = show;

f_ptr(1, "hello");
f_ref(1, "hello");
```
当然，也可以优雅起来
```cpp
FUNC* f_ptr = show;
FUNC& f_ref = show;

f_ptr(1, "hello");
f_ref(1, "hello");
```

在C++中，函数是一种数据类型，函数实体被看成对象，或者叫函数对象，它可以是
* **普通函数**
* **类的静态成员函数**
* **仿函数**
* **类的非静态成员函数**
* **lambda函数**
* **可被转换为函数指针的类对象**（即通过类中的`operator()`调用全局函数，这种行为纯属无聊）

这里就先不演示其他函数了，不过要着重讲一下类的非静态成员函数，lambda函数在之后会学习到

* 类的非静态成员函数只有地址，但是只有通过类的对象才能调用它，所以C++对它做了特别处理

类的非静态成员函数只有指针类型，没有引用类型，不能饮用
```cpp
struct A {
  void show(int i, const std::string& message) {
    std::cout << i << message << std::endl;
  }
};

int main() {
  A a;
  void (A::* f_ptr)(int, const std::string&) = &A::show;      // 定义类的成员函数的指针
  (a.*f_ptr)(1, "hello");                                // 用类成员函数指针调用成员函数
}
```
上述代码展示了如何通过类成员函数指针调用类的成员函数，同样，我们也可以用`using`让代码写的更优雅
```cpp
using FUNC_PTR = void (A::*)(int, const std::string&);        // 类成员函数指针类型
FUNC_PTR f_ptr = &A::show;                               // 让类成员函数指针指向类成员函数地址
(a.*f_ptr)(1, "hello");                                  // 用类成员函数指针调用类的成员函数
```
对类成员函数取地址的`&`符号不可省略，但是普通函数可省略

---

## 可调用对象包装器 `std::function`

旨在用一种简洁，统一，优雅的方式表示六种可调用对象以方便调用，~~即对象大一统理论（我编的~~

先来把所有的可调用对象定义出来，方便接下来演示
```cpp
// 普通函数
void show(int i, const std::string& message) {
  std::cout << i << message << std::endl;
}

// 静态成员函数
struct A {
  static void show(int i, const std::string& message) {
    std::cout << i << message << std::endl;
  }
};

// 仿函数
struct B {
  operator()(int i, const std::string& message) {
    std::cout << i << message << std::endl;
  }
};

// lambda函数
auto lb = [](int i, const std::string& message) {
  std::cout << i << message << std::endl;
};

// 非静态成员函数
struct C {
  void show(int i, const std::string& message) {
    std::cout << i << message << std::endl;
  }    
};

// 可以被转化为普通函数指针的类
struct D {
  using FUNC = void (*)(int, const std::string&);
  operator FUNC() {
    return show;
  }
};
```

添加头文件 `<functional>` ，使用包装器 `std::function` 来包装他们

语法：
>```cpp
>std::function<ret_type(param)> func_ptr = func_name;
>```

* 普通函数
```cpp
std::function<void(int, const std::string&)> fn1 = show;
fn1(1, "hello");
```
使用 `using`
```cpp
using FUNC = void(int, const std::string&);
std::function<FUNC> fn1 = show;
```

可以看到，包装器的写法以及用法和函数指针非常相似，而且更直观；不同的是，`std::function` 可以调用类内成员函数

下面，我们演示一下其他可调用对象的写法（初学对于lambda函数可跳过，后面会细讲）

* 静态成员函数
```cpp
std::function<void(int, const std::string&)> fn2 = A::show;
fn2(2, "hello");
```

* 仿函数
```cpp
std::function<void(int, const std::string&)> fn3 = B();      // 也可以是实例化对象名
fn3(3, "hello");
```

* lambda函数
```cpp
std::function<void(int, const std::string&)> fn4 = lb;
fn4(4, "hello");
```

* 非静态成员函数
```cpp
std::function<void(C&, int, const std::string&)> fn5 = &C::show;
fn4(c, 5, "hello");
```
包装非静态成员函数除了需要加作用域以外，还需要在 `std::function` 参数表中添加类的引用，在实际调用时添加实例化对象名

发现这里好像不统一了，凭啥你要传入3个参数？？？ 别急，下一节会讨论这个问题

* 可以被转换为函数指针的类对象
```cpp
std::function<void(int, const std::string&)> fn6 = D();
fn6(6, "hello");
```

</br>

***注意***

`std::function` 是一个模板类

类原型：
>```cpp
>template <class _Fty>
>class function...
>```
`_Fty`是可调用对象类型，类模板中只有一个参数

* 重载了`bool` 运算符，用于判断是否包装了可调用对象
* 如果`std::function` 对象未包装可调用对象，使用`std::function` 对象将抛出 `std::bad_function_call` 异常
```cpp
std::function<void(int, const std::string&)> fx;     // fx未定义
try {
  fx(1, "hello");
}
catch (std::bad_function_call e) {
    std::cout << "std::function_call" << std::endl;
}
```

如果在使用前添加判断，则不会抛出异常
```cpp
if(fx) fx(1, "hello");              // 程序不会执行
```

### 来看点有意思的

下面，我们来尝试实现一个简单的`std::function`

原文链接： [深入浅出C++的function](https://zhuanlan.zhihu.com/p/161356621)


从实现上来说，有两种办法可以实现`std::function`：一种是通过类的多态，即通过虚表来达到多态；另一种方法是通过C语言的函数指针来实现。今天我们只介绍通过类多态的方式来实现`function`，对于通过函数指针实现的方式你可以自己去研究一下。现在我们由浅入深的来分解一下`function`。通过观察我们可以发现`function`是一个包装类，它可以接收普通函数、函数类对象(也就是实现了`()`操作符的类对象）等。它是如何做到的呢？

最简单的方式就是通过类模板。我们都知道`function`的类模板参数是可变的，但我们为了简单，所以只实现有一个参数的`function`类模板。这也符合我们的目标，只是扒一下实现原理，并不是想自己去实现它。

OK，下面我们来看看该如何定义这个类模板吧。

```cpp
template<typename R, typename Arg0>
class myfunction<R(Arg0)> {
   ...
   public:
       R operator()(Arg0 arg0){
           return ...;
       }
};
```

上面的代码定义了一个最简单的，只有一个参数的类模板。它是 `function<int(int)>` 或 `function<std::string(int)>` 等格式的类模板。这样我们在外型上与标准库中的`std::function`类似了。接下来我们需要思考一下，如何让我们自己实现的`function`可以调用不同的函数呢？从其行为上可以推理出其内部应该有一个指针，而且这个指针具有多态性。想想C++中的多态是如何实现的？通过继承和虚表对吧。所以在`function`内部应该有一个基类指针，所有传入到`function`中的函数、类函数对象等都应该是继承于该类的子类成员。除此之外，还要在`()`操作符前加`virtual`关键字，让它创建虚表。

了解了上面的原理后，下面我们就在自己的`function`中增加基类及其类的指针。代码如下：

```cpp
template<typename R, typename Arg0>
class myfunction<R(Arg0)> function {

    private:
        class __callbase {
            public:
                virtual R operator()(Arg0 arg0) = 0;
                virtual ~__callbase() {}
        };

        __callbase *base_;

        ...

    public:
        ...
        R operator()(Arg0 arg0){
            return (*__callbase)(arg0); //这里调用基类对象的()操作符
        }

};
```

上面我们就将多态中的基类实现好了，在上面的代码中最关键是的`operator()`中增加了`virtual`关键字，这样该函数就被放到了vtable中，后面就可以在子类中实现该方法了。下面我们来实现子类。

```cpp
...
class myfunction<R(Arg0)> function{
    private:
        ...

        template<typename F>
        class __callable: public __callbase {

            public:
                callable(F functor)
                    : functor(functor){}

                virtual R operator()(Arg0 arg0) {
                    return functor(arg0);
                }

            private:
                F functor;
        };

        ...
    public:
        ...
        template<typename F>
        myfunction(F f): base_(new __callable<F>(f)){
        }

        ~myfunction(){
            if(base_) {
                delete base_;
                base_ = nullptr;
            }
        }
};
```

在子类的实现中，核心点是增加指向赋值给`function`类的函数指针或函数类对象，也就是上面`__callable`类中的`F functor` 成员。该成员的类型是通过模板`template<typename F>`推导出来的。如果我们在创建`function`时传入的是函数，那么`functor`就是一个函数指针，如果我们传入的是函数类对象，则`functor`就是类对象。另外你可以发现，我分别在`myfunction`类的构造函数和`__callable`类前定义了模板F，这样当我们在`main`函数中创建`myfunction对象时`，通过类型推导就可以获到`F`的具体类型了。代码如下：

```cpp
int print(int a) {
    ...
    return 0;
}

int main(...) {
    ...
    myfunction myfunc(print);   //通过这句可以获得F类型为函数指针
}
```

有了`functor`成员后，还需要在构造`__callable`时给`functor`赋值，也就是让`functor`指向具体的函数或函数类对象。之后重载`()`操作符就可以直接调用具体的函数或函数类对象了。通过以上讲解我想你应该已经知道标准库中的`function`实现的基本原理了。当然我们这里实现的比较简陋，真正的实现还要考虑很多性能的问题，所以实现的要比这个复杂得多。另外标准库中的实现是通过函数指针来实现的而非通过C++的多态。不过我们今天实现的的`myfunction`虽然与标准库有很多不同，但原理都是类似的，对于我们理解`function`已经足够了。

---

## 可调用对象适配器(绑定器) `std::bind`

`std::bind` 模板函数是一个通用的函数适配器（绑定器），它用一个可调用对象及其参数生成一个新的可调用对象，以适应模板

添加头文件 `<functional>`

函数原型：
>```cpp
>template <class Fx, class... Args>
>function<> bind(Fx&& fx, Args&... args);
>```

* **`Fx`**：需要绑定的可调用对象
* **`args`**：绑定参数表，可以是左值，右值和参数占位符 `std::placeholders::_n` ，如果参数不是占位符，默认为值传递，`std::ref(param)`则为引用传递

`std::bind` 返回 `std::function` 的对象

```cpp
void show(int i, const std::string& message) {
  std::cout << i << message << std::endl;
}

int main() {
  std::function<void(int, const std::string&)> fn1 = show;
  std::function<void(int, const std::string&)> fn2 = bind(show, atd::placeholders::_1, std::placeholders::_2);
  fn1(1, "hello");
  fn2(2, "hello");
}
```
两种写法等价
**output**
```
1hello
2hello
```

接下来，我们换一种写法
```cpp
std::function<void(const std::string&, int)> fn3 = show;         // error
fn3(3, "hello");
```
由于param不匹配，`std::function`无法包装 `show`

我们加上`std::bind`
```cpp
std::function<void(const std::string&, int)> fn3 = bind(show, std::placeholders::_2, std::placeholders::_1);
fn3(3, "hello");
```
**output**
```
3hello
```

现在，我们理解了适配器的含义了：**如果现有函数类型与包装器要求的函数类型不同，那么可以用它对现有函数类型进行转换**

实际开发中，更常见的是要求函数类型比现有函数类型少一个参数，可以把不匹配的参数提前绑定
```cpp
std::function<void(const std::string&)> fn4 = bind(show, 4, std::placeholders::_1);
fn4("hello");
```
**output**
```
4hello
```

需要注意的是，被提前绑定的数据是以值传递的方式传入的
```cpp
int item = 4;
std::function<void(const std::string&)> fn4 = bind(show, item, std::placeholders::_1);
item = 8;
fn4("hello");
```
上述代码中，`item` 以值传递的方式传入，故实参的改变不会影响形参的值

**output**
```
4hello
```

如果想要使用引用传递，需要用 `std::ref()` 处理
```cpp
int item = 4;
std::function<void(const std::string&)> fn4 = bind(show, std::ref(item), std::placeholders::_1);
item = 8;
fn4("hello");
```
**output**
```
8hello
```

那么包装器需要的参数比现有函数所多，该怎么办呢
```cpp
std::function(int, const std::string&, int) fn5 = bind(show, std::placeholders::_1, std::placeholders::_2);
fn5(5, "hello", 100);
```
上述代码中，只需要不绑定多余的参数就可以了
**output**
```
5hello
```

下面，我们演示对于六种可调用对象的绑定方法（初学对于lambda函数可跳过，后面会细讲）

同样先来把所有的可调用对象定义出来，方便接下来演示
```cpp
// 普通函数
void show(int i, const std::string& message) {
  std::cout << i << message << std::endl;
}

// 静态成员函数
struct A {
  static void show(int i, const std::string& message) {
    std::cout << i << message << std::endl;
  }
};

// 仿函数
struct B {
  operator()(int i, const std::string& message) {
    std::cout << i << message << std::endl;
  }
};

// lambda函数
auto lb = [](int i, const std::string& message) {
  std::cout << i << message << std::endl;
};

// 非静态成员函数
struct C {
  void show(int i, const std::string& message) {
    std::cout << i << message << std::endl;
  }    
};

// 可以被转化为普通函数指针的类
struct D {
  using FUNC = void (*)(int, const std::string&);
  operator FUNC() {
    return show;
  }
};
```

* 普通函数
```cpp
std::function<void(int, const std::string&)> fn1 = std::bind(show, std::placeholders::_1, std::placeholders::_2);
fn1(1, "hello");
```

* 静态成员函数
```cpp
std::function<void(int, const std::string&)> fn2 = std::bind(A::show, std::placeholders::_1, std::placeholders::_2);
fn2(2, "hello");
```

* 仿函数
```cpp
std::function<void(int, const std::string&)> fn3 = std::bind(B(), std::placeholders::_1, std::placeholders::_2);      // 也可以是实例化对象名
fn3(3, "hello");
```

* lambda函数
```cpp
std::function<void(int, const std::string&)> fn4 = std::bind(lb, std::placeholders::_1, std::placeholders::_2);
fn4(4, "hello");
```
也可以将lambda直接替换为函数体
```cpp
std::function<void(int, const std::string&)> fn4 = std::bind(
    [](int i, const std::string& message) {
       std::cout << i << message << std::endl;
    },
    std::placeholders::_1,
    std::placeholders::_2);
```

* 非静态成员函数
```cpp
std::function<void(C&, int, const std::string&)> fn5 = std::bind(
    &C::show, 
    std::placeholders::_1, 
    std::placeholders::_2, 
    std::placeholders::_3);
fn4(c, 5, "hello");
```
适配非静态成员函数时要多适配一个参数

* 可以被转换为函数指针的类对象
```cpp
std::function<void(int, const std::string&)> fn6 = std::bind(D(), std::placeholders::_1, std::placeholders::_2);
fn6(6, "hello");
```

在实际开发中，对于非静态成员函数，用function对象调用时，不希望把对象传进去，因为这样用不了模板，所以我们可以提前绑定类对象
```cpp
C c;
std::function<void(int, const std::string&)> fn5 = std::bind(
    &C::show, 
    &c, 
    std::placeholders::_1, 
    std::placeholders::_2);
fn4(5, "hello");
```

**终于，我们将六种可调用对象的形式统一了**，~~对象大一统理论完成（我编的~~

---

## lambda函数

总算讲到lambda函数了，相信你也不会回头看之前写过的lambda函数的包装和绑定方法了，所以我会在这里完整地写出来

C++11新增lambda函数后，可以解决很多之前无法解决的问题

lambda函数的特点是：距离近，简洁，高效，功能强大 ~~和优雅（我加的~~

我们先来写一个简单的lambda函数
```cpp
[](const int& i) -> void { std::cout << i << std::endl; };
```
看上去并不够简洁...吗？下面我们来试着使用lambda函数

调用STL算法 `std::for_each` 时，我们可以向第三个参数传入普通函数或仿函数
```cpp
//普通函数
void show(const int& v) {
    std::cout << v << std::endl;
}

//仿函数
struct Show {
  void operator()(const int& v){
    std::cout << e << std::endl;
  }
};

int main() {
  std::vector<int> v = {1, 2, 3};
  std::for_each(v.begin(), v.end(), show);
  std::for_each(v.begin(), v.end(), Show());
}
```
  
~~对象大一统理论后，~~ lambda函数同样也可以作为参数传入
```cpp
std::vector<int> v = {1, 2, 3};
std::for_each(v.begin(), v.end(),
              [](const int& v) -> void {
                std::cout << v << std::endl;
              });
```
**output**
```
1
2
3
```
但是看起来还是不够简洁，没关系，接着往下看

lambda函数没有函数名，也叫匿名函数，所以它并不能简单地直接调用自身变成递归函数，不过我们也可以用`auto`给他起个名字
```cpp
auto f = [](const int& v) -> void {
            std::cout << v << std::endl;
          };
std::for_each(v.begin(), v.end(), f);
f(10);
```
**output**
```
1
2
3
10
```
命名后可以使用像普通函数一样使用lambda函数

接下来我们来细讲一下lambda各部分的功能

**语法**：
>```cpp
>[capture_list](param) specifiers_exception_attr -> ret_type{ statement };
>```

* **(param)** 代表参数列表，其中括号内为形参，和普通函数的形参一样
  * lambda函数不能有默认参数
  * 所有参数必须有参数名
  * 不支持可变参数
  * 参数表为空时可以省略 `()`
* **->ret_type** 代表lambda函数的返回类型如 `-> int`、`-> string` 等。
  * 在大多数情况下不需要，因为编译器可以推导类型
    ```cpp
    auto f = [](const int& i, const double j) {
      return i + j;
    };
    auto value = f(1, 2.0);             // auto为double类型
    ```
* **{}** 内为函数主体，和普通函数一样
* **[capture]** 代表捕获列表，与函数传参原理相同，可以访问父作用中的非静态局部变量（静态局部变量可以直接访问，无法访问全局变量）
  * **空捕获**
    ```cpp
    int i = 0;
    auto f = [](const int& i) { std::cout << i << std::endl; };
    f(i);
    ```
  * **按值捕获** 在函数内不可修改变量的值
    在`[]`中可以填入想捕获的变量名，多个变量间用`,`分隔，表示将变量以值传递的方式传入
    ```cpp
    int i = 0;
    auto f = [i] { std::cout << i << std::endl; };
    f();
    ```
    在`[]`中填入`=`，表示将父作用域的全部变量以值传递的方式传入
    ```cpp
    int i = 0, j = 1;
    auto f = [=] { std::cout << i + j << std::endl; };
    f();
    ```
    以值传递方式传入的变量不会随着实参的改变而改变
    ```cpp
    int i = 0;
    auto f = [=] { std::cout << i << std::endl; };
    i = 10;
    f();
    ```
    **output**
    ```
    0
    ```
  * **按引用捕获**
    在`[]`中可以填入想捕获的变量名的引用，多个变量间用`,`分隔，表示将变量以引用传递的方式传入
    ```cpp
    int i = 0;
    auto f = [&i] { std::cout << i << std::endl; };
    i++;
    f();
    ```
    **output**
    ```
    1
    ```
    在`[]`中填入`&`，表示将父作用域的全部变量以引用传递的方式传入
    ```cpp
    int i = 0, j = 1;
    auto f = [=] { std::cout << i + j << std::endl; };
    j++;
    f();
    ```
    **output**
    ```
    2
    ```
    以引用传递方式传入的变量随着实参或形参的改变而改变
    ```cpp
    int i = 0;
    auto f = [=] { std::cout << i << std::endl; };
    i = 10;
    f();
    ```
    **output**
    ```
    10
    ```  
  * **混合方式捕获**
    ```cpp
    int i = 0, j = 1;
    auto f = [=, &i] { std::cout << i + j << std::endl; };
    i++;
    j++;
    f();
    ```
    表示按值捕获除`i`外的全部变量，按引用捕获变量`i`
    **output**
    ```
    1
    ```
    ```cpp
    int i = 0, j = 1;
    auto f = [&, i] { std::cout << i + j << std::endl; };
    i++;
    j++;
    f();
    ```
    表示按引用捕获除`i`外的全部变量，按值捕获变量`i`
    **output**
    ```
    2
    ```
  按值全部捕获和按引用全部捕获被称为**隐式捕获** ；捕获特定变量的值或引用被称为**显示捕获**
* **specifiers_exception_attr** 代表附加说明符，一般为`mutable`、`noexcept`等
  如果要修改按值捕获的变量的值，可以添加`mutable`关键字，但是在lambda函数外部，变量不会被修改
  ```cpp
  int i = 0;
  auto f = [i]() mutable { std::cout << ++i << std::endl; };
  std::cout << i << std::endl;      // 0
  f();                              // 1
  std::cout << i << std::endl;      // 0
  ```
  **output**
  ```
  0
  1
  0
  ```

那么，上面用于接收lambda函数的变量类型，到底被`auto`推导成了什么呢？

显然，lambda函数应当是个函数，返回值类型自然是函数指针
```cpp
int i = 0;
void(*f)() = [&, i] { std::cout << i + j << std::endl; };
```
然而，它竟然是错误的！原因也很明显，lambda函数比普通函数多一个捕获列表

lambda函数的本质是一个 **匿名类中的仿函数** ，捕获列表即 **类的成员变量** ，需要记住它的本质不是函数
  
不过，lambda函数属于可调用对象，包装器 `std::function` 可以接收它

```cpp
int i = 0;
std::function<void()> f = [&, i] { std::cout << i + j << std::endl; };
f();
```
不过，为了简洁优雅的书写形式，我们还是更喜欢使用`auto`类型来接收返回值

* 大一统理论中，`std::bind` 同样可以用于lambda函数
```cpp
std::function<void(int, const std::string&)> fn4 = std::bind(lb, std::placeholders::_1, std::placeholders::_2);
fn4(4, "hello");
```
也可以将lambda直接替换为函数体
```cpp
std::function<void(int, const std::string&)> fn4 = std::bind(
    [](int i, const std::string& message) {
       std::cout << i << message << std::endl;
    },
    std::placeholders::_1,
    std::placeholders::_2);
```

从上面可以看出，lambda函数能够方便我们随时随地写函数，这便是它最大的意义

### lambda函数递归

上面说了，因为lambda函数是匿名函数，无法简单地直接调用自身，但是我们还是可以通过一些办法实现递归
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

### lambda 能否返回一个 lambda

既然诚心诚意的问了，那我就光明正大的告诉你，当然是可以的。

```CPP
int main() {
    auto addition = [](int _1, int _2) -> int { return _1 + _2; };
    auto add3 = [=](int _1) -> int { return addition(_1, 3); };
    auto addition2 = [](int _1) -> auto {
        return [_1](int _2) -> int { return _1 + _2; };
    };
    auto add5 = addition2(5);
    std::cout << addition(10, 20) << std::endl; // => 30
    std::cout << addition2(10)(20) << std::endl; // => 30
    std::cout << add3(10) << std::endl; // => 13
    std::cout << add5(10) << std::endl; // => 15
}
```

恭喜你，如果你看懂了这个代码，你已经成功了和柯里化打了个照面了。第一个 addition 是我们最常见的函数定义，而第二个 addition2 则显得不那么常见。如果只从代码层面分析，他是一个依次接受两个参数的函数。如果只接受一个参数，他会返回一个接受一个参数的函数。但是实际上是因为 addition2 首先返回一个 lambda 函数，然后如果还有第二个参数他才会返回具体的计算结果。如果对这个不是很理解的话，可以对照 python 的装饰器思考一下，lambda 函数提供了一个更为简单的打包或者修饰函数的方法。当然具体怎么用当然还是看怎么写怎么方便来。

---

**edit & arrange** Serein
