#pragma once
#include "bsp_structs.h"

#ifdef MERGE
extern face_t* TryMerge(face_t* f1, face_t* f2);
extern face_t* MergeFaceToList(face_t* face, face_t* list);
extern face_t* FreeMergeListScraps(face_t* merged);
extern void MergePlaneFaces(face_t** list);
#endif