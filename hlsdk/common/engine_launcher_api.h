//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

// engine/launcher interface
#if !defined( ENGINE_LAUNCHER_APIH )
#define ENGINE_LAUNCHER_APIH
#ifdef _WIN32
#ifndef __MINGW32__
#pragma once
#endif /* not __MINGW32__ */
#endif

//typedef void ( *xcommand_t ) ( void );

#define RENDERTYPE_UNDEFINED	0
#define RENDERTYPE_SOFTWARE		1
#define RENDERTYPE_HARDWARE		2

#define ENGINE_LAUNCHER_API_VERSION 1
#include "cvardef.h"
typedef struct engine_api_s
{
	int		version;
	int		size;

	// Functions
	CConsoleVariable*	( *CreateConsoleVariable)				(IConsoleVariable pCvar ); //CvarRegister
	bool	( *DestroyConsoleVariable)			(IConsoleVariable*pCvar);
	void	( *GetConsoleVariable )				( const char *cmd);
	void	( *RegisterConsoleFunction )		(IConsoleFunction*func );
	void	( *UnregisterConsoleFunction )		(IConsoleFunction* func);
	int		( *LoadTexture_Launcher )			(char const* one, bool two, bool three, bool four );
	int		( *UnloadTexture_Launcher)			(RenderTextureAPI* launcher );
	int		( *CreateRenderMaterial )			( const char *name );
	int		( *GetEngineState )				( void );
	void	( *ConsoleBuffer_AddText )		( const char *text ); // append cmd at end of buf
	void	( *ConsoleBuffer_InsertText )	( const char *text ); // insert cmd at start of buf
	void	( *Con_Printf )					( char *, ... );
	void	( *Con_SafePrintf )				( char *, ... );
	void	( *ForceReloadProfile )			( void );
	void	( *GameSetBackground )			( unsigned char disable );
	void	( *GameSetState )				( int iState );
	void	( *GameSetSubState )			( int iState );
	bool	( *GetPauseState )				( void );
	int		( *Host_Frame )					( float time, int iState, int *stateInfo );
	void	( *Host_GetHostInfo )			( float *fps, int *nActive, int *nSpectators, int *nMaxPlayers, char *pszMap, int sizeOfMapString, float *flInKB, float *flOutKB );
	void	( *Host_Shutdown )				( void );
	bool	( *Game_Init )					( char *lpCmdLine, unsigned char *pMem, int iSize, struct exefuncs_s *pef, HWND* g_EngWnd, void *, int nosound, const char* cdpath );
	void	( *SCR_SetWindowPosition )		( int x, int y );
	void	( *IN_ActivateMouse )			( void );
	void	( *IN_ClearStates )				( void );
	void	( *IN_DeactivateMouse )			( void );
	void	( *IN_MouseEvent )				( int mstate );
	void	( *Keyboard_ReturnToGame )		( void );
	void	( *Key_ClearStates )			( void );
	void	( *Key_Event )					( int key, int down );
	int		( *LoadGame )					( const char *pszSlot );
	int		( *SaveGame )					( const char *pszSlot, const char *pszComment );
	void	( *SetMessagePumpDisableMode )	( bool bMode);
	void	( *SetPauseState )				( unsigned char bPause );
	void	( *SetStartupMode )				( unsigned char bMode );
	void	( *StoreProfile )				( void );
	double	( *Sys_FloatTime )				( void );
	void	( *S_BlockSound )				( void );
	void	( *S_UnblockSound )				( void );
	unsigned char* ( *COM_LoadTempFile )	( const char *path, int *pLength );
	unsigned char* ( *COM_LoadHeapFile )	( const char *path, int *pLength );
	void	( *COM_FreeFile )				( unsigned char *buffer );
	int		( *COM_ExpandFilename )			( char *filename, unsigned int unknown );
	void	( *Key_CharEvent )				( int one );
} engine_api_t;

//struct of engine/renderd3d functions to pass to gui.dll
typedef struct gui_imports_s
{
	int	version;
	int	iSizeOfStruct;
	void* render;
	void* renderfeatures;
	int(*CreateRenderMaterial)				(const char *name);
	unsigned CreateRenderVideoTexture;
	int(*LoadTexture_Launcher)			(const char* one, bool two, bool three, bool four);
	int(*UnloadTexture_Launcher)			(RenderTextureAPI* launcher);
	void(*Con_Printf)					(char *, ...);
	void(*ConsoleBuffer_AddText)				(const char *text);
	void(*ConsoleBuffer_InsertText)			(const char *text);
	void(*GetConsoleVariable)			(const char *cmd);
	unsigned char* (*COM_LoadTempFile)			(const char *path, int *pLength);
	unsigned char* (*COM_LoadHeapFile)	(const char *path, int *pLength);
	void(*COM_FreeFile)					(unsigned char *buffer);
	int(*COM_ExpandFilename)			(char *filename, unsigned int unknown);
} gui_imports;

//struct that gui.dll passes to exe
typedef struct gui_api_s
{
	int version;
	unsigned size;
	void(*GUI_Init) (gui_imports* imports);
	void (*GUI_Shutdown) (void);
	DWORD*(*GUI_Manager) (void);
} gui_api_t;

#endif // ENGINE_LAUNCHER_APIH
