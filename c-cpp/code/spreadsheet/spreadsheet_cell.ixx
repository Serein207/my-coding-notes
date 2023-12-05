export module spreadsheet_cell;
import <string>;
import <string_view>;

export class SpreadsheetCell {

public:
    virtual ~SpreadsheetCell() = default;
    virtual void set(std::string_view) { }
    virtual std::string getString() const { return ""; }
    [[nodiscard]] std::partial_ordering operator<=>(
        const SpreadsheetCell& rhs) const
        = default;

    enum class Color { Red = 1,
        Green,
        Blue,
        Yellow };
    void setColor(Color color) { m_color = color; }
    Color getColor() const { return m_color; }

private:
    std::string doubleToString(double value) const;
    double stringToDouble(std::string_view value) const;
    double m_value { 0 };
};