#ifndef QUATERNION_H
#define QUATERNION_H

#include <maths.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef struct
{
    float w;
    float x;
    float y;
    float z;
} Quaternion;

Quaternion quat_identity(void);

Quaternion quat_from_axis_angle(float angleDeg,
                                float ax,
                                float ay,
                                float az);

Quaternion quat_from_euler(float pitch,
                           float yaw,
                           float roll);

Quaternion quat_mul(Quaternion a,
                    Quaternion b);

Quaternion quat_normalize(Quaternion q);

void quat_to_matrix(Quaternion q,
                    float m[16]);

void quat_to_euler(Quaternion q,
                   float *pitch,
                   float *yaw,
                   float *roll);

#endif