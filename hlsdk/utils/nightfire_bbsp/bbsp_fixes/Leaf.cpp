
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
    
    leaf->contents = node->contents & 0xFF;
    if (!leaf->contents)
        leaf->contents = CONTENTS_EMPTY;

    //FIXME: ZHLT_LARGERANGE
    VectorCopy(node->mins, leaf->mins);
    VectorCopy(node->maxs, leaf->maxs);
 
    leaf->visofs = -1;
    leaf->firstmarksurface = 0;
    leaf->nummarksurfaces = 0;

    if (node->markfaces) 
    {
        leaf->firstmarksurface = g_numDMarkSurfaces;

        for (const auto& markface : *node->markfaces) 
        {
            if ((markface->flags & (CONTENTS_BSP | SURFACEFLAG_NODRAW | CONTENTS_PORTAL | CONTENTS_SOLID | CONTENTS_EMPTY)) == 0)
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

void CountLeaves(int level, node_t* node)
{
    if (node->planenum == PLANENUM_LEAF)
    {
        // It's a leaf
        if (node->occupied)
            ++g_numOccupiedLeafs;
        if (node->contents == CONTENTS_SOLID)
            ++g_numSolidLeafs2;

        ++g_numLeafs2;
    }

    if (node->children[0])
        CountLeaves(++level, node->children[0]);
    if (node->children[1])
        CountLeaves(++level, node->children[1]);
}

void NumberLeafs_r(int depth, node_t* node)
{
    if (!node)
        return;

    if (node->planenum != PLANENUM_LEAF)
    {
        NumberLeafs_r(++depth, node->children[0]);
        NumberLeafs_r(++depth, node->children[1]);
        return;
    }

    node->visleafnum = num_visleafs++;

    if (node->contents == CONTENTS_SOLID)
    {
        // solid block, viewpoint never inside
        
        //FIXME: todo:
        //node->visleafnum = -1; // nightfire is missing this
        return;
    }

    for (portal_t* p = node->portals; p;)
    {
        if (p->nodes[0] == node)  // only write out from first leaf
        {
            if (p->nodes[0]->contents != CONTENTS_SOLID && p->nodes[1]->contents != CONTENTS_SOLID)
            {
                ++num_visportals;
            }
            p = p->next[0];
        }
        else
        {
            p = p->next[1];
        }
    }
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
    if (node->planenum == PLANENUM_LEAF)
    {
        node->visleafnum = g_numVisLeafs++;
        return;
    }

    node->visleafnum = g_numVisLeafs++;
    AssignVisLeafNumbers_r(node->children[0]);
    AssignVisLeafNumbers_r(node->children[1]);
}