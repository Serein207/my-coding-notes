package abstractClasses;

public abstract class Person {
  private final String name;

  public Person(String name) {
    this.name = name;
  }

  public abstract String getDescription();

  public String getName() {
    return name;
  }
}