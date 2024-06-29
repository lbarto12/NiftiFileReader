#pragma once

#include <iostream>
#include <unordered_map>
#include <any>
#include <mutex>
#include <functional>

#include "Window.h"

struct IPCItem {
	IPCItem(std::any resource): resource(resource) {}
	std::any resource;
	std::mutex mutex;
};

typedef std::unordered_map<std::string, IPCItem*> IPCItemSet;

class IPCEventListenerBase abstract {
public: 
	virtual void launch() = 0;
	virtual void kill() = 0;
	virtual void awaitExit() = 0;

	std::string observed;
};

class IPC {
public:


	static void atomicAll(std::function<void(IPCItemSet&)> modify) {
		globalMutex.lock();
		modify(resources);
		globalMutex.unlock();
	}

	template <typename T>
	static void atomic(const std::string& item, std::function<void(T&)> modify) {
		auto resource = resources[item];
		std::lock_guard<std::mutex> lock(resource->mutex);
		modify(*(std::any_cast<T*>(resource->resource)));
	}

	template <typename T>
	static void atomicSet(std::vector<const std::string> targets, std::function<void(IPCItemSet)> modify) {
		IPCItemSet slice;
		for (const auto& i : targets) slice[i] = resources[i];

		std::lock_guard<std::mutex> lock(globalMutex);
		for (auto& i : slice) i.second->mutex.lock();
		modify(slice);
		for (auto& i : slice) i.second->mutex.unlock();
	}

	template <typename T>
	static T& create(const std::string& item, T* resource) {
		if (resources.find(item) != resources.end()) {
			std::cout << "IPC::create(" << item << "): Variable Already Exists, Item not added\n";
			return *resource;
		}
		resources[item] = new IPCItem(resource);
		deleters[item] = [&](std::any& a) {
			delete std::any_cast<T*>(a);
		};
		return *resource;
	}


	template <typename T>
	static T& get(const std::string& item) {
		resources[item]->mutex.lock();
		T& r = *std::any_cast<T*>(resources[item]->resource);
		resources[item]->mutex.unlock();
		return r;
	}

	static Window* BindWindow(Window* window, std::mutex& windowMutex) {
		serviceWindows[window] = &windowMutex;
		return window;
	}

	template <typename EVENTLISTENER = IPCEventListenerBase>
	static EVENTLISTENER* BindListener(EVENTLISTENER* l) {
		listeners[l] = l;
		return l;
	}

	static void StopListener(IPCEventListenerBase* l) {
		l->kill();
		l->awaitExit();
		listeners.erase(l);
		delete l;
	}

	static void StopAllListeners() {
		std::cout << "Stopping Listeners...\n";
		for (auto& i : listeners) {
			i.first->kill();
			i.first->awaitExit();
			delete  i.first;
		}
		listeners.clear();
	}

	static void Clear() {
		std::cout << "Clearing IPC...\n";
		for (auto& i : deleters) {
			i.second(resources[i.first]->resource);
			std::cout << "Deleted: " << i.first << std::endl;
		}
		resources.clear();
	}

	static void DestroyWindows() {
		std::cout << "Destroying Service Windows...";
		for (auto& i : serviceWindows) {
			delete i.first;
			delete i.second;
		}
		std::cout << "DONE\n";
	}

	static void ShutDown() {
		StopAllListeners();
		Clear();
		DestroyWindows();
	}


	static std::unordered_map<Window*, std::mutex*> serviceWindows;
	static std::unordered_map<IPCEventListenerBase*, IPCEventListenerBase*> listeners;
private:
	static IPCItemSet resources;
	static std::unordered_map<std::string, std::function<void(std::any& a)>> deleters;
	static std::mutex globalMutex;
};

IPCItemSet IPC::resources;
std::unordered_map<std::string, std::function<void(std::any& a)>> IPC::deleters;

std::mutex IPC::globalMutex;
std::unordered_map<Window*, std::mutex*> IPC::serviceWindows;
std::unordered_map<IPCEventListenerBase*, IPCEventListenerBase*> IPC::listeners;