# C++11：String

---
***contents***
- [C++11：String](#c11string)
  - [C++风格字符串 `std::string`](#c风格字符串-stdstring)
  - [原始字面量 *(raw string literal)*](#原始字面量-raw-string-literal)

---

## C++风格字符串 `std::string`

我们先来学习一下STL容器 `std::string`

* 构造函数
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

* 赋值操作
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

* 字符串拼接
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

* 查找
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

* 替换
```cpp
std::string str1 = "abcdefg";

str1.replace(1, 3, "1111");  // 从1号位置起 3个字符替换为"1111"

std::cout << "str1 = " << str1 << std::endl;  // a1111efg
```

* 比较
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

* 字符串存取
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

* 插入和删除
```cpp
std::string str = "hello";

// 插入
str.insert(1, "111");
std::cout << "str = " << str << std::endl;  // h111ello

// 删除
str.erase(1, 3);
std::cout << "str = " << str << std::endl;  // hello
```

* 子串
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

---

## 原始字面量 *(raw string literal)*

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

</br>

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

---
**edit & arrange**  Serein