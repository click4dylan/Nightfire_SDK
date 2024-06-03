
#include "vectors.h"
#include "globals.h"
const vec3_t    vec3_origin = { 0, 0, 0 };

void InvalidateBounds(vec3_t& mins, vec3_t& maxs)
{
    mins[0] = MAX_WORLD_BOUNDS;
    mins[1] = MAX_WORLD_BOUNDS;
    mins[2] = MAX_WORLD_BOUNDS;
    maxs[0] = -MAX_WORLD_BOUNDS;
    maxs[1] = -MAX_WORLD_BOUNDS;
    maxs[2] = -MAX_WORLD_BOUNDS;
}

void AddPointToBounds(const vec3_t v, vec3_t mins, vec3_t maxs)
{
    int             i;
    vec_t           val;

    for (i = 0; i < 3; i++)
    {
        val = v[i];
        if (val < mins[i])
        {
            mins[i] = val;
        }
        if (val > maxs[i])
        {
            maxs[i] = val;
        }
    }
}