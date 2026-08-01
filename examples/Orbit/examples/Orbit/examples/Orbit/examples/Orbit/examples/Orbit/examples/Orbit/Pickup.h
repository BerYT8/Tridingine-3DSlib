#pragma once

#include "Player.h"

class Pickup
{
public:

    bool Init();

    void RandomSpawn();

    void Update();

    void Draw();

    bool Collides(const Player& player) const;

    float X() const;
    float Y() const;
    float Radius() const;

private:

    float x;
    float y;

    float radius;

    float pulse;
};