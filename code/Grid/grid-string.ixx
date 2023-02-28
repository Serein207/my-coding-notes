export module grid:string;

import :main;

export template <>
class Grid<const char*> {
public:
	explicit Grid(size_t width = DefaultWidth,
		size_t height = DefaultHeight);
	virtual ~Grid() = default;

	// explicit default a copy constructor and assignment operator
	Grid(const Grid& src) = default;
	Grid& operator=(const Grid& rhs) = default;

	// explicit default a move constructor and assignment operator
	Grid(Grid&& src) = default;
	Grid& operator=(Grid&& src) = default;

	std::optional<std::string>& at(size_t x, size_t y);
	const std::optional<std::string>& at(size_t x, size_t y) const;

	size_t getHeight() const { return m_height; }
	size_t getWidth() const { return m_width; }

	static const size_t DefaultWidth { 10 };
	static const size_t DefaultHeight { 10 };

	void swap(Grid& other) noexcept;

private:
	void verifyCoordinate(size_t x, size_t y) const;

	std::vector<std::vector<std::optional<std::string>>> m_cells;
	size_t m_width { 0 }, m_height { 0 };
};

Grid<const char*>::Grid(size_t width, size_t height) 
	: m_width { width }, m_height { height } {
	m_cells.resize(m_width);
	for (auto& column : m_cells) {
		column.resize(m_height);
	}
}

void Grid<const char*>::verifyCoordinate(size_t x, size_t y) const {
	if (x >= m_width) {
		throw std::out_of_range {
			std::format("{} must be less than {}.", x, m_width) };
	}
	if (y >= m_height) {
		throw std::out_of_range {
			std::format("{} must be less than {}.", y, m_height) };
	}
}

const std::optional<std::string>& Grid<const char*>::at(
	size_t x, size_t y) const {
	verifyCoordinate(x, y);
	return m_cells[x][y];
}

std::optional<std::string>& Grid<const char*>::at(size_t x, size_t y) {
	return const_cast<std::optional<std::string>&>(
		std::as_const(*this).at(x, y));
}