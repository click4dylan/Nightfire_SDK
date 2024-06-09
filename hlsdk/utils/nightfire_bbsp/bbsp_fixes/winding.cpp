
#include <Windows.h>
#include <stdlib.h>
#include "globals.h"
#include "helpers.h"
#include "bsp_structs.h"
#include "winding.h"

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

void Winding::RemoveColinearPoints()
{
    vec3_t p[MAX_POINTS_ON_WINDING]{}; // temporary array to store points
    unsigned int nump = 0; // number of points in the temporary array

    // First pass: zero out near-zero components
    for (unsigned int i = 0; i < m_NumPoints; i++)
    {
        vec3_t& point = m_Points[i];

        if (fabs(point[0]) < NORMAL_EPSILON)
            point[0] = 0;

        if (fabs(point[1]) < NORMAL_EPSILON)
            point[1] = 0;

        if (fabs(point[2]) < NORMAL_EPSILON)
            point[2] = 0;
    }

    // Second pass: remove colinear points
    for (unsigned int i = 0; i < m_NumPoints; i++)
    {
        vec3_t& point = m_Points[i];
        vec3_t& next = m_Points[(i + 1) % m_NumPoints];

        vec3_t diff;
        VectorSubtract(point, next, diff);

        if (!VectorCompare(diff, vec3_origin))
        {
            VectorCopy(point, p[nump]);
            nump++;
        }
    }

    // Update the number of points
    if (m_NumPoints != nump)
    {
        m_NumPoints = nump;
        if (nump)
            memcpy(m_Points, p, nump * sizeof(vec3_t));
    }

    if (!m_NumPoints)
        return;

    // Third pass: check for additional colinear points and remove them
    unsigned int newNumPoints = 0;
    for (unsigned int i = 0; i < m_NumPoints; ++i)
    {
        vec3_t& point = m_Points[i];
        vec3_t& next = m_Points[(i + 1) % m_NumPoints];
        vec3_t& prev = m_Points[(m_NumPoints + i - 1) % m_NumPoints];

        vec3_t diffNext, diffPrev, sum;
        VectorSubtract(point, next, diffNext);
        VectorSubtract(point, prev, diffPrev);
        VectorNormalize(diffNext);
        VectorNormalize(diffPrev);

        VectorAdd(diffPrev, diffNext, sum);

        if (!VectorCompare(sum, vec3_origin))
        {
            VectorCopy(point, p[newNumPoints]);
            ++newNumPoints;
        }
    }

    // Final update of the number of points
    if (newNumPoints < 3)
        m_NumPoints = 0;

#if 0
    else
        m_NumPoints = newNumPoints;

    if (newNumPoints)
        memcpy(m_Points, p, newNumPoints * sizeof(vec3_t));
#endif
}

Winding* lastwinding{};

Winding::~Winding()
{
    if (lastwinding == this)
        lastwinding = nullptr;
    --g_numWindings;
    if (m_Points)
        delete[] m_Points;
    m_Points = nullptr;
}

Winding::Winding(const Winding& other)
{
    if (this == lastwinding)
        int breakp = 1;
    lastwinding = this;
    ++g_numWindings;
    m_NumPoints = other.m_NumPoints;
    m_MaxPoints = max(other.m_MaxPoints, 8);//(m_NumPoints + 3) & ~3;   // groups of 4

    m_Points = new vec3_t[m_MaxPoints];
    if (m_NumPoints)
        memcpy(m_Points, other.m_Points, sizeof(vec3_t) * m_NumPoints);
}

Winding::Winding()
{
    if (this == lastwinding)
        int breakp = 1;
    lastwinding = this;
    ++g_numWindings;
    m_NumPoints = 0;
    m_MaxPoints = 8;
    m_Points = new vec3_t[m_MaxPoints];
    memset(m_Points, 0, sizeof(vec3_t) * 8);
}

Winding::Winding(const struct dface_s& face)
{
    if (this == lastwinding)
        int breakp = 1;
    lastwinding = this;
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
    if (this == lastwinding)
        int breakp = 1;
    lastwinding = this;
    ++g_numWindings;
    vec3_t normal;
    vec_t dist;

    VectorCopy(plane.normal, normal);
    dist = plane.dist;
    initFromPlane(normal, dist);
}

Winding::Winding(UINT32 numpoints)
{
    if (this == lastwinding)
        int breakp = 1;
    lastwinding = this;
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
}

void Winding::shiftPoints(unsigned int offset, unsigned int start_index)
{
    unsigned int adjusted_start = start_index + offset;

    // If the adjusted start index exceeds the number of points
    if (adjusted_start > m_NumPoints)
    {
        start_index = 0;
        unsigned int temp_num_points = m_NumPoints;

        do
        {
            adjusted_start -= temp_num_points;
            temp_num_points = adjusted_start + temp_num_points - offset;
            offset = adjusted_start;
        } while (adjusted_start > temp_num_points);

        m_NumPoints = temp_num_points;
    }

    unsigned int points_to_move = m_NumPoints - adjusted_start;

    // If there are points to move, perform the memory copy
    if (points_to_move > 0)
    {
        // Calculate the destination and source pointers for memcpy
        vec3_t* dest = &m_Points[start_index];
        vec3_t* src = &m_Points[start_index + offset];
        std::memcpy(dest, src, sizeof(vec3_t) * points_to_move);
    }

    // Update the number of points in the winding
    m_NumPoints -= offset;
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

    if (m_NumPoints >= newsize)
        m_NumPoints = newsize;

    if (m_NumPoints)
       memcpy(newpoints, m_Points, 4 * ((sizeof(vec3_t) * m_NumPoints) >> 2));
   
    if (m_Points)
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
    vec_t           dists[MAX_POINTS_ON_WINDING + 8];
    int             sides[MAX_POINTS_ON_WINDING + 8];
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
#if 1
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

    // NIGHTFIRE fixme TODO , nightfire does not have these calls!
    //f->RemoveColinearPoints();
    //b->RemoveColinearPoints();

    if (!f->HasPoints())
    {
        delete f;
        *front = nullptr;
    }
    if (!b->HasPoints())
    {
        delete b;
        *back = nullptr;
    }

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

#if 1 
    // nightfire logic
    if (keepon)
    {
        if (!counts[0] && !counts[1])
            return true;
    }
    else if (!counts[0])
    {
        if (m_Points)
            delete[] m_Points;
        m_Points = NULL;
        m_NumPoints = 0;
        m_MaxPoints = 0;
        return false;
    }

    if (!counts[1])
    {
        return true;
    }
#else
    // hl1 logic
    if (keepon && !counts[0] && !counts[1])
    {
        return true;
    }

    if (!counts[0])
    {
        if (m_Points)
            delete[] m_Points;
        m_Points = NULL;
        m_NumPoints = 0;
        m_MaxPoints = 0;
        return false;
    }

    if (!counts[1])
    {
        return true;
    }
#endif

    unsigned maxpts = m_NumPoints + 8;                            // can't use counts[0]+2 because of fp grouping errors
    unsigned newNumPoints = 0;
    vec3_t newPoints[MAX_POINTS_ON_WINDING];
    //memset(newPoints, 0, sizeof(newPoints)); // nightfire does not clear this

    for (i = 0; i < m_NumPoints; i++)
    {
        const vec_t* p1 = m_Points[i];

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

    if (newNumPoints > m_MaxPoints)
        resize(newNumPoints);

    memcpy(m_Points, newPoints, 4 * ((sizeof(vec3_t) * newNumPoints) >> 2));
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
        if (m_Points)
            delete[] m_Points;
        m_NumPoints = f->m_NumPoints;
        m_MaxPoints = f->m_MaxPoints;
        m_Points = f->m_Points;
        f->m_Points = nullptr;
        delete f;
        return true;
    }
    else
    {
        m_MaxPoints = 0;
        m_NumPoints = 0;
        if (m_Points)
            delete[] m_Points;
        m_Points = nullptr;
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