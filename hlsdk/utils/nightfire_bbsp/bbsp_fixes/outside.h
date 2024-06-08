#pragma once
#include "bsp_structs.h"

extern node_t* FillOutside(entinfo_t* entinfo, node_t* node, int pass_num);
extern void PrintLeakInfoIfLeaked(entinfo_t* entinfo, int pass_num);