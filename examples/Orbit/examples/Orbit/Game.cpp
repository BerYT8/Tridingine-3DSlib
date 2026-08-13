#include "Game.h"

#include <Tridingine.h>

#include <cmath>

static float posY = 0.0f;
static float floatTime = 0.0f;

const float amplitude = 10.0f;
const float speed = 2.2f;

static AsyncSaveData data;

Game::Game()
{
    enemySpawnTimer = 0.0f;
    pickupSpawnTimer = 0.0f;

    enemySpawnDelay = 1.5f;

    score = 0;
    bestScore = score;
    wave = 1;

    gameOver = false;
}

bool Game::Init()
{
    randomInit(RANDOM_TIME_BASED);

    player.Init();

    enemies.clear();
    pickups.clear();

    SpawnEnemy();
    SpawnPickup();

    font = D2D_OpenFont("engine/fonts/PopHappinessStd-EB");
    if(!font)
        printf("Not opened font.\n");

    bool l = loadSimple("gameData.sav", data);
    if(l)
    {
        bestScore = data.getUint32Value("bestScore", 0);
    }

    return true;
}

void Game::Exit()
{
    enemies.clear();
    pickups.clear();

    D2D_CloseFont(font);

    bool s = saveSimple("gameData.sav", data);
}

void Game::Restart()
{
    enemies.clear();
    pickups.clear();

    player.Init();

    enemySpawnDelay = 1.5f;

    enemySpawnTimer = 0.0f;
    pickupSpawnTimer = 0.0f;

    score = 0;
    wave = 1;

    gameOver = false;

    SpawnEnemy();
    SpawnPickup();
}

void Game::Update()
{
    //--------------------------------------------------
    // Reiniciar
    //--------------------------------------------------

    if (gameOver)
    {
        if (input_isKeyPressed(INPUT_KEY_START))
            Restart();

        return;
    }

    //--------------------------------------------------
    // Jugador
    //--------------------------------------------------

    player.Update();

    //--------------------------------------------------
    // Temporizadores
    //--------------------------------------------------

    double dt = dt_get();

    enemySpawnTimer += dt;
    pickupSpawnTimer += dt;

    if (enemySpawnTimer >= enemySpawnDelay)
    {
        enemySpawnTimer = 0.0f;

        if (enemies.size() < MAX_ENEMIES)
            SpawnEnemy();
    }

    if (pickupSpawnTimer >= 8.0f)
    {
        pickupSpawnTimer = 0.0f;

        if (pickups.size() < MAX_PICKUPS)
            SpawnPickup();
    }

    //--------------------------------------------------
    // Enemigos
    //--------------------------------------------------

    for (Enemy& enemy : enemies)
    {
        enemy.Update(player);

        if (enemy.Collides(player))
        {
            gameOver = true;
        }
    }

    //--------------------------------------------------
    // Pickups
    //--------------------------------------------------

    for (size_t i = 0; i < pickups.size();)
    {
        pickups[i].Update();

        if (pickups[i].Collides(player))
        {
            pickups.erase(pickups.begin() + i);

            score++;

            if(bestScore < score)
                bestScore = score;

            if(data.getUint32Value("bestScore", 0) < bestScore)
                data.addValue("bestScore", bestScore);

            enemies.clear();

            if ((score % 5) == 0)
            {
                wave++;

                if (enemySpawnDelay > 0.8f)
                    enemySpawnDelay -= 0.1f;

                if (enemies.size() < MAX_ENEMIES)
                    SpawnEnemy();
            }

            continue;
        }

        ++i;
    }
}

void Game::DrawTop()
{
    //------------------------------------
    // Fondo
    //------------------------------------

    D2D_DrawRectSolid(
        0,
        0,
        400,
        240,
        0,
        -1.0f,
        0,
        0,
        Color_MakeColor(20,20,30,255)
    );

    //------------------------------------
    // Pickups
    //------------------------------------

    for (Pickup& p : pickups)
        p.Draw();

    //------------------------------------
    // Enemigos
    //------------------------------------

    for (Enemy& e : enemies)
        e.Draw();

    //------------------------------------
    // Jugador
    //------------------------------------

    player.Draw();

    //------------------------------------
    // Game Over
    //------------------------------------
    Vec2 sSize = S2S_GetScreenSize(TOP);

    if (gameOver)
    {
        D2D_DrawRectSolid(
            0,
            0,
            400,
            240,
            0,
            0.7f,
            0,
            0,
            Color_MakeColor(180,0,0,150)
        );

        D2D_DrawText((std::string("Your score: ") + std::to_string(score)).c_str(), 
                        font, 30, Color_White, 
                        0, 0, 0.8f, 
                        sSize.x, sSize.y, 
                        0, 0, 
                        0.5f, 0.5f,
                        0, 0, WORD_WRAP_MODE);
    }
    else
    {
        D2D_DrawText((std::string("Score: ") + std::to_string(score)).c_str(), 
                        font, 30, Color_Red, 
                        0, 0, 0.8f, 
                        sSize.x, sSize.y, 
                        0, 0, 
                        0, 0,
                        0, 0, WORD_WRAP_MODE);
    }

    if(S2S_IsGamePaused())
    {
        D2D_DrawRectSolid(0, 0, sSize.x, sSize.y, 0, 1.0f, 0, 0, Color_MakeColor(0,0,0,230));

        D2D_DrawText(std::string("GAME PAUSED").c_str(), 
                        font, 40, Color_Red, 
                        0, posY, 1.0f, 
                        sSize.x, sSize.y, 
                        0, 0, 
                        0.5f, 0.5f,
                        0, 0, WORD_WRAP_MODE);

        floatTime += (float)dt_get();
        posY = std::sin(floatTime * speed) * amplitude;
    }
}

void Game::DrawBot()
{
    Vec2 sSize = S2S_GetScreenSize(BOTTOM);
    D2D_DrawText((std::string("Best score: ") + std::to_string(bestScore)).c_str(), 
                    font, 20, Color_Green, 
                    0, 0, 0.8f, 
                    sSize.x, sSize.y, 
                    0, 0, 
                    0, 0,
                    0, 0, WORD_WRAP_MODE);
    if(gameOver)
    {
        D2D_DrawText(std::string("Pulse START to restart the game.").c_str(), 
                        font, 25, Color_White, 
                        0, 0, 0.8f, 
                        sSize.x, sSize.y, 
                        0, 0, 
                        0.5f, 0.5f,
                        0, 0, WORD_WRAP_MODE);
    }

    if(S2S_IsGamePaused())
    {
        D2D_DrawRectSolid(0, 0, sSize.x, sSize.y, 0, 1.0f, 0, 0, Color_MakeColor(0,0,0,230));

        D2D_DrawText(std::string("Pulse SELECT to replay the game.").c_str(), 
                        font, 30, Color_White, 
                        0, 0, 1.0f, 
                        sSize.x, sSize.y, 
                        0, 0, 
                        0.5f, 0.5f,
                        0, 0, WORD_WRAP_MODE);
    }
}

void Game::SpawnEnemy()
{
    if (enemies.size() >= MAX_ENEMIES)
        return;

    Enemy enemy;

    enemy.RandomSpawn();

    enemies.push_back(enemy);
}

void Game::SpawnPickup()
{
    if (pickups.size() >= MAX_PICKUPS)
        return;

    Pickup pickup;

    pickup.RandomSpawn();

    pickups.push_back(pickup);
}

int Game::GetScore()
{
    return score;
}