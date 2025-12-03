#pragma once

#include "Reg.hpp"
#include "MouseHelper.hpp"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>

class EntityManager : public Registry
{
    public:
        void Update(const float &dt)
        {
            for (auto ent : entToBit)
            {
                auto curEnt = ent.first;
                
                HandlePlayerMovement(curEnt);
                HandlePlayerWeapons(curEnt);
                ShootDelay(curEnt, dt);
            }
            HandleCreationAndDestruction();
            b2World_Step(_worldID, 0.016f, 4);
        }

        void Draw(sf::RenderWindow &window)
        {
            for (auto ent : entToBit)
            {
                auto curEnt = ent.first;
                DrawHitboxes(window, curEnt);
            }
        }

        b2WorldId GetWorldID()
        {
            return _worldID;
        }

        void CreateWorld(b2WorldDef *worldDef)
        {
            _worldID = b2CreateWorld(worldDef);
        }
    private:
        b2WorldId _worldID;

        //this is used in the drawhitboxes method
        sf::Vector2f GetPolySize(b2Polygon poly)
        {
            //returns the size of a polygon (in b2 scale)
            sf::Vector2f xRange = sf::Vector2f(poly.vertices[0].x, poly.vertices[0].x);
            sf::Vector2f yRange = sf::Vector2f(poly.vertices[0].y, poly.vertices[0].y);
            for (int i = 1; i < sizeof(poly.vertices)/sizeof(b2Vec2); i++)
            {
                if (poly.vertices[i].x < xRange.x) {xRange.x = poly.vertices[i].x;}
                else if (poly.vertices[i].x > xRange.y) {xRange.y = poly.vertices[i].x;}

                if (poly.vertices[i].y < yRange.x) {yRange.x = poly.vertices[i].y;}
                else if (poly.vertices[i].y > yRange.y) {yRange.y = poly.vertices[i].y;}
            }
            return sf::Vector2f(xRange.y-xRange.x, yRange.y-yRange.x);
        }

        void DrawHitboxes(sf::RenderWindow &window, Entity ent)
        {
            if (has<RigidBody, RenderHitboxes>(ent))
            {
                auto id = get<RigidBody>(ent)->bodyID;

                //can only render a circle or a square (ADD ROTATION BTW YOU MORON)
                int shapeCount = b2Body_GetShapeCount(id);
                b2ShapeId shapeIDs[shapeCount];
                b2Body_GetShapes(id, shapeIDs, shapeCount);

                for (int i = 0; i < shapeCount; i++)
                {
                    auto type = b2Shape_GetType(shapeIDs[0]);

                    if (type == b2_polygonShape)
                    {
                        auto rectB2 = b2Shape_GetPolygon(shapeIDs[i]);
                        sf::RectangleShape rectSF;
                        auto posB2 = b2Body_GetTransform(id);
                        rectSF.setPosition(posB2.p.x*32, posB2.p.y*32);
                        auto size = GetPolySize(rectB2);
                        rectSF.setSize(sf::Vector2f(size.x*32, size.y*32));
                        rectSF.setOrigin(size.x*32/2, size.y*32/2);
                        rectSF.setFillColor(get<RenderHitboxes>(ent)->col);

                        window.draw(rectSF);
                    }
                    else if (type == b2_circleShape)
                    {
                        auto cirB2 = b2Shape_GetCircle(shapeIDs[i]);
                        sf::CircleShape cirSF;
                        auto posB2 = b2Body_GetTransform(id);
                        cirSF.setPosition(posB2.p.x*32, posB2.p.y*32);
                        cirSF.setRadius(cirB2.radius*32);
                        cirSF.setFillColor(get<RenderHitboxes>(ent)->col);
                        cirSF.setOrigin(cirB2.radius*32,cirB2.radius*32);
                        window.draw(cirSF);
                    }
                }
            }
        }

        void HandlePlayerMovement(Entity ent)
        {
            if (has<PlayerMovement, RigidBody>(ent))
            {
                b2Vec2 dir{ 0.f, 0.f };

                // Basic WASD / Arrow movement input
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) ||
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Left))  dir.x -= 1.f;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) ||
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) dir.x += 1.f;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) ||
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Up))    dir.y -= 1.f;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) ||
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Down))  dir.y += 1.f;

                if (dir.x != 0.f || dir.y != 0.f) 
                {
                    // Normalise direction so diagonal speed isn�t faster
                    const float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
                    dir.x /= len;
                    dir.y /= len;

                    //multiply by movespd
                    auto spd = get<PlayerMovement>(ent)->moveSpd;
                    dir.x *= spd;
                    dir.y *= spd;

                    b2Body_ApplyLinearImpulseToCenter(get<RigidBody>(ent)->bodyID, dir, true);
                }
            }
        }
    
        void HandlePlayerWeapons(Entity ent)
        {
            if (has<RigidBody, PlayerWeaponLogic, WeaponArsenal>(ent))
            {
                auto arsenal = get<WeaponArsenal>(ent);
                auto selected = get<WeaponArsenal>(ent)->selected;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Numpad1)) { selected = 0; }
                else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Numpad2)) { selected = 1; }
                else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Numpad3)) { selected = 2; }
                else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Numpad4)) { selected = 3; }

                //dont shoot if mouse button not pressed, or gun on cd
                auto weapon = &arsenal->weapons[selected]; 
                if (!sf::Mouse::isButtonPressed(sf::Mouse::Left)) { return; }
                if (weapon->fireDelay > 0) { return; }
                for (int i = 0; i < weapon->bulletsShot; i++)
                {
                auto mousePos = MouseHelper::GetMousePos();
                    b2Vec2 dir = b2Vec2{mousePos.x/32, mousePos.y/32} - b2Body_GetPosition(get<RigidBody>(ent)->bodyID);
                    auto magnitude = std::sqrt(dir.x * dir.x + dir.y * dir.y);
                    dir.x /= magnitude;
                    dir.y /= magnitude;

                    auto parentID = get<RigidBody>(ent)->bodyID;
                    auto curBullet = CreateEntity();
                    auto bodyDef = b2DefaultBodyDef();
                    bodyDef.type = b2_dynamicBody;

                    bodyDef.position = b2Body_GetPosition(parentID);
                    add<RigidBody>(curBullet, RigidBody{b2CreateBody(_worldID, &bodyDef)});
                    auto shapeDef = b2DefaultShapeDef();
                    b2Circle cir;
                    cir.center = b2Vec2_zero;
                    cir.radius = weapon->bulletRadius;
                    b2CreateCircleShape(get<RigidBody>(curBullet)->bodyID, &shapeDef, &cir);
                    add<Bullet>(curBullet, Bullet{weapon->damage, weapon->pierce, weapon->dGroup});
                    
                    dir *= weapon->bulletSpeed;
                    b2Body_ApplyLinearImpulseToCenter(get<RigidBody>(curBullet)->bodyID, dir, true);

                    auto fdef = b2DefaultFilterJointDef();
                    fdef.bodyIdA = parentID;
                    fdef.bodyIdB = get<RigidBody>(curBullet)->bodyID;
                    b2CreateFilterJoint(_worldID, &fdef);

                    add<RenderHitboxes>(curBullet, RenderHitboxes{sf::Color::Green});
                }
                weapon->fireDelay = 1.f/weapon->fireRate;
            }
        }
    
        void ShootDelay(Entity ent, const float &dt)
        {
            if(has<WeaponArsenal>(ent))
            {
                auto arsenal = get<WeaponArsenal>(ent);

                for (int i = 0; i < arsenal->weapons.size(); i++)
                {
                    auto weapon = &arsenal->weapons[0];
                    if (weapon->fireDelay <= 0) {continue;}
                    weapon->fireDelay = std::max(0.f, weapon->fireDelay - dt);
                }
            }
        }
    };