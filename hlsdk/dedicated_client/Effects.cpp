#include <Windows.h>
#include <vector>
#include <extdll.h>	
//#include <CVector.h>
//#include "CString.h"
//#include "entity.h"
//#include <gpglobals.h>
//#include <const.h>
//#include <util.h>
//#include <cbase.h>
#include "globals.h"
#include "pattern_scanner.h"
#include "MinHook/MinHook.h"
#include <event_api.h>
#include "MetaHook.h"
#include <usercmd.h>
#include <cvardef.h>
#include <clientsideentity.h>
#include <NightfireFileSystem.h>
//#include <r_studioint.h>

//#include <vector_types.h>
//#include <eiface.h>
#include <com_model.h>
#include <pmtrace.h>
#include "bzip2/bzlib.h"
#include <sizebuf.h>
#include <fragbuf.h>
#include <netchan.h>
#include <platformdll.h>
//#include <enginefuncs.h>
#include <globalvars.h>
#include <server.h>
#include <nightfire_pointers.h>
#include "nightfire_hooks.h"

extern vec3_t vec3_origin;

#include <r_efx.h>

//#define g_EffectsAPI ((efx_api_t*)0x4310BDB0)

//todo: R_InvalidateAllLightmaps for dlights

void(*g_oCL_ParseTEnt)() {};

void Hooked_CL_ParseTEnt(void)
{
	//return g_oCL_ParseTEnt();

	efx_api_t* g_EffectsAPI = g_Pointers.g_pCL_EngineFuncs->pEffectsAPI;

	int		type;
	vec3_t	pos;

	type = g_Pointers.MSG_ReadByte();
	switch (type)
	{
	default:
	{
		g_EngineAPI.Con_Printf("Unknown tempent type '%d', ignoring\n", type);
		break;
	}
#if 1
		//RESTORED TEMPENTS GEARBOX REMOVED:
	case TE_DLIGHT:
	{
		dlight_t* dlight = g_EffectsAPI->CL_AllocDlight(0);
		dlight->origin[0] = g_Pointers.MSG_ReadCoord();
		dlight->origin[1] = g_Pointers.MSG_ReadCoord();
		dlight->origin[2] = g_Pointers.MSG_ReadCoord();
		dlight->minlight = g_Pointers.MSG_ReadCoord();
		dlight->radius = (float)g_Pointers.MSG_ReadByte() * 10.0f;
		dlight->color.r = g_Pointers.MSG_ReadByte();
		dlight->color.g = g_Pointers.MSG_ReadByte();
		dlight->color.b = g_Pointers.MSG_ReadByte();
		dlight->die = (float)g_Pointers.MSG_ReadByte() * 0.1f + g_Pointers.g_pCL_EngineFuncs->GetClientTime();
		dlight->decay = (float)g_Pointers.MSG_ReadByte() * 10.0f;
		break;
	}
	//TODO: OTHERS
#endif
	//NIGHTFIRE TEMPENTS
	case TE_BEAMPOINTS:
	{
		pos[0] = g_Pointers.MSG_ReadCoord();
		pos[1] = g_Pointers.MSG_ReadCoord();
		pos[2] = g_Pointers.MSG_ReadCoord();

		vec3_t end;
		end[0] = g_Pointers.MSG_ReadCoord();
		end[1] = g_Pointers.MSG_ReadCoord();
		end[2] = g_Pointers.MSG_ReadCoord();

		int spriteindex = g_Pointers.MSG_ReadShort();
		int startingframe = g_Pointers.MSG_ReadByte();
		float framerate = (float)g_Pointers.MSG_ReadByte() * 0.1f;
		float life = (float)g_Pointers.MSG_ReadByte() * 0.1f;
		float linewidth = (float)g_Pointers.MSG_ReadByte() * 0.1f;
		float noise = (float)g_Pointers.MSG_ReadByte() * 0.01f;
		float red = (float)((unsigned int)g_Pointers.MSG_ReadByte()) / 255;
		float green = (float)((unsigned int)g_Pointers.MSG_ReadByte()) / 255;
		float blue = (float)((unsigned int)g_Pointers.MSG_ReadByte()) / 255;
		float brightness = (float)((unsigned int)g_Pointers.MSG_ReadByte()) / 255;
		float speed = (float)g_Pointers.MSG_ReadByte() * 0.1f;

		g_EffectsAPI->R_BeamPoints(pos, end, spriteindex, life, linewidth, noise, brightness, speed, startingframe, framerate, red, green, blue);
		break;
	}
	case TE_BEAMENTPOINT:
	{
		int startent = g_Pointers.MSG_ReadShort();

		vec3_t end;
		end[0] = g_Pointers.MSG_ReadCoord();
		end[1] = g_Pointers.MSG_ReadCoord();
		end[2] = g_Pointers.MSG_ReadCoord();

		int spriteindex = g_Pointers.MSG_ReadShort();
		int startingframe = g_Pointers.MSG_ReadByte();
		float framerate = (float)g_Pointers.MSG_ReadByte() * 0.1f;
		float life = (float)g_Pointers.MSG_ReadByte() * 0.1f;
		float linewidth = (float)g_Pointers.MSG_ReadByte() * 0.1f;
		float noise = (float)g_Pointers.MSG_ReadByte() * 0.01f;
		float red = (float)((unsigned int)g_Pointers.MSG_ReadByte()) / 255;
		float green = (float)((unsigned int)g_Pointers.MSG_ReadByte()) / 255;
		float blue = (float)((unsigned int)g_Pointers.MSG_ReadByte()) / 255;
		float brightness = (float)((unsigned int)g_Pointers.MSG_ReadByte()) / 255;
		float speed = (float)g_Pointers.MSG_ReadByte() * 0.1f;

		g_EffectsAPI->R_BeamEntPoint(startent, end, spriteindex, life, linewidth, noise, brightness, speed, startingframe, framerate, red, green, blue);
		break;
	}
	case TE_BEAMENTS:
	{
		int startent = g_Pointers.MSG_ReadShort();
		int endent = g_Pointers.MSG_ReadShort();

		int spriteindex = g_Pointers.MSG_ReadShort();
		int startingframe = g_Pointers.MSG_ReadByte();
		float framerate = (float)g_Pointers.MSG_ReadByte() * 0.1f;
		float life = (float)g_Pointers.MSG_ReadByte() * 0.1f;
		float linewidth = (float)g_Pointers.MSG_ReadByte() * 0.1f;
		float noise = (float)g_Pointers.MSG_ReadByte() * 0.01f;
		float red = (float)((unsigned int)g_Pointers.MSG_ReadByte()) / 255;
		float green = (float)((unsigned int)g_Pointers.MSG_ReadByte()) / 255;
		float blue = (float)((unsigned int)g_Pointers.MSG_ReadByte()) / 255;
		float brightness = (float)((unsigned int)g_Pointers.MSG_ReadByte()) / 255;
		float speed = (float)g_Pointers.MSG_ReadByte() * 0.1f;

		g_EffectsAPI->R_BeamEnts(startent, endent, spriteindex, life, linewidth, noise, brightness, speed, startingframe, framerate, red, green, blue);
		break;
	}
	case TE_EXPLOSION:
	{
		pos[0] = g_Pointers.MSG_ReadCoord();
		pos[1] = g_Pointers.MSG_ReadCoord();
		pos[2] = g_Pointers.MSG_ReadCoord();

		int spriteindex = g_Pointers.MSG_ReadShort();
		float scale = (float)g_Pointers.MSG_ReadByte() * 0.1f;
		float framerate = (float)g_Pointers.MSG_ReadByte();
		int flags = g_Pointers.MSG_ReadByte();
		if (scale != 0.0f)
		{
			TEMPENTITY* mod = g_Pointers.R_DefaultSprite(pos, spriteindex, framerate);
			g_EffectsAPI->R_Sprite_Explode(mod, scale, flags);
			if ((flags & TE_EXPLFLAG_NODLIGHTS) == 0)
			{
				dlight_t* dlight = g_EffectsAPI->CL_AllocDlight(0);
				dlight->origin[0] = pos[0];
				dlight->origin[1] = pos[1];
				dlight->origin[2] = pos[2];
				dlight->color.r = 250;
				dlight->color.g = 250;
				dlight->color.b = 150;
				dlight->radius = 200.0f;
				dlight->decay = 800.0f;
				dlight->die = g_Pointers.g_pCL_EngineFuncs->GetClientTime() + 0.01;

				dlight_t* dlight2 = g_EffectsAPI->CL_AllocDlight(0);
				dlight2->origin[0] = pos[0];
				dlight2->origin[1] = pos[1];
				dlight2->origin[2] = pos[2];
				dlight2->color.r = 255;
				dlight2->color.g = 190;
				dlight2->color.b = 40;
				dlight2->radius = 150.0f;
				dlight2->decay = 200.0f;
				dlight2->die = g_Pointers.g_pCL_EngineFuncs->GetClientTime() + 1.0;
			}
		}
		if ((flags & TE_EXPLFLAG_NOSOUND) == 0)
		{
			int num = g_Pointers.g_pCL_EngineFuncs->RandomLong(0, 2);
			switch (num)
			{
			case 0:
				g_Pointers.S_StartDynamicSound(-1, 0, *g_Pointers.cl_sfx_r_exp1, pos, 1.0f, 0.3f, 0, 100);
				break;
			case 1:
				g_Pointers.S_StartDynamicSound(-1, 0, *g_Pointers.cl_sfx_r_exp3, pos, 1.0f, 0.3f, 0, 100);
				break;
			case 2:
				g_Pointers.S_StartDynamicSound(-1, 0, *g_Pointers.cl_sfx_r_exp2, pos, 1.0f, 0.3f, 0, 100);
				break;
			}
		}
		break;
	}
	case TE_SMOKE_COLOR:
	{
		pos[0] = g_Pointers.MSG_ReadCoord();
		pos[1] = g_Pointers.MSG_ReadCoord();
		pos[2] = g_Pointers.MSG_ReadCoord();

		int spriteindex = g_Pointers.MSG_ReadShort();
		float scale = (float)g_Pointers.MSG_ReadByte() * 0.1f;
		float framerate = (float)g_Pointers.MSG_ReadByte();
		float color = (float)g_Pointers.MSG_ReadByte();

		TEMPENTITY* mod = g_Pointers.R_DefaultSprite(pos, spriteindex, framerate);
		g_EffectsAPI->R_Sprite_Smoke_Color(mod, scale, color);
		break;
	}
	case TE_SMOKE:
	{
		pos[0] = g_Pointers.MSG_ReadCoord();
		pos[1] = g_Pointers.MSG_ReadCoord();
		pos[2] = g_Pointers.MSG_ReadCoord();

		int spriteindex = g_Pointers.MSG_ReadShort();
		float scale = (float)g_Pointers.MSG_ReadByte() * 0.1f;
		float framerate = (float)g_Pointers.MSG_ReadByte();

		TEMPENTITY* mod = g_Pointers.R_DefaultSprite(pos, spriteindex, framerate);
		g_EffectsAPI->R_Sprite_Smoke(mod, scale);
		break;
	}
	case TE_TRACER:
	{
		pos[0] = g_Pointers.MSG_ReadCoord();
		pos[1] = g_Pointers.MSG_ReadCoord();
		pos[2] = g_Pointers.MSG_ReadCoord();

		vec3_t end;
		end[0] = g_Pointers.MSG_ReadCoord();
		end[1] = g_Pointers.MSG_ReadCoord();
		end[2] = g_Pointers.MSG_ReadCoord();
		g_EffectsAPI->R_TracerEffect(pos, end);
		break;
	}
	case TE_LIGHTNING:
	{
		pos[0] = g_Pointers.MSG_ReadCoord();
		pos[1] = g_Pointers.MSG_ReadCoord();
		pos[2] = g_Pointers.MSG_ReadCoord();

		vec3_t end;
		end[0] = g_Pointers.MSG_ReadCoord();
		end[1] = g_Pointers.MSG_ReadCoord();
		end[2] = g_Pointers.MSG_ReadCoord();

		float life = (float)g_Pointers.MSG_ReadByte() * 0.1f;
		float width = (float)g_Pointers.MSG_ReadByte() * 0.1f;
		float noise = (float)g_Pointers.MSG_ReadByte() * 0.01f;
		int spriteindex = g_Pointers.MSG_ReadShort();

		g_EffectsAPI->R_BeamLightning(pos, end, spriteindex, life, width, noise, 0.6f, 3.5f);
		break;
	}
	case TE_SPARKS:
	{
		pos[0] = g_Pointers.MSG_ReadCoord();
		pos[1] = g_Pointers.MSG_ReadCoord();
		pos[2] = g_Pointers.MSG_ReadCoord();
		g_EffectsAPI->R_SparkEffect(pos, 8, -200, 200);
		break;
	}
	case TE_SPRITE:
	{
		pos[0] = g_Pointers.MSG_ReadCoord();
		pos[1] = g_Pointers.MSG_ReadCoord();
		pos[2] = g_Pointers.MSG_ReadCoord();

		int spriteindex = g_Pointers.MSG_ReadShort();
		float scale = (float)g_Pointers.MSG_ReadByte() * 0.1f;
		float brightness = (float)g_Pointers.MSG_ReadByte() / 255;
		g_EffectsAPI->R_TempSprite(pos, vec3_origin, scale, spriteindex, kRenderTransAlpha, kRenderFxNone, brightness, 0.0f, 256);
		break;
	}
	case TE_ELIGHT:
	{
		int key = g_Pointers.MSG_ReadShort();
		dlight_t* elight = g_EffectsAPI->CL_AllocElight(key);
		elight->origin[0] = g_Pointers.MSG_ReadCoord();
		elight->origin[1] = g_Pointers.MSG_ReadCoord();
		elight->origin[2] = g_Pointers.MSG_ReadCoord();
		elight->radius = g_Pointers.MSG_ReadCoord();
		elight->color.r = g_Pointers.MSG_ReadByte();
		elight->color.g = g_Pointers.MSG_ReadByte();
		elight->color.b = g_Pointers.MSG_ReadByte();
		float life = (float)g_Pointers.MSG_ReadByte() * 0.1f;
		elight->die = life + g_Pointers.g_pCL_EngineFuncs->GetClientTime();
		elight->decay = g_Pointers.MSG_ReadCoord();
		if (life != 0.0f)
			elight->decay /= life;
		break;
	}
	case TE_TEXTMESSAGE:
	{
		g_Pointers.CL_ParseTextMessage();
		break;
	}
	case TE_SHOWLINE:
	{
		pos[0] = g_Pointers.MSG_ReadCoord();
		pos[1] = g_Pointers.MSG_ReadCoord();
		pos[2] = g_Pointers.MSG_ReadCoord();

		vec3_t end;
		end[0] = g_Pointers.MSG_ReadCoord();
		end[1] = g_Pointers.MSG_ReadCoord();
		end[2] = g_Pointers.MSG_ReadCoord();

		g_EffectsAPI->R_ShowLine(pos, end);
		break;
	}
	case TE_MODEL:
	{
		pos[0] = g_Pointers.MSG_ReadCoord();
		pos[1] = g_Pointers.MSG_ReadCoord();
		pos[2] = g_Pointers.MSG_ReadCoord();

		vec3_t dir;
		dir[0] = g_Pointers.MSG_ReadCoord();
		dir[1] = g_Pointers.MSG_ReadCoord();
		dir[2] = g_Pointers.MSG_ReadCoord();

		vec3_t end;
		end[0] = 0.0f;
		end[1] = g_Pointers.MSG_ReadAngle();
		end[2] = 0.0f;

		int modelindex = g_Pointers.MSG_ReadShort();
		int soundtype = g_Pointers.MSG_ReadByte();
		float life = (float)g_Pointers.MSG_ReadByte() * 0.1f;

		g_EffectsAPI->R_TempModel(pos, dir, end, life, modelindex, soundtype);
		break;
	}
	case TE_BREAKMODEL:
	{
		pos[0] = g_Pointers.MSG_ReadCoord();
		pos[1] = g_Pointers.MSG_ReadCoord();
		pos[2] = g_Pointers.MSG_ReadCoord();

		vec3_t size;
		size[0] = g_Pointers.MSG_ReadCoord();
		size[1] = g_Pointers.MSG_ReadCoord();
		size[2] = g_Pointers.MSG_ReadCoord();

		vec3_t dir;
		dir[0] = g_Pointers.MSG_ReadCoord();
		dir[1] = g_Pointers.MSG_ReadCoord();
		dir[2] = g_Pointers.MSG_ReadCoord();

		float random = (float)g_Pointers.MSG_ReadByte() * 10.0f;
		int modelindex = g_Pointers.MSG_ReadShort();
		int count = g_Pointers.MSG_ReadByte();
		float life = (float)g_Pointers.MSG_ReadByte() * 0.1f;
		int flags = g_Pointers.MSG_ReadByte();

		g_EffectsAPI->R_BreakModel(pos, size, dir, random, life, count, modelindex, flags);
		break;
	}
	case TE_ARMOR_RICOCHET:
	{
		pos[0] = g_Pointers.MSG_ReadCoord();
		pos[1] = g_Pointers.MSG_ReadCoord();
		pos[2] = g_Pointers.MSG_ReadCoord();
		float scale = (float)g_Pointers.MSG_ReadByte() * 0.1f;
		g_EffectsAPI->R_RicochetSprite(pos, *g_Pointers.cl_sprite_ricochet, 0.1f, scale);
		break;
	}
	case TE_BUBBLES:
	{
		vec3_t mins;
		mins[0] = g_Pointers.MSG_ReadCoord();
		mins[1] = g_Pointers.MSG_ReadCoord();
		mins[2] = g_Pointers.MSG_ReadCoord();

		vec3_t maxs;
		maxs[0] = g_Pointers.MSG_ReadCoord();
		maxs[1] = g_Pointers.MSG_ReadCoord();
		maxs[2] = g_Pointers.MSG_ReadCoord();

		float height = g_Pointers.MSG_ReadCoord();
		int spriteindex = g_Pointers.MSG_ReadShort();
		int count = g_Pointers.MSG_ReadByte();
		float speed = g_Pointers.MSG_ReadCoord();
		g_EffectsAPI->R_Bubbles(mins, maxs, height, spriteindex, count, speed);
		break;
	}
	case TE_BUBBLETRAIL:
	{
		pos[0] = g_Pointers.MSG_ReadCoord();
		pos[1] = g_Pointers.MSG_ReadCoord();
		pos[2] = g_Pointers.MSG_ReadCoord();

		vec3_t end;
		end[0] = g_Pointers.MSG_ReadCoord();
		end[1] = g_Pointers.MSG_ReadCoord();
		end[2] = g_Pointers.MSG_ReadCoord();

		float height = g_Pointers.MSG_ReadCoord();
		int spriteindex = g_Pointers.MSG_ReadShort();
		int count = g_Pointers.MSG_ReadByte();
		float speed = g_Pointers.MSG_ReadCoord();
		g_EffectsAPI->R_BubbleTrail(pos, end, height, spriteindex, count, speed);
		break;
	}
	case TE_SMOKETRAIL:
	{
		pos[0] = g_Pointers.MSG_ReadCoord();
		pos[1] = g_Pointers.MSG_ReadCoord();
		pos[2] = g_Pointers.MSG_ReadCoord();

		vec3_t end;
		end[0] = g_Pointers.MSG_ReadCoord();
		end[1] = g_Pointers.MSG_ReadCoord();
		end[2] = g_Pointers.MSG_ReadCoord();

		int spriteindex = g_Pointers.MSG_ReadShort();
		int count = g_Pointers.MSG_ReadByte();
		int speed = (int)g_Pointers.MSG_ReadByte() * 2;
		int random = g_Pointers.MSG_ReadByte();
		float scale = (float)g_Pointers.MSG_ReadByte() * 0.1f;
		float life = (float)g_Pointers.MSG_ReadByte() * 0.1f;
		g_EffectsAPI->R_Sprite_Smoke_Trail(pos, end, spriteindex, count, speed, scale, life, random);
		break;
	}
	case TE_SPRITE_SPRAY_LIT:
	{
		pos[0] = g_Pointers.MSG_ReadCoord();
		pos[1] = g_Pointers.MSG_ReadCoord();
		pos[2] = g_Pointers.MSG_ReadCoord();

		vec3_t dir;
		dir[0] = g_Pointers.MSG_ReadCoord();
		dir[1] = g_Pointers.MSG_ReadCoord();
		dir[2] = g_Pointers.MSG_ReadCoord();

		int spriteindex = g_Pointers.MSG_ReadShort();
		int count = g_Pointers.MSG_ReadByte();
		int speed = (int)g_Pointers.MSG_ReadByte() * 2;
		int random = g_Pointers.MSG_ReadByte();
		int color = g_Pointers.MSG_ReadByte();
		float scale = (float)g_Pointers.MSG_ReadByte() * 0.1f;
		float brightness = (float)g_Pointers.MSG_ReadByte() * 0.1f;
		g_EffectsAPI->R_Sprite_Spray_Lit(pos, dir, spriteindex, count, speed, random, color, scale, brightness);
		break;
	}
	case TE_BSPDECAL:
	{
		pos[0] = g_Pointers.MSG_ReadCoord();
		pos[1] = g_Pointers.MSG_ReadCoord();
		pos[2] = g_Pointers.MSG_ReadCoord();

		int flags = 1;
		float decalsize = g_Pointers.MSG_ReadCoord();
		int textureindex = g_Pointers.MSG_ReadShort();
		int entityindex = g_Pointers.MSG_ReadShort();
		int modelindex = 0;
		if (entityindex)
			modelindex = g_Pointers.MSG_ReadShort();

		if (entityindex >= *g_Pointers.cl_maxents)
			g_Pointers.Sys_Error("Decal: entity = %i");

		static ConsoleVariable* r_decals = g_Pointers.g_pEngineFuncs->pfnGetConsoleVariableGame("r_decals");
		if (r_decals->getValue() != 0.0f)
			g_EffectsAPI->R_DecalShoot(textureindex, entityindex, modelindex, pos, flags);
		break;
	}
	case TE_DECAL:
	{
		pos[0] = g_Pointers.MSG_ReadCoord();
		pos[1] = g_Pointers.MSG_ReadCoord();
		pos[2] = g_Pointers.MSG_ReadCoord();

		int flags = 0;
		float decalsize = g_Pointers.MSG_ReadCoord();
		int textureindex = g_Pointers.MSG_ReadByte();
		int entityindex = g_Pointers.MSG_ReadShort();

		if (entityindex >= *g_Pointers.cl_maxents)
			g_Pointers.Sys_Error("Decal: entity = %i");

		static ConsoleVariable* r_decals = g_Pointers.g_pEngineFuncs->pfnGetConsoleVariableGame("r_decals");
		if (r_decals->getValue() != 0.0f)
			g_EffectsAPI->R_DecalShoot(textureindex, entityindex, 0, pos, flags);
		break;
	}
	case TE_WORLDDECAL:
	{
		pos[0] = g_Pointers.MSG_ReadCoord();
		pos[1] = g_Pointers.MSG_ReadCoord();
		pos[2] = g_Pointers.MSG_ReadCoord();

		int flags = 0;
		int entityindex = 0;
		float decalsize = g_Pointers.MSG_ReadCoord();
		int textureindex = g_Pointers.MSG_ReadByte();

		if (entityindex >= *g_Pointers.cl_maxents)
			g_Pointers.Sys_Error("Decal: entity = %i");

		static ConsoleVariable* r_decals = g_Pointers.g_pEngineFuncs->pfnGetConsoleVariableGame("r_decals");
		if (r_decals->getValue() != 0.0f)
			g_EffectsAPI->R_DecalShoot(textureindex, entityindex, 0, pos, flags);
		break;
	}
	case TE_WORLDDECALHIGH:
	{
		pos[0] = g_Pointers.MSG_ReadCoord();
		pos[1] = g_Pointers.MSG_ReadCoord();
		pos[2] = g_Pointers.MSG_ReadCoord();

		int flags = 0;
		int entityindex = 0;
		float decalsize = g_Pointers.MSG_ReadCoord();
		int textureindex = g_Pointers.MSG_ReadByte() + 256;

		if (entityindex >= *g_Pointers.cl_maxents)
			g_Pointers.Sys_Error("Decal: entity = %i");

		static ConsoleVariable* r_decals = g_Pointers.g_pEngineFuncs->pfnGetConsoleVariableGame("r_decals");
		if (r_decals->getValue() != 0.0f)
			g_EffectsAPI->R_DecalShoot(textureindex, entityindex, 0, pos, flags);
		break;
	}
	case TE_DECALHIGH:
	{
		pos[0] = g_Pointers.MSG_ReadCoord();
		pos[1] = g_Pointers.MSG_ReadCoord();
		pos[2] = g_Pointers.MSG_ReadCoord();

		int flags = 0;
		float decalsize = g_Pointers.MSG_ReadCoord();
		int textureindex = g_Pointers.MSG_ReadByte() + 256;
		int entityindex = g_Pointers.MSG_ReadShort();

		if (entityindex >= *g_Pointers.cl_maxents)
			g_Pointers.Sys_Error("Decal: entity = %i");

		static ConsoleVariable* r_decals = g_Pointers.g_pEngineFuncs->pfnGetConsoleVariableGame("r_decals");
		if (r_decals->getValue() != 0.0f)
			g_EffectsAPI->R_DecalShoot(textureindex, entityindex, 0, pos, flags);

		break;
	}
	};
}

// Fix missing tempents!
void Fix_TempEnts()
{
	if (!g_Pointers.CL_ParseTEnt)
		return;
	HookFunctionWithMinHook((void*)g_Pointers.CL_ParseTEnt, Hooked_CL_ParseTEnt, (void**)&g_oCL_ParseTEnt);
}