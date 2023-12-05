package clone;

import java.io.FilterOutputStream;

public class CloneTest {
  public static void main(String[] args) throws CloneNotSupportedException {
    var original = new Employee("Jason Q. Public", 50000);
    original.setHireDay(2000, 1, 1);
    Employee copy = original.clone();
    copy.raiseSalary(10);
    copy.setHireDay(2002, 12, 31);
    System.out.println("original=" + original);
    System.out.println("copy=" + copy);
  }
}