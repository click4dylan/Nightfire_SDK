#pragma once
#include "bsp_structs.h"

extern const char* ValueForKey(const entity_t* const ent, const char* const key);
extern void GetVectorForKey(const entity_t* const ent, const char* const key, vec3_t vec);
extern int ArrayUsage(int size_of_item, const char* name_of_array, int num_items, int max_items);
extern void PrintBSPFileSizes();
extern void LoadBSPFile(const char* filename);
extern int      FastChecksum(const void* const buffer, int bytes);
extern int      CopyLump(int lump, void* dest, int size, const dheader_t* const header);
extern void LoadBSPImage(dheader_t* const header);
extern void WriteBSPFile(const char* const filename);
extern void     AddLump(int len, FILE* bspfile, int lumpnum, const char* lumpname, void* data, dheader_t* header);
extern void WriteBSP(entinfo_t* entinfo, const char* filename);
extern void     SetLightStyles(entinfo_t* ent);
extern void StartCreatingBSP(entinfo_t* entinfo);
extern void WritePlanes();
extern void BeginBSPFile();
extern void     SetModelNumbers(entinfo_t* entinfo);
extern void            SetKeyValue(entity_t* ent, const char* const key, const char* const value);
extern void            UnparseEntities(entinfo_t* entinfo);