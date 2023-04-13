import Logger;
import <format>;
import <vector>;
import <functional>;

void logSomeMessages(int id, Logger& logger) {
	for (int i { 0 }; i < 10; ++i) {
		logger.log(std::format("Log entry {} from thread {}", i, id));
	}
}

int main() {
	Logger logger;
	std::vector<std::thread> threads;
	// create a few threads all working with the same Logger instance
	for (int i { 0 }; i < 10; ++i) {
		threads.emplace_back(logSomeMessages, i, std::ref(logger));
	}
	// wait for all threads to finish
	for (auto& t : threads) {
		t.join();
	}
}