package equals;

import java.time.LocalDate;
import java.util.*;
import java.util.Objects;

public class Employee {
  private String name;
  private double salary;
  private LocalDate hireDay;

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

  public LocalDate getHireDay() {
    return hireDay;
  }

  public void raiseSalary(double byPercent) {
    double raise = salary * byPercent / 100;
    salary += raise;
  }

  @Override
  public boolean equals(Object otherObject) {
    if (this == otherObject) return true;
    if (otherObject == null) return false;
    if (getClass() != otherObject.getClass()) return false;

    var other = (Employee) otherObject;

    return Objects.equals(name, other.name)
            && salary == other.salary
            && Objects.equals(hireDay, other.hireDay);
  }

  @Override
  public int hashCode() {
    return Objects.hash(name, salary, hireDay);
  }

  @Override
  public String toString() {
    return getClass().getName() +
            "{name='" + name + '\'' +
            ", salary=" + salary +
            ", hireDay=" + hireDay +
            '}';
  }
}