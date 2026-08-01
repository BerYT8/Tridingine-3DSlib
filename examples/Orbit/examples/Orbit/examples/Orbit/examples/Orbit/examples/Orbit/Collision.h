#pragma once

class Player;
class Enemy;
class Pickup;

namespace Collision
{
    bool CircleCircle(
        float x1,
        float y1,
        float r1,
        float x2,
        float y2,
        float r2
    );

    bool PlayerEnemy(
        const Player& player,
        const Enemy& enemy
    );

    bool PlayerPickup(
        const Player& player,
        const Pickup& pickup
    );
}