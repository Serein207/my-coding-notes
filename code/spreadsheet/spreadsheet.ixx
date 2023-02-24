module;
#include <cstddef>

export module spreadsheet;
export import spreadsheet_cell;

export class Spreadsheet {
public:
	Spreadsheet(size_t width=MaxWidth, size_t height=MaxHeight);
	Spreadsheet(const Spreadsheet& src);
	Spreadsheet(Spreadsheet&& src) noexcept;	// move constructor
	~Spreadsheet();

	Spreadsheet& operator=(const Spreadsheet& rhs);
	Spreadsheet& operator=(Spreadsheet&& rhs) noexcept;	// move assign
	void swap(Spreadsheet& other) noexcept;

	void setCellAt(size_t x, size_t y, const SpreadsheetCell& cell);
	const SpreadsheetCell& getCellAt(size_t x, size_t y) const;
	SpreadsheetCell& getCellAt(size_t x, size_t y);
	size_t getId() const;

	static const size_t MaxHeight { 100 };
	static const size_t MaxWidth { 100 };

private:
	void verifyCoordinate(size_t x, size_t y) const;
	size_t m_width { 0 };
	size_t m_height { 0 };
	SpreadsheetCell** m_cells { nullptr };
	static inline size_t ms_counter { 0 };
	size_t m_id { 0 };
};

export void swap(Spreadsheet& first, Spreadsheet& second) noexcept;