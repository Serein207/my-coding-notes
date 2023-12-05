# 内部类

**内部类(inner class)** 是定义在另一个类中的类。需要使用内部类有两个原因：

- 内部类可以对同一个包中的其他类隐藏。
- 内部类方法可以访问定义这些方法的作用域中的数据，包括原本私有的数据。

内部类原先对于简洁地实现回调非常重要，不过如今lambda表达式在这方面可以做得更好。
但内部类对于构建代码还是很有用的。

Java内部类的对象会有一个隐式引用，指向实例化这个对象的外部类对象。通过这个指针，它可以访问外部对象的全部状态。
静态内部类没有这个附加指针，所以Java的静态内部类就相当于C++中的嵌套类。

## 使用内部类访问对象状态

内部类的语法相当复杂。我们选择一个简单但不太实用的例子来说明内部类的使用。
我们将重构TimerTest示例，提取一个TalkingClock类。
构造一个语音时钟时需要提供两个参数：发出通知的间隔和开关铃声的标志。

```java
public class TalkingClock {
  private int interval;
  private boolean beep;
  public TalkingClock(int interval, boolean beep) {...}
  public void start() {...}

  // an inner class
  public class TimePrinter implements ActionListener {
    ...
  }
}
```

需要注意，这里的 `TimePrinter` 类位于 `TalkingClock` 类内部。
这并不意味着每个 `TalkingClock` 都有一个 `TimePrinter` 实例字段。
如前所示， `TimePrinter` 对象是由 `TalkingClock` 类的方法构造的。

下面是 `TimePrinter` 类的详细内容。需要注意一点， `actionPerformed` 方法在发出铃声之前会检查beep标志。

```java
public class TimePrinter implements ActionListener {
  public void actionPerformed(ActionEvent event) {
    System.out.println("At the tone, the time is "
        + Instant.ofEpochMilli(event.getWhen()));
    if (beep) Toolkit.getDefaultToolkit().beep();
  }
}
```

令人惊讶的事情发生了。 `TimePrinter` 类没有实例字段或者名为beep的变量，
实际上，beep指示创建这个 `TimePrinter` 的 `TalkingClock` 对象的字段。
可以看到，一个内部类的方法可以访问自身的实例字段，也可以访问创建它的外部类对象的实例字段。
为此，内部类的对象总是有一个隐式引用，指向创建它的外部变量。

这个引用在内部类的定义中是不可见的。不过，为了说明这个概念，我们将外部类对象的引用称为 *outer* 。
于是 `actionPerformed` 方法将等价于以下代码：

```java
public class TimePrinter implements ActionListener {
  public void actionPerformed(ActionEvent event) {
    System.out.println("At the tone, the time is "
        + Instant.ofEpochMilli(event.getWhen()));
    if (outer.beep) Toolkit.getDefaultToolkit().beep();
  }
}
```

外部类的引用在构造器中设置。编译器会修改所有的内部类的构造器，添加一个对应外部类引用的参数。
因为 `TimePrinter` 类没有定义构造器，所以编译器为这个类生成了一个无参数构造器，生成的代码如下所示：

```java
public TimePrinter(TalkingClock clock) {
  outer = clock;
}
```

再次强调，注意 *outer* 不是Java关键字。我们只是用它说明内部类的有关机制。

在 `start` 方法中构造一个 `TimePrinter` 对象后，编译器就会将当前语音时钟的 `this` 引用传递给这个构造器：

```java
var listener = new TimePrinter(this);
```

下面我们再来看访问控制。如果 `TimePrinter` 类是一个普通的类，它就需要通过 `TalkingClock` 类的公共方法访问beep表示标志，
而使用内部类是一个改进，现在就不再需要提供只有另一个类感兴趣的访问器了。

> **注释**
>
> 我们也可以把 `TimePrinter` 类声明为私有。这样一来，只有 `TalkingClock` 方法才能构造 `TimePrinter` 对象。
> 只有内部类可以是私有的，而常规类可以有包可见性或公共可见性。

## 内部类的特殊语法规则

在上一节中，我们解释了内部类有一个外部类的引用，我们把它叫做 _outer_ 。
事实上，这个外部类引用的正规语法还要复杂些。表达式

```java
OuterClass.this
```

表示外部类引用。
例如，可以像下面这样编写 `TimePrinter` 内部类的 `actionPerformed` 方法：

```java
public void actionPerformed(ActionEvent event) {
    ...
    if (TalkingClock.this.beep) Toolkit.getDefaultToolkit().beep();
}
```

反过来，可以用以下语法更加明确地编写内部类对象的构造器：

```java
outerObject.new InnerClass(construction parameters)
```

例如，

```java
ActionListener listener = this.new TimePrinter();
```

在这里，新构造的 `TimePrinter` 对象的外部类引用被设置为创建内部类对象的方法的 `this` 引用。
通常，`this.` 限定符是多余的。不过，也有可能通过显式地命名将外部类引用设置为其他对象。
例如，由于 `TimePrinter` 是一个公共内部类，可以为任意的语音时钟构造一个 `TimePrinter` ：

```java
var jabberer = new TalkingClock(1000, true);
TalkingClock.TimePrinter listener = jabberer.new TimePrinter();
```

需要注意，在外部类的作用域之外，可以这样引用内部类：

```java
OuterClass.InnerClass
```

> **注释**
>
> 内部类中声明的所有静态字段都必须是 `final` ，并初始化为一个编译时常量。
> 如果这个字段不是一个常量，就可能不唯一。
>
> 内部类不能有 `static` 方法。Java规范并没有做任何解释，应该是会带来复杂性。

## 局部内部类

如果仔细查看 `TalkingClock` 示例的代码就会发现，类型 `TimePrinter` 的名字只出现了一次：
就是在 `start` 方法中创建这个类型的对象时使用了一次。

在类似这样的情况下，可以在一个方法中局部地定义这个类。

```java
public void start(){
  class TimePrinter implements ActionListener {
    public void actionPerformed(ActionEvent event) {
      System.out.println("At the tone, the time is "
        + Instant.ofEpochMilli(event.getWhen()));
      if (beep) Toolkit.getDefaultToolkit().beep();
    }
  }
    
  var listener = new TimePrinter();
  var timer = new Timer(interval, listener);
  timer.start();
}
```

声明局部类时不能有访问说明符。局部类的作用域总是限定在声明这个局部类的块中。

局部类有一个很大的优势，即对外部世界完全隐藏，甚至 `TalkingClock` 类中的其他代码也不能访问它。
除 `start` 方法之外，没有任何方法知道 `TimePrinter` 的存在。

## 由外部方法访问变量

与其他内部类相比较，局部类还有另外一个优点。
它们不仅能够访问外部类的字段，还可以访问局部变量。
不过，那些局部变量必须是 **事实最终变量** 。这说明，它们一旦赋值就绝不会改变。

下面是一个典型的示例。这里，将 `TalkingClock` 构造器参数 `interval` 和 `beep` 移至 `start` 方法。

```java
public void start(int interval, boolean beep){
  class TimePrinter implements ActionListener {
    public void actionPerformed(ActionEvent event) {
      System.out.println("At the tone, the time is "
          + Instant.ofEpochMilli(event.getWhen()));
      if (beep) Toolkit.getDefaultToolkit().beep();
    }
  }

  var listener = new TimePrinter();
  var timer = new Timer(interval, listener);
  timer.start();
}
```

请注意， `TalkingClock` 类不再需要存储 `beep` 字段。它只是引用 `start` 方法的 `beep` 参数变量。

下面这行代码

``` java
if (beep)...
```

最后总会在 `start` 方法中，它可以访问 `beep` 变量的值。

为了能够清楚地看到这里一个微妙的问题，让我们仔细考虑这个控制流程。

1. 调用 `start` 方法
2. 调用内部类 `TimePrinter` 的构造器，从而初始化对象变量 `listener`
3. 将 `listener` 引用传递给 `Timer` 构造器，定时器开始计时， `start`方法退出。
   此时，`start` 方法中的 `beep` 参数不复存在。
4. 1秒后，`actionPerformed` 方法执行 `if (beep) ...` 。

要让 `actionPerformed` 方法中的代码正常工作，`TimePrinter` 类必须在 `beep` 参数值消失之前将它赋值到参数中。
实际上也是这么做的。即使局部变量出了作用域，内部字段都将永久保存。

## 匿名内部类

使用局部内部类时，通常还可以再进一步。加入只想要创建这个类的一个对象，甚至不需要为类指定名字。
这样一个类被称为 **匿名内部类(anonymous inner class)** 。

```java
public void start(int interval, boolean beep){
  var listener = new ActionListener {
    public void actionPerformed(ActionEvent event) {
      System.out.println("At the tone, the time is "
         + Instant.ofEpochMilli(event.getWhen()));
      if (beep) Toolkit.getDefaultToolkit().beep();
    }
  };
  
  var timer = new Timer(interval, listener);
  timer.start();
}
```

这个语法确实很晦涩难懂。它的含义是：创建一个类的新对象，这个类实现了 `ActionListener` 接口，
需要实现的方法 `actionPerformed` 是大括号 `{}` 中定义的方法。

一般地，语法如下：

```java
new SuperType(construction parameters) {
  // inner class methods and data
}
```

在这里，SuperType可以是接口，如果是这样，内部类就要实现这个接口。
SuperType也可以是一个类，如果是这样，内部类就要拓展这个类。

由于构造器名字必须和类名相同，而匿名内部类没有类名，所以，匿名内部类不能有构造器。
实际上，构造器要传参数给超类构造器。具体地，只要内部类实现了一个接口，就不能有任何构造参数。不过，仍然要提供一组小括号，如下所示：

```java
new InterfaceType() {
  // methods and data
}
```

必须仔细研究构造一个类的新对象与构造一个匿名内部类（扩展了那个类）的对象之前有什么区别。

```java
var queen=new Person("Marry");
// a Person object
var count=new Person("Dracula"){...};
// an object of an inner class extending Person
```

如果构造列表参数的结束小括号后面跟一个开始大做好，就是在定义匿名内部类。

> **注释**
>
> 尽管匿名类不能有构造器，但可以提供一个对象初始化块：
> ```java
> var count = new Person("Dracula") {
>     { /* initialization */ }
>     ...
> }
> ```

多年来，Java程序员习惯的做法是用匿名内部类实现事件监听器和其他回调。
如今最好还是使用lambda表达式。例如，`start` 方法用lambda表达式来编写会简单得多，如下所示：

```java
public void start(int interval, boolean beep) {
  var timer = new Timer(interval, event -> {
    System.out.println("At the tone, the time is "
        + Instant.ofEpochMilli(event.getWhen()));
    if (beep) Toolkit.getDefaultToolkit().beep();
  });
  timer.start();
}
```

> **注释**
>
> 下面的技巧称为 **“双括号初始化”** ，这里利用了内部类语法。假设你想构造一个数组列表，并将它传递到一个方法：
> ```java
> var friends = new ArrrayList<String>();
> friends.add("Harry");
> friends.add("Tony");
> invite(friends);
> ```
> 如果不再需要这个数组列表，最好是让他作为一个匿名列表。
> 不过作为一个匿名列表，该如何为它添加元素呢？方法如下：
> ```java
> invite(new ArrayList<String>() {{ add("Harry");  add("Tony"); }});
> ```
> 注意这里的双括号。外层括号建立了ArrayList的一个匿名子类。内层括号则是一个对象初始化块。
>
> 在实际中，这个技巧很少使用。大多数情况下， `invite` 方法会接收任何 `List<String>` ，
> 所以可以直接传入 `List.of("Haary", "Tony")` 。

## 静态内部类

有时候，使用内部类只是为了把一个类隐藏在另一个类的内部，并不需要内部列有外部对象的一个引用。
为此，可以将内部类声明为 `static` ，这样就不会生成那个引用。

下面是想要使用静态内部类的典型例子。考虑这样一个任务：
计算数组中的最小值和最大值。如果只遍历数组一次，同时计算出最小值和最大值，这样更为高效。

```java
double min = Double.POSITIVE_INFINITY;
double max = Double.NEGATIVE_INFINITY;
for(double v : values) {
  if (min > v)  min = v;
  if (max < v)  max = v;
}
```

不过，这个方法必须返回两个数。为此，可以定义一个包含两个值的类 `Pair`：

```java
class Pair {
  private double first;
  private double second;

  public Pair(double f, double s) {
    first = f;
    second = s;
  }
  public double getFirst() {
    return first;
  }
  public double getSecond() {
    return second;
  }
}
```

`minmax` 方法可以返回一个Pair类型的对象。

```java
class ArrayAlg {
  public static Pair minmax(double[] values) {
     ...
    return new Pair(min, max);
  }
}
```

这个方法的调用者可以使用 `getFirst()` 和 `getSecond()` 方法来获得答案：

```java
Pair p = ArrayAlg.minmax(d);
System.out.println("min = " + p.getFirst());
System.out.println("max = " + p.getSecond());
```

Pair可能会产生名字冲突，解决这个问题的办法是将Pair定义为ArrayAlg的一个公共内部类。
这样一来，就可以通过 `ArrayAlg.Pair` 访问这个类了：

```java
ArrayAlg.Pair p = ArrayAlg.minmax(d);
```

不过，与前面例子中所使用的内部类不同，我们不希望Pair对象中有其他对象的引用，维系，可以将这个内部类声明为 `static`
，从而不生成那个引用：

```java
class ArrayAlg {
  public static class Pair{
     ...
  }
  ...
}
```

当然，只有内部类可以声明为static。静态内部类就类似于其他内部类，只不过静态内部类的对象没有其他外部类对象的引用。
在我们的示例中，必须使用静态内部类，这是因为内部类对象是在一个静态方法中构造的：

```java
public static Pair minmax(double[] d) {
  ...
  return new Pair(min, max);
}
```

如果没有将Pair声明为static，那么编译器将会报错，指出没有可用的ArrayAlg类型隐式对象来初始化内部类对象。

> **注释**
>
> 只要内部类不需要访问外部类对象，就应该使用静态内部类。可以用 **嵌套类** 表示静态内部类。
>
> 在接口中声明的内部类自动是 `static` 和 `public` 。
>
> 类中声明的接口、记录和枚举都自动为 `static` 。
