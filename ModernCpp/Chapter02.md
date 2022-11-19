# Chapter2. Modern C++ 字符串

- [Chapter2. Modern C++ 字符串](#chapter2-modern-c-字符串)
  - [2.1 字符串字面量](#21-字符串字面量)
  - [2.2 `std::string`](#22-stdstring)
    - [2.2.1 构造函数](#221-构造函数)
    - [2.2.2 赋值操作](#222-赋值操作)
    - [2.2.3 字符串拼接](#223-字符串拼接)
    - [2.2.4 查找](#224-查找)
    - [2.2.5 替换](#225-替换)
    - [2.2.6 比较](#226-比较)
    - [2.2.7 字符串存取](#227-字符串存取)
    - [2.2.8 插入和删除](#228-插入和删除)
    - [2.2.9 子串](#229-子串)
    - [2.2.10 与C风格字符串的兼容](#2210-与c风格字符串的兼容)
  - [2.3 原始字面量 *(raw string literal)*](#23-原始字面量-raw-string-literal)
  - [2.4 标准用户定义字面量](#24-标准用户定义字面量)
  - [2.5 `std::vector` 和字符串CTAD](#25-stdvector-和字符串ctad)
  - [2.6 数值转换](#26-数值转换)
    - [2.6.1 高级数值转换函数](#261-高级数值转换函数)
      - [数值转换为字符串](#数值转换为字符串)
      - [字符串转换为数值](#字符串转换为数值)
    - [2.6.2 低级数值转化](#262-低级数值转化)
      - [数值转换为字符串](#数值转换为字符串-1)
      - [字符串转换为数值](#字符串转换为数值-1)
  - [2.7 `std::string_view` 字符串视图](#27-stdstring_view-字符串视图)
    - [2.7.1 `std::string_view` 和临时字符串](#271-stdstring_view-和临时字符串)
    - [2.7.2 `std::string_view` 字面量](#272-stdstring_view-字面量)

## 2.1 字符串字面量

下面的代码输出字符串hello，这段代码包含这个字符串本身，而不是一个包含这个字符串的变量。

```cpp
std::cout << "hello" << std::endl;
```

在上面的代码中 `"hello"` 是一个字符串字面量，因为这个字符串以值的方式写出，而不是一个变量。

字符串字面量可以赋值给变量，但因为字符串字面量存储在内存的只读部分，且使用了字面量池，所以这样做会产生风险。C++标准指出：字符串字面量的类型为“n个const char的数组”，然而为了向后兼容较老的不支持const的代码，大部分编译器不会强制程序将字符串字面量赋值给 `const char*` 类型的变量。这些编译器允许将字符串字面量赋值给不带有const的char*，而且整个程序课正常运行，除非试图修改字符串。一般情况下，试图修改字符串是未定义行为：

```cpp
char* ptr {"hello"}; // Assign the string literal to a variable
ptr[1] = 'a';        // Undefined behavior!
```

一种更安全的编码方法是在引用字符串常量时，使用指向const字符的指针。下面的代码包含同样的bug，但由于这段代码将字符传字面量赋值给 `const char*` ，因此编译器会捕捉到任何写入只读内存的企图。

```cpp
const char* ptr{"hello"};  // Assign the string literal to a variable
ptr[1] = 'a';              // Error! Attempts to write to read-only memory
```

还可以将字符串字面量用作字符数组（ `char[]` ）的初始值。这种情况下，编译器会创建一个足以放下这个字符串的数组，然后将字符串复制到这个数组。因此，编译器不会将字面量放在只读的内存中，也不会进行字面量的池操作。

```cpp
char arr[]{"hello"}; 
arr[1] = 'a';
```

## 2.2 `std::string` 

我们先来学习一下STL容器 `std::string`

### 2.2.1 构造函数
```cpp  
std::string s1;                 // string()

const char *str = "hello world";
std::string s2(str);            // string(const char* s)
std::cout << "s2 = " << s2 << std::endl;

std::string s3(s2);             // string(const string& str)
std::cout << "s3 = " << s3 << std::endl;

std::string s4(10, 'a');        // string(int n, char c)
std::cout << "s4 = " << s4 << std::endl;
```

### 2.2.2 赋值操作
```cpp
std::string str1;               // string &operator=(const char* s)
str1 = "hello world";
std::cout << "str1 = " << str1 << std::endl;

std::string str2;
str2 = str1;                    // string &operator=(const string &s)
std::cout << "str2 = " << str2 << std::endl;

std::string str3;
str3 = 'a';                     // string &operator=(char c)
std::cout << "str3 = " << str3 << std::endl;

std::string str4;
str4.assign("hello world");     // string &assign(const char *s)
std::cout << "str4 = " << str4 << std::endl;

std::string str5;
str5.assign("hello world", 5);  // string &assign(const char *s, int n) 拿前n个字符
std::cout << "str5 = " << str5 << std::endl;

std::string str6;
str6.assign(str5);              // string &assign(const string &s)
std::cout << "str6 = " << str6 << std::endl;

std::string str7;
str7.assign(10, 'a');           // string &assign(int n, char c)
std::cout << "str7 = " << str7 << std::endl;
```

### 2.2.3 字符串拼接
```cpp
// string &operator+=(const char *c);
std::string str1 = "hello ";
str1 += "world";
std::cout << "str1 = " << str1 << std::endl;

// string &operator+=(const char c);
str1 += '!';
std::cout << "str1 = " << str1 << std::endl;

// string &operator+=(const string &str);
std::string str2 = "hello";
str1 += str2;
std::cout << "str1 = " << str1 << std::endl;

// string &append(const char *s);
std::string str3 = "Hello ";
str3.append("world");
std::cout << "str3 = " << str3 << std::endl;

// string &append(const char *s, int n);
str3.append("abcde", 4);
std::cout << "str3 = " << str3 << std::endl;

// string &append(const string &s);
str3.append(str2);
std::cout << "str3 = " << str3 << std::endl;

// string &append(const string &s, int pos, int n); s中从pos开始截取n个
str3.append(str2, 0, 2);
std::cout << "str3 = " << str3 << std::endl;
```
字符串常量无法拼接字符串常量
```cpp
srd::string = "hello " + "world";           // error
```

### 2.2.4 查找
```cpp
// find从左向右查找
std::string str1 = "abcdefgde";
int pos = str1.find("de");
std::cout << "find position: " << pos << std::endl; // 3 从0开始
pos = str1.find("df");
std::cout << "find position: " << pos << std::endl; // -1

// rfind从右向左查找
pos = str1.rfind("de");
std::cout << "rfind position: " << pos << std::endl; // 7 绝对位置
```

### 2.2.5 替换
```cpp
std::string str1 = "abcdefg";

str1.replace(1, 3, "1111");  // 从1号位置起 3个字符替换为"1111"

std::cout << "str1 = " << str1 << std::endl;  // a1111efg
```

### 2.2.6 比较
```cpp
std::string str1 = "hello";
std::string str2 = "xello";

if (str1.compare(str2) == 0) {
   std::cout << "str1==str2" << std::endl;
} else if (str1.compare(str2) > 0) {
   std::cout << "str1>str2" << std::endl;
} else {
   std::cout << "str1<str2" << std::endl;
}
```
**output**
```
str1<str2
```

### 2.2.7 字符串存取
```cpp
std::string str = "hello";

// 通过[]访问单个字符
for (int i = 0; i < str.size(); i++) {
   std::cout << str[i] << " ";
}
std::cout << std::endl;

// 通过at访问单个字符
for (int i = 0; i < str.size(); i++) {
   std::cout << str.at(i) << " ";
}
std::cout << std::endl;

// 修改单个字符
str[0] = 'x';
std::cout << "str = " << str << std::endl;

str.at(1) = 'x';
std::cout << "str = " << str << std::endl;
```

### 2.2.8 插入和删除
```cpp
std::string str = "hello";

// 插入
str.insert(1, "111");
std::cout << "str = " << str << std::endl;  // h111ello

// 删除
str.erase(1, 3);
std::cout << "str = " << str << std::endl;  // hello
```

### 2.2.9 子串
```cpp
std::string str = "abcdefg";
std::string subStr = str.substr(1, 3);  // 从位置1开始截取3个字符

std::cout << "subStr = " << subStr << std::endl;
```

**example** 从邮件中获取用户名信息
```cpp
std::string email = "hello@sina.com";

int position = email.find("@");  // 5
std::string userName = email.substr(0, position);

std::cout << "user name: " << userName << std::endl;
```

### 2.2.10 与C风格字符串的兼容

为了达到兼容的目的，还可应用string类的 `c_str()` 方法获得一个表示C风格字符串的 `const char` 指针。不过，一旦string执行任何内存重分配或string对象被销毁了，返回的这个const指针就失效了。永远不要从函数中返回在基于栈的string上调用 `c_str()` 的结果。

还有一个 `data()` 方法，在C++14及更早的版本中，始终与 `c_str()` 一样返回 `const char*` 。从C++17起，在非const字符串上调用时， `data()` 返回 `char*`。

## 2.3 原始字面量 *(raw string literal)*

当我们想要将如下内容写入引号时
```
D:\code\C++\code_item
```
必须添加转义字符`\\`以防止编译器识别为非法转义字符
```cpp
std::string path = "D:\\code\\C++\\code_item";
std::cout << "path is " << path << std::endl;
```
有没有一种办法不让转义字符起所用，所见即所得呢？

**下面，隆重介绍，C++11原始字面量**

```cpp
R"(string)"
R"xxx(string)xxx"
```
原始字面量（值）可以直接表示字符串的实际含义，不需要转义和连接
```cpp
std::string path = R"(D:\code\C++\code_item)";
std::cout << "path is " << path << std::endl;
```
`xxx`为标签，不是字符串的一部分，添加标签可以提高代码的可读性
```cpp
std::string path = R"abcd(D:\code\C++\code_item)abcd";
std::cout << "path is " << path << std::endl;
```
前后标签必须一致
```cpp
std::string path = R"abcd(D:\code\C++\code_item)abc";    // error
```

字符串中引号内容不能换行
```cpp
std::string str = "1,    // error
              2,
              3,
              4";
```
但是我们可以在每行末尾添加`\`作为连接符，但是输出会变得很乱
```cpp
std::string str = "1,\
              2,\
              3,\
              4";
std::cout << str << std::endl;
```
**output**
```
1,              2,              3,              4
```
我们也可以用原始字面量，输出的可读性更好
```cpp
std::string str = R"(
              1,
              2,
              3,
              4)";
std::cout << str << std::endl;
```
**output**
```
              1,
              2,
              3,
              4
```

## 2.4 标准用户定义字面量
当我们使用 `auto` 推导字符串类型时，会推导为 `const char*` 类型，而不是我们更加希望的 `string` 类型。

为此，C++11支持用户定义字面量

```cpp
void operator""_r (const char* string, size_r size) {
   std::cout << str << " " << size << std::endl;
}

void test() {
   "123456"_r;
}
```
**output**
```
123456 6
```

为了解决 `auto` 推导的问题，我们可以这样写
```cpp
std::string operator""_s(const char* str, size_t size) {
   return str;
}

void test() {
   auto str = "123456"_s;
}
```
这样 `auto` 推导的str就是 `string` 类型了。

需要注意的是，用户定义的后缀必须有下划线 `_` 字符开始，标准库定义的用户定义后缀不以下划线开始。

在标准库中也提供了和上述用法相同的后缀 `s`（C++14）

```cpp
auto str = "123456"s;
```

用户标准定义字面量 `s` 在 `std::literals::string_literals` 命名空间中定义。但是， `string_literals` 和 `literals` 命名空间都是所谓的内联命名空间。因此，使用以下选项可以使这些字符串字面量可用于你的代码：

```cpp
using namespace std;
using namespace std::literals;
using namespace std::string_literals;
using namespace std::literals::string_literals;
```

用户定义字面量的形参列表 **有以下固定写法**，不可以有其他参数

| 参数表                     |
| -------------------------- |
| `const char*`              |
| `unsigned long long int`   |
| `long double`              |
| `char`                     |
| `const char*, std::size_t` |

## 2.5 `std::vector` 和字符串CTAD

对字符串使用CTAD时必须小心，以 `vector` 的以下声明为例：

```cpp
std::vector names{"John", "Sam", "Joe"};
```

推导出的类型将是 `std::vector<const char*>` ，而不是 `std::vector<std::string>` ！这是一个很容易犯的错误。

如果你需要一个 `std::vector<std::string>` ，可以用上 `std::string` 字面量。注意下例中每个字符串字面量后面的 `s` ：

```cpp
std::vector names{"John"s, "Sam"s, "Joe"s};
```

## 2.6 数值转换

C++标准模板库提供了高级数值转换函数和低级数值转换函数。

### 2.6.1 高级数值转换函数

std命名空间包含很多辅助函数，以便完成数值和字符串之间的转换，它们定义在 `<string>` 中，它们可以使数值与字符串之间的相互转换更加容易。

#### 数值转换为字符串

下面的函数可用于将数值转换为字符串，T可以是 `(unsigned) int` `(unsigned) long long` `float` `double` 以及 `long double`。所有这些函数都负责内存分配，它们会创建一个新的 `std::string` 对象并返回。

```cpp
string to_string(T val);
```

这些函数的使用非常简单直观，例如，喜爱按的代码将 `long double` 值转换为字符串：

```cpp
double d{3.141};
std::string s{std::to_string(d)};
```

#### 字符串转换为数值

通过下面这组同样在std命名空间中定义的函数，可以将字符串转换为数值。在这些函数原型中，str表示要转换的字符串，idx是一个指针，这个指针接收第一个未转换的字符分索引，base表示转换过程中使用的进制。idx可以是空指针，如果是空指针，则被忽略。如果不能执行任何转换，这些函数会抛出 `invalid_argument` 异常。如果转换的值超出返回类型的范围，则抛出 `out_of_range` 异常。

- `int stoi(const string& str, size_t* idx = 0, int base = 10);`
- `long stol(const string& str, size_t* idx = 0, int base = 10);`
- `unsigned long stoul(const string& str, size_t* idx = 0, int base = 10);`
- `long long stoll(const string& str, size_t* idx = 0, int base = 10);`
- `unsigned long long stoull(const string& str, size_t* idx = 0, int base = 10);`
- `float stof(const string& str, size_t* idx = 0);`
- `double stod(const string& str, size_t* idx = 0);`
- `long double stold(const string& str, size_t* idx = 0);`

下面是一个示例：

```cpp
const std::string toParse{ "   123USD" };
size_t index{ 0 };
int value{ std::stoi(toParse, &index) };
std::cout << std::format("Parsed value: {}", value) << std::endl;
std::cout << std::format("First non-parsed character: '{}'", toParse[index]) << std::endl;
```

输出如下所示：

```
Parsed value: 123
First non-parsed character: 'U'
```

base的默认值为10，采用十进制，base为16表示采用十六进制。如果base被设为0，函数会按照以下规则自动计算给定数字的进制：

- 如果数字以0x或0X开头，则被解析为十六进制数字。
- 如果数字以0开头，则被解析为八进制数字。
- 其他情况下，被解析为十进制数字

### 2.6.2 低级数值转化

C++也提供了许多低级数值转换函数，这些都在 `<charconv>` 头文件中定义。这些函数不执行内存分配，也不直接使用 `std::string` ，而是用又调用者分配的缓存区。

如果希望实现高性能、完美往返、独立于语言环境的转化，则应当使用这些函数。例如，在数值数据与JSON、XML等之间进行序列化/反序列化。

#### 数值转换为字符串

要将整数转换为字符，可以使用下面一组函数：

```cpp
to_chars_result to_chars(char* first, char* last, IntegerT value, int base = 10);
```

这里，`IntegerT` 可以是任何有符号或无符号的整型或char类型。结果是 `to_chars_result` 类型，类型定义如下所示：

```cpp
struct to_chars_result {
   char* ptr;
   errc ec;
};
```

如果转换成功，ptr成员将等于写入字符串尾后一位置的指针。如果转换失败（即 `ec == errc::value_too_large`），则它等于last。

下面是一个使用示例：

```cpp
const size_t BufferSize{ 50 };
std::string out(BufferSize, ' ');
auto result{ std::to_chars(out.data(), out.data() + out.size(), 12345) };
if (result.ec == std::errc{}) {
	std::cout << out << std::endl;
}
```

使用结构化绑定，可以将其写成

```cpp
std::string out(BufferSize, ' ');
auto [ptr, error] { std::to_chars(out.data(), out.data() + out.size(),12345) };
if (error == std::errc{}) {
	std::cout << out << std::endl;
}
```

类似地，下面的一组转换函数可用于浮点类型：

```cpp
to_chars_result to_chars(char* first, char* last, FloatT value);
to_chars_result to_chars(char* first, char* last, FloatT value,
                        chars_format format);
to_chars_result to_chars(char* first, char* last, FloatT value,
                        chars_format format, int precision);
```

这里，`FloatT` 可以是float、double或long double。可以使用 `chars_format` 标志的组合指定格式：

```cpp
enum class chars_format {
   scientific,                   // Style: (-)d.ddde±dd
   fixed,                        // Style: (-)ddd.ddd
   hex,                          // Style: (-)h.hhhp±d (Note: mo 0x!)
   general = fixed | scientific  // See next paragraph.
};
```

默认格式是 `chars_format::general` ，这将导致 `to_chars()` 将浮点值转换为 (-)ddd.ddd形式的十进制表示形式，或(-)d.ddde±dd形式的十进制表示形式，得到最短的表示形式，小数点前至少有一位数字（如果存在）。

如果指定了格式，但未指定精度，将为给定格式自动确定最简短的表示形式，最大精度为6个数字。例如：

```cpp
const size_t BufferSize{ 50 };
double value{ 0.314 };
std::string out(BufferSize, ' ');
auto [ptr, error] {std::to_chars(out.data(), out.data() + out.size(), value)};
if (error == std::errc{})	std::cout << out << std::endl;
```

#### 字符串转换为数值

对于相反的转换，即将字符串转换为数值，可以用下面的一组函数:

```cpp
from_chars_result from_chars(const char* first, const char* last, 
                              IntegerT& value, int base = 10);
from_chars_result from_chars(const char* first, const char* last, 
                              FloatT& value,
                              chars_format format = chars_format::general);
```

`from_chars_result` 的类型定义如下：

```cpp
struct from_chars_result {
   const char* ptr;
   errc ec;
};
```

结果类型的ptr成员是指向第一个未转换字符的指针，如果所有字符都成功转换，则它等于last。如果所有字符都未转换，则ptr等于first，错误代码的值将为 `errc::invalid_argument` 。如果解析后的值过大，无法由给定类型表示，则错误代码的值将是 `errc::result_out_of_range`。注意，`from_chars()` 不会忽略任何前导空白。

`to_chars()` 和 `from_chars()` 的完美往返特性可以表示如下：

```cpp
const size_t BufferSize{ 50 };
double value1{ 0.314 };
std::string out(BufferSize, ' ');
auto [ptr1, error1] {std::to_chars(out.data(), out.data() + out.size(),value1)};
if (error1 == std::errc{})	std::cout << out << std::endl;

double value2;
auto [ptr2, error2] {std::to_chars(out.data(), out.data() + out.size(), value2)};
if (error2 == std::errc{})	std::cout << "Perfect roundtrip" << std::endl;
else std::cout << "No perfect roundtrip?!?" << std::endl;
```

## 2.7 `std::string_view` 字符串视图

在C++17之前，为接收只读字符串的函数选择形参类型一直是一件进退两难的事情。有时，人们会编写同一个函数的多个重载版本，一个接收 `const chat*` ，另一个接收 `const std::string&`，但显然，这并不是一个优雅的解决方案。

C++17引入 `std::string_view` 类解决了所有这些问题，`std::string_view` 类是 `std::basic_string_view` 类模板的实例化，在 `<string_view>` 中定义。string_view 基本上就是 `const string&` 的简单替代品，但不会产生开销。它从不复制字符串，string_view支持与std::string类似的接口。一个例外是缺少 `c_str()` ，但 `data()` 是可用的。另外，string_view添加了 `remove_prefix(size_t)` 和 `remove)suffix(size_t)` 方法，前者将其实指针前移给定的偏移量来收缩字符串，后者则将结尾指针倒退给定的偏移量来收缩字符串。

下面的代码片段中，`extractExtension()` 函数提取给定文件名的拓展名（包括点号）并返回。注意，通常按值传递string_view，因为它们复制成本极低，它们只包含指向字符串的指针以及字符串的长度。

```cpp
std::string_view extractExtension(std::string fileName) {
	return fileName.substr(fileName.rfind('.'));
}
```

该函数可用于所有类型不同的字符串：

```cpp
std::string fileName { R"(C:\temp\my file.ext)" };
std::cout << std::format("C++ string: {}", extractExtension(fileName)) << std::endl;

const char* cString { R"(C:\temp\my file.ext)" };
std::cout << std::format("C string: {}", extractExtension(cString)) << std::endl;

std::cout << std::format("C++ string: {}", extractExtension(R"(C:\temp\my file.ext)")) << std::endl;
```

在对 `extractExtension()` 的所有这些调用中，没有进行一次复制。`extractExtension()` 函数的fileName参数只是指针和长度，该函数的返回类型也是如此。这都十分高效。

还有一个string_view构造函数，它接收任意原始缓冲区和长度。这可用于从字符串缓冲区（并非以NUL终止）构建string_view。如果确实有一个以NUL终止的字符缓冲区，但你已经知道字符串的长度，构造函数不必再次统计字符数目，这也是有用的。

```cpp
const char* raw {...};
size_t length {...};
std::cout << std::format("Raw: {}", extractExtension({raw, length} )) << std::endl;
```

最后一行也可以写成这样：

```cpp
std::cout << std::format("Raw: {}", extractExtension(std::string_view {raw, length} )) << std::endl;
```

> **注意**
>
> 在每当函数需要将制度字符串作为一个参数时，可使用 `std::string_view` 替代 `const std::string&` 或 `const char*` 。

无法从 `std::string_view` 隐式构建一个string。要么使用一个显示的string构造函数，要么使用 `std::string_view::data()` 成员。例如，假设有以下接收 `const string&` 的函数：

```cpp
void handleExtension(const string& extension) {...}
```

不能采取如下方式调用该函数：

```cpp
handleExtension(extractExtension("my file.ext"));
```

下面是两个可供使用的选项：

```cpp
handleExtension(extractExtension("my file.ext").data());          // data() method
handleExtension(std::string {extractExtension("my file.ext") });  // explicit ctor
```

由于同样的原因，无法连接一个string和一个string_view。下面的代码将无法编译：

```cpp
std::string { "Hello" };
std::string_view { " World" };
auto result { str + sv };
```

你可以对string_view使用 `data()` 方法，如下所示：

```cpp
auto result1 { str + sv.data() };
```

或者你可以使用 `append()`：

```cpp
std::string result2 { str };
result2.append(sv.data(), sv.size());
```

> **警告**
>
> 返回字符串的函数应返回 `const std::string&` 或 `std::string`，但不应该返回一个 `std::string_view`。返回 `std::string_view` 会还来使返回的 `std::string_view` 无效的风险，例如当它指向的字符串需要重新分配时。
>
> 将 `std::string&` 或 `std::string_view` 存储为类的数据成员需要确保它们指向的字符串在对象的声明周期内保持有效状态。存储 `std::string` 更安全。

### 2.7.1 `std::string_view` 和临时字符串

`std::string_view` 不应该用于保存一个临时字符串的视图，考虑以下示例：

```cpp
std::string s { "hello" };
std::string_view sv { s + "World!" };
std::cout << sv;
```

此段代码具有未定义行为。sv的初始化表达式将生成一个临时字符串，其中包含 `"Hello World!"`。然后，string_view存储指向此临时字符串的指针。在第二行代码的末尾，这个临时字符串被销毁，留下一个悬空指针的string_view。

> **警告**
>
> 永远不要使用 `std::string_view` 存储一个临时字符串的视图。

### 2.7.2 `std::string_view` 字面量

可使用标准用户定义字面量 `sv`，将字符串字面量解释为 `std::string_view`。例如：

```cpp
auto sv { "My string_view"sv };
```

标准用户定义字面量 `sv` 需要以下几条 `using` 命令之一：

```cpp
using namespace std::literals::string_view_literals;
using namespace std::string_view_literals;
using namespace std::literals;
using namespace std;
```