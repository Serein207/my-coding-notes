# Chapter17. Modern C++ 字符串本地化

> 本章分讨论本地化，本地化现在越来越重要，它允许编写为全世界不同地区进行本地化的软件。

- [Chapter17. Modern C++ 字符串本地化](#chapter17-modern-c-字符串本地化)
  - [17.1 宽字符](#171-宽字符)
  - [17.2 本地化字符串字面量](#172-本地化字符串字面量)
  - [17.3 非西方字符集](#173-非西方字符集)

## 17.1 宽字符

用字节表示字符的问题在于，不是所有语言（或字符集）都可以用8位（1字节）来表示。C++有一种内建类型 `wchar_t`，可以保存 **宽字符(wide character)**。然而，C++标准没有定义wchar_t的大小。一些编译器使用16位，而另一些编译期使用32位。为编写跨平台代码，将wchar_t假定为任何特定的数值都是不安全的。

在使用wchar_t时，需要在字符串和字符串字面量之前加上字母L，一表示应该使用宽字符编码。例如，要将wchar_t字符初始化为字母m，应该编写以下代码：

```cpp
wchar_t myWideCharacter { L'm' };
```

大部分常用类型和类都有宽字符版本。宽字符版本的string为wstring。前缀字母w也可以应用于流。wofstream处理宽字符文件输出流，wifstream处理宽字符文件输入流。

cout, cin, cerr和clog也有宽字节版本：wcout, wsin, wcerr和wclog。这些版本的使用和非宽字节版本的使用没有区别。

```cpp
wcout << L"I an a wide-character string literal." << endl;
```

## 17.2 本地化字符串字面量

本地化的一个关键点在于蹦年在源码中放置任何本机语言的字符串子民阿亮，除非是面向开发人员的调试字符串。在Windows应用程序中，通过将程序的所有字符串放在STRINGTABLE资源中达到了这个目的。如果需要将应用程序翻译为其他语言，只要翻译那些资源即可，而不需要修改任何源代码。有一些工具可以帮助完成翻译过程。

为了让源代码能本地化，不应该利用字符串字面量组成句子，即使单独的字面量也可以被本地化。例如：

```cpp
size_t n { 5 };
wstring filename { L"file1.txt" };
wcout << L"Read " << n << L"bytes from " << filename << endl;
```

这条语句不能被本地化为中文，因为中文的语序有所变化，中文的翻译可能如下：

```cpp
wcout << L"从" << filename << L"中读取" << n << L"个字节" << endl;
```

为了能正确地本地化这个字符串，可采用下面的方式来实现：

```cpp
cout << format(loadResource(IDS_TRANSFERRED), n, filename) << endl;
```

IDS_TRANSFERRED是字符串资源表中一个条目的名称。对于英文版，IDS_TRANSFERRED可定义为 `"Read {0} bytes from {1}"`。对于中文版，这条资源可以定义为 `"从{1}中读取{0}个字节"`。LoadResource()函数用给定的名称加载字符串资源，format()用n的值替换{0}，用filename的值替换{1}。

## 17.3 非西方字符集

国际标准ISO 10646定义的Universal Character Set（UCS）和Unicode都是标准化的字符集。UTF-8是Unicode编码的一个实例，例如Unicode字符编码为1到4个8位字节，UTF-16将Unicode字符编码为一个或两个16位的值，UTF-32将Unicode字符编码正好32位。

不同的应用程序使用不同编码。遗憾的是，本章前面提到过，C++标准并没有定义宽字符wchar_t的大小。为了解决这个问题，可以使用另外两个字符类型：`char16_t` 和 `char32_t`。下面的列表总结了支持的所有字符类型。

- char：存储8位。可用于保存ASCII字符，还可用作保存UTF-8编码的Unicode字符的基本构建块。使用UTF-8时，一个Unicode字符编码为1到4个char。
- charx_t：存储至少x位，x可以是8（C++20）、16或32。这种类型可用作UTF-x编码的Unicode字符的基本构建块。
- wchar_t：保存一个宽字符，宽字符的大小和编码取决于编译期。

使用charx_t而不是wchar_t的好处在于：标准保证charx_t的最小大小，它们的大小和编译器无关。

使用字符串前缀可将字符串字面量转换为特定类型。下面列出所有支持的字符串前置。

- u8：采用UTF-8编码的char（C++20中是char8_t）字符串字面量。
- u：采用UTF-16编码的char16_t字符串字面量（C++20保证）。
- U：采用UTF-32编码的char32_t字符串字面量（C++20保证）。
- L：采用编译器相关编码的wchar_t字符串字面量。

所有这些字符串字面量都可与原始字符串字面量前缀R结合使用。例如：

```cpp
const char* s1 { u8R"(Raw UTF-8 encoded string literal)" };
const wchar_t* s2 { LR"(Rae wide string literal)" };
const char16_t* s3 { uR"(Raw UTF-16 encoded string literal)" }; 
const char32_t* s4 { UR"(Raw UTF-32 encoded string literal)" };
```

如果通过u8 UTF-8字符串字面量使用了Unicode编码，那么在非原始字符串字面量中可通过 `\uABCD` 符号插入指定的Unicode码点。例如，`\u03C0` 表示pi字符，`\u00B2` 表示字符²，因此以下代码会打印出 `πr²`：

```cpp
const char* formula { u8"\u03C0 r\u00B2" };
```

字符串字面量也可具有前缀，已转换为特定类型。支持前缀u8、u、U和L，一些例子有 `u'a'` `U'a'` `L'a'` 和 `u8'a'`。

除std::string外，目前还支持wstring, u8string(C++20), u16string和u32string。

类似地，标准库提供了std::string_view, wstring_view, u8string_view(C++20), u16string_view和u32string_view。这些都基于basic_string_view。

多字节字符串可以使用Unicode编码或任何其他类型的编码。下面的转换函数可在char8_t/char16_t/char32_t和多字节字符之间来回转换：(C++20)mbrtoc8()和c8rtomb(), mbrtoc16(), c16tomb(), mbrtoc32(), 和 c32tomb()。

没有支持这些字符类型的cout或cin版本，因此很难向控制台打印这种字符串。如果想要更多地使用这样的字符串，那么需要求助于第三方库。ICU(International Components for Unicode)是一个十分知名的库，可为应用程序提供Unicode和全球化支持。