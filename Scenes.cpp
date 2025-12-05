#include "Scenes.hpp"
#include "tile_level_loader/level_system.hpp"
#include "Comps.hpp"

using ls = LevelSystem;

void Scene::Update(const float& dt)
{
    _entMan.Update(dt);
}

void Scene::Draw(sf::RenderWindow& window)
{
    _entMan.Draw(window);
}

SafeHouse::SafeHouse()
{
    //add other components to the player
    auto player = _entMan.CreateEntity();
    _entMan.add<RenderHitboxes>(player, RenderHitboxes{sf::Color::White});
    _entMan.add<PlayerMovement>(player, PlayerMovement{100});
    _entMan.add<Position>(player, Position{sf::Vector2f(300,300)});
    _entMan.add<Velocity>(player, Velocity{sf::Vector2f(0,0)});
    _entMan.add<Friction>(player, Friction{20});
    _entMan.add<Health>(player, {3, friendly});
    _entMan.add<CircleCollider>(player, CircleCollider{30});

    WeaponArsenal playerArsenal;

    playerArsenal.weapons.push_back(Weapon{});
    playerArsenal.weapons[0].bulletRadius = 10;
    playerArsenal.weapons[0].bulletSpeed = 200;
    playerArsenal.weapons[0].bulletsShot = 1;
    playerArsenal.weapons[0].bulletLifetime = 100;
    playerArsenal.weapons[0].damage = 1;
    playerArsenal.weapons[0].dGroup = damageGroup::enemy;
    playerArsenal.weapons[0].fireRate = 2;
    playerArsenal.weapons[0].pierce = 0;

    playerArsenal.weapons.push_back(Weapon{});
    playerArsenal.weapons[1].bulletRadius = 20;
    playerArsenal.weapons[1].bulletSpeed = 100;
    playerArsenal.weapons[1].bulletsShot = 1;
    playerArsenal.weapons[1].bulletLifetime = 1;
    playerArsenal.weapons[1].damage = 1;
    playerArsenal.weapons[1].dGroup = damageGroup::enemy;
    playerArsenal.weapons[1].fireRate = 10;
    playerArsenal.weapons[1].pierce = 0;

    _entMan.add<WeaponArsenal>(player, playerArsenal);
    _entMan.add<PlayerWeaponLogic>(player,{});

    //test enemy
    auto enemy = _entMan.CreateEntity();
    _entMan.add<RenderHitboxes>(enemy, RenderHitboxes{sf::Color::White});
    _entMan.add<Position>(enemy, Position{sf::Vector2f(500,300)});
    _entMan.add<Velocity>(enemy, Velocity{sf::Vector2f(0,0)});
    _entMan.add<Friction>(enemy, Friction{20});
    _entMan.add<CircleCollider>(enemy, CircleCollider{30});
    _entMan.add<Health>(enemy, {10, damageGroup::enemy});
    _entMan.add<EnemySafeMove>(enemy, EnemySafeMove{player, true, 50, {100, 400}});
    playerArsenal.weapons[0].dGroup = damageGroup::friendly;
    playerArsenal.weapons[1].dGroup = damageGroup::friendly;
    _entMan.add<WeaponArsenal>(enemy, playerArsenal);
    _entMan.add<EnemyShootingLogic>(enemy, EnemyShootingLogic{0.5f, player});
}