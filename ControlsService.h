#pragma once

#include "Service.h"
#include "UI.h"
#include "IPCEventListener.h"
#include "NiftiUtil.h"

class ControlsService : public Service {
public:
	ControlsService() : Service(new Window({500, 600}, "Control", sf::Style::Close)) {}

	void Start() {
		time = new Slider({0, 0});
		time->setPosition(0, -float(window->getSize().y / 2) + 25);

		sagittal = new Slider({ 0, 0 });
		sagittal->setPosition(0, -float(window->getSize().y / 2) + 50);

		coronal = new Slider({ 0, 0 });
		coronal->setPosition(0, -float(window->getSize().y / 2) + 75);

		axial = new Slider({ 0, 0 });
		axial->setPosition(0, -float(window->getSize().y / 2) + 100);


		chooseFile = new Button({500, 50},"Choose File", 24);
		chooseFile->setFillColor(sf::Color(55, 60, 67));
		chooseFile->setHoverFillColor(sf::Color(82, 89, 99));
		chooseFile->setPosition(0, window->getSize().y / 2 - 25);
		chooseFile->onClick([&]() {
			IPC::atomic<bool>("shouldLaunchFileService", [&](bool& flag) {
				flag = true;
			});
		});

		z.setFont(Resources::FONT);
		z.setCharacterSize(16);
		z.setString("Sagittal");
		z.setPosition(-160, -float(window->getSize().y / 2) + 50 - 10);

		x.setFont(Resources::FONT);
		x.setCharacterSize(16);
		x.setString("Axial");
		x.setPosition(-160, -float(window->getSize().y / 2) + 75 - 10);

		y.setFont(Resources::FONT);
		y.setCharacterSize(16);
		y.setString("Coronal");
		y.setPosition(-160, -float(window->getSize().y / 2) + 100 - 10);

		t.setFont(Resources::FONT);
		t.setCharacterSize(16);
		t.setString("Time");
		t.setPosition(-160, -float(window->getSize().y / 2) + 25 - 10);
		

	}

	void reset() {
		nifti_1_header header = IPC::get<nifti_1_header>("header");
		time->setRange({ 0, (float)header.dim[4] - 1 });
		sagittal->setRange({ 0, (float)header.dim[2] - 1 });
		axial->setRange({ 0, (float)header.dim[1] - 1 });
		coronal->setRange({ 0, (float)header.dim[3] - 1 });

		time->setValue(0);
		sagittal->setValue(0);
		axial->setValue(0);
		coronal->setValue(0);

	}

	void Update() override {
		time->listen(window);
		axial->listen(window);
		sagittal->listen(window);
		coronal->listen(window);
		chooseFile->listen(window);
	}

	void Render() override {
		window->draw(*time);
		window->draw(*axial);
		window->draw(*sagittal);
		window->draw(*coronal);
		window->draw(*chooseFile);

		window->draw(z);
		window->draw(x);
		window->draw(y);
		window->draw(t);

	}

	void OnExit() {
		IPC::atomic<bool>("terminate", [](bool& terminate) {
			terminate = true;
		});
	}

	~ControlsService() {
		delete time;
		delete axial;
		delete sagittal;
		delete coronal;
		delete chooseFile;
	}

	Slider* time = nullptr, *axial = nullptr, *sagittal = nullptr, * coronal = nullptr;
private:
	Button* chooseFile = nullptr;
	sf::Text z, x, y, t;

};

