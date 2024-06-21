#pragma once
#include <math.h>
#include <float.h>

#ifdef VEC_T_IS_FLOAT
typedef float vec_t;
typedef float vec3_t[3];
typedef float vec2_t[2];
typedef struct {
    float m[3][4];
} matrix3x4_t;

#else

typedef double vec_t;
typedef double vec3_t[3];
typedef double vec2_t[2];
typedef struct {
    double m[3][4];
} matrix3x4_t;

#endif

extern const vec3_t    vec3_origin;

#define NORMAL_EPSILON   0.00001
#define ON_EPSILON       0.01
#define EQUAL_EPSILON    0.001
#define DIST_EPSILON   0.01

#define DotProduct(x,y) ( (x)[0] * (y)[0] + (x)[1] * (y)[1]  +  (x)[2] * (y)[2])
#define CrossProduct(a, b, dest) \
{ \
    (dest)[0] = (a)[1] * (b)[2] - (a)[2] * (b)[1]; \
    (dest)[1] = (a)[2] * (b)[0] - (a)[0] * (b)[2]; \
    (dest)[2] = (a)[0] * (b)[1] - (a)[1] * (b)[0]; \
}
#define VectorCopy(a,b) { (b)[0]=(a)[0]; (b)[1]=(a)[1]; (b)[2]=(a)[2]; }
#define VectorClear(a)  { (a)[0] = (a)[1] = (a)[2] = 0.0; }
#define VectorFill(a,b)          { (a)[0]=(b); (a)[1]=(b); (a)[2]=(b);}

#define VectorSubtract(a,b,c)    { (c)[0]=(a)[0]-(b)[0]; (c)[1]=(a)[1]-(b)[1]; (c)[2]=(a)[2]-(b)[2]; }
#define VectorAdd(a,b,c)         { (c)[0]=(a)[0]+(b)[0]; (c)[1]=(a)[1]+(b)[1]; (c)[2]=(a)[2]+(b)[2]; }
#define VectorMultiply(a,b,c)    { (c)[0]=(a)[0]*(b)[0]; (c)[1]=(a)[1]*(b)[1]; (c)[2]=(a)[2]*(b)[2]; }
#define VectorDivide(a,b,c)      { (c)[0]=(a)[0]/(b)[0]; (c)[1]=(a)[1]/(b)[1]; (c)[2]=(a)[2]/(b)[2]; }
#define VectorLength(a)  sqrt((double) ((double)((a)[0] * (a)[0]) + (double)( (a)[1] * (a)[1]) + (double)( (a)[2] * (a)[2])) )
#define VectorAvg(a)             ( ( (a)[0] + (a)[1] + (a)[2] ) / 3 )
#define VectorMaximum(a) ( max( (a)[0], max( (a)[1], (a)[2] ) ) )
#define VectorMinimum(a) ( min( (a)[0], min( (a)[1], (a)[2] ) ) )

#define VectorMA(a, scale, b, dest) \
{ \
    (dest)[0] = (a)[0] + scale * (b)[0]; \
    (dest)[1] = (a)[1] + scale * (b)[1]; \
    (dest)[2] = (a)[2] + scale * (b)[2]; \
}

#define VectorScale(a,b,c)       { (c)[0]=(a)[0]*(b);(c)[1]=(a)[1]*(b);(c)[2]=(a)[2]*(b); }

inline bool isPointFinite(const vec_t* p)
{
    if (_finite(p[0]) && _finite(p[1]) && _finite(p[2]))
    {
        return true;
    }
    return false;
}

inline bool     VectorCompare(const vec3_t v1, const vec3_t v2)
{
    int             i;

    for (i = 0; i < 3; i++)
    {
        if (fabs(v1[i] - v2[i]) > EQUAL_EPSILON)
        {
            return false;
        }
    }
    return true;
}

inline vec_t   VectorNormalize(vec3_t v)
{
    double          length;

    length = DotProduct(v, v);
    length = sqrt(length);
    if (length < NORMAL_EPSILON)
    {
        VectorClear(v);
        return 0.0;
    }

    v[0] /= length;
    v[1] /= length;
    v[2] /= length;

    return length;
}

extern void InvalidateBounds(vec3_t& mins, vec3_t& maxs);

extern void AddPointToBounds(const vec3_t v, vec3_t mins, vec3_t maxs);

// Function to rotate a vector using a 3x4 matrix
inline void VectorRotate(const double in[3], const matrix3x4_t* matrix, double out[3]) {
    out[0] = in[0] * matrix->m[0][0] + in[1] * matrix->m[0][1] + in[2] * matrix->m[0][2];
    out[1] = in[0] * matrix->m[1][0] + in[1] * matrix->m[1][1] + in[2] * matrix->m[1][2];
    out[2] = in[0] * matrix->m[2][0] + in[1] * matrix->m[2][1] + in[2] * matrix->m[2][2];
}