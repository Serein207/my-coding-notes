# Java - 代理

在本章的最后，我们来讨论 **代理(proxy)** 。利用代理可以在运行时创建实现了一组给定接口的新类。
只有在编译时无法确定需要实现哪个接口时才有必要使用代理。对于编写应用程序的程序员来说，这种情况很少见，所以如果对这种高级技术不感兴趣，完全可以跳过本节内容。
不过，对于某些系统应用，代理提供的灵活性可能十分重要。

## 何时使用代理

假设你想构造一个类的对象，这个类实现了一个或多个接口，但是在编译时你可能并不知道这些接口到底是什么。
这个问题确实有些难度。要想构造一个具体的类，只需要使用 `newInstance` 方法或者使用反射找出构造器。
但是，不能实例化接口。需要在运行的程序中定义一个新类。

为了解决这个问题，有些程序会生成代码，将这些代码放在一个文件中，调用编译器然后再加载得到的类文件。
很自然地，这样做的速度会比较慢，并且需要部署编译器以及程序。而代理机制则是一种更好的解决方案。
代理类可以在运行时创建全新的类。这样一个代理类能够实现你指定的接口。具体地，代理类包含以下方法：

- 指定接口所需要的全部方法。
- `Object` 类中定义的全部方法（`toString`、`equals`等）。

不过，不能在运行时为这些方法定义新代码。
实际上，必须提供一个调用处理器`invocation handler`)。
调用处理器是实现了`InvocationHandler`接口的类的对象。
这个接口只有一个方法：

```java
Object invoke(Object proxy, Method method, Object[] args)
```

无论何时调用代理对象的方法，都会调用这个调用处理器的 `invoke` 方法，并提供 `Method` 对象和原调用的参数。
之后，调用处理器必须确定如何处理这个调用。

## 创建代理对象

要想创建一个代理对象，需要使用 `Proxy` 类的 `newProxyInstance` 方法。这个方法有三个参数：

- 一个 **类加载器（class loader）**。
  作为Java安全模型的一部分，对于平台和应用类、从因特网下载的类等等可以使用不同的类加载器。有关类加载器的详细内容将在卷Ⅱ第9章中讨论。在这个例子中，我们指定了加载平台和应用类的“系统类加载器”。
- 一个 `Class` 对象数组，每个元素对应需要实现的各个接口。
- 一个调用处理器。

还有两个需要解决的问题。如何定义处理器？另外，对于得到的代理对象能够做些什么？

- 当然，这两个问题的答案取决于我们想要用代理机制解决什么问题。使用代理可能出子很多目的，例如：
- 将方法调用路由到远程服务器。
- 将用户界面事件与正在运行的程序中的动作关联起来。
- 为了调试而跟踪方法调用。

在示例程序中，我们要使用代理和调用处理器跟踪方法调用。
我们定义了一个 `TraceHandler` 包装器类存储一个包装的对象，其 `invoke` 方法会打印所调用方法的名字和参数，随后调用这个方法，并提供所包装的对象作为隐式参数。

```java
class TraceHandler implements InvocationHandler {
  private Object target;
  public TraceHandler(Object t) {
    target = t;
  }

  public Object invoke(Object proxy, Method m, Objectl]args)
      throws Throwable {
    // print method name and parameters
    ...
    // invoke actual method
    return m.invoke(target, args);
  }
}

```

可以如下构造一个代理对象，只要调用它的某个方法，就会触发跟踪行为：

```java
Object value=...i
// construct wrapper
var handler = new TraceHandler(value);
// construct proxy for one or more interfaces
var interfaces = new Class[] { Comparable.class};
Object proxy = Proxy.newProxyInstance(
  ClassLoader.getSystemClassLoader(),
  new Class[] { Comparable.class },handler);
```

现在，只要在proxy上调用了某个接口的方法，就会打印这个方法的名字和参数，之后再用value调用这个方法。

在程序清单6-10所示的程序中，我们使用代理对象跟踪一个二分查找。
这里首先在数组中填充整数1~1000的代理，然后调用Arrays类的 `binarySearch` 方法在数组中查找一个随机整数。
最后，打印出匹配的元素。

```java
var elements = new Object[1000];
// fill elements with proxies for the integers 1 ... 1000
for (int i = 0; i < elements.length; i++){
  Integer value = i + 1;
  elements[i] = Proxy.newProxyInstance(...); // proxy for value;
}

// construct a random integer
Integer key = (int)(Math, random()* elements.length) + 1;
// search for the key
int result = Arrays.binarySearch(elements, key);
// print match if found
if (result >= 0) System.out.println(elements[result]);
```

`Integer` 类实现了 `Comparable` 接口。
代理对象属于在运行时定义的一个类（它有一个类似$Proxy0的名字）。这个类也实现了 `Comparable` 接口。
不过，它的 `compareTo` 方法调用了代理对象处理器的 `invoke` 方法。
> **注释**
>
> 在本章前面已经看到，`Integer`类实际上实现了`Comparable<Integer>`。
> 不过，在运行时，所有的泛型类型都会擦除，会用对应原始Comparable类的类对象构造代理。

`binarySearch`方法有以下调用：

```java
if (elements[i].compareTo(key) < 0)  ...
```

由于数组中填充了代理对象，所以 `compareTo` 会调用 `TraceHandler` 类中的 `invoke` 方法。
这个方法会打印方法名和参数，之后在包装的 `Integer` 对象上调用 `compareTo`。

最后，在示例程序的最后调用：

```java
System.out.println(elements[result]);
```

这个 `println` 方法调用代理对象的 `toString`，这个调用也会重定向到调用处理器。

下面是程序运行时完整的跟踪结果：

```
500.compareTo(288)
250.compareTo(288)
375.compareTo(288)
312.compareTo(288)
281.compareTo(288)
296.compareTo(288)
288.compareTo(288)
288.toString()
```

可以看到二分查找算法是如何查找 `key` 值的，每一步都会将查找区间缩减一半。
注意尽管 `toString` 方法不属于 `Comparable` 接口，但这个方法也会被代理。
在下一节中会看到，某些 `Object` 方法总是会被代理。

## 代理类的特性

我们已经看到了代理类的具体使用，接下来了解它们的一些特性。
需要记住，代理类是在程序运行过程中动态创建的。
不过，一旦创建，它们就是常规的类，与虚拟机中的任何其他类没有什么区别。

所有的代理类都扩展 `Proxy` 类。一个代理类只有一个实例字段——即调用处理器，它在 `Proxy` 超类中定义。
完成代理对象任务所需要的任何额外数据都必须存储在调用处理器中。
例如，在程序清单6-10给出的程序中，代理 `Comparable` 对象时，`TraceHandler` 就包装了具体的对象。

所有的代理类都要覆盖 `Object` 类的 `toString`、`equals` 和 `hashCode` 方法。
如同所有代理方法一样，这些方法只是在调用处理器上调用 `invoke`。
`Object` 类中的其他方法（如`clone`和`getClass`)没有重新定义。

没有定义代理类的名字，`Oracle`虚拟机中的`Proxy`类会生成以字符串$Proxy开头的类名。

对于一个特定的类加载器和一组接口，只能有一个代理类。
也就是说，如果使用同一个类加载器和接口数组调用两次`newProxyInstance`方法，将得到同一个类的两个对象。
也可以利用 `getProxyClass` 方法获得这个类：

```java
Class proxyClass = Proxy.getProxyClass(null, interfaces);
```

代理类总是public和final。如果代理类实现的所有接口都是public，这个代理类就不属于任何特定的包；
否则，所有非公共的接口都必须属于同一个包，而且代理类也属于这个包。

可以通过调用 `Proxy` 类的 `isProxyClass` 方法检测一个特定的 `Class` 对象是否表示一个代理类。
> **注释**
>
> 调用一个目标代理的默认方法会触发调用处理器。要具体调用这个方法，可以
> 使用 `InvocationHandler` 接口的静态方法 `invokeDefault`。例如，下面是一个调用处理器，
> 它会调用默认方法，并把抽象方法传递到另一个目标：
> ```java
> InvocationHandler handler = (proxy, method, args)-> {
>   if (method.isDefault())
>     return InvocationHandler. invokeDefault(proxy, method, args);
>   else
>     return method,invoke(target, args);
> }
> ```

---
到此为止，Java程序设计语言的面向对象特性就介绍完毕了。
接口、lambda表达式和内部类是我们经常会遇到的几个概念，不过，克隆、服务加载器和代理等高级技术主要是设计库及构建工具的程序员感兴趣，开发应用程序的程序员对此可能不太关心。
接下来可以在第7章学习如何处理程序中的异常情况。
