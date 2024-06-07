
#include <Windows.h>
#include <stdlib.h>
#include "globals.h"
#include "helpers.h"
#include "bsp_structs.h"
#include "winding.h"

//#undef BOGUS_RANGE
//#undef ON_EPSILON
//#define ON_EPSILON 0.01

//replaced with C++
#if 0
DWORD windingconstructoradr = 0x412850;
Winding* CallWindingConstructor(Winding* winding)
{
    Winding* ret;
    DWORD w = (DWORD)winding;
    __asm
    {
        mov eax, w
        call windingconstructoradr
        mov ret, eax
    }
    return ret;
}

void WindingConstructor(Winding* winding, unsigned int numpoints)
{
    *(DWORD*)winding = 0x4237F8; //vtable
    ++g_numWindings;
    winding->m_NumPoints = numpoints;
    winding->m_MaxPoints = (winding->m_NumPoints + 3) & ~3;   // groups of 4

    winding->m_Points = new vec3_t[winding->m_MaxPoints];
    memset(winding->m_Points, 0, sizeof(vec3_t) * winding->m_NumPoints);
}

Winding* AllocWinding(unsigned int numpoints)
{
    void* mem = malloc(0x10);
    if (mem)
        WindingConstructor((Winding*)mem, numpoints);
    return (Winding*)mem;
}
#endif

#define MAX_POINTS_ON_WINDING 128
#define	SIDE_FRONT		0
#define	SIDE_ON			2
#define	SIDE_BACK		1
#define	SIDE_CROSS		-2

void Winding::RemoveColinearPoints()
{
    unsigned int    i;
    unsigned int    nump;
    int             j, k;
    vec3_t          v1, v2, v3;
    vec3_t          p[128];

    // TODO: OPTIMIZE:  this could be 1/2 the number of vectornormalize calls by caching one of the previous values through the loop
    // TODO: OPTIMIZE: Remove the modulo operations inside the loop and replace with compares
    nump = 0;
    for (i = 0; i < m_NumPoints; i++)
    {
        j = (i + 1) % m_NumPoints;                  // i + 1
        k = (i + m_NumPoints - 1) % m_NumPoints;    // i - 1 
        VectorSubtract(m_Points[i], m_Points[j], v1);
        VectorSubtract(m_Points[i], m_Points[k], v2);
        VectorNormalize(v1);
        VectorNormalize(v2);
        VectorAdd(v1, v2, v3);
        if (!VectorCompare(v3, vec3_origin))
        {
            VectorCopy(m_Points[i], p[nump]);
            nump++;
        }
#if 0
        else
        {
            Log("v3 was (%4.3f %4.3f %4.3f)\n", v3[0], v3[1], v3[2]);
        }
#endif
    }

    if (nump == m_NumPoints)
    {
        return;
    }

#if 0
    Warning("RemoveColinearPoints: Removed %u points, from %u to %u\n", m_NumPoints - nump, m_NumPoints, nump);
    Warning("Before :\n");
    Print();
#endif
    m_NumPoints = nump;
    memcpy(m_Points, p, nump * sizeof(vec3_t));

#if 0
    Warning("After :\n");
    Print();

    Warning("==========\n");
#endif
}

Winding::~Winding()
{
    --g_numWindings;
    delete[] m_Points;
}

Winding::Winding(const Winding& other)
{
    ++g_numWindings;
    m_NumPoints = other.m_NumPoints;
    m_MaxPoints = max(other.m_MaxPoints, 8);//(m_NumPoints + 3) & ~3;   // groups of 4

    m_Points = new vec3_t[m_MaxPoints];
    if (m_NumPoints)
        memcpy(m_Points, other.m_Points, sizeof(vec3_t) * m_NumPoints);
}

Winding::Winding()
{
    ++g_numWindings;
    m_NumPoints = 0;
    m_MaxPoints = 8;
    m_Points = new vec3_t[m_MaxPoints];
    memset(m_Points, 0, sizeof(vec3_t) * 8);
}

Winding::Winding(const struct dface_s& face)
{
    ++g_numWindings;

    m_NumPoints = face.num_vertices;
    m_MaxPoints = (m_NumPoints + 7) & 0xFFFFFFF8; //(m_NumPoints + 3) & ~3;	// groups of 4

    m_MaxPoints = max(m_MaxPoints, 8);

    m_Points = new vec3_t[m_MaxPoints];

    const dvertex_t* dv = &g_dverts[face.first_vertex_index];
    for (unsigned int i = 0; i < face.num_vertices; ++i)
    {
        VectorCopy(dv->point, m_Points[i]);
        ++dv;
    }
    RemoveColinearPoints();
}

Winding::Winding(const struct plane_s& plane)
{
    ++g_numWindings;
    vec3_t normal;
    vec_t dist;

    VectorCopy(plane.normal, normal);
    dist = plane.dist;
    initFromPlane(normal, dist);
}

Winding::Winding(UINT32 numpoints)
{
    ++g_numWindings;

    if (numpoints >= 3)
    {
        if (numpoints >= 8)
        {
            m_NumPoints = numpoints;
            m_MaxPoints = (numpoints + 7) & 0xFFFFFFF8;
        }
    }
    else
    {
        m_MaxPoints = 8;
    }

    m_Points = new vec3_t[m_MaxPoints];
    memset(m_Points, 0, sizeof(vec3_t) * m_MaxPoints);
}

void Winding::getCenter(vec3_t& center) const
{
    unsigned int    i;
    vec_t           scale;

    if (m_NumPoints > 0)
    {
        VectorCopy(vec3_origin, center);
        for (i = 0; i < m_NumPoints; i++)
        {
            VectorAdd(m_Points[i], center, center);
        }

        scale = 1.0 / m_NumPoints;
        VectorScale(center, scale, center);
    }
    else
    {
        VectorClear(center);
    }
}

vec_t Winding::getArea() const
{
    unsigned int    i;
    vec3_t          d1, d2, cross;
    vec_t           total;

    //hlassert(m_NumPoints >= 3);

    total = 0.0;
    if (m_NumPoints >= 3)
    {
        for (i = 2; i < m_NumPoints; i++)
        {
            VectorSubtract(m_Points[i - 1], m_Points[0], d1);
            VectorSubtract(m_Points[i], m_Points[0], d2);
            CrossProduct(d1, d2, cross);
            total += 0.5 * VectorLength(cross);
        }
    }
    return total;
}

void Winding::initFromPlane(const vec3_t normal, const vec_t dist)
{
#if 0
    int axis = -1;
    double max = -BOGUS_RANGE;
    vec3_t up = { 0.0, 0.0, 0.0 };

    // Find the major axis
    for (int i = 0; i < 3; ++i) {
        double absNormal = fabs(normal[i]);
        if (absNormal > max) {
            max = absNormal;
            axis = i;
        }
    }

    if (axis == -1) {
        Error("Winding::initFromPlane no major axis found\n");
    }

    // Set the up vector
    if (axis == 0 || axis == 1) {
        up[2] = 1.0;
    }
    else {
        up[0] = 1.0;
    }

    // Project the up vector onto the plane
    double dot = up[0] * normal[0] + up[1] * normal[1] + up[2] * normal[2];
    up[0] -= dot * normal[0];
    up[1] -= dot * normal[1];
    up[2] -= dot * normal[2];

    VectorNormalize(up);

    // Set the initial points for the winding
    vec3_t origin = {
        normal[0] * dist,
        normal[1] * dist,
        normal[2] * dist
    };

    vec3_t right = {
        up[1] * normal[2] - up[2] * normal[1],
        up[2] * normal[0] - up[0] * normal[2],
        up[0] * normal[1] - up[1] * normal[0]
    };

    VectorScale(up, 32768.0, up);
    VectorScale(right, 32768.0, right);

    m_NumPoints = 4;
    m_MaxPoints = 8;
    m_Points = new vec3_t[m_MaxPoints];

    // Point 0
    m_Points[0][0] = origin[0] - right[0];
    m_Points[0][1] = origin[1] - right[1];
    m_Points[0][2] = origin[2] - right[2];

    m_Points[0][0] += up[0];
    m_Points[0][1] += up[1];
    m_Points[0][2] += up[2];

    // Point 1
    m_Points[1][0] = origin[0] + right[0];
    m_Points[1][1] = origin[1] + right[1];
    m_Points[1][2] = origin[2] + right[2];

    m_Points[1][0] += up[0];
    m_Points[1][1] += up[1];
    m_Points[1][2] += up[2];

    // Point 2
    m_Points[2][0] = origin[0] + right[0];
    m_Points[2][1] = origin[1] + right[1];
    m_Points[2][2] = origin[2] + right[2];

    m_Points[2][0] -= up[0];
    m_Points[2][1] -= up[1];
    m_Points[2][2] -= up[2];

    // Point 3
    m_Points[3][0] = origin[0] - right[0];
    m_Points[3][1] = origin[1] - right[1];
    m_Points[3][2] = origin[2] - right[2];

    m_Points[3][0] -= up[0];
    m_Points[3][1] -= up[1];
    m_Points[3][2] -= up[2];
#else
    int             i;
    vec_t           max, v;
    vec3_t          org, vright, vup;

    // find the major axis               

    max = -BOGUS_RANGE;
    int x = -1;
    for (i = 0; i < 3; i++)
    {
        v = fabs(normal[i]);
        if (v > max)
        {
            max = v;
            x = i;
        }
    }
    if (x == -1)
    {
        Error("Winding::initFromPlane no major axis found\n");
    }

    VectorCopy(vec3_origin, vup);
    switch (x)
    {
    case 0:
    case 1:
        vup[2] = 1;
        break;
    case 2:
        vup[0] = 1;
        break;
    }

    v = DotProduct(vup, normal);
    VectorMA(vup, -v, normal, vup);
    VectorNormalize(vup);

    VectorScale(normal, dist, org);

    CrossProduct(vup, normal, vright);

    VectorScale(vup, BOGUS_RANGE, vup);
    VectorScale(vright, BOGUS_RANGE, vright);

    // project a really big     axis aligned box onto the plane
    m_NumPoints = 4;
    m_MaxPoints = 8;
    m_Points = new vec3_t[m_MaxPoints];

    VectorSubtract(org, vright, m_Points[0]);
    VectorAdd(m_Points[0], vup, m_Points[0]);

    VectorAdd(org, vright, m_Points[1]);
    VectorAdd(m_Points[1], vup, m_Points[1]);

    VectorAdd(org, vright, m_Points[2]);
    VectorSubtract(m_Points[2], vup, m_Points[2]);

    VectorSubtract(org, vright, m_Points[3]);
    VectorSubtract(m_Points[3], vup, m_Points[3]);
#endif
}

void Winding::shiftPoints(unsigned int offset, unsigned int start_index)
{
#if 1
    if (offset == 0 || start_index >= m_NumPoints)
        return; // No shift needed

    // Adjust offset if it exceeds the number of points
    offset %= m_NumPoints;

    // Calculate the new number of points after shifting
    unsigned int new_num_points = m_NumPoints - offset;

    // Shift points in the winding array
    if (new_num_points > 0)
    {
        // Move points to their new positions
        memmove(&m_Points[start_index], &m_Points[(start_index + offset) % m_NumPoints], sizeof(vec3_t) * new_num_points);

        // Update the number of points
        m_NumPoints = new_num_points;
    }
    else
    {
        // If all points are shifted out, set the number of points to zero
        m_NumPoints = 0;
    }

    //fixme
    //unsigned int numPoints = m_NumPoints;
    //if (numPoints == 0 || startIndex == 0)
    //    return;
    //
    //startIndex = startIndex % numPoints;
    //std::rotate(m_Points, m_Points + shiftAmount, m_Points + numPoints);
    //std::rotate(m_Points + numPoints - startIndex, m_Points + numPoints, m_Points + numPoints);
#else
    unsigned int adjustedStartIndex = startIndex % m_NumPoints; // Adjust start index to be within range
    unsigned int adjustedShiftAmount = shiftAmount % m_NumPoints; // Ensure shift amount doesn't exceed winding size

    if (adjustedStartIndex + adjustedShiftAmount > m_NumPoints)
    {
        // Adjust start index and shift amount
        adjustedShiftAmount = m_NumPoints - adjustedStartIndex;
    }

    // Shift points by copying them
    if (adjustedShiftAmount > 0)
    {
        unsigned int remainingPoints = m_NumPoints - adjustedStartIndex - adjustedShiftAmount;
        if (remainingPoints > 0)
        {
            memmove(m_Points[startIndex], m_Points[startIndex + adjustedShiftAmount], 24 * remainingPoints);
        }
    }

    // Update the number of points
    m_NumPoints -= adjustedShiftAmount;
#endif
}

void Winding::Invert()
{
    // Calculate the halfway point
    unsigned int halfNumPoints = m_NumPoints / 2;

    // Loop through each point pair up to the halfway point
    for (unsigned int i = 0; i < halfNumPoints; ++i) {
        unsigned int oppositeIndex = m_NumPoints - i - 1;

        // Swap points
        std::swap(m_Points[i], m_Points[oppositeIndex]);
    }
}

void Winding::getPlane(vec3_t& normal, vec_t& dist) const
{
    vec3_t          v1, v2;

    //hlassert(m_NumPoints >= 3);

    if (m_NumPoints >= 3)
    {
        VectorSubtract(m_Points[1], m_Points[0], v1);
        VectorSubtract(m_Points[2], m_Points[0], v2);
        CrossProduct(v2, v1, normal);
        VectorNormalize(normal);
        dist = DotProduct(m_Points[0], normal);
    }
    else
    {
        VectorClear(normal);
        dist = 0.0;
    }
}

void Winding::resize(UINT32 newsize)
{
    newsize = (newsize + 7) & 0xFFFFFFF8;

    vec3_t* newpoints = new vec3_t[newsize];
    m_NumPoints = min(newsize, m_NumPoints);

    if (m_NumPoints)
       memcpy(newpoints, m_Points, 4 * ((sizeof(vec3_t) * m_NumPoints) >> 2));
   
    delete[] m_Points;
    m_MaxPoints = newsize;
    m_Points = newpoints;
}

void Winding::addPoint(const vec3_t newpoint)
{
    if (m_NumPoints >= m_MaxPoints)
    {
        resize(m_NumPoints + 1);
    }
    VectorCopy(newpoint, m_Points[m_NumPoints]);
    m_NumPoints++;
}

void Winding::insertPoint(const vec3_t newpoint, const unsigned int offset)
{
    if (offset >= m_NumPoints)
    {
        addPoint(newpoint);
    }
    else
    {
        if (m_NumPoints >= m_MaxPoints)
        {
            resize(m_NumPoints + 1);
        }

        unsigned x;
        for (x = m_NumPoints; x > offset; x--)
        {
            VectorCopy(m_Points[x - 1], m_Points[x]);
            //fixme
            if (x - 1 > x)
                break;
        }
        VectorCopy(newpoint, m_Points[x]);

        m_NumPoints++;
    }
}

bool Winding::Divide(const vec3_t normal, const vec_t dist, Winding** front, Winding** back)
{
    return Clip(true, normal, dist, front, back);
}

bool Winding::Divide(const struct plane_s& split, Winding** front, Winding** back)
{
    return Clip(true, split.normal, split.dist, front, back);
}

bool Winding::Clip(const struct plane_s& plane, Winding** front, Winding** back)
{
    return Clip(false, plane.normal, plane.dist, front, back);
}

bool Winding::Clip(const vec3_t normal, const vec_t dist, Winding** front, Winding** back)
{
    return Clip(false, normal, dist, front, back);
}

bool Winding::Clip(const bool divide, const vec3_t normal, const vec_t dist, Winding** front, Winding** back)
{
    vec_t           dists[MAX_POINTS_ON_WINDING + 4];
    int             sides[MAX_POINTS_ON_WINDING + 4];
    int             counts[3];
    vec_t           dot;
    unsigned int    i, j;
    unsigned int    maxpts;

    counts[0] = counts[1] = counts[2] = 0;

    // determine sides for each point
    for (i = 0; i < m_NumPoints; i++)
    {
        dot = DotProduct(m_Points[i], normal);
        dot -= dist;
        dists[i] = dot;
        if (dot > ON_EPSILON)
        {
            sides[i] = SIDE_FRONT;
        }
        else if (dot < -ON_EPSILON)
        {
            sides[i] = SIDE_BACK;
        }
        else
        {
            sides[i] = SIDE_ON;
        }
        counts[sides[i]]++;
    }
    sides[i] = sides[0];
    dists[i] = dists[0];

#if 1
    // nightfire behavior
    if (!counts[0] && !counts[1])
    {
        *front = nullptr;
        *back = nullptr;
        return false;
    }
    if (!counts[0])
    {
        if (divide)
        {
            *front = nullptr;
            *back = this;
        }
        else
        {
            *front = nullptr;
            *back = new Winding(*this);
        }
        return false;
    }
    if (!counts[1])
    {
        if (divide)
        {
            *front = this;
            *back = nullptr;
        }
        else
        {
            *front = new Winding(*this);
            *back = nullptr;
        }
        return false;
    }
#else
    // half life behavior
    if (!counts[0])
    {
        *front = NULL;
        *back = new Winding(*this);
        return false;
    }
    if (!counts[1])
    {
        *front = new Winding(*this);
        *back = NULL;
        return false;
    }
#endif

    maxpts = m_NumPoints + 8;                            // can't use counts[0]+2 because
    // of fp grouping errors

    Winding* f = new Winding(maxpts);
    Winding* b = new Winding(maxpts);

    *front = f;
    *back = b;

    f->m_NumPoints = 0;
    b->m_NumPoints = 0;

    for (i = 0; i < m_NumPoints; i++)
    {
        vec_t* p1 = m_Points[i];

        if (sides[i] == SIDE_ON)
        {
            VectorCopy(p1, f->m_Points[f->m_NumPoints]);
            VectorCopy(p1, b->m_Points[b->m_NumPoints]);
            f->m_NumPoints++;
            b->m_NumPoints++;
            continue;
        }
        else if (sides[i] == SIDE_FRONT)
        {
            VectorCopy(p1, f->m_Points[f->m_NumPoints]);
            f->m_NumPoints++;
        }
        else if (sides[i] == SIDE_BACK)
        {
            VectorCopy(p1, b->m_Points[b->m_NumPoints]);
            b->m_NumPoints++;
        }

        if ((sides[i + 1] == SIDE_ON) || (sides[i + 1] == sides[i]))  // | instead of || for branch optimization
        {
            continue;
        }

        // generate a split point
        vec3_t mid;
        unsigned int tmp = i + 1;
        if (tmp >= m_NumPoints)
        {
            tmp = 0;
        }
        vec_t* p2 = m_Points[tmp];
        dot = dists[i] / (dists[i] - dists[i + 1]);
#if 0
        // TODO FIXME: this is nightfire logic! 
        for (j = 0; j < 3; j++)
        {                                                  // avoid round off error when possible
            if (normal[j] < 1.0)
            {
                if (normal[j] > -1.0)
                {
                    mid[j] = p1[j] + dot * (p2[j] - p1[j]);
                }
                else
                {
                    mid[j] = -dist;
                }
            }
            else
            {
                mid[j] = dist;
            }
        }
#else
#if 0
        for (j = 0; j < 3; j++)
        {                                                  // avoid round off error when possible
            if (normal[j] < 1.0 - NORMAL_EPSILON)
            {
                if (normal[j] > -1.0 + NORMAL_EPSILON)
                {
                    mid[j] = p1[j] + dot * (p2[j] - p1[j]);
                }
                else
                {
                    mid[j] = -dist;
                }
            }
            else
            {
                mid[j] = dist;
            }
        }
#else
        for (j = 0; j < 3; j++)
        {                                                  // avoid round off error when possible
            if (normal[j] == 1)
                mid[j] = dist;
            else if (normal[j] == -1)
                mid[j] = -dist;
            else
                mid[j] = p1[j] + dot * (p2[j] - p1[j]);
        }
#endif
#endif

        VectorCopy(mid, f->m_Points[f->m_NumPoints]);
        VectorCopy(mid, b->m_Points[b->m_NumPoints]);
        f->m_NumPoints++;
        b->m_NumPoints++;
    }

    if ((f->m_NumPoints > maxpts) || (b->m_NumPoints > maxpts)) // | instead of || for branch optimization
        Error("Winding::Clip : points exceeded estimate");
    
    if ((f->m_NumPoints > MAX_POINTS_ON_WINDING) || (b->m_NumPoints > MAX_POINTS_ON_WINDING)) // | instead of || for branch optimization
        Error("Winding::Clip : MAX_POINTS_ON_WINDING");
#if 1
    // NIGHTFIRE fixme TODO , nightfire does not have these calls!
    f->RemoveColinearPoints();
    b->RemoveColinearPoints();
#endif
    return true;
}

bool Winding::Clip(const vec3_t normal, const vec_t dist, bool keepon)
{
    vec_t           dists[MAX_POINTS_ON_WINDING];
    unsigned int             sides[MAX_POINTS_ON_WINDING];
    unsigned int             counts[3];
    vec_t           dot;
    unsigned int             i, j;

    counts[0] = counts[1] = counts[2] = 0;

    // determine sides for each point
    // do this exactly, with no epsilon so tiny portals still work
    for (i = 0; i < m_NumPoints; i++)
    {
        dot = DotProduct(m_Points[i], normal);
        dot -= dist;
        dists[i] = dot;
        if (dot > ON_EPSILON)
        {
            sides[i] = SIDE_FRONT;
        }
        else if (dot < ON_EPSILON)
        {
            sides[i] = SIDE_BACK;
        }
        else
        {
            sides[i] = SIDE_ON;
        }
        counts[sides[i]]++;
    }
    sides[i] = sides[0];
    dists[i] = dists[0];

    if (keepon && !counts[0] && !counts[1])
    {
        return true;
    }

    if (!counts[0])
    {
        delete[] m_Points;
        m_Points = NULL;
        m_NumPoints = 0;
        return false;
    }

    if (!counts[1])
    {
        return true;
    }

    unsigned maxpts = m_NumPoints + 8;                            // can't use counts[0]+2 because of fp grouping errors
    unsigned newNumPoints = 0;
    vec3_t* newPoints = new vec3_t[maxpts];
    memset(newPoints, 0, sizeof(vec3_t) * maxpts);

    for (i = 0; i < m_NumPoints; i++)
    {
        vec_t* p1 = m_Points[i];

        if (sides[i] == SIDE_ON)
        {
            VectorCopy(p1, newPoints[newNumPoints]);
            newNumPoints++;
            continue;
        }
        else if (sides[i] == SIDE_FRONT)
        {
            VectorCopy(p1, newPoints[newNumPoints]);
            newNumPoints++;
        }

        if (sides[i + 1] == SIDE_ON || sides[i + 1] == sides[i])
        {
            continue;
        }

        // generate a split point
        vec3_t mid;
        unsigned int tmp = i + 1;
        if (tmp >= m_NumPoints)
        {
            tmp = 0;
        }
        vec_t* p2 = m_Points[tmp];
        dot = dists[i] / (dists[i] - dists[i + 1]);
        for (j = 0; j < 3; j++)
        {                                                  // avoid round off error when possible
#if 1
            // nightfire logic
            if (normal[j] < 1.0)
            {
                if (normal[j] > -1.0)
                {
                    mid[j] = p1[j] + dot * (p2[j] - p1[j]);
                }
                else
                {
                    mid[j] = -dist;
                }
            }
            else
            {
                mid[j] = dist;
            }
#else
            // hl1 logic
            if (normal[j] < 1.0 - NORMAL_EPSILON)
            {
                if (normal[j] > -1.0 + NORMAL_EPSILON)
                {
                    mid[j] = p1[j] + dot * (p2[j] - p1[j]);
                }
                else
                {
                    mid[j] = -dist;
                }
            }
            else
            {
                mid[j] = dist;
            }
#endif
        }

        VectorCopy(mid, newPoints[newNumPoints]);
        newNumPoints++;
    }

    if (newNumPoints > maxpts)
    {
        Error("Winding::Clip : points exceeded estimate");
    }

    delete[] m_Points;
    m_Points = newPoints;
    m_NumPoints = newNumPoints;

    RemoveColinearPoints();

    return true;
}

bool Winding::Clip(const struct plane_s& split, bool keepon)
{
    return Clip(split.normal, split.dist, keepon);
}

bool Winding::Chop(const vec3_t normal, const vec_t dist, bool keep_front)
{
    Winding* f;
    Winding* b;

    Clip(false, normal, dist, &f, &b);

    if (!keep_front)
        std::swap(f, b);

    if (b)
        delete b;

    if (f)
    {
        delete[] m_Points;
        m_NumPoints = f->m_NumPoints;
        m_Points = f->m_Points;
        f->m_Points = NULL;
        delete f;
        return true;
    }
    else
    {
        m_NumPoints = 0;
        delete[] m_Points;
        m_Points = NULL;
        return false;
    }
}

int Winding::WindingOnPlaneSide(const vec3_t normal, const vec_t dist)
{
    bool            front, back;
    unsigned int    i;
    vec_t           d;

    front = false;
    back = false;
    for (i = 0; i < m_NumPoints; i++)
    {
        d = DotProduct(m_Points[i], normal) - dist;
        if (d < -ON_EPSILON)
        {
            if (front)
            {
                return SIDE_CROSS;
            }
            back = true;
            continue;
        }
        if (d > ON_EPSILON)
        {
            if (back)
            {
                return SIDE_CROSS;
            }
            front = true;
            continue;
        }
    }

    if (back)
    {
        return SIDE_BACK;
    }
    if (front)
    {
        return SIDE_FRONT;
    }
    return SIDE_ON;
}

int Winding::classifyPointAgainstPlaneEdges(char* sides_buffer, const int max_sides, const plane_s& plane, const vec3_t point)
{
    int backSideCount = 0;

    for (unsigned int i = 0; i < m_NumPoints; ++i)
    {
        const vec3_t& currentPoint = m_Points[i];
        const vec3_t& nextPoint = m_Points[(i + 1) % m_NumPoints];

        vec3_t edge;
        VectorSubtract(currentPoint, nextPoint, edge);
        VectorNormalize(edge);

        vec3_t cross;
        CrossProduct(edge, plane.normal, cross);
        VectorNormalize(cross);

        double dotTestPoint = DotProduct(cross, point);
        double dotCurrentPoint = DotProduct(cross, currentPoint);

        if (dotTestPoint - dotCurrentPoint <= DIST_EPSILON)
        {
            sides_buffer[i] = SIDE_FRONT;
        }
        else
        {
            sides_buffer[i] = SIDE_BACK;
            ++backSideCount;
        }
    }

    return backSideCount;
}