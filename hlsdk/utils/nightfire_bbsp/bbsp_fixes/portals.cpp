#include <stdio.h>
#include <math.h>
#include "helpers.h"
#include "winding.h"
#include "bsp_structs.h"
#include "face.h"
#include "globals.h"
#include "Leaf.h"
#include "textures.h"
#include "portals.h"
#include "bspfile.h"
#include "log.h"
#include "planes.h"
#include "Nodes.h"

void PrintPortalConnection(vec3_t start, vec3_t end)
{
    fprintf(linefile, "%f %f %f - %f %f %f\n",
        start[0], start[1], start[2],
        end[0], end[1], end[2]);

    fprintf(pointfile, "%f %f %f\n", start[0], start[1], start[2]);

    vec3_t direction;
    VectorSubtract(end, start, direction);

    vec_t length = VectorLength(direction);
    VectorNormalize(direction);

    while (length > 2.0)
    {
        fprintf(pointfile, "%f %f %f\n", start[0], start[1], start[2]);

        for (int i = 0; i < 3; ++i)
            start[i] += direction[i] * 2;

        length -= 2.0;
    }
}

void MarkLeakTrail(portal_t* portal)
{
    portal_t* previousPortal = prevleaknode;
    prevleaknode = portal;

    if (!portal)
    {
        if (!previousPortal)
            return;

        vec3_t entityOrigin;
        const char* entityOriginStr = ValueForKey(g_EntInfo->entities[g_LeakEntity], "origin");

        if (!strlen(entityOriginStr) || sscanf(entityOriginStr, "%lf %lf %lf", &entityOrigin[0], &entityOrigin[1], &entityOrigin[2]) != 3)
            return;

        vec3_t centerPreviousPortal;
        previousPortal->winding->getCenter(centerPreviousPortal);

        PrintPortalConnection(entityOrigin, centerPreviousPortal);
        return;
    }

    const Winding* winding = portal->winding;
    Developer(DEVELOPER_LEVEL_SPAM, "Flowing through portal %p : area %f : \n", portal, winding->getArea());

    for (unsigned int i = 0; i < winding->m_NumPoints; ++i)
    {
        Developer(DEVELOPER_LEVEL_SPAM, "(%f %f %f) ", winding->m_Points[i][0], winding->m_Points[i][1], winding->m_Points[i][2]);
    }
    Developer(DEVELOPER_LEVEL_SPAM, "\n");

    if (previousPortal)
    {
        vec3_t centerCurrentPortal;
        vec3_t centerPreviousPortal;

        winding->getCenter(centerCurrentPortal);
        previousPortal->winding->getCenter(centerPreviousPortal);

        PrintPortalConnection(centerCurrentPortal, centerPreviousPortal);
    }
}

void RemovePortalFromNode(portal_t* portal, node_t* l)
{
    portal_t** pp;
    portal_t* t;

    // remove reference to the current portal
    pp = &l->portals;
    while (1)
    {
        t = *pp;
        if (!t)
        {
            Error("RemovePortalFromNode: portal not in leaf");
        }

        if (t == portal)
        {
            break;
        }

        if (t->nodes[0] == l)
        {
            pp = &t->next[0];
        }
        else if (t->nodes[1] == l)
        {
            pp = &t->next[1];
        }
        else
        {
            Error("RemovePortalFromNode: portal not bounding leaf");
        }
    }

    if (portal->nodes[0] == l)
    {
        *pp = portal->next[0];
        portal->nodes[0] = NULL;
    }
    else if (portal->nodes[1] == l)
    {
        *pp = portal->next[1];
        portal->nodes[1] = NULL;
    }
}

void WritePortalFile_r(int depth, node_t* node)
{
    if (!node)
        return;

    // Traverse the BSP tree until a leaf node is reached
    if (node->planenum != PLANENUM_LEAF)
    {
        WritePortalFile_r(++depth, node->children[0]);
        WritePortalFile_r(++depth, node->children[1]);
    }

    //if (node->contents == CONTENTS_SOLID)
        //return;

    for (portal_t* p = node->portals; p;)
    {
        Winding* w = p->winding;
        if (w->HasPoints() && p->nodes[0] == node)
        {
            if (p->nodes[0]->contents != CONTENTS_SOLID && p->nodes[1]->contents != CONTENTS_SOLID)
            {
                // write out to the file

                // sometimes planes get turned around when they are very near
                // the changeover point between different axis.  interpret the
                // plane the same way vis will, and flip the side orders if needed
                plane_t plane2;
                p->winding->getPlane(plane2);

                if (DotProduct(gMappedPlanes[p->planenum].normal, plane2.normal) < 1.0 - ON_EPSILON)
                {
                    // backwards...
                    fprintf(portalfile, "%i %u %i %i ", p->planenum ^ 1, w->m_NumPoints, p->nodes[1]->visleafnum, p->nodes[0]->visleafnum);
                }
                else
                {
                    fprintf(portalfile, "%i %u %i %i ", p->planenum, w->m_NumPoints, p->nodes[0]->visleafnum, p->nodes[1]->visleafnum);
                }
                for (unsigned int i = 0; i < w->m_NumPoints; ++i)
                {
                    fprintf(portalfile, "(%f %f %f) ", w->m_Points[i][0], w->m_Points[i][1], w->m_Points[i][2]);
                }
                fprintf(portalfile, "\n");
            }
        }
        if (p->nodes[0] == node)
            p = p->next[0];
        else
            p = p->next[1];
    }
}

void WritePortalfile(node_t* node)
{
    num_visleafs = 0;
    num_visportals = 0;
    NumberLeafs_r(0, node);

    portalfile = fopen(g_portfilename, "w");
    if (!portalfile)
    {
        Error("Error writing portal file %s", g_portfilename);
        return;
    }

    fprintf(portalfile, "ZPRT 1\n");
    fprintf(portalfile, "%i %i\n", num_visleafs, num_visportals);
    WritePortalFile_r(0, node);

    fclose(portalfile);
    Log("Writing portal file '%s'\n", g_portfilename);
}

void MakeHeadnodePortals(node_t* node)
{
    vec3_t bounds[2];

    // pad with some space so there will never be null volume leafs
    for (int i = 0; i < 3; ++i)
    {
        bounds[0][i] = node->mins[i] - SIDESPACE;
        bounds[1][i] = node->maxs[i] + SIDESPACE;
    }

    // Create a solid leaf node
    g_OutsideNode = new node_t;
    g_OutsideNode->contents = CONTENTS_SOLID;
    g_OutsideNode->planenum = PLANENUM_LEAF;

    // Create p
    portal_t* portals[6];
    unsigned int planes[6];

    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 2; ++j)
        {
            int n = j * 3 + i;

            vec3_t normal{};
            vec_t dist;
            if (j)
            {
                normal[i] = -1;
                dist = -bounds[j][i];
            }
            else
            {
                normal[i] = 1;
                dist = bounds[j][i];
            }

            unsigned int plane_num = FindIntPlane(normal, dist);
            plane_t* plane = &gMappedPlanes[plane_num];

            portal_t* portal = new portal_t;
            portal->planenum = plane_num;
            portal->winding = new Winding(*plane);
            portals[n] = portal;
            planes[n] = plane_num;

            // Add portal to nodes
            AddPortalToNodes(portal, node, g_OutsideNode);
        }
    }

    // clip the basewindings by all the other planes
    for (int i = 0; i < 6; ++i)
    {
        for (int j = 0; j < 6; ++j)
        {
            if (j == i)
                continue;

            portals[i]->winding->Clip(gMappedPlanes[planes[j]], true);
        }
    }
}