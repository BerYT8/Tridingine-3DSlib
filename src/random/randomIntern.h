#pragma once
#include <random>
#include <utils/random.h>

class randomIntern
{
private:
    static unsigned int currentSeed;
    static std::mt19937 rng;
    static RandomType randomType;

public:
    randomIntern(/* args */) = default;
    ~randomIntern() = default;

    static void setRandomType(RandomType type);

    static unsigned int getSeed() { return currentSeed; }
    static RandomType getRandomType() { return randomType; }

    static void randomInit(RandomType type = RANDOM_TIME_BASED);
    static void setRandomTimeSeed();
    static void setRandomSeedBased(float min = 0.0f, float max = 1.0f);
    static void setSeed(unsigned int seed);
    static int randomInt(int min, int max);
    static float randomFloat(float min, float max);
};
