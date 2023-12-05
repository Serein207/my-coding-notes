package lambda;

import java.time.Instant;
import java.util.Arrays;
import javax.swing.*;
import javax.swing.Timer;

public class LambdaTest {
  public static void main(String[] args) {
    var planets = new String[]{"Mercury", "Venus", "Earth", "Mars", "Jupiter", "Saturn", "Uranus", "Neptune"};
    System.out.println(Arrays.toString(planets));
    System.out.println("Sorted in dictionary order:");
    Arrays.sort(planets);
    System.out.println(Arrays.toString(planets));
    System.out.println("Sorted by length:");
    Arrays.sort(planets, (first, second) -> first.length() - second.length());
    System.out.println(Arrays.toString(planets));

    var timer = new Timer(1000, event -> System.out.println("The time is " + Instant.ofEpochMilli(event.getWhen())));
    timer.start();

    // keep program running until user selects "OK"
    JOptionPane.showMessageDialog(null, "Quit program?");
    System.exit(0);
  }
}