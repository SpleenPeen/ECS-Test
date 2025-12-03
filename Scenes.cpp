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
    //create a world in the ent manager - necessary for collisions
    auto wdef = b2DefaultWorldDef();
    wdef.gravity = b2Vec2_zero;
    _entMan.CreateWorld(&wdef);

    //define and add a player body
    b2BodyDef tDef = b2DefaultBodyDef();
    tDef.position = b2Vec2{12.5, 5};
    tDef.linearDamping = 60;
    tDef.type = b2_dynamicBody;
    auto player = _entMan.CreateEntity();
    _entMan.add<RigidBody>(player, RigidBody{b2CreateBody(_entMan.GetWorldID(), &tDef)});
    
    //add a shape to the player body 
    b2ShapeDef tsdef = b2DefaultShapeDef();
    b2Circle cir;
    cir.center = b2Vec2{0};
    cir.radius = 3;
    b2CreateCircleShape(_entMan.get<RigidBody>(player)->bodyID, &tsdef, &cir);

    //add other components to the player
    _entMan.add<RenderHitboxes>(player, RenderHitboxes{sf::Color::White});
    _entMan.add<PlayerMovement>(player, PlayerMovement{300});
    WeaponArsenal playerArsenal;

    playerArsenal.weapons.push_back(Weapon{});

    playerArsenal.weapons[0].bulletRadius = 1;
    playerArsenal.weapons[0].bulletSpeed = 100;
    playerArsenal.weapons[0].bulletsShot = 1;
    playerArsenal.weapons[0].damage = 1;
    playerArsenal.weapons[0].dGroup = damageGroup::player;
    playerArsenal.weapons[0].fireRate = 10;
    playerArsenal.weapons[0].pierce = 0;

    _entMan.add<WeaponArsenal>(player, playerArsenal);
    _entMan.add<PlayerWeaponLogic>(player,{});
}