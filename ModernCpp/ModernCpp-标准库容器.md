# Modern C++ 标准库容器

> 本章你将看到现代C++中新增的容器介绍。

- [Modern C++ 标准库容器](#modern-c-标准库容器)
  - [`span`](#span)


## `span`

假设使用这个函数打印一个vector中的内容：

```cpp
void print(const vector<int>& values) {
  for (const auto& value: values) { cout << value << " "; }
  cout << endl;
}
```

假设还想打印C风格数组的内容，一种选择是重载 `print()` 函数，以接受指向数组第一个元素的指针，以及要打印的元素数量：

```cpp
void print(const int values[], size_t count) {
  for (size_t i { 0 }; i < count; i++) { cout << values[i] << " "; }
  cout << endl;
}
```

如果还想打印 `std::array`，那么可以提供第三个重载，但是函数的参数类型是什么？对于 `std::array`，必须指定array中的元素类型和数量作为模板参数。事情似乎变得越来越复杂了。

`std::span` 在C++20中引入并在 `<span>` 中定义，它允许编写单个函数来处理vector、C风格数组和任意大小的 `std::array`。下面是使用span的print函数实现：

```cpp
void print(span<int> values) {
  for (const auto& value : values) { cout << value << " "; }
  cout << endl;
}
```

注意，span的复制成本很低。它基本上只包含一个指向序列第一个元素的指针和一些元素。span永远不会复制数据。因此，它通常是通过值传递的。

有几个用于创建span的构造函数。例如，可以创建一个包含给定vector、std::array或C风格数组的所有元素的数组。还可以通过传递第一个元素的地址和想要在span中包含的元素的数量，来创建一个只包含部分容器元素的span。

可以使用 `subspan()` 方法从现有的span创建子视图。它的第一个参数是span中的偏移量，第二个参数是包含在子视图中的元素数量。还有两个名为 `first()` 和 `last()` 的附加方法，分别返回前n个元素或后n个元素的span子视图。

span有类似于vector和array的方法：`begin()` `end()` `rbegin()` `rend()` `front()` `back()` `operator[]` `data()` `size()` 和 `empty()`。

下面的代码演示了调用 `print(span)` 的几种方法：

```cpp
vector v {11, 22, 33, 44, 55, 66};
// pass the whole vector, implicitly converted to a span
print(v);
// pass an explicitly created span
span mySpan{v};
print(mySpan);
// create a subview and pass that
span subspan{mySpan.subspan(2, 3)};
print(subspan);
// pass a subview created in-line
print({v.data() + 2, 3});

// pass an std::array
array<int, 5> arr {5, 4, 3, 2, 1};
print(arr);
print({arr.data() + 2, 3});

// pass a C-style array
int carr[] {9, 8, 7, 6, 5};
print(carr);
print({carr + 2, 3}); // a subview of the C-style array
```

**output**

```
11 22 33 44 55 66
11 22 33 44 55 66
33 44 55
33 44 55
5 4 3 2 1
3 2 1
9 8 7 6 5
7 6 5
```

与提供字符串只读视图的 `string_view` 不同，span可以提供对底层元素的读写访问。记住，span值包含一个指向序列中第一个元素的指针和元素的数量；也就是说，span永远不会复制数据。因此，修改span中的元素实际上就是修改底层序列中的元素。如果不需要，可以创建一个包含const元素的span。例如，`print()` 函数没有理由修改指定span中的任何元素。可以通过以下方式防止修改：

```cpp
void print(span<const int> values) {
  for (const auto& value : values) { cout << value << " "; }
  cout << endl;
}
```

> **注意**
>
> 在编写接受 `const vector<T>&` 的函数时，考虑使用 `span<const T>` 作为替换。这样函数就可以处理来自vector, array, C风格数组等的数据序列的视图和子视图。

