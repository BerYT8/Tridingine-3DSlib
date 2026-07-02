#include <utils/random.h>
#include "randomIntern.h"

static bool initialized = false;

void randomInit(RandomType type)
{
    if(initialized)
        return;
    initialized = true;
    randomIntern::randomInit(type);
}

void setRandomType(RandomType type)
{
    if(!initialized)
        return;
    randomIntern::setRandomType(type);
    randomInt(0, 10); // Forzar actualización de la semilla
}

unsigned int getSeed()
{
    if(!initialized)
        return 0;
    return randomIntern::getSeed();
}

void setSeed(unsigned int seed)
{
    if(!initialized)
        return;
    randomIntern::setSeed(seed);
}

void doRand(float min, float max)
{
    if(!initialized)
        return;
    switch (randomIntern::getRandomType())
    {
    case RANDOM_TIME_BASED:
        randomIntern::setRandomTimeSeed();
        break;
    case RANDOM_SEED_BASED:
        randomIntern::setRandomSeedBased(min, max);
        break;
    default:
        randomIntern::setRandomTimeSeed();
        break;
    }
}

int randomInt(int min, int max)
{
    if(!initialized)
        return 0;
    if(min > max)
    {
        int temp = min;
        min = max;
        max = temp;
    }
    int value = randomIntern::randomInt(min, max);
    doRand(min, max);
    return value;
}

float randomFloat(float min, float max)
{
    if(!initialized)
        return 0.0f;
    if(min > max)
    {
        float temp = min;
        min = max;
        max = temp;
    }
    float value = randomIntern::randomFloat(min, max);
    doRand(min, max);
    return value;
}

bool randomBool()
{
    if(!initialized)
        return false;
    return randomInt(0, 1) == 1;
}