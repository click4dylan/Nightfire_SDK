
#include <stdio.h>
#include <math.h>
#include "helpers.h"
#include "winding.h"
#include "bsp_structs.h"
#include "face.h"
#include "globals.h"
#include "Leaf.h"
#include "Nodes.h"
#include "cmdlib.h"
#include "log.h"
#include "brush.h"
#include "hooks.h"
#include "worldbsp.h"
#include "bspfile.h"
#include "planes.h"

void SetModelBounds(dmodel_t* model, vec3_t& mins, vec3_t& maxs)
{
    model->mins[0] = mins[0];
    model->mins[1] = mins[1];
    model->mins[2] = mins[2];
    model->maxs[0] = maxs[0];
    model->maxs[1] = maxs[1];
    model->maxs[2] = maxs[2];
}

void WriteNodeAndLeafData(int depth, node_t* node)
{
    while (node->planenum != -1)
    {
        fprintf(
            lightingfile,
            "NODE %i %i %i %i\n",
            node->visleafnum,
            node->planenum,
            node->children[0]->visleafnum,
            node->children[1]->visleafnum);

        WriteNodeAndLeafData(depth + 1, node->children[0]);
        node = node->children[1];
        depth++;
    }

    unsigned int leafType = node->leaf_type;
    if((leafType & 0xFF) == 0)
        leafType = (leafType & 0xFFFFFF00) | 1;;
    fprintf(lightingfile, "LEAF %i %u\n", node->visleafnum, leafType);
}

void WriteLightingFile(node_t* node)
{
    char filename[MAX_PATH];
    g_numVisLeafs = 0;
    AssignVisLeafNumbers_r(node);

    safe_snprintf(filename, sizeof(filename), "%s.lbsp", g_Mapname);
    lightingfile = fopen(filename, "w");

    if (!lightingfile)
    {
        Error("Could not open lighting bsp file '%s'\n", filename);
        return;
    }

    fprintf(lightingfile, "%i\n", g_numVisLeafs);
    WriteNodeAndLeafData(0, node);

    fclose(lightingfile);
    lightingfile = nullptr;
}

void BuildLightingBSPTree(entity_t* ent)
{
    double originalMaxNodeSize; // Stores the original maximum node size
    double startTime; // Records the start time of the function for logging
    face_t* original_face_list; // Stores the list of original_face_list
    face_t* inverted_face_list; // Stores the list of inverted original_face_list
    node_t* node; // Represents the current node
    double elapsedTime; // Stores the elapsed time for the verbose logging

    // Check if lighting is enabled
    if (g_lighting)
    {
        // Store original max node size and set it to 512
        originalMaxNodeSize = static_cast<double>(g_MaxNodeSize);
        g_MaxNodeSize = 512;

        // Record the start time
        startTime = I_FloatTime();
        Verbose("Building lighting BSP tree...\n");

        // Free existing brush original_face_list and copy face lists
        FreeBrushFaces(ent, CONTENTS_DETAIL | CONTENTS_BSP);
        original_face_list = CopyFaceList(ent, CONTENTS_DETAIL | CONTENTS_BSP);
        inverted_face_list = CopyFaceList_Inverted(ent, CONTENTS_DETAIL | CONTENTS_BSP);

        // Allocate node and calculate its bounds
        node = new node_t;
        CalcNodeBoundsFromFaces(node, original_face_list);

        // Build the BSP tree
        BuildBSPTree(false, node, original_face_list);
        FilterFacesIntoTree(inverted_face_list, node, true, true);
        FilterBrushesIntoTree(node, ent, CONTENTS_DETAIL | CONTENTS_BSP);

        // Strip outside original_face_list and mark empty brush original_face_list
        StripOutsideFaces(node, ent, CONTENTS_DETAIL);
        MarkEmptyBrushFaces(CONTENTS_DETAIL, ent);

        // Build marks and write the lighting file
        MarkFaceFragments(node, ent);
        WriteLightingFile(node);

        // Set all original_face_list to leaf node and recursively free the node
        SetAllFacesLeafNode(0, ent);
        delete node;

        // Free brush original_face_list again
        FreeBrushFaces(ent, CONTENTS_DETAIL | CONTENTS_BSP);

        // If verbose logging is enabled, log the time elapsed
        if (g_verbose)
        {
            Verbose("Building lighting BSP tree : ");
            elapsedTime = I_FloatTime();
            LogTimeElapsed(elapsedTime - startTime);
        }

        // Restore the original max node size
        g_MaxNodeSize = static_cast<unsigned __int64>(originalMaxNodeSize);
    }
}

bool CalcWorldBounds(const vec3_t& mins, const vec3_t& maxs)
{
    return mins[0] < maxs[0] && mins[1] < maxs[1] && mins[2] < maxs[2];
}

void ExpandBounds(vec3_t& minBounds, vec3_t& maxBounds, const vec3_t& point) 
{
    for (int i = 0; i < 3; ++i) 
    {
        if (point[i] < minBounds[i])
            minBounds[i] = point[i];

        if (point[i] > maxBounds[i])
            maxBounds[i] = point[i];
    }
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
    g_OutsideNode->leaf_type = LEAF_SOLID_AKA_OPAQUE;
    g_OutsideNode->planenum = -1;

    // Create portals
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

            plane_t* plane = &gMappedPlanes[planes[j]];
            portals[i]->winding->Clip(plane->normal, plane->dist, true);
        }
    }
}

void BuildBSPTree(bool makeNodePortals, node_t* node, face_t* bspFaces) 
{
    double startTime = I_FloatTime();

    if (makeNodePortals)
        Verbose("Building BSP tree...\n");

    g_numNodes = 0;
    g_numLeafs = 0;
    g_TimesCalledSplitFaces = 0;

    BuildBspTree_r(0, node, bspFaces, makeNodePortals);

    ResetPrintOnce();

    if (makeNodePortals) 
    {
        Verbose("%5i nodes\n", g_numNodes);
        Verbose("%5i leafs\n", g_numLeafs);
        if (g_verbose) 
        {
            Verbose("Building BSP tree : ");
            double endTime = I_FloatTime();
            LogTimeElapsed(endTime - startTime);
        }
    }
}

void BuildBspTree_r(int bspdepth, node_t* node, face_t* original_face, bool make_node_portals)
{
#if 0
    if (g_MakeNodePortals)
    {
        // subdivide large original_face
        face_t** prevptr = &original_face;
        face_t* f;
        while (1)
        {
            f = *prevptr;
            if (!f || !f->winding)
            {
                break;
            }

            SubdivideFace(f, prevptr);
            f = *prevptr;
            prevptr = &f->next;
        }

        //list = original_face;
    }
#endif

    unsigned int planenum;                // Planenum of the current node
    face_t* front_face_new;      // New front list after splitting
    face_t* back_face_new;       // New back list after splitting

    // Checking if the recommended maximum BSP depth is exceeded
    if (bspdepth > 256)
        PrintOnce("BuildBspTree_r MAX_RECOMMENDED_BSP_DEPTH exceeded\n");

    // Calculating the planenum of the current node based on its bounds and size
    // If the node size is within a maximum size, choose a plane from a list, otherwise choose a middle plane.
    planenum = CalcNodePlane(node, bspdepth, original_face);

    if (planenum == -1) {
        // If planenum is -1, it means the node has no bounds, mark it as a leaf
        node->planenum = -1;
        ++g_numLeafs;
    }
    else {
        // Otherwise, split the original_face based on the selected plane
        node->planenum = planenum & 0xFFFFFFFE;
        ++g_numNodes;

        // Splitting the original_face into front and back
        SplitFaces(original_face, planenum & 0xFFFFFFFE, bspdepth, &front_face_new, &back_face_new);

        // Allocating memory for the child nodes
        node->children[0] = new node_t;
        node->children[1] = new node_t;

        // compute bounds
        CalcNodeChildBounds(planenum & 0xFFFFFFFE, node);

        if (make_node_portals) {
            // Making the current node a portal node and splitting its portals
            MakeNodePortal(node);
            SplitNodePortals(node);
        }

        // Recursively build the BSP tree for the child nodes
        BuildBspTree_r(bspdepth + 1, node->children[0], front_face_new, make_node_portals);
        BuildBspTree_r(bspdepth + 1, node->children[1], back_face_new, make_node_portals);
    }
}

void PrintLeakInfoIfLeaked(entinfo_t* entinfo, int pass_num)
{
    const char* val_for_key;
    vec3_t origin;

    if (g_bLeaked)
    {
        GetVectorForKey(entinfo->entities[g_CurrentEntity], "origin", origin);
        val_for_key = ValueForKey(entinfo->entities[g_CurrentEntity], "classname");
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
    }
}

node_t* FillOutside(entinfo_t* entinfo, node_t* node, int pass_num) 
{
    g_EntInfo = entinfo;
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

    for (unsigned int i = 1; i < entinfo->numentities; ++i)
    {
        if (g_bLeaked)
            break;

        entity_t* entity = entinfo->entities[i];
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
        PrintLeakInfoIfLeaked(entinfo, pass_num);
        fclose(pointfile);
        fclose(linefile);
        pointfile = NULL;
        linefile = NULL;
        if (!g_bLeaked) 
        {
            CalcInternalNodes(node);
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

void WriteHullFile(const char* fileNamePrefix, entity_t* entity, int brushFlag, char appendMode, char writeFlag) 
{
    char fileName[264];
    safe_snprintf(fileName, 0x104u, "%s%s", g_Mapname, fileNamePrefix);
    const char* mode = (appendMode) ? "a+" : "w";
    FILE* file = fopen(fileName, mode);
    if (!file)
        Error("Could not open hullfile '%s' for writing\n", fileName);

    unsigned int numBrushes = entity->numbrushes;
    for (unsigned int brushIndex = 0; brushIndex < numBrushes; ++brushIndex) 
    {
        brush_t* brush = entity->brushes[brushIndex];
        if ((brushFlag & brush->brushflags) != 0) 
        {
            unsigned int numSides = brush->numsides;
            for (unsigned int sideIndex = 0; sideIndex < numSides; ++sideIndex) 
            {
                side_t* side = brush->brushsides[sideIndex];
                face_t* original_face = (writeFlag) ? side->final_face : side->face_fragments;
                while (original_face) 
                {
                    Winding* winding = original_face->winding;
                    int numPoints = winding->m_NumPoints;
                    fprintf(file, "%i %i %i %i\n", original_face->planenum, 0, -2, numPoints);
                    for (int pointIndex = 0; pointIndex < numPoints; ++pointIndex) 
                    {
                        fprintf(file, "%5.2f %5.2f %5.2f\n",
                            winding->m_Points[pointIndex][0],
                            winding->m_Points[pointIndex][1],
                            winding->m_Points[pointIndex][2]);
                    }
                    fprintf(file, "\n");
                    original_face = original_face->next;
                }
            }
        }
    }
    fprintf(file, "%s", "-1 -1 -1 -1\n");
    fclose(file);
}

void WritePortalFile_r(int depth, node_t* headnode) {
    if (!headnode)
        return;

    // Traverse the BSP tree until a leaf node is reached
    while (headnode->planenum != -1) 
    {
        depth++;
        WritePortalFile_r(depth, headnode->children[0]);
        headnode = headnode->children[1];
        if (!headnode)
            return;
    }

    // Process portals of the leaf node
    portal_t* portals = headnode->portals;
    while (portals) 
    {
        Winding* winding = portals->winding;
        if (winding->Valid()) 
        {
            node_t* node0 = portals->nodes[0];
            node_t* node1 = portals->nodes[1];
            if (node0 == headnode && node0->leaf_type != LEAF_SOLID_AKA_OPAQUE && node1->leaf_type != LEAF_SOLID_AKA_OPAQUE) 
            {
                plane_t* plane = &gMappedPlanes[portals->planenum];
                vec3_t winding_normal;
                vec_t winding_dist;
                portals->winding->getPlane(winding_normal, winding_dist);

                if (DotProduct(winding_normal, plane->normal) >= 0.99)
                    fprintf(portalfile, "%i %u %i %i ", portals->planenum, winding->m_NumPoints, node0->visleafnum, node1->visleafnum);
                else
                    fprintf(portalfile, "%i %u %i %i ", portals->planenum ^ 1, winding->m_NumPoints, node1->visleafnum, node0->visleafnum);
                for (unsigned int i = 0; i < winding->m_NumPoints; ++i)
                    fprintf(portalfile, "(%f %f %f) ", winding->m_Points[i][0], winding->m_Points[i][1], winding->m_Points[i][2]);
                fprintf(portalfile, "\n");
            }
        }
        if (portals->nodes[0] == headnode)
            portals = portals->next[0];
        else
            portals = portals->next[1];
    }
}

void NumberLeafs_r(int depth, node_t* node)
{
    node_t* current_node = node;
    int current_depth = depth;

    if (current_node) 
    {
        while (current_node->planenum != -1) 
        {
            NumberLeafs_r(++current_depth, current_node->children[0]);
            current_node = current_node->children[1];
            ++current_depth;
            if (!current_node)
                return;
        }

        current_node->visleafnum = num_visleafs++;

        if (current_node->leaf_type != LEAF_SOLID_AKA_OPAQUE) 
        {
            portal_t* portals = current_node->portals;
            if (portals) 
            {
                do 
                {
                    node_t* node0 = portals->nodes[0];
                    if (node0 == current_node) 
                    {
                        if (node0->leaf_type != LEAF_SOLID_AKA_OPAQUE && portals->nodes[1]->leaf_type != LEAF_SOLID_AKA_OPAQUE)
                            ++num_visportals;
                        portals = portals->next[0];
                    }
                    else 
                    {
                        portals = portals->next[1];
                    }
                } while (portals);
            }
        }
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

//ClipSideIntoTree_r/AddWindingToConvexHull
face_t* CombineFacesByPlane(face_t* face_fragments, face_t* original_face)
{
    face_t* final_face = new face_t(*original_face, new Winding);
    const plane_t& plane = gMappedPlanes[original_face->planenum];
    bool isFirstValidWinding = true;

    for (const face_t* currentFace = face_fragments; currentFace; currentFace = currentFace->next)
    {
        if (!currentFace->winding->Valid())
            continue;

        if (isFirstValidWinding)
        {
            if (final_face->winding)
                delete final_face->winding;
            final_face->winding = new Winding(*currentFace->winding);
            isFirstValidWinding = false;
            continue;
        }

        for (unsigned int pointIndex = 0; pointIndex < currentFace->winding->m_NumPoints; ++pointIndex)
        {
            char sides[128] { SIDE_FRONT };
            int numPointsInBack = final_face->winding->classifyPointAgainstPlaneEdges(sides, sizeof(sides), plane, currentFace->winding->m_Points[pointIndex]);

            if (numPointsInBack == 1)
            {
                unsigned int first_back_index;
                for (first_back_index = 0; first_back_index < final_face->winding->m_NumPoints; ++first_back_index)
                {
                    if (sides[first_back_index] != SIDE_FRONT)
                        break;
                }
                final_face->winding->insertPoint(currentFace->winding->m_Points[pointIndex], first_back_index + 1);
            }
            else if (numPointsInBack > 1)
            {
                unsigned int first_front_index;
                for (first_front_index = 0; first_front_index < final_face->winding->m_NumPoints; ++first_front_index)
                {
                    if (sides[first_front_index] == SIDE_FRONT)
                        break;
                }

                unsigned int offset = 0;
                for (unsigned int k = 0; k < final_face->winding->m_NumPoints; ++k)
                {
                    offset = (k + first_front_index) % final_face->winding->m_NumPoints;
                    if (sides[offset] != SIDE_FRONT)
                        break;
                }
                final_face->winding->shiftPoints(numPointsInBack - 1, (offset + 1) % final_face->winding->m_NumPoints);
                final_face->winding->insertPoint(currentFace->winding->m_Points[pointIndex], offset + 1);
            }
        }
    }

    if (final_face->winding->Valid())
    {
        final_face->winding->RemoveColinearPoints();
        return final_face;
    }
    else
    {
        FreeFace(final_face);
        final_face = new face_t(*original_face, new Winding);
        return final_face;
    }
}

void GetFinalBrushFaces(entity_t* entity, int brushflags)
{
    // Measure elapsed time
    I_FloatTime();

    // Loop through each brush in the entity
    for (unsigned int i = 0; i < entity->numbrushes; ++i)
    {
        brush_t* brush = entity->brushes[i];

        // Check if the brush flag matches the processing flag
        if ((brush->brushflags & brushflags) != 0)
        {
            // Loop through each side of the brush
            for (unsigned int s = 0; s < brush->numsides; ++s)
            {
                side_t* side = brush->brushsides[s];
                
                if (side->final_face)
                    FreeFace(side->final_face);

                if ((side->original_face->flags & (CONTENTS_DETAIL | CONTENTS_BSP | CONTENTS_NODRAW | CONTENTS_UNKNOWN)) != 0)
                {
                    // clear face points
                    side->final_face = new face_t(*side->original_face, new Winding);
                }
                else
                {
                    // combine fragments
                    side->final_face = CombineFacesByPlane(side->face_fragments, side->original_face);
                }
            }
        }
    }
}

void WriteSmdSkeletonData(FILE* file) {
    fprintf(file, "version 1\n");
    fprintf(file, "nodes\n");
    fprintf(file, "  0 \"Root\" -1\n");
    fprintf(file, "end\n");
    fprintf(file, "skeleton\n");
    fprintf(file, "time\n");
    fprintf(file, "0 -2.161199 0.000000 1150.805420 -3.141593 0.000000 0.000000\n");
    fprintf(file, "end\n");
}

#if 0
void WriteTriangleVertices(FILE* file, Winding* winding, int index) {
    vec3_t v[3];
    winding->GetTriangleVertices(index, v);
    // Write vertices to file
    for (int i = 0; i < 3; ++i) {
        float x = v[i][0] / 256.0;
        float y = -v[i][1] / 256.0;
        float z = v[i][2] / 256.0;
        fprintf(file, "%.6f %.6f %.6f\n", x, y, z);
    }
}

void WriteSmdTriangles(FILE* file, entity_t* entity, float tesselation, const char* texture) {
    // Iterate through brushes and generate triangles
    for (int i = 0; i < entity->numbrushes; ++i) {
        brush_t* brush = entity->brushes[i];
        for (int j = 0; j < brush->numsides; ++j) {
            face_t* face = brush->brushsides[j]->original_face_list;
            Winding* winding = face->winding;
            if (winding && winding->m_NumPoints >= 3) {
                winding->RemoveColinearPoints();
                int num_triangles = winding->m_NumPoints - 2;
                for (int k = 0; k < num_triangles; ++k) {
                    fprintf(file, "%s\n", texture);
                    // Write triangle vertices to file
                    WriteTriangleVertices(file, winding, k);
                }
            }
        }
    }
}

void WriteQcFile(FILE* file, const char* water_model_name) {
    fprintf(file, "$modelname %s.mdl\n", water_model_name);
    fprintf(file, "$flags %u\n", 0x800);
    fprintf(file, "$texture \"mdl_water\"\n");
    fprintf(file, "$body studio \"%s\"\n", water_model_name);
    fprintf(file, "$sequence idle1 \"%s\"\n", water_model_name);
}

void ProcessWaterBrushes(entity_t* entity, float tesselation, const char* texture, const char* water_model_name, const char* mapname) {
    char DirectoryName[256];
    CopyDirectoryName(mapname, DirectoryName);
    char water_smd_path[256];
    safe_snprintf(water_smd_path, 256, "%s%s.smd", DirectoryName, water_model_name);
    char water_qc_path[256];
    safe_snprintf(water_qc_path, 256, "%s%s.qc", DirectoryName, water_model_name);
    FILE* smd_file = SafeOpenWrite(water_smd_path);
    FILE* qc_file = SafeOpenWrite(water_qc_path);
    if (!smd_file || !qc_file) {
        Log("Failed to open output files for water model.\n");
        return;
    }
    // Write skeleton data to smd file
    WriteSmdSkeletonData(smd_file);
    // Write triangles data to smd file
    WriteSmdTriangles(smd_file, entity, tesselation, texture);
    fclose(smd_file);
    // Write qc file for model compilation
    WriteQcFile(qc_file, water_model_name);
    fclose(qc_file);
}

void GenerateWaterModel(entity_t* entity) {
    const char* classname = ValueForKey(entity, "classname");
    if (classname && !_stricmp(classname, "func_water")) {
        const char* spawnflags = ValueForKey(entity, "spawnflags");
        if (spawnflags && !(atol(spawnflags) & 2) && g_water) {
            ++g_NumWaterModels;
            float tesselation = 32.0;
            const char* tesselation_str = ValueForKey(entity, "tesselation");
            if (*tesselation_str) {
                tesselation = atof(tesselation_str);
                if (tesselation < 1.0) {
                    Warning("Water tesselation amount of '%f' ignored, setting to 64\n", tesselation);
                    tesselation = 64.0;
                }
            }
            const char* texture = ValueForKey(entity, "texture");
            char water_model_name[256];
            char directory[256];
            char mapname[256];
            CopyDirectoryName(g_Mapname, directory);
            ExtractFileNameWithoutExtension(mapname, g_Mapname);
            safe_snprintf(water_model_name, 256, "%s_water_%d", mapname, g_NumWaterModels);
            char mdl_path[256];
            safe_snprintf(mdl_path, 256, "water/%s.mdl", water_model_name);
            SetKeyValue(entity, "watermodel", mdl_path);
            // Process brushes to generate water model
            ProcessWaterBrushes(entity, tesselation, texture, water_model_name, mapname);
        }
    }
}
#endif

// FIXME Todo: this isn't the same as the original
void SnapVerts(entity_t* ent)
{
    double startTime = I_FloatTime();

    if (!ent->index)
        Verbose("SnapVerts ...\n");

#if 0
    if (dword_8BE33C8) {
        j__free(dword_8BE33C8);
    }

    dword_8BE33C8 = nullptr;
    dword_8BE33CC = 0;
    dword_8BE33D0 = 0;
#endif

    for (unsigned int i = 0; i < ent->numbrushes; ++i) 
    {
        brush_t* brush = ent->brushes[i];
        for (unsigned int j = 0; j < brush->numsides; ++j) 
        {
            side_t* side = brush->brushsides[j];
            face_t* final_face = side->final_face;
            if (final_face && final_face->winding && final_face->winding->Valid()) 
            {
                Winding* winding = final_face->winding;
                for (unsigned int k = 0; k < winding->m_NumPoints; ++k) 
                {
                    vec_t* point = winding->m_Points[k];
                    vec_t x = point[0];
                    vec_t y = point[1];
                    vec_t z = point[2];
                    vec_t distance = sqrt(x * x + y * y + z * z);
                    // Snap vertices within a small threshold
                    if (distance < 0.1) 
                    {
                        // Snap the vertex
                        point[0] = round(x);
                        point[1] = round(y);
                        point[2] = round(z);
                    }
                }
            }
        }
    }

#if 0
    // Free memory
    if (dword_8BE33C8) {
        j__free(dword_8BE33C8);
    }
    dword_8BE33C8 = nullptr;
    dword_8BE33CC = 0;
    dword_8BE33D0 = 0;
#endif

    if (!ent->index) 
    {
        Verbose("SnapVerts : ");
        double endTime = I_FloatTime();
        LogTimeElapsed(endTime - startTime);
    }
}

void ModelBSP(entity_t* entity, dmodel_t* model, int modelbsp_index)
{
    model->headnode[3] = 0;
    model->firstleaf = g_numDLeafs;
    model->firstface = g_numDFaces;
    //GenerateWaterModel(entity);
    const char* classname = ValueForKey(entity, "classname");
    Verbose("===ModelBSP (%d) (entity %d : %s)===\n", modelbsp_index, entity->index, classname);
    face_t* original_face = CopyFaceList(entity, CONTENTS_BSP);
    node_t* node = new node_t;
    BuildBSPTree(0, node, original_face);
    face_t* inverted_face_fragments = CopyFaceList_Inverted(entity, CONTENTS_BSP);
    FilterFacesIntoTree(inverted_face_fragments, node, true, false);
    FilterBrushesIntoTree(node, entity, CONTENTS_DETAIL | CONTENTS_BSP);
    StripOutsideFaces(node, entity, CONTENTS_DETAIL | CONTENTS_BSP);
    MarkEmptyBrushFaces(CONTENTS_DETAIL | CONTENTS_BSP, entity);
    GetFinalBrushFaces(entity, CONTENTS_DETAIL | CONTENTS_BSP);
    SnapVerts(entity);
    SetAllFacesLeafNode(0, entity);

    delete node;
    
    node_t* model_node = new node_t;
    model_node->planenum = -1;
    model_node->leaf_type = LEAF_EMPTY_AKA_NOT_OPAQUE;
    SetAllFacesLeafNode(model_node, entity);

    CalcNodeBoundsFromBrushes(model_node, entity);
    Verbose(
        "  model bounds : (%.0f %.0f %.0f), (%.0f %.0f %.0f)\n",
        model_node->mins[0],
        model_node->mins[1],
        model_node->mins[2],
        model_node->maxs[0],
        model_node->maxs[1],
        model_node->maxs[2]);

    WriteHullFile(".p2", entity, CONTENTS_DETAIL | CONTENTS_BSP, 1, 1);
    MarkFinalFaceFragments(model_node, entity);
    SetModelBounds(model, model_node->mins, model_node->maxs);
    EmitDrawNode_r(model_node);
    delete model_node;

    model->numleafs = g_numDLeafs - model->firstleaf;
    model->numfaces = g_numDFaces - model->firstface;
}

void WorldBSP(entity_t* ent, entinfo_t* info, dmodel_t* dmodel)
{
    node_t* headnode; // Points to the head node of the BSP tree
    face_t* inverted_face_fragments; // Stores inverted original_face_list
    face_t* bsp_backfaces; // Stores backfaces
    face_t* bsp_faces; // Stores BSP original_face_list
    face_t* bsp_frontfaces; // Stores frontfaces

    // Initializing variables
    dmodel->headnode[3] = g_numDNodes;
    dmodel->firstleaf = g_numDLeafs;
    dmodel->firstface = g_numDFaces;

    // Build BSP tree
    BuildLightingBSPTree(ent);
    bsp_faces = CopyFaceList(ent, CONTENTS_BSP);
    headnode = new node_t;
    CalcNodeBoundsFromFaces(headnode, bsp_faces);
    Verbose(
        "  inital world bounds : (%.0f %.0f %.0f), (%.0f %.0f %.0f)\n",
        headnode->mins[0],
        headnode->mins[1],
        headnode->mins[2],
        headnode->maxs[0],
        headnode->maxs[1],
        headnode->maxs[2]);

    // Check if world bounds are valid
    if (CalcWorldBounds(headnode->mins, headnode->maxs))
    {
        // Make headnode portals
        MakeHeadnodePortals(headnode);
        BuildBSPTree(1, headnode, bsp_faces);
        inverted_face_fragments = CopyFaceList_Inverted(ent, CONTENTS_BSP);
        FilterFacesIntoTree(inverted_face_fragments, headnode, true, false);

        // Fill outside
        if (!g_nofill)
        {
            Verbose("Original node contents\n");
            PrintNodeMetricsByPlane(headnode);
            headnode = FillOutside(info, headnode, 1);
            if (!g_bLeaked)
            {
                SetAllFacesLeafNode(0, ent);
                Verbose("Filled node contents\n");
                PrintNodeMetricsByChildren(headnode);
            }
        }

        // Finalize BSP tree
        Verbose("Final node contents\n");
        PrintNodeMetricsByPlane(headnode);
        FilterBrushesIntoTree(headnode, ent, CONTENTS_BSP);
        StripOutsideFaces(headnode, ent, CONTENTS_BSP);
        WriteHullFile(".p0", ent, CONTENTS_BSP, 0, 0);
        MarkEmptyBrushFaces(CONTENTS_BSP, ent);

        // Handle additional details
        if (g_nofill || g_bLeaked)
        {
            FilterBrushesIntoTree(headnode, ent, CONTENTS_DETAIL);
            StripOutsideFaces(headnode, ent, CONTENTS_DETAIL);
            MarkEmptyBrushFaces(CONTENTS_DETAIL, ent);
            WriteHullFile(".p1", ent, CONTENTS_DETAIL | CONTENTS_BSP, 0, 0);
        }
        else
        {
            delete headnode;
            FreeBrushFaces(ent, CONTENTS_DETAIL | CONTENTS_BSP);
            Verbose("===FinalBSP===\n");
            bsp_frontfaces = CopyFaceList(ent, CONTENTS_BSP);
            headnode = new node_t;
            CalcNodeBoundsFromFaces(headnode, bsp_frontfaces);
            Verbose(
                "  final world bounds : (%f %f %f), (%f %f %f)\n",
                headnode->mins[0],
                headnode->mins[1],
                headnode->mins[2],
                headnode->maxs[0],
                headnode->maxs[1],
                headnode->maxs[2]);

            if (CalcWorldBounds(headnode->mins, headnode->maxs))
            {
                MakeHeadnodePortals(headnode);
                BuildBSPTree(true, headnode, bsp_frontfaces);
                bsp_backfaces = CopyFaceList_Inverted(ent, CONTENTS_BSP);
                FilterFacesIntoTree(bsp_backfaces, headnode, true, false);
                Verbose("Original node contents\n");
                PrintNodeMetricsByPlane(headnode);
                WritePortalfile(headnode);
                headnode = FillOutside(info, headnode, 2);
                if (!g_bLeaked)
                    SetAllFacesLeafNode(0, ent);
                FilterBrushesIntoTree(headnode, ent, CONTENTS_DETAIL | CONTENTS_BSP);
                StripOutsideFaces(headnode, ent, CONTENTS_DETAIL | CONTENTS_BSP);
                WriteHullFile(".p1", ent, CONTENTS_DETAIL | CONTENTS_BSP, 0, 0);
                MarkEmptyBrushFaces(CONTENTS_DETAIL | CONTENTS_BSP, ent);
            }
            else
            {
                Error("Bad world bounds\n");
            }
        }

        // Write portal file
        WritePortalfile(headnode);

        // Process brush original_face_list
        GetFinalBrushFaces(ent, CONTENTS_DETAIL | CONTENTS_BSP);

        // Snap vertices
        SnapVerts(ent);

        // Write hull files
        WriteHullFile(".p2", ent, CONTENTS_BSP, 1, 0);
        WriteHullFile(".p3", ent, CONTENTS_DETAIL, 1, 0);
        WriteHullFile(".p4", ent, CONTENTS_DETAIL | CONTENTS_BSP, 1, 0);

        // Build marks
        MarkFinalFaceFragments(headnode, ent);

        // Set model bounds
        SetModelBounds(dmodel, headnode->mins, headnode->maxs);

        // Emit draw node
        EmitDrawNode_r(headnode);

        // Free memory
        delete headnode;
    }
    else
    {
        Error("Bad world bounds\n");
    }

    // Update model leaf and face counts
    dmodel->numleafs = g_numDLeafs - dmodel->firstleaf;
    dmodel->numfaces = g_numDFaces - dmodel->firstface;
}