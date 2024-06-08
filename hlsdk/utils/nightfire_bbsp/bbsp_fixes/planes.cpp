#include "planes.h"
#include "brush.h"
#include "log.h"
#include "helpers.h"
#include "textures.h"

unsigned int ChoosePlaneFromList(node_t* node, face_t* list)
{
    unsigned int bestSplit = -1;        // Best plane number found
    int bestValue = -1000000;     // Best value found
    face_t* split;          // Current face being examined
    face_t* check;            // Other face for comparison
    unsigned int sameplane, splits;
    plane_t* plane;
    int side;              // Side of the current winding on the plane
    int value;             // Current value calculated for the plane

    for (split = list; split; split = split->next) {
        if (!split->winding->Valid())
            continue;

        plane = &gMappedPlanes[split->planenum];
        sameplane = 0;
        splits = 0;

        for (check = list; check; check = check->next) {
            if (check != split && check->winding->Valid()) {
                // Check if the planes are identical or opposite
                if (check->planenum == split->planenum || (check->planenum ^ 1) == split->planenum) {
                    ++sameplane;
                    continue;
                }
                else {
                    side = check->winding->WindingOnPlaneSide(plane->normal, plane->dist);
                    if (side == SIDE_CROSS) {
                        ++splits;
                    }
                    else if (side == SIDE_ON) {
                        ++sameplane;
                    }
                }
            }
        }

        value = sameplane - splits;

        // bestSplit defaults to leaf (-1)
        if (value > bestValue) {
            bestSplit = split->planenum;
            bestValue = value;
        }
    }

    return bestSplit;
}

unsigned int ChooseMidPlaneFromListHL1(node_t* node, face_t* original_face, const vec3_t mins, const vec3_t maxs)
{
    int             j, l;
    face_t* p;
    face_t* bestsurface;
    vec_t           bestvalue;
    vec_t           value;
    vec_t           dist;
    plane_t* plane;

    //
    // pick the plane that splits the least
    //
    bestvalue = 6 * 8192 * 8192;
    bestsurface = NULL;

    for (p = original_face; p; p = p->next)
    {
        bool on_node = p->planenum == node->planenum || p->planenum == (node->planenum ^ 1);
        if (on_node)
        {
            continue;
        }

        plane = &gMappedPlanes[p->planenum];

        // check for axis aligned surfaces
        l = plane->closest_axis;
        if (l > plane_z)
        {
            continue;
        }

        //
        // calculate the split metric along axis l, smaller values are better
        //
        value = 0;

        dist = plane->dist * plane->normal[l];
        for (j = 0; j < 3; j++)
        {
            if (j == l)
            {
                value += (maxs[l] - dist) * (maxs[l] - dist);
                value += (dist - mins[l]) * (dist - mins[l]);
            }
            else
            {
                value += 2 * (maxs[j] - mins[j]) * (maxs[j] - mins[j]);
            }
        }

        if (value > bestvalue)
        {
            continue;
        }

        //
        // currently the best!
        //
        bestvalue = value;
        bestsurface = p;
    }

    if (!bestsurface)
    {
        for (p = original_face; p; p = p->next)
        {
            bool on_node = p->planenum == node->planenum || p->planenum == (node->planenum ^ 1);
            if (!on_node)
            {
                return p->planenum;                                  // first valid surface
            }
        }
        //Error2("ChooseMidPlaneFromList: no valid planes");
        return -1;
    }

    return bestsurface->planenum;
}

unsigned int ChooseMidPlaneFromList(node_t* node, int axis)
{
    vec3_t normal;
    memset(normal, 0, sizeof(normal));
    normal[axis] = 1.0;

    vec_t dist = (vec_t)(int)(g_MaxNodeSize * ((unsigned __int64)floor(node->mins[axis] / (double)g_MaxNodeSize) + 1));

    return FindIntPlane(normal, dist);
}

//plane_s gMappedPlanes[MAX_MAP_PLANES];
plane_s sorted_planes[MAX_MAP_PLANES * 2];

unsigned int FindIntPlane(const vec_t* const normal, const vec_t dist)
{
#if 1
    plane_t desired_plane;
    desired_plane.dist = dist;
    VectorCopy(normal, desired_plane.normal);

    SnapNormal(desired_plane.normal);
    desired_plane.dist = SnapPlaneDist(desired_plane.dist);

    for (unsigned i = 0; i < gNumMappedPlanes; ++i)
    {
        plane_t& existing_plane = gMappedPlanes[i];
        if (!IsDifferentPlane(&existing_plane, &desired_plane))
            return i;
    }

    hlassume(gNumMappedPlanes < MAX_MAP_PLANES, assume_MAX_MAP_PLANES);

    desired_plane.closest_axis = (planetypes)PlaneTypeForNormal(desired_plane.normal);
    plane_t& new_plane = gMappedPlanes[gNumMappedPlanes++];
    plane_t& second_new_plane = gMappedPlanes[gNumMappedPlanes++];

    plane_t mirrored_plane = { {-desired_plane.normal[0], -desired_plane.normal[1], -desired_plane.normal[2]}, -desired_plane.dist, desired_plane.closest_axis };

    // always put axial planes facing positive first
    if (desired_plane.closest_axis <= last_axial && (desired_plane.normal[0] < 0.0 || desired_plane.normal[1] < 0.0 || desired_plane.normal[2] < 0.0))
    {
        new_plane = mirrored_plane;
        second_new_plane = desired_plane;
        return gNumMappedPlanes - 1;
    }

    new_plane = desired_plane;
    second_new_plane = mirrored_plane;
    return gNumMappedPlanes - 2;
#else
    DWORD adr = 0x420B70;
    const vec_t* const Normal = normal;
    vec_t Dist = dist;
    __asm
    {
        mov edx, adr
        mov eax, Normal
        lea ecx, Dist
        push[ecx + 4]
        push[ecx]
        call edx
        add esp, 8
    }
#endif
}

unsigned int FindIntPlane(const vec_t* const normal, const vec_t* const origin)
{
#if 1
    plane_t dummy_plane;
    VectorCopy(normal, dummy_plane.normal);
    dummy_plane.dist = DotProduct(origin, normal);
    return FindIntPlane(dummy_plane.normal, dummy_plane.dist);
#else
    DWORD adr = 0x420B70;
    const vec_t* const Normal = normal;
    vec_t Dist = DotProduct(origin, normal);
    __asm
    {
        mov edx, adr
        mov eax, Normal
        lea ecx, Dist
        push[ecx + 4]
        push[ecx]
        call edx
        add esp, 8
    }
#endif
}

int PlaneTypeForNormal(const vec3_t normal)
{
    vec_t           ax, ay, az;

    ax = fabs(normal[0]);

#if 1 
    // nightfire version, todo: fixme / test
    if (ax >= 1.0 - NORMAL_EPSILON)
    {
        return plane_x;
    }

    ay = fabs(normal[1]);
    if (ay >= 1.0 - NORMAL_EPSILON)
    {
        return plane_y;
    }

    az = fabs(normal[2]);
    if (az >= 1.0 - NORMAL_EPSILON)
    {
        return plane_z;
    }
#else
    if (ax == 1.0)
    {
        return plane_x;
    }

    ay = fabs(normal[1]);
    if (ay == 1.0)
    {
        return plane_y;
    }

    az = fabs(normal[2]);
    if (az == 1.0)
    {
        return plane_z;
    }
#endif

    if ((ax > ay) && (ax > az))
    {
        return plane_anyx;
    }
    if ((ay > ax) && (ay > az))
    {
        return plane_anyy;
    }
    return plane_anyz;
}

int IsDifferentPlane(const plane_s* a1, const plane_s* a2)
{
    double X = fabs(a1->dist - a2->dist);
    if (X > 0.5)
        return 1;
    if (X <= DIST_EPSILON && fabs(a1->normal[0] - a2->normal[0]) <= NORMAL_EPSILON && fabs(a1->normal[1] - a2->normal[1]) <= NORMAL_EPSILON && fabs(a1->normal[2] - a2->normal[2]) <= NORMAL_EPSILON)
        return 0;
    return -1;
}

unsigned int PlaneFromPoints(const vec3_t p0, const vec3_t p1, const vec3_t p2)
{
    vec3_t t1, t2, normal;
    vec_t dist;

    /* calc plane normal */
    VectorSubtract(p0, p1, t1);
    VectorSubtract(p2, p1, t2);
    CrossProduct(t1, t2, normal);
    if (VectorNormalize(normal))
    {
        dist = DotProduct(p0, normal);
        return FindIntPlane(normal, dist);
    }
 
    return -1;
}

void AddHullPlane(brush_t* brush, vec3_t normal, vec_t dist)
{
    // Create a new brush side
    side_t* newBrushSide = brush->CreateNewBrushSide();

    // Find the integer plane number
    unsigned int intPlane = FindIntPlane(normal, dist);

    // Assign the integer plane number to the new brush side
    newBrushSide->plane_num = intPlane;

    // Allocate originalFace and winding
    newBrushSide->original_face = new face_t(intPlane);

    // Assign the winding to the original face
    newBrushSide->original_face->winding = new Winding;
    newBrushSide->original_face->brush = brush;
    newBrushSide->original_face->brushside = newBrushSide;

    // Copy default values to brush side texture data
    safe_strncpy(newBrushSide->td.name, "special/bevel", sizeof(newBrushSide->td.name));
    safe_strncpy(newBrushSide->td.material, "wld_fullbright", sizeof(newBrushSide->td.material));
    newBrushSide->td.lightmapscale = 0.0;
    newBrushSide->td.lightmaprotation = 0.0;
    newBrushSide->td.surfaceflags = FLAG_BEVEL;

    // Copy default values to original face texture data
    newBrushSide->original_face->flags = FLAG_BEVEL;
    memset(newBrushSide->original_face->vecs, 0, sizeof(newBrushSide->original_face->vecs));
    newBrushSide->original_face->vecs[0][2] = 1.0;
    newBrushSide->original_face->vecs[1][2] = 1.0;

    // Set texinfo for brush texture
    newBrushSide->original_face->texinfo = TexinfoForBrushTexture(newBrushSide, vec3_origin);
}

#define Q_rint( in ) ( (vec_t)floor( in + 0.5 ) )

vec_t SnapPlaneDist(const vec_t dist, const vec_t epsilon)
{
    if (fabs(dist - Q_rint(dist)) < epsilon)
        return Q_rint(dist);
    return dist;
}

/*
   SnapNormal()
   Snaps a near-axial normal vector.
   Returns qtrue if and only if the normal was adjusted.
 */

bool SnapNormal(vec3_t normal, vec_t epsilon)
{
#if Q3MAP2_EXPERIMENTAL_SNAP_NORMAL_FIX
    int i;
    bool adjusted = false;

    // A change from the original SnapNormal() is that we snap each
    // component that's close to 0.  So for example if a normal is
    // (0.707, 0.707, 0.0000001), it will get snapped to lie perfectly in the
    // XY plane (its Z component will be set to 0 and its length will be
    // normalized).  The original SnapNormal() didn't snap such vectors - it
    // only snapped vectors that were near a perfect axis.

    for (i = 0; i < 3; i++)
    {
        if (normal[i] != 0.0 && -epsilon < normal[i] && normal[i] < epsilon) {
            normal[i] = 0.0;
            adjusted = true;
        }
    }

    if (adjusted) {
        VectorNormalize(normal, normal);
        return true;
    }
    return false;
#else

#if 1
    // nightfire code:

    for (int i = 0; i < 3; ++i)
    {
        if (normal[i] > 1.0 - epsilon)
        {
            VectorClear(normal);
            normal[i] = 1.0;
            return true;
        }
        else if (normal[i] < epsilon - 1.0)
        {
            VectorClear(normal);
            normal[i] = -1.0;
            return true;
        }
    }

    return false;
#else
    int i;

    // I would suggest that you uncomment the following code and look at the
    // results:

    /*
       Sys_Printf("normalEpsilon is %f\n", normalEpsilon);
       for (i = 0;; i++)
       {
        normal[0] = 1.0;
        normal[1] = 0.0;
        normal[2] = i * 0.000001;
        VectorNormalize(normal, normal);
        if (1.0 - normal[0] >= epsilon) {
            Sys_Printf("(%f %f %f)\n", normal[0], normal[1], normal[2]);
            Error("SnapNormal: test completed");
        }
       }
     */

     // When the normalEpsilon is 0.00001, the loop will break out when normal is
     // (0.999990 0.000000 0.004469).  In other words, this is the vector closest
     // to axial that will NOT be snapped.  Anything closer will be snaped.  Now,
     // 0.004469 is close to 1/225.  The length of a circular quarter-arc of radius
     // 1 is PI/2, or about 1.57.  And 0.004469/1.57 is about 0.0028, or about
     // 1/350.  Expressed a different way, 1/350 is also about 0.26/90.
     // This means is that a normal with an angle that is within 1/4 of a degree
     // from axial will be "snapped".  My belief is that the person who wrote the
     // code below did not intend it this way.  I think the person intended that
     // the epsilon be measured against the vector components close to 0, not 1.0.
     // I think the logic should be: if 2 of the normal components are within
     // epsilon of 0, then the vector can be snapped to be perfectly axial.
     // We may consider adjusting the epsilon to a larger value when we make this
     // code fix.

    for (i = 0; i < 3; i++)
    {
        if (fabs(normal[i] - 1) < epsilon) {
            VectorClear(normal);
            normal[i] = 1;
            return true;
        }
        if (fabs(normal[i] - -1) < epsilon) {
            VectorClear(normal);
            normal[i] = -1;
            return false;
        }
    }
    return false;
#endif
#endif
}

bool PlaneEqual(const plane_s* plane1, const plane_s* plane2, double onepsilon, double normalepsilon)
{
    if (fabs(plane1->dist - plane2->dist) >= onepsilon)
        return false;

    for (int i = 0; i < 3; ++i)
    {
        if (fabs(plane1->normal[i] - plane2->normal[i]) >= normalepsilon)
            return false;
    }
    return true;
}
