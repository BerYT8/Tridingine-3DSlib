#pragma once

#include <vector>
#include <Tridingine.h>

#include "Player.h"
#include "Enemy.h"
#include "Pickup.h"

static constexpr int MAX_ENEMIES = 25;
static constexpr int MAX_PICKUPS = 3;

class Game
{
public:

    Game();

    bool Init();
    void Update();
    void DrawTop();
    void DrawBot();
    void Exit();

    int GetScore();

private:

    void SpawnEnemy();
    void SpawnPickup();
    void Restart();

private:

    Player player;

    std::vector<Enemy> enemies;
    std::vector<Pickup> pickups;

    float enemySpawnTimer;
    float pickupSpawnTimer;

    float enemySpawnDelay;

    u32 score;
    u32 bestScore;
    int wave;

    bool gameOver;

    D2D_Font *font;
};