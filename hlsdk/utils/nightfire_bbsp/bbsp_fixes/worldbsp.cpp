
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
#include "portals.h"
#include "outside.h"

void SetModelBounds(dmodel_t* model, vec3_t& mins, vec3_t& maxs)
{
    VectorCopy(mins, model->mins);
    VectorCopy(maxs, model->maxs);
}

void WriteLightingNodeAndLeafData(int depth, node_t* node)
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

        WriteLightingNodeAndLeafData(depth + 1, node->children[0]);
        node = node->children[1];
        depth++;
    }

    unsigned int leafType = node->contents;
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
    WriteLightingNodeAndLeafData(0, node);

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
#ifdef VARIABLE_LIGHTING_MAX_NODE_SIZE
        g_MaxNodeSize = g_LightingMaxNodeSize;
#else
        g_MaxNodeSize = 512;
#endif

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
        StripOutsideFaceFragments(node, ent, CONTENTS_DETAIL);
        MarkEmptyBrushFaces(ent, CONTENTS_DETAIL);

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
    unsigned int planenum;                // Planenum of the current node
    face_t* front_face_new;      // New front list after splitting
    face_t* back_face_new;       // New back list after splitting

    // Checking if the recommended maximum BSP depth is exceeded
    if (bspdepth > 256)
        PrintOnce("BuildBspTree_r MAX_RECOMMENDED_BSP_DEPTH exceeded\n");

    // Calculating the planenum of the current node based on its bounds and size
    // If the node size is within a maximum size, choose a plane from a list, otherwise choose a middle plane.
    planenum = CalcNodePlane(node, bspdepth, original_face);

    if (planenum == -1) 
    {
        // If planenum is -1, it means the node has no bounds, mark it as a leaf
        node->planenum = -1;
        ++g_numLeafs;
    }
    else 
    {
        // Otherwise, split the original_face based on the selected plane
        node->planenum = planenum & 0xFFFFFFFE;
        ++g_numNodes;

#ifdef SUBDIVIDE
        //TODO fixme: is it necessary to subdivide in this function?
        if (!g_nosubdiv)
        {
            // subdivide large faces
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
        }
#endif

        // Splitting the original_face into front and back
        SplitFaces(original_face, planenum & 0xFFFFFFFE, bspdepth, &front_face_new, &back_face_new);

        // Allocating memory for the child nodes
        node->children[0] = new node_t;
        node->children[1] = new node_t;

        // compute bounds
        CalcNodeChildBounds(planenum & 0xFFFFFFFE, node);

        if (make_node_portals) 
        {
            // Making the current node a portal node and splitting its portals
            MakeNodePortal(node);
            SplitNodePortals(node);
        }

        // Recursively build the BSP tree for the child nodes
        BuildBspTree_r(bspdepth + 1, node->children[0], front_face_new, make_node_portals);
        BuildBspTree_r(bspdepth + 1, node->children[1], back_face_new, make_node_portals);
    }
}

void WriteHullFile(const char* fileNamePrefix, entity_t* entity, int brushflags, bool append, bool write_final_faces)
{
    char fileName[MAX_PATH];
    safe_snprintf(fileName, MAX_PATH, "%s%s", g_Mapname, fileNamePrefix);

    FILE* file = fopen(fileName, append ? "a+" : "w");
    if (!file)
        Error("Could not open hullfile '%s' for writing\n", fileName);

    for (unsigned int brushIndex = 0; brushIndex < entity->numbrushes; ++brushIndex)
    {
        brush_t* brush = entity->brushes[brushIndex];
        if ((brush->brushflags & brushflags) != 0)
        {
            for (unsigned int sideIndex = 0; sideIndex < brush->numsides; ++sideIndex)
            {
                side_t* side = brush->brushsides[sideIndex];
                face_t* face = (write_final_faces) ? side->final_face : side->face_fragments;
                while (face)
                {
                    Winding* winding = face->winding;
                    fprintf(file, "%i %i %i %i\n", face->planenum, 0, -2, winding->m_NumPoints);
                    for (unsigned int i = 0; i < winding->m_NumPoints; ++i)
                    {
                        fprintf(file, "%5.2f %5.2f %5.2f\n", winding->m_Points[i][0], winding->m_Points[i][1], winding->m_Points[i][2]);
                    }
                    fprintf(file, "\n");
                    face = face->next;
                }
            }
        }
    }
    fprintf(file, "%s", "-1 -1 -1 -1\n");
    fclose(file);
}


//TODO: water model generation
#if 0
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
            if (final_face && final_face->winding && final_face->winding->HasPoints()) 
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
    //TODO
    //GenerateWaterModel(entity);
    const char* classname = ValueForKey(entity, "classname");
    Verbose("===ModelBSP (%d) (entity %d : %s)===\n", modelbsp_index, entity->index, classname);
    face_t* original_face = CopyFaceList(entity, CONTENTS_BSP);
    node_t* node = new node_t;
    BuildBSPTree(0, node, original_face);
    face_t* inverted_face_fragments = CopyFaceList_Inverted(entity, CONTENTS_BSP);
    FilterFacesIntoTree(inverted_face_fragments, node, true, false);
    FilterBrushesIntoTree(node, entity, CONTENTS_DETAIL | CONTENTS_BSP);
    StripOutsideFaceFragments(node, entity, CONTENTS_DETAIL | CONTENTS_BSP);
    MarkEmptyBrushFaces(entity, CONTENTS_DETAIL | CONTENTS_BSP);
    GetFinalBrushFaces(entity, CONTENTS_DETAIL | CONTENTS_BSP);
    SnapVerts(entity);
    SetAllFacesLeafNode(nullptr, entity);

    delete node;
    
    node_t* model_node = new node_t;
    model_node->planenum = -1;
    model_node->contents = CONTENTS_EMPTY;
    SetAllFacesLeafNode(model_node, entity);

    CalcNodeBoundsFromBrushes(model_node, entity);
    Verbose("  model bounds : (%.0f %.0f %.0f), (%.0f %.0f %.0f)\n", model_node->mins[0], model_node->mins[1], model_node->mins[2], model_node->maxs[0], model_node->maxs[1], model_node->maxs[2]);

    WriteHullFile(".p2", entity, CONTENTS_DETAIL | CONTENTS_BSP, APPEND, WRITE_FINAL_FACES);
    MarkFinalFaceFragments(model_node, entity);
    SetModelBounds(model, model_node->mins, model_node->maxs);
    EmitDrawNode_r(model_node);
    delete model_node;

    model->numleafs = g_numDLeafs - model->firstleaf;
    model->numfaces = g_numDFaces - model->firstface;
}

void WorldBSP(entity_t* ent, mapinfo_t* info, dmodel_t* dmodel)
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

    Verbose("  initial world bounds : (%.0f %.0f %.0f), (%.0f %.0f %.0f)\n", headnode->mins[0], headnode->mins[1], headnode->mins[2], headnode->maxs[0], headnode->maxs[1], headnode->maxs[2]);

    // Check if world bounds are valid
    if (!CalcWorldBounds(headnode->mins, headnode->maxs))
    {
        Error("Bad world bounds\n");
    }
    else 
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
        StripOutsideFaceFragments(headnode, ent, CONTENTS_BSP);
        WriteHullFile(".p0", ent, CONTENTS_BSP, TRUNCATE, WRITE_FACE_FRAGMENTS);
        MarkEmptyBrushFaces(ent, CONTENTS_BSP);

        // Handle additional details
        if (g_nofill || g_bLeaked)
        {
            FilterBrushesIntoTree(headnode, ent, CONTENTS_DETAIL);
            StripOutsideFaceFragments(headnode, ent, CONTENTS_DETAIL);
            MarkEmptyBrushFaces(ent, CONTENTS_DETAIL);
            WriteHullFile(".p1", ent, CONTENTS_DETAIL | CONTENTS_BSP, TRUNCATE, WRITE_FACE_FRAGMENTS);
        }
        else
        {
            delete headnode;
            FreeBrushFaces(ent, CONTENTS_DETAIL | CONTENTS_BSP);
            Verbose("===FinalBSP===\n");
            bsp_frontfaces = CopyFaceList(ent, CONTENTS_BSP);
            headnode = new node_t;
            CalcNodeBoundsFromFaces(headnode, bsp_frontfaces);
            Verbose("  final world bounds : (%f %f %f), (%f %f %f)\n", headnode->mins[0], headnode->mins[1], headnode->mins[2], headnode->maxs[0], headnode->maxs[1], headnode->maxs[2]);

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
                StripOutsideFaceFragments(headnode, ent, CONTENTS_DETAIL | CONTENTS_BSP);
                WriteHullFile(".p1", ent, CONTENTS_DETAIL | CONTENTS_BSP, TRUNCATE, WRITE_FACE_FRAGMENTS);
                MarkEmptyBrushFaces(ent, CONTENTS_DETAIL | CONTENTS_BSP);
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
        WriteHullFile(".p2", ent, CONTENTS_BSP, APPEND, WRITE_FACE_FRAGMENTS);
        WriteHullFile(".p3", ent, CONTENTS_DETAIL, APPEND, WRITE_FACE_FRAGMENTS);
        WriteHullFile(".p4", ent, CONTENTS_DETAIL | CONTENTS_BSP, APPEND, WRITE_FACE_FRAGMENTS);

        // Build marks
        MarkFinalFaceFragments(headnode, ent);

        // Set model bounds
        SetModelBounds(dmodel, headnode->mins, headnode->maxs);

        // Emit draw node
        EmitDrawNode_r(headnode);

        // Free memory
        delete headnode;
    }

    // Update model leaf and face counts
    dmodel->numleafs = g_numDLeafs - dmodel->firstleaf;
    dmodel->numfaces = g_numDFaces - dmodel->firstface;
}