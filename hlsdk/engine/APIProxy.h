#ifndef __APIPROXY__
#define __APIPROXY__

#include "archtypes.h"     // DAL
#include "netadr.h"
#include "Sequence.h"
#include "cvardef.h"

#ifndef _WIN32
#include "enums.h"
#endif

#define	MAX_ALIAS_NAME	32

typedef struct cmdalias_s
{
	struct cmdalias_s	*next;
	char	name[MAX_ALIAS_NAME];
	char	*value;
} cmdalias_t;


// ********************************************************
// Functions exported by the client .dll
// ********************************************************

// Function type declarations for client exports
typedef int (*INITIALIZE_FUNC)	( struct cl_enginefuncs_s* );
typedef int (*SHUTDOWN_FUNC)	( void);
typedef void (*HUD_INIT_FUNC)	( void );
typedef int (*HUD_VIDINIT_FUNC)	( void );
typedef int (*HUD_REDRAW_FUNC)	( float, int );
typedef int (*HUD_UPDATECLIENTDATA_FUNC) ( struct client_data_s*, float );
typedef void (*HUD_RESET_FUNC)    ( void );
typedef void (*CL_DRAWSKY_FUNC)    (const float*);
typedef void (*CL_DRAWTERRAIN_FUNC)    (void);
typedef void (*CL_DRAWOCEAN_FUNC)    (void);
typedef void (*HUD_CLIENTMOVE_FUNC)( struct playermove_s *ppmove, qboolean server );
typedef void (*HUD_CLIENTMOVEINIT_FUNC)( struct playermove_s *ppmove );
typedef char (*HUD_TEXTURETYPE_FUNC)( char *name );
typedef void (*HUD_IN_ACTIVATEMOUSE_FUNC) ( void );
typedef void (*HUD_IN_DEACTIVATEMOUSE_FUNC)		( void );
typedef void (*HUD_IN_MOUSEEVENT_FUNC)		( int mstate );
typedef void (*HUD_IN_CLEARSTATES_FUNC)		( void );
typedef void (*HUD_CL_CREATEMOVE_FUNC)		( float frametime, struct usercmd_s *cmd, int active );
typedef int (*HUD_CL_ISTHIRDPERSON_FUNC) ( void );
typedef void (*HUD_CL_GETCAMERAOFFSETS_FUNC )( float *ofs );
typedef struct kbutton_s * (*HUD_KB_FIND_FUNC) ( const char *name );
typedef void ( *HUD_CAMTHINK_FUNC )( void );
typedef void ( *HUD_CALCREF_FUNC ) ( struct ref_params_s *pparams );
typedef int	 ( *HUD_ADDENTITY_FUNC ) ( int type, struct cl_entity_s *ent, const char *modelname );
typedef void ( *HUD_CREATEENTITIES_FUNC ) ( void );
typedef void (*HUD_CLIENTSIDEENTITIESDRAW_FUNC) (void);
typedef void ( *HUD_DRAWNORMALTRIS_FUNC ) ( void );
typedef void ( *HUD_DRAWTRANSTRIS_FUNC ) ( void );
typedef void ( *HUD_STUDIOEVENT_FUNC ) ( const struct mstudioevent_s *event, const struct cl_entity_s *entity );
typedef void ( *HUD_POSTRUNCMD_FUNC ) ( struct local_state_s *from, struct local_state_s *to, struct usercmd_s *cmd, int runfuncs, double time, unsigned int random_seed );
typedef void ( *HUD_TXFERLOCALOVERRIDES_FUNC )( struct entity_state_s *state, const struct clientdata_s *client );
typedef void ( *HUD_PROCESSPLAYERSTATE_FUNC )( struct entity_state_s *dst, const struct entity_state_s *src );
typedef void ( *HUD_TXFERPREDICTIONDATA_FUNC ) ( struct entity_state_s *ps, const struct entity_state_s *pps, struct clientdata_s *pcd, const struct clientdata_s *ppcd, struct weapon_data_s *wd, const struct weapon_data_s *pwd );
typedef int ( *HUD_CONNECTIONLESS_FUNC )( const struct netadr_s *net_from, const char *args, char *response_buffer, int *response_buffer_size );
typedef	int	( *HUD_GETHULLBOUNDS_FUNC ) ( int hullnumber, float *mins, float *maxs );
typedef void (*HUD_PREFRAME_FUNC)		(void);
typedef void (*HUD_FRAME_FUNC)		( double );
typedef void (*HUD_POSTFRAME_FUNC)		(void);
typedef int (*HUD_KEY_EVENT_FUNC ) ( int eventcode, int keynum, const char *pszCurrentBinding );
typedef void (*HUD_TEMPENTUPDATE_FUNC) ( double frametime, double client_time, double cl_gravity, struct tempent_s **ppTempEntFree, struct tempent_s **ppTempEntActive, 	int ( *Callback_AddVisibleEntity )( struct cl_entity_s *pEntity ),	void ( *Callback_TempEntPlaySound )( struct tempent_s *pTemp, float damp ) );
typedef struct cl_entity_s *(*HUD_GETUSERENTITY_FUNC ) ( int index, int unused );
typedef void (*CLIENT_LevelStart_FUNC) ( void );
typedef void (*CLIENT_LevelEnd_FUNC) (void);
typedef BOOL (*HUD_IsTeamPlay_FUNC) (void);
typedef void(*HUD_ShowLoading_FUNC) (bool unknown, bool unknown2);
typedef void(*HUD_ShowSaving_FUNC) ();
typedef int(*CLIENT_SaveState_FUNC) ();
typedef int(*CLIENT_LoadState_FUNC) ();

// Pointers to the exported client functions themselves
typedef struct
{
	INITIALIZE_FUNC pCLIENT_Initialize;
	SHUTDOWN_FUNC pCLIENT_Shutdown;
	HUD_INIT_FUNC pHUD_Init;
	HUD_VIDINIT_FUNC pHUD_VidInit;
	HUD_REDRAW_FUNC pHUD_Redraw;
	HUD_UPDATECLIENTDATA_FUNC pHUD_UpdateClientData;
	HUD_RESET_FUNC pHUD_Reset;
	CL_DRAWSKY_FUNC pCL_DrawSky;
	CL_DRAWTERRAIN_FUNC pCL_DrawTerrain;
	CL_DRAWOCEAN_FUNC pCL_DrawOcean;
	HUD_CLIENTMOVE_FUNC pHUD_ClientMove;
	HUD_CLIENTMOVEINIT_FUNC pHUD_ClientMoveInit;
	HUD_TEXTURETYPE_FUNC pHUD_ClientMoveTexture;
	HUD_IN_ACTIVATEMOUSE_FUNC pIN_ActivateMouse;
	HUD_IN_DEACTIVATEMOUSE_FUNC pIN_DeactivateMouse;
	HUD_IN_MOUSEEVENT_FUNC pIN_MouseEvent;
	HUD_IN_CLEARSTATES_FUNC pIN_ClearStates;
	HUD_CL_CREATEMOVE_FUNC pCL_CreateMove;
	HUD_CL_ISTHIRDPERSON_FUNC pCL_IsThirdPerson;
	HUD_CL_GETCAMERAOFFSETS_FUNC pCL_CameraOffset;
	HUD_KB_FIND_FUNC pKB_Find;
	HUD_CAMTHINK_FUNC pCAM_Think;
	HUD_CALCREF_FUNC pV_CalcRefdef;
	HUD_ADDENTITY_FUNC pHUD_AddEntity;
	HUD_CREATEENTITIES_FUNC pHUD_CreateEntities;
	HUD_CLIENTSIDEENTITIESDRAW_FUNC pHUD_ClientSideEntitiesDraw;
	HUD_DRAWNORMALTRIS_FUNC pHUD_DrawNormalTriangles;
	HUD_DRAWTRANSTRIS_FUNC pHUD_DrawTransparentTriangles;
	HUD_STUDIOEVENT_FUNC pHUD_StudioEvent;
	HUD_POSTRUNCMD_FUNC pHUD_PostRunCmd;
	HUD_TXFERLOCALOVERRIDES_FUNC pHUD_TxferLocalOverrides;
	HUD_PROCESSPLAYERSTATE_FUNC pHUD_ProcessPlayerState;
	HUD_TXFERPREDICTIONDATA_FUNC pHUD_TxferPredictionData;
	HUD_CONNECTIONLESS_FUNC pHUD_ConnectionlessPacket;
	HUD_GETHULLBOUNDS_FUNC pHUD_GetHullBounds;
	HUD_PREFRAME_FUNC pHUD_PreFrame;
	HUD_FRAME_FUNC pHUD_Frame;
	HUD_POSTFRAME_FUNC pHUD_PostFrame;
	HUD_KEY_EVENT_FUNC pHUD_Key_Event;
	HUD_TEMPENTUPDATE_FUNC pHUD_TempEntUpdate;
	HUD_GETUSERENTITY_FUNC pHUD_GetUserEntity;
	CLIENT_LevelStart_FUNC pCLIENT_LevelStart;
	CLIENT_LevelEnd_FUNC pCLIENT_LevelEnd;
	HUD_IsTeamPlay_FUNC pHUD_IsTeamPlay;
	HUD_ShowLoading_FUNC pHUD_ShowLoading;
	HUD_ShowSaving_FUNC pHUD_ShowSaving;
	CLIENT_SaveState_FUNC pCLIENT_SaveState;
	CLIENT_LoadState_FUNC pCLIENT_LoadState;
} cldll_func_t;

// Function type declarations for client destination functions
typedef int (*DST_INITIALIZE_FUNC)	(struct cl_enginefuncs_s*);
typedef int (*DST_SHUTDOWN_FUNC)	(void);
typedef void (*DST_HUD_INIT_FUNC)	(void);
typedef int (*DST_HUD_VIDINIT_FUNC)	(void);
typedef int (*DST_HUD_REDRAW_FUNC)	(float, int);
typedef int (*DST_HUD_UPDATECLIENTDATA_FUNC) (struct client_data_s*, float);
typedef void (*DST_HUD_RESET_FUNC)    (void);
typedef void (*DST_CL_DRAWSKY_FUNC)    (const float*);
typedef void (*DST_CL_DRAWTERRAIN_FUNC)    (void);
typedef void (*DST_CL_DRAWOCEAN_FUNC)    (void);
typedef void (*DST_HUD_CLIENTMOVE_FUNC)(struct playermove_s* ppmove, qboolean server);
typedef void (*DST_HUD_CLIENTMOVEINIT_FUNC)(struct playermove_s* ppmove);
typedef char (*DST_HUD_TEXTURETYPE_FUNC)(char* name);
typedef void (*DST_HUD_IN_ACTIVATEMOUSE_FUNC) (void);
typedef void (*DST_HUD_IN_DEACTIVATEMOUSE_FUNC)		(void);
typedef void (*DST_HUD_IN_MOUSEEVENT_FUNC)		(int mstate);
typedef void (*DST_HUD_IN_CLEARSTATES_FUNC)		(void);
typedef void (*DST_HUD_CL_CREATEMOVE_FUNC)		(float frametime, struct usercmd_s* cmd, int active);
typedef int (*DST_HUD_CL_ISTHIRDPERSON_FUNC) (void);
typedef void (*DST_HUD_CL_GETCAMERAOFFSETS_FUNC)(float* ofs);
typedef struct kbutton_s* (*DST_HUD_KB_FIND_FUNC) (const char* name);
typedef void (*DST_HUD_CAMTHINK_FUNC)(void);
typedef void (*DST_HUD_CALCREF_FUNC) (struct ref_params_s* pparams);
typedef int	 (*DST_HUD_ADDENTITY_FUNC) (int type, struct cl_entity_s* ent, const char* modelname);
typedef void (*DST_HUD_CREATEENTITIES_FUNC) (void);
typedef void (*DST_HUD_CLIENTSIDEENTITIESDRAW_FUNC) (void);
typedef void (*DST_HUD_DRAWNORMALTRIS_FUNC) (void);
typedef void (*DST_HUD_DRAWTRANSTRIS_FUNC) (void);
typedef void (*DST_HUD_STUDIOEVENT_FUNC) (const struct mstudioevent_s* event, const struct cl_entity_s* entity);
typedef void (*DST_HUD_POSTRUNCMD_FUNC) (struct local_state_s* from, struct local_state_s* to, struct usercmd_s* cmd, int runfuncs, double time, unsigned int random_seed);
typedef void (*DST_HUD_TXFERLOCALOVERRIDES_FUNC)(struct entity_state_s* state, const struct clientdata_s* client);
typedef void (*DST_HUD_PROCESSPLAYERSTATE_FUNC)(struct entity_state_s* dst, const struct entity_state_s* src);
typedef void (*DST_HUD_TXFERPREDICTIONDATA_FUNC) (struct entity_state_s* ps, const struct entity_state_s* pps, struct clientdata_s* pcd, const struct clientdata_s* ppcd, struct weapon_data_s* wd, const struct weapon_data_s* pwd);
typedef int (*DST_HUD_CONNECTIONLESS_FUNC)(const struct netadr_s* net_from, const char* args, char* response_buffer, int* response_buffer_size);
typedef	int	(*DST_HUD_GETHULLBOUNDS_FUNC) (int hullnumber, float* mins, float* maxs);
typedef void (*DST_HUD_PREFRAME_FUNC)		(void);
typedef void (*DST_HUD_FRAME_FUNC)		(double);
typedef void (*DST_HUD_POSTFRAME_FUNC)		(void);
typedef int (*DST_HUD_KEY_EVENT_FUNC) (int eventcode, int keynum, const char* pszCurrentBinding);
typedef void (*DST_HUD_TEMPENTUPDATE_FUNC) (double frametime, double client_time, double cl_gravity, struct tempent_s** ppTempEntFree, struct tempent_s** ppTempEntActive, int (*Callback_AddVisibleEntity)(struct cl_entity_s* pEntity), void (*Callback_TempEntPlaySound)(struct tempent_s* pTemp, float damp));
typedef struct cl_entity_s* (*DST_HUD_GETUSERENTITY_FUNC) (int index, int unused);
typedef void (*DST_CLIENT_LevelStart_FUNC) (void);
typedef void (*DST_CLIENT_LevelEnd_FUNC) (void);
typedef BOOL(*DST_HUD_IsTeamPlay_FUNC) (void);
typedef void(*DST_HUD_ShowLoading_FUNC) (bool unknown, bool unknown2);
typedef void(*DST_HUD_ShowSaving_FUNC) ();
typedef int(*DST_CLIENT_SaveState_FUNC) ();
typedef int(*DST_CLIENT_LoadState_FUNC) ();

// Pointers to the client destination functions
typedef struct
{
	DST_INITIALIZE_FUNC pCLIENT_Initialize;
	DST_SHUTDOWN_FUNC pCLIENT_Shutdown;
	DST_HUD_INIT_FUNC pHUD_Init;
	DST_HUD_VIDINIT_FUNC pHUD_VidInit;
	DST_HUD_REDRAW_FUNC pHUD_Redraw;
	DST_HUD_UPDATECLIENTDATA_FUNC pHUD_UpdateClientData;
	DST_HUD_RESET_FUNC pHUD_Reset;
	DST_CL_DRAWSKY_FUNC pCL_DrawSky;
	DST_CL_DRAWTERRAIN_FUNC pCL_DrawTerrain;
	DST_CL_DRAWOCEAN_FUNC pCL_DrawOcean;
	DST_HUD_CLIENTMOVE_FUNC pHUD_ClientMove;
	DST_HUD_CLIENTMOVEINIT_FUNC pHUD_ClientMoveInit;
	DST_HUD_TEXTURETYPE_FUNC pHUD_ClientMoveTexture;
	DST_HUD_IN_ACTIVATEMOUSE_FUNC pIN_ActivateMouse;
	DST_HUD_IN_DEACTIVATEMOUSE_FUNC pIN_DeactivateMouse;
	DST_HUD_IN_MOUSEEVENT_FUNC pIN_MouseEvent;
	DST_HUD_IN_CLEARSTATES_FUNC pIN_ClearStates;
	DST_HUD_CL_CREATEMOVE_FUNC pCL_CreateMove;
	DST_HUD_CL_ISTHIRDPERSON_FUNC pCL_IsThirdPerson;
	DST_HUD_CL_GETCAMERAOFFSETS_FUNC pCL_CameraOffset;
	DST_HUD_KB_FIND_FUNC pKB_Find;
	DST_HUD_CAMTHINK_FUNC pCAM_Think;
	DST_HUD_CALCREF_FUNC pV_CalcRefdef;
	DST_HUD_ADDENTITY_FUNC pHUD_AddEntity;
	DST_HUD_CREATEENTITIES_FUNC pHUD_CreateEntities;
	DST_HUD_CLIENTSIDEENTITIESDRAW_FUNC pHUD_ClientSideEntitiesDraw;
	DST_HUD_DRAWNORMALTRIS_FUNC pHUD_DrawNormalTriangles;
	DST_HUD_DRAWTRANSTRIS_FUNC pHUD_DrawTransparentTriangles;
	DST_HUD_STUDIOEVENT_FUNC pHUD_StudioEvent;
	DST_HUD_POSTRUNCMD_FUNC pHUD_PostRunCmd;
	DST_HUD_TXFERLOCALOVERRIDES_FUNC pHUD_TxferLocalOverrides;
	DST_HUD_PROCESSPLAYERSTATE_FUNC pHUD_ProcessPlayerState;
	DST_HUD_TXFERPREDICTIONDATA_FUNC pHUD_TxferPredictionData;
	DST_HUD_CONNECTIONLESS_FUNC pHUD_ConnectionlessPacket;
	DST_HUD_GETHULLBOUNDS_FUNC pHUD_GetHullBounds;
	DST_HUD_PREFRAME_FUNC pHUD_PreFrame;
	DST_HUD_FRAME_FUNC pHUD_Frame;
	DST_HUD_POSTFRAME_FUNC pHUD_PostFrame;
	DST_HUD_KEY_EVENT_FUNC pHUD_Key_Event;
	DST_HUD_TEMPENTUPDATE_FUNC pHUD_TempEntUpdate;
	DST_HUD_GETUSERENTITY_FUNC pHUD_GetUserEntity;
	DST_CLIENT_LevelStart_FUNC pCLIENT_LevelStart;
	DST_CLIENT_LevelEnd_FUNC pCLIENT_LevelEnd;
	DST_HUD_IsTeamPlay_FUNC pHUD_IsTeamPlay;
	DST_HUD_ShowLoading_FUNC pHUD_ShowLoading;
	DST_HUD_ShowSaving_FUNC pHUD_ShowSaving;
	DST_CLIENT_SaveState_FUNC pCLIENT_SaveState;
	DST_CLIENT_LoadState_FUNC pCLIENT_LoadState;
} cldll_func_dst_t;




// ********************************************************
// Functions exported by the engine
// ********************************************************

// Function type declarations for engine exports
typedef ConsoleVariable*			(*pfnEngSrc_pfnCreateConsoleVariableClient)		(IConsoleVariable pCvar);
typedef qboolean					(*pfnEngSrc_pfnDestroyConsoleVariableClient)	(IConsoleVariable* pCvar);
typedef ConsoleVariable*			(*pfnEngSrc_pfnGetConsoleVariableClient)		(const char* szVarName);
typedef void						(*pfnEngSrc_pfnRegisterClient)					(IConsoleFunction* pFunc);
typedef void						(*pfnEngSrc_pfnUnregisterClient)				(IConsoleFunction* pFunc);
typedef int							(*pfnEngSrc_pfnGetScreenInfo_t) 				(struct SCREENINFO_s* pscrinfo);
typedef int							(*pfnEngSrc_pfnHookUserMsg_t)					(const char* szMsgName, pfnUserMsgHook pfn);
typedef int							(*pfnEngSrc_pfnServerCmd_t)						(const char* szCmdString);
typedef int							(*pfnEngSrc_pfnClientCmd_t)						(const char* szCmdString);
typedef void						(*pfnEngSrc_pfnGetPlayerInfo_t)					(int ent_num, struct hud_player_info_s* pinfo);
typedef void						(*pfnEngSrc_pfnPlaySoundByName_t)				(const char* szSound, float volume);
typedef void						(*pfnEngSrc_pfnPlaySoundByIndex_t)				(int iSound, float volume);
typedef void						(*pfnEngSrc_pfnAngleVectors_t)					(const float* vecAngles, float* forward, float* right, float* up);
typedef struct client_textmessage_s*(*pfnEngSrc_pfnTextMessageGet_t)				(const char* pName);
typedef int							(*pfnEngSrc_pfnDrawCharacter_t)					(int x, int y, int xnumber, int ynumber, unsigned char r, unsigned char g, unsigned char b, unsigned char a);
typedef void						(*pfnEngSrc_pfnDrawSetTextColor_t)				(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
typedef void						(*pfnEngSrc_pfnConsolePrint_t)					(const char* string);
typedef void						(*pfnEngSrc_pfnCenterPrint_t)					(const char* string);
typedef int							(*pfnEngSrc_GetWindowCenterX_t)					(void);
typedef int							(*pfnEngSrc_GetWindowCenterY_t)					(void);
typedef void						(*pfnEngSrc_GetViewAngles_t)					(float*);
typedef void						(*pfnEngSrc_SetViewAngles_t)					(float*);
typedef int							(*pfnEngSrc_GetMaxClients_t)					(void);
typedef void						(*pfnEngSrc_Con_Printf_t)						(const char* fmt, ...);
typedef void						(*pfnEngSrc_Con_DPrintf_t)						(const char* fmt, ...);
typedef void						(*pfnEngSrc_Con_NPrintf_t)						(int pos, const char* fmt, ...);
typedef const char*					(*pfnEngSrc_PhysInfo_ValueForKey_t)				(const char* key);
typedef const char*					(*pfnEngSrc_ServerInfo_ValueForKey_t)			(const char* key);
typedef float						(*pfnEngSrc_GetClientMaxspeed_t)				(void);
typedef int							(*pfnEngSrc_CheckParm_t)						(const char* parm, char** ppnext);
typedef void						(*pfnEngSrc_Key_Event_t)						(int key, bool down);
typedef void						(*pfnEngSrc_GetMousePosition_t)					(int* mx, int* my);
typedef int							(*pfnEngSrc_IsNoClipping_t)						(void);
typedef struct cl_entity_s*			(*pfnEngSrc_GetLocalPlayer_t)					(void);
typedef struct cl_entity_s*			(*pfnEngSrc_GetViewModel_t)						(void);
typedef struct cl_entity_s*			(*pfnEngSrc_GetEntityByIndex_t)					(int idx);
typedef float						(*pfnEngSrc_GetClientTime_t)					(void);
typedef float						(*pfnEngSrc_GetClientOldTime_t)					(void);
typedef float						(*pfnEngSrc_GetServerGravityValue_t)			();
typedef struct hud_player_info_s*   (*pfnEngSrc_pfnEngGetPlayerInfo_t)				(int ent_num); //DYLAN FIXME, return value!
typedef void						(*pfnEngSrc_V_CalcShake_t)						(void);
typedef void						(*pfnEngSrc_V_ApplyShake_t)						(float* origin, float* angles, float factor);
typedef BOOLEAN						(*pfnEngSrc_PM_Ready)							(void);
typedef int							(*pfnEngSrc_PM_PointContents_t)					(float* point, int* truecontents);
typedef int							(*pfnEngSrc_PM_WaterCheck)						(const float* p);
typedef int							(*pfnEngSrc_PM_WaterEntity_t)					(float* p);
typedef struct pmtrace_s*			(*pfnEngSrc_PM_TraceLine_t)						(float* start, float* end, int brushflags, int flags, int usehull, int ignore_pe);
typedef struct model_s*				(*pfnEngSrc_CL_LoadModel_t)						(const char* modelname, int* index);
typedef int							(*pfnEngSrc_CL_CreateVisibleEntity_t)			(int type, struct cl_entity_s* ent); //EntityTypes first arg
typedef void						(*pfnEngSrc_pfnPlaySoundByNameAtLocation_t)		(const char* szSound, float volume, float* origin);
typedef unsigned short				(*pfnEngSrc_pfnPrecacheEvent_t)					(int type, const char* psz);
typedef void						(*pfnEngSrc_pfnPlaybackEvent_t)					(int flags, const struct edict_s* pInvoker, unsigned short eventindex, float delay, float* origin, float* angles, float fparam1, float fparam2, int iparam1, int iparam2, int bparam1, int bparam2);
typedef void						(*pfnEngSrc_pfnWeaponAnim_t)					(int iAnim, int body);
typedef float						(*pfnEngSrc_pfnRandomFloat_t)					(float flLow, float flHigh);
typedef long						(*pfnEngSrc_pfnRandomLong_t)					(long lLow, long lHigh);
typedef void						(*pfnEngSrc_pfnHookEvent_t)						(const char* name, void (*pfnEvent)(struct event_args_s* args));
typedef int							(*pfnEngSrc_Con_IsVisible_t)					();
typedef const char*					(*pfnEngSrc_pfnGetGameDirectory_t)				(void);
typedef const char*					(*pfnEngSrc_Key_LookupBinding_t)				(const char* pBinding);
typedef const char*					(*pfnEngSrc_pfnGetLevelName_t)					(void);
typedef const char*					(*pfnEngSrc_COM_FileBase)						(const char* name, char* dest, unsigned int size);
typedef byte*						(*pfnEngSrc_COM_LoadHeapFile_t)					(const char* path, int *pLength);
typedef char*						(*pfnEngSrc_COM_ParseFile_t)					(const char* data, char* dest, unsigned int size);
typedef void						(*pfnEngSrc_COM_FreeFile_t)						(void* buffer);
typedef struct model_s*				(*pfnEngSrc_GetModelByIndex_t)					(int index);
typedef int							(*pfnEngSrc_ModelFrameCount)					(struct model_s* model);
typedef char*						(*pfnEngSrc_COM_Parse)							(const char* name);
typedef char*						(*pfnEngSrc_COM_Token)							(void);
//typedef struct triangleapi_s*		pfnEngSrc_RenderAPI;
//typedef struct efx_api_s*			pfnEngSrc_EffectsAPI;
//typedef struct event_api_s*		pfnEngSrc_pEventAPI;
//typedef struct net_api_s*			pfnEngSrc_pNetAPI;
typedef int							(*pfnEngSrc_COM_ExpandFilename_t)				(const char* fileName, char* nameOutBuffer, int nameOutBufferSize);
typedef const char*					(*pfnEngSrc_PlayerInfo_ValueForKey_t)			(int playerNum, const char* key);
typedef void						(*pfnEngSrc_PlayerInfo_SetValueForKey_t)		(const char* key, const char* value);
typedef qboolean					(*pfnEngSrc_GetPlayerUniqueID_t)				(int iPlayer, char playerID[16]);
typedef struct cl_entity_s*			(*pfnEngSrc_GetCameraEntity)					();
typedef int							(*pfnEngSrc_GetPointIllum)						(const float* pos);
typedef void						(*pfnEngSrc_PlayMovie)							(const char* name);
typedef void						(*pfnEngSrc_LevelFog)							(bool enabled, bool ishaze, const int& color, float start, float end);
typedef int							(*pfnEngSrc_GetSpriteFrameTexture)				(struct model_s* model, int frame);
typedef char*						(*pfnEngSrc_GetFontMetrics)						(int fonttype);
typedef void						(*pfnEngSrc_SetFont)							(int fonttype);
typedef void*						(*pfnEngSrc_GUI_Manager)						();
typedef struct model_s*				(*pfnEngSrc_Mod_ForName)						(const char* name, bool unused, bool maybe_clientside);
typedef qboolean					(*pfnEngSrc_CL_FindTouchedLeafs)				(struct cl_entity_s* ent, int listsize, int* list, int* leafcount, int* leaf_topnode);
typedef int							(*pfnEngSrc_CL_GetPVS)							(const float* pos);
typedef qboolean					(*pfnEngSrc_CL_CheckVisibility)					(const unsigned char* pset, int headnode, int* numleafs, int* leafnums, int max_ent_leafs); //MAX_ENT_LEAFS
typedef void						(*pfnEngSrc_SetupModelMaterials)				(struct cl_entity_s* ent);
typedef void						(*pfnEngSrc_DestroyModelMaterials)				(struct cl_entity_s* ent);
typedef void						(*pfnEngSrc_StudioAnimationBounds)				(struct model_s* model, int index, const float* rotation, float* mins, float* maxs);
typedef int							(*pfnEngSrc_CL_GetLevelTextureCount)			();
typedef const char*					(*pfnEngSrc_CL_GetLevelTextureName)				(unsigned int tex);
typedef int							(*pfnEngSrc_CL_GetLevelSurfaceCount)			();
typedef struct msurface_s* const	(*pfnEngSrc_CL_GetLevelSurface)					(unsigned int index);
typedef int							(*pfnEngSrc_CL_GetVertsForSurface)				(struct msurface_s* const surface);
typedef const char*					(*pfnEngSrc_CL_GetEntityString)					();
typedef qboolean					(*pfnEngSrc_CL_SaveRead)						(void* buf, unsigned int size);
typedef qboolean					(*pfnEngSrc_CL_SaveWrite)						(const void* buf, unsigned int size);
typedef int							(*pfnEngSrc_IndexForSurface)					(const msurface_t*);
typedef const msurface_t*			(*pfnEngSrc_SurfaceForIndex)					(int index);

// Pointers to the exported engine functions themselves
typedef struct cl_enginefuncs_s
{
	int version; //11
	int size; //424
	pfnEngSrc_pfnCreateConsoleVariableClient CreateConsoleVariableClient;
	pfnEngSrc_pfnDestroyConsoleVariableClient DestroyConsoleVariableClient;
	pfnEngSrc_pfnGetConsoleVariableClient GetConsoleVariableClient;
	pfnEngSrc_pfnRegisterClient RegisterClient;
	pfnEngSrc_pfnUnregisterClient UnregisterClient;
	pfnEngSrc_pfnGetScreenInfo_t GetScreenInfo_t;
	pfnEngSrc_pfnHookUserMsg_t HookUserMsg_t;
	pfnEngSrc_pfnServerCmd_t ServerCmd;
	pfnEngSrc_pfnClientCmd_t ClientCmd;
	pfnEngSrc_pfnGetPlayerInfo_t GetPlayerInfo;
	pfnEngSrc_pfnPlaySoundByName_t PlaySoundByName;
	pfnEngSrc_pfnPlaySoundByIndex_t PlaySoundByIndex;
	pfnEngSrc_pfnAngleVectors_t AngleVectors;
	pfnEngSrc_pfnTextMessageGet_t TextMessageGet;
	pfnEngSrc_pfnDrawCharacter_t DrawCharacter;
	pfnEngSrc_pfnDrawSetTextColor_t DrawSetTextColor;
	pfnEngSrc_pfnConsolePrint_t ConsolePrint;
	pfnEngSrc_pfnCenterPrint_t CenterPrint;
	pfnEngSrc_GetWindowCenterX_t GetWindowCenterX;
	pfnEngSrc_GetWindowCenterY_t GetWindowCenterY;
	pfnEngSrc_GetViewAngles_t GetViewAngles;
	pfnEngSrc_SetViewAngles_t SetViewAngles;
	pfnEngSrc_GetMaxClients_t GetMaxClients;
	pfnEngSrc_Con_Printf_t Con_Printf;
	pfnEngSrc_Con_DPrintf_t Con_DPrintf;
	pfnEngSrc_Con_NPrintf_t Con_NPrintf;
	pfnEngSrc_PhysInfo_ValueForKey_t PhysInfo_ValueForKey;
	pfnEngSrc_ServerInfo_ValueForKey_t ServerInfo_ValueForKey;
	pfnEngSrc_GetClientMaxspeed_t GetClientMaxspeed;
	pfnEngSrc_CheckParm_t CheckParm;
	pfnEngSrc_Key_Event_t Key_Event;
	pfnEngSrc_GetMousePosition_t GetMousePosition;
	pfnEngSrc_IsNoClipping_t IsNoClipping;
	pfnEngSrc_GetLocalPlayer_t GetLocalPlayer;
	pfnEngSrc_GetViewModel_t GetViewModel;
	pfnEngSrc_GetEntityByIndex_t GetEntityByIndex;
	pfnEngSrc_GetClientTime_t GetClientTime;
	pfnEngSrc_GetClientOldTime_t GetClientOldTime;
	pfnEngSrc_GetServerGravityValue_t GetServerGravityValue;
	pfnEngSrc_pfnEngGetPlayerInfo_t EngGetPlayerInfo;
	pfnEngSrc_V_CalcShake_t V_CalcShake;
	pfnEngSrc_V_ApplyShake_t V_ApplyShake;
	pfnEngSrc_PM_Ready PM_Ready;
	pfnEngSrc_PM_PointContents_t PM_PointContents;
	pfnEngSrc_PM_WaterCheck PM_WaterCheck;
	pfnEngSrc_PM_WaterEntity_t PM_WaterEntity;
	pfnEngSrc_PM_TraceLine_t PM_TraceLine;
	pfnEngSrc_CL_LoadModel_t CL_LoadModel;
	pfnEngSrc_CL_CreateVisibleEntity_t CL_CreateVisibleEntity;
	pfnEngSrc_pfnPlaySoundByNameAtLocation_t PlaySoundByNameAtLocation;
	pfnEngSrc_pfnPrecacheEvent_t PrecacheEvent;
	pfnEngSrc_pfnPlaybackEvent_t PlaybackEvent;
	pfnEngSrc_pfnWeaponAnim_t WeaponAnim;
	pfnEngSrc_pfnRandomFloat_t RandomFloat;
	pfnEngSrc_pfnRandomLong_t RandomLong;
	pfnEngSrc_pfnHookEvent_t HookEvent;
	pfnEngSrc_Con_IsVisible_t Con_IsVisible;
	pfnEngSrc_pfnGetGameDirectory_t GetGameDirectory;
	pfnEngSrc_Key_LookupBinding_t Key_LookupBinding;
	pfnEngSrc_pfnGetLevelName_t GetLevelName;
	pfnEngSrc_COM_FileBase COM_FileBase;
	pfnEngSrc_COM_LoadHeapFile_t COM_LoadHeapFile;
	pfnEngSrc_COM_ParseFile_t COM_ParseFile;
	pfnEngSrc_COM_FreeFile_t COM_FreeFile;
	pfnEngSrc_GetModelByIndex_t GetModelByIndex;
	pfnEngSrc_ModelFrameCount ModelFrameCount;
	pfnEngSrc_COM_Parse COM_Parse;
	pfnEngSrc_COM_Token COM_Token;
	struct triangleapi_s* pRenderAPI;
	struct efx_api_s* pEffectsAPI;
	struct event_api_s* pEventAPI;
	struct net_api_s* pNetAPI;
	pfnEngSrc_COM_ExpandFilename_t COM_ExpandFilename;
	pfnEngSrc_PlayerInfo_ValueForKey_t PlayerInfo_ValueForKey;
	pfnEngSrc_PlayerInfo_SetValueForKey_t PlayerInfo_SetValueForKey;
	pfnEngSrc_GetPlayerUniqueID_t GetPlayerUniqueID;
	pfnEngSrc_GetCameraEntity GetCameraEntity;
	pfnEngSrc_GetPointIllum GetPointIllum;
	pfnEngSrc_PlayMovie PlayMovie;
	pfnEngSrc_LevelFog LevelFog;
	pfnEngSrc_GetSpriteFrameTexture GetSpriteFrameTexture;
	pfnEngSrc_GetFontMetrics GetFontMetrics;
	pfnEngSrc_SetFont SetFont;
	pfnEngSrc_GUI_Manager GUI_Manager;
	pfnEngSrc_Mod_ForName Mod_ForName;
	pfnEngSrc_CL_FindTouchedLeafs CL_FindTouchedLeafs;
	pfnEngSrc_CL_GetPVS CL_GetPVS;
	pfnEngSrc_CL_CheckVisibility CL_CheckVisibility;
	pfnEngSrc_SetupModelMaterials SetupModelMaterials;
	pfnEngSrc_DestroyModelMaterials DestroyModelMaterials;
	pfnEngSrc_StudioAnimationBounds StudioAnimationBounds;
	pfnEngSrc_CL_GetLevelTextureCount CL_GetLevelTextureCount;
	pfnEngSrc_CL_GetLevelTextureName CL_GetLevelTextureName;
	pfnEngSrc_CL_GetLevelSurfaceCount CL_GetLevelSurfaceCount;
	pfnEngSrc_CL_GetLevelSurface CL_GetLevelSurface;
	pfnEngSrc_CL_GetVertsForSurface CL_GetVertsForSurface;
	pfnEngSrc_CL_GetEntityString CL_GetEntityString;
	pfnEngSrc_CL_SaveRead CL_SaveRead;
	pfnEngSrc_CL_SaveWrite CL_SaveWrite;
	pfnEngSrc_IndexForSurface IndexForSurface;
	pfnEngSrc_SurfaceForIndex SurfaceForIndex;
} cl_enginefunc_t;


// Function type declarations for engine destination functions
typedef ConsoleVariable*			(*pfnEngDst_pfnCreateConsoleVariableClient)		(IConsoleVariable pCvar);
typedef qboolean					(*pfnEngDst_pfnDestroyConsoleVariableClient)	(IConsoleVariable* pCvar);
typedef ConsoleVariable*			(*pfnEngDst_pfnGetConsoleVariableClient)		(const char* szVarName);
typedef void						(*pfnEngDst_pfnRegisterClient)					(IConsoleFunction* pFunc);
typedef void						(*pfnEngDst_pfnUnregisterClient)				(IConsoleFunction* pFunc);
typedef int							(*pfnEngDst_pfnGetScreenInfo_t) 				(struct SCREENINFO_s* pscrinfo);
typedef int							(*pfnEngDst_pfnHookUserMsg_t)					(const char* szMsgName, pfnUserMsgHook pfn);
typedef int							(*pfnEngDst_pfnServerCmd_t)						(const char* szCmdString);
typedef int							(*pfnEngDst_pfnClientCmd_t)						(const char* szCmdString);
typedef void						(*pfnEngDst_pfnGetPlayerInfo_t)					(int ent_num, struct hud_player_info_s* pinfo);
typedef void						(*pfnEngDst_pfnPlaySoundByName_t)				(const char* szSound, float volume);
typedef void						(*pfnEngDst_pfnPlaySoundByIndex_t)				(int iSound, float volume);
typedef void						(*pfnEngDst_pfnAngleVectors_t)					(const float* vecAngles, float* forward, float* right, float* up);
typedef struct client_textmessage_s*(*pfnEngDst_pfnTextMessageGet_t)				(const char* pName);
typedef int							(*pfnEngDst_pfnDrawCharacter_t)					(int x, int y, int xnumber, int ynumber, unsigned char r, unsigned char g, unsigned char b, unsigned char a);
typedef void						(*pfnEngDst_pfnDrawSetTextColor_t)				(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
typedef void						(*pfnEngDst_pfnConsolePrint_t)					(const char* string);
typedef void						(*pfnEngDst_pfnCenterPrint_t)					(const char* string);
typedef int							(*pfnEngDst_GetWindowCenterX_t)					(void);
typedef int							(*pfnEngDst_GetWindowCenterY_t)					(void);
typedef void						(*pfnEngDst_GetViewAngles_t)					(float*);
typedef void						(*pfnEngDst_SetViewAngles_t)					(float*);
typedef int							(*pfnEngDst_GetMaxClients_t)					(void);
typedef void						(*pfnEngDst_Con_Printf_t)						(const char* fmt, ...);
typedef void						(*pfnEngDst_Con_DPrintf_t)						(const char* fmt, ...);
typedef void						(*pfnEngDst_Con_NPrintf_t)						(int pos, const char* fmt, ...);
typedef const char*					(*pfnEngDst_PhysInfo_ValueForKey_t)				(const char* key);
typedef const char*					(*pfnEngDst_ServerInfo_ValueForKey_t)			(const char* key);
typedef float						(*pfnEngDst_GetClientMaxspeed_t)				(void);
typedef int							(*pfnEngDst_CheckParm_t)						(const char* parm, char** ppnext);
typedef void						(*pfnEngDst_Key_Event_t)						(int key, bool down);
typedef void						(*pfnEngDst_GetMousePosition_t)					(int* mx, int* my);
typedef int							(*pfnEngDst_IsNoClipping_t)						(void);
typedef struct cl_entity_s*			(*pfnEngDst_GetLocalPlayer_t)					(void);
typedef struct cl_entity_s*			(*pfnEngDst_GetViewModel_t)						(void);
typedef struct cl_entity_s*			(*pfnEngDst_GetEntityByIndex_t)					(int idx);
typedef float						(*pfnEngDst_GetClientTime_t)					(void);
typedef float						(*pfnEngDst_GetClientOldTime_t)					(void);
typedef float						(*pfnEngDst_GetServerGravityValue_t)			();
typedef struct hud_player_info_s*   (*pfnEngDst_pfnEngGetPlayerInfo_t)				(int ent_num); //DYLAN FIXME, return value!
typedef void						(*pfnEngDst_V_CalcShake_t)						(void);
typedef void						(*pfnEngDst_V_ApplyShake_t)						(float* origin, float* angles, float factor);
typedef BOOLEAN						(*pfnEngDst_PM_Ready)							(void);
typedef int							(*pfnEngDst_PM_PointContents_t)					(float* point, int* truecontents);
typedef int							(*pfnEngDst_PM_WaterCheck)						(const float* p);
typedef int							(*pfnEngDst_PM_WaterEntity_t)					(float* p);
typedef struct pmtrace_s*			(*pfnEngDst_PM_TraceLine_t)						(float* start, float* end, int brushflags, int flags, int usehull, int ignore_pe);
typedef struct model_s*				(*pfnEngDst_CL_LoadModel_t)						(const char* modelname, int* index);
typedef int							(*pfnEngDst_CL_CreateVisibleEntity_t)			(int type, struct cl_entity_s* ent); //EntityTypes first arg
typedef void						(*pfnEngDst_pfnPlaySoundByNameAtLocation_t)		(const char* szSound, float volume, float* origin);
typedef unsigned short				(*pfnEngDst_pfnPrecacheEvent_t)					(int type, const char* psz);
typedef void						(*pfnEngDst_pfnPlaybackEvent_t)					(int flags, const struct edict_s* pInvoker, unsigned short eventindex, float delay, float* origin, float* angles, float fparam1, float fparam2, int iparam1, int iparam2, int bparam1, int bparam2);
typedef void						(*pfnEngDst_pfnWeaponAnim_t)					(int iAnim, int body);
typedef float						(*pfnEngDst_pfnRandomFloat_t)					(float flLow, float flHigh);
typedef long						(*pfnEngDst_pfnRandomLong_t)					(long lLow, long lHigh);
typedef void						(*pfnEngDst_pfnHookEvent_t)						(const char* name, void (*pfnEvent)(struct event_args_s* args));
typedef int							(*pfnEngDst_Con_IsVisible_t)					();
typedef const char*					(*pfnEngDst_pfnGetGameDirectory_t)				(void);
typedef const char*					(*pfnEngDst_Key_LookupBinding_t)				(const char* pBinding);
typedef const char*					(*pfnEngDst_pfnGetLevelName_t)					(void);
typedef const char*					(*pfnEngDst_COM_FileBase)						(const char* name, char* dest, unsigned int size);
typedef byte*						(*pfnEngDst_COM_LoadHeapFile_t)					(const char* path, int *pLength);
typedef char*						(*pfnEngDst_COM_ParseFile_t)					(const char* data, char* dest, unsigned int size);
typedef void						(*pfnEngDst_COM_FreeFile_t)						(void* buffer);
typedef struct model_s*				(*pfnEngDst_GetModelByIndex_t)					(int index);
typedef int							(*pfnEngDst_ModelFrameCount)					(struct model_s* model);
typedef char*						(*pfnEngDst_COM_Parse)							(const char* name);
typedef char*						(*pfnEngDst_COM_Token)							(void);
//typedef struct triangleapi_s*		pfnEngDst_RenderAPI;
//typedef struct efx_api_s*			pfnEngDst_EffectsAPI;
//typedef struct event_api_s*		pfnEngDst_pEventAPI;
//typedef struct net_api_s*			pfnEngDst_pNetAPI;
typedef int							(*pfnEngDst_COM_ExpandFilename_t)				(const char* fileName, char* nameOutBuffer, int nameOutBufferSize);
typedef const char*					(*pfnEngDst_PlayerInfo_ValueForKey_t)			(int playerNum, const char* key);
typedef void						(*pfnEngDst_PlayerInfo_SetValueForKey_t)		(const char* key, const char* value);
typedef qboolean					(*pfnEngDst_GetPlayerUniqueID_t)				(int iPlayer, char playerID[16]);
typedef struct cl_entity_s*			(*pfnEngDst_GetCameraEntity)					();
typedef int							(*pfnEngDst_GetPointIllum)						(const float* pos);
typedef void						(*pfnEngDst_PlayMovie)							(const char* name);
typedef void						(*pfnEngDst_LevelFog)							(bool enabled, bool ishaze, const int& color, float start, float end);
typedef int							(*pfnEngDst_GetSpriteFrameTexture)				(struct model_s* model, int frame);
typedef char*						(*pfnEngDst_GetFontMetrics)						(int fonttype);
typedef void						(*pfnEngDst_SetFont)							(int fonttype);
typedef void*						(*pfnEngDst_GUI_Manager)						();
typedef struct model_s*				(*pfnEngDst_Mod_ForName)						(const char* name, bool unused, bool maybe_clientside);
typedef qboolean					(*pfnEngDst_CL_FindTouchedLeafs)				(struct cl_entity_s* ent, int listsize, int* list, int* leafcount, int* leaf_topnode);
typedef int							(*pfnEngDst_CL_GetPVS)							(const float* pos);
typedef qboolean					(*pfnEngDst_CL_CheckVisibility)					(const unsigned char* pset, int headnode, int* numleafs, int* leafnums, int max_ent_leafs); //MAX_ENT_LEAFS
typedef void						(*pfnEngDst_SetupModelMaterials)				(struct cl_entity_s* ent);
typedef void						(*pfnEngDst_DestroyModelMaterials)				(struct cl_entity_s* ent);
typedef void						(*pfnEngDst_StudioAnimationBounds)				(struct model_s* model, int index, const float* rotation, float* mins, float* maxs);
typedef int							(*pfnEngDst_CL_GetLevelTextureCount)			();
typedef const char*					(*pfnEngDst_CL_GetLevelTextureName)				(unsigned int tex);
typedef int							(*pfnEngDst_CL_GetLevelSurfaceCount)			();
typedef struct msurface_s* const	(*pfnEngDst_CL_GetLevelSurface)					(unsigned int index);
typedef int							(*pfnEngDst_CL_GetVertsForSurface)				(struct msurface_s* const surface);
typedef const char*					(*pfnEngDst_CL_GetEntityString)					();
typedef qboolean					(*pfnEngDst_CL_SaveRead)						(void* buf, unsigned int size);
typedef qboolean					(*pfnEngDst_CL_SaveWrite)						(const void* buf, unsigned int size);
typedef int							(*pfnEngDst_IndexForSurface)					(const msurface_t*);
typedef const msurface_t*			(*pfnEngDst_SurfaceForIndex)					(int index);


// Pointers to the engine destination functions
typedef struct
{
	int version; //11
	int size; //424
	pfnEngDst_pfnCreateConsoleVariableClient CreateConsoleVariableClient;
	pfnEngDst_pfnDestroyConsoleVariableClient DestroyConsoleVariableClient;
	pfnEngDst_pfnGetConsoleVariableClient GetConsoleVariableClient;
	pfnEngDst_pfnRegisterClient RegisterClient;
	pfnEngDst_pfnUnregisterClient UnregisterClient;
	pfnEngDst_pfnGetScreenInfo_t GetScreenInfo;
	pfnEngDst_pfnHookUserMsg_t HookUserMsg;
	pfnEngDst_pfnServerCmd_t ServerCmd;
	pfnEngDst_pfnClientCmd_t ClientCmd;
	pfnEngDst_pfnGetPlayerInfo_t GetPlayerInfo;
	pfnEngDst_pfnPlaySoundByName_t PlaySoundByName;
	pfnEngDst_pfnPlaySoundByIndex_t PlaySoundByIndex;
	pfnEngDst_pfnAngleVectors_t AngleVectors;
	pfnEngDst_pfnTextMessageGet_t TextMessageGet;
	pfnEngDst_pfnDrawCharacter_t DrawCharacter;
	pfnEngDst_pfnDrawSetTextColor_t DrawSetTextColor;
	pfnEngDst_pfnConsolePrint_t ConsolePrint;
	pfnEngDst_pfnCenterPrint_t CenterPrint;
	pfnEngDst_GetWindowCenterX_t GetWindowCenterX;
	pfnEngDst_GetWindowCenterY_t GetWindowCenterY;
	pfnEngDst_GetViewAngles_t GetViewAngles;
	pfnEngDst_SetViewAngles_t SetViewAngles;
	pfnEngDst_GetMaxClients_t GetMaxClients;
	pfnEngDst_Con_Printf_t Con_Printf;
	pfnEngDst_Con_DPrintf_t Con_DPrintf;
	pfnEngDst_Con_NPrintf_t Con_NPrintf;
	pfnEngDst_PhysInfo_ValueForKey_t PhysInfo_ValueForKey;
	pfnEngDst_ServerInfo_ValueForKey_t ServerInfo_ValueForKey;
	pfnEngDst_GetClientMaxspeed_t GetClientMaxspeed;
	pfnEngDst_CheckParm_t CheckParm;
	pfnEngDst_Key_Event_t Key_Event;
	pfnEngDst_GetMousePosition_t GetMousePosition;
	pfnEngDst_IsNoClipping_t IsNoClipping;
	pfnEngDst_GetLocalPlayer_t GetLocalPlayer;
	pfnEngDst_GetViewModel_t GetViewModel;
	pfnEngDst_GetEntityByIndex_t GetEntityByIndex;
	pfnEngDst_GetClientTime_t GetClientTime;
	pfnEngDst_GetClientOldTime_t GetClientOldTime;
	pfnEngDst_GetServerGravityValue_t GetServerGravityValue;
	pfnEngDst_pfnEngGetPlayerInfo_t EngGetPlayerInfo;
	pfnEngDst_V_CalcShake_t V_CalcShake;
	pfnEngDst_V_ApplyShake_t V_ApplyShake;
	pfnEngDst_PM_Ready PM_Ready;
	pfnEngDst_PM_PointContents_t PM_PointContents;
	pfnEngDst_PM_WaterCheck PM_WaterCheck;
	pfnEngDst_PM_WaterEntity_t PM_WaterEntity;
	pfnEngDst_PM_TraceLine_t PM_TraceLine;
	pfnEngDst_CL_LoadModel_t CL_LoadModel;
	pfnEngDst_CL_CreateVisibleEntity_t CL_CreateVisibleEntity;
	pfnEngDst_pfnPlaySoundByNameAtLocation_t PlaySoundByNameAtLocation;
	pfnEngDst_pfnPrecacheEvent_t PrecacheEvent;
	pfnEngDst_pfnPlaybackEvent_t PlaybackEvent;
	pfnEngDst_pfnWeaponAnim_t WeaponAnim;
	pfnEngDst_pfnRandomFloat_t RandomFloat;
	pfnEngDst_pfnRandomLong_t RandomLong;
	pfnEngDst_pfnHookEvent_t HookEvent;
	pfnEngDst_Con_IsVisible_t Con_IsVisible;
	pfnEngDst_pfnGetGameDirectory_t GetGameDirectory;
	pfnEngDst_Key_LookupBinding_t Key_LookupBinding;
	pfnEngDst_pfnGetLevelName_t GetLevelName;
	pfnEngDst_COM_FileBase COM_FileBase;
	pfnEngDst_COM_LoadHeapFile_t COM_LoadHeapFile;
	pfnEngDst_COM_ParseFile_t COM_ParseFile;
	pfnEngDst_COM_FreeFile_t COM_FreeFile;
	pfnEngDst_GetModelByIndex_t GetModelByIndex;
	pfnEngDst_ModelFrameCount ModelFrameCount;
	pfnEngDst_COM_Parse COM_Parse;
	pfnEngDst_COM_Token COM_Token;
	struct triangleapi_s* pRenderAPI;
	struct efx_api_s* pEffectsAPI;
	struct event_api_s* pEventAPI;
	struct net_api_s* pNetAPI;
	pfnEngDst_COM_ExpandFilename_t COM_ExpandFilename;
	pfnEngDst_PlayerInfo_ValueForKey_t PlayerInfo_ValueForKey;
	pfnEngDst_PlayerInfo_SetValueForKey_t PlayerInfo_SetValueForKey;
	pfnEngDst_GetPlayerUniqueID_t GetPlayerUniqueID;
	pfnEngDst_GetCameraEntity GetCameraEntity;
	pfnEngDst_GetPointIllum GetPointIllum;
	pfnEngDst_PlayMovie PlayMovie;
	pfnEngDst_LevelFog LevelFog;
	pfnEngDst_GetSpriteFrameTexture GetSpriteFrameTexture;
	pfnEngDst_GetFontMetrics GetFontMetrics;
	pfnEngDst_SetFont SetFont;
	pfnEngDst_GUI_Manager GUI_Manager;
	pfnEngDst_Mod_ForName Mod_ForName;
	pfnEngDst_CL_FindTouchedLeafs CL_FindTouchedLeafs;
	pfnEngDst_CL_GetPVS CL_GetPVS;
	pfnEngDst_CL_CheckVisibility CL_CheckVisibility;
	pfnEngDst_SetupModelMaterials SetupModelMaterials;
	pfnEngDst_DestroyModelMaterials DestroyModelMaterials;
	pfnEngDst_StudioAnimationBounds StudioAnimationBounds;
	pfnEngDst_CL_GetLevelTextureCount CL_GetLevelTextureCount;
	pfnEngDst_CL_GetLevelTextureName CL_GetLevelTextureName;
	pfnEngDst_CL_GetLevelSurfaceCount CL_GetLevelSurfaceCount;
	pfnEngDst_CL_GetLevelSurface CL_GetLevelSurface;
	pfnEngDst_CL_GetVertsForSurface CL_GetVertsForSurface;
	pfnEngDst_CL_GetEntityString CL_GetEntityString;
	pfnEngDst_CL_SaveRead CL_SaveRead;
	pfnEngDst_CL_SaveWrite CL_SaveWrite;
	pfnEngDst_IndexForSurface IndexForSurface;
	pfnEngDst_SurfaceForIndex SurfaceForIndex;
} cl_enginefunc_dst_t;

// ********************************************************
// Functions exposed by the engine to the module
// ********************************************************

// Functions for ModuleS
typedef void (*PFN_KICKPLAYER)(int nPlayerSlot, int nReason);

typedef struct modshelpers_s
{
	PFN_KICKPLAYER m_pfnKickPlayer;

	// reserved for future expansion
	int m_nVoid1;
	int m_nVoid2;
	int m_nVoid3;
	int m_nVoid4;
	int m_nVoid5;
	int m_nVoid6;
	int m_nVoid7;
	int m_nVoid8;
	int m_nVoid9;
} modshelpers_t;

// Functions for moduleC
typedef struct modchelpers_s
{
	// reserved for future expansion
	int m_nVoid0;
	int m_nVoid1;
	int m_nVoid2;
	int m_nVoid3;
	int m_nVoid4;
	int m_nVoid5;
	int m_nVoid6;
	int m_nVoid7;
	int m_nVoid8;
	int m_nVoid9;
} modchelpers_t;


// ********************************************************
// Information about the engine
// ********************************************************
typedef struct engdata_s
{
	cl_enginefunc_t	*pcl_enginefuncs;		// functions exported by the engine
	cl_enginefunc_dst_t *pg_engdstAddrs;	// destination handlers for engine exports
	cldll_func_t *pcl_funcs;				// client exports
	cldll_func_dst_t *pg_cldstAddrs;		// client export destination handlers
	struct modfuncs_s *pg_modfuncs;			// engine's pointer to module functions
	struct cmd_function_s **pcmd_functions;	// list of all registered commands
	void *pkeybindings;						// all key bindings (not really a void *, but easier this way)
	void (*pfnConPrintf)(char *, ...);		// dump to console
	struct cvar_s **pcvar_vars;				// pointer to head of cvar list
	struct glwstate_t *pglwstate;			// OpenGl information
	void *(*pfnSZ_GetSpace)(struct sizebuf_s *, int); // pointer to SZ_GetSpace
	struct modfuncs_s *pmodfuncs;			// &g_modfuncs
	void *pfnGetProcAddress;				// &GetProcAddress
	void *pfnGetModuleHandle;				// &GetModuleHandle
	struct server_static_s *psvs;			// &svs
	struct client_static_s *pcls;			// &cls
	void (*pfnSV_DropClient)(struct client_s *, qboolean, char *, ...);	// pointer to SV_DropClient
	void (*pfnNetchan_Transmit)(struct netchan_s *, int, byte *);		// pointer to Netchan_Transmit
	void (*pfnNET_SendPacket)(enum netsrc_s sock, int length, void *data, netadr_t to); // &NET_SendPacket
	struct cvar_s *(*pfnCvarFindVar)(const char *pchName);				// pointer to Cvar_FindVar
	int *phinstOpenGlEarly;					// &g_hinstOpenGlEarly

	// Reserved for future expansion
	void *pVoid0;							// reserved for future expan
	void *pVoid1;							// reserved for future expan
	void *pVoid2;							// reserved for future expan
	void *pVoid3;							// reserved for future expan
	void *pVoid4;							// reserved for future expan
	void *pVoid5;							// reserved for future expan
	void *pVoid6;							// reserved for future expan
	void *pVoid7;							// reserved for future expan
	void *pVoid8;							// reserved for future expan
	void *pVoid9;							// reserved for future expan
} engdata_t;


// ********************************************************
// Functions exposed by the security module
// ********************************************************
typedef void (*PFN_LOADMOD)(char *pchModule);
typedef void (*PFN_CLOSEMOD)(void);
typedef int (*PFN_NCALL)(int ijump, int cnArg, ...);

typedef void (*PFN_GETCLDSTADDRS)(cldll_func_dst_t *pcldstAddrs);
typedef void (*PFN_GETENGDSTADDRS)(cl_enginefunc_dst_t *pengdstAddrs);
typedef void (*PFN_MODULELOADED)(void);

typedef void (*PFN_PROCESSOUTGOINGNET)(struct netchan_s *pchan, struct sizebuf_s *psizebuf);
typedef qboolean (*PFN_PROCESSINCOMINGNET)(struct netchan_s *pchan, struct sizebuf_s *psizebuf);

typedef void (*PFN_TEXTURELOAD)(char *pszName, int dxWidth, int dyHeight, char *pbData);
typedef void (*PFN_MODELLOAD)(struct model_s *pmodel, void *pvBuf);

typedef void (*PFN_FRAMEBEGIN)(void);
typedef void (*PFN_FRAMERENDER1)(void);
typedef void (*PFN_FRAMERENDER2)(void);

typedef void (*PFN_SETMODSHELPERS)(modshelpers_t *pmodshelpers);
typedef void (*PFN_SETMODCHELPERS)(modchelpers_t *pmodchelpers);
typedef void (*PFN_SETENGDATA)(engdata_t *pengdata);

typedef void (*PFN_CONNECTCLIENT)(int iPlayer);
typedef void (*PFN_RECORDIP)(unsigned int pnIP);
typedef void (*PFN_PLAYERSTATUS)(unsigned char *pbData, int cbData);

typedef void (*PFN_SETENGINEVERSION)(int nVersion);

// typedef class CMachine *(*PFN_PCMACHINE)(void);
typedef int (*PFN_PCMACHINE)(void);
typedef void (*PFN_SETIP)(int ijump);
typedef void (*PFN_EXECUTE)(void);

typedef struct modfuncs_s
{
	// Functions for the pcode interpreter
	PFN_LOADMOD m_pfnLoadMod;
	PFN_CLOSEMOD m_pfnCloseMod;
	PFN_NCALL m_pfnNCall;

	// API destination functions
	PFN_GETCLDSTADDRS m_pfnGetClDstAddrs;
	PFN_GETENGDSTADDRS m_pfnGetEngDstAddrs;

	// Miscellaneous functions
	PFN_MODULELOADED m_pfnModuleLoaded;     // Called right after the module is loaded

	// Functions for processing network traffic
	PFN_PROCESSOUTGOINGNET m_pfnProcessOutgoingNet;   // Every outgoing packet gets run through this
	PFN_PROCESSINCOMINGNET m_pfnProcessIncomingNet;   // Every incoming packet gets run through this

	// Resource functions
	PFN_TEXTURELOAD m_pfnTextureLoad;     // Called as each texture is loaded
	PFN_MODELLOAD m_pfnModelLoad;         // Called as each model is loaded

	// Functions called every frame
	PFN_FRAMEBEGIN m_pfnFrameBegin;       // Called at the beginning of each frame cycle
	PFN_FRAMERENDER1 m_pfnFrameRender1;   // Called at the beginning of the render loop
	PFN_FRAMERENDER2 m_pfnFrameRender2;   // Called at the end of the render loop

	// Module helper transfer
	PFN_SETMODSHELPERS m_pfnSetModSHelpers;
	PFN_SETMODCHELPERS m_pfnSetModCHelpers;
	PFN_SETENGDATA m_pfnSetEngData;

	// Which version of the module is this?
	int m_nVersion;

	// Miscellaneous game stuff
	PFN_CONNECTCLIENT m_pfnConnectClient;	// Called whenever a new client connects
	PFN_RECORDIP m_pfnRecordIP;				// Secure master has reported a new IP for us
	PFN_PLAYERSTATUS m_pfnPlayerStatus;		// Called whenever we receive a PlayerStatus packet

	// Recent additions
	PFN_SETENGINEVERSION m_pfnSetEngineVersion;	// 1 = patched engine

	// reserved for future expansion
	int m_nVoid2;
	int m_nVoid3;
	int m_nVoid4;
	int m_nVoid5;
	int m_nVoid6;
	int m_nVoid7;
	int m_nVoid8;
	int m_nVoid9;
} modfuncs_t;


#define k_nEngineVersion15Base		0
#define k_nEngineVersion15Patch		1
#define k_nEngineVersion16Base		2
#define k_nEngineVersion16Validated	3		// 1.6 engine with built-in validation


typedef struct validator_s
{
	int m_nRandomizer;			// Random number to be XOR'd into all subsequent fields
	int m_nSignature1;			// First signature that identifies this structure
	int m_nSignature2;			// Second signature
	int m_pbCode;				// Beginning of the code block
	int m_cbCode;				// Size of the code block
	int m_nChecksum;			// Checksum of the code block
	int m_nSpecial;				// For engine, 1 if hw.dll, 0 if sw.dll.  For client, pclfuncs checksum
	int m_nCompensator;			// Keeps the checksum correct
} validator_t;


#define k_nChecksumCompensator 0x36a8f09c	// Don't change this value: it's hardcorded in cdll_int.cpp, 

#define k_nModuleVersionCur 0x43210004


#endif // __APIPROXY__
