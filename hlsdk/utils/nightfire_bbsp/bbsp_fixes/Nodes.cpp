
#include <stdio.h>
#include <math.h>
#include "helpers.h"
#include "winding.h"
#include "bsp_structs.h"
#include "face.h"
#include "globals.h"
#include "Leaf.h"
#include "brush.h"
#include "log.h"
#include "Nodes.h"
#include "portals.h"
#include "planes.h"

void node_t::ClearMarkFaces()
{
    if (markfaces)
    {
        markfaces->clear();
        delete markfaces;
        markfaces = nullptr;
        --g_numNodesWithMarkFaces;
    }
}

void node_t::ClearMarkBrushes()
{
    if (markbrushes)
    {
        markbrushes->clear();
        delete markbrushes;
        markbrushes = nullptr;
        --g_numNodesWithMarkBrushes;
    }
}

bool node_t::IsPortalLeaf()
{
    return !valid && children[0] == nullptr && children[1] == nullptr;
}

// disable warning for loss of floating point precision (the .bsp uses floats!)
#pragma warning( disable : 4244 )

void WriteDrawNodes_r(int depth, const node_t* const node)
{
    dnode_t* n;
    int i;

    // emit a node
    hlassume(g_numDNodes < MAX_MAP_NODES, assume_MAX_MAP_NODES);
    n = &g_dnodes[g_numDNodes++];

    VectorCopy(node->mins, n->mins);
    VectorCopy(node->maxs, n->maxs);

    if (node->planenum & 1)
    {
        plane_t* plane = &gMappedPlanes[node->planenum];
        Fatal(assume_ODD_PLANE_IN_NODE, "WriteDrawNodes_r: odd planenum (%i) (%.2f %.2f %.2f) (%.2f)", node->planenum, plane->normal[0], plane->normal[1], plane->normal[2], plane->dist);
    }

    n->planenum = node->planenum;

    //
    // recursively output the other nodes
    //
    for (i = 0; i < 2; i++)
    {
        if (node->children[i]->planenum == PLANENUM_LEAF)
        {
            //nightfire doesn't do this, fixme
#if 0
            if (node->children[i]->contents == CONTENTS_SOLID)
            {
                n->children[i] = -1;
            }
            else
#endif
            {
                //fixme
                n->children[i] = -1 - g_numDLeafs;//-(g_numDLeafs + 1);
                WriteDrawLeaf(node->children[i]);
            }
        }
        else
        {
            n->children[i] = g_numDNodes;
            WriteDrawNodes_r(depth + 1, node->children[i]);
        }
    }
}

#pragma warning( default : 4244 )

void EmitDrawNode_r(node_t* n)
{
    if (n->planenum == -1)
        WriteDrawLeaf(n);
    else
        WriteDrawNodes_r(0, n);
    CheckFatal();
}

void CalcNodeChildBounds(unsigned int planenum, node_t* node)
{
    for (int child = 0; child < 2; ++child)
    {
        for (int axis = 0; axis < 3; ++axis)
        {
            node->children[child]->mins[axis] = node->mins[axis];
            node->children[child]->maxs[axis] = node->maxs[axis];
        }
    }

    const plane_t* plane = &gMappedPlanes[planenum];
    const auto axis = plane->closest_axis;
    if (axis <= plane_z)
    {
        if (plane->normal[axis] < 1)
        {
            node->children[0]->mins[axis] = -plane->dist;
            node->children[1]->maxs[axis] = -plane->dist;
        }
        else
        {
            node->children[0]->mins[axis] = plane->dist;
            node->children[1]->maxs[axis] = plane->dist;
        }
    }
}

void CalcNodeBoundsFromBrushes(node_t* node, entity_t* ent)
{
    // Reset the node's bounds
    InvalidateBounds(node->mins, node->maxs);

    // Iterate through each brush in the entity
    for (unsigned int i = 0; i < ent->numbrushes; ++i)
    {
        // Calculate bounds from the current brush
        CalcBrushBounds(node->maxs, node->mins, ent->brushes[i]);
    }

    // Check if the calculated bounds are invalid
    if (node->maxs[0] < node->mins[0] || node->maxs[1] < node->mins[1] || node->maxs[2] < node->mins[2])
    {
        // If invalid, reset bounds to default values
        for (int i = 0; i < 3; ++i)
        {
            node->mins[i] = 0.0;
            node->maxs[i] = 0.0;
        }
    }
}

void CalcNodeBoundsFromFaces(node_t* node, face_t* face)
{
    // Reset the node's bounds
    InvalidateBounds(node->mins, node->maxs);

    // Iterate through each brush in the entity
    for (; face; face = face->next)
    {
        // Calculate bounds from the current brush
        AddFaceToBounds(face, node->mins, node->maxs);
    }

    // Check if the calculated bounds are invalid
    if (node->maxs[0] < node->mins[0] || node->maxs[1] < node->mins[1] || node->maxs[2] < node->mins[2])
    {
        // If invalid, reset bounds to default values
        for (int i = 0; i < 3; ++i)
        {
            node->mins[i] = 0.0;
            node->maxs[i] = 0.0;
        }
    }
}

#if 0
void RecursivelyFreeNode(node_t* node)
{
    // Recursively free the children nodes
    if (node->children[0])
        RecursivelyFreeNode(node->children[0]);
    if (node->children[1])
        RecursivelyFreeNode(node->children[1]);

    // Free portals associated with the node
    for (portal_t* portals = node->portals; portals;)
    {
        portal_t* nextPortal = portals->next[portals->nodes[1] == node];
        delete portals;
        portals = nextPortal;
    }

    // Free markfaces associated with the node
    node->markfaces->clear();
    delete node->markfaces;

    // Free markbrushes associated with the node
    node->markbrushes->clear();
    delete node->markbrushes;

    // Free the node itself
    free(node);
    --g_numNodesAllocated;
}
#endif

void CountNodesAndLeafsByPlane(int level, node_t* node) 
{
    if (node->planenum != -1) 
    {
        // It's a node
        if (node->contents == CONTENTS_EMPTY)
           ++g_numEmptyNodes;
        else if (node->contents == CONTENTS_SOLID)
            ++g_numSolidNodes;
        CountNodesAndLeafsByPlane(++level, node->children[0]);
        CountNodesAndLeafsByPlane(++level, node->children[1]);
    }
    else 
    {
        // It's a leaf
        if (node->contents == CONTENTS_EMPTY)
            ++g_numEmptyLeafs;
        else if (node->contents == CONTENTS_SOLID)
            ++g_numSolidLeafs;
    }
}

void CountNodesAndLeafsByChildren(int level, node_t* node)
{
    if (node->children[0])
    {
        if (node->contents == CONTENTS_EMPTY)
            ++g_numEmptyNodes;
        else if (node->contents == CONTENTS_SOLID)
            ++g_numSolidNodes;

        if (node->children[0])
            CountNodesAndLeafsByChildren(++level, node->children[0]);
        if (node->children[1])
            CountNodesAndLeafsByChildren(++level, node->children[1]);
    }
    else
    {
        if (node->contents == CONTENTS_EMPTY)
            ++g_numEmptyLeafs;
        else if (node->contents == CONTENTS_SOLID)
            ++g_numSolidLeafs;
    }
}

void PrintNodeMetricsByPlane(node_t* node) 
{
    g_numEmptyNodes = 0;
    g_numSolidNodes = 0;
    g_numEmptyLeafs = 0;
    g_numSolidLeafs = 0;

    CountNodesAndLeafsByPlane(0, node);

    Verbose("%5i empty nodes\n", g_numEmptyNodes);
    Verbose("%5i solid nodes\n", g_numSolidNodes);
    Verbose("%5i empty leafs\n", g_numEmptyLeafs);
    Verbose("%5i solid leafs\n", g_numSolidLeafs);
}

void PrintNodeMetricsByChildren(node_t* node)
{
    g_numEmptyNodes = 0;
    g_numSolidNodes = 0;
    g_numEmptyLeafs = 0;
    g_numSolidLeafs = 0;

    CountNodesAndLeafsByChildren(0, node);

    Verbose("%5i empty nodes\n", g_numEmptyNodes);
    Verbose("%5i solid nodes\n", g_numSolidNodes);
    Verbose("%5i empty leafs\n", g_numEmptyLeafs);
    Verbose("%5i solid leafs\n", g_numSolidLeafs);
}

void SetAllFacesLeafNode(node_t* leafNode, entity_t* entity)
{
    for (unsigned int brushIndex = 0; brushIndex < entity->numbrushes; ++brushIndex)
    {
        brush_t* brush = entity->brushes[brushIndex];

        for (unsigned int sideIndex = 0; sideIndex < brush->numsides; ++sideIndex)
        {
            side_t* side = brush->brushsides[sideIndex];

            // Set leaf node for main original_face
            if (side->original_face)
                side->original_face->leaf_node = leafNode;

            // Set leaf node for extra original_face
            if (side->final_face)
                side->final_face->leaf_node = leafNode;

            // Set leaf node for normal original_face
            for (face_t* face = side->face_fragments; face; face = face->next)
                face->leaf_node = leafNode;

            // Set leaf node for inverted original_face
            for (face_t* face = side->inverted_face_fragments; face; face = face->next)
                face->leaf_node = leafNode;
        }
    }
}

//FIXME
void CalcInternalNodes_r(node_t* node)
{
    node_t* i; // esi

    for (i = node; i->planenum != -1; i = i->children[1])
        CalcInternalNodes_r(i->children[0]);
    if (!i->occupied)
        i->contents = CONTENTS_SOLID;
}

void MarkUnoccupiedLeafsAsSolid(node_t* node)
{
    CalcInternalNodes_r(node);
}

unsigned int g_PortalLog[MAX_MAP_PORTALS];
unsigned int g_PortalCount = 0;

void AddPortalPlaneToLog(unsigned int clipplane)
{
    //FIXME: nightfire has no hlassume error or even a bounds check for this!
    if (g_PortalCount + 2 <= MAX_MAP_PORTALS)
    {
        g_PortalLog[g_PortalCount++] = clipplane;
        g_PortalLog[g_PortalCount++] = clipplane ^ 1;
        return;
    }

    Error("Error: exceeded MAX_MAP_PORTALS!");
}

bool FindPlaneInPortalPlaneLog(unsigned int clipplane)
{
    for (unsigned int i = 0; i < g_PortalCount; i += 2)
    {
        if (g_PortalLog[i] == clipplane || g_PortalLog[i + 1] == clipplane)
            return true;
    }
    return false;
}

void MakeNodePortal(node_t* node)
{
    Winding* w = new Winding(gMappedPlanes[node->planenum]);

    portal_t* new_portal = new portal_t;
    new_portal->planenum = node->planenum;
    new_portal->onnode = node;

    g_PortalCount = 0;
    unsigned int clipplane = node->planenum;
    int side = SIDE_FRONT;

    for (portal_t* p = node->portals; p; p = p->next[side])
    {
        if (p->nodes[0] == node)
        {
            clipplane = p->planenum;
            side = SIDE_FRONT;
        }
        else if (p->nodes[1] == node)
        {
            clipplane = p->planenum ^ 1;
            side = SIDE_BACK;
        }
        else
        {
            Error("MakeNodePortal: mislinked portal");
        }

        if (FindPlaneInPortalPlaneLog(clipplane))
            continue;

        if (node->planenum == clipplane || node->planenum == (clipplane ^ 1))
        {
            AddPortalPlaneToLog(clipplane);
            continue;
        }

        w->Clip(gMappedPlanes[clipplane], true);

        if (!w->HasPoints())
        {
            Warning("MakeNodePortal: new portal was clipped away from node @ (%.0f,%.0f,%.0f)-(%.0f,%.0f,%.0f)", node->mins[0], node->mins[1], node->mins[2], node->maxs[0], node->maxs[1], node->maxs[2]);
            delete new_portal;
            delete w;
            return;
        }
       
        AddPortalPlaneToLog(clipplane);
    }

    new_portal->winding = w;
    AddPortalToNodes(new_portal, node->children[0], node->children[1]);
}

void AddPortalToNodes(portal_t* p, node_t* front, node_t* back)
{
    if (p->nodes[0] || p->nodes[1])
        Error("AddPortalToNode: allready included");

    p->nodes[0] = front;
    p->next[0] = front->portals;
    front->portals = p;

    p->nodes[1] = back;
    p->next[1] = back->portals;
    back->portals = p;
}

unsigned int CalcNodePlane(node_t* node, int bsp_depth, face_t* face)
{
    //if (g_MaxNodeSize != 1024)
    //    int islightingtree = 1;

    int axis = 0;
    vec3_t bounds;
    VectorSubtract(node->maxs, node->mins, bounds);

    while (bounds[axis] <= (double)g_MaxNodeSize)
    {
        if (++axis >= 3)
            return ChoosePlaneFromList(node, face);
    }

    return ChooseMidPlaneFromList(node, axis);
}

void SplitNodePortals(node_t* node)
{
    portal_t* p;
    portal_t* next_portal;
    portal_t* new_portal;
    node_t* f;
    node_t* b;
    node_t* other_node;
    int             side = 0;
    plane_t* plane;
    Winding* frontwinding;
    Winding* backwinding;

    plane = &gMappedPlanes[node->planenum];
    f = node->children[0];
    b = node->children[1];

    for (p = node->portals; p; p = next_portal)
    {
        if (p->nodes[0] == node)
        {
            side = 0;
        }
        else if (p->nodes[1] == node)
        {
            side = 1;
        }
        else
        {
            Error("SplitNodePortals: mislinked portal");
        }
        next_portal = p->next[side];

        other_node = p->nodes[!side];
        RemovePortalFromNode(p, p->nodes[0]);
        RemovePortalFromNode(p, p->nodes[1]);

        // cut the portal into two portals, one on each side of the cut plane
        //p->winding->Divide(*plane, &frontwinding, &backwinding);
        //NIGHTFIRE FIXME: CALLING WINDING->CLIP
        p->winding->Clip(*plane, &frontwinding, &backwinding);

        if (!frontwinding)
        {
            if (backwinding)
                delete backwinding;
            backwinding = nullptr;

            if (side == 0)
            {
                AddPortalToNodes(p, b, other_node);
            }
            else
            {
                AddPortalToNodes(p, other_node, b);
            }
            continue;
        }

        if (!backwinding)
        {
            if (frontwinding)
                delete frontwinding;
            frontwinding = nullptr;

            if (side == 0)
            {
                AddPortalToNodes(p, f, other_node);
            }
            else
            {
                AddPortalToNodes(p, other_node, f);
            }
            continue;
        }

        // the winding is split
        new_portal = new portal_t;
        *new_portal = *p;
        new_portal->winding = backwinding;
        delete p->winding;
        p->winding = frontwinding;

        if (side == 0)
        {
            AddPortalToNodes(p, f, other_node);
            AddPortalToNodes(new_portal, b, other_node);
        }
        else
        {
            AddPortalToNodes(p, other_node, f);
            AddPortalToNodes(new_portal, other_node, b);
        }
    }

    node->portals = NULL;
}
