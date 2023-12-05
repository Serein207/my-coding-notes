record Point(double x, double y) {
  public static Point ORIGIN = new Point();

  public Point() {
    this(0, 0);
  }

  public static double distance(Point p, Point q) {
    return Math.hypot(p.x - q.x, p.y - q.y);
  }

  public double distanceFromOrigin() {
    return Math.hypot(x, y);
  }
}