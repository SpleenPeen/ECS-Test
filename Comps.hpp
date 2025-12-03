#pragma once

#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>
#include <vector>

//supporting structs and enums

enum damageGroup //to specify who the bullet hits
{
    player,
    enemy,
    both
};

struct Weapon
{
    float fireRate; //bullets per second
    float fireDelay;
    int bulletSpeed;
    int bulletSpread; //degrees of offset
    int bulletsShot;
    int damage;
    int bulletRadius;
    int pierce;
    damageGroup dGroup;
};

//can be added to entities
struct Health
{
    int hp = 3;
};

struct RigidBody
{
    b2BodyId bodyID;
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
    std::vector<Weapon> weapons;
    int selected = 0;
};

struct PlayerWeaponLogic
{
    
};

struct Bullet
{
    int damage = 1;
    int pierce = 0;
    damageGroup dGroup;
};

//YOU NEED TO ADD YOUR NEW COMPONENTS HERE FOR THEM TO BE AVAILABLE ON THE ENTITIES
using AllComponents = std::tuple<Health, RigidBody, RenderHitboxes, PlayerMovement, WeaponArsenal, Bullet, PlayerWeaponLogic>;