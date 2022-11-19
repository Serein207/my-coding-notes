module Logger;

Logger::Logger() {
	// start background thread
	m_thread = std::thread { &Logger::processEntries, this };
}

void Logger::log(std::string entry) {
	// lock mutex and add entry to the queue
	std::unique_lock lock { m_mutex };
	m_queue.push(std::move(entry));
	// notify condition variable to wake up thread
	m_condVar.notify_all();
}

void Logger::processEntries() {
	// open log file
	std::ofstream logFile { "log.txt" };
	if (logFile.fail()) {
		std::cerr << "Fail to open logFile." << std::endl;
		return;
	}

	// create a lock for m_mutex, but do not yet acquire a lock on it
	std::unique_lock lock { m_mutex, std::defer_lock };
	// start processing loop
	while (true) {
		lock.lock();

		if (!m_exit) {	// only wait for notification if we don't have to exit
			m_condVar.wait(lock);
		}
		else {
			// we have to exit, process the remaining entries in the queue
			processEntriesHelper(m_queue, logFile);
			break;
		}

		// condition variable is notified, so something might be in the queue

		// while we till have the lock, swap the contents of the current queue
		// with an empty local queue on the stack
		std::queue<std::string> localQueue;
		localQueue.swap(m_queue);

		// now that all entries have been moved from the current queue to the
		// local queue, we can release the lock so other threads are not blocked
		// while we process the entries
		lock.unlock();

		// process the entries in the local queue on the stack. this happens after
		// having released the lock, so other threads are not blocked anymore
		processEntriesHelper(localQueue, logFile);
	}
}

void Logger::processEntriesHelper(std::queue<std::string>& queue,
	std::ofstream& ofs) const {
	while (!queue.empty()) {
		ofs << queue.front() << std::endl;
		queue.pop();
	}
}

Logger::~Logger() {
	{
		std::unique_lock lock { m_mutex };
		// gracefully shut down the thread by setting m_exit to true
		m_exit = true;
	}
	// notify condition variable to wake up thread
	m_condVar.notify_all();
	// wait until thread is shut down. this should be outside the above code
	// block because the lock must be released before calling join()
	m_thread.join();
}