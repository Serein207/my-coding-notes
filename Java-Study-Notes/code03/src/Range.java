record Range(int from, int to) {
  public Range {
    if (from > to) {
      int tmp = from;
      from = to;
      to = tmp;
    }
  }
}