public class Employee {
  private static int nextId = 1;

  private final String name;
  private final double salary;
  private final int id;

  public Employee(String n, double s) {
    name = n;
    salary = s;
    id = advanceId();
  }

  public static int advanceId() {
    int r = nextId;
    nextId++;
    return r;
  }

  public static int getNextId() {
    return nextId;
  }

  public String getName() {
    return name;
  }

  public double getSalary() {
    return salary;
  }

  public int getId() {
    return id;
  }
}