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
                
                HandleVelocity(curEnt, dt);
                HandleFriction(curEnt, dt);
                HandlePlayerMovement(curEnt);
                HandlePlayerWeapons(curEnt);
                ShootDelay(curEnt, dt);
                BulletLifeTime(curEnt, dt);
                HandleHealth(curEnt);
                HandleBulletColls(curEnt);
            }
            HandleCreationAndDestruction();
        }

        void Draw(sf::RenderWindow &window)
        {
            for (auto ent : entToBit)
            {
                auto curEnt = ent.first;
                DrawHitboxes(window, curEnt);
            }
        }

    private:
        void HandleVelocity(Entity ent, const float &dt)
        {
            if (has<Position, Velocity>(ent))
            {
                get<Position>(ent)->pos += get<Velocity>(ent)->vel*dt;
            }
        }

        void HandleFriction(Entity ent, const float &dt)
        {
            if (has<Velocity, Friction>(ent))
            {
                auto vel = get<Velocity>(ent);
                vel->vel -= (get<Friction>(ent)->friction*vel->vel)*dt;
            }
        }

        void DrawHitboxes(sf::RenderWindow &window, Entity ent)
        {
            if (has<CircleCollider, RenderHitboxes, Position>(ent))
            {
                auto collider = get<CircleCollider>(ent);
                auto pos = get<Position>(ent);
                auto render = get<RenderHitboxes>(ent);

                sf::CircleShape cir;
                cir.setRadius(collider->radius);
                cir.setPosition(pos->pos);
                cir.setOrigin(sf::Vector2f(cir.getRadius(), cir.getRadius()));
                cir.setFillColor(render->col);

                auto curPos = cir.getPosition();
                window.draw(cir);
            }
        }

        void HandlePlayerMovement(Entity ent)
        {
            if (has<PlayerMovement, Velocity, Position>(ent))
            {
                sf::Vector2f dir = {0,0};

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
                    dir /= len;

                    //multiply by movespd
                    auto spd = get<PlayerMovement>(ent)->moveSpd;
                    auto vel = &get<Velocity>(ent)->vel;
                    *vel += dir*(float)spd;
                }
            }
        }
    
        void HandlePlayerWeapons(Entity ent)
        {
            if (has<PlayerWeaponLogic, WeaponArsenal, Position>(ent))
            {
                auto arsenal = get<WeaponArsenal>(ent);
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1)) { arsenal->selected = 0; }
                else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2)) { arsenal->selected  = 1; }
                else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num3)) { arsenal->selected  = 2; }
                else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num4)) { arsenal->selected  = 3; }

                arsenal->selected = std::min(arsenal->selected, (int)(sizeof(arsenal->weapons)/sizeof(Weapon))-1);

                //dont shoot if mouse button not pressed, or gun on cd
                auto weapon = &arsenal->weapons[arsenal->selected]; 
                if (weapon->bulletRadius <= 0) { return; } //to prevent non defined weapons from shooting
                if (!sf::Mouse::isButtonPressed(sf::Mouse::Left)) { return; }
                if (weapon->fireDelay > 0) { return; }
                for (int i = 0; i < weapon->bulletsShot; i++)
                {
                    auto mousePos = MouseHelper::GetMousePos();
                    auto dir = (sf::Vector2f)mousePos - get<Position>(ent)->pos;
                    auto magnitude = std::sqrt(dir.x * dir.x + dir.y * dir.y);
                    dir /= magnitude;

                    auto curBullet = CreateEntity();
                    add<Position>(curBullet, {get<Position>(ent)->pos});
                    add<Bullet>(curBullet, Bullet{weapon->damage, weapon->pierce, weapon->dGroup, weapon->bulletLifetime});
                    add<Velocity>(curBullet, {dir * (float)weapon->bulletSpeed});
                    add<RenderHitboxes>(curBullet, RenderHitboxes{sf::Color::Green});
                    add<CircleCollider>(curBullet, {weapon->bulletRadius});
                }
                weapon->fireDelay = 1.f/weapon->fireRate;
            }
        }
    
        void BulletLifeTime(Entity ent, const float &dt)
        {
            if (has<Bullet>(ent))
            {
                auto bul = get<Bullet>(ent);
                bul->lifeTime -= dt;

                if (bul->lifeTime <= 0)
                {
                    Destroy(ent);
                }
            }
        }

        void HandleHealth(Entity ent)
        {
            if (has<Health>(ent))
            {
                auto health = get<Health>(ent);
                if (health->hp <= 0)
                {
                    Destroy(ent);
                }
            }
        }

        void HandleBulletColls(Entity ent)
        {
            if (has<Bullet>(ent)){return;}
            if (!has<Health, CircleCollider, Position>(ent)){return;}
            auto bullets = getAllEnt<Bullet>();
            auto eCol = get<CircleCollider>(ent);
            auto eHP = get<Health>(ent);
            auto ePos = get<Position>(ent);
            for (auto curBul : bullets)
            {
                if (!has<CircleCollider, Position>(curBul)){continue;}
                auto bul = get<Bullet>(curBul);
                if (bul->dGroup != eHP->dGroup){continue;}
                auto dist = ePos->pos - get<Position>(curBul)->pos;
                if (std::sqrt(dist.x * dist.x + dist.y * dist.y) > (eCol->radius + get<CircleCollider>(curBul)->radius)){continue;;}
                eHP->hp -= bul->damage;
                Destroy(curBul);
                
            }
        }

        void ShootDelay(Entity ent, const float &dt)
        {
            if(has<WeaponArsenal>(ent))
            {
                auto arsenal = get<WeaponArsenal>(ent);

                for (int i = 0; i < (int)(sizeof(arsenal->weapons)/sizeof(Weapon)); i++)
                {
                    auto weapon = &arsenal->weapons[i];
                    if (weapon->fireDelay <= 0) {continue;}
                    weapon->fireDelay = std::max(0.f, weapon->fireDelay - dt);
                }
            }
        }
    };