# Java - 服务加载器

有时你会采用一个服务架构开发一个应用。有些平台支持这种方法，如OSGi(http://osgi.org)
，可以用于开发环境、应用服务器和其他复杂的应用。这些平台超出了本书讨论的范畴，不过JDK也提供了一个加载服务的简单机制（这里会介绍）。这种机制由Java平台模块系统提供支持，详细内容参见本书卷Ⅱ第9章。

提供一个服务时，程序通常希望服务设计者对于如何实现这个服务的特性能有一些自由。另外还希望有多个实现可供选择。利用 `ServiceLoader`
类可以很容易地加载符合一个公共接口的服务。

定义一个接口（或者，如果愿意，也可以定义一个超类），其中包含这个服务的各个实例应当提供的方法。例如，假设你的服务要提供加密。

```java
package serviceLoader;

public interface Cipher {
  byte[] encrypt(byte[] source, byte[] key);
  byte[] decrypt(byte[] source, byte[] key);
  int strength();
}
```

服务提供者可以提供一个或多个实现这个服务的类，例如：

```java
package serviceLoader.impl;

public class CaesarCipher implements Cipher {
  public byte[] encrypt(byte[] source, byte[] key) {
    var result = new byte[source.length];
    for (int i = 0; i < source.length; i++)
      result[i] = (byte) (source[i] + key[0]);
    return result;
  }

  public byte[] decrypt(byte[] source, byte[] key) {
    return encrypt(source, new byte[]{(byte) -key[0]});
  }
  public int strength() {return 1;}
}
```

实现类可以放在任意的包中，而不一定是服务接口所在的包。每个实现类必须有一个无参数构造器。

现在把这些类的类名增加到 `META-INF/services` 目录下的一个UTF-8编码的文本文件中,文件名必须与接口的完全限定名一致。在我们的例子中，文件
`META-INF/services/serviceloader.Cipher` 必须包含这样一行：

```java
serviceLoader.impl.CaesarCipher
```

在这个例子中，我们提供了一个实现类。你也可以提供多个类，以后可以从中选择。

完成这个准备工作之后，程序可以如下初始化一个服务加载器：

```java
public static ServiceLoader<Cipher> cipherLoader=ServiceLoader.load(Cipher.class);
```

这个初始化工作只在程序中完成一次。

服务加载器的 `iterator` 方法会返回一个迭代器来迭代处理所提供的所有服务实现。（有关迭代器的更多信息参见第9章。）
最容易的做法是使用一个增强的for循环进行遍历。在循环中，选择一个适当的对象来完成服务。

```java
public static Cipher getCipher(int minStrength) {
  for (Cipher cipher : cipherLoader) {
    if (Cipher.strength() >= minStrength)   return cipher;
  }
  return null;
}
```

或者，也可以使用流（见本书卷Ⅱ的第1章）查找所要的服务。stream方法会生成 `ServiceLoader.Provider` 实例的一个流。
这个接口包含 `type` 和 `get` 方法，可以用来得到提供者类和提供者实例。
如果按类型选择一个提供者，只需要调用 `type`，而没有必要实例化任何服务实例。

```java
public static Optional<Cipher> getCipher2(int minStrength) {
  return cipherLoader.stream().
    filter(descr->descr.type() == serviceloader.impl.CaesarCipher.class)
    .findFirst()
    .map(Serviceloader.Provider::get);
}
```

最后，如果想要得到任何服务实例，只需要调用 `findFirst`：

```java
Optional<Cipher> cipher = cipherLoader.findFirst();
```

`Optional` 类会在本书卷Ⅱ的第1章详细解释。
