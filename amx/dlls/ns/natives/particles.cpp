/* AMX Mod X 
 *   Natural Selection Module 
 * 
 * by the AMX Mod X Development Team 
 *
 * This file is part of AMX Mod X. 
 * 
 * 
 *  This program is free software; you can redistribute it and/or modify it 
 *  under the terms of the GNU General Public License as published by the 
 *  Free Software Foundation; either version 2 of the License, or (at 
 *  your option) any later version. 
 * 
 *  This program is distributed in the hope that it will be useful, but 
 *  WITHOUT ANY WARRANTY; without even the implied warranty of 
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 *  General Public License for more details. 
 * 
 *  You should have received a copy of the GNU General Public License 
 *  along with this program; if not, write to the Free Software Foundation, 
 *  Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 * 
 *  In addition, as a special exception, the author gives permission to 
 *  link the code of this program with the Half-Life Game Engine ("HL 
 *  Engine") and Modified Game Libraries ("MODs") developed by Valve, 
 *  L.L.C ("Valve"). You must obey the GNU General Public License in all 
 *  respects for all of the code used other than the HL Engine and MODs 
 *  from Valve. If you modify this file, you may extend this exception 
 *  to your version of the file, but you are not obligated to do so. If 
 *  you do not wish to do so, delete this exception statement from your 
 *  version. 
 */ 

#include "../sdk/amxxmodule.h"

#include "../ns.h"
#include "../utilfunctions.h"
#include "../NEW_Util.h"

#include "../CVector.h"
#include "../CString.h"
#include "../ParticleManager.h"

#define KVI(__KEY) PSKeyValueI(__KEY,amx,params)
#define KVF(__KEY) PSKeyValueF(__KEY,amx,params)
#define KVS(__KEY) PSKeyValueS(__KEY,amx,params)
#define NEXT params[__pcount++]

typedef enum partsystype_e
{
	PSYS_TYPE_INT,
	PSYS_TYPE_FLOAT,
	PSYS_TYPE_STRING
}partsystype;
typedef struct partsyskey_s
{
	const char	*Name;
	partsystype	 type;
}partsyskey;

cell PSKeyValueI(const char *name, AMX *amx, cell *params)
{
	if (params[1]==0)
	{
		MF_LogError(amx,AMX_ERR_NATIVE,"Invalid particle system handle provided!");
		return 0;
	}
	KeyValueData kvd;

	char StrData[1024];

	snprintf(StrData,sizeof(StrData)-1,"%d",params[2]);

	kvd.szClassName=const_cast<char *>(STRING(reinterpret_cast<edict_t *>(params[1])->v.classname));
	kvd.szKeyName=name;
	kvd.szValue=&StrData[0];
	kvd.fHandled=0;
	//printf("\"%s\" \"%s\"\n",kvd.szKeyName,kvd.szValue);

	MDLL_KeyValue(reinterpret_cast<edict_t *>(params[1]),&kvd);

	return 1;
}
cell PSKeyValueF(const char *name, AMX *amx, cell *params)
{
	if (params[1]==0)
	{
		MF_LogError(amx,AMX_ERR_NATIVE,"Invalid particle system handle provided!");
		return 0;
	}
	KeyValueData kvd;

	char StrData[1024];

	snprintf(StrData,sizeof(StrData)-1,"%f",amx_ctof2(params[2]));

	kvd.szClassName=const_cast<char *>(STRING(reinterpret_cast<edict_t *>(params[1])->v.classname));
	kvd.szKeyName=name;
	kvd.szValue=&StrData[0];
	kvd.fHandled=0;

	//printf("\"%s\" \"%s\"\n",kvd.szKeyName,kvd.szValue);

	MDLL_KeyValue(reinterpret_cast<edict_t *>(params[1]),&kvd);

	return 1;

}
cell PSKeyValueS(const char *name, AMX *amx, cell *params)
{
	if (params[1]==0)
	{
		MF_LogError(amx,AMX_ERR_NATIVE,"Invalid particle system handle provided!");
		return 0;
	}
	KeyValueData kvd;

	kvd.szClassName=const_cast<char *>(STRING(reinterpret_cast<edict_t *>(params[1])->v.classname));
	kvd.szKeyName=name;
	kvd.szValue=MF_GetAmxString(amx,params[2],0,NULL);
	kvd.fHandled=0;
	//printf("\"%s\" \"%s\"\n",kvd.szKeyName,kvd.szValue);

	MDLL_KeyValue(reinterpret_cast<edict_t *>(params[1]),&kvd);

	return 1;

}
static cell AMX_NATIVE_CALL ns_set_ps_name(AMX *amx, cell *params)
{
	return KVS("targetname");
}
static cell AMX_NATIVE_CALL ns_set_ps_sprite(AMX *amx, cell *params)
{
	return KVS("pSprite");
}
static cell AMX_NATIVE_CALL ns_set_ps_genrate(AMX *amx, cell *params)
{
	return KVI("pGenRate");
}
static cell AMX_NATIVE_CALL ns_set_ps_genshape(AMX *amx, cell *params)
{
	return KVI("pGenShape");
}
static cell AMX_NATIVE_CALL ns_set_ps_genshape_params(AMX *amx, cell *params)
{
	return KVS("pGenShapeParams");
}
static cell AMX_NATIVE_CALL ns_set_ps_spriteframes(AMX *amx, cell *params)
{
	return KVI("pSpriteNumFrames");
}
static cell AMX_NATIVE_CALL ns_set_ps_numparticles(AMX *amx, cell *params)
{
	return KVI("pNumParticles");
}
static cell AMX_NATIVE_CALL ns_set_ps_size(AMX *amx, cell *params)
{
	return KVF("pSize");
}
static cell AMX_NATIVE_CALL ns_set_ps_vel_params(AMX *amx, cell *params)
{
	return KVS("pVelParams");
}
static cell AMX_NATIVE_CALL ns_set_ps_vel_shape(AMX *amx, cell *params)
{
	return KVI("pVelShape");
}
static cell AMX_NATIVE_CALL ns_set_ps_sys_life(AMX *amx, cell *params)
{
	return KVF("pSystemLifetime");
}
static cell AMX_NATIVE_CALL ns_set_ps_particle_life(AMX *amx, cell *params)
{
	return KVF("pLifetime");
}
static cell AMX_NATIVE_CALL ns_set_ps_rendermode(AMX *amx, cell *params)
{
	return KVI("pRenderMode");
}
static cell AMX_NATIVE_CALL ns_set_ps_to_gen(AMX *amx, cell *params)
{
	return KVS("pPSToGen");
}
static cell AMX_NATIVE_CALL ns_set_ps_anim_speed(AMX *amx, cell *params)
{
	return KVI("pAnimationSpeed");
}
static cell AMX_NATIVE_CALL ns_set_ps_spawn_flags(AMX *amx, cell *params)
{
	return KVI("spawnflags");
}
static cell AMX_NATIVE_CALL ns_set_ps_base_color(AMX *amx, cell *params)
{
	return KVS("pBaseColor");
}
static cell AMX_NATIVE_CALL ns_set_ps_scale(AMX *amx, cell *params)
{
	return KVF("pScale");
}
static cell AMX_NATIVE_CALL ns_set_ps_max_alpha(AMX *amx, cell *params)
{
	return KVF("pMaxAlpha");
}
// ns_create_partsys(const name[], pGenShape, const pGenShapeParams[], pGenRate, const pSprite[], 
// pSpriteFrames, pNumParticles, Float:pSize, const pVelParams[], pVelShape, 
// Float:pSystemLifetime, Float:pParticleLifetime, pRenderMode, const pPSToGen[], pAnimationSpeed, pSpawnFlags)
static cell AMX_NATIVE_CALL ns_create_partsys(AMX *amx, cell *params)
{
	return (cell)CREATE_NAMED_ENTITY(MAKE_STRING("env_particles_custom"));
};
static cell AMX_NATIVE_CALL ns_spawn_ps(AMX *amx, cell *params)
{
	if (params[1]==0)
	{
		MF_LogError(amx, AMX_ERR_NATIVE, "Invalid particle system handle");
		return 0;
	}

	edict_t *Ent=reinterpret_cast<edict_t *>(params[1]);
	MDLL_Spawn(Ent);

	if (!Ent->free)
	{
		REMOVE_ENTITY(Ent);
	}
	return ParticleMan.Add(STRING(Ent->v.targetname),0);
}
// ns_fire_ps(Particle:id,Float:origin[3],Float:angles[3],flags=0)
static cell AMX_NATIVE_CALL ns_fire_partsys(AMX *amx, cell *params)
{
	float *origin=(float*)MF_GetAmxAddr(amx,params[2]);
	float *angles=(float*)MF_GetAmxAddr(amx,params[3]);

	ParticleMan.FireSystem(static_cast<int>(params[1]),origin,angles,static_cast<int>(params[4]));

	return 0;
};

static cell AMX_NATIVE_CALL ns_get_partsys_id(AMX *amx, cell *params)
{
	return ParticleMan.Find(MF_GetAmxString(amx,params[1],0,NULL));;
};

AMX_NATIVE_INFO particle_natives[] = {
	{ "ns_create_ps",				ns_create_partsys },
	{ "ns_set_ps_name",				ns_set_ps_name },
	{ "ns_set_ps_sprite",			ns_set_ps_sprite },
	{ "ns_set_ps_genrate",			ns_set_ps_genrate },
	{ "ns_set_ps_genshape",			ns_set_ps_genshape },
	{ "ns_set_ps_genshape_params",	ns_set_ps_genshape_params },
	{ "ns_set_ps_spriteframes",		ns_set_ps_spriteframes },
	{ "ns_set_ps_numparticles",		ns_set_ps_numparticles },
	{ "ns_set_ps_size",				ns_set_ps_size },
	{ "ns_set_ps_vel_params",		ns_set_ps_vel_params },
	{ "ns_set_ps_vel_shape",		ns_set_ps_vel_shape },
	{ "ns_set_ps_sys_life",			ns_set_ps_sys_life },
	{ "ns_set_ps_particle_life",	ns_set_ps_particle_life },
	{ "ns_set_ps_rendermode",		ns_set_ps_rendermode },
	{ "ns_set_ps_to_gen",			ns_set_ps_to_gen },
	{ "ns_set_ps_anim_speed",		ns_set_ps_anim_speed },
	{ "ns_set_ps_spawn_flags",		ns_set_ps_spawn_flags },
	{ "ns_set_ps_base_color",		ns_set_ps_base_color },
	{ "ns_set_ps_scale",			ns_set_ps_scale },
	{ "ns_set_ps_max_alpha",		ns_set_ps_max_alpha },
	{ "ns_spawn_ps",				ns_spawn_ps },

	{ "ns_fire_ps",					ns_fire_partsys },
	{ "ns_get_ps_id",				ns_get_partsys_id },

	{ NULL,								NULL }
};
void AddNatives_Particles()
{
	MF_AddNatives(particle_natives);
}
