#pragma once

extern void Fix_GUI_GetAction_Crash();
extern void Fix_Sound_Overflow();
extern void Fix_Engine_Bugs();
extern void Fix_Gamespy();
extern void Fix_Water_Hull();
extern void Fix_RainDrop_WaterCollision();
extern void GetImportantOffsets();
extern int EV_GetTraceHull();
extern bool UTIL_CheckForWater_Hook(float startx, float starty, float startz, float endx, float endy, float endz, float* dest);