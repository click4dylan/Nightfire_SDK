//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

// exefuncs.h
#ifndef EXEFUNCS_H
#define EXEFUNCS_H

#include "nightfire_other.h"

// Engine hands this to DLLs for functionality callbacks
typedef struct exefuncs_s
{
	int	version;
	int	iSizeOfStruct;
	int	isDedicated;
	void* render;
	void* renderplatforminfo;
	void* renderfeatures;
	CreateRenderMaterialT CreateRenderMaterial;
	CreateRenderViewportT CreateRenderViewport;
	Render_DoRenderT Render_DoRender;
	Render_DoFlipT Render_DoFlip;
	AppActivateT AppActivate; //sub_401060
	CDAudio_PlayT CDAudio_Play;
	CDAudio_PauseT CDAudio_Pause;
	CDAudio_ResumeT CDAudio_Resume;
	CDAudio_UpdateT CDAudio_Update;
	ErrorMessageT ErrorMessage;; //4010d0
	Sys_PrintfT Sys_Printf; //401040
	IsValidCDT IsValidCD;
	GetCDKeyT GetCDKey;
	ChangeGameDirectoryT ChangeGameDirectory;
	Launcher_PlayMovieT Launcher_PlayMovie;
	Launcher_OpenFrontEndT Launcher_OpenFrontEnd;
	getOSVersionT getOSVersion;
	GUI_ManagerT GUI_Manager;
} exefuncs_t;

#endif
