export module spreadsheet_cell;
import <string>;
import <string_view>;

export class SpreadsheetCell {

public:
	virtual ~SpreadsheetCell() = default;
	virtual void set(std::string_view) = 0;
	virtual std::string getString() const = 0;
	[[nodiscard]] std::partial_ordering operator<=>(
		const SpreadsheetCell& rhs) const = default;

	enum class Color { Red = 1, Green, Blue, Yellow };
	void setColor(Color color);
	Color getColor() const;

private:
	Color m_color { Color::Red };
};
