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

    if (input_isKeyDown(INPUT_KEY_CPAD_LEFT))
        x -= speed * dt;

    if (input_isKeyDown(INPUT_KEY_CPAD_RIGHT))
        x += speed * dt;

    if (input_isKeyDown(INPUT_KEY_CPAD_UP))
        y -= speed * dt;

    if (input_isKeyDown(INPUT_KEY_CPAD_DOWN))
        y += speed * dt;

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