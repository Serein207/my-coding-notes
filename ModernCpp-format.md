# Modern C++ `std::format`

> 直到C++20之前，字符串的格式化一般是通过 `printf()` 之类的C风格函数或是C++的I/O流完成的。
>
> C++20引入了 `std::format()` ，用来格式化字符串，它定义在 `<format>` 中。它基本上结合了C风格的 `printf()` 和C++的I/O流的所有优点，是一种类型安全且可拓展的机制。现在是时候看看 `std::format()` 的强大之处了。

- [Modern C++ `std::format`](#modern-c-stdformat)
    - [格式说明符](#格式说明符)
      - [1. width](#1-width)
      - [2. \[fill\]align](#2-fillalign)
      - [3. sign](#3-sign)
      - [4. `#`](#4-)
      - [5. type](#5-type)
      - [6. precision](#6-precision)
      - [7. `0`](#7-0)
    - [格式说明符错误](#格式说明符错误)
    - [支持自定义类型](#支持自定义类型)

`format()` 的第一个参数是待格式化的字符串，后续参数是用于填充带格式化字符串中占位符的值。带目前位置，使用 `format()` 时的占位符一般都是一对花括号 `{}` 。在这些花括号内可以是格式为[index][:specifier]的字符串。可以省略所有占位符中的index，也可以为所有占位符指定从零开始的索引，以指明应用于此占位符的第二个和后续参数。如果省略index，则 `format()` 的第二个和后续参数传递的值将按给定顺序用于所有占位符。specifier是一种格式说明符，用于更改值在输出中格式化的方式。如果需要输出 `{or}` 字符，则需要将其转义为 `{{or}}` 。

现在看看如何使用index部分。以下对format()的调用省略了占位符中的显式索引：

```C++
auto result = std::format("I'm {}! My name is {}", "nice", "kitty");
auto result2 = std::format("I'm {}! My name is {}", 5, "kitty");
std::cout << result << '\n' << result2 << '\n';
```

可以按以下方式手动指定索引：

```cpp
auto result3 = std::format("{0} and {1} are {0}", 5, 7);
auto result4 = std::format("{1} and {0} are {0}", 5, 7);
```

不允许混合使用手动和自动索引，下例使用了一个无效的字符串格式化：

```cpp
auto s2 { std::format("{0} and {}", 5, 7) };
```

### 格式说明符

格式说明符用于控制值在输出中的格式，前缀为冒号。格式说明符的一般形式如下所示（注释：严个来说，在精度和类型之间有一个可选的L，本处不做讨论）：

```cpp
[[fill]align][sign][#][0][width][.precision][type]
```

方括号里的所有说明符都是可选的，下面将详细讨论各个说明符。

#### 1. width

width指定带格式化的值所占字段的最小宽度。width也可以是另一组花括号，称为动态宽度。如果在花括号中制定了索引，例如 `{3}` ，则width取自给定索引对应的 `format()` 的实参。如果未指定索引，例如 `{}`，则width取自 `format()` 实参列表中的下一个参数。示例如下：

```cpp
int i {42};
std::cout << std::format("|{:5}|", i) << std::endl;           // |   42|
std::cout << std::format("|{:{w}.5}|", i, w=5) << std::endl;  // |   42|
```

#### 2. [fill]align

fill指定占位符的填充字符，通常是一个字符填充了空间的小块，align是在其字段中的对齐方式：

- `<` 表示左对齐（非整数和非浮点数的默认对齐方式）
- `>` 表示右对齐（整数和浮点数的默认对齐方式）
- `^` 表示居中对齐

fill字符会被插入输出中，以确保输出中的字段达到说明符的[width]指定的最小宽度。如果未指定[width]，则[fill]align无效。

示例如下：

```cpp
int i {42};
std::cout << std::format("|{:<7}|", i) << std::endl;        // |42     |
std::cout << std::format("|{:>7}|", i) << std::endl;        // |     42|
std::cout << std::format("|{:_>7}|", i) << std::endl;       // |_____42|
std::cout << std::format("|{:_^7}|", i) << std::endl;       // |__42___|
```

#### 3. sign

sign指定输出中的数值符号。有一些符号可以添加到输出中，下面展示了常见的符号：

- `+` 表示显示整数和负数的符号
- `-` 表示只显示负数的符号（默认方式）
- space 表示对于负数使用符号，对于整数使用空格

示例如下：

```cpp
int i {42};
std::cout << std::format("|{:<5}|", i) << std::endl;    // |42   |
std::cout << std::format("|{:<+5}|", i) << std::endl;   // |+42  |
std::cout << std::format("|{:< 5}|", i) << std::endl;   // | 42  |
std::cout << std::format("|{:< 5}|", -i) << std::endl;  // |-42  |
```

#### 4. `#`

`#` 启用所谓的备用格式(alternate formatting)规则。如果为整型启用，并且还制定了十六进制、二进制或八进制数字格式，则备用格式会在格式化数字前面加入 `0x` `0X` `0b` `0B` 或 `0`。如果为浮点型启用，则备用格式将始终输出十进制分隔符，即使后面没有数字。

以下两节给出了备用格式的示例。

#### 5. type

type指定了给定值要被格式化的类型，以下是几个选项：

- 整型：b（二进制），B(二进制，指定#时使用0B），d（十进制），o（八进制），x（小写字母abcde的十六进制），X（大写字母ABCDE的十六进制，当指定#时，使用0X）。如果type未指定，整型默认使用d。
- 浮点型：支持以下浮点格式
  - e,E: 以小写e或大写E表示指数的科学表示法，按照给定精度或6格式化。
  - f,F: 固定表示法，按照给定精度或6格式化。
  - g,G: 以小写e或大写E表示指数的通用表示法，按照给定精度或6格式化。
  - a,A: 带有小写字母或大写字母的十六进制表示法。
  - 如果type未指定，浮点型默认使用g。
- 布尔型：s（以文本形式输出true或false），b,B,c,d,o,x,X（以整数输出0或1）。如果type未指定，布尔型默认使用s。
- 字符型：c（输出字符副本），b,B,d,o,x,X（整数表示）。如果type未指定，字符型默认使用c。
- 字符串：s（输出字符串副本）。如果type未指定，字符串默认使用s。
- 指针：p（0x为前缀的十六进制表示法）。如果type未指定，指针默认使用p。

整型的示例如下：

```cpp
int i {42};
std::cout << std::format("|{:10d}|", i) << std::endl;   // |        42|
std::cout << std::format("|{:10b}|", i) << std::endl;   // |    101010|
std::cout << std::format("|{:#10b}|", i) << std::endl;  // |  0b101010|
std::cout << std::format("|{:10X}|", i) << std::endl;   // |        2A|
std::cout << std::format("|{:#10X}|", i) << std::endl;  // |      0X2A|
```

字符串的示例如下：

```cpp
std::string s {"ProCpp"};
std::cout << std::format("|{:_^10}|", s) << std::endl;  // |__ProCpp__|
```

浮点型的示例将在下一小节给出。

#### 6. precision

precision只能用于浮点和字符串类型，它的格式为一个点后跟浮点类型要求输出的小数点数位数，或字符串要输出的字符数。就像width一样，这也可以是另一组花括号，在这种情况下，它被称动态精度。precision取自 `format()` 实参列表的下一个实参或具有给定索引的实参。

浮点型的示例如下：

```cpp
double d {3.1415 / 2.3};
std::cout << std::format("|{:12g}|", d) << std::endl;     // |       1.365870|
std::cout << std::format("|{:12.2}|", d) << std::endl;    // |           1.37|
std::cout << std::format("|{:12e}|", d) << std::endl;     // |   1.265870e+00|

int width {12};
int precision {3};
std::cout << std::format("|{2:{0}.{1}f}|", width, precision, d) << std::endl;     // |       1.366|
```

#### 7. `0`

0表示，对于数值，将0茶树格式化结果中，以达到[width]指定的最小宽度，这些0插入在数值前面，但在符号以及任何0x、0X、0b、0B前缀之后。如果指定了对齐，则将忽略本选项。

示例如下：

```cpp
int i {42};
std::cout << std::format("|{:06d}|", i) << std::endl;   // |000042|
std::cout << std::format("|{:+06d}|", i) << std::endl;   // |+00042|
std::cout << std::format("|{:06X}|", i) << std::endl;   // |00002A|
std::cout << std::format("|{:#06X}|", i) << std::endl;   // |0X002A|
```

### 格式说明符错误

如前所述，格式说明都需要遵循严格的规则。如果格式说明符包含错误，将抛出 `std::format_error` 异常。

```cpp
try {
  std::cout << std::format("An integer: {:.}", 5);
} catch (const std::format_error& caught_exception) {
  std::cout << caught_exception.what();     // "missing precision specifier"
}
```

### 支持自定义类型

可以拓展C++20格式库以添加对自定义类型的支持。这涉及编写 `std::formatter` 类模板的特化版本，该模板包含两个方法模板： `parse()` 和 `format()`。

假设有一个用来存储键值对的类：

```cpp
class KeyValue {
 public:
   KeyValue(std::string_view key, int value) : m_key{key}, m_value{value} {}

   const std::string& getKey() const { return m_key; }
   int getValue() const { return m_value; }
 private:
   std::string m_key;
   int m_value;
};
```

可以通过编写以下类模板特化来实现KeyValue对象的自定义formatter。此自定义格式还支持自定义格式说明符： `{:a}` 只输出键，`{:b}` 只输出值，`{:c}` 和 `{}` 同时输出键和值。

```cpp
template<>
class std::formatter<KeyValue> {
public:
  constexpr auto parse(auto& context) {
    auto iter { context.begin() };
    const auto end { context.end() };
    if (iter == end || *iter == '}') {  // {} format specifier
      m_outputType = OutputType::KeyAndValue;
      return iter;
    }

    switch (*iter) {
    case 'a': // {:a} format specifier
      m_outputType = OutputType::KeyOnly;
      break;
    case 'b': // {:b} format specifier
      m_outputType = OutputType::ValueOnly;
      break;
    case 'c':	// {:c} format specifier
      m_outputType = OutputType::KeyAndValue;
      break;
    default:
      throw std::format_error { "Invalid KeyValue format specifier." };
    }

    ++iter;
    if (iter != end && *iter != '}') {
      throw std::format_error { "Invalid KeyValue format specifier." };
    }
    return iter;
  }

  auto format(const KeyValue& kv, auto& context) {
    switch (m_outputType) {
      using enum OutputType;

    case KeyOnly:
      return std::format_to(context.out(), "{}", kv.getKey());
    case ValueOnly:
      return std::format_to(context.out(), "{}", kv.getValue());
    default:
      return std::format_to(context.out(), "{} - {}",
        kv.getKey(), kv.getValue());
    }
  }

private:
  enum class OutputType {
    KeyOnly,
    ValueOnly,
    KeyAndValue
  };
  OutputType m_outputType { OutputType::KeyAndValue };
};
```

`parse()` 方法负责解析范围[context.begin(), context.end()]内的格式说明符。它将解析格式说明符的结果都存储在formatter类的数据成员中，并且应该返回一个迭代器，该迭代器指向解析格式说明符字符串结束后的下一个字符。

`format()` 方法根据 `parse()` 解析的格式规范格式化第一个实参，将结果写入 `context.out()` ，并返回一个指向输出末尾的迭代器。在本例中，通过将工作转发到 `std::format_to()` 来执行实际的格式化。`format_to()` 函数接收预先分配的缓冲区作为第一个参数，并将结果字符串写入其中，而 `format()` 则创建一个新的字符串对象以返回。

可以这样测试自定义formatter：

```cpp
KeyValue keyValue { "Key1", 11 };
std::cout << std::format("{}", keyValue) << std::endl;
std::cout << std::format("{:a}", keyValue) << std::endl;
std::cout << std::format("{:b}", keyValue) << std::endl;
std::cout << std::format("{:c}", keyValue) << std::endl;
try { std::cout << std::format("{:cd}", keyValue) << std::endl; }
catch (const std::format_error& caught_exception) { std::cout << caught_exception.what(); }
```

输出如下：

```
Key1 - 11
Key1
11
Key1 - 11
Invalid KeyValue format specifier.
```