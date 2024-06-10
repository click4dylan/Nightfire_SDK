#pragma once
#include "bsp_structs.h"

extern const char* ValueForKey(const entity_t* const ent, const char* const key);
extern void GetVectorForKey(const entity_t* const ent, const char* const key, vec3_t vec);
extern int ArrayUsage(int size_of_item, const char* name_of_array, int num_items, int max_items);
extern void PrintBSPFileSizes();
extern void LoadBSPFile(const char* filename);
extern int FastChecksum(const void* const buffer, int bytes);
extern int CopyLump(int lump, void* dest, int size, const dheader_t* const header);
extern void LoadBSPImage(dheader_t* const header);
extern void WriteBSPFile(const char* const filename);
extern void WriteBSP(mapinfo_t* mapfile, const char* filename);
extern void AddLump(int len, FILE* bspfile, int lumpnum, const char* lumpname, void* data, dheader_t* header);
extern void SetLightStyles(mapinfo_t* ent);
extern void StartCreatingBSP(mapinfo_t* mapfile);
extern void WritePlanes();
extern void BeginBSPFile();
extern void SetModelNumbers(mapinfo_t* mapfile);
extern void SetKeyValue(entity_t* ent, const char* const key, const char* const value);
extern void UnparseEntities(mapinfo_t* mapfile);