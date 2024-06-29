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
#include "mapfile.h"
#include "scriplib.h"
#include "textures.h"
#include "planes.h"

// replaced by C++
#if 0
void FreeEntity(entity_t* ent)
{
    if (ent)
    {
        if (ent->epairs)
            delete ent->epairs;
        for (unsigned i = 0; i < ent->numbrushes; ++i)
        {
            if (ent->brushes[i])
                delete ent->brushes[i];
        }
        free(ent->brushes);
        free(ent);
        --g_numEnts;
    }
}
#endif

// replaced by C++
#if 0
void FreeMap(mapinfo_t* ent)
{
    if (ent)
    {
        for (unsigned i = 0; i < ent->numentities; ++i)
            FreeEntity(ent->entities[i]);
        free(ent->entities);
        free(ent);
        --g_numMapsAlloced;
    }
}
#endif

unsigned int GetNumEngineEntities(mapinfo_t* mapfile)
{
    unsigned int engineEntityCount = 0;

    for (unsigned int entityIndex = 0; entityIndex < mapfile->numentities; ++entityIndex)
    {
        entity_t* entity = mapfile->entities[entityIndex];
        const char* className = ValueForKey(entity, "classname");

        if (!className ||
            (!_stricmp(className, "info_node") || !_stricmp(className, "info_node_air")))
        {
            ++engineEntityCount;
        }
        else
        {
            bool isLightEntity = (!_stricmp(className, "light") ||
                !_stricmp(className, "light_spot") ||
                !_stricmp(className, "light_environment"));
            if (!isLightEntity ||
                (strlen(ValueForKey(entity, "targetname")) || atol(ValueForKey(entity, "style"))))
            {
                ++engineEntityCount;
            }
        }
    }

    return engineEntityCount;
}

void CheckBrushForWater(entity_t* mapent)
{
    if (!_stricmp(ValueForKey(mapent, "classname"), "func_water"))
    {
        for (unsigned int i = 0; i < mapent->numbrushes; ++i)
        {
            mapent->brushes[i]->brushflags |= CONTENTS_WATER;
        }
    }
}

unsigned int GetNumBrushes(entity_t* entity)
{
    return entity->numbrushes;
}

unsigned int GetNumDetailBrushes(entity_t* entity)
{
    unsigned int detailBrushCount = 0;

    for (unsigned int i = 0; i < entity->numbrushes; ++i)
    {
        brush_t* brush = entity->brushes[i];
        if (brush->brushflags & CONTENTS_DETAIL)
            ++detailBrushCount;
    }

    return detailBrushCount;
}

unsigned int GetNumFaces(entity_t* entity)
{
    unsigned int totalFaces = 0;

    for (unsigned int i = 0; i < entity->numbrushes; ++i)
    {
        brush_t* brush = entity->brushes[i];
        totalFaces += brush->numsides;
    }

    return totalFaces;
}

unsigned int GetNumDetailFaces(entity_t* entity)
{
    unsigned int totalDetailFaces = 0;

    for (unsigned int i = 0; i < entity->numbrushes; ++i)
    {
        brush_t* brush = entity->brushes[i];
        if (brush->brushflags & CONTENTS_DETAIL)
            totalDetailFaces += brush->numsides;
    }

    return totalDetailFaces;
}

epair_t* ParseEpair()
{
    epair_t* e = new epair_t;

    if (strlen(g_token) >= MAX_KEY - 1)
        Error("ParseEpair: Key token too long (%i > MAX_KEY)", (int)strlen(g_token));

    e->key = _strdup(g_token);
    GetToken(false);

    if (strlen(g_token) >= ZHLT3_MAX_VALUE - 1)
        Error("ParseEpar: Value token too long (%i > ZHLT3_MAX_VALUE)", (int)strlen(g_token));

    e->value = _strdup(g_token);

    return e;
}

entity_t* AllocNewEntities(mapinfo_t* mapfile)
{
    // Allocate a new entity
    entity_t* newEntity = new entity_t;

    // Check if we need to allocate more space for the entities array
    if (mapfile->numentities == mapfile->numalloced)
    {
        // Increase allocation size by 64
        mapfile->numalloced += 64;

        // Allocate new memory for the entities array
        entity_t** newEntitiesArray = (entity_t**)calloc(1, 4 * mapfile->numalloced);

        // Copy old entities to the new array and free the old array
        if (mapfile->entities)
        {
            memcpy(newEntitiesArray, mapfile->entities, 4 * mapfile->numentities);
            delete mapfile->entities;
        }

        // Update the entities array to the new array
        mapfile->entities = newEntitiesArray;
    }

    // Add the new entity to the entities array and set its index
    mapfile->entities[mapfile->numentities] = newEntity;
    newEntity->index = mapfile->numentities++;

    // Return the new entity
    return newEntity;
}

bool ParseMapEntity(mapinfo_t* map)
{
    bool result = GetToken(1);
    if (result)
    {
        if (strcmp(g_token, "{"))
            Error("Parsing Entity %i, expected '{' got '%s'", map->numentities, g_token);

        entity_t* ent = AllocNewEntities(map);
        while (1)
        {
            if (!GetToken(1))
                Error("ParseEntity: EOF without closing brace");

            if (!strcmp(g_token, "}"))
                break;

            if (!strcmp(g_token, "{"))
            {
                if (!ent->index && !g_MapVersion)
                    g_MapVersion = atol(ValueForKey(ent, "mapversion"));

                ParseMapBrush(ent);
            }
            else
            {
                epair_t* new_epair = ParseEpair();
                new_epair->next = ent->epairs;
                ent->epairs = new_epair;
            }
        }
        CheckFatal();
        return true;
    }
    return result;
}

// makes original, pristine planes from the .map file
bool MakeBrushPlanes(entity_t* entity, brush_t* brush) 
{
    vec3_t origin;
    unsigned int side_num = 0;
    unsigned int num_sides = brush->numsides;

    // Get origin
    GetVectorForKey(entity, "origin", origin);

    for (unsigned int i = 0; i < num_sides; ++i) 
    {
        side_t* brushside = brush->brushsides[i];

        // Translate plane points by origin
        for (int j = 0; j < 3; ++j) 
        {
            for (int k = 0; k < 3; ++k) 
            {
                brushside->planepts[j][k] -= origin[k];
            }
        }

        // Calculate plane number
        unsigned int planenum = PlaneFromPoints(brushside->planepts[0], brushside->planepts[1], brushside->planepts[2]);
        brushside->plane_num = planenum;

        // Check for errors
        if (planenum == -1) 
        {
            Fatal(assume_PLANE_WITH_NO_NORMAL, "Entity %i, Brush %i, Side %i: plane with no normal",
                brush->entitynum, brush->brushnum, side_num);
        }

        for (unsigned int j = 0; j < side_num; ++j) 
        {
            unsigned int plane_num_other = brush->brushsides[j]->plane_num;
            if (brushside->plane_num == plane_num_other || brushside->plane_num == (plane_num_other ^ 1)) 
            {
                Fatal(assume_BRUSH_WITH_COPLANAR_FACES,
                    "Entity %i, Brush %i, Side %i: has a coplanar plane at (%.0f, %.0f, %.0f), texture %s",
                    brush->entitynum, brush->brushnum, side_num,
                    origin[0] + brushside->planepts[0][0],
                    origin[1] + brushside->planepts[0][1],
                    origin[2] + brushside->planepts[0][2],
                    brushside->td.name);
            }
        }

        ++side_num;
    }

    return true;
}

// makes original, pristine faces from the .map file
void MakeBrushFaces(entity_t* ent, brush_t* brush) 
{
    vec3_t origin;
    GetVectorForKey(ent, "origin", origin);

    for (unsigned int side_index = 0; side_index < brush->numsides; ++side_index)
    {
        side_t* side = brush->brushsides[side_index];
        face_t* face = new face_t(side->plane_num);

        // Initialize face properties
        side->original_face = face;
        face->brush = brush;
        face->brushside = side;
        side->parent_brush = brush;

        // Get texture info
        texinfo_t* info = TexinfoForBrushTexture(side, origin);
        memcpy(face->vecs, info->vecs, sizeof(face->vecs));
        face->texinfo = info;
        face->flags = side->td.surfaceflags;
    }
}

// make original, pristine entire brushes (including faces) from .map file
void CreateBrush(brush_t* brush, entity_t* ent)
{
    MakeBrushPlanes(ent, brush);
    CheckFatal();
    MakeBrushFaces(ent, brush);
    CheckFatal();
    MakeHullFaces(brush);
    CheckFatal();
    AddBrushBevels(brush);
    CheckFatal();
}

void ParseBrush(entity_t *mapent) 
{
    unsigned int brushIndex = 0;
    unsigned int numBrushes = mapent->numbrushes;
    char originStr[4096];
    BoundingBox bounds;
    bounds.reset();
    brush_t *brush;
    BoundingBox *p_bounds;

    // Process brushes with CONTENTS_ORIGIN
    for (brushIndex = 0; brushIndex < numBrushes; ++brushIndex) 
    {
        brush = mapent->brushes[brushIndex];
        if (brush->brushflags & CONTENTS_ORIGIN) 
        {
            CreateBrush(brush, mapent);

            brush->contents = CONTENTS_EMPTY;
            brush->FreeSides();

            if (brush->entitynum) 
            {
                safe_snprintf(originStr, sizeof(originStr), "%i %i %i",
                         (int)((brush->bounds.m_Maxs[0] + brush->bounds.m_Mins[0]) * 0.5),
                         (int)((brush->bounds.m_Mins[1] + brush->bounds.m_Maxs[1]) * 0.5),
                         (int)((brush->bounds.m_Mins[2] + brush->bounds.m_Maxs[2]) * 0.5));
                SetKeyValue(mapent, "origin", originStr);
            }
        }
    }

    // Create all brushes
    for (brushIndex = 0; brushIndex < numBrushes; ++brushIndex) 
    {
        CreateBrush(mapent->brushes[brushIndex], mapent);
    }

    // Calculate model_center if "light_origin" key exists
    if (mapent->index && *ValueForKey(mapent, "light_origin")) 
    {
        for (brushIndex = 0; brushIndex < numBrushes; ++brushIndex) 
        {
            brush = mapent->brushes[brushIndex];
            if (!(brush->brushflags & CONTENTS_ORIGIN)) 
            {
                p_bounds = &brush->bounds;
                bounds.add(p_bounds->m_Mins);
                bounds.add(p_bounds->m_Maxs);
            }
        }
        safe_snprintf(originStr, sizeof(originStr), "%i %i %i",
                 (int)((bounds.m_Maxs[0] + bounds.m_Mins[0]) * 0.5),
                 (int)((bounds.m_Maxs[1] + bounds.m_Mins[1]) * 0.5),
                 (int)((bounds.m_Maxs[2] + bounds.m_Mins[2]) * 0.5));
        SetKeyValue(mapent, "model_center", originStr);
    }

    // Check if entity contains only an origin brush
    if (mapent->numbrushes == 1 && (mapent->brushes[0]->brushflags & CONTENTS_ORIGIN)) 
    {
        brush = mapent->brushes[0];
        Error("Entity %i, contains ONLY an origin brush near (%.0f,%.0f,%.0f)\n",
              mapent->index, brush->bounds.m_Mins[0], brush->bounds.m_Mins[1], brush->bounds.m_Mins[2]);
    }

    GetVectorForKey(mapent, "origin", mapent->origin);
}

void ExportBevels(mapinfo_t* mapfile) 
{
    char FileName[MAX_PATH];
    safe_snprintf(FileName, MAX_PATH, "%s_bevel.map", g_Mapname);
    FILE* file = fopen(FileName, "wb");
    if (!file)
        Error("Could not open '%s'\n", FileName);

    fprintf(file, "{\n\"classname\" \"worldspawn\"\n\"mapversion\" \"510\"\n");

    for (unsigned int i = 0; i < mapfile->numentities; ++i) 
    {
        entity_t* entity = mapfile->entities[i];
        for (unsigned int j = 0; j < entity->numbrushes; ++j) 
        {
            brush_t* brush = entity->brushes[j];
            fprintf(file, "{\n");

            for (unsigned int k = 0; k < brush->numsides; ++k) 
            {
                side_t* side = brush->brushsides[k];
                plane_t* plane = &gMappedPlanes[side->plane_num];

                // Calculate side length
                double sideLength = fabs(plane->normal[0] * 16.0) + fabs(plane->normal[1] * 16.0) +
                    fabs(plane->normal[2] * 36.0) + fabs(plane->dist);

                // Create winding
                plane_t newplane = *plane;
                newplane.dist = sideLength;
                Winding* winding = new Winding(newplane);

                // Export winding
                for (int l = 0; l < 3; ++l) 
                {
                    double* points = winding->m_Points[l];
                    fprintf(file, "( %i %i %i ) ", (int)points[0], (int)points[1], (int)points[2]);
                }

                // Export texture info
                //NIGHTFIRE BUG, this code was outputting wrong info. fixed here:
#if 1
                fprintf(file, "%s [ %f %f %f %f ] [ %f %f %f %f ] 0 1 1 0 %s [ %f %f ]\n",
                    side->td.name, side->td.uaxis[0], side->td.uaxis[1], side->td.uaxis[2], side->td.ushift, 
                    side->td.vaxis[0], side->td.vaxis[1], side->td.vaxis[2], side->td.vshift,
                    side->td.material, side->td.lightmapscale, side->td.lightmaprotation);
#else
                // original code
                fprintf(
                    file,
                    "%s [ %f %f %f %f ] [ %f %f %f %f ] 0 1 1 0 %s [ %f %f ]\n",
                    side->td.name,
                    side->td.uaxis[0],
                    side->td.uaxis[1],
                    side->td.uaxis[2],
                    side->td.vaxis[0],
                    side->td.vaxis[0],
                    side->td.vaxis[1],
                    side->td.vaxis[2],
                    side->td.ushift,
                    side->td.material,
                    side->td.lightmapscale,
                    side->td.lightmaprotation);
#endif

                // Clean up
                delete winding;
            }
            fprintf(file, "}\n");
        }
    }
    fclose(file);
    Log("Completed\n");
}

mapinfo_t* LoadMapFile(const char* filename)
{
    Verbose("===LoadMapFile===\n");
    LoadScriptFile(filename);
    Verbose("Loading map '%s'\n", filename);
    mapinfo_t* mapfile = new mapinfo_t;
    g_MapVersion = 0;
    while (ParseMapEntity(mapfile))
    {
        entity_t* mapent = mapfile->entities[mapfile->numentities - 1];
        if (mapent->numbrushes)
        {
            CheckBrushForWater(mapfile->entities[mapfile->numentities - 1]);
            unsigned int NumBrushes = GetNumBrushes(mapent);
            unsigned int NumDetailBrushes = GetNumDetailBrushes(mapent);
            unsigned int NumFaces = GetNumFaces(mapent);
            unsigned int numdetail = GetNumDetailFaces(mapent);
            Verbose("Entity %i\n", mapfile->numentities - 1);
            Verbose("%5u bsp brushes\n", NumBrushes - NumDetailBrushes);
            Verbose("%5u detail brushes\n", NumDetailBrushes);
            Verbose("%5u total brushes\n\n", NumBrushes);
            Verbose("%5u bsp faces\n", NumFaces - numdetail);
            Verbose("%5u detail faces\n", numdetail);
            Verbose("%5u total faces\n\n", NumFaces);
            ParseBrush(mapent);
        }
        else
        {
            GetVectorForKey(mapfile->entities[mapfile->numentities - 1], "origin", mapent->origin);
        }
    }
    if (g_showbevels)
        ExportBevels(mapfile);
    int NumEngineEntities = GetNumEngineEntities(mapfile);
    Verbose("World Totals\n");
    Verbose("%5i map entities \n", mapfile->numentities - NumEngineEntities);
    Verbose("%5i engine entities\n", NumEngineEntities);
    hlassume(NumEngineEntities < MAX_MAP_ENGINE_ENTITIES, assume_MAX_ENGINE_ENTITIES);
    CheckFatal();
    return mapfile;
}

void ParseMapBrush(entity_t* ent)
{
    if (g_MapVersion < 510)
    {
        Error("Map version is '%d', expecting at least '%d'\n", g_MapVersion, 510);
    }

    brush_t* brush = ent->CreateNewBrush();
    GetToken(true);

    // Check for "BRUSHFLAGS"
    if (strcmp(g_token, "BRUSHFLAGS") == 0)
    {
        while (IsScriptNonEmptyAndNotComment())
        {
            GetToken(false);
            if (strcmp(g_token, "DETAIL") == 0)
            {
                brush->brushflags |= CONTENTS_DETAIL;
            }
        }
    }
    else
    {
        UnGetToken();
    }

    while (GetToken(true))
    {
        if (strcmp(g_token, "}") == 0)
        {
            break;
        }

        side_s* brush_side = brush->CreateNewBrushSide();
        ParseBrushSide(brush, brush_side);
    }

    HandleBrushFlags(brush);
}
#if 0
Special Texture : special/bsp
LeafType : 2
RenderFlags : 0x00000021
BrushFlags : 0x00000100

Special Texture : special/portal
LeafType : 1
RenderFlags : 0x00000031
BrushFlags : 0x00000100

Special Texture : special/nodraw
LeafType : 2
RenderFlags : 0x00000020
BrushFlags : 0000000000

Special Texture : special/hint
LeafType : 1
RenderFlags : 0x00000021
BrushFlags : 0x00001100

Special Texture : special/skip
LeafType : 1
RenderFlags : 0x00000220
BrushFlags : 0x00001000

Special Texture : special/opaque
LeafType : 1
RenderFlags : 0x00000020
BrushFlags : 0x00000200

Special Texture : special/clip
LeafType : 1
RenderFlags : 0x00000020
BrushFlags : 0x00070200

Special Texture : special/playerclip
LeafType : 1
RenderFlags : 0x00000020
BrushFlags : 0x00010200

Special Texture : special/npcclip
LeafType : 1
RenderFlags : 0x00000020
BrushFlags : 0x00040200

Special Texture : special/enemyclip
LeafType : 1
RenderFlags : 0x00000020
BrushFlags : 0x00020200

Special Texture : special/itemclip
LeafType : 1
RenderFlags : 0x00000020
BrushFlags : 0x00080200

Special Texture : special/origin
LeafType : 1
RenderFlags : 0x00000020
BrushFlags : 0x00000400

Special Texture : special/sky
LeafType : 2
RenderFlags : 0x00060004
BrushFlags : 0x00000900

Special Texture : special/aaatrigger
LeafType : 1
RenderFlags : 0x00000020
BrushFlags : 0x00200200

Special Texture : special/trigger
LeafType : 1
RenderFlags : 0x00000020
BrushFlags : 0x00200200
#endif

const special_texture_t g_SpecialTextures[] = {
    {"special/bsp", CONTENTS_SOLID, SURFACEFLAG_BSP, BRUSHFLAG_BSP},
    {"special/portal", CONTENTS_EMPTY, SURFACEFLAG_PORTAL, BRUSHFLAG_PORTAL},
    {"special/nodraw", CONTENTS_SOLID, SURFACEFLAG_NODRAW, BRUSHFLAG_NODRAW},
    {"special/hint", CONTENTS_EMPTY, SURFACEFLAG_HINT, BRUSHFLAG_HINT},
    {"special/skip", CONTENTS_EMPTY, SURFACEFLAG_SKIP, BRUSHFLAG_SKIP},
    {"special/opaque", CONTENTS_EMPTY, SURFACEFLAG_OPAQUE, BRUSHFLAG_OPAQUE},
    {"special/clip", CONTENTS_EMPTY, SURFACEFLAG_CLIP, BRUSHFLAG_CLIP},
    {"special/playerclip", CONTENTS_EMPTY, SURFACEFLAG_PLAYERCLIP, BRUSHFLAG_PLAYERCLIP},
    {"special/npcclip", CONTENTS_EMPTY, SURFACEFLAG_NPCCLIP, BRUSHFLAG_NPCCLIP},
    {"special/enemyclip", CONTENTS_EMPTY, SURFACEFLAG_ENEMYCLIP, BRUSHFLAG_ENEMYCLIP},
    {"special/itemclip", CONTENTS_EMPTY, SURFACEFLAG_ITEMCLIP, BRUSHFLAG_ITEMCLIP},
    {"special/origin", CONTENTS_EMPTY, SURFACEFLAG_ORIGIN, BRUSHFLAG_ORIGIN},
    {"special/sky", CONTENTS_SOLID, SURFACEFLAG_SKY, BRUSHFLAG_SKY},
    {"special/aaatrigger", CONTENTS_EMPTY, SURFACEFLAG_AAATRIGGER, BRUSHFLAG_AAATRIGGER},
    {"special/trigger", CONTENTS_EMPTY, SURFACEFLAG_TRIGGER, BRUSHFLAG_TRIGGER},

    //NEW
#ifdef BBSP_NULL_SUPPORT
    {"special/null", CONTENTS_SOLID, SURFACEFLAG_NULL, BRUSHFLAG_NULL},
#endif
#ifdef BBSP_BLOCKLIGHT_SUPPORT
    {"special/blocklight", CONTENTS_EMPTY | CONTENTS_SOLID, SURFACEFLAG_BLOCKLIGHT, BRUSHFLAG_BLOCKLIGHT},
#endif

    // add new things above this or it will break GetDefaultFlagsForTextureName
    {"special/liquids", CONTENTS_EMPTY, SURFACEFLAG_LIQUIDS, BRUSHFLAG_LIQUIDS}
};

void GetDefaultFlagsForTextureName(
    const char* texture,
    unsigned int* contents,
    unsigned int* render_flags,
    unsigned int* brush_flags)
{
    /*
    // for debugging
    static bool printed = false;
    if (!printed)
    {
        char tmp[1024];
        for (int i = 0; i < ARRAYSIZE(g_SpecialTextures); ++i)
        {
            const auto tex = &g_SpecialTextures[i];
            sprintf_s(tmp, "Special Texture: %s\nLeafType: %i\nRenderFlags: %#010x\nBrushFlags: %#010x\n\n", tex->name, tex->contents, tex->renderflags, tex->brushflags);
            OutputDebugStringA(tmp);
        }
        printed = true;
    }
    */

    // Loop through special textures
    for (unsigned int i = 0; i < ARRAYSIZE(g_SpecialTextures) - 1; ++i)
    {
        if (!_stricmp(texture, g_SpecialTextures[i].name))
        {
            *contents = g_SpecialTextures[i].contents;
            *render_flags = g_SpecialTextures[i].renderflags;
            *brush_flags = g_SpecialTextures[i].brushflags;
            return;
        }
    }

    // Check if the texture matches "liquids"
    if (_strnicmp(texture, "liquids", 7) == 0)
    {
        *contents = CONTENTS_EMPTY;
        *render_flags = SURFACEFLAG_LIQUIDS;
        *brush_flags = BRUSHFLAG_LIQUIDS;
        return;
    }

    // Assign values to output parameters
    *contents = CONTENTS_SOLID;
    *render_flags = 0;
    *brush_flags = 0;
}

void ParseRotation(brush_t* brush, side_s* brush_side)
{
    GetToken(false);
#ifdef _DEBUG
    double rotation = atof(g_token);
    int breakpointme_in_debug = 1; // so we can see the value
#endif
}

void ParseFlags(brush_t* brush, side_s* brush_side)
{
    unsigned int texture_brushflags;
    unsigned int contents;
    GetDefaultFlagsForTextureName(brush_side->td.name, &contents, &brush_side->td.surfaceflags, &texture_brushflags);
    if (contents > brush->contents)
        brush->contents = contents;
    brush->brushflags |= texture_brushflags;
    GetToken(false);
    brush_side->td.surfaceflags |= atol(g_token);
}

void ParseBrushSide(brush_t* brush, side_s* brush_side)
{
    for (int i = 0; i < 3; ++i)
    {
        ParsePlanePoints(brush, brush_side, i);
    }

    ParseTexture(brush, brush_side);
    ParseAxis(brush, brush_side->td.uaxis, brush_side->td.ushift, "UAxis");
    ParseAxis(brush, brush_side->td.vaxis, brush_side->td.vshift, "VAxis");
    ParseRotation(brush, brush_side);
    ParseScale(brush, brush_side);
    ParseFlags(brush, brush_side);
    ParseMaterial(brush, brush_side);
    ParseLightmapInfo(brush, brush_side);
}

void ParsePlanePoints(brush_t* brush, side_s* brush_side, int pointIndex)
{
    if (pointIndex > 0)
    {
        GetToken(true);
    }

    if (strcmp(g_token, "(") != 0)
    {
        Error("Parsing (PlanePoint #%d) Entity %i, Brush %i, Side %i : Expecting '(' got '%s'", pointIndex + 1, brush->entitynum, brush->brushnum, brush->numsides, g_token);
    }

    for (int i = 0; i < 3; ++i)
    {
        GetToken(false);
        brush_side->planepts[pointIndex][i] = atof(g_token);
    }

    GetToken(false);
    if (strcmp(g_token, ")") != 0)
    {
        Error("Parsing (PlanePoint #%d) Entity %i, Brush %i, Side %i : Expecting ')' got '%s'", pointIndex + 1, brush->entitynum, brush->brushnum, brush->numsides, g_token);
    }
}

void ParseTexture(brush_t* brush, side_s* brush_side)
{
    GetToken(false);
    _strupr(g_token);
    safe_strncpy(brush_side->td.name, g_token, MAX_TEXTURE_LENGTH);
}

void ParseMaterial(brush_t* brush, side_s* brush_side)
{
    GetToken(false);
    _strupr(g_token);
    safe_strncpy(brush_side->td.material, g_token, MAX_TEXTURE_LENGTH);
}

void ParseAxis(brush_t* brush, vec3_t axis, vec_t& shift, const char* axisName)
{
    GetToken(false);
    if (strcmp(g_token, "[") != 0)
    {
        Error("Parsing (%s) Entity %i, Brush %i, Side %i : Expecting '[' got '%s'", axisName, brush->entitynum, brush->brushnum, brush->numsides, g_token);
    }

    for (int i = 0; i < 3; ++i)
    {
        GetToken(false);
        axis[i] = atof(g_token);
    }

    GetToken(false);
    shift = atof(g_token);

    GetToken(false);
    if (strcmp(g_token, "]") != 0)
    {
        Error("Parsing (%s) Entity %i, Brush %i, Side %i : Expecting ']' got '%s'", axisName, brush->entitynum, brush->brushnum, brush->numsides, g_token);
    }
}

void ParseScale(brush_t* brush, side_s* brush_side)
{
    GetToken(false);
    brush_side->td.uscale = atof(g_token);

    GetToken(false);
    brush_side->td.vscale = atof(g_token);
}

void ParseLightmapInfo(brush_t* brush, side_s* brush_side)
{
    GetToken(false);
    if (strcmp(g_token, "[") != 0)
    {
        Error("Parsing (LightmapInfo) Entity %i, Brush %i, Side %i : Expecting '[' got '%s'", brush->entitynum, brush->brushnum, brush->numsides, g_token);
    }

    GetToken(false);
    brush_side->td.lightmapscale = atof(g_token);

    GetToken(false);
    brush_side->td.lightmaprotation = atof(g_token);

    GetToken(false);
    if (strcmp(g_token, "]") != 0)
    {
        Error("Parsing (LightmapInfo) Entity %i, Brush %i, Side %i : Expecting ']' got '%s'", brush->entitynum, brush->brushnum, brush->numsides, g_token);
    }
}

void HandleBrushFlags(brush_t* brush)
{
    if (brush->brushflags & CONTENTS_ORIGIN)
        HandleOriginBrush(brush);
   
    //FIXME
    if (((brush->brushflags >> 8) & (CONTENTS_SOLID | CONTENTS_EMPTY)) == 0)
    {
        brush->brushflags |= CONTENTS_BSP;
    }

    bool found_keep = false;
    for (unsigned int i = 0; i < brush->numsides; ++i) 
    {
        if ((brush->brushsides[i]->td.surfaceflags & SURFACEFLAG_KEEP))
        {
            found_keep = true;
            break;
        }
    }
    if (found_keep)
    {
        for (unsigned int j = 0; j < brush->numsides; ++j)
        {
            if (!(brush->brushsides[j]->td.surfaceflags & SURFACEFLAG_KEEP))
                brush->brushsides[j]->td.surfaceflags = SURFACEFLAG_NODRAW;
        }
    }
}

void HandleOriginBrush(brush_t* brush)
{
    brush->brushflags = CONTENTS_ORIGIN;
    brush->contents = CONTENTS_EMPTY;

    for (unsigned int i = 0; i < brush->numsides; ++i)
    {
        side_t* side = brush->brushsides[i];
        side->td = brush_texture_t();
        side->td.surfaceflags = SURFACEFLAG_NODRAW;
        safe_strncpy(side->td.name, "special/origin", MAX_TEXTURE_LENGTH);
    }
}

entity_t* EntityForModel(unsigned int modnum, mapinfo_t* map)
{
    unsigned int i;
    const char* s;
    char            name[16];

    sprintf(name, "*%i", modnum);
    // search the entities for one using modnum
    for (i = 0; i < map->numentities; i++)
    {
        s = ValueForKey(map->entities[i], "model");
        if (!strcmp(s, name))
        {
            return map->entities[i];
        }
    }

    return map->entities[0];
}

entity_t* FindTargetEntity(mapinfo_t* map, const char* const target)
{
    unsigned int i;
    const char* n;

    for (i = 0; i < map->numentities; i++)
    {
        n = ValueForKey(map->entities[i], "targetname");
        if (!strcmp(n, target))
        {
            return map->entities[i];
        }
    }

    return NULL;
}