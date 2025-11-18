#pragma once

struct Health
{
    int hp = 100;
};

struct Position
{
    float x = 0;
    float y = 0;
};

struct Velocity
{
    float vx = 0;
    float vy = 0;
};

struct Rect
{
    sf::RectangleShape shape;
};

struct Circle
{
    sf::CircleShape shape;
};

struct EnemyMovement
{
    int moveSpd;
    size_t playerEnt;
};

using AllComponents = std::tuple<Health, Velocity, Position, Rect, Circle, EnemyMovement>;