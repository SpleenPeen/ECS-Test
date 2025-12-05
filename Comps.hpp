#pragma once

#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>
#include <vector>

using Entity = uint32_t;

//supporting structs and enums

enum damageGroup //to specify who the bullet hits
{
    friendly,
    enemy,
    both
};

struct Weapon
{
    float fireRate = 0; //bullets per second
    float fireDelay = 0;
    int bulletSpeed = 0;
    int bulletSpread = 0; //degrees of offset
    int bulletsShot = 0;
    int speedVariation = 0;
    float bulletLifetime = 0;
    int damage = 0;
    int bulletRadius = 0;
    int pierce = 0;
    damageGroup dGroup = both;
};

//can be added to entities
struct Position
{
    sf::Vector2f pos;
};

struct Velocity
{
    sf::Vector2f vel;
};

struct Health
{
    int hp = 3;
    damageGroup dGroup;
};

struct RenderHitboxes
{
    sf::Color col = sf::Color::White;
};

struct PlayerMovement
{
    int moveSpd = 100;
};

struct WeaponArsenal
{
    int selected = 0;
    std::vector<Weapon> weapons;
};

struct CircleCollider
{
    int radius;
};

struct PlayerWeaponLogic
{
    
};

struct Bullet
{
    int damage = 1;
    int pierce = 0;
    damageGroup dGroup;
    float lifeTime;
};

struct Friction
{
    float friction;
};

struct EnemySafeMove
{
    Entity target;
    bool walkAndShoot;
    int moveSpd;
    std::vector<int> range;
};

struct EnemyShootingLogic
{
    float moveDelay; //amount of time to stand still after a shot
    Entity target;
    float moveTimer = 0;
};

//YOU NEED TO ADD YOUR NEW COMPONENTS HERE FOR THEM TO BE AVAILABLE ON THE ENTITIES
using AllComponents = std::tuple
<
    EnemyShootingLogic, EnemySafeMove, Friction, Position, Velocity, CircleCollider, 
    Health, RenderHitboxes, PlayerMovement, WeaponArsenal, Bullet, PlayerWeaponLogic
>;