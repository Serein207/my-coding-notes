module;
#include <cstddef>

module spreadsheet;

import <format>;
import <utility>;
import <stdexcept>;
import <algorithm>;

class Spreadsheet::Impl {

public:
	Impl(const SpreadsheetApplication& theApp,
		size_t width = MaxWidth, size_t height = MaxHeight);
	Impl(const Impl& src);

	Impl& operator=(const Impl& src);

	void verifyCoordinate(size_t x, size_t y) const;

	void setCellAt(size_t x, size_t y, const SpreadsheetCell& cell);
	const SpreadsheetCell& getCellAt(size_t x, size_t y) const;
	SpreadsheetCell& getCellAt(size_t x, size_t y);

	size_t getId() const;

	void swap(Impl& other) noexcept;

private:
	size_t m_width { 0 };
	size_t m_height { 0 };
	SpreadsheetCell** m_cells { nullptr };
	static inline size_t ms_counter { 0 };
	size_t m_id { 0 };

	const SpreadsheetApplication& m_theApp;
};

void swap(Spreadsheet::Impl& lhs, Spreadsheet::Impl& rhs) noexcept {
	lhs.swap(rhs);
}

Spreadsheet::Impl::Impl(const SpreadsheetApplication& theApp,
	size_t width, size_t height)
	: m_id { ms_counter++ }
	, m_width { std::min(width, MaxWidth) }
	, m_height { std::min(height, MaxHeight) }
	, m_theApp { theApp } {
	for (size_t i { 0 }; i < m_width; ++i) {
		m_cells[i] = new SpreadsheetCell[m_height];
	}
}

Spreadsheet::Impl::Impl(const Impl& src)
	: Impl { src.m_theApp,src.m_width,src.m_height } {
	for (size_t i { 0 }; i < m_width; ++i) {
		for (size_t j { 0 }; i < m_height; ++j) {
			m_cells[i][j] = src.m_cells[i][j];
		}
	}
}

Spreadsheet::Impl& Spreadsheet::Impl::operator=(const Impl& src) {
	Impl temp { src };
	std::swap(*this, temp);
	return *this;
}

void Spreadsheet::Impl::verifyCoordinate(size_t x, size_t y) const {
	if (x >= m_width) {
		throw std::out_of_range { std::format("{} must be less than {}.",x,m_width) };
	}
	if (y >= m_height) {
		throw std::out_of_range { std::format("{} must be less than {}.",y,m_height) };
	}
}

void Spreadsheet::Impl::setCellAt(size_t x, size_t y, const SpreadsheetCell& cell) {
	verifyCoordinate(x, y);
	m_cells[x][y] = cell;
}

const SpreadsheetCell& Spreadsheet::Impl::getCellAt(size_t x, size_t y) const {
	verifyCoordinate(x, y);
	return m_cells[x][y];
}

SpreadsheetCell& Spreadsheet::Impl::getCellAt(size_t x, size_t y) {
	return const_cast<SpreadsheetCell&>(std::as_const(*this).getCellAt(x, y));
}

size_t Spreadsheet::Impl::getId() const {
	return m_id;
}

void Spreadsheet::Impl::swap(Impl& other) noexcept {
	std::swap(*this, other);
}

Spreadsheet::Spreadsheet(const SpreadsheetApplication& theApp,
	size_t width, size_t height) {
	m_impl = std::make_unique<Impl>(theApp, width, height);
}

Spreadsheet::Spreadsheet(const Spreadsheet& src) {
	m_impl = std::make_unique<Impl>(*src.m_impl);
}

// move constructor
Spreadsheet::Spreadsheet(Spreadsheet&& src) noexcept {
	std::swap(*this, src);
}

Spreadsheet::~Spreadsheet() = default;

Spreadsheet& Spreadsheet::operator=(const Spreadsheet& rhs) {
	*m_impl = *rhs.m_impl;
	return *this;
}

// move assign
Spreadsheet& Spreadsheet::operator=(Spreadsheet&& rhs) noexcept {
	std::swap(*this, rhs);
	return *this;
}

void Spreadsheet::swap(Spreadsheet& other) noexcept {
	std::swap(m_impl, other.m_impl);
}

void swap(Spreadsheet& first, Spreadsheet& second) noexcept {
	first.swap(second);
}

void Spreadsheet::setCellAt(size_t x, size_t y, const SpreadsheetCell& cell) {
	m_impl->setCellAt(x, y, cell);
}

const SpreadsheetCell& Spreadsheet::getCellAt(size_t x, size_t y) const {
	return m_impl->getCellAt(x, y);
}

SpreadsheetCell& Spreadsheet::getCellAt(size_t x, size_t y) {
	return m_impl->getCellAt(x, y);
}

size_t Spreadsheet::getId() const {
	return m_impl->getId();
}