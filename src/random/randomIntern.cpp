#include "randomIntern.h"
#include <random>
#include <ctime>

#if defined(PLATFORM_3DS)
#include <3ds.h>
#endif

unsigned int randomIntern::currentSeed = 0;
std::mt19937 randomIntern::rng = std::mt19937(randomIntern::currentSeed);

RandomType randomIntern::randomType = RANDOM_TIME_BASED;

void randomIntern::setRandomType(RandomType type)
{
    randomType = type;
}

void randomIntern::randomInit(RandomType type)
{
    setRandomType(type);
    setRandomTimeSeed();
}

void randomIntern::setRandomTimeSeed()
{
#if defined(PLATFORM_3DS)
    unsigned int timeSeed = static_cast<unsigned int>(svcGetSystemTick() ^ time(nullptr));
#else
    unsigned int timeSeed = static_cast<unsigned int>(time(nullptr));
#endif

    std::seed_seq seq{
        currentSeed,
        timeSeed,
        static_cast<unsigned int>(time(nullptr))};

    std::vector<unsigned int> seeds(1);
    seq.generate(seeds.begin(), seeds.end());

    setSeed(seeds[0]);
}

void randomIntern::setRandomSeedBased(float min, float max)
{
    unsigned int newSeed = currentSeed;

    // Mezcla determinista (tipo hash)
    newSeed ^= 0x9e3779b9 + (newSeed << 6) + (newSeed >> 2) + static_cast<unsigned int>(min * 1000) + static_cast<unsigned int>(max * 1000);

    setSeed(newSeed);
}

void randomIntern::setSeed(unsigned int seed)
{
    currentSeed = seed;
    rng.seed(seed);
}

int randomIntern::randomInt(int min, int max)
{
    std::uniform_int_distribution<int> dist(min, max);
    return dist(rng);
}

float randomIntern::randomFloat(float min, float max)
{
    std::uniform_real_distribution<float> dist(min, max);
    return dist(rng);
}