#include "Player.h"

#include <Tridingine.h>

bool Player::Init()
{
    x = 200.0f;
    y = 120.0f;

    radius = 8.0f;
    speed = 180.0f;

    return true;
}

void Player::Update()
{
    double dt = dt_get();
    Vec2 cpad = input_getCPad();

    x += speed * dt * cpad.x;
    y += speed * dt * cpad.y;

    // Límites de la pantalla
    if (x < radius)
        x = radius;

    if (x > 400.0f - radius)
        x = 400.0f - radius;

    if (y < radius)
        y = radius;

    if (y > 240.0f - radius)
        y = 240.0f - radius;
}

void Player::Draw()
{
    D2D_DrawCircleSolid(
        x,
        y,
        radius,
        0.0f,
        0.2f,
        0.5f,
        0.5f,
        Color_MakeColor(60,220,255,255)
    );
}

float Player::X() const
{
    return x;
}

float Player::Y() const
{
    return y;
}

float Player::Radius() const
{
    return radius;
}