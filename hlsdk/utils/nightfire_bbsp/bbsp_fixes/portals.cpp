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

//TODO: FIXME: this is broken
void MarkLeakTrail(portal_t* portal)
{
    portal_t* n1 = prevleaknode;
    portal_t* previousPortal = prevleaknode;
    prevleaknode = portal;

    if (!portal)
    {
        if (!previousPortal)
            return;

        vec3_t entityOrigin;
        const char* entityOriginStr = ValueForKey(g_EntInfo->entities[g_CurrentEntity], "origin");

        if (!strlen(entityOriginStr) || sscanf(entityOriginStr, "%lf %lf %lf", &entityOrigin[0], &entityOrigin[1], &entityOrigin[2]) != 3)
            return;

        vec3_t centerPreviousPortal;
        previousPortal->winding->getCenter(centerPreviousPortal);

        fprintf(linefile, "%f %f %f - %f %f %f\n",
            entityOrigin[0], entityOrigin[1], entityOrigin[2],
            centerPreviousPortal[0], centerPreviousPortal[1], centerPreviousPortal[2]);

        fprintf(pointfile, "%f %f %f\n", entityOrigin[0], entityOrigin[1], entityOrigin[2]);

        vec3_t direction;
        for (int i = 0; i < 3; ++i)
            direction[i] = centerPreviousPortal[i] - entityOrigin[i];

        double length = VectorLength(direction);
        VectorNormalize(direction);

        while (length > 2.0)
        {
            for (int i = 0; i < 3; ++i)
                entityOrigin[i] += direction[i] * 2;

            fprintf(pointfile, "%f %f %f\n", entityOrigin[0], entityOrigin[1], entityOrigin[2]);
            length -= 2.0;
        }
        return;
    }

    Winding* winding = portal->winding;
    double area = winding->getArea();
    Developer(DEVELOPER_LEVEL_SPAM, "Flowing through portal %p : area %f : \n", portal, area);

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

        fprintf(linefile, "%f %f %f - %f %f %f\n",
            centerCurrentPortal[0], centerCurrentPortal[1], centerCurrentPortal[2],
            centerPreviousPortal[0], centerPreviousPortal[1], centerPreviousPortal[2]);

        fprintf(pointfile, "%f %f %f\n", centerCurrentPortal[0], centerCurrentPortal[1], centerCurrentPortal[2]);

        vec3_t direction;
        for (int i = 0; i < 3; ++i)
            direction[i] = centerPreviousPortal[i] - centerCurrentPortal[i];

        double length = VectorLength(direction);
        VectorNormalize(direction);

        while (length > 2.0)
        {
            for (int i = 0; i < 3; ++i)
                centerCurrentPortal[i] += direction[i] * 2;

            fprintf(pointfile, "%f %f %f\n", centerCurrentPortal[0], centerCurrentPortal[1], centerCurrentPortal[2]);
            length -= 2.0;
        }
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