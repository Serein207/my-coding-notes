export module spreadsheet;
export import spreadsheet_cell;
export import string_spreadsheet_cell;
export import double_spreadsheet_cell;

import <string_view>;
import <string>;
import <memory>;

export class SpreadsheetApplication {};

export class Spreadsheet {
public:
	Spreadsheet(const SpreadsheetApplication& theApp,
		size_t width = MaxWidth, size_t height = MaxHeight);
	Spreadsheet(const Spreadsheet& src);
	Spreadsheet(Spreadsheet&& src) noexcept;	// move constructor
	~Spreadsheet();

	Spreadsheet& operator=(const Spreadsheet& rhs);
	Spreadsheet& operator=(Spreadsheet&& rhs) noexcept;	// move assign

	void setCellAt(size_t x, size_t y, const SpreadsheetCell& cell);
	const SpreadsheetCell& getCellAt(size_t x, size_t y) const;
	SpreadsheetCell& getCellAt(size_t x, size_t y);

	size_t getId() const;

	static const size_t MaxHeight { 100 };
	static const size_t MaxWidth { 100 };

	void swap(Spreadsheet& other) noexcept;

private:
	class Impl;
	std::unique_ptr<Impl> m_impl;
	friend void swap(Spreadsheet::Impl& lhs, Spreadsheet::Impl& rhs) noexcept;
};

export void swap(Spreadsheet& first, Spreadsheet& second) noexcept;