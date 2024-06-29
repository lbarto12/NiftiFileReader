#pragma once
#include <SFML/Graphics.hpp>

class Window : public sf::RenderWindow {
public:
	Window(
		const sf::Vector2u& size = { 800, 500 },
		const std::string& title = "Game Window",
		uint8_t style = sf::Style::Default
	) : sf::RenderWindow(sf::VideoMode(size.x, size.y), title, style),
		videoMode(sf::VideoMode(size.x, size.y)), 
		windowTitle(title), 
		style(style),
		view(createView(size))
	{
		this->resolution = sf::Vector2f(this->getSize().x, this->getSize().y);
		this->setFramerateLimit(120);
		this->view.setCenter(0, 0);
		this->setView(view);
	}

	bool handle() {
		while (this->pollEvent(this->event)) {
			if (shouldClose) {
				this->close();
				return false;
			}
			switch (this->event.type)
			{
			case sf::Event::Closed: this->close();
			case sf::Event::GainedFocus: focused = true; break;
			case sf::Event::LostFocus: focused = false; break;
			case sf::Event::MouseWheelMoved: scrollDelta = event.mouseWheel.delta; break;
			case sf::Event::Resized: {
				view = getLetterboxView(view, this->event.size.width, this->event.size.height);
				this->setView(view);
				break;
			}
			case sf::Event::KeyReleased: {
				switch (this->event.key.code)
				{
				case sf::Keyboard::F11: {
					if (this->fullscreen) this->create(this->videoMode, this->windowTitle, this->style);
					else this->create(this->videoMode, this->windowTitle, sf::Style::Fullscreen);
					this->fullscreen = !this->fullscreen;
					this->view = this->getLetterboxView(this->view, this->resolution.x, this->resolution.y);
					this->setView(view);
					break;
				}
				default:
					break;
				}
			}
			default: break;
			}
		}

		return this->isOpen();
	}


	sf::View getLetterboxView(sf::View view, int windowWidth, int windowHeight) // Preserves aspect ratio
	{
		float windowRatio = windowWidth / (float)windowHeight;
		float viewRatio = view.getSize().x / (float)view.getSize().y;
		float sizeX = 1, sizeY = 1, posX = 0, posY = 0;
		bool horizontalSpacing = true;
		if (windowRatio < viewRatio) horizontalSpacing = false;
		// If horizontalSpacing is true, the black bars will appear on the left and right side.
		// Otherwise, the black bars will appear on the top and bottom.
		if (horizontalSpacing) { sizeX = viewRatio / windowRatio; posX = (1 - sizeX) / 2.f; }
		else { sizeY = windowRatio / viewRatio; posY = (1 - sizeY) / 2.f; }
		view.setViewport(sf::FloatRect(posX, posY, sizeX, sizeY));
		return view;
	}

	sf::View createView(sf::Vector2u res) {
		sf::View view;
		view.setSize(res.x, res.y);
		view.setCenter(view.getSize().x / 2, view.getSize().y / 2);
		view = getLetterboxView(view, res.x, res.y);
		return view;
	}

	void dispatchClose() {
		shouldClose = true;
	}

	void resetScroll() {
		scrollDelta = 0;
	}

public:
	bool focused = true;
	bool shouldClose = false;

	sf::Event event;
	bool fullscreen = false;
	sf::VideoMode videoMode;
	std::string windowTitle;
	uint8_t style;
	int scrollDelta = 0;

	sf::Vector2f resolution;
	sf::View view;
};