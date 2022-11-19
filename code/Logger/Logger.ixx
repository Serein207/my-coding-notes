export module Logger;
import <string>;
import <queue>;
import <mutex>;
import <condition_variable>;
import <thread>;
import <iostream>;
import <fstream>;

export class Logger {
public:
	// starts a background thread writing log entries to a file
	Logger();
	// gracefully shut down background thread
	virtual ~Logger();
	// prevent copy construction and assignment
	Logger(const Logger& src) = delete;
	Logger& operator=(const Logger& rhs) = delete;
	// add log entry to the queue
	void log(std::string entry);
private:
	// the function running in the background thread
	void processEntries();
	// helper method to process a queue of entries
	void processEntriesHelper(std::queue<std::string>& queue,
		std::ofstream& ofs) const;
	// mutex and condition variable to protect access to the queue
	std::mutex m_mutex;
	std::condition_variable m_condVar;
	std::queue<std::string> m_queue;
	// the background thread
	std::thread m_thread;
	// boolean telling the background thread to terminate
	bool m_exit { false };
};