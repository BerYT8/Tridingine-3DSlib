#include <draw/3d/quaternion.h>

Quaternion quat_identity()
{
    Quaternion q = {1,0,0,0};
    return q;
}

Quaternion quat_normalize(Quaternion q)
{
    float l = sqrtf(q.w*q.w +
                    q.x*q.x +
                    q.y*q.y +
                    q.z*q.z);

    q.w/=l;
    q.x/=l;
    q.y/=l;
    q.z/=l;

    return q;
}

Quaternion quat_from_axis_angle(float angle,
                                float ax,
                                float ay,
                                float az)
{
    float r = angle * (float)M_PI / 180.0f;

    float s = sinf(r*0.5f);

    Quaternion q;

    q.w = cosf(r*0.5f);
    q.x = ax*s;
    q.y = ay*s;
    q.z = az*s;

    return quat_normalize(q);
}

Quaternion quat_mul(Quaternion a,
                    Quaternion b)
{
    Quaternion q;

    q.w = a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z;

    q.x = a.w*b.x + a.x*b.w + a.y*b.z - a.z*b.y;

    q.y = a.w*b.y - a.x*b.z + a.y*b.w + a.z*b.x;

    q.z = a.w*b.z + a.x*b.y - a.y*b.x + a.z*b.w;

    return quat_normalize(q);
}

Quaternion quat_from_euler(float pitch,
                           float yaw,
                           float roll)
{
    float p = pitch * M_PI / 180.0f * 0.5f;
    float y = yaw   * M_PI / 180.0f * 0.5f;
    float r = roll  * M_PI / 180.0f * 0.5f;

    float cp = cosf(p);
    float sp = sinf(p);

    float cy = cosf(y);
    float sy = sinf(y);

    float cr = cosf(r);
    float sr = sinf(r);

    Quaternion q;

    q.w = cr*cp*cy + sr*sp*sy;
    q.x = sr*cp*cy - cr*sp*sy;
    q.y = cr*sp*cy + sr*cp*sy;
    q.z = cr*cp*sy - sr*sp*cy;

    return quat_normalize(q);
}

void quat_to_matrix(Quaternion q,
                    float m[16])
{
    q = quat_normalize(q);

    float xx=q.x*q.x;
    float yy=q.y*q.y;
    float zz=q.z*q.z;

    float xy=q.x*q.y;
    float xz=q.x*q.z;
    float yz=q.y*q.z;

    float wx=q.w*q.x;
    float wy=q.w*q.y;
    float wz=q.w*q.z;

    m[0]=1-2*(yy+zz);
    m[1]=2*(xy+wz);
    m[2]=2*(xz-wy);
    m[3]=0;

    m[4]=2*(xy-wz);
    m[5]=1-2*(xx+zz);
    m[6]=2*(yz+wx);
    m[7]=0;

    m[8]=2*(xz+wy);
    m[9]=2*(yz-wx);
    m[10]=1-2*(xx+yy);
    m[11]=0;

    m[12]=0;
    m[13]=0;
    m[14]=0;
    m[15]=1;
}