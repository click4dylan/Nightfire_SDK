/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
#ifndef ENGINECALLBACK_H
#define ENGINECALLBACK_H
#ifdef _WIN32
#ifndef __MINGW32__
#pragma once
#endif /* not __MINGW32__ */
#endif

#include "event_flags.h"

// Fix warning in MSVC8
#undef SERVER_EXECUTE

// Must be provided by user of this code
extern enginefuncs_t g_engfuncs;

// The actual engine callbacks
#define NUM_FOR_EDICT (*g_engfuncs.pfnNUM_FOR_EDICT)
#define UNKNOWN1_FIXEDLIGHT (*g_engfuncs.pfnUnknown1_FixedLight)
#define CHECKBOTTOM	(*g_engfuncs.pfnCheckBottom)
#define ALLOCATEDECAL (*g_engfuncs.pfnAllocateDecal)
#define GETBONETRANSFORM1 (*g_engfuncs.pfnGetBoneTransform1)
#define LOADFILEFORME2 (*g_engfuncs.pfnLoadFileForMe2)
#define FS_FINDFIRST (*g_engfuncs.pfnFS_FindFirst)
#define EXPLODESTRING (*g_engfuncs.pfnExplodeString)
#define GETGAMEDLL (*g_engfuncs.pfnGetGameDLL)
#define UNKNOWN2 (*g_engfuncs.pfnUnknown2)
#define UNKNOWN3 (*g_engfuncs.pfnUnknown3)
#define UNKNOWN4 (*g_engfuncs.pfnUnknown4)
#define UNKNOWN5 (*g_engfuncs.pfnUnknown5)
#define UNKNOWN6 (*g_engfuncs.pfnUnknown6)

#define GETPLAYERUSERID (*g_engfuncs.pfnGetPlayerUserId)
#define PRECACHE_MODEL	(*g_engfuncs.pfnPrecacheModel)
#define PRECACHE_SOUND	(*g_engfuncs.pfnPrecacheSound)
#define PRECACHE_GENERIC	(*g_engfuncs.pfnPrecacheGeneric)
#define SET_MODEL		(*g_engfuncs.pfnSetModel)
#define MODEL_INDEX		(*g_engfuncs.pfnModelIndex)
#define MODEL_FRAMES	(*g_engfuncs.pfnModelFrames)
#define SET_SIZE		(*g_engfuncs.pfnSetSize)
#define CHANGE_LEVEL	(*g_engfuncs.pfnChangeLevel)
#define GET_SPAWN_PARMS	(*g_engfuncs.pfnGetSpawnParms)
#define SAVE_SPAWN_PARMS (*g_engfuncs.pfnSaveSpawnParms)
#define VEC_TO_YAW		(*g_engfuncs.pfnVecToYaw)
#define VEC_TO_ANGLES	(*g_engfuncs.pfnVecToAngles)
#define MOVE_TO_ORIGIN  (*g_engfuncs.pfnMoveToOrigin)
#define CHANGE_YAW		(*g_engfuncs.pfnChangeYaw) //old?
#define CHANGE_PITCH	(*g_engfuncs.pfnChangePitch)
#define MAKE_VECTORS	(*g_engfuncs.pfnMakeVectors)
//Dylan added MAKE_AIM_VECTORS from util.cpp
inline void MAKE_AIM_VECTORS(const Vector &vecAngles) {
	float rgflVec[3];
	vecAngles.CopyToArray(rgflVec);
	rgflVec[0] = -rgflVec[0];
	MAKE_VECTORS(rgflVec);
}
#define CREATE_ENTITY	(*g_engfuncs.pfnCreateEntity)
#define REMOVE_ENTITY	(*g_engfuncs.pfnRemoveEntity)
#define CREATE_NAMED_ENTITY		(*g_engfuncs.pfnCreateNamedEntity)
#define MAKE_STATIC		(*g_engfuncs.pfnMakeStatic)
#define ENT_IS_ON_FLOOR	(*g_engfuncs.pfnEntIsOnFloor)
#define DROP_TO_FLOOR	(*g_engfuncs.pfnDropToFloor)
#define WALK_MOVE		(*g_engfuncs.pfnWalkMove)
#define SET_ORIGIN		(*g_engfuncs.pfnSetOrigin)
#define EMIT_SOUND_DYN2 (*g_engfuncs.pfnEmitSound)
#define BUILD_SOUND_MSG (*g_engfuncs.pfnBuildSoundMsg)
#define TRACE_LINE		(*g_engfuncs.pfnTraceLine)
#define TRACE_TOSS		(*g_engfuncs.pfnTraceToss)
#define TRACE_MONSTER_HULL		(*g_engfuncs.pfnTraceMonsterHull)
#define TRACE_HULL		(*g_engfuncs.pfnTraceHull)
#define GET_AIM_VECTOR	(*g_engfuncs.pfnGetAimVector)
#define SERVER_COMMAND	(*g_engfuncs.pfnServerCommand)
#define SERVER_EXECUTE	(*g_engfuncs.pfnServerExecute)
#define CLIENT_COMMAND	(*g_engfuncs.pfnClientCommand)
#define PARTICLE_EFFECT	(*g_engfuncs.pfnParticleEffect)
#define LIGHT_STYLE		(*g_engfuncs.pfnLightStyle)
#define DECAL_INDEX		(*g_engfuncs.pfnDecalIndex)
#define POINT_CONTENTS	(*g_engfuncs.pfnPointContents)
#define CRC32_INIT           (*g_engfuncs.pfnCRC32_Init)
#define CRC32_PROCESS_BUFFER (*g_engfuncs.pfnCRC32_ProcessBuffer)
#define CRC32_PROCESS_BYTE   (*g_engfuncs.pfnCRC32_ProcessByte)
#define CRC32_FINAL          (*g_engfuncs.pfnCRC32_Final)
#define RANDOM_LONG		(*g_engfuncs.pfnRandomLong)
#define RANDOM_FLOAT	(*g_engfuncs.pfnRandomFloat)
#define GETPLAYERAUTHID	(*g_engfuncs.pfnGetPlayerWONId) //pfnGetPlayerAuthID is NULL

inline void MESSAGE_BEGIN( int msg_dest, int msg_type, const float *pOrigin = NULL, edict_t *ed = NULL ) {
	(*g_engfuncs.pfnMessageBegin)(msg_dest, msg_type, pOrigin, ed);
}
#define MESSAGE_END		(*g_engfuncs.pfnMessageEnd)
#define WRITE_BYTE		(*g_engfuncs.pfnWriteByte)
#define WRITE_CHAR		(*g_engfuncs.pfnWriteChar)
#define WRITE_SHORT		(*g_engfuncs.pfnWriteShort)
#define WRITE_LONG		(*g_engfuncs.pfnWriteLong)
#define WRITE_ANGLE		(*g_engfuncs.pfnWriteAngle)
#define WRITE_COORD		(*g_engfuncs.pfnWriteCoord)
#define WRITE_STRING	(*g_engfuncs.pfnWriteString)
#define WRITE_ENTITY	(*g_engfuncs.pfnWriteEntity)
#define CVAR_REGISTER	(*g_engfuncs.pfnCreateConsoleVariableGame)
#define CVAR_GET_POINTER (*g_engfuncs.pfnGetConsoleVariableGame)
#define ALERT			(*g_engfuncs.pfnAlertMessage)
#define ENGINE_FPRINTF	(*g_engfuncs.pfnEngineFprintf)
#define ALLOC_PRIVATE	(*g_engfuncs.pfnPvAllocEntPrivateData)

inline float CVAR_GET_FLOAT(const char* name)
{
	ConsoleVariable* pVar = CVAR_GET_POINTER(name);
	if (pVar)
		return pVar->getValue();
	return 0;
}
inline int CVAR_GET_INT(const char* name)
{
	ConsoleVariable* pVar = CVAR_GET_POINTER(name);
	if (pVar)
		return (int)pVar->getValue();
	return 0;
}
inline bool CVAR_GET_BOOL(const char* name)
{
	ConsoleVariable* pVar = CVAR_GET_POINTER(name);
	if (pVar)
		return pVar->getValue() != 0.0f;
	return 0;
}
inline const char* CVAR_GET_STRING(const char* name)
{
	ConsoleVariable* pVar = CVAR_GET_POINTER(name);
	if (pVar)
		return pVar->getValueString();
	return "";
}
inline void *GET_PRIVATE( edict_t *pent )
{
	if ( pent )
		return pent->pvPrivateData; //dylan changed from pent->pvPrivateData
	return NULL;
}

#define FREE_PRIVATE	(*g_engfuncs.pfnFreeEntPrivateData)
//#define STRING			(*g_engfuncs.pfnSzFromIndex)
#define ALLOC_STRING	(*g_engfuncs.pfnAllocString)
#define FIND_ENTITY_BY_STRING	(*g_engfuncs.pfnFindEntityByString)
#define GETENTITYILLUM	(*g_engfuncs.pfnGetEntityIllum)
#define FIND_ENTITY_IN_SPHERE		(*g_engfuncs.pfnFindEntityInSphere)
#define FIND_CLIENT_IN_PVS			(*g_engfuncs.pfnFindClientInPVS)
#define EMIT_AMBIENT_SOUND			(*g_engfuncs.pfnEmitAmbientSound)
#define GET_MODEL_PTR				(*g_engfuncs.pfnGetModelPtr)
#define REG_USER_MSG				(*g_engfuncs.pfnRegUserMsg)
#define GET_BONE_POSITION			(*g_engfuncs.pfnGetBonePosition)
#define FUNCTION_FROM_NAME			(*g_engfuncs.pfnFunctionFromName)
#define NAME_FOR_FUNCTION			(*g_engfuncs.pfnNameForFunction)
#define TRACE_TEXTURE				(*g_engfuncs.pfnTraceTexture)
#define CLIENT_PRINTF				(*g_engfuncs.pfnClientPrintf)
//#define CMD_ARGS					(*g_engfuncs.pfnCmd_Args)
//#define CMD_ARGC					(*g_engfuncs.pfnCmd_Argc)
//#define CMD_ARGV					(*g_engfuncs.pfnCmd_Argv)
#define GET_ATTACHMENT			(*g_engfuncs.pfnGetAttachment)
#define SET_VIEW				(*g_engfuncs.pfnSetView)
#define SET_CROSSHAIRANGLE		(*g_engfuncs.pfnCrosshairAngle)
#define LOAD_FILE_FOR_ME		(*g_engfuncs.pfnCOM_LoadHeapFile)
#define FREE_FILE				(*g_engfuncs.pfnCOM_FreeFile)
#define COMPARE_FILE_TIME		(*g_engfuncs.pfnCompareFileTime)
#define GET_GAME_DIR			(*g_engfuncs.pfnGetGameDir)
#define IS_MAP_VALID			(*g_engfuncs.pfnIsMapValid)
#define NUMBER_OF_ENTITIES		(*g_engfuncs.pfnNumberOfEntities)
#define IS_DEDICATED_SERVER		(*g_engfuncs.pfnIsDedicatedServer)

#define PRECACHE_EVENT			(*g_engfuncs.pfnPrecacheEvent)
#define PLAYBACK_EVENT_FULL		(*g_engfuncs.pfnPlaybackEvent)

#define ENGINE_SET_PVS			(*g_engfuncs.pfnSetFatPVS)
#define ENGINE_SET_PAS			(*g_engfuncs.pfnSetFatPAS)

#define ENGINE_CHECK_VISIBILITY (*g_engfuncs.pfnCheckVisibility)

#define DELTA_SET				( *g_engfuncs.pfnDeltaSetField )
#define DELTA_UNSET				( *g_engfuncs.pfnDeltaUnsetField )
#define DELTA_ADDENCODER		( *g_engfuncs.pfnDeltaAddEncoder )
#define ENGINE_CURRENT_PLAYER   ( *g_engfuncs.pfnGetCurrentPlayer )

#define	ENGINE_CANSKIP			( *g_engfuncs.pfnCanSkipPlayer )

#define DELTA_FINDFIELD			( *g_engfuncs.pfnDeltaFindField )
#define DELTA_SETBYINDEX		( *g_engfuncs.pfnDeltaSetFieldByIndex )
#define DELTA_UNSETBYINDEX		( *g_engfuncs.pfnDeltaUnsetFieldByIndex )

#define ENGINE_GETPHYSINFO		( *g_engfuncs.pfnGetPhysicsInfoString )

#define ENGINE_SETGROUPMASK		( *g_engfuncs.pfnSetGroupMask )

#define ENGINE_INSTANCE_BASELINE ( *g_engfuncs.pfnCreateInstancedBaseline )

#define ENGINE_FORCE_UNMODIFIED	( *g_engfuncs.pfnForceUnmodified )

#define PLAYER_CNX_STATS		( *g_engfuncs.pfnGetPlayerStats )

#endif		//ENGINECALLBACK_H
