# Chapter18. Modern C++ 正则表达式

> 本章介绍正则表达式库，通过这个库很容易对字符串执行模式匹配。正则表达式非常强大，建议使用正则表达式，而不要自己编写字符串处理代码。

- [Chapter18. Modern C++ 正则表达式](#chapter18-modern-c-正则表达式)
  - [18.1 ECMAScript语法](#181-ecmascript语法)
    - [1. 锚点](#1-锚点)
    - [2. 通配符](#2-通配符)
    - [3. 替代](#3-替代)
    - [4. 分组](#4-分组)
    - [5. 重复](#5-重复)
    - [6. 优先级](#6-优先级)
    - [7. 字符集合匹配](#7-字符集合匹配)
    - [8. 词边界](#8-词边界)
    - [9. 后向引用](#9-后向引用)
    - [10. lookahead](#10-lookahead)
    - [11. 正则表达式和原始字面量](#11-正则表达式和原始字面量)
    - [12. 常用正则表达式](#12-常用正则表达式)
  - [18.2 regex库](#182-regex库)
  - [18.3 `regex_match()`](#183-regex_match)
  - [18.4 `regex_search()`](#184-regex_search)
  - [18.5 `regex_iterator`](#185-regex_iterator)
  - [18.6 `regex_token_iterator`](#186-regex_token_iterator)
  - [18.7 `regex_replace()`](#187-regex_replace)

正则表达式在 `<regex>` 中定义，是标准库字符串相关的一个强大特性。正则而表达式是一种用于字符串处理的微型语言。正则表达式适用于一些与字符串相关的操作。

- 验证：检查输入字符串是否格式正确。
- 决策：判断输入表示哪种字符串。
- 解析：从字符串中提取信息。
- 转换：搜索子字符串，将子字符串替换为新的格式化的子字符串。
- 遍历：搜索所有子字符串。
- 符号化：根据一组分隔符将字符串分解为多个子字符串。

在深入介绍正则表达式的细节之前，需要介绍一些重要的术语。

- **模式(pattern)**：正则表达式实际上是通过字符串表示的模式。
- **匹配(match)**：判断给定的正则表达式和给定序列[first,last)中的所有字符是否匹配。
- **搜索(search)**：判断在给定序列[first,last)中是否存在匹配给定正则表达式的子字符串。
- **替换(replace)**：在给定序列中识别子字符串，然后将子字符串替换为从其他模式计算得到的新子字符串，其他模式称为 **替换模式(substitution pattern)**。

有几种不同的正则表达式语法。C++包含对以下几种语法的支持：

- **ECMAScript**：基于ECMAScript标准的语法。JavaScript、ActionScript和Jscript等语言都使用该标准。
- **basic**：基本的POSIX语法。
- **extended**：拓展的POSIX语法。
- **awk**：POSIX awk实用工具使用的语法。
- **grep**：POSIX grep实用工具使用的语法。
- **egrep**：用逗号分隔的POSIX egrep语法。

C++中的默认语法是ECMAScript，接下来将详细介绍这种语法。这也是最强大的正则表达式语法，建议使用ECMAScript，而不要使用其他功能受限的语法。

## 18.1 ECMAScript语法

正则表达式模式是一个字符串序列，这种模式表达了要匹配的内容。正则表达式中的任何字符都表示匹配自己，但以下特殊字符除外：

```ECMAScript
^ $ \ . * + ? () [] {} |
```

下面将逐一讲解这些特殊字符。如果需要匹配这些特殊字符，那么需要通过 `\` 字符将其转义。例如：

```ECMAScript
\[ 或 \. 或 \\
```

### 1. 锚点

特殊字符`^`和`$`被称为锚点(anchor)。`^`字符匹配行终止符前面的位置，`$`字符匹配行终止符所在的位置。^和$默认还分别匹配字符串的开头和结尾位置，但是可以禁用这种行为。

例如， `^test$` 只匹配字符串test，不匹配包含test和其他任何字符的字符串，例如1test、2test和test abc等。

### 2. 通配符

通配符(wildcard)可用于匹配除换行符外的任意字符。例如，正则表达式 `a.c` 可以匹配abc和a5c，但不匹配ab5c和ac。

### 3. 替代

|字符表示“或”的关系。例如，`a|b` 表示匹配a或b。

### 4. 分组

圆括号 `()` 用于标记子表达式，子表达式也称为捕获组(capture group)。捕获组有以下用途：

- 捕获组可用于识别源字符串中单独的子序列，在结果中会返回每一个标记的子表达式（捕获组）。如以下正则表达式：`(.)(ab|cd)(.)` 。其中有3个标记的子表达式。对字符串lcd4运行regex_search()，执行这个正则表达式会得到含有4个条目的匹配结果。第一个条目是完整匹配1cd4，接下来3个条目是3个标记的子表达式。这三个条目为：1、cd、4。
- 捕获组可在匹配过程中用于向后引用(back reference)的目的。
- 捕获组可在替换操作的过册灰姑娘中用于识别组件。

### 5. 重复

使用以下4个重复字符可重复匹配正则表达式中的部分模式：

- *匹配零次或多次之前的部分。例如 `a*b` 可匹配b、ab、aab、aaaab等字符串。
- +匹配一次或多次之前的部分。例如 `a+b` 可匹配ab、aab、aaaab等字符串，但不能匹配b。
- ?匹配零次或一次之前的部分。例如 `a?b` 匹配b和ab，不能匹配其他任何字符串。
- {...}表示区间的重复。`a{n}` 重复匹配a正好n次；`a{n,}` 重复将匹配n次或更多次；`a{n,m}` 重复将a匹配n到m次，包含n次和m次。例如 `^a{3,4}$` 可以匹配aaa和aaaa，但不能匹配a、aa和aaaaa等字符串。

重复匹配字符串称为贪婪匹配，因为这些字符可以找出最长匹配，但仍匹配正则表达式的其余部分。为进行非贪婪匹配，可在重复字符的后面加上一个?例如 `*?` `+?` `??` `{...}?` 。非贪婪匹配将其模式重复尽可能少的次数，但仍匹配正则表达式的其余部分。

下表列出了贪婪匹配和非贪婪匹配的正则表达式，一家在输入序列aaabbb上运行它们后得到的子字符串。

| 正则表达式                   | 匹配的子字符串 |
| ---------------------------- | -------------- |
| 贪婪匹配：`(a+)(ab)*(b+)`    | "aaa" "" "bbb" |
| 非贪婪匹配：`(a+?)(ab)*(b+)` | "aa" "ab" "bb" |

### 6. 优先级

与数学公式一样，正则表达式中元素的优先级也很重要。正则表达式的优先级如下：

- 元素：例如a，是正则表达式最基本的构建块
- 量词：例如+、*、?、和{...}，紧密绑定至左侧的元素，如b+。
- 串联：例如ab+c，在量词之后绑定。
- 替代符：例如|，最后绑定。

例如正则表达式 `ab+c|d` ，它匹配abc, abbc, abbbc等字符串，还能匹配d。圆括号可以改变优先级顺序。例如，`ab+(c|d)` 可以匹配abc, abbc, abbbc, ..., abd, abbd和abbbd等字符串。不过，如果使用了圆括号，也意味着将圆括号内的内容标记为子表达式或捕获组。使用 `(?:...)` ，可在避免创建新捕获组的情况下修改优先级。例如，`ab+(?:c|d)` 和之前的 `ab+(c|d)` 匹配的内容是一样的，但没有创建多余的捕获组。

### 7. 字符集合匹配

`(a|b|c|...|z)` 这种表达式既冗长，又会引入捕获组，为了避免这种正则表达式，可以用一种特殊语法，指定一组字符或字符的范围。此外，还可以使用“否定”形式匹配。在方括号之间指定字符集合，`[c1c1...cn]` 可以匹配字符c1, c2,..., cn中的任意字符。例如，`[abc]` 可以匹配a, b, c中的任意字符。如果第一个字符是^，那么表示“除了这些字符之外的任意字符”：

- `ab[cde]` 匹配abc, abd, abe。
- `ab[^cde]` 匹配abf和abp等字符串，但不匹配abc, abd和abe。

如果想要匹配^, [, ]字符本身，需要转义这些字符，例如：`[\[\^\]]` 匹配[, ^, ]。

如果想要指定所有字母，可采取两种解决方案。

一种方案是使用方括号内的范围藐视，这允许使用 `[a-zA-Z]` 这样的表达方式。这种表达方式能识别a到z和A到Z范围内的所有字母。如果需要匹配连字符，则需要转义这个字符，例如 `[a-zA-Z\-]+` 匹配任意单词，包括带连字符的单词。

另一种方案是使用某种字符类(character class)。字符类表示特定的字符，表示方法为 `[:name:]` ，可使用什么字符类取决于locale，但下表的名称总是可以识别的。这些字符类的含义也取决于locale。这个表假定使用标准的C locale。

| 字符类别名称 | 说明                                             |
| ------------ | ------------------------------------------------ |
| digit        | 数字                                             |
| d            | 同digit                                          |
| xdigit       | 十六进制使用的数字和字母（大小写）               |
| alpha        | 字母数字字符，是所有的大小写字母                 |
| alnum        | alpha类和digit类的组合                           |
| w            | 同alnum                                          |
| lower        | 小写字母                                         |
| upper        | 大写字母                                         |
| blank        | 一行中分割单词的空白字符，' '或'\t'              |
| space        | 空白字符，' '  '\t' '\n' '\r' '\v '\f'           |
| s            | 同space                                          |
| print        | 可打印字符，占用打印位置                         |
| cntrl        | 控制符，不占用打印位置                           |
| graph        | 带有图形表示的字符，包括除空格外的所有可打印字符 |
| punct        | 标点符号字符，包括不是alnum的所有graph           |

字符类用在字符集中，例如，英语中的 `[[:alpha:]]*` 等同于 `[a-zA-Z]*`。

由于有些概念使用非常频繁，例如匹配数字，因此这些字符类有缩写模式。例如，通过以下任意模式可以识别一个或多个数字序列：

- `[0-9]+`
- `[[:digit:]]+`
- `[[:d:]]+`
- `\d+`

下标列出了字符类可用的转义符号：

| 转义符号 | 等价于     |
| -------- | ---------- |
| \d       | `[[:d:]]`  |
| \D       | `[^[:d:]]` |
| \s       | `[:s:]`    |
| \S       | `[^:s:]`   |
| \w       | `_[:w:]]`  |
| \W       | `_[^:w:]]` |

下面举一些示例：

- `Test[5-8]` 匹配Test5, Test6, Test7, Test8。
- `[[:lower:]]` 匹配a和b等，但不匹配A和B等。
- `[^[:lower:]]` 匹配除了小写字母之外的任意字符。
- `[[:lower:]5-7]` 匹配任意小写字母，还匹配数字5, 6, 7。

### 8. 词边界

词边界(word boundary)的意思可能是：

- 单词的第一个字符，这个字符是单词字符之一，而且之前的字符不是单词字符。单词字符是字母、数字或下划线，这等价于 `[A-Za-z0-9_]`。
- 单词的结尾字符，这是单词字符之后的非单词字符，之前的字符是单词字符。
- 如果源字符串的第一个字符在单词字符（即字母、数字或下划线）之后，则表示源字符串的开头位置。匹配源字符串的开头位置默认为启用，但也可以禁用(regex_constants::match_not_bow，bow表示字母开头)。
- 如果源字符串的最后一个字符是单词字符之一，则表示源字符串的结束位置。匹配源字符串的结束位置默认为启用，但也可以禁用(regex_constants::match_not_eow，eow表示字母结尾)。

通过\b可匹配单词边界，通过\B匹配单词边界外的任何内容。

### 9. 后向引用

通过后向引用(back reference)可引用正则表达式本身的捕获组：\n表示第n个捕获组，且n>0。例如，正则表达式 `(\d+)-.*-\1` 匹配以下格式的字符串：

- 在一个捕获组中(\d+)捕获的一个或多个数字
- 接下来是一个连字符-
- 接下来是0个或多个字符.*
- 接下来是另一个连字符-
- 接下来是第一个捕获组捕获到的相同数字\1

这个正则表达式能匹配123-abc-123和1234-a-1234等字符串，但不能匹配123-abc-1234和123-abc-321等字符串。

### 10. lookahead

正向表达式支持正向lookahead(?=模式)和负向lookahead(?!模式)。lookahead后面的字符必须匹配（正向）或不匹配（负向）lookahead模式，但这些字符还没有使用。

例如， `a(?!b)` 模式包含一个负向lookahead，以匹配之后不跟b的字母。`a(?=b)` 模式包含一个正向lookahead，以匹配之后后跟b的字母，但不使用b，b不是匹配的一部分。

下面是一个更复杂的示例。正则表达式匹配一个输入序列，该输入序列至少包含一个小写字母、至少一个大小字母、至少一个标点符号，并且至少8个字符长。例如，可使用下面这样的正则表达式来强制密码满足特定条件：

```ECMAScript
(?=.*[[:lower:]])(?=.*[[:upper:]])(?=[[:punct:]]).{8,}
```

### 11. 正则表达式和原始字面量

使用原始字面量可使C++源代码中的复杂正则表达式更容易阅读。例如以下正则表达式：

```cpp
"(|\\n|\\r|\\\\)"
```

这个正则表达式搜索空格、换行符、回车符或反斜杠。从中可以看出，这个正则表达式需要使用很多转义字符。使用原始字符串字面量，这个正则表达式可替换为以下更便于阅读的版本：

```cpp
R"((|\n|\r|\\))"
```

### 12. 常用正则表达式

对于常见的模式，例如验证密码、电话号码、社会安全号码、IP地址、邮件地址、信用卡号码和日期等，不需要自己编写。regexr.com, regex101.com, regextester.com等网站具有预定义模式集合，有一部分允许在线测试这些预定义的模式，甚至是自己的模式，因此可以在代码中使用它们之前轻松地验证它们是否正确。

下面开始讲解如何在C++代码中真正使用正则表达式。

## 18.2 regex库

正则表达式库的所有内容都在 `<regex>` 中和std命名空间中。正则表达式库中定义的基本模板类型包括以下几种：

- basic_regex: 表示某个特定正则表达式的对象。
- match_results: 匹配正则表达式的子字符串，包括所有的捕获组。它是sub_match的集合。
- sub_match: 包含输入序列中一个迭代器对的对象，这些迭代器表示匹配的特定捕获组。迭代器对中的一个迭代器指向匹配的捕获组中的第一个字符，另一个迭代器指向匹配的捕获组中最后一个字符后面的字符。它的str()方法把匹配的捕获组返回为字符串。

regex库提供了3个关键算法：regex_match(), regex_search()和regex_replace()。所有这些算法都有不同的版本，用于处理不同的字符串或表示开始和结束的迭代器对。迭代器可以具有以下类型：

- const char*
- const wchar_t*
- string::const_iterator
- wstring::const_iterator

事实上，可使用任何具有双向迭代器行为的迭代器。

regex库还定义了以下两类正则表达式迭代器，这两类正则表达式迭代器适合于查找源字符串中的所有模式：

- regex_iterator: 遍历一个模式在源字符串中出现的所有位置
- regex_token_iterator: 遍历一个模式在源字符串中出现的所有捕获组。

为方便regex库的使用，C++标准定义了很多属于以上模板的类型别名，如下所示：

```cpp
using regex = basic_regex<char>;
using wregex = basic_regex<wchar_t>;

using csub_match = sub_match<const char*>;
using wcsub_match = sub_match<const wchar_t*>;
using ssub_match = sub_match<string::const_iterator>; 
using wcsub_match = sub_match<wstring::const_iterator>;

using cmatch = match_results<const char*>;
using wcmatch = match_results<const wchar_t*>;
using smatch = match_results<string::const_iterator>;
using wsmatch = match_results<wstring::const_iterator>;

using cregex_iterator = regex_iterator<const char*>;
using wcregex_iterator = regex_iterator<const wchar_t*>;
using sregex_iterator = regex_iterator<string::const_iterator>;
using wcregex_iterator = regex_iterator<wstring::const_iterator>;

using cregex_token_iterator = regex_token_iterator<const char*>; 
using wcregex_token_iterator = regex_token_iterator<const wchar_t*>;
using sregex_token_iterator = regex_token_iterator<string::const_iterator>;
using wcregex_token_iterator = regex_token_iterator<wstring::const_iterator>;
```

下面将讲解regex_match(), regex_search()和regex_replace()算法及regex_iterator和regex_token_iterator类。

## 18.3 `regex_match()`


## 18.4 `regex_search()`


## 18.5 `regex_iterator`


## 18.6 `regex_token_iterator`


## 18.7 `regex_replace()` 