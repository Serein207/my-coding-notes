module spreadsheet_cell;
import <charconv>;
import <iostream>;

SpreadsheetCell::SpreadsheetCell(double initialValue)
	: m_value { initialValue } {}

SpreadsheetCell::SpreadsheetCell(std::string_view initialValue)
	: SpreadsheetCell { stringToDouble(initialValue) } {}

SpreadsheetCell::SpreadsheetCell(const SpreadsheetCell& src)
	: m_value { src.m_value } {}

SpreadsheetCell::~SpreadsheetCell() {
	std::cout << "Destructor called" << std::endl;
}

void SpreadsheetCell::setValue(double value) {
	m_value = value;
}

double SpreadsheetCell::getValue() const {
	m_numAccess++;
	return m_value;
}

void SpreadsheetCell::setString(std::string_view value) {
	m_value = stringToDouble(value);
}

std::string SpreadsheetCell::getString() const {
	m_numAccess++;
	return doubleToString(m_value);
}

std::string SpreadsheetCell::doubleToString(double value) {
	return std::to_string(value);
}

double SpreadsheetCell::stringToDouble(std::string_view value) {
	double number { 0 };
	std::from_chars(value.data(), value.data() + value.size(), number);
	return number;
}

void SpreadsheetCell::setColor(Color color) { m_color = color; }

SpreadsheetCell::Color SpreadsheetCell::getColor() const { return m_color; }

SpreadsheetCell operator+(const SpreadsheetCell& lhs,
	const SpreadsheetCell& rhs) {
	return SpreadsheetCell { lhs.getValue() + rhs.getValue() };
}

