
#include <stdio.h>
#include <math.h>
#include "helpers.h"
#include "winding.h"
#include "bsp_structs.h"
#include "face.h"
#include "globals.h"
#include "Leaf.h"
#include "textures.h"
#include "brush.h"
#include "bspfile.h"
#include "portals.h"
#include "log.h"

void WriteDrawLeaf(node_t* node) 
{
    dleaf_t* leaf = &g_dleafs[g_numDLeafs++];
    
    leaf->contents = node->leaf_type & 0xFF;
    if (!leaf->contents)
        leaf->contents = LEAF_EMPTY_AKA_NOT_OPAQUE;

    leaf->mins[0] = node->mins[0];
    leaf->mins[1] = node->mins[1];
    leaf->mins[2] = node->mins[2];
    leaf->maxs[0] = node->maxs[0];
    leaf->maxs[1] = node->maxs[1];
    leaf->maxs[2] = node->maxs[2];
    leaf->visofs = -1;

    leaf->firstmarksurface = 0;
    leaf->nummarksurfaces = 0;

    if (node->markfaces) 
    {
        leaf->firstmarksurface = g_numDMarkSurfaces;

        for (const auto& markface : *node->markfaces) 
        {
            if ((markface->flags & (CONTENTS_BSP | CONTENTS_NODRAW | CONTENTS_PORTAL | CONTENTS_UNKNOWN | CONTENTS_SOLID)) == 0)
            {
                WriteFace_AkaBuildDrawIndicesForFace(markface);
                g_dmarksurfaces[g_numDMarkSurfaces++] = markface->outputnumber;
            }
        }

        node->ClearMarkFaces();

        leaf->nummarksurfaces = g_numDMarkSurfaces - leaf->firstmarksurface;
    }

    leaf->firstdrawmarkbrush_index = 0;
    leaf->numdrawmarkbrushes = 0;

    if (node->markbrushes) 
    {
        leaf->firstdrawmarkbrush_index = g_numDMarkBrushes;

        for (const auto& markbrush : *node->markbrushes)
        {
            if ((markbrush->brushflags & (CONTENTS_HINTSKIP | CONTENTS_ORIGIN)) == 0)
            {
                WriteDrawBrush(markbrush);
                g_dmarkbrushes[g_numDMarkBrushes++] = markbrush->output_num;
            }
        }
        node->ClearMarkBrushes();

        leaf->numdrawmarkbrushes = g_numDMarkBrushes - leaf->firstdrawmarkbrush_index;
    }
}

node_t* PointInLeaf(node_t* node, const vec3_t point)
{
    if (node->planenum == -1)
        return node;

    const plane_t& plane = gMappedPlanes[node->planenum];
    vec_t d = DotProduct(plane.normal, point) - plane.dist;

    if (d > 0)
        return PointInLeaf(node->children[0], point);

    return PointInLeaf(node->children[1], point);
}

bool PlaceOccupant(const vec3_t point, node_t* headnode, unsigned int entindex)
{
    node_t* n = PointInLeaf(headnode, point);
    MarkLeafOccupancyAndCheckLeaks(n, entindex);
    if (g_bLeaked)
    {
        g_CurrentEntity = entindex;
        MarkLeakTrail(nullptr);
    }
    return true;
}

void MarkLeafOccupancyAndCheckLeaks(node_t* headnode, unsigned int occupancyValue)
{
    if (headnode == g_OutsideNode) 
    {
        g_bLeaked = 1;
        return;
    }

    if (!headnode->occupied && headnode->leaf_type != LEAF_SOLID_AKA_OPAQUE) 
    {
        headnode->occupied = occupancyValue;

        for (portal_t* portal = headnode->portals; portal != nullptr; portal = portal->next[portal->nodes[1] == headnode]) 
        {
            if (portal->nodes[0] == headnode)
                MarkLeafOccupancyAndCheckLeaks(portal->nodes[1], occupancyValue);
            else
                MarkLeafOccupancyAndCheckLeaks(portal->nodes[0], occupancyValue);

            if (g_bLeaked) 
            {
                MarkLeakTrail(portal);
                return;
            }
        }
    }
}

void CountLeaves(int level, node_t* node)
{
    if (node->planenum == -1)
    {
        // It's a leaf
        if (node->occupied)
            ++g_numOccupiedLeafs;
        if (node->leaf_type == LEAF_SOLID_AKA_OPAQUE)
            ++g_numSolidLeafs2;

        ++g_numLeafs2;
    }

    if (node->children[0])
        CountLeaves(++level, node->children[0]);
    if (node->children[1])
        CountLeaves(++level, node->children[1]);
}

void PrintLeafMetrics(node_t* node, const char* name)
{
    g_numLeafs2 = 0;
    g_numSolidLeafs2 = 0;
    g_numOccupiedLeafs = 0;
    g_numFloodedLeafs = 0;

    CountLeaves(0, node);

    Verbose("Leaf metrics : %s\n", name);
    Verbose("%5i leafs\n", g_numLeafs2);
    Verbose("%5i solid leafs\n", g_numSolidLeafs2);
    Verbose("%5i occupied leafs\n", g_numOccupiedLeafs);
    Verbose("%5i flooded leafs\n", g_numFloodedLeafs);
}

void AssignVisLeafNumbers_r(node_t* node)
{
    while (node->planenum != -1)
    {
        node->visleafnum = g_numVisLeafs++;
        AssignVisLeafNumbers_r(node->children[0]);
        node = node->children[1];
    }
    node->visleafnum = g_numVisLeafs++;
}