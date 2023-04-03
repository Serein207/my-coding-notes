# Modern C++ 标准库容器

> 本章你将看到现代C++中新增的容器介绍。

- [Modern C++ 标准库容器](#modern-c-标准库容器)
  - [`span`](#span)
  - [哈希函数](#哈希函数)
  - [`unordered_map`](#unordered_map)
    - [`unordered_map` 示例：电话簿](#unordered_map-示例电话簿)


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

## 哈希函数

哈希函数的结果未必是唯一的。两个或多个键哈希到同一个桶索引，就称为**冲突(collision)** 。当使用不同键得相同的哈希值，或把不同的哈希值转换为同一个桶索引时，就会发生冲突。可采用多种方法处理冲突，例如 **二次重哈希(quadratic re-hashing)** 和 **线性链(linear chaining)** 等方法。

哈希函数的选择非常重要。不产生冲突的哈希函数称为“完美哈希”。完美哈希的查找时间是常量；常规的哈希查找时间接近1，与怨怒是数量无关。随着冲突的增加，查找时间会增加，性能会降低。

C++标准为所有基本数据类型提供了哈希函数，还为error_code, error_condition, optional, variant, bitset, unique_ptr, shared_ptr, type_index, string, string_view, vector\<bool\>, thread::id提供了哈希函数。如果使用的键类型没有可用的标准哈希函数，就必须实现自己的哈希函数。

下面演示如何编写自定义的哈希函数。代码定义了一个类IntWrapper，它仅封装了一个整数。还提供了 `operator==`，因为这是在无效关联容器中使用键所必须的。

```cpp
class IntWrapper {
public:
  IntWrapper(int i) : m_wrapperInt { i } {}
  int getValue() const { return m_wrapperInt; }
  bool operator==(const IntWrapper& other) const = default; // =default since C++20
private:
  int m_wrapperInt;
};
```

为给IntWrapper类编写哈希函数，应该先给IntWrapper编写 `std::hash` 模板的特例。 `std::hash` 模板在 `<functional>` 头文件中定义。这个特例需要实现函数调用运算符，计算并返回给定IntWrapper实例的哈希值。对于本例，请求被简单地转发给了整数的标准哈希函数：

```cpp
namespace std {
template<> struct hash<IntWrapper> {
  size_t operator()(const IntWrapper& wrapper) const {
    return std::hash<int>()(wrapper.getValue());
  }
};
}
```

注意，一般不允许把任何内容放在std命名空间中，但std类模板的特例是这条规则的例外。如果类包含多个数据成员，就需要在计算哈希时考虑所有数据成员，这不是这里要讨论的内容。

## `unordered_map`

unordered_map容器在 `<unordered_map>` 头文件中定义，是一个类模板，如下所示：

```cpp
template <class Key,
          class T,
          class Hash = hash<Key>,
          class Pred = std::equal_to<Key>,
          class Alloc = std::allocator<std::pair<const Key, T>>>
class unordered_map;
```

总共有5个模板参数：键类型、值类型、哈希类型、判等比较类型和分配器类型。

与普通的map一样，unordered_map中的所有键都应该是唯一的。除此之外，它有一些哈希专用方法。例如，`load_factor()` 返回每一个桶的平均元素数，以反映冲突的次数。`bucket_count()` 方法返回容器的桶的数量。还提供了local_iterator和const_local_iterator，用于遍历单个桶中的元素，但是不能用来遍历多个桶。`bucket(key)` 方法返回指定元素的桶的索引，`begin(n)` 返回引用索引为n的桶中第一个元素的local_iterator。`end(n)` 返回引用索引为n的桶中最后一个元素之后的local_iterator。

### `unordered_map` 示例：电话簿

下面的示例将使用 `unordered_map` 表示电话簿。使用人名表示键，电话号码则是与键相关的值。

```cpp
void printMap(const auto& m) {    // C++20 abbreviated function template
  for (const auto& [key, value] : m) {
    std::cout << std::format("{} (Phone: {})", key, value) << '\n';
  }
  std::cout << "-------" << '\n';
}

int main() {
  // create a hash table
  std::unordered_map<std::string, std::string> phoneBook {
    {"Marc G.", "123-456789"},
    { "Scott K.", "654-987321" } };
  printMap(phoneBook);

  // add/remove some phone numbers
  phoneBook.insert(std::make_pair("John D.", "321-987654"));
  phoneBook["Johan G."] = "963-258147";
  phoneBook["Freddy K."] = "999-256256";
  phoneBook.erase("Freddy K.");
  printMap(phoneBook);

  // find the bucket index for a specific key
  const size_t bucket{ phoneBook.bucket("Marc G.") };
  std::cout << std::format("Marc G. is in bucket {} containing the following {} names:",
    bucket, phoneBook.bucket_size(bucket)) << '\n';
  // get begin and end iterators for the elements in this bucket
  // "auto" is here. The complier deduces the type of
  // both as unordered_map<string, string>::const_local_iterator
  auto localBegin{ phoneBook.cbegin(bucket) };
  auto localEnd{ phoneBook.cend(bucket) };
  for (auto iter{ localBegin }; iter != localEnd; ++iter) {
    std::cout << std::format("\t{} (Phone: {})", iter->first, iter->second) << '\n';
  }
  std::cout << "-------" << '\n';

  // print some statistics about the hash table
  std::cout << std::format("There are {} buckets.", phoneBook.bucket_count()) << '\n';
  std::cout << std::format("Average number of elements in a bucket is {}.",
    phoneBook.load_factor()) << '\n';
}
```

**output**

```
Scott K. (Phone: 654-987321)
Marc G. (Phone: 123-456789)
-------
Scott K. (Phone: 654-987321)
Marc G. (Phone: 123-456789)
Johan G. (Phone: 963-258147)
John D. (Phone: 321-987654)
-------
Marc G. is in bucket 1 containing the following 2 names:
        Scott K. (Phone: 654-987321)
        Marc G. (Phone: 123-456789)
-------
There are 8 buckets.
Average number of elements in a bucket is 0.5.
```