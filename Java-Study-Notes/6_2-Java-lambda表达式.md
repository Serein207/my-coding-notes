# Java - lambda表达式

## 为什么引入lambda表达式

如果需要按照指定时间间隔完成工作，将这个工作放在一个 `ActionListener` 的 `actionPerformed` 方法中：

```java
class Worker implements ActionListener {
  public void actionPerformed(ActionEvent event) {
    // do some work
  }
}
```

然后，想要重复这个代码时，可以构造一个Worker类的示例，再把这个实例提交到一个Timer对象。

这里的重点是 `actionPerformed` 方法包含希望以后执行的代码。

或者可以考虑如何用一个定制比较器完成排序。如果想按长度而不是默认的字典顺序对字符串进行排序，可以向sort方法传入一个 `Compatator`
对象：

```java
class LengthComparator implements Comparable<String> {
  public void compare(String first, String second) {
    return first.length() - second.length();
  }
}
...
Arrays.sort(strings, new LengthComparator());
```

这两份例子有一些共同点，都是将一个代码块传递到某个目标，这个代码块会在将来某个时间调用。

## lambda表达式的语法

再来考虑上一节讨论排序的例子。我们传入代码来检查一个字符串是否比另一个字符串短。这里要计算：

```java
first.length() - second.length()
```

first和second是什么？它们都是字符串。我们要指定它们的类型：

```java
(String first, String second) ->
  first.length - second.length()
```

这就是你看到的第一个lambda表达式。lambda表达式就是一个代码块，以及必须传入代码的所有变量的规范。

你已经见过Java中一种简单的lambda表达式形式：参数，箭头，以及一个表达式。
如果代码要完成的计算无法放在一个表达式中，就可以像写方法一样，把这些代码放在 `{}` 中，并包含显示的 `retrun` 语句。例如：

```java
(String first, String second)->{
    if(first.length() < second.length())  return-1;
    else if(first.length() > second.length()) return 1;
    else return 0;
}
```

即使lambda表达式没有参数，仍要提供空括号，就像无参数方法一样：

```java
()->{for(int i=100;i>=0;i--)  System.out.println(i);}
```

如果可以推导一个lambda表达式的类型参数，则可以忽略其类型。例如：

```java
Comparator<String> comp=(first,second)->
    first.length()-second.length();
```

在这里，编译器可以推导出first和second必然是字符串。因为这个lambda表达式将赋值给一个字符串比较器。

如果方法只有一个参数，而且这个参数的类型可以推导得出，那么甚至还可以省略小括号：

```java
ActionListener listener = event->
    System.out.println("The time is "
    +Instant.ofEpochMilli(event.getWhen()));
```

无须指定lambda表达式的返回类型。lambda表达式的返回值类型总是会由上下文推导得出。例如，下面的表达式

```java
(String first, String second) -> first.length() - second.length()
```

可以在需要int类型结果的上下文中使用。

最后，可以使用 `var` 指示一个推导的类型。这不常见，发明这个语法是为了关联注解。

```java
(@NonNull var first, @NonNull var second) -> first.length() - second.length()
```

## 函数式接口

Java有很多封装代码块的接口，如 `ActionListener` 或 `Comparator` 。lambda表达式与这些接口都是兼容的。

对于只有一个抽象方法的接口，需要这种接口的对象时，就可以提供一个lambda表达式。这种接口称为 **函数式接口**。

下面考虑 `Arrays.sort` 方法。它的第二个参数需要一个 `Comparator` 实例，Comparator就是只有一个方法的接口，所以可以提供一个lambda表达式：

```java
Arrays.sort(words,
    (first, second) -> first.lenght() - second.length());
```

在底层，`Arrays.sort` 方法会接收实现了 `Comparator<String>` 的某个类对象。在这个对象上调用compare方法会执行这个lambda表达式的体。

lambda表达式可以转换为接口：

```java
var timer = new Timer(1000, event -> {
  System.out.println("At the tone, the time is "
    + Instant.ofEpochMilli(event.getWhen()));
  Toolkit.getDefaultToolkit().beep();
});
```

与使用实现了 `ActionListener` 接口的类相比，这段代码的可读性要好得多。

在Java中，对lambda表达式所能做的也只是转换为函数式接口。

> **注释**
>
> 甚至不能把lambda表达式赋值给类型为Object的变量，Object不是一个函数式接口。

Java API在 `java.util.function` 包中定义了汗多非常通用的函数式接口。
其中一个接口 `BiFunction<T, U, R>` 描述了参数类型为T和U而且返回类型为R的函数。
可以把我们的字符串比较lambda表达式保存在这个类型的变量中：

```java
BiFunction<String, String, Integer> comp=
    (first,second)->first.length()-second.length();
```

不过，这对于排序并没有帮助。没有哪个 `Arrays.sort` 方法想要接收一个 `BiFunction`。
类似 `Comparator` 的接口往往有一个特定的用途，而不只是提供一个有指定参数和返回值类型的方法。
想要用lambda表达式做某些处理时，还是希望谨记表达式的用途，为它建立一个特定的函数式接口。

`java.util.function` 包中有一个尤其有用的接口 `Predicate`：

```java
public interface Predicate<T> {
  boolean test(T t);
  // additional default and static methods
}
```

`ArrayList` 类有一个 `removeIf` 方法，它的参数就是一个 `Predicate`。
这个接口专门用来传递lambda表达式。
例如，下面的语句将一个数组列表删除所有的null值：

```java
list.removeIf(e->e==null);
```

另一个有用的函数式接口是 `Supplier<T>`：

```java
public interface Supplier<T> {
  T get();
}
```

供应者（supplier）没有参数，调用时会生成一个T类型的值。供应者用于实现 **懒计算(lazy evaluation)** 。例如，考虑以下调用：

```java
LocalDate hireDay = Objects.requireNonNullElse(day,
    LocalDate.of(1970, 1, 1));
```

这不是最优的，我们 预计day很少为null，所以希望只在必要时才构造默认的 `LocalDate` 。
通过使用供应者，我们就能延迟这个计算：

```java
LocalDate hireDay = Objects.requireNonNullElse(day,
    () -> LocalDate.of(1970, 1, 1));
```

`requireNonNullOrElseGet` 方法只在需要值时才调用供应者。

## 方法引用

有时，lambda表达式设计一个方法引用。例如，假设你希望只要出现一个定时器时间就打印这个事件对象。当然，为此也可以调用：

```java
var timer = new Timer(1000, event -> System.out.println(event));
```

但是，如果直接把 `println` 方法传递到 `Timer` 构造器就更好了。具体做法如下：

```java
var time = new Timer(1000, System.out::println);
```

表达式 `System.out::println` 是一个 **方法引用(method reference)** ，它只是编译器生成一个函数式接口的实例，覆盖这个接口的抽象方法来调用给定的方法。
在这个例子中，会生成一个 `ActionListener` ，它的 `actionPerformed(ActionEvent e)` 方法要调用 `System.out.println(e)` 。

> **注释**
>
> 类似于lambda表达式，方法引用也不是一个对象。不过，为一个类型为函数式的接口的变量赋值时会生成一个对象。

再来看一个例子，假设你想对字符串进行排序，而不考虑字幕的大小写。可以传递以下方法表达式：

```java
Arrays.sort(strings, String::compareToIgnoreCase)
```

从这些例子可以看出，要用 `::` 操作符分割方法名与对象或类名，主要有3种情况：

1. object::instanceMethod
2. Class::instanceMethod
3. Class::staticMethod

在第1种情况下，方法引用等价与一个lambda表达式，其参数要传递到方法。对于 `System.out.println` ，对象是 `System.out`
，所以这个方法表达式等价于 `x->System.out.println(x)`。

对于第2种情况，第1个参数会成为方法的饮食参数。例如, `String::compareToIgnoreCase`
等同于 `(x, y) ->x.compareToIgnoreCase(y)`。

在第3种情况下，所有参数都传递到静态方法： `Math::pow` 等价于 `(x, y) ->Math.pow(x, y)` 。

注意，只有当lambda表达式的体只调用一个方法而不做其他操作时，才能把lambda表达式重写为方法引用。考虑下面的lambda表达式：

```
s ->s.length() == 0
```

这里只有一个方法调用。但是还有一个比较，所以这里不能使用方法引用。

| 方法引用              | 等价的lambda表达式              | 说明                                                         |
|:------------------|:--------------------------|:-----------------------------------------------------------|
| separator::equals | x->separator.equals(x)    | 情况1<br/>lambda参数作为这个方法的显式参数传入                              |
| String::trim      | x->x.strip()              | 情况2</br>lambda表达式会成为隐式参数                                   |
| String::concat    | (x, y)->x.concat(y)       | 情况2<br/>第一个lambda参数会成为隐式参数，其余的参数会传递到方法                     |
| Integer.valueOf   | x->Integer.valueOf(x)     | 情况3<br/>lambda参数会传递到这个静态方法                                 |
| Integer.sum       | (x, y)->Integer.sum(x, y) | 情况3<br/>两个lambda参数都会传递到这个静态方法，`Integer.sum` 方法专门创建作为一个方法引用 |
| String::new       | x->new String(x)          | 情况2<br/>lambda参数传递到这个构造器                                   |
| String[]::new     | n->new String[n]          | 情况2<br/>lambda参数是数组长度                                      |

> **注释**
>
> 包含对象的方法引用与等价的lambda表达式还有一个细微的差别。考虑一个方法引用，如 `separator::equals` 。
> 如果 `separator` 为null，构造 `separator::equals` 时就会立即抛出一个 `NullPointerExpection` 异常。
> 而lambda表达式 `x ->separator.equals(x)` 只在调用时才会抛出 `NullPointerExpection` 。

可以在方法引用中使用 `this` 参数。例如，`this::equals` 等同于 `x->this.equals(x)`。使用 `super` 也是合法的。下面的方法表达式

```java
super::instanceMethod
```

使用 `this` 作为目标，会调用给定方法的超类版本。下面给出一个例子：

```java
class Greeter {
  public void greet(ActionEvent event) {
    System.out.println("Hello, the time is "
        + Instant.ofEpochMilli(event.getWhen()));
  }
}

class RepeatedGreeter extends Greeter {
  public void greet(ActionEvent event) {
    var timer = new Timer(1000, super::greet);
    timer.start();
  }
}
```

`RepeatedGreeter.greet` 方法开始执行时，会构造一个Timer，每次定时器滴答时会执行 `super::greet` 方法。

## 构造器引用

构造器引用与方法引用很类似，只不过方法名为 `new` 。
例如， `Person::new` 是 `Person` 构造器的一个引用。哪一个构造器呢？这取决于上下文。
可以在各个字符串上调用构造器，把这个字符串列表转换为一个 `Person` 对象数组，调用如下：

```java
ArratList<String> names = ...;
Stream<Person> stream = names.stream().map(Person::new);
List<Person> people = stream.toList();
```

`map` 方法会为各个列表元素调用 `Person(String)` 构造器。
如果有多个构造器，编译器会选择一个 `String` 参数类型的构造器，因为它从上下文推到出这是在第哦啊用带一个字符传的构造器。

可以用数组类型建立构造器引用。例如， `int[]::new` 是一个构造器引用，它有一个参数：数组的长度。
这等价于lambda表达式 `x->new int[x]` 。

Java无法构造泛型类型T的数组。数组构造器引用对于克服这个限制很有用。
例如，我们需要一个 `Person` 对象数组。
`Stream` 接口有一个 `toArray` 方法可以返回Object数组：

```java
Object[] people = stream.toArray();
```

不过，这并不让人满意。流库利用构造器引用解决了这个问题。可以把 `Person[]::new` 传入 `toArray` 方法：

```java
Person[] people = stream.toArray(Person[]::new);
```

`toArray` 方法调用这个构造器来得到一个有正确类型的数组。然后填充并返回这个数组。

## 变量作用域

通常，你可能希望能够在lambda表达式种访问外围方法或类中的变量。考虑下面这个例子：

```java
public static void repeatMessage(String text, int delay){
    ActionListener listener = event->{
      System.out.println(text);
      Toolkit.getDefaultToolkit().beep();
    };
    new Timer(delay,listener).start();
}
```

来看这样一个调用：

```java
repeatMessage("Hello",1000); // print Helllo every 1000ms
```

现在来看lambda表达式中的变量 `text` 。注意这个变量并不是在这个lambda表达式中定义的。
实际上，这是 `repeatMessage` 方法的一个参数变量。

lambda表达式的代码可能在 `repeatMessage` 调用返回很久以后才运行，而那时这个参数变量已经不存在了。
`text` 变量是如何保留下来的呢？

lambda表达式有3个部分：

1. 一个代码块；
2. 参数；
3. *自由* 变量的值，这是指非参数而且不再代码中定义的变量。

在我们的例子中，这个lambda表达式有一个自由变量 `text`
。表示lambda表达式的数据接口必须存储自由变量的值，在这里就是字符串 `"Hello"` 。
我们说这些值是被lambda表达式 **捕获(captured)** 。

可以看到，lambda表达式可以捕获外围作用域变量的值。在Java中，为了确保所有捕获的值式明确定义的，这里有一个重要的限制。
在lambda表达式中，只能引用值不会改变的变量。例如，下面的做法是不合法的：

```java
public static void countDown(int start, int delay) {
  ActionListener listener = event -> {
    start--;    // Error: can't mutate caputured variable
    System.out.println(start);
  };
  new Timer(delay, listener).start();
}
```

这个限制是有原因的。如果在lambda表达式中更改变量，并发执行多个动作时就会不安全。
对于目前位置我们看到的动作不会发生这种情况，不过一般来讲，这确实是一个严重的问题。

另外如果在lambda表达式中引用一个变量，而这个变量可能在外部改变，这也是不合法的。
例如，下面就是不合法的：

```java
public static void repeat(String text, int count) {
  for(int i = 1; i <= count; i++) {
    Actionlistener listener = event -> {
      System.out.println(i + ": " + text);
        // Error: can't refer to changing i
    };
    new Timer(delay,listener).start();
  }
}
```

这里有一条规则：lambda表达式中捕获的变量必须是 **事实最终变量(effectively final)** 。事实最终变量是指，
这个变量初始化后就不会再为它重新赋值。在这里， `text` 总是指示同一个 `String` 对象，所以捕获这个变量是可以的。
不过， `i` 的值会改变，因此不能捕获 `i` 。

lambda表达式的体与 *嵌套块有相同的作用域*。
这里同样使用命名冲入和遮蔽有关的规则。
在lambda表达式中声明一个与局部变量同名的参数或局部变量是不合法的。

```java
Path first = Path.of("/usr/bin");
Comparator<String> comp =
    (first, second) -> first.length() - second.length();
    // Error: Variable first already defined
```

在一个方法中，不能有两个同名的局部变量，因此，lambda表达式中同样也不能有同名的局部变量。

在一个lambda表达式中使用 `this` 关键字时，是指创建这个lambda表达式的方法的 `this` 参数。
例如，考虑下面的代码：

```java
public class Application {
  public void init() {
    ActionListener listener = event -> {
      System.out.println(this.toString());
      ...
    }
    ...
  }
}
```

表达式 `this.toString()` 会抵用 `Application` 对象的 `toString` 方法，而不是 `ActionListener` 实例的方法。
在lambda表达式中， `this` 的使用并没有任何特殊之处。
lambda表达式的作用域嵌套在 `init` 方法中，不论 `this` 在lambda表达式总还是出现在这个方法中的其他位置，其含义并没有不同。

## 处理lambda表达式

下面来看如何编写方法处理lambda表达式。
使用lambda表达式的重点是 **延迟执行(deferred execution)**。毕竟，如果想要立即执行代码，完全可以直接执行，而无须把他包装在一个lambda表达式中。
之所以希望以后再执行代码，这里有很多愿意，如：

- 在一个单独的线程中运行代码；
- 多次运行代码；
- 在算法的适当位置运行代码（例如，排序中的比较操作）；
- 发生某种情况时运行代码（如，点击了一个按钮，数据已经到达）；
- 只在必要时才运行代码。

下面来看一个简单的例子。假设你想要重复一个动作n次，将这个动作和重复次数传递到一个 `repeat` 方法：

```java
repeat(10,()->System.out.println("Hello, world!"));
```

要接收这个lambda表达式，需要选择一个函数式接口。表格列出了Java
API中提供的最重要的函数式接口。在这里，我们可以使用 `Runnable` 接口：

```java
public static void repeat(int n, Runnable action) {
  for(int i = 0; i < n; i++)
    action.run();
}
```

| 函数式接口             | 参数类型 | 返回类型    | 抽象方法名  | 描述             | 其他方法                       |
|-------------------|------|---------|--------|----------------|----------------------------|
| Runnable          | 无    | void    | run    | 运行一个无参数或返回值的动作 ||
| Supplier<T>       | 无    | T       | get    | 提供一个T类型的值      |                            |
| Consumer<T>       | T    | void    | accept | 处理一个T类型的值      | andThen                    |
| BiConsumer<T, U>  | T, U | void    | accept | 处理T和U类型的值      | andThen                    |
| Function<T, R>    | T    | R       | apply  | 有一个T类型参数的函数    | compose, andThen, identity |
| BiFunction        | T, U | R       | apply  | 有T和U参数类型的函数    | andThen                    |
| UnaryOperator<T>  | T    | T       | apply  | 类型T上的一元操作符     | compose, andThen, identity |
| BinaryOperator    | T, T | T       | apply  | 类型T上的二元操作符     | andThen, maxBy, minBy      |
| Predicate<T>      | T    | boolean | test   | 布尔值函数          | and, or, negate, isEqual   |
| BiPredicate<T, U> | T, U | boolean | test   | 有两个参数的布尔值函数    | and, or, negate            |

需要说明，调用 `action.run()` 时会执行这个lambda表达式的主体。

现在让这个例子更复杂一些。我们希望告诉这个动作它出现在哪一次迭代中。
为此，需要选择一个合适的函数式接口，其中要包含一个方法，这个方法有一个 `int` 参数而且返回类型为 `void`。
处理 `int` 值的标准接口如下：

```java
public interface IntConsumer {
  void accept(int value);
}
```

下面给出 `repeat` 方法的改进版本：

```java
public static void repeat(int n, IntConsumer action) {
  for(int i = 0; i < n; i++)
    action.accept(i);
}
```

可以如下调用：

```java
repeat(10, i-> System.out.println("Countdown: " + (9 - i)));
```

表格列入了基本类型 `int` `long` `double`的34个可用的特殊化接口。使用这些特殊化接口比使用通用接口更高效。
处于这个原因，我们使用了 `IntConsumer` 而不是 `Consumer<Integer>`。

| 函数式接口               | 参数类型 | 返回类型    | 抽象方法名        |
|---------------------|------|---------|--------------|
| BooleanSupplier     | 无    | boolean | getAsBoolean |
| PSupplier           | 无    | p       | getAsP       |
| PConsumer           | p    | void    | accept       |
| objPConsumer<T>     | T, p | void    | accept       |
| PFunction<T>        | p    | T       | apply        |
| PToQFunction        | p    | q       | applyAsQ     |
| ToPFunction<T>      | T    | p       | applyAsP     |
| ToPBiFunction<T, U> | T, U | p       | applyAsP     |
| PUnaryOperator      | p    | p       | applyAsP     |
| PBinaryOperator     | p, p | p       | applyAsP     |
| PPredicate          | p    | boolean | test         |

注：p, q是 int, long, double; P, Q 是Int, Long, Double

> **注释**
>
> 大多数标准函数式接口都提供了非抽象方法来生成或合并函数。
> 例如，`Predicate.isEqual(a)` 等同于 `a::equals`，不过如果a为null也能正常工作，
> 已经提供了默认方法 `and` `or` `negate` 来合并谓词。
> 例如， `Predicate.isEqual(a).or(Predaicate.isEqual(b))` 就等同于
> `x->a.equal(x) || b.equals(x)` 。

> **注释**
>
> 如果设计你自己的接口，其中只有一个抽象方法，可以用 `@FunctionalInterface` 注解来标记这个接口。
> 这样做有两个优点。如果你无意中增加了另一个抽象方法，编译器会给出一个错误消息。
> 另外javadoc页中会指出你的接口是一个函数式接口
>
> 并不是必须使用注解。根据定义，任何只有一个抽象方法的接口都是函数式接口。
> 不过使用 `@FunctionalInterface` 注解确实是一个好主意。

> **注释**
>
> 有些程序员喜欢将方法调用串起来，如：
> ```
> String input = "623490242384092380105211 ";
> boolean isPrime = input.strip().transform(BigInteger::new).isProbablePrime(20);
> ```
> String类的 `transform` 方法（Java12）对字符串应用一个 `Function`，并生成结果。
> 同样地，这些调用也可以写为：
> ```
> boolean prime = new BigInteger(input.strip()).isProbablePrime(20);
> ```
> 不过这样一来，你的视线必须所有跳来跳去，先找出哪一个先执行，哪一个后执行：
> 首先调用 `strip` ，然后构造 `BigInteger`，最后检测它是否是一个可能的素数。

## 再谈 `Comparator`

`Comparetor` 接口包含很多方便的静态方法来创建比较器。这些方法可以用于lambda表达式或方法引用。

静态 `comparing` 方法接受一个“键提取器”函数，它将类型T映射为一个可比较的类型（如String）。
对要比较的对象应用这个参数，然后对返回的键完成比较。例如，假设有一个 `Person` 对象数组，可以如下按名字对这些对象进行排序：

```java
Arrays.sort(people,Comparator.comparing(Person::getName));
```

与手动实现一个 `Comparator` 相比，这当然要容易得多。另外，代码也更为清晰。

可以把比较器与 `thenComparing` 方法串起来，来处理比较结果相同得情况。例如：

```java
Arrays.sort(people,
    Comparator.comparing(Person::getLastName)
    .thenComparing(Person::getFirstName));
```

如果两个人姓相同，就会使用第二个比较器。

这些方法有很多变体形式。可以为 `comparing` 和 `thenComparing` 方法提取得键指定一个比较器。例如，可以如下根据人名长度完成排序：

```java
Arrays.sort(people,Comparator.comparing(Person::getName,
    (s,t)->Integer.compare(s.length,t.length())));
```

另外，`comparing` 和 `thenComparing` 方法都有变体形式，可以避免int，long，double值的装箱。
要完成前一个操作，还有一种更容易的做法：

```java
Arrays.sort(people, Comparator.comparingInt(p->p.getName().length()));
```

如果键函数可能返回null，可能就要用到 `nullsFirst` 和 `nullsLast` 适配器。
这些静态方法会修改现有的比较器，从而在遇到null值是不会抛出异常，而是将这个值标记为小于或大于正常值。
例如，假设一个人没有中名时 `getMiddleName` 会返回一个null，就可以使用
`Comparetor.comparingInt(Person::getMiddleName(), Comparator.nullsFirst(...))`。

`nullsFirst` 方法需要一个比较器，在这里就是比较两个字符串的比较器。
`naturalOrder` 方法可以为任何实现了 `Comparable` 的类建立一个比较器。
在这里， `Comparator.<String>naturalOrder()` 正是我们所需要的。
下面是一个完整的调用，可以按可能为null的中名进行排序。注意 `naturalOrder` 的类型会推导得出。

```java
Arrays.sort(people, comparing(Person::getMiddleName, nullsFirst(naturalOrder())));
```

静态 `reverseOrder` 方法会提供自然顺序的逆序。
要让比较器逆序比较，可以使用 `reversed` 实例方法。
例如 `naturalOrder().reversed()` 等同于 `reverseOrder()` 。
