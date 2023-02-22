import spreadsheet_cell;
import <format>;
import <iostream>;
import <memory>;

int main() {
	SpreadsheetCell aThirdCell { "test" };
	SpreadsheetCell aFourthCell { 4.4 };
	auto aFifthCell { std::make_unique<SpreadsheetCell>("5.5") };
	std::cout << "aThirdCell: " << aThirdCell.getValue() << std::endl;
	std::cout << "aFourthCell: " << aFourthCell.getValue() << std::endl;
	std::cout << "aFifthCell: " << aFifthCell->getValue() << std::endl;
}