package inheritance;

public class Manager extends Employee {
  private double bonus;

  /**
   * @param name   the employee's name
   * @param salary the salary
   * @param year   the year
   * @param month  the hire month
   * @param day    the hire day
   */
  public Manager(String name, double salary, int year, int month, int day) {
    super(name, salary, year, month, day);
    bonus = 0;
  }

  @Override
  public double getSalary() {
    double baseSalary = super.getSalary();
    return baseSalary + bonus;
  }

  public void setBonus(double bonus) {
    this.bonus = bonus;
  }
}