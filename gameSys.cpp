#include <SFML/Graphics.hpp>
#include <iostream> //for debug
#include <cstdlib> //for rand
#include <ctime> //for seeding rand
#include <cmath> 
#include <string> //for score concat

#include "gameSys.hpp"
#include "gameParams.hpp"
#include "Systems.hpp"

EntityManager mgr;
sf::RectangleShape check;

Entity player;
Entity enemy;

void GameSys::init()
{
    player = mgr.CreateEntity();
    mgr.add<Health>(player, Health{5});
    mgr.add<Position>(player, Position{100,100});
    mgr.add<Velocity>(player, Velocity{100,100});
    mgr.add<Rect>(player, Rect{sf::RectangleShape(sf::Vector2f(50,50))});

    enemy = mgr.CreateEntity();
    mgr.add<Health>(enemy, Health{1});
    mgr.add<Position>(enemy, Position{300,100});
    mgr.add<Circle>(enemy, Circle{sf::CircleShape(20)});
    mgr.add<Velocity>(enemy, Velocity{100,100});
    mgr.add<EnemyMovement>(enemy, EnemyMovement{100, player});
}

void GameSys::update(const float &dt) 
{
    mgr.Update(dt);
}

void GameSys::render(sf::RenderWindow &window) 
{
    mgr.Draw(window);
}

void GameSys::clean()
{
	
}