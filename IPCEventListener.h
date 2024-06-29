#pragma once
#include <iostream>
#include <thread>
#include <functional>
#include "IPC.h"


template <typename T>
class IPCEventListener : public IPCEventListenerBase {
public:
	IPCEventListener(const std::string& observe): observed(observe), last(IPC::get<T>(observe)) {
	}

	IPCEventListener* listen(std::function<void(T&)> _callback) {
		callback = _callback;
		launch();
		return this;
	}

	void launch() override {
		thread = std::thread(&IPCEventListener::__thread_bind, this);
	}

	void kill() override {
		killflag = true;
	}

	void awaitExit() override {
		std::cout << "Stopping Listener: " << typeid(*this).name() << "... ";
		thread.join();
		std::cout << "DONE\n";
	}

	std::string observed;
private:
	std::thread thread;

	void __thread_bind() {
		std::cout << "IPCEventListener: Listening to (" << observed << ")\n";
		while (isRunning) {
			T val = IPC::get<T>(observed);
			if (val != last) {
				IPC::atomic<T>(observed, callback);
				last = val;
			}
			if (killflag) isRunning = false;
		}		
	}

	bool isRunning = true, killflag = false;
	std::function<void(T&)> callback;
	T last;
};