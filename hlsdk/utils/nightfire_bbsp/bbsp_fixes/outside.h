#pragma once
#include "bsp_structs.h"

extern node_t* FillOutside(mapinfo_t* mapfile, node_t* node, int pass_num);
extern void PrintLeakInfoIfLeaked(mapinfo_t* mapfile, int pass_num);