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
#include "scriplib.h"
#include "filelib.h"

const char* ValueForKey(const entity_t* const ent, const char* const key)
{
    epair_t* ep;

    for (ep = ent->epairs; ep; ep = ep->next)
    {
        if (!strcmp(ep->key, key))
        {
            return ep->value;
        }
    }
    return "";
}

void GetVectorForKey(const entity_t* const ent, const char* const key, vec3_t vec)
{
    const char* k;
    double          v1, v2, v3;

    k = ValueForKey(ent, key);
    // scanf into doubles, then assign, so it is vec_t size independent
    v1 = v2 = v3 = 0;
    sscanf(k, "%lf %lf %lf", &v1, &v2, &v3);
    vec[0] = v1;
    vec[1] = v2;
    vec[2] = v3;
}

#if 0
entity_t* FindTargetEntity(const char* const target)
{
    int             i;
    const char* n;

    for (i = 0; i < g_numentities; i++)
    {
        n = ValueForKey(&g_entities[i], "targetname");
        if (!strcmp(n, target))
        {
            return &g_entities[i];
        }
    }

    return NULL;
}
#endif

int ArrayUsage(int size_of_item, const char* name_of_array, int num_items, int max_items)
{
    int v5; // esi
    double v7; // [esp+8h] [ebp-8h]
    float v8; // [esp+18h] [ebp+8h]

    if (max_items)
        v7 = (double)num_items * 100.0 / (double)max_items;
    else
        v7 = 0.0;
    v8 = v7;
    v5 = size_of_item * num_items;
    Log("%-12s  %7i/%-7i  %7i/%-7i  (%4.1f%%)\n", name_of_array, num_items, max_items, size_of_item * num_items, size_of_item * max_items, v8);
    return v5;
}

void PrintBSPFileSizes()
{
    int totalmemory; // esi
    int v1; // esi
    int v2; // esi
    int v3; // esi
    int v4; // esi
    int v5; // esi
    int v6; // esi
    int v7; // esi
    int v8; // esi
    int v9; // esi
    int v10; // esi
    int v11; // esi
    int v12; // esi
    int v13; // esi
    int v14; // esi
    int v15; // esi
    int v16; // esi
    int v17; // eax

    Log("\n");
    Log("Object names  Objects/Maxobjs  Memory / Maxmem  Fullness\n");
    Log("------------  ---------------  ---------------  --------\n");
    totalmemory = ArrayUsage(0x38, "models", g_numDModels, 1024);
    v1 = ArrayUsage(0xC, "brushes", g_numDBrushes, 32767) + totalmemory;
    v2 = ArrayUsage(8, "brushsides", g_numDBrushSides, 196602) + v1;
    v3 = ArrayUsage(0x30, "faces", g_numDFaces, 262144) + v2;
    v4 = ArrayUsage(0x14, "planes", g_numDPlanes, 262144) + v3;
    v5 = ArrayUsage(0xC, "verts", g_numDVerts, 262144) + v4;
    v6 = ArrayUsage(0xC, "normals", g_numDVerts, 262144) + v5;
    v7 = ArrayUsage(4, "indices", g_numDIndices, 262144) + v6;
    v8 = ArrayUsage(0x24, "nodes", g_numDNodes, 262144) + v7;
    v9 = ArrayUsage(0x30, "leaves", g_numDLeafs, 32000) + v8;
    v10 = ArrayUsage(4, "markbrushes", g_numDMarkBrushes, 262144) + v9;
    v11 = ArrayUsage(4, "marksurfaces", g_numDMarkSurfaces, 262144) + v10;
    v12 = ArrayUsage(0x40, "textures", g_numDTextures, 1024) + v11;
    v13 = ArrayUsage(0x40, "materials", g_numDMaterials, 1024) + v12;
    v14 = ArrayUsage(0x20, "texmatrix", g_numDTexMatrix, 65536) + v13;
    v15 = GlobUsage("lightdata", g_lightdatasize, MAX_MAP_LIGHTDATA) + v14;
    v16 = GlobUsage("visdata", g_visdatasize, MAX_MAP_VISIBILITY) + v15;
    v17 = GlobUsage("entdata", g_entdatasize, MAX_MAP_ENTSTRING /*sizeof(g_dentdata)*/); //todo fixme
    Log("=== Total BSP file data space used: %d bytes ===\n", v17 + v16);
}

int      FastChecksum(const void* const buffer, int bytes)
{
    int             checksum = 0;
    char* buf = (char*)buffer;

    while (bytes--)
    {
        checksum = _rotl(checksum, 4) ^ (*buf);
        buf++;
    }

    return checksum;
}

#define LUMP_ENTITIES 0
#define LUMP_PLANES 1
#define LUMP_TEXTURES 2
#define LUMP_MATERIALS 3
#define LUMP_VERTEXES 4
#define LUMP_NORMALS 5
#define LUMP_INDICES 6
#define LUMP_VISIBILITY 7
#define LUMP_NODES 8
#define LUMP_FACES 9
#define LUMP_LIGHTING 10
#define LUMP_LEAFS 11
#define LUMP_MARKSURFACES 12
#define LUMP_MARKBRUSHES 13
#define LUMP_MODELS 14
#define LUMP_BRUSHES 15
#define LUMP_BRUSHSIDES 16
#define LUMP_TEXMATRIX 17

int CopyLump(int lump, void* dest, int size, const dheader_t* const header)
{
    int length, ofs;

    length = header->lumps[lump].length;
    ofs = header->lumps[lump].offset;

    if (length % size)
        Error("LoadBSPFile: odd lump size");

    memcpy(dest, (byte*)header + ofs, length);

    return length / size;
}

void SwapBSPFile(bool todisk)
{
    // not needed on windows..
}

void LoadBSPImage(dheader_t* const header)
{
    unsigned int     i;

    // swap the header
    for (i = 0; i < sizeof(dheader_t) / 4; i++)
    {
        ((int*)header)[i] = LittleLong(((int*)header)[i]);
    }

    if (header->version != BSPVERSION)
        Error("BSP is version %i, not %i", header->version, BSPVERSION);

    g_numDModels = CopyLump(LUMP_MODELS, g_dmodels, sizeof(dmodel_t), header);
    g_numDVerts = CopyLump(LUMP_VERTEXES, g_dverts, sizeof(dvertex_t), header);
    g_numDIndices = CopyLump(LUMP_INDICES, g_dindices, sizeof(int), header);
    g_numDPlanes = CopyLump(LUMP_PLANES, g_dplanes, sizeof(dplane_t), header);
    g_numDLeafs = CopyLump(LUMP_LEAFS, g_dleafs, sizeof(dleaf_t), header);
    g_numDNodes = CopyLump(LUMP_NODES, g_dnodes, sizeof(dnode_t), header);
    g_numDFaces = CopyLump(LUMP_FACES, g_dFaces, sizeof(dface_t), header);
    g_numDMarkSurfaces = CopyLump(LUMP_MARKSURFACES, g_dmarksurfaces, sizeof(int), header);
    g_numDMarkBrushes = CopyLump(LUMP_MARKBRUSHES, g_dmarkbrushes, sizeof(int), header);
    g_numDTextures = CopyLump(LUMP_TEXTURES, g_dtextures, sizeof(texturename_t), header);
    g_numDMaterials = CopyLump(LUMP_MATERIALS, g_dmaterials, sizeof(texturename_t), header);
    g_numDBrushes = CopyLump(LUMP_BRUSHES, g_dbrushes, sizeof(dbrush_t), header);
    g_numDBrushSides = CopyLump(LUMP_BRUSHSIDES, g_dbrushsides, sizeof(dbrushside_t), header);
    g_numDTexMatrix = CopyLump(LUMP_TEXMATRIX, g_dtexmatrix, sizeof(dtexmatrix_t), header);
    g_visdatasize = CopyLump(LUMP_VISIBILITY, g_dvisdata, 1, header);
    g_lightdatasize = CopyLump(LUMP_LIGHTING, g_dlightdata, 1, header);
    g_entdatasize = CopyLump(LUMP_ENTITIES, g_dentdata, 1, header);

    free(header); // everything has been copied out

    //
    // swap everything
    //      
    SwapBSPFile(false);

    g_dmodels_checksum = FastChecksum(g_dmodels, sizeof(dmodel_t) * g_numDModels);
    g_dverts_checksum = FastChecksum(g_dverts, sizeof(dvertex_t) * g_numDVerts);
    g_dnormals_checksum = FastChecksum(g_dnormals, sizeof(dnormal_t) * g_numDVerts);
    g_dindicies_checksum = FastChecksum(g_dindices, sizeof(int) * g_numDIndices);
    g_dplanes_checksum = FastChecksum(g_dplanes, sizeof(dplane_t) * g_numDPlanes);
    g_dleafs_checksum = FastChecksum(g_dleafs, sizeof(dleaf_t) * g_numDLeafs);
    g_dnodes_checksum = FastChecksum(g_dnodes, sizeof(dnode_t) * g_numDNodes);
    g_dfaces_checksum = FastChecksum(g_dFaces, sizeof(dface_t) * g_numDFaces);
    g_dmarksurfaces_checksum = FastChecksum(g_dmarksurfaces, sizeof(int) * g_numDMarkSurfaces);
    g_dmarkbrushes_checksum = FastChecksum(g_dmarkbrushes, sizeof(int) * g_numDMarkBrushes);
    g_dtextures_checksum = FastChecksum(g_dtextures, g_numDTextures * sizeof(texturename_t));
    g_dmaterials_checksum = FastChecksum(g_dmaterials, g_numDMaterials * sizeof(texturename_t));
    g_dbrushes_checksum = FastChecksum(g_dbrushes, sizeof(dbrush_t) * g_numDBrushes);
    g_dbrushsides_checksum = FastChecksum(g_dbrushsides, sizeof(dbrushside_t) * g_numDBrushSides);
    g_dtexmatrix_checksum = FastChecksum(g_dtexmatrix, sizeof(dtexmatrix_t) * g_numDTexMatrix);
    g_dvisdata_checksum = FastChecksum(g_dvisdata, g_visdatasize);
    g_dlightdata_checksum = FastChecksum(g_dlightdata, g_lightdatasize);
    g_dentdata_checksum = FastChecksum(g_dentdata, g_entdatasize);
}

void LoadBSPFile(const char* filename)
{
    void* header;
    LoadFile(filename, &header);
    LoadBSPImage((dheader_t* const)header);
}

void AddLump(int len, FILE* bspfile, int lumpnum, const char* lumpname, void* data, dheader_t* header)
{
    lump_t* lump = &header->lumps[lumpnum];
    lump->offset = LittleLong(ftell(bspfile));
    lump->length = LittleLong(len);
    Verbose("Writing BSP lump %d (%s) %d bytes\n", lumpnum, lumpname, len);
    SafeWrite(bspfile, data, (len + 3) & ~3);
}

void WriteBSPFile(const char* const filename)
{
    dheader_t       outheader;
    dheader_t* header;
    FILE* bspfile;

    header = &outheader;
    memset(header, 0, sizeof(dheader_t));

    SwapBSPFile(true);

    header->version = LittleLong(BSPVERSION);

    bspfile = SafeOpenWrite(filename);
    Verbose("Writing temp BSP header (%d bytes)\n", sizeof(dheader_t));
    SafeWrite(bspfile, header, sizeof(dheader_t));         // overwritten later

    AddLump(sizeof(dplane_t) * g_numDPlanes, bspfile, LUMP_PLANES, "LUMP_PLANES", g_dplanes, header);
    AddLump(sizeof(dleaf_t) * g_numDLeafs, bspfile, LUMP_LEAFS, "LUMP_LEAFS", g_dleafs, header);
    AddLump(sizeof(dvertex_t) * g_numDVerts, bspfile, LUMP_VERTEXES, "LUMP_VERTS", g_dverts, header);
    AddLump(sizeof(dvertex_t) * g_numDVerts, bspfile, LUMP_NORMALS, "LUMP_NORMALS", g_dnormals, header);
    AddLump(sizeof(int) * g_numDIndices, bspfile, LUMP_INDICES, "LUMP_INDICES", g_dindices, header);
    AddLump(sizeof(dnode_t) * g_numDNodes, bspfile, LUMP_NODES, "LUMP_NODES", g_dnodes, header);
    AddLump(sizeof(dface_t) * g_numDFaces, bspfile, LUMP_FACES, "LUMP_FACES", g_dFaces, header);
    AddLump(sizeof(int) * g_numDMarkSurfaces, bspfile, LUMP_MARKSURFACES, "LUMP_MARKSURFACES", g_dmarksurfaces, header);
    AddLump(sizeof(int) * g_numDMarkBrushes, bspfile, LUMP_MARKBRUSHES, "LUMP_MARKBRUSHES", g_dmarkbrushes, header);
    AddLump(sizeof(dmodel_t) * g_numDModels, bspfile, LUMP_MODELS, "LUMP_MODELS", g_dmodels, header);
    AddLump(sizeof(texturename_t) * g_numDTextures, bspfile, LUMP_TEXTURES, "LUMP_TEXTURES", g_dtextures, header);
    AddLump(sizeof(texturename_t) * g_numDMaterials, bspfile, LUMP_MATERIALS, "LUMP_MATERIALS", g_dmaterials, header);
    AddLump(sizeof(dtexmatrix_t) * g_numDTexMatrix, bspfile, LUMP_TEXMATRIX, "LUMP_TEXMATRIX", g_dtexmatrix, header);
    AddLump(sizeof(dbrush_t) * g_numDBrushes, bspfile, LUMP_BRUSHES, "LUMP_BRUSHES", g_dbrushes, header);
    AddLump(sizeof(dbrushside_t) * g_numDBrushSides, bspfile, LUMP_BRUSHSIDES, "LUMP_BRUSHSIDES", g_dbrushsides, header);
    AddLump(g_lightdatasize, bspfile, LUMP_LIGHTING, "LUMP_LIGHTING", g_dlightdata, header);
    AddLump(g_visdatasize, bspfile, LUMP_VISIBILITY, "LUMP_VISIBILITY", g_dvisdata, header);
    AddLump(g_entdatasize, bspfile, LUMP_ENTITIES, "LUMP_ENTITIES", g_dentdata, header);
    Verbose("Writing final BSP header (%d bytes)\n", 148);

    fseek(bspfile, 0, SEEK_SET);
    SafeWrite(bspfile, header, sizeof(dheader_t));

    fclose(bspfile);
}

void SetLightStyles(mapinfo_t* ent)
{
    unsigned int             stylenum;
    const char* t;
    entity_t* e;
    unsigned int             i, j;
    char            value[10];
    char            lighttargets[MAX_SWITCHED_LIGHTS][MAX_LIGHTTARGETS_NAME];

    // any light that is controlled (has a targetname)
    // must have a unique style number generated for it

    stylenum = 0;
    for (i = 1; i < ent->numentities; i++)
    {
        e = ent->entities[i];

        t = ValueForKey(e, "classname");
        if (_strnicmp(t, "light", 5))
        {
            continue;
        }
        t = ValueForKey(e, "targetname");
        if (!t[0])
        {
            continue;
        }

        // find this targetname
        for (j = 0; j < stylenum; j++)
        {
            if (!strcmp(lighttargets[j], t))
            {
                break;
            }
        }
        if (j == stylenum)
        {
            hlassume(stylenum < MAX_SWITCHED_LIGHTS, assume_MAX_SWITCHED_LIGHTS);
            safe_strncpy(lighttargets[j], t, MAX_LIGHTTARGETS_NAME);
            stylenum++;
        }
        safe_snprintf(value, sizeof(value), "%i", 32 + j);
        SetKeyValue(e, "style", value);
    }

}

void WriteBSP(mapinfo_t* mapfile, const char* filename)
{
    SetModelNumbers(mapfile);
    SetLightStyles(mapfile);
    UnparseEntities(mapfile);
    WriteBSPFile(filename);
}

void WritePlanes()
{
    g_numDPlanes = gNumMappedPlanes;
    for (unsigned i = 0; i < g_numDPlanes; ++i)
    {
        plane_t& in = gMappedPlanes[i];
        dplane_t& out = g_dplanes[i];
        out.closest_axis = in.closest_axis;
        out.normal[0] = in.normal[0];
        out.normal[1] = in.normal[1];
        out.normal[2] = in.normal[2];
        out.dist = in.dist;
    }
}

void     SetModelNumbers(mapinfo_t* mapfile)
{
    unsigned int             i;
    unsigned int             models;
    char            value[10];

    models = 1;
    for (i = 1; i < mapfile->numentities; i++)
    {
        if (mapfile->entities[i]->numbrushes)
        {
            safe_snprintf(value, sizeof(value), "*%i", models);
            models++;
            SetKeyValue(mapfile->entities[i], "model", value);
        }
    }
}

void FinishBSPFile()
{
    WritePlanes();
    Verbose("--- FinishBSPFile ---\n");

    Verbose("Visible textures:\n");

    const char* texture = (const char*)g_dtextures;
    for (unsigned int i = 0; i < g_numDTextures; ++i)
    {
        int numrefs = g_TextureReferenceCount[i];
        Verbose("%11s %5i refs : '%s'\n", numrefs < 16 ? " -Warning- " : "", numrefs, texture);

        texture += sizeof(texturename_t);
    }

    if (g_chart)
        PrintBSPFileSizes();
}

void BeginBSPFile()
{
    memset(g_TextureReferenceCount, 0, sizeof(g_TextureReferenceCount));
    g_numDModels = 0;
    g_visdatasize = 0;
    g_lightdatasize = 0;
    g_entdatasize = 0;
    g_numDPlanes = 0;
    g_numDVerts = 0;
    g_numDIndices = 0;
    g_numDTextures = 0;
    g_numDMaterials = 0;
    g_numDTexMatrix = 0;
    g_numDFaces = 0;
    g_numDMarkSurfaces = 0;
    g_numDNodes = 0;
    g_numDLeafs = 1;
    g_dleafs[0].contents = CONTENTS_SOLID;
}

void StartCreatingBSP(mapinfo_t* mapfile)
{
    BeginBSPFile();

    if (mapfile->numentities)
    {
        WorldBSP(*mapfile->entities, mapfile, &g_dmodels[g_numDModels]);
        ++g_numDModels;
    }

    for (unsigned int i = 1; i < mapfile->numentities; ++i)
    {
        if (mapfile->entities[i]->numbrushes)
        {
            hlassume(g_numDModels < MAX_MAP_MODELS, assume_MAX_MAP_MODELS);
            ModelBSP(mapfile->entities[i], &g_dmodels[g_numDModels], g_numDModels);
            ++g_numDModels;
        }
    }

    FinishBSPFile();
}

void SetKeyValue(entity_t* ent, const char* const key, const char* const value)
{
    epair_t* ep;

    for (ep = ent->epairs; ep; ep = ep->next)
    {
        if (!strcmp(ep->key, key))
        {
            free(ep->value);
            ep->value = _strdup(value);
            return;
        }
    }
    ep = new epair_t;
    ep->next = ent->epairs;
    ent->epairs = ep;
    ep->key = _strdup(key);
    ep->value = _strdup(value);
}

void UnparseEntities(mapinfo_t* mapfile)
{
    char* buf;
    char* end;
    epair_t* ep;
    char            line[MAXTOKEN];
    unsigned int             i;

    buf = g_dentdata;
    end = buf;
    *end = 0;

    for (i = 0; i < mapfile->numentities; i++)
    {
        ep = mapfile->entities[i]->epairs;
        if (!ep)
        {
            continue;                                      // ent got removed
        }

        strcat(end, "{\n");
        end += 2;

        for (ep = mapfile->entities[i]->epairs; ep; ep = ep->next)
        {
            sprintf(line, "\"%s\" \"%s\"\n", ep->key, ep->value);
            strcat(end, line);
            end += strlen(line);
        }
        strcat(end, "}\n");
        end += 2;

        if (end > buf + MAX_MAP_ENTSTRING)
        {
            Error("Entity text too long");
        }
    }
    g_entdatasize = end - buf + 1;
}