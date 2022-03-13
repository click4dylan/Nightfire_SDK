// vi: set ts=4 sw=4 :
// vim: set tw=75 :

// engine_api.cpp - implementation of Half-Life engine functions

/*
 * Copyright (c) 2001-2003 Will Day <willday@hpgx.net>
 *
 *    This file is part of Metamod.
 *
 *    Metamod is free software; you can redistribute it and/or modify it
 *    under the terms of the GNU General Public License as published by the
 *    Free Software Foundation; either version 2 of the License, or (at
 *    your option) any later version.
 *
 *    Metamod is distributed in the hope that it will be useful, but
 *    WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with Metamod; if not, write to the Free Software Foundation,
 *    Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *    In addition, as a special exception, the author gives permission to
 *    link the code of this program with the Half-Life Game Engine ("HL
 *    Engine") and Modified Game Libraries ("MODs") developed by Valve,
 *    L.L.C ("Valve").  You must obey the GNU General Public License in all
 *    respects for all of the code used other than the HL Engine and MODs
 *    from Valve.  If you modify this file, you may extend this exception
 *    to your version of the file, but you are not obligated to do so.  If
 *    you do not wish to do so, delete this exception statement from your
 *    version.
 *
 */

#include <extdll.h>

#include <meta_api.h>

#include "singlep.h"

enginefuncs_t meta_engfuncs = 
{
  NULL, //pfnCreateConsoleVariableGame()
  NULL, //pfnDestroyConsoleVariableGame()
  NULL, //pfnGetConsoleVariableGame()
  NULL, //pfnRegisterGame()
  NULL, //pfnUnregisterGame()
  NULL, //pfnPrecacheModel()
  NULL, //pfnPrecacheSound()
  NULL, //pfnPrecacheGeneric()
  NULL, //pfnSetModel()
  NULL, //pfnModelIndex()
  NULL, //pfnModelFrames()
  NULL, //pfnSetSize()
  NULL, //pfnChangeLevel()
  NULL, //pfnGetSpawnParms()
  NULL, //pfnSaveSpawnParms()
  NULL, //pfnVecToYaw()
  NULL, //pfnVecToAngles()
  NULL, //pfnMoveToOrigin()
  NULL, //pfnChangeYaw()
  NULL, //pfnChangePitch()
  NULL, //pfnFindEntityByString()
  NULL, //pfnGetEntityIllum()
  NULL, //pfnGetPointIllum()
  NULL, //pfnFindEntityInSphere()
  NULL, //pfnFindClientInPVS()
  NULL, //pfnEntitiesInPVS()
  NULL, //pfnMakeVectors()
  NULL, //pfnAngleVectors()
  NULL, //pfnCreateEntity()
  NULL, //pfnRemoveEntity()
  NULL, //pfnCreateNamedEntity()
  NULL, //pfnMakeStatic()
  NULL, //pfnCheckBottom()
  NULL, //pfnDropToFloor()
  NULL, //pfnWalkMove()
  NULL, //pfnSetOrigin()
  NULL, //pfnEmitSound()
  NULL, //pfnEmitAmbientSound()
  NULL, //pfnTraceLine()
  NULL, //pfnTraceToss()
  NULL, //pfnTraceCharacterHull()
  NULL, //pfnTraceHull()
  NULL, //pfnTraceModel()
  NULL, //pfnTraceTexture()
  NULL, //pfnGetAimVector()
  NULL, //pfnServerCommand()
  NULL, //pfnServerExecute()
  NULL, //pfnClientCommand()
  NULL, //pfnParticleEffect()
  NULL, //pfnLightStyle()
  NULL, //pfnAllocateDecal()
  NULL, //pfnDecalIndex()
  NULL, //pfnPointContents()
  NULL, //pfnWaterCheck()
  NULL, //pfnMessageBegin()
  NULL, //pfnMessageEnd()
  NULL, //pfnWriteByte()
  NULL, //pfnWriteChar()
  NULL, //pfnWriteShort()
  NULL, //pfnWriteLong()
  NULL, //pfnWriteAngle()
  NULL, //pfnWriteCoord()
  NULL, //pfnWriteString()
  NULL, //pfnWriteEntity()
  NULL, //pfnAlertMessage()
  NULL, //pfnEngineFprintf()
  NULL, //pfnPvAllocEntPrivateData()
  NULL, //pfnPvEntPrivateData()
  NULL, //pfnFreeEntPrivateData()
  NULL, //pfnSzFromIndex()
  NULL, //pfnAllocString()
  NULL, //pfnGetVarsOfEnt()
  NULL, //pfnPEntityOfEntOffset()
  NULL, //pfnEntOffsetOfPEntity()
  NULL, //pfnIndexOfEdict()
  NULL, //pfnPEntityOfEntIndex()
  NULL, //pfnFindEntityByVars()
  NULL, //pfnGetModelPtr()
  NULL, //pfnRegUserMsg()
  NULL, //pfnUnregUserMsgs()
  NULL, //pfnGetBonePosition()
  NULL, //pfnGetBoneCoord()
  sp_FunctionFromName, //pfnFunctionFromName()
  sp_NameForFunction, //pfnNameForFunction()
  NULL, //pfnClientPrintf()
  NULL, //pfnServerPrint()
  NULL, //pfnGetAttachment()
  NULL, //pfnCRC32_Init()
  NULL, //pfnCRC32_ProcessBuffer()
  NULL, //pfnCRC32_ProcessByte()
  NULL, //pfnCRC32_Final()
  NULL, //pfnRandomLong()
  NULL, //pfnRandomFloat()
  NULL, //pfnSetView()
  NULL, //pfnTime()
  NULL, //pfnCrosshairAngle()
  NULL, //pfnCOM_Log()
  NULL, //pfnCOM_FileBase()
  NULL, //pfnCOM_LoadHeapFile()
  NULL, //pfnCOM_ParseFile()
  NULL, //pfnCOM_Token()
  NULL, //pfnCOM_FreeFile()
  NULL, //pfnEndSection()
  NULL, //pfnCompareFileTime()
  NULL, //pfnGetGameDir()
  NULL, //pfnFadeClientVolume()
  NULL, //pfnSetClientMaxspeed()
  NULL, //pfnCreateFakeClient()
  NULL, //pfnRunPlayerMove()
  NULL, //pfnNumberOfEntities()
  NULL, //pfnGetInfoKeyBuffer()
  NULL, //pfnInfoKeyValue()
  NULL, //pfnSetKeyValue()
  NULL, //pfnSetClientKeyValue()
  NULL, //pfnIsMapValid()
  NULL, //pfnStaticDecal()
  NULL, //pfnGetPlayerUserId()
  NULL, //pfnBuildSoundMsg()
  NULL, //pfnIsDedicatedServer()
  NULL, //pfnGetPlayerWONId()
  NULL, //pfnInfo_RemoveKey()
  NULL, //pfnGetPhysicsKeyValue()
  NULL, //pfnSetPhysicsKeyValue()
  NULL, //pfnGetPhysicsInfoString()
  NULL, //pfnPrecacheEvent()
  NULL, //pfnPlaybackEvent()
  NULL, //pfnSetFatPVS()
  NULL, //pfnSetFatPAS()
  NULL, //pfnCheckVisibility()
  NULL, //pfnDeltaSetField()
  NULL, //pfnDeltaUnsetField()
  NULL, //pfnDeltaAddEncoder()
  NULL, //pfnGetCurrentPlayer()
  NULL, //pfnCanSkipPlayer()
  NULL, //pfnDeltaFindField()
  NULL, //pfnDeltaSetFieldByIndex()
  NULL, //pfnDeltaUnsetFieldByIndex()
  NULL, //pfnSetGroupMask()
  NULL, //pfnCreateInstancedBaseline()
  NULL, //pfnForceUnmodified()
  NULL, //pfnGetPlayerStats()
  NULL, //pfnPM_FindSafeSpot()
  NULL, //pfnIndexForSurface()
  NULL, //pfnSurfaceForIndex()
  NULL //pfnSV_ClosestSurfaceToPoint()
};

C_DLLEXPORT int GetEngineFunctions(enginefuncs_t *pengfuncsFromEngine, 
		int *interfaceVersion) 
{
	if(!pengfuncsFromEngine) {
		UTIL_LogPrintf("GetEngineFunctions called with null pengfuncsFromEngine");
		return(FALSE);
	}
	else if(*interfaceVersion != ENGINE_INTERFACE_VERSION) {
		UTIL_LogPrintf("GetEngineFunctions version mismatch; requested=%d ours=%d", *interfaceVersion, ENGINE_INTERFACE_VERSION);
		// Tell metamod what version we had, so it can figure out who is out of date.
		*interfaceVersion = ENGINE_INTERFACE_VERSION;
		return(FALSE);
	}
	memcpy(pengfuncsFromEngine, &meta_engfuncs, sizeof(enginefuncs_t));
	return(TRUE);
}
