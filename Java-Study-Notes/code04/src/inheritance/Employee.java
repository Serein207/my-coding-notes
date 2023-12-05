package inheritance;

import java.time.*;

public class Employee {
  private final String name;
  private final LocalDate hireDay;
  private double salary;

  public Employee(String name, double salary, int year, int month, int day) {
    this.name = name;
    this.salary = salary;
    hireDay = LocalDate.of(year, month, day);
  }

  public String getName() {
    return name;
  }

  public double getSalary() {
    return salary;
  }

  public void raiseSalary(double btPercent) {
    double raise = salary * btPercent / 100;
    salary += raise;
  }
}