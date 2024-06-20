#include "brad.h"
#include "cmdlib.h"
//#include "mathlib.h"
#include "bspfile.h"
#include "log.h"
#include "helpers.h"
#include "globals.h"
#include "filelib.h"

// #define      ON_EPSILON      0.001

tnode_t* tnodes{};
static tnode_t* tnode_p;

/*
 * ==============
 * MakeTnode
 * 
 * Converts the disk node structure into the efficient tracing structure
 * ==============
 */
#if 0
static void     MakeTnode(const int nodenum)
{
    tnode_t*        t;
    dplane_t*       plane;
    int             i;
    dnode_t*        node;

    t = tnode_p++;

    node = g_dnodes + nodenum;
    plane = g_dplanes + node->planenum;

    t->plane.closest_axis = plane->closest_axis;
    VectorCopy(plane->normal, t->plane.normal);
    t->plane.dist = plane->dist;

    for (i = 0; i < 2; i++)
    {
        if (node->children[i] < 0)
            t->children[i] = (tnode_s*)g_dleafs[-node->children[i] - 1].contents;
        else
        {
            t->children[i] = tnode_p - tnodes;
            MakeTnode(node->children[i]);
        }
    }

}
#endif

/*
 * =============
 * MakeTnodes
 * 
 * Loads the node structure out of a .bsp file to be used for light occlusion
 * =============
 */
void            MakeTnodes()
{
    char filepath[MAX_PATH]; // File path for .lbsp file
    char* filedata; // Content of the loaded file

    // Format the file path for the .lbsp file
    safe_snprintf(filepath, sizeof(filepath), "%s.lbsp", g_Mapname);

    // Load the file content
    LoadFile(filepath, (void**)&filedata);

    // Get the first token (number of TNodes)
    const char* token = strtok(filedata, " \r\n\t");
    if (!token)
        Error("MakeTnodes: Damaged or invalid .lbsp file\n");

    // Parse the number of TNodes
    int numTNodes = atol(token);

    // Allocate memory for the TNodes
    tnodes = (tnode_t*)calloc(1, sizeof(tnode_s) * numTNodes);

    if (!tnodes)
        Error("MakeTnodes: Failed to allocate memory for TNodes\n");

    // Loop through each TNode
    for (int i = 0; i < numTNodes; i++)
    {
        tnode_t* currentTNode = &tnodes[i];

        // Get the next token
        token = strtok(NULL, " \r\n\t");
        if (!token)
            Error("MakeTnodes: Damaged or invalid .lbsp file\n");

        // Check if the token is "NODE"
        if (!_stricmp(token, "NODE"))
        {
            // Skip the identifier
            if (!strtok(NULL, " \r\n\t"))
                Error("MakeTnodes: Damaged or invalid .lbsp file\n");

            // Get the plane number
            token = strtok(NULL, " \r\n\t");
            if (!token)
                Error("MakeTnodes: Damaged or invalid .lbsp file\n");
            currentTNode->planenum = atol(token);

            // Copy the plane data to the TNode
            memcpy(&currentTNode->plane, &g_dplanes[currentTNode->planenum], sizeof(dplane_t));

            // Get the index of the first child
            token = strtok(NULL, " \r\n\t");
            if (!token)
                Error("MakeTnodes: Damaged or invalid .lbsp file\n");
            currentTNode->index_to_children[0] = atol(token);

            // Get the index of the second child
            token = strtok(NULL, " \r\n\t");
            if (!token)
                Error("MakeTnodes: Damaged or invalid .lbsp file\n");
            currentTNode->index_to_children[1] = atol(token);

            // Set the child pointers
            currentTNode->children[0] = &tnodes[currentTNode->index_to_children[0]];
            currentTNode->children[1] = &tnodes[currentTNode->index_to_children[1]];
        }
        // Check if the token is "LEAF"
        else if (!_stricmp(token, "LEAF"))
        {
            // Set the planenum to -1 to indicate a leaf node
            currentTNode->planenum = -1;

            // Skip the identifier
            if (!strtok(NULL, " \r\n\t"))
                Error("MakeTnodes: Damaged or invalid .lbsp file\n");

            // Get the contents of the leaf node
            token = strtok(NULL, " \r\n\t");
            if (!token)
                Error("MakeTnodes: Damaged or invalid .lbsp file\n");
            currentTNode->children[0] = (tnode_s*)atol(token);
        }
        else
        {
            Error("MakeTnodes: Unknown token in .lbsp file\n");
        }
    }

    // Free the loaded file
    free(filedata);
}

//==========================================================

tnode_s*             TestLine_r(const struct tnode_s* node, const vec3_t start, const vec3_t stop)
{
    tnode_t*        tnode;
    float           front, back;
    vec3_t          mid;
    float           frac;
    int             side;
    tnode_s*        r;

    if (node->planenum == -1)
        return node->children[0];

    switch (tnode->plane.closest_axis)
    {
    case plane_x:
        front = start[0] - tnode->plane.dist;
        back = stop[0] - tnode->plane.dist;
        break;
    case plane_y:
        front = start[1] - tnode->plane.dist;
        back = stop[1] - tnode->plane.dist;
        break;
    case plane_z:
        front = start[2] - tnode->plane.dist;
        back = stop[2] - tnode->plane.dist;
        break;
    default:
        front = (start[0] * tnode->plane.normal[0] + start[1] * tnode->plane.normal[1] + start[2] * tnode->plane.normal[2]) - tnode->plane.dist;
        back = (stop[0] * tnode->plane.normal[0] + stop[1] * tnode->plane.normal[1] + stop[2] * tnode->plane.normal[2]) - tnode->plane.dist;
        break;
    }

    if (front >= -ON_EPSILON && back >= -ON_EPSILON)
        return TestLine_r(tnode->children[0], start, stop);

    if (front < ON_EPSILON && back < ON_EPSILON)
        return TestLine_r(tnode->children[1], start, stop);

    side = front < 0;

    frac = front / (front - back);

    mid[0] = start[0] + (stop[0] - start[0]) * frac;
    mid[1] = start[1] + (stop[1] - start[1]) * frac;
    mid[2] = start[2] + (stop[2] - start[2]) * frac;

    r = TestLine_r(tnode->children[side], start, mid);
    if (((unsigned int)r & 0xFF) != CONTENTS_EMPTY)
        return r;
    return TestLine_r(tnode->children[!side], mid, stop);
}

tnode_s* TestLine(const vec3_t start, const vec3_t stop)
{
    return TestLine_r(0, start, stop);
}

void FreeTnodes_r(tnode_t* node)
{
    // Traverse down to the rightmost node
    while (node->planenum != -1)
    {
        // Recursively free the left child
        FreeTnodes_r(node->children[0]);
        node = node->children[1];
    }

    // Free the nodes starting from the rightmost child
    while (node)
    {
        tnode_t* next = (tnode_t*)node->planenum;
        free(node);
        node = next;
    }
}

void FreeTnodes()
{
    FreeTnodes_r(tnodes);
}