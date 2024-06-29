#pragma once

#include "Service.h";
#include "NiftiUtil.h";
#include "UI.h";

class ViewService : public Service {
public:
	ViewService(Slider* slider, sf::Vector2i position, const std::string& viewName, std::function<void(size_t, size_t, sf::Image&, nifti_1_header*, const std::vector<std::vector<uint8_t>>&)> calc) 
		: Service(new Window({ 400, 300 }, "view", sf::Style::Close)), calculate(calc), slider(slider) {
		window->setPosition(position);
		window->setTitle(viewName);
	}

	void Start() override {
		data = &IPC::get<std::vector<std::vector<uint8_t>>>("loadedData");

		header = &IPC::get<nifti_1_header>("header");

		render = new sf::RectangleShape((sf::Vector2f)window->getSize());
		render->setPosition(-float(window->getSize().x / 2), -float(window->getSize().y / 2));

		texture = new sf::Texture();

		setFrame(0);
		slider->onChange([&](int idx) {
			this->index = idx;
			setFrame(idx);
		});

	}

	void setFrame(size_t idx) {
		
		sf::Image image;
		calculate(time, index, image, header, *data);
		delete texture;
		texture = new sf::Texture();
		texture->loadFromImage(image);
		render->setTexture(texture);
	}

	void setTime(size_t time) {
		this->time = time;
		setFrame(index);
	}

	void Update() override { 
		if (index < 0) index = 0;
		if (index >= header->dim[3]) index = header->dim[3];

		if (IPC::get<bool>("resetViews")) {
			kill();
		}
	}

	void Render() override {
		window->draw(*render);
	}

	
	size_t index = 0, time = 0;
private:
	std::function<void(size_t, size_t, sf::Image&, nifti_1_header*, const std::vector<std::vector<uint8_t>>&)> calculate;
	std::vector<std::vector<uint8_t>>* data = nullptr;
	nifti_1_header* header = nullptr;
	sf::RectangleShape* render = nullptr;
	sf::Texture* texture = nullptr;
	Slider* slider = nullptr;
};
