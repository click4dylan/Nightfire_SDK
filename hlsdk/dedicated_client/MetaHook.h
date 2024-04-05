#pragma once

extern void RunMetaHook();
extern void ShutdownMetaHook();

#include <winsani_in.h>
#include <windows.h>
#include <winsani_out.h>
#include <stdio.h>

#include <extdll.h>
//#include <vector_types.h>

#include <wrect.h>
#include <interface.h>

typedef int(*pfnUserMsgHook)(const char* pszName, int iSize, void* pbuf);

//#include <cdll_export.h>
#include <cdll_int.h>

typedef struct cl_exportfuncs_s
{
	int version; //5
	int size; //200 bytes
	INITIALIZE_FUNC CLIENT_Initialize;
	SHUTDOWN_FUNC CLIENT_Shutdown;
	HUD_INIT_FUNC HUD_Init;
	HUD_VIDINIT_FUNC HUD_VidInit;
	HUD_REDRAW_FUNC HUD_Redraw;
	HUD_UPDATECLIENTDATA_FUNC HUD_UpdateClientData;
	HUD_RESET_FUNC HUD_Reset;
	CL_DRAWSKY_FUNC CL_DrawSky;
	CL_DRAWTERRAIN_FUNC CL_DrawTerrain;
	CL_DRAWOCEAN_FUNC CL_DrawOcean;
	HUD_CLIENTMOVE_FUNC HUD_ClientMove;
	HUD_CLIENTMOVEINIT_FUNC HUD_ClientMoveInit;
	HUD_TEXTURETYPE_FUNC HUD_ClientMoveTexture;
	HUD_IN_ACTIVATEMOUSE_FUNC IN_ActivateMouse;
	HUD_IN_DEACTIVATEMOUSE_FUNC IN_DeactivateMouse;
	HUD_IN_MOUSEEVENT_FUNC IN_MouseEvent;
	HUD_IN_CLEARSTATES_FUNC IN_ClearStates;
	HUD_CL_CREATEMOVE_FUNC CL_CreateMove;
	HUD_CL_ISTHIRDPERSON_FUNC CL_IsThirdPerson;
	HUD_CL_GETCAMERAOFFSETS_FUNC CL_CameraOffset;
	HUD_KB_FIND_FUNC KB_Find;
	HUD_CAMTHINK_FUNC CAM_Think;
	HUD_CALCREF_FUNC V_CalcRefdef;
	HUD_ADDENTITY_FUNC HUD_AddEntity;
	HUD_CREATEENTITIES_FUNC HUD_CreateEntities;
	HUD_CLIENTSIDEENTITIESDRAW_FUNC HUD_ClientSideEntitiesDraw;
	HUD_DRAWNORMALTRIS_FUNC HUD_DrawNormalTriangles;
	HUD_DRAWTRANSTRIS_FUNC HUD_DrawTransparentTriangles;
	HUD_STUDIOEVENT_FUNC HUD_StudioEvent;
	HUD_POSTRUNCMD_FUNC HUD_PostRunCmd;
	HUD_TXFERLOCALOVERRIDES_FUNC HUD_TxferLocalOverrides;
	HUD_PROCESSPLAYERSTATE_FUNC HUD_ProcessPlayerState;
	HUD_TXFERPREDICTIONDATA_FUNC HUD_TxferPredictionData;
	HUD_CONNECTIONLESS_FUNC HUD_ConnectionlessPacket;
	HUD_GETHULLBOUNDS_FUNC HUD_GetHullBounds;
	HUD_PREFRAME_FUNC HUD_PreFrame;
	HUD_FRAME_FUNC HUD_Frame;
	HUD_POSTFRAME_FUNC HUD_PostFrame;
	HUD_KEY_EVENT_FUNC HUD_Key_Event;
	HUD_TEMPENTUPDATE_FUNC HUD_TempEntUpdate;
	HUD_GETUSERENTITY_FUNC HUD_GetUserEntity;
	CLIENT_LevelStart_FUNC CLIENT_LevelStart;
	CLIENT_LevelEnd_FUNC CLIENT_LevelEnd;
	HUD_IsTeamPlay_FUNC HUD_IsTeamPlay;
	HUD_ShowLoading_FUNC HUD_ShowLoading;
	HUD_ShowSaving_FUNC HUD_ShowSaving;
	CLIENT_SaveState_FUNC CLIENT_SaveState;
	CLIENT_LoadState_FUNC CLIENT_LoadState;
}
cl_exportfuncs_t;

extern cl_exportfuncs_s g_oExportFuncs;
extern struct enginefuncs_s* g_pEngineFuncs;
extern struct engine_studio_api_s* g_pStudioModelAPI;
extern class CStudioModelRenderer* g_pStudioAPI;
extern void* g_pGlobalVariables;

typedef void(*PauseMetaAudioPlaybackFn)();
typedef void(*ResumeMetaAudioPlaybackFn)();
extern PauseMetaAudioPlaybackFn g_oPauseMetaAudioPlayback;
extern ResumeMetaAudioPlaybackFn g_oResumeMetaAudioPlayback;
extern void PostFrame();