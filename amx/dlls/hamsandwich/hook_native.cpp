/* Ham Sandwich
 *   Copyright 2007
 *   By the AMX Mod X Development Team
 *
 *  Ham Sandwich is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation; either version 2 of the License, or (at
 *  your option) any later version.
 *
 *  Ham Sandwich is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Ham Sandwich; if not, write to the Free Software Foundation,
 *  Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *  In addition, as a special exception, the author gives permission to
 *  link the code of Ham Sandwich with the Half-Life Game Engine ("HL
 *  Engine") and Modified Game Libraries ("MODs") developed by Valve,
 *  L.L.C ("Valve"). You must obey the GNU General Public License in all
 *  respects for all of the code used other than the HL Engine and MODs
 *  from Valve. If you modify this file, you may extend this exception
 *  to your version of the file, but you are not obligated to do so. If
 *  you do not wish to do so, delete this exception statement from your
 *  version.
 */
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <extdll.h>
#include "amxxmodule.h"


#include "CVector.h"

#include "hook.h"
#include "forward.h"
#include "hook_callbacks.h"
#include "call_funcs.h"
#include "hook_create.h"
#include "offsets.h"
#include "hooklist.h"
#include "ham_utils.h"

OffsetManager Offsets;

bool gDoForwards=true;

CVector<Hook *> hooks[HAM_LAST_ENTRY_DONT_USE_ME_LOL];


#define V(__KEYNAME, __STUFF__) 0, 0, __KEYNAME, RT_##__STUFF__, PC_##__STUFF__, reinterpret_cast<void *>(Hook_##__STUFF__), Create_##__STUFF__, Call_##__STUFF__

hook_t hooklist[] =
{
	{ V("spawn",					Void_Void) },
	{ V("precache",					Void_Void) },
	{ V("keyvalue",					Void_Int) },
	{ V("objectcaps",				Int_Void) },
	{ V("activate",					Void_Void) },
	{ V("setobjectcollisionbox",	Void_Void) },
	{ V("classify",					Int_Void) },
	{ V("deathnotice",				Void_Entvar) },
	{ V("traceattack",				Void_Entvar_Float_Vector_Trace_Int) },
	{ V("takedamage",				Int_Entvar_Entvar_Float_Int) },
	{ V("takehealth",				Int_Float_Int) },
	{ V("killed",					Void_Entvar_Int) },
	{ V("bloodcolor",				Int_Void) },
	{ V("tracebleed",				Void_Float_Vector_Trace_Int) },
	{ V("istriggered",				Int_Cbase) },
	{ V("mymonsterpointer",			Cbase_Void) },
	{ V("mysquadmonsterpointer",	Cbase_Void) },
	{ V("gettogglestate",			Int_Void) },
	{ V("addpoints",				Void_Int_Int) },
	{ V("addpointstoteam",			Void_Int_Int) },
	{ V("addplayeritem",			Int_Cbase) },
	{ V("removeplayeritem",			Int_Cbase) },
	{ V("giveammo",					Int_Int_Str_Int) },
	{ V("getdelay",					Float_Void) },
	{ V("ismoving",					Int_Void) },
	{ V("overridereset",			Void_Void) },
	{ V("damagedecal",				Int_Int) },
	{ V("settogglestate",			Void_Int) },
	{ V("startsneaking",			Void_Void) },
	{ V("stopsneaking",				Void_Void) },
	{ V("oncontrols",				Int_Entvar) },
	{ V("issneaking",				Int_Void) },
	{ V("isalive",					Int_Void) },
	{ V("isbspmodel",				Int_Void) },
	{ V("reflectgauss",				Int_Void) },
	{ V("hastarget",				Int_Int) },
	{ V("isinworld",				Int_Void) },
	{ V("isplayer",					Int_Void) },
	{ V("isnetclient",				Int_Void) },
	{ V("teamid",					Str_Void) },
	{ V("getnexttarget",			Cbase_Void) },
	{ V("think",					Void_Void) },
	{ V("touch",					Void_Cbase) },
	{ V("use",						Void_Cbase_Cbase_Int_Float) },
	{ V("blocked",					Void_Cbase) },
	{ V("respawn",					Cbase_Void) },
	{ V("updateowner",				Void_Void) },
	{ V("fbecomeprone",				Int_Void) },
	{ V("center",					Vector_Void) },
	{ V("eyeposition",				Vector_Void) },
	{ V("earposition",				Vector_Void) },
	{ V("bodytarget",				Vector_pVector) },
	{ V("illumination",				Int_Void) },
	{ V("fvisible",					Int_Cbase) },
	{ V("fvecvisible",				Int_pVector) },
	
	/** Entity specific hooks **/

	/* CBasePlayer */
	{ V("player_jump",				Void_Void) },
	{ V("player_duck",				Void_Void) },
	{ V("player_prethink",			Void_Void) },
	{ V("player_postthink",			Void_Void) },
	{ V("player_getgunposition",	Vector_Void) },
	{ V("player_shouldfadeondeath",	Int_Void) },
	{ V("player_impulsecommands",	Void_Void) },
	{ V("player_updateclientdata",	Void_Void) },

	/* CBasePlayerItem */
	{ V("item_addtoplayer",			Int_Cbase) },
	{ V("item_addduplicate",		Int_Cbase) },
	{ V("item_candeploy",			Int_Void) },
	{ V("item_deploy",				Int_Void) },
	{ V("item_canholster",			Int_Void) },
	{ V("item_holster",				Void_Int) },
	{ V("item_updateiteminfo",		Void_Void) },
	{ V("item_preframe",			Void_Void) },
	{ V("item_postframe",			Void_Void) },
	{ V("item_drop",				Void_Void) },
	{ V("item_kill",				Void_Void) },
	{ V("item_attachtoplayer",		Void_Cbase) },
	{ V("item_primaryammoindex",	Int_Void) },
	{ V("item_secondaryammoindex",	Int_Void) },
	{ V("item_updateclientdata",	Int_Cbase) },
	{ V("item_getweaponptr",		Cbase_Void) },
	{ V("item_itemslot",			Int_Void) },
	
	/* CBasePlayerWeapon */
	{ V("weapon_extractammo",		Int_Cbase) },
	{ V("weapon_extractclipammo",	Int_Cbase) },
	{ V("weapon_addweapon",			Int_Void) },
	{ V("weapon_playemptysound",	Int_Void) },
	{ V("weapon_resetemptysound",	Void_Void) },
	{ V("weapon_sendweaponanim",	Void_Int_Int_Int) },
	{ V("weapon_isusable",			Int_Void) },
	{ V("weapon_primaryattack",		Void_Void) },
	{ V("weapon_secondaryattack",	Void_Void) },
	{ V("weapon_reload",			Void_Void) },
	{ V("weapon_weaponidle",		Void_Void) },
	{ V("weapon_retireweapon",		Void_Void) },
	{ V("weapon_shouldweaponidle",	Int_Void) },
	{ V("weapon_usedecrement",		Int_Void) },
	/** Mod specific hooks **/

	/* The Specialists */
	{ V("ts_breakablerespawn",		Int_Int) },
	{ V("ts_canusedthroughwalls",	Int_Void) },
	{ V("ts_respawnwait",			Deprecated) },

	/* Counter-Strike */
	{ V("cstrike_restart",			Void_Void) },
	{ V("cstrike_roundrespawn",		Void_Void) },
	{ V("cstrike_item_candrop",		Int_Void) },
	{ V("cstrike_item_getmaxspeed",	Float_Void) },

	/* Day of Defeat */
	{ V("dod_roundrespawn",			Void_Void) },
	{ V("dod_roundrespawnent",		Void_Void) },
	{ V("dod_roundstore",			Void_Void) },
	{ V("dod_areasetindex",			Void_Int) },
	{ V("dod_areasendstatus",		Void_Cbase) },
	{ V("dod_getstate",				Int_Void) },
	{ V("dod_getstateent",			Int_Cbase) },
	{ V("dod_item_candrop",			Int_Void) },

	
	/* Team Fortress Classic */
	{ V("tfc_engineeruse",			Int_Cbase) },
	{ V("tfc_finished",				Void_Void) },
	{ V("tfc_empexplode",			Void_Entvar_Float_Float) },
	{ V("tfc_calcempdmgrad",		Int_pFloat_pFloat) },
	{ V("tfc_takeempblast",			Void_Entvar) },
	{ V("tfc_empremove",			Void_Void) },
	{ V("tfc_takeconcussionblast",	Void_Entvar_Float) },
	{ V("tfc_concuss",				Void_Entvar) },

	/* Earth's Special Forces */
	{ V("esf_isenvmodel",			Int_Void) },
	{ V("esf_takedamage2",			Int_Entvar_Entvar_Float_Float_Int) },

	/* Natural-Selection */
	{ V("ns_getpointvalue",			Int_Void) },
	{ V("ns_awardkill",				Void_Entvar) },
	{ V("ns_resetentity",			Void_Void) },
	{ V("ns_updateonremove",		Void_Void) },

	{ V("ts_giveslowmul",			Void_Void) },
	{ V("ts_goslow",				Void_Float_Int) },
	{ V("ts_inslow",				Int_Void) },
	{ V("ts_isobjective",			Int_Void) },
	{ V("ts_enableobjective",		Void_Int) },
	{ V("ts_onfreeentprivatedata",	Void_Void) },
	{ V("ts_shouldcollide",			Int_Cbase) },

};


void FailPlugin(AMX *amx, int id, int err, const char *reason)
{
	int fwd=MF_RegisterSPForwardByName(amx, "__fatal_ham_error", FP_CELL, FP_CELL, FP_STRING, FP_DONE);

	MF_ExecuteForward(fwd, id, err, reason);

	MF_UnregisterSPForward(fwd);
}
static cell AMX_NATIVE_CALL RegisterHam(AMX *amx, cell *params)
{
	// Make sure the function we're requesting is within bounds
	int func=params[1];
	int post=params[4];

	CHECK_FUNCTION(func);

	char *function=MF_GetAmxString(amx, params[3], 0, NULL);
	char *classname=MF_GetAmxString(amx, params[2], 1, NULL);
	
	// Check the entity

	// create an entity, assign it the gamedll's class, hook it and destroy it
	edict_t *Entity=CREATE_ENTITY();

	CALL_GAME_ENTITY(PLID,classname,&Entity->v.classname); //->v

	if (Entity->v.pvPrivateData == NULL)
	{
		REMOVE_ENTITY(Entity);

		MF_LogError(amx, AMX_ERR_NATIVE,"Failed to retrieve classtype for \"%s\", hook for \"%s\" not active.",classname,function);

		return 0;
	}
	void **vtable=GetVTable(Entity->v.pvPrivateData, Offsets.GetBase());

	REMOVE_ENTITY(Entity);

	if (vtable == NULL)
	{
		MF_LogError(amx, AMX_ERR_NATIVE,"Failed to retrieve vtable for \"%s\", hook for \"%s\" not active.",classname,function);

		return 0;
	}

	// Verify that the function is valid
	// Don't fail the plugin if this fails, just emit a normal error
	int fwd=hooklist[func].makefunc(amx, function);

	if (fwd == -1)
	{
		MF_LogError(amx, AMX_ERR_NATIVE, "Function %s not found.", function);

		return 0;
	}

	// We've passed all tests...

	int **ivtable=(int **)vtable;

	void *vfunction=(void *)ivtable[hooklist[func].vtid];

	// Check the list of this function's hooks, see if the function we have is a hook

	CVector<Hook *>::iterator end=hooks[func].end();
	for (CVector<Hook *>::iterator i=hooks[func].begin();
		 i!=end;
		 ++i)
	{
		if ((*i)->tramp == vfunction)
		{
			// Yes, this function is hooked
			Forward *pfwd=new Forward(fwd);
			if (post)
			{
				(*i)->post.push_back(pfwd);
			}
			else
			{
				(*i)->pre.push_back(pfwd);
			}
			return reinterpret_cast<cell>(pfwd);
		}
	}

	// If we got here, the function is not hooked
	Hook *hook=new Hook(vtable, hooklist[func].vtid, hooklist[func].targetfunc, hooklist[func].isvoid, hooklist[func].paramcount, classname);
	hooks[func].push_back(hook);

	Forward *pfwd=new Forward(fwd);
	if (post)
	{
		hook->post.push_back(pfwd);
	}
	else
	{
		hook->pre.push_back(pfwd);
	}

	return reinterpret_cast<cell>(pfwd);
}
// RegisterHamFromEntity(Ham:function, EntityId, const Callback[], Post=0);
static cell AMX_NATIVE_CALL RegisterHamFromEntity(AMX *amx, cell *params)
{
	// Make sure the function we're requesting is within bounds
	int func=params[1];
	int post=params[4];

	CHECK_FUNCTION(func);

	char *function=MF_GetAmxString(amx, params[3], 0, NULL);
	int entid=params[2];
	char classname[64];
	
	// Check the entity

	edict_t *Entity=INDEXENT_NEW(entid);


	if (Entity->v.pvPrivateData == NULL)
	{

		MF_LogError(amx, AMX_ERR_NATIVE,"Failed to retrieve classtype for entity id \"%d\", hook for \"%s\" not active.",entid,function);

		return 0;
	}
	void **vtable=GetVTable(Entity->v.pvPrivateData, Offsets.GetBase());


	if (vtable == NULL)
	{
		MF_LogError(amx, AMX_ERR_NATIVE,"Failed to retrieve vtable for entity id \"%d\", hook for \"%s\" not active.",entid,function);

		return 0;
	}

	// Verify that the function is valid
	// Don't fail the plugin if this fails, just emit a normal error
	int fwd=hooklist[func].makefunc(amx, function);

	if (fwd == -1)
	{
		MF_LogError(amx, AMX_ERR_NATIVE, "Function %s not found.", function);

		return 0;
	}

	// We've passed all tests...

	int **ivtable=(int **)vtable;

	void *vfunction=(void *)ivtable[hooklist[func].vtid];

	// Check the list of this function's hooks, see if the function we have is a hook

	CVector<Hook *>::iterator end=hooks[func].end();
	for (CVector<Hook *>::iterator i=hooks[func].begin();
		 i!=end;
		 ++i)
	{
		if ((*i)->tramp == vfunction)
		{
			// Yes, this function is hooked
			Forward *pfwd=new Forward(fwd);
			if (post)
			{
				(*i)->post.push_back(pfwd);
			}
			else
			{
				(*i)->pre.push_back(pfwd);
			}
			return reinterpret_cast<cell>(pfwd);
		}
	}

	// Note down the classname for the given class
	// It may very well be wrong (such as lots of TS weapons have the same classname)
	// but it's the best we can do, and better than nothing.
	// (only used for display)
	snprintf(classname, sizeof(classname) - 1, "%s", STRING(Entity->v.classname));

	// If we got here, the function is not hooked
	Hook *hook=new Hook(vtable, hooklist[func].vtid, hooklist[func].targetfunc, hooklist[func].isvoid, hooklist[func].paramcount, classname);
	hooks[func].push_back(hook);

	Forward *pfwd=new Forward(fwd);
	if (post)
	{
		hook->post.push_back(pfwd);
	}
	else
	{
		hook->pre.push_back(pfwd);
	}

	return reinterpret_cast<cell>(pfwd);
}
static cell AMX_NATIVE_CALL ExecuteHam(AMX *amx, cell *params)
{
	int func=params[1];

	CHECK_FUNCTION(func);

	gDoForwards=false;
	return hooklist[func].call(amx, params);
}
static cell AMX_NATIVE_CALL ExecuteHamB(AMX *amx, cell *params)
{
	int func=params[1];

	CHECK_FUNCTION(func);

	gDoForwards=true;
	return hooklist[func].call(amx, params);
}


static cell AMX_NATIVE_CALL IsHamValid(AMX *amx, cell *params)
{
	int func=params[1];

	if (func >= 0 && 
		func < HAM_LAST_ENTRY_DONT_USE_ME_LOL &&
		hooklist[func].isset!=0)
	{
		return 1;
	}
	return 0;
}

static cell AMX_NATIVE_CALL DisableHamForward(AMX *amx, cell *params)
{
	Forward *fwd=reinterpret_cast<Forward *>(params[1]);

	if (fwd == 0)
	{
		MF_LogError(amx, AMX_ERR_NATIVE, "Invalid HamHook handle.");
		return -1;
	}

	fwd->state=FSTATE_STOP;
	return 0;
}
static cell AMX_NATIVE_CALL EnableHamForward(AMX *amx, cell *params)
{
	Forward *fwd=reinterpret_cast<Forward *>(params[1]);

	if (fwd == 0)
	{
		MF_LogError(amx, AMX_ERR_NATIVE, "Invalid HamHook handle.");
		return -1;
	}

	fwd->state=FSTATE_OK;
	return 0;
}
AMX_NATIVE_INFO RegisterNatives[] =
{
	{ "RegisterHam",			RegisterHam },
	{ "RegisterHamFromEntity",	RegisterHamFromEntity },
	{ "ExecuteHam",				ExecuteHam },
	{ "ExecuteHamB",			ExecuteHamB },
	{ "IsHamValid",				IsHamValid },
	{ "DisableHamForward",		DisableHamForward },
	{ "EnableHamForward",		EnableHamForward },

	{ NULL,						NULL }
};
