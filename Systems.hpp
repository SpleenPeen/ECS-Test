#include "Reg.hpp"
#include <SFML/Graphics.hpp>

class EntityManager : public Registry
{
    public:
        void Update(const float &dt)
        {
            for (auto ent : entToBit)
            {
                auto curEnt = ent.first;
                
                UpdateEnemyMov(dt, curEnt);
                VelSys(dt, curEnt);
                ShapeUpdatePos(curEnt);
            }
            HandleCreationAndDestruction();
        }

        void Draw(sf::RenderWindow &window)
        {
            for (auto ent : entToBit)
            {
                auto curEnt = ent.first;
                DrawShape(window, curEnt);
            }
        }

    private:
        void DrawShape(sf::RenderWindow &window, Entity ent)
        {
            if(has<Rect>(ent))
            {
                window.draw(get<Rect>(ent)->shape);
            }
            if (has<Circle>(ent))
            {
                window.draw(get<Circle>(ent)->shape);
            }
        }

        void ShapeUpdatePos(Entity ent)
        {
            if (has<Position, Rect>(ent))
            {
                auto pos = get<Position>(ent);
                get<Rect>(ent)->shape.setPosition(pos->x, pos->y);
            }
            if (has<Position, Circle>(ent))
            {
                auto pos = get<Position>(ent);
                get<Circle>(ent)->shape.setPosition(pos->x, pos->y);
            } 
        }

        void VelSys(const float &dt, Entity ent)
        {
            if (!has<Position, Velocity>(ent)){return;}

            auto pos = get<Position>(ent);
            auto vel = get<Velocity>(ent);

            pos->x += vel->vx * dt;
            pos->y += vel->vy * dt;
        }

        void UpdateEnemyMov(const float &dt, Entity ent)
        {
            if (!has<Position, Velocity, EnemyMovement>(ent)) { return; }

            auto enemyMoveInfo =  get<EnemyMovement>(ent);
            auto playPos = get<Position>(enemyMoveInfo->playerEnt);
            auto enemyPos = get<Position>(ent);
            auto enemyVel = get<Velocity>(ent);

            float xdist = playPos->x - enemyPos->x;
            float ydist = playPos->y - enemyPos->y;
            float mult = enemyMoveInfo->moveSpd/(sqrt(pow(xdist, 2) + pow(ydist, 2)));
            enemyVel->vx = xdist * mult;
            enemyVel->vy = ydist * mult;
        }
};