#include "MouseHelper.hpp"


sf::RenderWindow* MouseHelper::window;

void MouseHelper::SetWindow(sf::RenderWindow* rendWindow)
{
    window = rendWindow;
}

sf::Vector2i MouseHelper::GetMousePos()
{
    return sf::Mouse::getPosition(*window);
}