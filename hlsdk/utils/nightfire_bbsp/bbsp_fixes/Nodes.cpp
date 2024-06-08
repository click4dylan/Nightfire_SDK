
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

void WriteDrawNodes_r(int depth, const node_t* const node)
{
    dnode_t* n;
    int             i;
    face_t* f;

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
        if (node->children[i]->planenum == -1)
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
        if (node->leaf_type == LEAF_EMPTY_AKA_NOT_OPAQUE)
           ++g_numEmptyNodes;
        else if (node->leaf_type == LEAF_SOLID_AKA_OPAQUE)
            ++g_numSolidNodes;
        CountNodesAndLeafsByPlane(++level, node->children[0]);
        CountNodesAndLeafsByPlane(++level, node->children[1]);
    }
    else 
    {
        // It's a leaf
        if (node->leaf_type == LEAF_EMPTY_AKA_NOT_OPAQUE)
            ++g_numEmptyLeafs;
        else if (node->leaf_type == LEAF_SOLID_AKA_OPAQUE)
            ++g_numSolidLeafs;
    }
}

void CountNodesAndLeafsByChildren(int level, node_t* node)
{
    if (node->children[0])
    {
        if (node->leaf_type == LEAF_EMPTY_AKA_NOT_OPAQUE)
            ++g_numEmptyNodes;
        else if (node->leaf_type == LEAF_SOLID_AKA_OPAQUE)
            ++g_numSolidNodes;

        if (node->children[0])
            CountNodesAndLeafsByChildren(++level, node->children[0]);
        if (node->children[1])
            CountNodesAndLeafsByChildren(++level, node->children[1]);
    }
    else
    {
        if (node->leaf_type == LEAF_EMPTY_AKA_NOT_OPAQUE)
            ++g_numEmptyLeafs;
        else if (node->leaf_type == LEAF_SOLID_AKA_OPAQUE)
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

void CalcInternalNodes_r(node_t* node)
{
    for (node_t* i = node; i->planenum != -1; i = i->children[1])
    {
        CalcInternalNodes_r(i->children[0]);
    }

    if (!node->occupied)
    {
        node->leaf_type = LEAF_SOLID_AKA_OPAQUE;
    }
}

void CalcInternalNodes(node_t* node)
{
    CalcInternalNodes_r(node);
}

unsigned int g_PortalLog[MAX_MAP_PORTALS];

void MakeNodePortal(node_t* node)
{
    portal_t* new_portal;
    portal_t* p;
    plane_t* plane;
    unsigned int clipplane;
    Winding* w;
    int side = 0;

    plane = &gMappedPlanes[node->planenum];

    // Initialize new winding for the node's plane
    w = new Winding(*plane);

    new_portal = new portal_t;
    new_portal->planenum = node->planenum;
    new_portal->onnode = node;

    unsigned int portalCount = 0;

    for (p = node->portals; p; p = p->next[side])
    {
        clipplane = p->planenum;
        if (p->nodes[0] == node)
        {
            clipplane = p->planenum;
            side = 0;
        }
        else if (p->nodes[1] == node)
        {
            clipplane = p->planenum ^ 1;
            side = 1;
        }
        else
        {
            Error("MakeNodePortal: mislinked portal");
        }

        // Check if the portal plane has already been processed
        bool is_new_plane = true;
        for (unsigned int i = 0; i < portalCount; i += 2)
        {
            if (clipplane == g_PortalLog[i] || clipplane == g_PortalLog[i + 1])
            {
                is_new_plane = false;
                break;
            }
        }

        if (is_new_plane)
        {
            // Clip the portal with the current plane
            w->Clip(gMappedPlanes[clipplane], true);

            // Check if the resulting winding is valid
            if (w->Valid())
            {
                // Add the processed plane to the portal log
                //FIXME: hlassume doesnt exist for this. nightfire had no count check!
                if (portalCount < MAX_MAP_PORTALS) // Assuming there's a defined max size
                {
                    g_PortalLog[portalCount++] = clipplane;
                    g_PortalLog[portalCount++] = clipplane ^ 1;
                }
                else
                {
                    Error("MakeNodePortal: portal log exceeded capacity");
                    delete new_portal;
                    delete w; // Clean up memory
                    return;
                }
            }
            else
            {
                // Print a warning if the portal was clipped away
                Warning("MakeNodePortal: new portal was clipped away from node @ (%.0f,%.0f,%.0f)-(%.0f,%.0f,%.0f)",
                    node->mins[0], node->mins[1], node->mins[2],
                    node->maxs[0], node->maxs[1], node->maxs[2]);
                // Free the portal memory
                delete new_portal;
                delete w; // Clean up memory
                return;
            }
        }
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
    int maxsize = g_MaxNodeSize;
    if (maxsize != 1024)
        int islightingtree = 1;

    int axis; // esi
    vec3_t bounds; // [esp+10h] [ebp-1Ch]

    axis = 0;
    bounds[0] = node->maxs[0] - node->mins[0];
    bounds[1] = node->maxs[1] - node->mins[1];
    bounds[2] = node->maxs[2] - node->mins[2];

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
