#include "Collision.h"

#include "Player.h"
#include "Enemy.h"
#include "Pickup.h"

#include <cmath>

namespace Collision
{

bool CircleCircle(
    float x1,
    float y1,
    float r1,
    float x2,
    float y2,
    float r2
)
{
    float dx = x2 - x1;
    float dy = y2 - y1;

    float rr = r1 + r2;

    return (dx * dx + dy * dy) <= rr * rr;
}

bool PlayerEnemy(
    const Player& player,
    const Enemy& enemy
)
{
    return CircleCircle(
        player.X(),
        player.Y(),
        player.Radius(),
        enemy.X(),
        enemy.Y(),
        enemy.Size() * 0.5f
    );
}

bool PlayerPickup(
    const Player& player,
    const Pickup& pickup
)
{
    return CircleCircle(
        player.X(),
        player.Y(),
        player.Radius(),
        pickup.X(),
        pickup.Y(),
        pickup.Radius()
    );
}

}