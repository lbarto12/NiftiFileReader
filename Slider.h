#pragma once

#include <functional>
#include "Window.h"

class Slider: public sf::Drawable, public sf::Transformable {
public:
	Slider(sf::Vector2f range, float size = 200, float thumbRadius = 10): range(range) {
		thumb = new sf::CircleShape(thumbRadius);
		track = new sf::RectangleShape({size, 3});

		thumb->setFillColor(sf::Color(55, 60, 67));
		track->setFillColor(sf::Color(227, 242, 253));

		thumb->setOrigin(thumbRadius, thumbRadius);
		track->setOrigin(size / 2, 1.5);

		thumb->setPosition(track->getGlobalBounds().left, 0);
	}



	void draw(sf::RenderTarget& target, sf::RenderStates states) const {
		target.draw(*track);
		target.draw(*thumb);
	}

	void onChange(std::function<void(int)> f) {
		on_change = f;
	}

	void listen(Window* window) {


		if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
			auto mousePos = window->mapPixelToCoords(sf::Mouse::getPosition(*window));
			bool mouseOver = thumb->getGlobalBounds().contains(mousePos);
			if (mouseOver && !dragging) {
				dragging = true;
				selected = thumb;
			}
			if (dragging && selected) {
				auto tb = track->getGlobalBounds();
				if (mousePos.x < tb.left)
					selected->setPosition(tb.left, selected->getPosition().y);
				else if (mousePos.x > tb.left + tb.width)
					selected->setPosition(tb.left + tb.width, selected->getPosition().y);
				else
					selected->setPosition(mousePos.x, selected->getPosition().y);

				int old = value;
				value = (std::abs(thumb->getPosition().x - tb.left) / tb.width) * (range.y - range.x);
				if (old != value && on_change) on_change(value);
			}
		}
		else if (!sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
			dragging = false;
			selected = nullptr;
		}
	}

	void setPosition(int x, int y) {
		track->setPosition(x, y);
		setValue(value);
	}

	void setValue(int val) {
		value = val;
		auto tb = track->getGlobalBounds();
		thumb->setPosition(tb.left + value / (range.y - range.x) * tb.width, track->getPosition().y);
	}

	void setRange(sf::Vector2f nRange) {
		range = nRange;
		this->setValue(0);
	}

	~Slider() {
		delete thumb;
		delete track;
	}

private:
	sf::Vector2f range;
	int value = 0;
	sf::CircleShape* thumb;
	sf::RectangleShape* track;

	sf::CircleShape* selected = nullptr;

	std::function<void(int)> on_change;

	static bool dragging;

};

bool Slider::dragging = false;