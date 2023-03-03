export module grid:main;
import <vector>;
import <optional>;
import <stdexcept>;
import <format>;

export template <typename T>
class Grid {
public:
  explicit Grid(size_t width = DefaultWidth,
    size_t height = DefaultHeight);
  virtual ~Grid() = default;

  // explicit default a copy constructor and assignment operator
  Grid(const Grid& src) = default;
  Grid<T>& operator=(const Grid<T>& rhs) = default;

  // explicit default a move constructor and assignment operator
  Grid(Grid&& src) = default;
  Grid<T>& operator=(Grid<T>&& src) = default;

  template <typename E>
  Grid(const Grid<E>& src);

  template <typename E>
  Grid<T>& operator=(const Grid<E>& rhs);

  std::optional<T>& at(size_t x, size_t y);
  const std::optional<T>& at(size_t x, size_t y) const;

  size_t getHeight() const { return m_height; }
  size_t getWidth() const { return m_width; }

  static const size_t DefaultWidth { 10 };
  static const size_t DefaultHeight { 10 };

  void swap(Grid& other) noexcept;

private:
  void verifyCoordinate(size_t x, size_t y) const;

  std::vector<std::vector<std::optional<T>>> m_cells;
  size_t m_width { 0 }, m_height { 0 };
};

template <typename T>
Grid<T>::Grid(size_t width, size_t height)
  : m_width { width }, m_height { height } {
  m_cells.resize(m_width);
  for (auto& column : m_cells) {
    column.resize(m_height);
  }
}

template <typename T>
template <typename E>
Grid<T>::Grid(const Grid<E>& src)
  :Grid { src.getWidth(),src.getHeight() } {
  for (size_t i { 0 }; m_width; i++) {
    for (size_t j { 0 }; j < m_height; j++) {
      m_cells[i][j] = src.at(i, j);
    }
  }
}

template <typename T>
void Grid<T>::verifyCoordinate(size_t x, size_t y) const {
  if (x >= m_width) {
    throw std::out_of_range {
      std::format("{} must be less than {}.", x, m_width) };
  }
  if (y >= m_height) {
    throw std::out_of_range {
      std::format("{} must be less than {}.", y, m_height) };
  }
}

template <typename T>
const std::optional<T>& Grid<T>::at(size_t x, size_t y) const {
  verifyCoordinate(x, y);
  return m_cells[x][y];
}

template <typename T>
std::optional<T>& Grid<T>::at(size_t x, size_t y) {
  return const_cast<std::optional<T>&>(std::as_const(*this).at(x, y));
}

template <typename T>
void Grid<T>::swap(Grid& other) noexcept {
  std::swap(m_width, other.m_width);
  std::swap(m_height, other.m_height);
  std::swap(m_cells, other.m_cells);
}

template <typename T>
template <typename E>
Grid<T>& Grid<T>::operator=(const Grid<E>& rhs) {
  Grid<T> temp { rhs };
  swap(temp);
  return *this;
}