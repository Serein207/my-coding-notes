public class Main {
  public static void main(String[] args) {
    var staff = new Employee[3];

    staff[0] = new Employee("Tom", 40000);
    staff[1] = new Employee("Dick", 60000);
    staff[2] = new Employee("Harry", 65000);

    for (Employee e : staff) {
      System.out.println("name=" + e.getName() + ",id=" + e.getId() + ",salary=" + e.getSalary());
    }

    int n = Employee.getNextId();
    System.out.println("Next available id=" + n);
  }
}