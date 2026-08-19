#include "Pickup.h"

#include <Tridingine.h>

#include <cstdlib>
#include <cmath>

bool Pickup::Init()
{
    radius = 6.0f;

    pulse = 0.0f;

    x = 0.0f;
    y = 0.0f;

    return true;
}

void Pickup::RandomSpawn()
{
    Init();

    x = randomFloat(20.0f, 380.0f);
    y = randomFloat(20.0f, 220.0f);
}

void Pickup::Update()
{
    pulse += 0.12f;

    if (pulse > 6.283185f)
        pulse = 0.0f;
}

void Pickup::Draw()
{
    float r = radius + sinf(pulse) * 1.5f;
    
    D2D_DrawCircleSolid(
        x,
        y,
        r,
        0.0f,
        0.09f,
        0.5f,
        0.5f,
        Color_MakeColor(50,255,120,255));

    D2D_DrawCircleSolid(
        x,
        y,
        r + 3.0f,
        0.0f,
        0.1f,
        0.5f,
        0.5f,
        Color_MakeColor(50,255,120,60)
    );
}

bool Pickup::Collides(const Player& player) const
{
    float dx = player.X() - x;
    float dy = player.Y() - y;

    float dist = sqrtf(dx * dx + dy * dy);

    return dist < (player.Radius() + radius);
}

float Pickup::X() const
{
    return x;
}

float Pickup::Y() const
{
    return y;
}

float Pickup::Radius() const
{
    return radius;
}