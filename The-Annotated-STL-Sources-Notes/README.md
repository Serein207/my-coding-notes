# The Annotated STL Sources Notes

《STL源码剖析》笔记

> 这本书不适合C++初学者，不适合泛型技术初学者，或STL初学者，这本书也不适合带领你学习面向对象技术。

笔记基于个人理解完成，如有错误，欢迎指正。

## 目录 Content

- 第一章 STL概论与实现版本简介
  - [1.1 STL概论](Chapter1/1_1.md)
  - [1.2 STL六大组件](Chapter1/1_2.md)
- 第二章 空间配置器（allocator）
  - [2.1 空间配置器的标准接口](Chapter2/2_1.md)
  - [2.2 具备层次配置能力的SGI空间配置器](Chapter2/2_2.md)
  - [2.3 内存基本处理工具](Chapter2/2_3.md)
- 第三章 迭代器（iterator）概念与traits编程技法
  - [3.1 迭代器设计思维——STL关键所在](Chapter3/3_1.md)
  - [3.2 迭代器是一种smart pointer](Chapter3/3_2.md)
  - [3.3 迭代器相应类型](Chapter3/3_3.md)
  - [3.4 Traits编程技法——STL源码门钥](Chapter3/3_4.md)
  - [3.5 `std::iterator` 的保证](Chapter3/3_5.md)
  - [3.6 iterator源码完整重列](Chapter3/3_6.md)
  - [3.7 SGI STL的私房菜：`__type_traits`](Chapter3/3_7.md)
- 第四章 序列式容器（sequence containers）
  - [4.1 容器的概观与分类](Chapter4/4_1.md)
  - [4.2 vector](Chapter4/4_2.md)
  - [4.3 list](Chapter4/4_3.md)
  - [4.4 deque](Chapter4/4_4.md)
  - [4.5 stack](Chapter4/4_5.md)
  - [4.6 queue](Chapter4/4_6.md)
  - [4.7 heap](Chapter4/4_7.md)
  - [4.8 priority_queue](Chapter4/4_8.md)
  - [4.9 slist](Chapter4/4_9.md)
- 第五章 关联式容器（associated containers）
- 第六章 算法（algorithms）
- 第七章 仿函数或函数对象（functors, or function objects）
- 第八章 适配器（adapters）