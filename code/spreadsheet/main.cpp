import spreadsheet;
import <format>;
import <iostream>;
import <memory>;

int main() {
	DoubleSpreadsheetCell myDbl;
	myDbl.set(8.4);
	StringSpreadsheetCell result { myDbl + myDbl };
}