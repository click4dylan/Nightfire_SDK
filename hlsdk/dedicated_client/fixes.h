#pragma once

extern void Fix_GUI_GetAction_Crash();
extern void Fix_Sound_Overflow();
extern void Fix_Engine_Bugs();
extern void Fix_GameDLL_Bugs();
extern void Fix_Gamespy();
extern void Fix_Water_Hull();
extern void Fix_RainDrop_WaterCollision();
extern void Fix_AI_TurnSpeed();
extern void Fix_AlertMessage_Crash();
extern void Fix_Con_DPrintf_StackSmash_Crash();
extern void Fix_Netchan();
extern void Fix_RateLimiter();
extern void Fix_UserInfoString();
extern void Fix_FpsCap();
extern void Fix_RateDesync();
extern int EV_GetTraceHull();
extern void GetImportantOffsets();
extern bool UTIL_CheckForWater_Hook(float startx, float starty, float startz, float endx, float endy, float endz, float* dest);