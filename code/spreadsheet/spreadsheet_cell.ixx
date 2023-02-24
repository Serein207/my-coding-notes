export module spreadsheet_cell;
import <string>;
import <string_view>;

export class SpreadsheetCell {
public:
	SpreadsheetCell() = default;
	SpreadsheetCell(double initialValue);
	explicit SpreadsheetCell(std::string_view initialValue);
	SpreadsheetCell(const SpreadsheetCell& src);
	~SpreadsheetCell();

	SpreadsheetCell& operator=(const SpreadsheetCell&) = default;
	[[nodiscard]] std::partial_ordering operator<=>(
		const SpreadsheetCell& rhs) const = default;

	void setValue(double value);
	double getValue() const;

	void setString(std::string_view value);
	std::string getString() const;

	enum class Color { Red = 1, Green, Blue, Yellow };
	void setColor(Color color);
	Color getColor() const;

private:
	static std::string doubleToString(double value);
	static double stringToDouble(std::string_view value);
	double m_value { 0 };
	mutable size_t m_numAccess { 0 };
	Color m_color { Color::Red };
};

export SpreadsheetCell operator+(const SpreadsheetCell& lhs, 
	const SpreadsheetCell& rhs);