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
#ifndef USERCMD_H
#define USERCMD_H
#ifdef _WIN32
#ifndef __MINGW32__
#pragma once
#endif /* not __MINGW32__ */
#endif
/*
typedef struct usercmd_s
{
	short	lerp_msec;      // Interpolation time on client
	byte	msec;           // Duration in ms of command
	vec3_t	viewangles;     // Command view angles.

// intended velocities
	float	forwardmove;    // Forward velocity.
	float	sidemove;       // Sideways velocity.
	float	upmove;         // Upward velocity.
	byte	lightlevel;     // Light level at spot where we are standing.
	unsigned short  buttons;  // Attack buttons
	byte    impulse;          // Impulse command issued.
	byte	weaponselect;	// Current weapon id

// Experimental player impact stuff.
	int		impact_index;
	vec3_t	impact_position;
} usercmd_t;
*/
//Nightfire below
//size: 0x38/56
#if 1
typedef struct usercmd_s
{
	byte	lerp_msec;      // Interpolation time on client
	byte    pad1;
	byte	msec;           // Duration in ms of command / client's frametime //2
	byte    pad2;
	vec3_t	viewangles;     // Command view angles. //0x4

// intended velocities
	float	forwardmove;    // Forward velocity. //15    //0x10
	float	sidemove;       // Sideways velocity. //19 //0x14
	float	upmove;         // Upward velocity. //23 //0x18
	byte	lightlevel;     // Light level at spot where we are standing. //27 //0x1C
	byte    pad3[2];
	int		buttons;  // Attack buttons //28 //0x20
	byte    impulse;          // Impulse command issued. //32 //SAME TEST IT //0x24
	byte	weaponselect;	// Current weapon id //33 //DYLAN TODO: TEST THIS TO SEE IF SIZE IS CORRECT //0x25
	//byte pad4[3]; //correct but i messed up previously..
// Experimental player impact stuff.
	float frametime;
	float time;
	float pad4[2];

	//int		impact_index; //used by patch to specify we use float for msec/frametime //0x28 is right
	//vec3_t	impact_position; //patch, 0 = frametime, 1 = gpGlobals->time //0x2C, 0x30, 0x34
} usercmd_t;
#else
typedef struct usercmd_s
{
	byte	lerp_msec;      // Interpolation time on client
	byte    pad1;
	byte	msec;           // Duration in ms of command / client's frametime //2
	byte    pad2;
	vec3_t	viewangles;     // Command view angles. //0x4

// intended velocities
	float	forwardmove;    // Forward velocity. //15    //0x10
	float	sidemove;       // Sideways velocity. //19 //0x14
	float	upmove;         // Upward velocity. //23 //0x18
	byte	lightlevel;     // Light level at spot where we are standing. //27 //0x1C
	byte    pad3[2];
	int		buttons;  // Attack buttons //28 //0x20
	byte    impulse;          // Impulse command issued. //32 //SAME TEST IT //0x24
	byte	weaponselect;	// Current weapon id //33 //DYLAN TODO: TEST THIS TO SEE IF SIZE IS CORRECT //0x25
	//byte pad4[3]; //correct but i messed up previously..
// Experimental player impact stuff.
	int		impact_index; //used by patch to specify we use float for msec/frametime //0x28 is right
	vec3_t	impact_position; //patch, 0 = frametime, 1 = gpGlobals->time //0x2C, 0x30, 0x34
	//byte pad4[3]; //wrong
} usercmd_t;
#endif
#endif // USERCMD_H

