#pragma once

#include "Window.h"
#include <functional>

#include "Resources.h"

class Button : public sf::Drawable, public sf::Transformable {
public:
	Button(const sf::Vector2f& size, const std::string& txt, size_t textSize = 30) {
		text.setFont(Resources::FONT);
		text.setCharacterSize(textSize);
		text.setString(txt);
		body.setSize(size);

		text.setOrigin(text.getGlobalBounds().getSize() / 2.f + text.getLocalBounds().getPosition()); // nope

		auto b = body.getGlobalBounds();
		body.setOrigin(b.left + b.width / 2, b.top + b.height / 2);
	}

	void setPosition(float x, float y) {
		text.setPosition(x, y);
		body.setPosition(x, y);
	}

	void setText(const std::string& txt) {
		text.setString(txt);
	}

	void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
		target.draw(body, states);
		target.draw(text, states);
	}

	void setFillColor(sf::Color color) {
		fillColor = color;
		body.setFillColor(color);
	}

	void setHoverFillColor(sf::Color color) {
		hoverFillColor = color;
	}


	void onClick(std::function<void(void)> _callback) {
		callback = _callback;
	}

	void listen(Window* window) {

		bool mouseOver = this->body.getGlobalBounds().contains(window->mapPixelToCoords(sf::Mouse::getPosition(*window)));

		if (mouseOver && body.getFillColor() != hoverFillColor)
			body.setFillColor(hoverFillColor);
		else if (!mouseOver && body.getFillColor() != fillColor)
			body.setFillColor(fillColor);

		if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && !lDown) {
			if (mouseOver) {
				lDown = true;
				if (callback) callback();

			}
		}
		else if (!sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
			lDown = false;
		}
	}

	sf::RectangleShape& Body() {
		return body;
	}

	sf::Text& Text() {
		return text;
	}

private:
	std::function<void(void)> callback;
	sf::RectangleShape body;
	sf::Text text;

	sf::Color fillColor, hoverFillColor;

	static bool lDown;
};

bool Button::lDown = false;