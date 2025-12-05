#pragma once

#include "Reg.hpp"
#include "MouseHelper.hpp"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
#include "gameParams.hpp"

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
                HandleEnemySafeMove(curEnt);
                HandleEnemyShooting(curEnt, dt);
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
                //clamp movement to screen
                ClampToScreen(ent);
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

                arsenal->selected = std::min(arsenal->selected, (int)arsenal->weapons.size()-1);

                //shootgun
                if (!sf::Mouse::isButtonPressed(sf::Mouse::Left)) { return; }
                auto weapon = &arsenal->weapons[arsenal->selected]; 
                auto mousePos = MouseHelper::GetMousePos();
                auto pos = get<Position>(ent);
                Shoot(weapon, (sf::Vector2f)mousePos, pos->pos);
            }
        }

        bool Shoot(Weapon* weapon, sf::Vector2f target, sf::Vector2f spawnPos, int range = -1)//-1 means doesn't care
        {
            if (weapon->bulletRadius <= 0) { return false; } //to prevent non defined weapons from shooting
            if (weapon->fireDelay > 0) { return false; }

            auto dir = target - spawnPos;
            auto magnitude = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            if (range >= 0 && magnitude > range) {return false;}
            dir /= magnitude;

            for (int i = 0; i < weapon->bulletsShot; i++)
            {
                auto curBullet = CreateEntity();
                add<Position>(curBullet, Position{spawnPos});
                add<Bullet>(curBullet, Bullet{weapon->damage, weapon->pierce, weapon->dGroup, weapon->bulletLifetime});
                auto newAngle = (std::atan2f(dir.y, dir.x)*180/M_PI + (rand() % (weapon->bulletSpread+1) - weapon->bulletSpread/2))*M_PI/180;
                auto newDir = sf::Vector2f(std::cosf(newAngle), std::sinf(newAngle));

                add<Velocity>(curBullet, {newDir * (float)(weapon->bulletSpeed+(rand() % (weapon->speedVariation*2+1))-weapon->speedVariation/2)});
                add<CircleCollider>(curBullet, {weapon->bulletRadius});
                //this is added for testing purposes
                sf::Color col = sf::Color::Red;
                if (weapon->dGroup == enemy) { col = sf::Color::Green; }
                add<RenderHitboxes>(curBullet, RenderHitboxes{col});
            }
            weapon->fireDelay = 1.f/weapon->fireRate;
            return true;
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

                for (int i = 0; i < (int)arsenal->weapons.size(); i++)
                {
                    auto weapon = &arsenal->weapons[i];
                    if (weapon->fireDelay <= 0) {continue;}
                    weapon->fireDelay = std::max(0.f, weapon->fireDelay - dt);
                }
            }
        }
    
        void HandleEnemySafeMove(Entity ent)
        {
            if (!has<EnemySafeMove, WeaponArsenal, Velocity, Position>(ent)){return;}
            if (has<EnemyShootingLogic>(ent))
            {
                if(get<EnemyShootingLogic>(ent)->moveTimer > 0) {return;}
            }
            //clamp movement to screen
            ClampToScreen(ent);
            auto enemyMove = get<EnemySafeMove>(ent);
            if (!Exists(enemyMove->target)){return;}

            auto vel = get<Velocity>(ent);

            sf::Vector2f dir = get<Position>(enemyMove->target)->pos - get<Position>(ent)->pos;
            float dist = std::sqrt(dir.x * dir.x + dir.y * dir.y);

            if (dist <= enemyMove->range[get<WeaponArsenal>(ent)->selected]) {return;}
            dir /= dist;
            vel->vel += dir * (float)enemyMove->moveSpd;
        }

        void ClampToScreen(Entity ent)
        {
            if (!has<Position>(ent)){return;}

            float offest = 0;
            if (has<CircleCollider>(ent))
            {
                offest = get<CircleCollider>(ent)->radius;
            }

            auto pos = get<Position>(ent);
            pos->pos.x = std::clamp(pos->pos.x, offest, (float)Params::gameW-offest);
            pos->pos.y = std::clamp(pos->pos.y, offest, (float)Params::gameH-offest);
        }
    
        void HandleEnemyShooting(Entity ent, const float& dt)
        {
            if (!has<Position, EnemyShootingLogic, WeaponArsenal>(ent)){return;}
            auto shootLog = get<EnemyShootingLogic>(ent);
            if (shootLog->moveTimer > 0) {shootLog->moveTimer -= dt;}
            if (!Exists(shootLog->target)) {return;}
            if (!has<Position>(shootLog->target)){return;}
            auto weaponArse = get<WeaponArsenal>(ent);
            int range = -1;
            if (has<EnemySafeMove>(ent))
            {
                auto sigma = get<EnemySafeMove>(ent)->range[weaponArse->selected];
                range = get<EnemySafeMove>(ent)->range[weaponArse->selected];
            }
            if (Shoot(&weaponArse->weapons[weaponArse->selected], get<Position>(shootLog->target)->pos, get<Position>(ent)->pos, range))
            {
                if (shootLog->moveDelay <= 0){return;}
                shootLog->moveTimer = shootLog->moveDelay;
            }
            
        }
    };