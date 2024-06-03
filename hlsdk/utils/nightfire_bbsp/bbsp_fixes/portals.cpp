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

void WriteLeakTrailPoint(const vec3_t point1, const vec3_t point2)
{
    fprintf(linefile, "%f %f %f - %f %f %f\n", point1[0], point1[1], point1[2], point2[0], point2[1], point2[2]);
    fprintf(pointfile, "%f %f %f\n", point1[0], point1[1], point1[2]);
}

void MarkLeakTrail(portal_t* n2)
{
    portal_t* n1 = prevleaknode;
    vec3_t p2, p1, dir;

    prevleaknode = n2;

    if (!n2)
    {
        if (!n1)
            return;
        entity_t* ent = (g_EntInfo)->entities[g_CurrentEntity];
        const char* entityOriginStr = ValueForKey(ent, "origin");
        vec3_t origin;
        if (!strlen(entityOriginStr) || sscanf(entityOriginStr, "%lf %lf %lf", &origin[0], &origin[1], &origin[2]) != 3)
            return;

        n1->winding->getCenter(p1);
        WriteLeakTrailPoint(origin, p1);
        return;
    }

    double area = n2->winding->getArea();
    Developer(DEVELOPER_LEVEL_SPAM, "Flowing through portal %p : area %f : \n", n2, area);

    for (unsigned pointIndex = 0; pointIndex < n2->winding->m_NumPoints; ++pointIndex)
    {
        Developer(DEVELOPER_LEVEL_SPAM, "(%f %f %f) ", n2->winding->m_Points[pointIndex][0], n2->winding->m_Points[pointIndex][1], n2->winding->m_Points[pointIndex][2]);
    }

    Developer(DEVELOPER_LEVEL_SPAM, "\n");

    if (n1)
    {
        n2->winding->getCenter(p2);
        n1->winding->getCenter(p1);

        VectorSubtract(p2, p1, dir);
        vec_t len = VectorLength(dir);
        VectorNormalize(dir);

        while (len > 2)
        {
            fprintf(pointfile, "%f %f %f\n", p1[0], p1[1], p1[2]);
            for (int i = 0; i < 3; i++)
                p1[i] += dir[i] * 2;
            len -= 2;
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