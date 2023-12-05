package innerClass;

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.time.*;

public class InnerClassTest {
  public static void main(String[] args) {
    var clock = new TalkingClock(1000, true);
    clock.start();

    JOptionPane.showMessageDialog(null, "Quit program?");
    System.exit(0);
  }
}

class TalkingClock {
  private int interval;
  private boolean beep;

  public TalkingClock(int interval, boolean beep) {
    this.interval = interval;
    this.beep = beep;
  }
  public void start() {
    var listener = new TimePrinter();
    var timer = new Timer(interval, listener);
    timer.start();
  }

  public class TimePrinter implements ActionListener {
    public void actionPerformed(ActionEvent event) {
      System.out.println("At the tone, the time is "
          + Instant.ofEpochMilli(event.getWhen()));
      if (beep) Toolkit.getDefaultToolkit().beep();
    }
  }
}