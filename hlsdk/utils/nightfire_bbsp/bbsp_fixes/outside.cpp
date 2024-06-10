#include "outside.h"
#include "globals.h"
#include "bspfile.h"
#include "log.h"
#include "Leaf.h"
#include "Nodes.h"
#include "face.h"
#include "portals.h"

node_t* PointInLeaf(node_t* node, const vec3_t point)
{
    if (node->planenum == PLANENUM_LEAF)
        return node;

    const plane_t& plane = gMappedPlanes[node->planenum];
    vec_t d = DotProduct(plane.normal, point) - plane.dist;

    if (d > 0)
        return PointInLeaf(node->children[0], point);

    return PointInLeaf(node->children[1], point);
}

#if 0
int      valid = 0;

bool RecursiveFillOutside(node_t* l, const bool fill)
{
    portal_t* p;
    int             s;

    if (l->contents == CONTENTS_SOLID)
        return false;

    if (l->valid == valid)
    {
        return false;
    }

    if (l->occupied)
    {
        g_LeakEntity = l->occupied;
        return true;
    }

    l->valid = valid;

    // fill it and it's neighbors
    if (fill)
    {
        l->contents = CONTENTS_SOLID;
        l->planenum = -1;
    }

    for (p = l->portals; p;)
    {
        s = (p->nodes[0] == l);

        if (RecursiveFillOutside(p->nodes[s], fill))
        {                                                  // leaked, so stop filling
            MarkLeakTrail(p);
            return true;
        }
        p = p->next[!s];
    }

    return false;
}

bool PlaceOccupant(const vec3_t point, node_t* node, unsigned int entindex)
{
    node_t* n = PointInLeaf(node, point);

    if (n->contents == CONTENTS_SOLID)
        return false;

    node->occupied = entindex;
    return true;
}

node_t* FillOutside(mapinfo_t* mapfile, node_t* node, int pass_num)
{
    g_EntInfo = mapfile;
    g_bLeaked = false;
    bool inside = false;

    Verbose("----- FillOutside ----\n");

    pointfile = fopen(g_pointfilename, "w");
    if (!pointfile)
        Error("Couldn't open pointfile '%s'\n", g_pointfilename);
    linefile = fopen(g_linefilename, "w");
    if (!linefile)
        Error("Couldn't open linefile '%s'\n", g_linefilename);

    PrintLeafMetrics(node, "Original tree");

    for (unsigned int i = 1; i < mapfile->numentities; ++i)
    {
        entity_t* entity = mapfile->entities[i];
        vec_t origin[3];
        GetVectorForKey(entity, "origin", origin);
        const char* cl = ValueForKey(entity, "classname");

        if (!entity->numbrushes)
        {
            // Check if origin is close to (0, 0, 0)
            //FIXME: zhlt in hl1 had a fix for this!  if (*ValueForKey(&g_entities[i], "origin")) //--vluzacn
            if (!VectorCompare(origin, vec3_origin))
            {
                origin[2] += 1;                            // so objects on floor are ok

                // nudge playerstart around if needed so clipping hulls always have a valid point
                if (!strcmp(cl, "info_player_start"))
                {
                    int             x, y;

                    for (x = -16; x <= 16; x += 16)
                    {
                        for (y = -16; y <= 16; y += 16)
                        {
                            origin[0] += x;
                            origin[1] += y;
                            if (PlaceOccupant(origin, node, i))
                            {
                                inside = true;
                                goto gotit;
                            }
                            origin[0] -= x;
                            origin[1] -= y;
                        }
                    }
                gotit:;
                }
                else
                {
                    if (PlaceOccupant(origin, node, i))
                        inside = true;
                }
            }
        }
    }

    if (!inside)
    {
        Warning("No entities exist in world, no filling performed");
        fclose(pointfile);
        fclose(linefile);
        pointfile = NULL;
        linefile = NULL;
        _unlink(g_pointfilename);
        _unlink(g_linefilename);
        return node;
    }

    if (!g_OutsideNode->portals)
    {
        Warning("No outside node portal found in world, no filling performed");
        fclose(pointfile);
        fclose(linefile);
        pointfile = NULL;
        linefile = NULL;
        _unlink(g_pointfilename);
        _unlink(g_linefilename);
        return node;
    }

    int s = !(g_OutsideNode->portals->nodes[1] == g_OutsideNode);
    prevleaknode = NULL;
    valid++;
    bool ret = RecursiveFillOutside(g_OutsideNode->portals->nodes[s], false);

    fclose(pointfile);
    fclose(linefile);
    pointfile = NULL;
    linefile = NULL;

    if (ret)
    {
        vec3_t origin;
        GetVectorForKey(mapfile->entities[g_LeakEntity], "origin", origin);
        const char* val_for_key = ValueForKey(mapfile->entities[g_LeakEntity], "classname");
        Warning("=== LEAK (pass %d) ===\nEntity %s @ (%4.0f,%4.0f,%4.0f)", pass_num, val_for_key, origin[0], origin[1], origin[2]);
        PrintOnce("\n"
            "  A LEAK is a hole in the map, where the inside of it is exposed to the\n"
            "(unwanted) outside region.  The entity listed in the error is just a helpful\n"
            "indication of where the beginning of the leak pointfile starts, so the\n"
            "beginning of the line can be quickly found and traced to until reaching the\n"
            "outside. Unless this entity is accidentally on the outside of the map, it\n"
            "probably should not be deleted.  Some complex rotating objects entities need\n"
            "their origins outside the map.  To deal with these, just enclose the origin\n"
            "brush with a solid world brush\n");
        if (!g_bLeaked)
            Log("Leak pointfile generated\n\n");
        if (g_bLeakOnly)
            Error("Stopped by leak.");
        g_bLeaked = true;

        return node;
    }
    
    ret = RecursiveFillOutside(g_OutsideNode->portals->nodes[s], true);

    MarkUnoccupiedLeafsAsSolid(node);
    PrintLeafMetrics(node, "fill");

    node = ClearOutFaces(node);
    PrintLeafMetrics(node, "final");

    return node;
}
#else

bool PlaceOccupant(const vec3_t point, node_t* node, unsigned int entindex)
{
    node_t* n = PointInLeaf(node, point);
    MarkLeafOccupancyAndCheckLeaks(n, entindex);
    if (g_bLeaked)
    {
        g_LeakEntity = entindex;
        MarkLeakTrail(nullptr);
    }
    return true;
}

node_t* FillOutside(mapinfo_t* mapfile, node_t* node, int pass_num)
{
    g_EntInfo = mapfile;
    g_bLeaked = false;
    bool inside = false;

    Verbose("----- FillOutside ----\n");
    pointfile = fopen(g_pointfilename, "w");
    if (!pointfile)
        Error("Couldn't open pointfile '%s'\n", g_pointfilename);
    linefile = fopen(g_linefilename, "w");
    if (!linefile)
        Error("Couldn't open linefile '%s'\n", g_linefilename);

    PrintLeafMetrics(node, "Original tree");

    for (unsigned int i = 1; i < mapfile->numentities; ++i)
    {
        if (g_bLeaked)
            break;

        entity_t* entity = mapfile->entities[i];
        vec_t origin[3];
        GetVectorForKey(entity, "origin", origin);
        const char* cl = ValueForKey(entity, "classname");

        if (!entity->numbrushes)
        {
            // Check if origin is close to (0, 0, 0)
            //FIXME: zhlt in hl1 had a fix for this!  if (*ValueForKey(&g_entities[i], "origin")) //--vluzacn
            if (!VectorCompare(origin, vec3_origin))
            {
                origin[2] += 1;                            // so objects on floor are ok

                // nudge playerstart around if needed so clipping hulls always have a valid point
                if (!strcmp(cl, "info_player_start"))
                {
                    int             x, y;

                    for (x = -16; x <= 16; x += 16)
                    {
                        for (y = -16; y <= 16; y += 16)
                        {
                            origin[0] += x;
                            origin[1] += y;
                            if (PlaceOccupant(origin, node, i))
                            {
                                inside = true;
                                goto gotit;
                            }
                            origin[0] -= x;
                            origin[1] -= y;
                        }
                    }
                gotit:;
                }
                else
                {
                    if (PlaceOccupant(origin, node, i))
                        inside = true;
                }
            }
        }
    }

    if (inside)
    {
        PrintLeakInfoIfLeaked(mapfile, pass_num);
        fclose(pointfile);
        fclose(linefile);
        pointfile = NULL;
        linefile = NULL;
        if (!g_bLeaked)
        {
            MarkUnoccupiedLeafsAsSolid(node);
            PrintLeafMetrics(node, "fill");
            node = ClearOutFaces(node);
        }
        PrintLeafMetrics(node, "final");
    }
    else
    {
        Warning("No entities exist in world, no filling performed");
        fclose(pointfile);
        fclose(linefile);
        pointfile = NULL;
        linefile = NULL;
        _unlink(g_pointfilename);
        _unlink(g_linefilename);
    }

    return node;
}

void MarkLeafOccupancyAndCheckLeaks(node_t* node, unsigned int occupancyValue)
{
    if (node == g_OutsideNode)
    {
        g_bLeaked = true;
        return;
    }

    if (node->contents == CONTENTS_SOLID)
        return;

    if (node->occupied)
        return;

    node->occupied = occupancyValue;

    for (portal_t* p = node->portals; p;)
    {
        int s = (p->nodes[0] == node);

        MarkLeafOccupancyAndCheckLeaks(p->nodes[s], occupancyValue);

        if (g_bLeaked)
        {
            MarkLeakTrail(p);
            return;
        }

        p = p->next[!s];
    }
}

void PrintLeakInfoIfLeaked(mapinfo_t* mapfile, int pass_num)
{
    if (g_bLeaked)
    {
        vec3_t origin;
        GetVectorForKey(mapfile->entities[g_LeakEntity], "origin", origin);
        const char* val_for_key = ValueForKey(mapfile->entities[g_LeakEntity], "classname");
        Warning("=== LEAK (pass %d) ===\nEntity %s @ (%4.0f,%4.0f,%4.0f)", pass_num, val_for_key, origin[0], origin[1], origin[2]);
        PrintOnce("\n"
            "  A LEAK is a hole in the map, where the inside of it is exposed to the\n"
            "(unwanted) outside region.  The entity listed in the error is just a helpful\n"
            "indication of where the beginning of the leak pointfile starts, so the\n"
            "beginning of the line can be quickly found and traced to until reaching the\n"
            "outside. Unless this entity is accidentally on the outside of the map, it\n"
            "probably should not be deleted.  Some complex rotating objects entities need\n"
            "their origins outside the map.  To deal with these, just enclose the origin\n"
            "brush with a solid world brush\n");
        if (!g_bLeaked) //???
            Log("Leak pointfile generated\n\n");
        if (g_bLeakOnly)
            Error("Stopped by leak.");
    }
}

#endif