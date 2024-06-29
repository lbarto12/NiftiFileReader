#pragma once

#include "SFML/Graphics.hpp"

static class Resources {
public:
	static bool CREATE() {
		FONT.loadFromFile("font.ttf");
		return true;
	}

	static sf::Font FONT;

private:
	static bool __static_init;
};

sf::Font Resources::FONT = sf::Font();



// Static loads, etc.
bool Resources::__static_init = Resources::CREATE();

