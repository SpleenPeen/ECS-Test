#pragma once

#include <box2d/box2d.h>
#include "Systems.hpp"
#include <unordered_map>

class Scene
{
    protected:
        EntityManager _entMan;
    public:
        Scene() = default;
        virtual void Update(const float& dt);
        virtual void Draw(sf::RenderWindow& window);
};

class SafeHouse : public Scene
{
    public:
        SafeHouse();
};