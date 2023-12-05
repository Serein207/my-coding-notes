package abstractClasses;

import java.time.*;

public class Employee extends Person {
  private final double salary;
  private final LocalDate hireDay;

  public Employee(String name, double salary, int year, int month, int day) {
    super(name);
    this.salary = salary;
    hireDay = LocalDate.of(year, month, day);
  }

  public double getSalary() {
    return salary;
  }

  public LocalDate getHireDay() {
    return hireDay;
  }

  @Override
  public String getDescription() {
    return "an employee with a salary of $%.2f".formatted(salary);
  }
}