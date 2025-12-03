#include <SFML/Graphics.hpp>
#include <iostream> //for debug
#include <cstdlib> //for rand
#include <ctime> //for seeding rand
#include <cmath> 
#include <string> //for score concat

#include "gameSys.hpp"
#include "gameParams.hpp"
#include "Systems.hpp"
#include "Scenes.hpp"
#include "tile_level_loader/level_system.hpp"

using ls = LevelSystem;

enum Screen
{
    other,
    ts  
};

SafeHouse sfScene;

Screen curScreen;

void GameSys::init()
{
    sfScene = SafeHouse();
    curScreen = ts;
    ls::set_color(ls::EMPTY, sf::Color(10, 10, 30));
    ls::set_color(ls::WALL, sf::Color(60, 60, 80));
    ls::set_color(ls::WAYPOINT, sf::Color(120, 120, 120));
    ls::set_color(ls::START, sf::Color(80, 255, 80));
    ls::set_color(ls::END, sf::Color(255, 80, 80));
    ls::get_height();
    ls::get_width();
    ls::load_level("res/levels/td_1.txt", 50);
}

void GameSys::update(const float &dt) 
{
    switch (curScreen)
    {
        case ts:
            sfScene.Update(dt);
            break;
    }
}

void GameSys::render(sf::RenderWindow &window) 
{
    ls::render(window);
    switch (curScreen)
    {
        case ts:
            sfScene.Draw(window);
            break;
    }
}

void GameSys::clean()
{
	
}