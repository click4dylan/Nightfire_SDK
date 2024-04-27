#include <Windows.h>
#include <vector>
#include "globals.h"
#include "pattern_scanner.h"
#include "MinHook/MinHook.h"
#include <event_api.h>
#include "MetaHook.h"
#include <cvardef.h>
//typedef float vec_t;
//typedef float vec2_t[2];
//typedef float vec3_t[3];
//#include <eiface.h>
#include <extdll.h>
#include <com_model.h>
#include <pmtrace.h>
#include "bzip2/bzlib.h"
#include <sizebuf.h>
#include <fragbuf.h>
#include <netchan.h>
#include <platformdll.h>
//#include <enginefuncs.h>
#include <globalvars.h>
#include "renderfuncs.h"
#include <bsprender.h>
#include <nightfire_pointers.h>

enum
{
	BEAM_POINTS = 0,
	BEAM_ENTPOINT,
	BEAM_ENTS,
	BEAM_HOSE,
};

int beam_sprite = -1;

int (*SV_StudioSetupBones)(struct model_s* model, float frame, int sequence, const float* angles, const float* origin, const unsigned char* controller, const unsigned char* blending, int bone) = (int (*)(struct model_s*, float, int, const float*, const float*, const unsigned char*, const unsigned char*, int))0x4307B8E0;
struct studiohdr_s* (*Mod_Extradata)(struct model_s* model) = (struct studiohdr_s* (*)(struct model_s*))0x4304B3D0;
struct studiohdr_s** pstudiohdr = (struct studiohdr_s**)0x44905D80;
void (*PM_PlayerTrace)(struct pmtrace_s* dest, const float* start, const float* end, int brushflags, int traceflags, int ignore_pe) = (void(*)(struct pmtrace_s*, const float*, const float*, int, int, int))0x4306B320;
void (*PM_DrawPhysEntBBox)(int num, int pcolor, float life) = (void(*)(int, int, float))0x4105DC30;
void (*R_DrawBeam)(BEAM* beam, float delta) = (void(*)(BEAM*, float))0x43076900;

void RunRenderCode()
{
	if (g_MetaAudioDllHinst)
	{
		static std::vector<BSPLine>& (*GetMapAudioMesh)() = (std::vector<BSPLine>&(*)())GetProcAddress((HMODULE)g_MetaAudioDllHinst, "GetMapAudioMesh");
		if (GetMapAudioMesh && beam_sprite != -1)
		{
			std::vector<BSPLine>& mesh = GetMapAudioMesh();
			for (auto& line : mesh)
			{
				BEAM beam;
				memset(&beam, 0, sizeof(BEAM));
				beam.source[0] = line.start[0];
				beam.source[1] = line.start[1];
				beam.source[2] = line.start[2];
				beam.target[0] = line.end[0];
				beam.target[1] = line.end[1];
				beam.target[2] = line.end[2];
				beam.delta[0] = (line.end[0] - line.start[0]);
				beam.delta[1] = (line.end[1] - line.start[1]);
				beam.delta[2] = (line.end[2] - line.start[2]);
				beam.modelIndex = beam_sprite;
				beam.type = BEAM_POINTS;
				beam.die = 0;
				beam.r = line.red / 255;
				beam.g = line.green / 255;
				beam.b = line.blue / 255;
				beam.segments = 2;
				beam.brightness = line.alpha / 255;
				beam.flags = FBEAM_ISACTIVE;
				beam.speed = 1;
				beam.frameRate = 1;
				beam.width = 5;
				beam.frameCount = 1;
				R_DrawBeam(&beam, 0);
			}
		}
	}
}

#include "clientdll_funcs.h"

void RunTestCode()
{

	if (gViewPort && gViewPort->singleplayerscoreboard)
	{
		static bool initial_visible_state = false;
		static bool first_press = true;
		static bool pressing = false;

		if (GetAsyncKeyState(VK_F1))
		{

			if (first_press)
			{
				first_press = false;
				initial_visible_state = gViewPort->singleplayerscoreboard->is_visible;
				gViewPort->singleplayerscoreboard->setVisible(true);
			}

			pressing = true;
		}
		else if (pressing)
		{
			first_press = true;
			pressing = false;
			gViewPort->singleplayerscoreboard->setVisible(initial_visible_state);
		}
	}
}

__declspec(naked) void Hooked_SCR_ConnectMsg()
{
	__asm
	{
		call RunRenderCode
		call RunTestCode
		SUB ESP, 0x0C
		MOV ECX, 0x449B03F4
		MOV EAX, 0x430568D8
		jmp eax
	}
}

void(*g_oCL_PrecacheResources)();
void CL_PrecacheResources()
{
	//ConsoleVariable* egon_amplitude = (ConsoleVariable*)0x4310F998;
	//float val = egon_amplitude->getValue();
	//egon_amplitude->setValueFloat(1.0f);
	//g_pEngineFuncs->pfnCreateConsoleVariableGame(IConsoleVariable(CVAR_FLOAT, "egon_amplitude", "", "1.0", FCVAR_EXTDLL));
	g_oCL_PrecacheResources();
	beam_sprite = g_Pointers.g_pEngineFuncs->pfnPrecacheModel("sprites/laser_beam.spz");
}

void Hook_CL_PrecacheResources()
{
	HookFunctionWithMinHook((void*)0x43033DC0, (void*)&CL_PrecacheResources, (void**)&g_oCL_PrecacheResources);
}

void Hook_SCR_ConnectMsg()
{
	PlaceJMP((BYTE*)0x430568D0, (DWORD)&Hooked_SCR_ConnectMsg, 8);
}