#pragma once

#include "Player.h"

class Enemy
{
public:

    bool Init();

    void RandomSpawn();

    void Update(const Player& player);

    void Draw();

    bool Collides(const Player& player) const;

    float X() const;
    float Y() const;
    float Size() const;

private:

    float x;
    float y;

    float size;
    float speed;

    float rotation;
};