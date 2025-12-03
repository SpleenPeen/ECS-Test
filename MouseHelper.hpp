#pragma once

#include <SFML/Graphics.hpp>

class MouseHelper
{
    private:
        static sf::RenderWindow* window;
    public:
        static void SetWindow(sf::RenderWindow* rendWindow);
        static sf::Vector2i GetMousePos();
};