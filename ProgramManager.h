#pragma once
#include <iostream>
#include <unordered_map>
#include "Service.h"

#include "IPC.h"

typedef std::unordered_map<Service*, Service*> ServiceProvider;

class ProgramManager abstract {
public:
	ProgramManager() {}

	virtual void Start() {};
	virtual void Update() {};
	virtual void OnExit() {};

	void Launch() {
		Start();
		while (isRunning) {
			std::vector<Service*> toErase;
			for (auto& service : services) { // stop services on window close
				if (!service.first->isRunning) {
					service.first->awaitExit();
					toErase.push_back(service.first);
				}
			}
			for (auto& i : toErase) {
				services.erase(i);
				delete i;
			}
			//std::cout << IPC::serviceWindows.size() << " ";
			for (auto& win : IPC::serviceWindows) { // manage all windows from main thread, required
				if (!win.first || !win.first->isOpen() || !win.second) continue;
				//std::lock_guard<std::mutex> lock(*win.second);
				win.first->handle();
			}
			Update();
		}
	}

	template <typename _Service = Service>
	_Service* LaunchService(_Service* service) {
		if (services.find(service) != services.end()) {
			if (service->isRunning) {
				std::cerr << "Cannot ReLaunch Duplicate Service: " << typeid(_Service).name() << std::endl;
				return service;
			}
			delete services[service];
		}
		services[service] = service;
		service->launch();
		std::cout << "Launched Service: " << typeid(_Service).name() << std::endl;
		return service;
	}

	template <typename _Service = Service>
	void StopService(Service* service) {
		service->kill();
		service->awaitExit();
		services.erase(service);
		delete service;
		std::cout << "Stopped Service: " << typeid(_Service).name() << std::endl;
	}

	void KillAll() {
		std::cout << "Stopping Services:\n";
		for (auto& i : services) {
			i.first->kill();
			i.first->awaitExit();
		}
		services.clear();

	}

	~ProgramManager() {
		OnExit();
	}

protected:
	bool isRunning = true;

	ServiceProvider services;
	
};