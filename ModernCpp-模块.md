# Modern C++ 模块

 > 在C++20引入模块之前，头文件是用来为可重用代码段提供接口的。但是头文件确实有很多问题，比如避免多次包含同一个头文件，以及确保头文件以正确的顺序包含。此外，包含头文件会新增上万行编译器必须处理的代码。如果多个源文件都包含头文件，那么这些编译单元就会变得更大。
>
> 模块解决了所有这些问题，甚至更多。模块导入的顺序并不重要。模块只需要编译一次，而不像头文件那样要反复编译，因此可以大大缩短编译时间。模块中的某些修改不会触发该模块的用户重新编译。模块不受任何外部定义的宏的影响，并且在模块内部定义的任何宏对模块外部的任何代码都不可见。

- [Modern C++ 模块](#modern-c-模块)
  - [模块接口文件](#模块接口文件)
  - [模块实现文件](#模块实现文件)
  - [从实现中分离接口](#从实现中分离接口)
  - [可见性和可访问性](#可见性和可访问性)
  - [子模块](#子模块)
  - [模块分区](#模块分区)
    - [实现分区](#实现分区)
  - [头文件单元](#头文件单元)

## 模块接口文件

模块接口文件，是为模块提供的功能定义的接口。MSVC的模块接口后缀名为 `.ixx`。模块接口以声明开头，声明该文件正在定义一个具有特定名称的模块。这成为模块声明。模块的名称可以是任何有效的C++标识符。名称中可以含点 `.`，但不能以点开头或结尾，也不能在一行中包含连续的点。有效名称的示例有dataModel, myCompany.dataModel, myCompony.dataModel.core, dataModel_core等。

模块需要显式地声明要导出的内容，即当用户代码导入模块时，哪些内容应该是课件的。使用 `export` 关键字从模块中导出实体（比如类、函数、常量、其他模块等）。任何没有从模块中导出的内容，只在该模块中可见。所有导出实体的集合称为模块接口。

下面是一个名为 `Person.ixx` 的模块接口文件的示例：

```cpp
export module person;   // module declaration

import <string>;        // import declaration

export class Person {
 public:
   Person(std::string firstName, std::string lastName)
     : m_firstName { std::move(firstName) }
     , m_lastName { std::move(lastName) } {}

   const std::string& getFirstName() const { return m_firstName; }
   const std::string& getLastName() const { return m_lastName; }
 private:
   std::string m_firstName;
   std::string m_lastName;
};
```

可以通过以下方式导入Person模块，来使用Person类（`test.cpp`):

```cpp
import person;
import <iostream>;
import <string>;

int main() {
  Person person { "Kole", "Webb" };
  std::cout << person.getFirstName() << " " << person.getLastName() << std::endl;
}
```

所有的C++头文件都是可导入头文件，可以通过 `import` 声明导入。C++中可用的C的头文件不能保证是可导入的。为了安全起见，在C的头文件中使用 `#include` 替换 `import` 声明。这样的 `#include` 指令应该放在全局模块片段中，它必须出现在任何命名模块之前，以匿名模块的声明开始。全局模块片段只能包含预处理指令，比如 `#include`。这样的全局模块片段和注释是唯一允许出现在命名模块声明之前的内容。比如，如果需要使用C的头文件 `<cstddef>` 中的功能，可以如下所示:

```cpp
module;               // start of the global module fragment
#include <cstddef>    // include legacy header files

export module person; // named module declaration

import <string>;

export class Person { ... };
```

从已命名模块声明到文件末尾的所有内容称为 **模块权限**。

几乎所有内容都可以从模块导出，只要它有一个名称。比如类定义、函数原型、枚举类类型、`using` 声明、命名空间等。如果使用 `export` 关键字显式导出命名空间，则该名称空间中的所有内容也将自动导出。比如，下面的代码片段导出整个DataModel命名空间。因此，没有必要显式地导出单个类和类型别名：

```cpp
export module dataModel;
import <vector>;
export namespace DataModel {
  class Person {...};
  class Address {...};
  using Persons = std::vector<Person>;
}
```

还可以使用导出块(export block)导出整个声明块：

```cpp
export {
  namespace DataModel {
    class Person {...};
    class Address {...};
    using Persons = std::vector<Person>;
  }
}
```

## 模块实现文件

一个模块可以被拆分为一个模块接口文件和一个或多个模块实现文件。模块实现文件通常以 `.cpp` 作为拓展名。可以自由决定将哪些实现移到模块实现文件中，以及哪些实现保留函数原型、类定义等。

模块实现文件同样包含一个已命名的模块声明，用于指定实现所针对的模块，但没有 `export` 关键字。比如，可以将前面的person模块拆分为接口和实现文件，如下所示：

- 接口文件 person.ixx
  ```cpp
  export module person;   // module declaration

  import <string>;        // import declaration

  export class Person {
   public:
     Person(std::string firstName, std::string lastName);

     const std::string& getFirstName() const;
     const std::string& getLastName() const;
   private:
     std::string m_firstName;
     std::string m_lastName;
  };
  ```

- 实现文件 person.cpp
  ```cpp
  module person;

  Person::Person(std::string firstName, std::string lastName)
       : m_firstName { std::move(firstName) }
       , m_lastName { std::move(lastName) } {}

  const std::string& Person::getFirstName() const { return m_firstName; }
  const std::string& Person::getLastName() const { return m_lastName; }
  ```

注意，实现文件没有person模块导入声明。 `module person` 的声明隐式包含 `import person` 的声明。还需要注意的是，实现文件没有 `<string>` 的导入声明，由于隐式的 `import person`，并且因为这个实现文件是同一个person模块的一部分，所以它隐式地从模块接口文件中继承了 `<string>` 的导入声明。与之相反，向test.cpp文件添加 `import person` 的声明不会隐式继承 `<string>` 的导入声明，因为test.cpp不是person模块的一部分。

> **注意**
>
> 模块接口和模块实现文件中的所有导入声明都必须唯一文件的顶部，在已命名模块声明之后，但需要在任何其他声明之前。

类似于模块接口文件，如果需要在模块实现文件中使用 `#include` 导入已有的头文件，则应将它们放入全局模块片段中，并使用与模块接口文件相同的语法。

> **警告**
>
> 模块实现文件不能导出任何内容，只有模块接口文件可以。

## 从实现中分离接口

模块接口的工作方式不同于头文件。模块接口(module interface)仅由类定义、函数原型等组成，虽然这些实现在模块接口文件中，但模块接口中不包含任何函数或方法的实现。因此，只要变更不涉及模块接口的部分，变更模块接口文件中的函数、方法的实现是不需要重新编译该模块的，比如函数签名、参数签名、返回值签名等。但有两个例外，使用 `inline` 标记的函数或方法和模板的定义，对于这两者，编译器需要在编译时知道他们的完整实现。因此，内联函数、方法或模板定义的任何变更都可能引发用户代码的重新编译。

尽管从技术上将，不再需要将接口从实现分离，但仍建议这么做，目的是创建整洁和易读的接口。将接口从实现文件中分离可以通过几种方式实现。一种是将模块拆分为接口文件和实现文件，如上一节所述；另一种是在单个模块的接口文件中拆分接口和实现，如下所述：

```cpp
export module person;   
import <string>;     
// class definitions
export class Person { 
 public:
   Person(std::string firstName, std::string lastName);

   const std::string& getFirstName() const;
   const std::string& getLastName() const;
 private:
   std::string m_firstName;
   std::string m_lastName;
};
// class implementations
Person::Person(std::string firstName, std::string lastName)
     : m_firstName { std::move(firstName) }
     , m_lastName { std::move(lastName) } {}

const std::string& Person::getFirstName() const { return m_firstName; }
const std::string& Person::getLastName() const { return m_lastName; }
```

## 可见性和可访问性

当在不属于person模块的另一个源文件（比如test.cpp）中导入person模块时，不会隐式地从person模块的接口文件中继承 `<string>` 的导入声明。因此，如果在test.cpp中没有显式导入 `<string>` ，那么 `std::string` 是不可见的。

尽管如此，即使在test.cpp中没有显式导入 `<string>`，下面的代码也可以正常工作。

```cpp
const auto& lastName { person.getLastName() };
auto length { lastName.length() };
```

为什么会这样呢？在C++中，实体的可见性（visibility）和可访问性（accessibility）是有区别的。通过导入person模块，`<string>` 可以被访问，但是不可见；可访问类的成员函数自动变得可见。所有这些都意味着可以使用 `<string>` 中的某些功能，比如自动类型推导和调用 `length()` 等方法，将 `getLastName()` 的结果存储在变量中。

为了使 `std::string` 在test.cpp中可见，我们必须显式导入 `<string>`。当想要使用时，这样的显式导入是必要的。因为运算符 `<<` 不是 `std::string` 的方法，而是只有通过导入 `<string>` 才可见的非成员函数。

```cpp
std::cout << person.getLastName() << std::endl;
```

## 子模块

C++标准中并没有描述子模块(submodules)，但是标准允许在模块名中使用点 `.`，这就可以在任何想要的层次结构中构建模块。比如，之前章节给出的命名空间DataModel示例如下：

```cpp
export module dataModel;
import <vector>;
export namespace DataModel {
  class Person {...};
  class Address {...};
  using Persons = std::vector<Person>;
}
```

类Person和类Address都位于命名空间DataModel和模块dataModel中。这可以通过定义两个子模块来重新构造 dataModel.person和dataModel.address。子模块dataModel.person的模块接口文件如下：

```cpp
export module dataModel.person;
export namespace DataModel { class Person {...}; }
```

下面是子模块dataModel.address的接口文件：

```cpp
export module dataModel.address;
export namespace DataModel { class Address {...}; }
```

最后，模块dataModel的定义如下。它导入并导出两个子模块：

```cpp
export module dataModel;
export import dataModel.person;
export import dataModel.address;
import <vector>;
export namespace DataModel { using Persons = std::vector<Person>; }
```

当然，子模块中类的方法实现也可以放入模块实现文件中。比如，假设类Address有一个默认构造函数，这个实现可以放在文件 dataModel.address.cpp 中：

```cpp
module dataModel.address;
import <iostream>;
DataModel::Address::Address() { std::cout << "Hello" << std::endl; }
```

使用子模块构造函数的代码的好处是：用户可以一次导入它们想要使用的模块特定部分或全部内容。例如，如果用户只对Address的使用感兴趣，那么下面的 `import` 声明就足够了：

```cpp
import dataModel.address;
```

如果用户想要使用模块dataModel的所有内容，那么下面的导入声明是最简单的：

```cpp
import dataModel;
```

## 模块分区

另一种构建模块的方法是将他们拆分到单独的分区中。子模块和分区之间的区别是：子模块的构造对模块的用户来说是可见的，允许用户有选择地只导入想使用的子模块。另一方面，分区用于在内部构造模块。分区不对模块的用户公开。在模块接口分区文件(module interface partition file)中声明的所有分区，最终必须由主模块接口文件导出。一个模块通常只有一个这样的主模块接口文件，那就是包含导出模块名称的接口文件。

模块分区名创建时用冒号将模块名和分区名隔开。分区名通常可以是任何合法的标识符。例如，前一节的DataModel模块可以使用分区而不是子模块来重新构造。下面是在模块接口分区文件dataModel.person.ixx的person分区：

```cpp
export module dataModel:person;
export namespace DataModel { class Person {...}; }
```

下面是包含默认构造函数的address分区：

```cpp
export module dataModel:address;
export namespace DataModel {
  class Address {
   public:
     Address();
     //...
  }; 
}
```

但是，将实现文件与分区结合使用时需注意：只有一个具有特定分区名称的文件。因此，以下声明开头的实现文件的格式是不正确的：

```cpp
module dataModel:address;
```

相反，可以将address分区的实现放在dataModel模块的实现文件中。示例如下：

```cpp
module dataModel;  // not dataModel:address
import <iostream>;
DataModel::Address::Address() { std::cout << "Hello" << std::endl; }
```

多个文件不能具有相同的分区名称。因此，具有多个相同分区名的模块接口分区文件是非法的，并且对于模块接口分区文件中声明的实现，也不能出现在具有相同分区名的实现文件中。只需要将这些实现放在模块的实现文件中即可。

需要记住的重点是，在编写分区接口中的模块时，每个接口的分区最终都必须由主模块接口文件直接或间接导出。要导出分区，只需指定分区名称，比如 `import :person`。而 `import dataModel:person` 这样则是非法的。记住，分区不会对模块的用户公开，分区只能在内部构造模块。因此，用户不能导入特定的分区，只能导入整个模块。分区只能在模块内部导入，因此在冒号之前指定模块名称是多余且非法的。下面是dataModel模块的主模块接口文件：

```cpp
export module dataModel;  // dataModel module (primary module interface file)
export import :person;    // import and export person partition
export import :address;   // import and export address partition
import <vector>;
export namespace DataModel { using Person = std::vector<Person>; }
```

这个分区结构的dataModel模块可以按如下方式使用：

```cpp
import dataModel;
int main() {
  DataModel::Person person;
}
```

> **注意**
>
> 分区用于在内部构造模块，分区在模块外部不可见。因此，模块的用户无法导入特定的分区，只能导入整个模块。

前面已经解释过模块名称声明中隐式包括导入名称声明，但分区不是这样的

比如dataModel:person分区没有隐式导入dataModel的声明。在本例中，甚至不允许在dataModel:person接口分区文件中显式导入dataModel。因为这样做会导致环形依赖，两个文件互相包含。

要打破这种环形依赖，可以将dataModel:person分区需要的功能从dataModel接口文件中转移至另一个分区，随后通过dataModel:person接口分区文件和dataModel接口文件导入该分区。

### 实现分区

分区不需要在模块的接口分区文件中声明，但可以在模块的实现分区文件（.cpp）中声明。这种情况下，它是实现分区，有时也称为内部分区。主模块的接口文件不会导出这样的分区。

例如，有以下math主模块的接口文件（math.ixx）

```cpp
export module math;   // math module declaration
export namespace Math {
  double superLog(double z, double b);
  double lerchZeta(double lambda, double alpha, double s);
}
```

math函数的实现需要一些辅助函数，这些辅助函数不能被模块导出。实现分区是放置此类辅助函数的理想场所。下面在名为math_helper.cpp文件中定义了这类实现分区：

```cpp
module math:details;  // math:details implementation partition
double someHelperFunction(double a) { return ...; }
```

其他的math模块实现文件可以通过导入此实现分区来访问这些辅助函数。比如，math模块的实现文件（math.cpp）可能如下所示：

```cpp
module math;
import :details;
double Math::superLog(double z, double b) { return...; }
double Math::lerchZeta(double lambda, double alpha, double s) { return...; }
```

当然，只有当多个其他源文件都使用这些辅助函数的情况下，将实现分区与辅助函数一起使用才会有效果。

## 头文件单元

如果有头文件，比如定义Person类的person.h头文件，则可以通过将其转换为适当的模块（person.ixx)进行模块化，并导入声明使其对用户代码可用。但是，有时不能模块化这类头文件。在这种情况下，可以如下所示，直接导入头文件：

```cpp
import "person.h";
```

有了这样的声明，person.h头文件中的所有内容都将隐式导出。此外，头文件中定义的宏对于用户代码可见，但实际模块中却并非如此。

相比于 `#include` 头文件，导入头文件可以提高构建效率，因为person.h头文件将隐式转换为模块，因此只需要编译一次，而不是在源文件中 `#include` 头文件时每次都进行编译。具体请参阅编译器文档，了解如何使用头文件单元。