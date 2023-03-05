# Modern C++ 异常

> C++程序不可避免地会遇到错误。例如，程序可能无法打开某个文件，网络连接可能断开，或者用户可能输入不正确的值。C++语言提供了“异常”的特性，用来处理这些不正常但能预料的情况。
>
> 在C++中，并不强制异常处理。在Java中是强制的，没有给出可能抛出异常列表的函数不允许抛出任何异常。在C++中，函数可抛出它想抛出的任何异常，除非指定不会抛出任何异常（使用 `noexcept` 关键字）。

- [Modern C++ 异常](#modern-c-异常)
  - [异常机制](#异常机制)
    - [抛出和捕获异常](#抛出和捕获异常)
    - [异常类型](#异常类型)
    - [按 `const` 引用捕获异常对象](#按-const-引用捕获异常对象)
    - [抛出并捕获多个异常](#抛出并捕获多个异常)
      - [1. 匹配和 `const`](#1-匹配和-const)
      - [2. 匹配所有异常](#2-匹配所有异常)
    - [未捕获的异常](#未捕获的异常)
    - [`noexcept` 说明符](#noexcept-说明符)


## 异常机制

在文件的输入输出中经常发生异常情况。下面的函数打开一个文件，从这个文件中读取整数列表，然后将整数存储在 `std::vector` 中。其中缺少错误处理代码：

```cpp
std::vector<int> readIntegerFile(std::string_view filename) {
  std::ifstream inputStream { filename.data() };
  std::vector<int> integers;
  int temp;
  while (inputStream >> temp) {
    integers.push_back(temp);
  }
  return integers;
}
```

下面的代码从 `ifstream` 持续地读取，一直到文件地结尾或发生错误为止。

```cpp
while (inputStream >> temp)
```

如果 `<<` 运算符发生错误，就会设置 `ifstream` 对象的错误位。在此情况下，`bool()` 转换运算符将返回 `false` ，while循环将终止。

可以这样使用 `readIntegerFile()`：

```cpp
const std::string filename { "Integer.txt" };
std::vector<int> myInt { readIntegerFile(filename) };
for (const auto& element : myInt) {
  std::cout << element << " ";
}
std::cout << std::endl;
```

### 抛出和捕获异常

为了使用异常，要在程序中包括两部分：处理异常的try/catch结构和抛出异常的throw语句。二者都必须以某种形式出现，以进行异常处理。

try/catch结构如下所示：

```cpp
try {
  // ... code with may result in an exception being thrown
} catch (exception_type1 exception_name) {
  // ... code with response to the exception of type1
} catch (exception_type2 exception_name) {
  // ... code with response to the exception of type2
}
// ... remaining code
```

导致抛出异常的代码可能直接包含 `throw` 语句，也可能调用一个函数，这个函数可能直接抛出异常，也可能经过多层调用后抛出，每层调用一个抛出异常的函数。

如果没有抛出异常，`catch` 语句之后或者在抛出异常的函数后的代码不会执行，根据抛出的异常的类型，控制会立刻转移到对应的catch块。

如果 `catch` 块没有执行控制转移（例如从函数中返回，抛出新的异常或者重新抛出异常），那么会执行 `catch` 块最后语句之后的代码。

演示异常处理的最简单的示例是避免除0。这个示例抛出一个 `std::invalid_argument` 类型的异常，它定义在 `<stdexcept>` 中。

```cpp
double safeDivide(double num, double den) {
  if (den == 0) 
    throw std::invalid_argument("denominator is zero");
  return num / den;
}

int main() {
  try {
    std::cout << safeDivide(5, 2) << std::endl;
    std::cout << safeDivide(5, 0) << std::endl;
    std::cout << safeDivide(3, 3) << std::endl;
  } catch (const std::invalid_argument& e) {
    std::cout << "exception: " << e.what() << std::endl;
  }
  return 0;
}
```

输出如下所示：

```
2.5
exception: denominator is zero
```

`throw` 是C++关键字，这是抛出异常的唯一方法。在前面的代码片段中，抛出了一个 `std::invalid_argument` 的新对象。这是C++标准库提供的标准异常。标准库中的所有异常构成了一个层次结构，详见之后的内容。该结构中每个类都支持 `what()` 方法，该方法返回一个描述异常的 `const char*` 字符串。该字符串在异常的构造函数中提供。

回到 `readInteger()` 函数，最容易发生的问题就是打开文件失败。这正是需要抛出异常的情况，代码抛出一个 `std::exception` 类型的异常，这种异常类型定义在 `<exception>` 中。如果文件打开失败：

```cpp
std::vector<int> readIntegerFile(std::string_view filename) {
  std::ifstream inputStream { filename.data() };
  if (inputStream.fail()) {
    throw exception {};
  }

  std::vector<int> integers;
  int temp;
  while (inputStream >> temp) {
    integers.push_back(temp);
  }
  return integers;
}
```

> **注意**
>
> 始终在代码文档中记录函数可能抛出的异常，因为函数的用户需要了解可能抛出哪些异常，从而加以适当处理。

如果函数打开文件失败并执行了 `throw exception {}` 语句，那么函数的其余部分将被跳过，把控制转交给最近的异常处理程序。

如果还编写了处理异常的代码，这种情况下抛出异常效果最好。异常处理是这样一种方法：try执行一块代码，并用另一块代码处理可能发生的任何错误。在下面的 `main()` 函数中， `catch` 语句捕获任何被 `try` 块抛出的 `exception` 类型异常，并输出错误消息。如果 `try` 块结束时没有抛出异常，`catch` 块将被忽略。

```cpp
int main() {
  const std::string filename { "IntegerFile.txt" };
  std::vector<int> myInt;
  try {
    myInt = readIntegerFile(filename);
  } catch (const exception& e) {
    std::cerr <<"Unable to open file " << filename << std::endl;
    return 1;
  }
  for (const auto& element : myInt) {
    std::cout << element <<  " ";
  }
  std::cout << std::endl;
}
```

### 异常类型

可抛出任何类型的异常。可以抛出一个 `std::exception` 类型的对象，但异常未必是对象，也可以抛出一个简单的int值，如下所示：

```cpp
std::vector<int> readIntegerFile(std::string_view filename) {
  std::ifstream inputStream { filename.data() };
  if (inputStream.fail()) {
    throw 5;
  }
  //...
}
```

此后必须修改 `catch` 语句：

```cpp
try {
  myInt = readIntegerFile(filename);
} catch (const int e) {
  std::cerr << std::format("Unable to open file {} (Error Code {})", filename, e) << std::endl;
  return 1;
}
```

另外，也可以抛出一个C风格的 `const char*` 字符串。这项技术有时有用，因为字符串可包含与异常相关的信息。

```cpp
std::vector<int> readIntegerFile(std::string_view filename) {
  std::ifstream inputStream { filename.data() };
  if (inputStream.fail()) {
    throw "Unable to open file";
  }
  //...
}
```

当捕获 `const char*` 异常时，可输出结果：

```cpp
try {
  myInt = readIntegerFile(filename);
} catch (const char* e) {
  std::cerr << e << std::endl;
  return 1;
}
```

尽管前面有这样的示例，但通常应将对象作为异常抛出，原因有以下两点：

- 对象的类名可传递信息。
- 对象可存储信息，包括描述异常的字符串。

C++标准库在类层次结构中定义了许多预定义的异常类，也可编写自己的异常类，并将它们放入标准层次结构中。在之后会就此详细讨论。

### 按 `const` 引用捕获异常对象

在前面的示例中，`readIntegerFile()` 抛出了一个 `std::exception` 类型的对象。`catch` 处理如下所示：

```cpp
} catch (const std::exception& e) {
```

然而，再次并没有要求按 `const` 引用捕获对象。可以按值捕获对象，如下所示：

```cpp
} catch (std::exception e) {
```

此外，也可按非 `const` 引用捕获对象：

```cpp
} catch (std::exception& e) {
```

另外，如 `const char*` 示例所示，只要指向异常的指针被抛出，就可以捕获他。

> **注意**
>
> 建议按 `const` 引用捕获对象，可避免按值捕获时可能出现的对象截断。

### 抛出并捕获多个异常

打开文件失败不是 `readIntegerFile()` 遇到的唯一问题。如果格式不正确，读取文件中的数据也会导致错误。下面是 `readIntegerFile()` 的一个实现，如果无法打开文件，或者无法正确读取数据，就会抛出异常。这里使用从 `std::exception` 派生的 `std::runtime_error`，它允许你在构造函数中指定描述字符串。我们在 `<stdexcept>` 中定义 `std::runtime_error` 异常。

```cpp
std::vector<int> readIntegerFile(std::string_view filename) {
  std::ifstream inputStream { filename.data() };
  if (inputStream.fail()) {
    throw std::runtime_error { "Unable to open file" };
  }

  std::vector<int> integers;
  int temp;
  while (inputStream >> temp) {
    integer.push_back(temp);
  }

  if (!inputStream.eof()) {
    throw std::runtime_error { "Unable to read file" };
  }

  return integers;
}
```

`std::invalid_argument` 和 `std::runtime_error` 类没有公共的默认构造函数，只有以字符串作为参数的构造函数。

现在 `main()` 可调用两个 `catch` 语句捕获 `std::invalid_argument` 和 `std::runtime_error` 异常：

```cpp
try {
  myInt = readIntegerFile(filename);
} catch (const std::invalid_argument& e) {
  std::cerr << e.what() << std::endl;
  return 1;
} catch (const std::runtime_error& e) {
  std::cerr << e.what() << std::endl;
  return 2;
}
```

如果异常在 `try` 块内部抛出，编译器将使用恰当的 `catch` 处理程序与异常类型匹配。因此，如果 `readIntegerFile()` 无法打开文件并抛出 `std::invalid_argument` 异常，第一个 `catch` 语句将捕获这个异常。如果 `readIntegerFile()` 无法正确文件并抛出 `std::runtime_error` 异常，第二个 `catch` 语句将捕获这个异常。

#### 1. 匹配和 `const` 

对于想要捕获的异常类型而言，增加 `const` 属性不会影响匹配的目的。也就是说，这一行可以与 `std::runtime_error` 类型的任何异常匹配：

```cpp
} catch (const std::runtime_error& e) {
```

下面这行也可以与 `std::runtime_error` 类型的任何异常匹配：

```cpp
} catch (std::runtime_error& e) {
```

#### 2. 匹配所有异常

可用特定语法编写与所有异常匹配的 `catch` 语句：

```cpp
try {
  myInt = readIntegerFile(filename);
} catch (...) {
  std::cerr << "Error reading or opening file" << std::endl;
  return 1;
}
```

这三个点并非省略符，而是与所有异常类型匹配的通配符。当调用缺乏文档的代码时，可以用这一语法确保捕获所有可能的异常。然而，如果有被抛出的一组异常的完整信息，这种技术并不理想，因为它将所有异常都同等对待。

与所有异常匹配的catch块可以用做默认的catch处理程序。当异常抛出时，会按在代码中显示顺序查找 `catch` 处理程序。下例用 `catch` 处理程序显式处理 `std::invalid_argument` 和 `std::runtime_error` 异常，并用默认的catch处理程序处理其他异常：

```cpp
try {
  // code that can throw exceptions
} catch (const std::invalid_argument& e) {
  // handle invalid_argument exception
} catch (const std::runtime_error& e) {
  // handle runtime_error exception
} catch (...) {
  // handle all other exceptions
}
```

### 未捕获的异常

如果程序抛出的异常没有捕获，程序将终止。可对 `main()` 函数使用try/catch结构，以捕获所有未经处理的异常，如下所示：

```cpp
try {
  main(argc, argv);
} catch (...) {
  // issue error message and terminate program
}
// normal terminal code
```

然而，这一行为通常并非我们希望的。异常的作用在于给程序一个机会，以处理和修正不希望看到的或不曾预期的情况。

> 捕获并处理程序中可能抛出的所有异常

如果存在未捕获异常，程序行为也可能发生变化。当程序遇到未捕获的异常时，会调用内建的 `terminate()` 函数，这个函数调用 `<cstdlib>` 中的 `abort()` 函数来终止程序。可调用 `set_terminate()` 函数来设置自己的 `terminate_handler()`，这个函数采用指向回调函数（既没有参数，也没有返回值）的指针作为参数。 以上三个函数都在 `<exception>` 中声明。下面的代码展示了它们的用法：

```cpp
try {
  main(argc, argv);
} catch (...) {
  if (std::terminate_handler != nullptr)
    std::terminate_handler();
  else
    std::terminate();
}
// normal termination code
```

回调函数必须终止程序（使用 `abort()` 或 `_Exit()` 函数），错误时无法忽略的。还有个 `exit()` 函数，它会接收返回给操作系统的参数，并调用析构函数来清理资源，但不建议在 `std::terminate_handler` 中调用 `exit()` 函数。

在退出之前可使用 `std::terminate_handler` 打印有用的错误消息。下例中，回调答应错误消息并用 `_Exit()` 函数退出程序：

```cpp
[[noreturn]] void myTerminate() {
  std::cout << "Uncaught exception!" << std::endl;
  _Exit(1); 
}

int main() {
  std::set_terminate(myTerminate);

  const std::string filename { "IntegerFile.txt" };
  std::vector<int> myInt;

  for (const auto& element : myInt) {
    std::cout << element << " ";
  }
  std::cout << std::endl;
}
```

当设置新的 `terminate_handler()` 时，`set_terminate()` 并返回旧的 `terminate_handler()`。`terminate_handler()` 被应用于整个程序，因此最好保存旧的 `terminate_handler()`。上面的示例中不需要重新保存，因为整个程序都需要新的 `terminate_handler()` 。

使用 `terminate_handler()` 并不是一种非常有效的处理异常的办法。建议分别捕获并处理每个异常，以提供更精确的错误处理。

### `noexcept` 说明符