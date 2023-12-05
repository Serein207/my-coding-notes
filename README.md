# My Coding Notes

文中有些许错误，不保证全部正确，部分以个人理解完成

持续更新中...

- [My Coding Notes](#my-coding-notes)
  - [Rust学习笔记](#rust学习笔记)
  - [C/C++语法学习笔记](#cc语法学习笔记)
    - [Modern C++ 目录](#modern-c-目录)
    - [C++基础语法 目录](#c基础语法-目录)
    - [C基础语法 目录](#c基础语法-目录-1)
  - [C++ Concurrency In Action Notes 学习笔记](#c-concurrency-in-action-notes-学习笔记)
  - [Effective Cpp Series 学习笔记](#effective-cpp-series-学习笔记)
    - [Effective C++](#effective-c)
    - [More Effective C++](#more-effective-c)
    - [Effective Modern C++](#effective-modern-c)
  - [STL源码剖析笔记](#stl源码剖析笔记)
  - [Java学习笔记](#java学习笔记)


## Rust学习笔记

- [01 基础语法](/rust-study-notes/01.md)
- [02 所有权](/rust-study-notes/02.md)
- [03 struct](/rust-study-notesrust/03.md)

## C/C++语法学习笔记

Modern C++ 内容主要摘自《C++20高级编程》和cppreference.com

包括语法、编程技巧、源码示例

### Modern C++ 目录

- 第I部分 C++简介
  - [Chapter1 基础语法](c-cpp/ModernCpp/Chapter01.md)
  - [Chapter2 字符串](c-cpp/ModernCpp/Chapter02.md)
  - [Chapter3 format](c-cpp/ModernCpp/Chapter03.md)
  - [Chapter4 初始化](c-cpp/ModernCpp/Chapter04.md)
  - [Chapter5 常量](c-cpp/ModernCpp/Chapter05.md)
  - [Chapter6 类型](c-cpp/ModernCpp/Chapter06.md)
  - [Chapter7 引用](c-cpp/ModernCpp/Chapter07.md)
- 第II部分 C+编码方法
  - [Chapter8 类和对象](c-cpp/ModernCpp/Chapter08.md)
  - [Chapter9 智能指针](c-cpp/ModernCpp/Chapter09.md)
  - [Chapter10 模块](c-cpp/ModernCpp/Chapter10.md)
  - [Chapter11 类模板](c-cpp/ModernCpp/Chapter11.md)
  - [Chapter12 函数模板&变量模板](c-cpp/ModernCpp/Chapter12.md)
  - [Chapter13 概念和约束](c-cpp/ModernCpp/Chapter13.md)
  - [Chapter14 异常](c-cpp/ModernCpp/Chapter14.md)
  - [Chapter15 标准库容器](c-cpp/ModernCpp/Chapter15.md)
  - [Chapter16 函数指针，函数对象，lambda表达式](c-cpp/ModernCpp/Chapter16.md)
  - [Chapter17 字符串本地化](c-cpp/ModernCpp/Chapter17.md)
  - [Chapter18 正则表达式](c-cpp/ModernCpp/Chapter18.md)
  - [Chapter19 variant, any, tuple](c-cpp/ModernCpp/Chapter19.md)
- 第III部分 C++高级特性
  - [Chapter20 高级模板](c-cpp/ModernCpp/Chapter20.md)
  - [Chapter21 模板元编程](c-cpp/ModernCpp/Chapter21.md)
  - [Chapter22 多线程](c-cpp/ModernCpp/Chapter22.md)
  - [Chapter23 互斥](c-cpp/ModernCpp/Chapter23.md)
  - [Chapter24 条件变量](c-cpp/ModernCpp/Chapter24.md)
  - [Chapter25 异步](c-cpp/ModernCpp/Chapter25.md)
  - [Chapter26 协程](c-cpp/ModernCpp/Chapter26.md)

### C++基础语法 目录

1. [命名空间&输入输出流对象&字符串&引用](c-cpp/Cpp/l_08.md)
2. [函数&内存](c-cpp/Cpp/l_09.md)
3. [类：定义&访问权限&构造与析构&深浅拷贝](c-cpp/Cpp/l_10.md)
4. [类：类对象做类成员&`static`/`const`修饰&对象模型&友元](c-cpp/Cpp/l_11.md)
5. [类：委托构造&重载（静态多态）](c-cpp/Cpp/l_12.md)
6. [类：继承](c-cpp/Cpp/l_13.md)
7. [类：动态（继承）多态](c-cpp/Cpp/l_14.md)
8. [函数高级](c-cpp/Cpp/l_15.md)
9. [模板初级](c-cpp/Cpp/l_16.md)
10. [类模板深入](c-cpp/Cpp/l_17.md)
11. [STL初识](c-cpp/Cpp/l_18.md)
12. [STL数组&基于范围的for循环](c-cpp/Cpp/l_19.md)
13. [C++文件操作](c-cpp/Cpp/l_20.md)

### C基础语法 目录

1. [lesson1变量常量](c-cpp/C/l_01.md)
2. [lesson2数组&函数](c-cpp/C/l_02.md)
3. [lesson3指针](c-cpp/C/l_03.md)
4. [lesson4字符串](c-cpp/C/l_04.md)
5. [lesson5复合类型](c-cpp/C/l_05.md)
6. [lesson6内存操作](c-cpp/C/l_06.md)
7. [lesson7文件操作](c-cpp/C/l_07.md)
8. [C文件操作函数](c-cpp/C/C文件操作.md)
9. [常用简单算法](c-cpp/C/常用算法.md)

## C++ Concurrency In Action Notes 学习笔记

- [Chapter2 线程管理](Effective-Cpp-Series-Notes/Chapter2线程管理.md)
- [Chapter3 线程间共享数据](Effective-Cpp-Series-Notes/Chapter3线程间共享数据.md)
- [Chapter4 同步并发操作](Effective-Cpp-Series-Notes/Chapter4同步并发操作.md)
- [Chapter5 C++内存模型和原子类型操作](Effective-Cpp-Series-Notes/Chapter5C++内存模型和原子类型操作.md)

## Effective Cpp Series 学习笔记

Effective C++, More Effective C++, Effective Modern C++笔记

### Effective C++

- **1 让自己习惯C++**
  - [条款01：视C++为一个语言联邦](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter1/01.md)
  - [条款02：尽量以const,enum,inline替换#define](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter1/02.md)
  - [条款03：尽可能使用const](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter1/03.md)
  - [条款04：确定对象被使用前已先被初始化](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter1/04.md)
- **2 构造/析构/赋值运算**
  - [条款05：了解C++默默编写并调用哪些函数](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter2/05.md)
  - [条款06：若不想使用编译器自动生成的函数，就该明确拒绝](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter2/06.md)
  - [条款07：为多态基类声明virtual析构函数](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter2/07.md)
  - [条款08：别让异常逃离析构函数](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter2/08.md)
  - [条款09：绝不在构造和析构过程中调用virtual函数](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter2/09.md)
  - [条款10：令operator=返回一个reference to `*this`](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter2/10.md)
  - [条款11：在operator=中处理“自我赋值”](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter2/11.md)
  - [条款12：复制对象时勿忘其每一个成分](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter2/12.md)
- **3 资源管理**
  - [条款13：以对象管理资源](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter3/13.md)
  - [条款14：在资源管理类中小心copying行为](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter3/14.md)
  - [条款15：在资源管理类中提供对原始资源的访问](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter3/15.md)
  - [条款16：成对使用new和delete时要采用相同形式](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter3/16.md)
  - [条款17：以独立语句将new对象置入智能指针](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter3/17.md)
- **4 设计与声明**
  - [条款18：让接口容易被正确使用，不易被误用](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter4/18.md)
  - [条款19：设计class犹如设计type](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter4/19.md)
  - [条款20：宁以pass-by-reference-to-const替换pass-by-value](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter4/20.md)
  - [条款21：必须返回对象时，别妄想返回其reference](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter4/21.md) 
  - [条款22：将成员变量声明为private](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter4/22.md)
  - [条款23：宁以non-member、non-friend替换member函数](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter4/23.md)
  - [条款24：若所有参数皆需类型转换，请为此采用non-member函数](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter4/24.md)
  - [条款25：考虑写一个不抛异常的swap函数](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter4/25.md)
- **5 实现**
  - [条款26：尽可能延后变量定义式的出现时间](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter5/26.md)
  - [条款27：尽量少做转型操作](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter5/27.md)
  - [条款28：避免返回handles执行对象内部成分](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter5/28.md)
  - [条款29：为“异常安全”而努力是值得的](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter5/29.md)
  - [条款30：透彻了解inline的里里外外](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter5/30.md)
  - [条款31：将文件间的编译依存关系降至最低](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter5/31.md)
- **6 继承与面向对象设计**
  - [条款32：确定你的public继承建模出is-a关系](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter6/32.md)
  - [条款33：避免遮蔽继承而来的名称](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter6/33.md)
  - [条款34：区分接口继承和实现继承](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter6/34.md)
  - [条款35：考虑virtual函数以外的其他选择](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter6/35.md)
  - [条款36：绝不重新定义继承而来的non-virtual函数](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter6/36.md)
  - [条款37：绝不重新定义继承而来的缺省参数值](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter6/37.md)
  - [条款38：通过复合建模出has-a或“根据某物实现出”](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter6/38.md)
  - [条款39：明智而谨慎地使用private继承](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter6/39.md)
  - [条款40：明智而谨慎地使用多重继承](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter6/40.md)
- **7 模板与泛型编程**
  - [条款41：了解隐式接口和编译期多态](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter7/41.md)
  - [条款42：了解typename的双重意义](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter7/42.md)
  - [条款43：学习处理模板化基类内的名称](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter7/43.md)
  - [条款44：将与参数无关的代码抽离templates](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter7/44.md)
  - [条款45：运用成员函数模板接受所有兼容类型](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter7/45.md)
  - [条款46：需要类型转换时请为模板定义非成员函数](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter7/46.md)
  - [条款47：请使用traits classes表现类型信息](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter7/47.md)
  - [条款48：认识template元编程](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter7/48.md)
- **8 定制new和delete**
  - [条款49：了解new-handler的行为](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter8/49.md)
  - [条款50：了解new和delete的合理替换时机](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter8/50.md)
  - [条款51：编写new和delete时需固守常规](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter8/51.md)
  - [条款52：写了placement new也要写placement delete](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter8/52.md)
- **9 杂项讨论**
  - [条款53：不要忽视编译器的警告](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter9/53.md)
  - [条款54：让自己熟悉包括TR1在内的标准程序库](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter9/54.md)
  - [条款55：让自己熟悉Boost](Effective-Cpp-Series-Notes/EffectiveCpp/Chapter9/55.md)

### More Effective C++

> loading...

### Effective Modern C++

- **1 类型推导**
  - [条款01：理解模板类型推导](Effective-Cpp-Series-Notes/EffectiveModernCpp/Chapter1/01.md)
  - [条款02：理解`auto`类型推导](Effective-Cpp-Series-Notes/EffectiveModernCpp/Chapter1/02.md)
  - [条款03：理解`decltype`](Effective-Cpp-Series-Notes/EffectiveModernCpp/Chapter1/03.md)
  - [条款04：掌握查看类型推导结果的方法](Effective-Cpp-Series-Notes/EffectiveModernCpp/Chapter1/04.md)
- **2 `auto`**
  - [条款05：优先使用`auto`，而非显式类型声明](Effective-Cpp-Series-Notes/EffectiveModernCpp/Chapter2/05.md)
  - [条款06：当`auto`推导的类型不符合要求时，使用带显式类型的初始化器习惯用法](Effective-Cpp-Series-Notes/EffectiveModernCpp/Chapter2/06.md)
- **3 转向现代C++**
  - [条款07：在创建对象时注意区分`()`和`{}`](Effective-Cpp-Series-Notes/EffectiveModernCpp/Chapter3/07.md)
  - [条款08：优先选用`nullptr`，而非`0`或`NULL`](Effective-Cpp-Series-Notes/EffectiveModernCpp/Chapter3/08.md)
  - [条款09：优先选用别名声明，而非`typedef`](Effective-Cpp-Series-Notes/EffectiveModernCpp/Chapter3/09.md)
  - [条款10：优先选用限定作用域的枚举类型，而非不限定作用域的枚举类型](Effective-Cpp-Series-Notes/EffectiveModernCpp/Chapter3/10.md)
  - [条款11：优先选用删除函数，而非private未定义函数](Effective-Cpp-Series-Notes/EffectiveModernCpp/Chapter3/11.md)
  - [条款12：为意在重写的函数添加`override`声明](Effective-Cpp-Series-Notes/EffectiveModernCpp/Chapter3/12.md)
  - [条款13：优先选用`const_iterator`，而非`iterator`](Effective-Cpp-Series-Notes/EffectiveModernCpp/Chapter3/13.md)
  - [条款14：只要函数不会抛出异常，就为其加上`noexcept`声明](Effective-Cpp-Series-Notes/EffectiveModernCpp/Chapter3/14.md)
  - [条款15：只要有可能使用`constexpr`，就使用它](Effective-Cpp-Series-Notes/EffectiveModernCpp/Chapter3/15.md)
  - [条款16：保证`const`成员函数的线程安全性](Effective-Cpp-Series-Notes/EffectiveModernCpp/Chapter3/16.md)
  - [条款17：理解特种成员函数的生成机制](Effective-Cpp-Series-Notes/EffectiveModernCpp/Chapter3/17.md)
- **4 智能指针**
  - [条款18：使用`std::unique_ptr`管理具备专属所有权的资源](Effective-Cpp-Series-Notes/EffectiveModernCpp/Chapter4/18.md)
  - [条款19：使用`std::shared_ptr`管理具备共享所有权的资源](Effective-Cpp-Series-Notes/EffectiveModernCpp/Chapter4/19.md)
  - [条款20：对于类似`std::shared_ptr`但有可能空悬的指针使用`std::weak_ptr`](Effective-Cpp-Series-Notes/EffectiveModernCpp/Chapter4/20.md)
  - [条款21：优先选用`std::make_unique`和`std::make_shared`，而非直接使用`new`](Effective-Cpp-Series-Notes/EffectiveModernCpp/Chapter4/21.md)
  - [条款22：使用Pimpl习惯用法时，将特殊成员函数的定义放到实现文件中](Effective-Cpp-Series-Notes/EffectiveModernCpp/Chapter4/22.md)
- **5 右值引用、移动语义和完美转发**
  - [条款23：理解`std::move`和`std::forward`](Effective-Cpp-Series-Notes/EffectiveModernCpp/Chapter5/23.md)
  - [条款24：区分万能引用和右值引用](Effective-Cpp-Series-Notes/EffectiveModernCpp/Chapter5/24.md)
  - [条款25：针对右值引用实施`std::move`，针对万能引用实施`std::forward`](Effective-Cpp-Series-Notes/EffectiveModernCpp/Chapter5/25.md)
  - [条款26：避免依万能引用类型进行重载](Effective-Cpp-Series-Notes/EffectiveModernCpp/Chapter5/26.md)
  - [条款27：熟悉依万能引用类型进行重载的替代方案](Effective-Cpp-Series-Notes/EffectiveModernCpp/Chapter5/27.md)
  - [条款28：理解引用折叠](Effective-Cpp-Series-Notes/EffectiveModernCpp/Chapter5/28.md)
  - [条款29：假定移动操作不存在、成本高、未使用](Effective-Cpp-Series-Notes/EffectiveModernCpp/Chapter5/29.md)
  - [条款30：熟悉完美转发的失败情形](Effective-Cpp-Series-Notes/EffectiveModernCpp/Chapter5/30.md)
- **6 lambda表达式**
  - [条款31：避免默认捕获模式](Effective-Cpp-Series-Notes/EffectiveModernCpp/Chapter6/31.md)
  - [条款32：适用初始化捕获将对象移入闭包](Effective-Cpp-Series-Notes/EffectiveModernCpp/Chapter6/32.md)
  - [条款33：对`auto&&`类型的形参使用`decltype`，以`std::forward`之](Effective-Cpp-Series-Notes/EffectiveModernCpp/Chapter6/33.md)
  - [条款34：优先选用lambda式，而非`std::bind`](Effective-Cpp-Series-Notes/EffectiveModernCpp/Chapter6/34.md)
- **7 并发API**
  - [条款35：优先选用基于任务而非基于线程的程序设计](Effective-Cpp-Series-Notes/EffectiveModernCpp/Chapter7/35.md)
  - [条款36：如果异步是必要的，则指定`std::launch::async`](Effective-Cpp-Series-Notes/EffectiveModernCpp/Chapter7/36.md)
  - [条款37：使用`std::thread`类型对象在所有路径皆不可`join`](Effective-Cpp-Series-Notes/EffectiveModernCpp/Chapter7/37.md)
  - [条款38：对变化多端的线程句柄析构函数行为保持关注](Effective-Cpp-Series-Notes/EffectiveModernCpp/Chapter7/38.md)
  - [条款39：考虑对一次性事件通信使用以`void`为模板类型实参的期值](Effective-Cpp-Series-Notes/EffectiveModernCpp/Chapter7/39.md)
  - [条款40：对并发使用`std::atomic`，对特种内存使用`volatile`](Effective-Cpp-Series-Notes/EffectiveModernCpp/Chapter7/40.md)
- **8 微调**
  - [条款41：针对可复制的形参，在移动成本低并且一定会被复制的前提下，考虑将其按值传递](Effective-Cpp-Series-Notes/EffectiveModernCpp/Chapter8/41.md)
  - [条款42：考虑置入而非插入](Effective-Cpp-Series-Notes/EffectiveModernCpp/Chapter8/42.md)

## STL源码剖析笔记

- 第一章 STL概论与实现版本简介
  - [1.1 STL概论](/The-Annotated-STL-Sources-Notes/Chapter1/1_1.md)
  - [1.2 STL六大组件](/The-Annotated-STL-Sources-Notes/Chapter1/1_2.md)
- 第二章 空间配置器（allocator）
  - [2.1 空间配置器的标准接口](/The-Annotated-STL-Sources-Notes/Chapter2/2_1.md)
  - [2.2 具备层次配置能力的SGI空间配置器](/The-Annotated-STL-Sources-Notes/Chapter2/2_2.md)
  - [2.3 内存基本处理工具](/The-Annotated-STL-Sources-Notes/Chapter2/2_3.md)
- 第三章 迭代器（iterator）概念与traits编程技法
  - [3.1 迭代器设计思维——STL关键所在](/The-Annotated-STL-Sources-Notes/Chapter3/3_1.md)
  - [3.2 迭代器是一种smart pointer](/The-Annotated-STL-Sources-Notes/Chapter3/3_2.md)
  - [3.3 迭代器相应类型](/The-Annotated-STL-Sources-Notes/Chapter3/3_3.md)
  - [3.4 Traits编程技法——STL源码门钥](/The-Annotated-STL-Sources-Notes/Chapter3/3_4.md)
  - [3.5 `std::iterator` 的保证](/The-Annotated-STL-Sources-Notes/Chapter3/3_5.md)
  - [3.6 iterator源码完整重列](/The-Annotated-STL-Sources-Notes/Chapter3/3_6.md)
  - [3.7 SGI STL的私房菜：`__type_traits`](/The-Annotated-STL-Sources-Notes/Chapter3/3_7.md)
- 第四章 序列式容器（sequence containers）
  - [4.1 容器的概观与分类](/The-Annotated-STL-Sources-Notes/Chapter4/4_1.md)
  - [4.2 vector](/The-Annotated-STL-Sources-Notes/Chapter4/4_2.md)
  - [4.3 list](/The-Annotated-STL-Sources-Notes/Chapter4/4_3.md)
  - [4.4 deque](/The-Annotated-STL-Sources-Notes/Chapter4/4_4.md)
  - [4.5 stack](/The-Annotated-STL-Sources-Notes/Chapter4/4_5.md)
  - [4.6 queue](/The-Annotated-STL-Sources-Notes/Chapter4/4_6.md)
  - [4.7 heap](/The-Annotated-STL-Sources-Notes/Chapter4/4_7.md)
  - [4.8 priority_queue](/The-Annotated-STL-Sources-Notes/Chapter4/4_8.md)
  - [4.9 slist](/The-Annotated-STL-Sources-Notes/Chapter4/4_9.md)
- 第五章 关联式容器（associated containers）
- 第六章 算法（algorithms）
- 第七章 仿函数或函数对象（functors, or function objects）
- 第八章 适配器（adapters）

## Java学习笔记

内容来源 《Java核心技术》（原书12版）

- [6.1 接口](/Java-Study-Notes/6_1-Java-接口.md)
- [6.2 lambda表达式](/Java-Study-Notes/6_2-Java-lambda表达式.md)
- [6.3 内部类](/Java-Study-Notes/6_3-Java-内部类.md)
- [6.4 服务加载器](/Java-Study-Notes/6_4-Java-服务加载器.md)
- [6.5 代理](/Java-Study-Notes/6_5-Java-代理.md)