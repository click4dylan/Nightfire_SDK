
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

//todo
void WriteLightingFile(node_t* node)
{

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
    // Expand the bounding box
    vec_t expandedMins[3];
    vec_t expandedMaxs[3];
    for (int i = 0; i < 3; ++i)
    {
        expandedMins[i] = node->mins[i] - 24.0f;
        expandedMaxs[i] = node->maxs[i] + 24.0f;
    }

    // Create a solid leaf node
    node_t* solidLeaf = new node_t;
    solidLeaf->leaf_type = LEAF_SOLID_AKA_OPAQUE;
    solidLeaf->planenum = -1;
    g_CurrentNode = solidLeaf;

    // Create portals
    portal_t* portals[6];
    int portalIndex = 0;

    for (int i = 0; i < 2; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            vec_t normal[3] = { 0.0, 0.0, 0.0 };
            normal[j] = (i == 0) ? -1.0 : 1.0;

            unsigned int plane_num = FindIntPlane(normal, (i == 0) ? expandedMins[j] : expandedMaxs[j]);
            plane_t* plane = &gMappedPlanes[plane_num];

            portal_t* portal = new portal_t;
            portal->planenum = plane_num;

            Winding* winding = new Winding(*plane);
            portal->winding = winding;

            // Add portal to nodes
            AddPortalToNodes(portal, node, solidLeaf);

            portals[portalIndex++] = portal;
        }
    }

    // Clip portals
    for (int i = 0; i < 6; ++i)
    {
        for (int j = 0; j < 6; ++j)
        {
            if (j != i)
            {
                portals[i]->winding->Clip(gMappedPlanes[j].normal, gMappedPlanes[j].dist, true);
            }
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

void PrintLeakInfoIfLeaked(entinfo_t* entinfo, int pass_num)
{
    const char* val_for_key;
    vec3_t origin;

    if (g_bLeaked)
    {
        GetVectorForKey(entinfo->entities[g_CurrentEntity], "origin", origin);
        val_for_key = ValueForKey(entinfo->entities[g_CurrentEntity], "classname");
        Warning("=== LEAK (pass %d) ===\nEntity %s @ (%4.0f,%4.0f,%4.0f)", pass_num, val_for_key, origin[0], origin[1], origin[2]);
        Fatal(assume_LEAK, "\n"
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

node_t* FillOutside(entinfo_t* entinfo, node_t* a2, int pass_num) 
{
    node_t* node = a2;
    entinfo_t* entInfo = entinfo;
    unsigned int numEntities = entInfo->numentities;
    unsigned int entityIndex = 1;
    g_EntInfo = entinfo;
    g_bLeaked = 0;
    bool occupantFound = false;

    Verbose("----- FillOutside ----\n");
    pointfile = fopen(g_pointfilename, "w");
    if (!pointfile)
        Error("Couldn't open pointfile '%s'\n", g_pointfilename);
    linefile = fopen(g_linefilename, "w");
    if (!linefile)
        Error("Couldn't open linefile '%s'\n", g_linefilename);

    PrintLeafMetrics(a2, "Original tree");

    for (; entityIndex < numEntities; ++entityIndex) 
    {
        if (g_bLeaked)
            break;

        entity_t* entity = entInfo->entities[entityIndex];
        double origin[3];
        GetVectorForKey(entity, "origin", origin);
        const char* classname = ValueForKey(entity, "classname");

        if (!entity->numbrushes) 
        {
            // Check if origin is close to (0, 0, 0)
            int i;
            for (i = 0; i < 24 && fabs(origin[i / 8] - 0.0) <= 0.001; i += 8);

            if (i >= 24)
                // All coordinates are near 0, skip processing
                continue;

            if (!strcmp(classname, "info_player_start")) 
            {
                for (int x = -16; x <= 16 && !occupantFound; x += 16) 
                {
                    for (int y = -16; y <= 16 && !occupantFound; y += 16) 
                    {
                        vec3_t testOrigin = { origin[0] + x, origin[1] + y, origin[2] };
                        if (PlaceOccupant(testOrigin, node, entityIndex))
                            occupantFound = true;
                    }
                }
            }
            else {
                if (PlaceOccupant(origin, node, entityIndex)) {
                    occupantFound = true;
                }
            }
        }
    }

    if (occupantFound) 
    {
        PrintLeakInfoIfLeaked(entInfo, pass_num);
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
        brush_t* brush = entity->firstbrush[brushIndex];
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

        current_node->visleafnum = num_visleafs;
        num_visleafs += 1;
        if (current_node->leaf_type != LEAF_SOLID_AKA_OPAQUE) 
        {
            portal_t* portals = current_node->portals;
            if (portals) 
            {
                int visible_portals = num_visportals;
                do 
                {
                    node_t* node0 = portals->nodes[0];
                    if (node0 == current_node) 
                    {
                        if (node0->leaf_type != LEAF_SOLID_AKA_OPAQUE && portals->nodes[1]->leaf_type != LEAF_SOLID_AKA_OPAQUE)
                            ++visible_portals;
                        portals = portals->next[0];
                    }
                    else 
                    {
                        portals = portals->next[1];
                    }
                } while (portals);
                num_visportals = visible_portals;
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

int classifyPointsAgainstPlane(
    char* out_128_bytes,
    plane_t* plane,
    Winding* winding,
    vec_t* point)
{
#if 1
    //fixme
    unsigned int numPoints = winding->m_NumPoints;
    int numPointsInFront = 0;

    for (unsigned int i = 0; i < numPoints; ++i)
    {
        vec3_t* windingPoint = &winding->m_Points[i];
        vec3_t* nextWindingPoint = &winding->m_Points[(i + 1) % numPoints];

        vec3_t edge = {
            windingPoint[0] - nextWindingPoint[0],
            windingPoint[1] - nextWindingPoint[1],
            windingPoint[2] - nextWindingPoint[2]
        };

        double dot = DotProduct(edge, plane->normal);

        if (dot <= 0.01)
            out_128_bytes[i] = 0;
        else
        {
            out_128_bytes[i] = 1;
            ++numPointsInFront;
        }
    }

    return numPointsInFront;
#else
    unsigned int numPoints = winding->m_NumPoints;
    int numPointsClassifiedAsFront = 0;
    int currentIndex = 0;

    if (numPoints == 0)
        return 0;

    char* classificationResults = out_128_bytes;
    vec3_t* points = winding->m_Points;

    for (unsigned int i = 0; i < numPoints; ++i) {
        vec3_t currentPoint;
        VectorCopy(points[currentIndex], currentPoint);
        vec3_t nextPoint;
        VectorCopy(points[(i + 1) % numPoints], nextPoint);

        vec3_t edgeVector;
        edgeVector[0] = currentPoint[0] - nextPoint[0];
        edgeVector[1] = currentPoint[1] - nextPoint[1];
        edgeVector[2] = currentPoint[2] - nextPoint[2];
        VectorNormalize(edgeVector);

        vec3_t crossProduct;
        crossProduct[0] = edgeVector[1] * plane->normal[2] - edgeVector[2] * plane->normal[1];
        crossProduct[1] = edgeVector[2] * plane->normal[0] - edgeVector[0] * plane->normal[2];
        crossProduct[2] = edgeVector[0] * plane->normal[1] - edgeVector[1] * plane->normal[0];
        VectorNormalize(crossProduct);

        double dotProduct = crossProduct[0] * point[0] + crossProduct[1] * point[1] + crossProduct[2] * point[2];
        double pointPlaneDist = crossProduct[0] * currentPoint[0] + crossProduct[1] * currentPoint[1] + crossProduct[2] * currentPoint[2];

        if (dotProduct - pointPlaneDist <= ON_EPSILON) {
            classificationResults[i] = SIDE_BACK;
        }
        else {
            classificationResults[i] = SIDE_FRONT;
            numPointsClassifiedAsFront++;
        }

        currentIndex++;
    }

    return numPointsClassifiedAsFront;
#endif
}

//ClipSideIntoTree_r/AddWindingToConvexHull
face_t* CombineFacesByPlane(face_t* face_fragments, face_t* original_face)
{
#if 1
    Winding* initialWinding = new Winding();
    face_t* transformedFace = new face_t(*original_face, initialWinding);
    plane_t* plane = &gMappedPlanes[original_face->planenum];
    bool isFirstValidWinding = true;

    for (const face_t* currentFace = face_fragments; currentFace; currentFace = currentFace->next)
    {
        Winding* currentWinding = currentFace->winding;
        if (currentWinding->Valid())
        {
            if (isFirstValidWinding)
            {
                if (transformedFace->winding)
                    delete transformedFace->winding;
                transformedFace->winding = new Winding(*currentWinding);
                isFirstValidWinding = false;
                continue;
            }

            for (unsigned int pointIndex = 0; pointIndex < currentWinding->m_NumPoints; ++pointIndex)
            {
                char pointClassifications[128] = { SIDE_FRONT };
                int numPointsInFront = classifyPointsAgainstPlane(
                    pointClassifications, plane, transformedFace->winding, currentWinding->m_Points[pointIndex]);

                unsigned int numPointsInTransformedFace = transformedFace->winding->m_NumPoints;

                if (numPointsInFront == 1)
                {
                    for (unsigned int j = 0; j < numPointsInTransformedFace; ++j)
                    {
                        if (pointClassifications[j] == SIDE_BACK)
                        {
                            transformedFace->winding->insertPoint(currentWinding->m_Points[pointIndex], j + 1);
                            break;
                        }
                    }
                }
                else if (numPointsInFront > 1)
                {
                    unsigned int offset = 0;
                    for (unsigned int k = 0; k < numPointsInTransformedFace; ++k)
                    {
                        if (pointClassifications[k] == SIDE_FRONT)
                        {
                            offset = (k + 1) % numPointsInTransformedFace;
                            while (pointClassifications[offset] == SIDE_FRONT)
                            {
                                offset = (offset + 1) % numPointsInTransformedFace;
                            }
                            break;
                        }
                    }
                    transformedFace->winding->shiftPoints(numPointsInFront - 1, (offset + 1) % numPointsInTransformedFace);
                    transformedFace->winding->insertPoint(currentWinding->m_Points[pointIndex], offset + 1);
                }
            }
        }
    }

    if (transformedFace->winding->Valid())
    {
        transformedFace->winding->RemoveColinearPoints();
        return transformedFace;
    }
    else
    {
        FreeFace(transformedFace);
        Winding* fallbackWinding = new Winding();
        return new face_t(*original_face, fallbackWinding ? fallbackWinding : nullptr);
    }
#else
    // Create a new Winding object for the initial transformation
    Winding* initialWinding = new Winding();
    if (!initialWinding) 
        return nullptr;

    // Duplicate the winding of the input face
    face_t* transformedFace = new face_t(*original_face, initialWinding);
    if (!transformedFace) 
    {
        delete initialWinding;
        return nullptr;
    }

    // Retrieve the plane associated with the input face
    plane_t* plane = &gMappedPlanes[original_face->planenum];
    bool isFirstValidWinding = true;

    // Iterate through all reference original_face_list
    for (face_t* currentFace = face_fragments; currentFace; currentFace = currentFace->next)
    {
        Winding* currentWinding = currentFace->winding;
        if (currentWinding->Valid())
        {
            if (isFirstValidWinding)
            {
                // For the first valid winding, duplicate it directly
                if (transformedFace->winding)
                    delete transformedFace->winding;//transformedFace->winding->~Winding();//delete transformedFace->winding;
                transformedFace->winding = new Winding(*currentWinding);
                isFirstValidWinding = false;
                continue;
            }

            // For subsequent valid windings, classify and potentially insert points
            for (unsigned int pointIndex = 0; pointIndex < currentWinding->m_NumPoints; ++pointIndex)
            {
                char pointClassifications[128] = { 0 };
                int numPointsInFront = classifyPointsAgainstPlane(
                    pointClassifications, plane, transformedFace->winding, currentWinding->m_Points[pointIndex]);

                unsigned int numPointsInTransformedFace = transformedFace->winding->m_NumPoints;
                if (numPointsInFront == 1)
                {
                    for (unsigned int j = 0; j < numPointsInTransformedFace; ++j)
                    {
                        if (pointClassifications[j])
                        {
                            transformedFace->winding->insertPoint(currentWinding->m_Points[pointIndex], j + 1);  //+ 1 in nightfire because has wrapper for it
                            break;
                        }
                    }
                }
                else if (numPointsInFront > 1)
                {
                    unsigned int offset = 0;
                    for (unsigned int k = 0; k < numPointsInTransformedFace; ++k)
                    {
                        if (!pointClassifications[k])
                        {
                            for (unsigned int m = 0; m < numPointsInTransformedFace; ++m)
                            {
                                offset = (m + k) % numPointsInTransformedFace;
                                if (pointClassifications[offset]) 
                                    break;
                            }
                            break;
                        }
                    }
                    transformedFace->winding->shiftPoints(numPointsInFront - 1, (offset + 1) % numPointsInTransformedFace);
                    transformedFace->winding->insertPoint(currentWinding->m_Points[pointIndex], offset + 1); //+ 1 in nightfire because has wrapper for it
                }
            }
        }
    }

    // Check if the transformed face has a valid winding and return accordingly
    if (transformedFace->winding->Valid())
    {
        transformedFace->winding->RemoveColinearPoints();
        return transformedFace;
    }
    else
    {
        FreeFace(transformedFace);
        Winding* fallbackWinding = new Winding();
        return new face_t(*original_face, fallbackWinding ? fallbackWinding : nullptr);
    }
#endif
}

void GetFinalBrushFaces(entity_t* entity, int processingFlag)
{
    // Measure elapsed time
    I_FloatTime();

    unsigned int numBrushes = entity->numbrushes;
    unsigned int currentBrushIndex = 0;

    // Loop through each brush in the entity
    for (unsigned int brushIndex = 0; currentBrushIndex < numBrushes; brushIndex = currentBrushIndex)
    {
        brush_t* brush = entity->firstbrush[currentBrushIndex];

        // Check if the brush flag matches the processing flag
        if ((processingFlag & brush->brushflags) != 0)
        {
            unsigned int numSides = brush->numsides;

            // Loop through each side of the brush
            for (unsigned int sideIndex = 0; sideIndex < numSides; ++sideIndex)
            {
                side_t* side = brush->brushsides[sideIndex];
                
                if (side->final_face)
                    FreeFace(side->final_face);

                // Process the side's original_face_list based on their flags
                face_t* finalFace = nullptr;
                if ((side->original_face->flags & (CONTENTS_DETAIL | CONTENTS_BSP | CONTENTS_NODRAW | CONTENTS_UNKNOWN)) != 0)
                    finalFace = new face_t(*side->original_face, new Winding);
                else
                    finalFace = CombineFacesByPlane(side->face_fragments, side->original_face);

                // Store the processed original_face_list as extra original_face_list for the side
                side->final_face = finalFace;
            }
        }

        ++currentBrushIndex;
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
        brush_t* brush = entity->firstbrush[i];
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

void ModelBSP(entity_t* entity, dmodel_t* model, int modelbsp_index)
{
    int firstleaf = g_numDLeafs;
    int firstface = g_numDFaces;
    model->headnode[3] = 0;
    model->firstleaf = firstleaf;
    model->firstface = firstface;
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
    //SnapVerts(entity);
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
    int firstleaf; // Represents the index of the first leaf node in the BSP tree
    int firstface; // Represents the index of the first face in the BSP tree
    node_t* headnode; // Points to the head node of the BSP tree
    face_t* inverted_face_fragments; // Stores inverted original_face_list
    face_t* bsp_backfaces; // Stores backfaces
    face_t* bsp_faces; // Stores BSP original_face_list
    face_t* bsp_frontfaces; // Stores frontfaces

    // Initializing variables
    firstleaf = g_numDLeafs;
    dmodel->headnode[3] = g_numDNodes;
    firstface = g_numDFaces;
    dmodel->firstleaf = firstleaf;
    dmodel->firstface = firstface;

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
        //SnapVerts(ent);

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