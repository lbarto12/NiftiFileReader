#pragma once
#include <iostream>
#include "IPC.h"
#include "Window.h"
#include <thread>
#include <Windows.h>

class Service abstract {
public:
	Service() {}
	Service(Window* window) : window(window) {
		window->setActive(false);
		if (window) IPC::BindWindow(window, windowMutex);
	}

	virtual void Start() {}
	virtual void Update() {}
	virtual void OnExit() {}
	virtual void Render() {}

	void _render() {
		if (!window) return;
		IPC::serviceWindows[window]->lock();
		window->clear();
		Render();
		window->display();
		IPC::serviceWindows[window]->unlock();
	}

	void launch() {
		Start();
		thread = std::thread(&Service::__thread_bind, this);
	}
	
	void kill() {
		killFlag = true;
		if (window) window->dispatchClose();
	}

	void awaitExit() {
		std::cout << "Stopping Service: " + std::string(typeid(*this).name()) + "... ";
		thread.join();
		if (window && IPC::serviceWindows.find(window) != IPC::serviceWindows.end()) IPC::serviceWindows.erase(window);
		std::cout << "DONE\n";
		delete window;
	}

	bool isRunning = true, killFlag = false;
protected:
	Window* window = nullptr;
	std::mutex windowMutex;
	std::thread thread;

	virtual void __thread_bind() {
		while (isRunning) {
			this->Update();
			if (window) {
				if (!window->isOpen()) {
					kill();
					isRunning = false;
					break;
				}
				this->_render();
			}
			if (killFlag) isRunning = false;
		}
		OnExit();
	}
};