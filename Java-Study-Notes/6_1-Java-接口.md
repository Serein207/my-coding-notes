# Java - 接口

---

## 接口的概念

Java中，接口 *(interface)* 不是类，而是对希望符合这个接口的类的一组需求。

Arrays类中的sort方法承诺可以对对象数组进行排序，但要求满足下面这个条件：**对象所属的类必须实现Compared接口**

下面是Comparable接口的代码：

```java
public interface Comparable {
  int compareTo(Object other);
}
```

在这个接口中，compareTo方法是**抽象**
的，它没有具体实现。任何实现Comparable接口的类都需要包含一个compareTo方法，这个方法必须接收一个Object参数，并返回一个整数。否则这个类属于抽象类。

在Java5中，Comparable接口已经提升为一个泛型类型。

```java
public interface Comparable<T> {
  int compareTo(T other);
}
```

例如，在实现 `Comparable<Employee>` 接口的类中，必须提供以下方法

```java
int compareTo(Employee other)
```

接口中所有方法都是public方法，因此在接口中声明方法时，不必提供关键字public。
接口可能包含多个方法，但绝不会拥有实例字段。

现在，假设希望使用Arrays类的sort方法对Employee对象数组进行排序，Employee类就必须实现Comparable接口。

为了让一个类实现一个接口，需要完成下面两个步骤：

1. 将类声明为实现给定的接口
2. 对接口中的所有方法提供定义

要声明一个类实现某个接口，需要使用关键字 `implements`：

```java
class Employee implements Comparable
```

当然，现在Employee类需要提供compareTo方法。假设我们希望根据员工的薪水进行比较。
以下是实现compareTo方法的一个实现：

```java
class Employee implements Comparable {
  public int compareTo(Object otherObject) {
    Employee other = (Employee) otherObject;
    return Double.compare(salary, other.salary);
  }
}
```

> **警告**
>
>在接口声明中，没有将compareTo方法声明为public，这是因为，**接口**中所有方法都自动是public方法。
> 不过，在实现接口时，必须把方法声明为public；否则，编译器将认为这个方法的访问属性是包可访问。

我们可以做得更好一些。可以为泛型Comparable接口提供一个类型参数。

```java
class Employee implements Comparable<Employee> {
  public int compareTo(Employee other) {
    return Double.compare(salary, other.salary);
  }
}
```

---

## 接口的属性

接口不是类。具体来说，不能用 `new` 操作符实例化一个接口：

```java
x=new Comaprable(...);  // error
```

不过，尽管不能构造接口对象，但仍然能声明接口变量：

```java
Comaprable x; // ok
```

接口变量必须引用实现了这个接口的一个类对象：

```java
x=new Employee(...);  // ok
// provided Employee implements Comparable
```

接下来，如同使用 `instanceof` 检查一个对象是都属于某个特定类一样，也可以使用 `instanceof` 检查一个对象是否实现了某个特定的接口：

```java
if(anObject instanceof Comparable){...}
```

与建立类的继承层次结构一样，也可以拓展接口，这里允许有多条接口链，从通用性较高的接口拓展到专用性较高的接口。
例如，假设有一个名为Movable的接口：

```java
public interface Moveable {
  void move(double x, double y);
}
```

然后，可以假设一个名为Powered的接口拓展了以上Movable接口：

```java
public interface Powered extends Movable {
  double milesPerGallon();
}
```

虽然接口中不能包含实例字段，但是可以包含常量。例如：

```java
public interface Powered extends Movable {
  double milesPerGallon();
  double SPPED_LIMIT = 95;  // a public static final constant
}
```

接口中的方法都自动为public，类似地，接口中的字段总是 `public static final` 。

尽管每个类只能有一个超类，但可以实现多个接口。
例如，Java中有一个非常重要的内置接口，名为 `Cloneable` 。如果你的类实现了这个接口，`Object` 类中的 `clone`
方法就可以创建你的类对象的一个完全副本。
如果你希望自己设计的类皆能够克隆又能够比较，只要实现这两个接口就可以了。可以使用逗号将你想要实现的各个接口分隔开。

```java
class Employee implements Cloneable, Comparable
```

> **注释**
>
> 记录和枚举不能拓展其他类，但是它们可以实现接口。
>
> 接口可以是密封的(`sealed`)。与密封类一样，直接子类型必须在 `permits` 子句中声明，或者放在同一个源文件中。

## 接口与抽象类

为什么Java要引入接口概念呢？为什么不将Comparable直接设计成一个抽象类呢？

```java
abstract class Comparable {   // Why not?
  public abstract int compareTo(Object other);
}
```

这样一来，Employee类只需要拓展这个抽象类，并提供compareTo方法：

```java
class Employee extends Comparable {   // Why not?
  public int compareTo(Object other) {...}
}
```

非常遗憾，使用抽象基类表示通用属性存在一个严重的问题。每个类只能拓展一个类。
假设Employee类已经拓展了另一个类，例如Person，它就不能再拓展第二个类了。

```java
class Employee extends Person, Comparable   // error
```

但是每个类可以实现任意多个接口，如下所示：

```java
class Employee extends Person implements Comparable   // ok
```

Java不支持多重继承，一个类只允许拥有一个超类。

## 静态和私有方法

在Java8中，允许在接口中增加静态方法。

目前位置，通常的做法都是将静态方法放在伴随类中。在标准库中，你会看到成对出现的接口和实用工具类，如 `Collection/Collections`
或 `Path/Paths`。

可以由一个URI或者字符串序列构造一个文件或者目录的路径，如 `Paths.get("jdk-17", "conf", "security")` 。在Java11中，`Path`
接口提供了等价的方法：

```java
public interface Path {
  public static Path of(URI uri) {...}
  public static Path of(String first, String... more) {...}
}
```

这样一来，`Path` 类就不再是必要的了。

类似地，实现你自己的接口时，没有理由再为实用工具方法另外提供一个伴随类。

在Java9中，接口的方法可以是private方法。private方法可以是静态方法或是示例方法。
由于私有方法只能在接口本身的方法中使用，所以他们的用途很有限，只是作为接口中其他方法的辅助方法。

## 默认方法

可以为任何接口方法提供一个默认实现。必须用 `default` 修饰符标记这样一个方法。

```java
public interface Comaprable<T> {
  default int compareTo(T other) {return 0;}
  // by default, all elements are the same
}
```

当然，这没有太大用处，因为Comparable的每一个具体实现都会覆盖这个方法。不过有些情况下，默认方法可能很有用。  
例如， `Iterator` 接口用于访问一个数据结构中的元素。这个接口声明了一个 `remove` 方法，如下所示：

```java
public interface Iterator<E> {
  boolean hasNext();
  E next();
  default void remove() {
    throw new UnsupportedOperationException("remove");
  }
  ...
}
```

如果你的迭代器式只读的，就不用操心实现 `remove` 方法。

默认方法的一个重要用法是 ***“接口演化”(interface evolution)***。  
以 `Collection` 接口为例，这个接口作为Java的一部分已经有很多年了。假设很久以前你提供了这样一个类：

```java
public class Bag implements Collection
```

后来，在Java8中，又为这个接口增加了一个 `stream` 方法。

假设 `stream` 方法不是一个默认方法，那么 `Bag` 类将不能编译，因为它没有实现这个新方法。为接口增加一个非默认方法不能保证
***“源代码兼容”(source compatible)***
。或是不重新编译这个类，而只是使用原先的一个包含这个类的JAR文件，如果一个程序在一个 `Bag` 实例上调用 `stream`
方法，就会出现一个 `AbstractMethodError` 。

将方法实现为一个默认方法就可以解决这两个问题。

## 解决默认方法冲突

如果现在一个接口中将一个方法定义为默认方法，然后又在超类或另一个接口中定义了同样的方法，规则如下：

1. 超类优先。如果超类提供了一个具体的方法，同名且有相同参数类型的默认方法会被忽略。
2. 接口冲突。如果一个接口提供了一个默认方法，另一个接口提供了一个同名且有相同参数类型的方法（不论是否是默认方法），必须覆盖这个方法来解决冲突。  

下面来看第二个规则。考虑两个包含 `getName` 方法的接口：

```java
interface Person {
  default String getName() {return "";}
}

interface Named {
  default String getName() {return getClass().getName() + "_" + hashCode();}
}
```

如果有一个类同时实现了这两个接口会怎么样呢？

```java
class Student implements Person, Named {...} 
```

这个类会继承Person和Named接口提供的两个不一样的 `getName` 方法。Java编译器会报告一个错误，让程序员解决二义性问题。
只需要在Student类中提供一个 `getName` 方法即可。在这个方法中，可以选择两个冲突方法中的一个，如下所示：

```java
class Student implements Person, Named {
  public String getName() {return Person.super.getName();}
  ...
} 
```

现在假设Named接口没有为 `getName` 提供默认实现：

```java
interface Named {
  String getName();
}
```

此时，编译器会报错。两个接口如何冲突并不重要，必须由程序员解决二义性。

我们只讨论了两个接口的命名冲突。现在来考虑另一种情况，一个类拓展了一个超类，同时实现了一个接口，并从超类和接口继承了同样的方法。例如，假设Person是一个类，Student定义为：

```java
class Student extends Person implements Named {...
}
```

在这种情况下，只会考虑超类方法，接口的所有默认方法都会被忽略。Student从Person继承了 `getName`
方法，Named接口是否为 `getName` 提供了实现方法都不会带来声明区别，这正是“类优先”规则。

> **警告**
>
> 绝对不能创建一个默认方法重新定义 `Object` 类中的某个方法。由于“类优先规则，这样的方法绝对无法超越 `Object` 的方法。

## 接口与回调

**回调(callback)** 是一种常见的程序设计模式。在这种模式中，可以指定某个特定事物发生时应采取的动作。

在 `java.swing` 包中有一个Timer类，如果希望经过一定时间间隔就得到通知，Timer类就很有用。
例如，程序中有一个时钟，你可以请求每秒通知一次，以便更新时钟的表盘。

构造定时器时，需要设置一个时间间隔，并告诉定时器经过这个时间间隔时要做些什么。

你可以向定时器传入某个类的对西昂，然后，定时器调用这个对象的某个方法。
当然，定时器需要知道要调用哪个方法。它要求你指定一个类的对象，这个类要实现 `java.awt.event` 包的 `ActionListener`
接口。下面是这个接口：

```java
public interface ActionListener {
  void actionPerformed(ActionEvent event);
}
```

当达到指定时间间隔时，定时器就调用 `actionPerformed` 方法。

假设你希望每秒打印一条消息“At the tone, the time is ...”，然后响一声，俺么可以定义一个实现 `ActionListener`
接口的类，然后将想要执行的语句放在 `actionPerformed` 方法中。

```java
class TimePrinter implements ActionEvent {
  public void actionPerformed(ActionEvent event) {
    System.out.println("At the tone, the time is "
        + Instant.ofEpochMilli(event.getWhen()));
    Toolkit.getDefaultToolkit().beep();
  }
}
```

需要注意 `actionPerformed` 方法的 `ActionEvent` 参数，这个参数提供了事件的相关信息，例如，发生这个事件的时间。

接下来，构造这个类的一个对象，并将它传递到 `Timer` 构造器。

```
var listener = new TimePrinter();
Timer t = new Timer(1000, listener);
```

`Timer` 构造器的第一个参数时一个时间间隔，第二个参数时监听器对象。

最后，启动定时器：

```java
t.start();
```

每过1秒就会显示下面的消息，然后响一声铃。

```
At the tone, the time is ...
```

## `Comparator` 接口

假设我们希望按照长度递增的顺序对字符串进行排序，而不是按字典顺序进行排序。  
要处理这种情况，`Arrays.sort` 方法还有第二个版本，接收一个数组和一个比较器作为参数，比较器是实现了 `Comparator` 接口的类的实例。

```java
public interface Comaprator<T> {
  int compare(T first, T second);
}
```

要按照长度比较字符串，可以定义如下一个实现 `Comparator<String>` 的类：

```java
class LengthComparator implements Comparable<String> {
  public int compare(String first, String second) {
    return first.length() - second.length();
  }
}
```

具体完成比较时，需要建立一个实例：

```java
var comp = new LengthComparator();
if (comp.compare(words[i], words[j] > 0) ...
```

将这个调用与 `words[i].compareTo(words[j])` 做个比较。这个 `compare` 方法要在比较器对象上调用，而不是在字符串本身调用。

要对一个数组排序，需要为 `Array.sort()` 方法传入一个 `LengthComparator` 对象：

```java
String[] friends = {"Peter", "Paul", "Mary"};
Arrays.sort(friends, new LengthComparator());
```

## 对象克隆

本节我们会讨论 `Cloneable` 接口，这个接口表示一个类提供了一个安全的clone方法。

如果希望一个新对象，它的初始状态与原始相同，但是之后它们的状态可能不同，这种情况下就要使用 `clone` 方法（即克隆）。

`clone` 方法是 `Object` 的一个protected方法，你的代码不能直接调用这个方法，类只能克隆自己的对象。

如果浅拷贝不能满足共享的子对象是不可变的，需要重新定义 `clone` 方法来建立深拷贝，克隆所有子对象。

对于每一个类，需要确定以下选项是否成立：

1. 默认的 `clone` ·方法就能满足要求；
2. 可以在可变的子对象上调用 `clone` 来弥补默认的 `clone` 方法；
3. 不该使用 `clone``。

实际上第三个选项是默认选项。如果选择1或2，类必须：

1. 实现 `Cloneable` 接口；
2. 重新定义 `clone` 方法，并指定public访问修饰符。

在这里，`Cloneable` 接口的出现与接口的正常使用并没有关系。它没有指定 `clone` 方法，这个方法是从Object类继承的。
这个接口只是作为一个标记，指示类设计者了解克隆过程。如果一个对象请求克隆，但是没有实现这个接口，就会生成一个异常。

> **注释**
>
> `Cloneable` 接口是Java提供的少数 **标记接口** 之一。`Comparable`
>
等接口的通常用途是确保一个类实现一个特定的方法或一组方法。标记接口不包含任何方法，它唯一的作用就是允许在类型查询中使用 `instanceof`：
> ```java
> if (obj instanceof Cloneable) ...
>```
> 建议你自己的程序中不要使用标记接口。

即使 `clone` 的默认（浅拷贝）能够满足要求，还是需要实现 `Cloneable` 接口，将 `clone` 重新定义为public，再调用 `super.clone()`
。下面给出一个例子：

```java
class Employee implements Cloneable {
  public Employee clone() throws CloneNotSupportedException {
    return (Employee) super.clone();
  }
  ...
}
```

与 `Object.clone` 提供的浅拷贝相比，还需要更多工作，克隆对象中的实例字段。

下面来看创建深拷贝的 `clone` 方法的一个例子：

```java
class Employee implements Cloneable {
  public Employee clone() throws CloneNotSupportedException {
    Employee cloned = (Employee) super.clone();

    clone.hireDay = (Date) hireDay.clone();
    return cloned;
  }
}
```

Object类的clone方法有可能抛出一个 `CloneNotSupportedExpection`
，如果在一个对象上调用clone，但这个对象的类并没有实现 `Cloneable`
接口，就会发生这种情况。当然，Employee和Date类实现了 `Cloneable` 接口，所以不会抛出这个异常。
不过，编译器不知道这一点，因此，我们声明了这个异常：

```java
public Employee clone()throws CloneNotSupportedExpection
```

必须当心子类的克隆。例如，一旦为Employee类定义了clone方法，任何人都可能用他来克隆Manager对象。
这需要保证Manager没有需要深拷贝的字段或不可克隆的字段。
