import spreadsheet_cell;
import <format>;
import <iostream>;
import <memory>;

int main() {
	SpreadsheetCell myCell { 4 };
	SpreadsheetCell aThirdCell = myCell + 5.6;
	std::cout << aThirdCell.getValue() << std::endl;
}