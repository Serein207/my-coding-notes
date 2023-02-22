module spreadsheet;

import <format>;
import <utility>;

Spreadsheet::Spreadsheet(size_t width, size_t height)
	: m_width { width }, m_height { height } {
	m_cells = new SpreadsheetCell * [m_width];
	for (size_t i { 0 }; i < m_width; ++i) {
		m_cells[i] = new SpreadsheetCell[m_height];
	}
}

Spreadsheet::Spreadsheet(const Spreadsheet& src) 
	: Spreadsheet { src.m_width, src.m_height } {
	for (size_t i { 0 }; i < m_width; ++i) {
		for(size_t j { 0 }; i < m_height; ++j) {
			m_cells[i][j] = src.m_cells[i][j];
		}
	}
}

Spreadsheet::~Spreadsheet() {
	for (size_t i { 0 }; i < m_width; ++i) {
		delete[] m_cells[i];
	}
	delete[] m_cells;
	m_cells = nullptr;
}

Spreadsheet& Spreadsheet::operator=(const Spreadsheet& rhs) {
	Spreadsheet temp { rhs };
	swap(temp);
	return *this;
}

void Spreadsheet::swap(Spreadsheet& other) noexcept {
	std::swap(m_height, other.m_height);
	std::swap(m_width, other.m_width);
	std::swap(m_cells, other.m_cells);
}

void swap(Spreadsheet& first, Spreadsheet& second) noexcept {
	first.swap(second);
}

void Spreadsheet::vertifyCoordinate(size_t x, size_t y) const {
	if (x >= m_width) {
		throw std::out_of_range { std::format("{} must be less than {}.",x,m_width) };
	}
	if (y >= m_height) {
		throw std::out_of_range { std::format("{} must be less than {}.",y,m_height) };
	}
}

void Spreadsheet::setCellAt(size_t x, size_t y, const SpreadsheetCell& cell) {
	vertifyCoordinate(x, y);
	m_cells[x][y] = cell;
}

SpreadsheetCell& Spreadsheet::getCellAt(size_t x, size_t y) const {
	vertifyCoordinate(x, y);
	return m_cells[x][y];
}