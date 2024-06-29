#pragma once


#include <fstream>
#include <vector>
#include <functional>

#include "IPC.h"
#include "Resources.h"
#include <Windows.h>
#include "Window.h"
#include "Button.h"
#include "Service.h"

class FileService : public Service {
public:
	FileService() : Service(new Window({ 400, 300 }, "Choose File", sf::Style::Close)) {}

	void Start() override {

		system("dir /B .\\resources\\ > ./bi_dir_scan.txt");
		std::string line;
		std::ifstream stream("bi_dir_scan.txt");
		while (std::getline(stream, line))
			files.push_back(line);

		for (int i = 0; i < files.size(); ++i) {
			auto t = sf::Text(files[i], Resources::FONT, 17);
			t.setPosition(-float(this->window->getSize().x / 2) + 10, -float(this->window->getSize().y / 2) + i * 22 + 10);
			t.setOutlineThickness(2);
			render.push_back(t);
		}

		select.setFillColor(sf::Color(55, 60, 67));
		select.setHoverFillColor(sf::Color(82, 89, 99));
		select.setPosition(-100, 125);
		select.onClick([&]() {

			if (selected.empty()) return;
			IPC::atomic<std::string>("currentFile", [&](std::string& file) {
				file = selected;
			});
			kill();
		});

		cancel.setFillColor(sf::Color(55, 60, 67));
		cancel.setHoverFillColor(sf::Color(82, 89, 99));
		cancel.setPosition(100, 125);
		cancel.onClick([&]() {
			kill();
		});
	}

	void Update() override {
		select.listen(window);
		cancel.listen(window);

		if (window->scrollDelta) {
			long store = scrollPos;
			scrollPos += -window->scrollDelta * 20;
			scrollPos = std::max(long(0), std::min(scrollPos, long(20 * files.size())));
			if (scrollPos != store) {
				for (auto& i : render) {
					i.move(0, window->scrollDelta * 20);
				}
			}
			window->resetScroll();
		}

		auto mouse = window->mapPixelToCoords(sf::Mouse::getPosition(*window));

		if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
			for (auto& i : render) {
				i.setOutlineColor(sf::Color::Black);
				if (i.getGlobalBounds().contains(mouse)) {
					this->selected = i.getString();
					i.setOutlineColor(sf::Color(27, 89, 54));
				}
			}
		}

		for (auto& i : render) {
			i.setFillColor(sf::Color::White);
			if (i.getGlobalBounds().contains(mouse))
				i.setFillColor(sf::Color::Blue);
		}
	}

	void Render() override {
		for (const auto& i : render)
			window->draw(i);

		window->draw(select);
		window->draw(cancel);
	}

	void OnExit() override {
		IPC::atomic<bool>("fileServiceOpen", [&](bool& f) { f = false; });
	}

private:
	std::vector<std::string> files;

	Button select = { {200, 50}, "Select", 20 };
	Button cancel = { {200, 50}, "Cancel", 20 };
	std::vector<sf::Text> render;

	long scrollPos = 0;

public:
	std::string selected;
};