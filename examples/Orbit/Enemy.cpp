#include "Enemy.h"
#include "include/maths/vector2.h"

#include <Tridingine.h>

#include <cmath>

bool Enemy::Init()
{
    size = 12.0f;
    speed = 1.2f;

    x = 0.0f;
    y = 0.0f;

    rotation = 0.0f;

    return true;
}

void Enemy::RandomSpawn()
{
    Init();

    switch (randomInt(0, 3))
    {
    case 0:
        x = randomFloat(0.0f, 399.0f);
        y = -size;
        break;

    case 1:
        x = 400.0f + size;
        y = randomFloat(0.0f, 239.0f);
        break;

    case 2:
        x = randomFloat(0.0f, 399.0f);
        y = 240.0f + size;
        break;

    case 3:
        x = -size;
        y = randomFloat(0.0f, 239.0f);
        break;
    }

    speed = randomFloat(60.0f, 120.0f);
}

void Enemy::Update(const Player& player)
{
    float dx = player.X() - x;
    float dy = player.Y() - y;

    float len = sqrtf(dx * dx + dy * dy);

    if (len > 0.001f)
    {
        dx /= len;
        dy /= len;

        double dt = dt_get();

        x += dx * speed * static_cast<float>(dt);
        y += dy * speed * static_cast<float>(dt);
    }

    rotation = vec2_look_at(vec2_create(x, y), vec2_create(player.X(), player.Y()));
}

void Enemy::Draw()
{
    D2D_DrawRectSolid(
        x,
        y,
        size,
        size,
        rotation,
        0.3f,
        0.5f,
        0.5f,
        Color_MakeColor(255, 80, 80, 255)
    );
}

bool Enemy::Collides(const Player& player) const
{
    float dx = player.X() - x;
    float dy = player.Y() - y;

    float distance = sqrtf(dx * dx + dy * dy);

    return distance < (player.Radius() + size * 0.5f);
}

float Enemy::X() const
{
    return x;
}

float Enemy::Y() const
{
    return y;
}

float Enemy::Size() const
{
    return size;
}