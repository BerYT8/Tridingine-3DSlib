#ifndef RANDOM_H
#define RANDOM_H

#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

/* Tipo de random */
typedef enum
{
    RANDOM_TIME_BASED,
    RANDOM_SEED_BASED
} RandomType;

#ifdef __cplusplus
extern "C"
{
#endif

void randomInit(RandomType type);
void setRandomType(RandomType type);
unsigned int getSeed();
void setSeed(unsigned int seed);
int randomInt(int min, int max);
float randomFloat(float min, float max);
bool randomBool();

#ifdef __cplusplus
}
#endif

#endif