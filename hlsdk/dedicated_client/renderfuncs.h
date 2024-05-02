#pragma once
#include <beamdef.h>

enum DevTextColor : int
{
	WHITE = 0,
	RED = 1,
	ORANGE = 2,
	YELLOW = 3,
	GREEN = 4,
	PURPLE = 5
};
inline void DrawDevText(DevTextColor color, int x, int y, const char* text)
{
	__asm
	{
		mov eax, color
		push text
		push y
		push x
		mov ecx, 0x43056180
		call ecx
		add esp, 12
	}
}
extern int (*SV_StudioSetupBones)(struct model_s* model, float frame, int sequence, const float* angles, const float* origin, const unsigned char* controller, const unsigned char* blending, int bone);
extern struct studiohdr_s* (*Mod_Extradata)(struct model_s* model);
extern struct studiohdr_s** pstudiohdr;
extern void (*PM_PlayerTrace)(struct pmtrace_s* dest, const float* start, const float* end, int brushflags, int traceflags, int ignore_pe);
extern void (*PM_DrawPhysEntBBox)(int num, int pcolor, float life);
extern void(*R_DrawBeam)(BEAM* beam, float delta);

extern void Hook_SCR_ConnectMsg();
extern void Hook_CL_PrecacheResources();