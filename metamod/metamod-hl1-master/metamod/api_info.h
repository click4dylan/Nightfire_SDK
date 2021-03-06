// vi: set ts=4 sw=4 :
// vim: set tw=75 :

// api_info.h - structures to store info about api routines

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

#ifndef API_INFO_H
#define API_INFO_H

#include "types_meta.h"			// mBOOL


#define P_PRE 0		// plugin function called before gamedll
#define P_POST 1	// plugin function called after gamedll


typedef struct api_info_s {
	mBOOL trace;		// if true, log info about this function
	int loglevel;		// level at which to log info about this function
	const char *name;	// string representation of function name
} api_info_t;


// DLL api functions
typedef struct dllapi_info_s {
	api_info_t pfnGameInit;
	api_info_t pfnSpawn;
	api_info_t pfnThink;
	api_info_t pfnUse;
	api_info_t pfnTouch;
	api_info_t pfnBlocked;
	api_info_t pfnKeyValue;
	api_info_t pfnSave;
	api_info_t pfnRestore;
	api_info_t pfnSetAbsBox;
	api_info_t pfnBlockedTest;
	api_info_t pfnSaveWriteFields;
	api_info_t pfnSaveReadFields;
	api_info_t pfnSaveGlobalState;
	api_info_t pfnRestoreGlobalState;
	api_info_t pfnResetGlobalState;
	api_info_t pfnClientConnect;
	api_info_t pfnClientDisconnect;
	api_info_t pfnClientKill;
	api_info_t pfnClientPutInServer;
	api_info_t pfnClientCommand;
	api_info_t pfnClientUserInfoChanged;
	api_info_t pfnServerActivate;
	api_info_t pfnServerDeactivate;
	api_info_t pfnPlayerPreThink;
	api_info_t pfnPlayerPostThink;
	api_info_t pfnStartFrame;
	api_info_t pfnParmsNewLevel;
	api_info_t pfnParmsChangeLevel;
	api_info_t pfnGetGameDescription;
	api_info_t pfnGetPlayerSpawnSpot;
	//api_info_t pfnPlayerCustomization;
	api_info_t pfnSpectatorConnect;
	api_info_t pfnSpectatorDisconnect;
	api_info_t pfnSpectatorThink;
	api_info_t pfnSys_Error;
	api_info_t pfnPM_Move;
	api_info_t pfnPM_Init;
	api_info_t pfnSetupVisibility;
	api_info_t pfnAddToFullPack;
	api_info_t pfnUpdateClientData;
	
	api_info_t pfnPM_FindTextureType;
	api_info_t pfnCreateBaseline;
	api_info_t pfnRegisterEncoders;
	api_info_t pfnGetWeaponData;
	api_info_t pfnCmdStart;
	api_info_t pfnCmdEnd;
	api_info_t pfnConnectionlessPacket;
	api_info_t pfnGetHullBounds;
	api_info_t pfnCreateInstancedBaselines;
	api_info_t pfnInconsistentFile;
	api_info_t pfnAllowLagCompensation;
	api_info_t END;
} dllapi_info_t;


// "New" api functions
typedef struct newapi_info_s {
	api_info_t pfnOnFreeEntPrivateData;
	api_info_t pfnGameShutdown;
	api_info_t pfnShouldCollide;
	api_info_t pfnOnSaveGame;
	api_info_t pfnOnLoadGame;
	api_info_t pfnBot_GetParams;
	api_info_t pfnAddbot;
	/*
	api_info_t pfnShouldCollide;
	// Added 2005/08/11 (no SDK update)
	api_info_t pfnCvarValue;
	// Added 2005/11/22 (no SDK update)
	api_info_t pfnCvarValue2;
	*/
	api_info_t END;
} newapi_info_t;


// Engine functions
typedef struct engine_info_s {
	api_info_t pfnCreateConsoleVariableGame;
	api_info_t pfnDestroyConsoleVariableGame;
	api_info_t pfnGetConsoleVariableGame;
	api_info_t pfnRegisterGame;
	api_info_t pfnUnregisterGame;
	api_info_t pfnPrecacheModel;
	api_info_t pfnPrecacheSound;
	api_info_t pfnPrecacheGeneric;
	api_info_t pfnSetModel;
	api_info_t pfnModelIndex;
	api_info_t pfnModelFrames;
	api_info_t pfnSetSize;
	api_info_t pfnChangeLevel;
	api_info_t pfnGetSpawnParms;
	api_info_t pfnSaveSpawnParms;
	api_info_t pfnVecToYaw;
	api_info_t pfnVecToAngles;
	api_info_t pfnMoveToOrigin;
	api_info_t pfnChangeYaw;
	api_info_t pfnChangePitch;
	api_info_t pfnFindEntityByString;
	api_info_t pfnGetEntityIllum;
	api_info_t pfnGetPointIllum;
	api_info_t pfnFindEntityInSphere;
	api_info_t pfnFindClientInPVS;
	api_info_t pfnEntitiesInPVS;
	api_info_t pfnMakeVectors;
	api_info_t pfnAngleVectors;
	api_info_t pfnCreateEntity;
	api_info_t pfnRemoveEntity;
	api_info_t pfnCreateNamedEntity;
	api_info_t pfnMakeStatic;
	api_info_t pfnCheckBottom;
	api_info_t pfnDropToFloor;
	api_info_t pfnWalkMove;
	api_info_t pfnSetOrigin;
	api_info_t pfnEmitSound;
	api_info_t pfnEmitAmbientSound;
	api_info_t pfnTraceLine;
	api_info_t pfnTraceToss;
	api_info_t pfnTraceCharacterHull;
	api_info_t pfnTraceHull;
	api_info_t pfnTraceModel;
	api_info_t pfnTraceTexture;
	api_info_t pfnGetAimVector;
	api_info_t pfnServerCommand;
	api_info_t pfnServerExecute;
	api_info_t pfnClientCommand;
	api_info_t pfnParticleEffect;
	api_info_t pfnLightStyle;
	api_info_t pfnAllocateDecal;
	api_info_t pfnDecalIndex;
	api_info_t pfnPointContents;
	api_info_t pfnWaterCheck;
	api_info_t pfnMessageBegin;
	api_info_t pfnMessageEnd;
	api_info_t pfnWriteByte;
	api_info_t pfnWriteChar;
	api_info_t pfnWriteShort;
	api_info_t pfnWriteLong;
	api_info_t pfnWriteAngle;
	api_info_t pfnWriteCoord;
	api_info_t pfnWriteString;
	api_info_t pfnWriteEntity;
	api_info_t pfnAlertMessage;
	api_info_t pfnEngineFprintf;
	api_info_t pfnPvAllocEntPrivateData;
	api_info_t pfnPvEntPrivateData;
	api_info_t pfnFreeEntPrivateData;
	api_info_t pfnSzFromIndex;
	api_info_t pfnAllocString;
	api_info_t pfnGetVarsOfEnt;
	api_info_t pfnPEntityOfEntOffset;
	api_info_t pfnEntOffsetOfPEntity;
	api_info_t pfnIndexOfEdict;
	api_info_t pfnPEntityOfEntIndex;
	api_info_t pfnFindEntityByVars;
	api_info_t pfnGetModelPtr;
	api_info_t pfnRegUserMsg;
	api_info_t pfnUnregUserMsgs;
	api_info_t pfnGetBonePosition;
	api_info_t pfnGetBoneCoord;
	api_info_t pfnFunctionFromName;
	api_info_t pfnNameForFunction;
	api_info_t pfnClientPrintf;
	api_info_t pfnServerPrint;
	api_info_t pfnGetAttachment;
	api_info_t pfnCRC32_Init;
	api_info_t pfnCRC32_ProcessBuffer;
	api_info_t pfnCRC32_ProcessByte;
	api_info_t pfnCRC32_Final;
	api_info_t pfnRandomLong;
	api_info_t pfnRandomFloat;
	api_info_t pfnSetView;
	api_info_t pfnTime;
	api_info_t pfnCrosshairAngle;
	api_info_t pfnCOM_Log;
	api_info_t pfnCOM_FileBase;
	api_info_t pfnCOM_LoadHeapFile;
	api_info_t pfnCOM_ParseFile;
	api_info_t pfnCOM_Token;
	api_info_t pfnCOM_FreeFile;
	api_info_t pfnEndSection;
	api_info_t pfnCompareFileTime;
	api_info_t pfnGetGameDir;
	api_info_t pfnFadeClientVolume;
	api_info_t pfnSetClientMaxspeed;
	api_info_t pfnCreateFakeClient;
	api_info_t pfnRunPlayerMove;
	api_info_t pfnNumberOfEntities;
	api_info_t pfnGetInfoKeyBuffer;
	api_info_t pfnInfoKeyValue;
	api_info_t pfnSetKeyValue;
	api_info_t pfnSetClientKeyValue;
	api_info_t pfnIsMapValid;
	api_info_t pfnStaticDecal;
	api_info_t pfnGetPlayerUserId;
	api_info_t pfnBuildSoundMsg;
	api_info_t pfnIsDedicatedServer;
	api_info_t pfnGetPlayerWONId;
	api_info_t pfnInfo_RemoveKey;
	api_info_t pfnGetPhysicsKeyValue;
	api_info_t pfnSetPhysicsKeyValue;
	api_info_t pfnGetPhysicsInfoString;
	api_info_t pfnPrecacheEvent;
	api_info_t pfnPlaybackEvent;
	api_info_t pfnSetFatPVS;
	api_info_t pfnSetFatPAS;
	api_info_t pfnCheckVisibility;
	api_info_t pfnDeltaSetField;
	api_info_t pfnDeltaUnsetField;
	api_info_t pfnDeltaAddEncoder;
	api_info_t pfnGetCurrentPlayer;
	api_info_t pfnCanSkipPlayer;
	api_info_t pfnDeltaFindField;
	api_info_t pfnDeltaSetFieldByIndex;
	api_info_t pfnDeltaUnsetFieldByIndex;
	api_info_t pfnSetGroupMask;
	api_info_t pfnCreateInstancedBaseline;
	api_info_t pfnForceUnmodified;
	api_info_t pfnGetPlayerStats;
	api_info_t pfnPM_FindSafeSpot;
	api_info_t pfnIndexForSurface;
	api_info_t pfnSurfaceForIndex;
	api_info_t pfnSV_ClosestSurfaceToPoint;
	api_info_t END;
} engine_info_t;


extern dllapi_info_t dllapi_info;
extern newapi_info_t newapi_info;
extern engine_info_t engine_info;

#endif /* API_INFO_H */
