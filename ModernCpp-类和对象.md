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

注意，既可以将 `=default` 直接放在类定义中，也可以放在实现文件中。在同一文件中，已声明的函数不能再被定义成弃置函数。

### 显式删除的默认构造函数

C++还支持显示删除的默认构造函数(explicitly deleted default constructor)。例如，可定义一个只有静态方法的类，这个类没有任何构造函数，也不想让编译器生成默认构造函数。在此情况下可以显式删除默认构造函数：

```cpp
export class MyClass {
 public:
   MyClass() = delete;
};
```

> **注意**
>
> 使用 `=delete` 弃置的函数属于显示弃置，会参与重载决议，弃置函数被使用后程序非良构。

### 显示默认或显示删除的拷贝构造函数

正如可以将编译器生成的默认构造函数设置为显式默认或显式删除，同样可以将编译器生成的拷贝构造函数设置为默认或将其删除：

```cpp
SpreadsheetCell(const SpreadsheetCell& src) = default;
```

或者

```cpp
SpreadsheetCell(const SpreadsheetCell& src) = delete;
```

通过删除拷贝构造函数，对象将无法复制，这可用于禁止按值传递对象。

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
std::cout << "a2.m_data = " << *a2.m_data <<std::endl;
```
两个代码块不可以在同一作用域进行
```cpp
A a3;
a3 = std::move(a1);                    // 调用移动赋值函数
std::cout << "a3.m_data = " << *a3.m_data <<std::endl;
```