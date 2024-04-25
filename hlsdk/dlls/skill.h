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
//=========================================================
// skill.h - skill level concerns
//=========================================================

struct skilldata_t
{

	int iSkillLevel; // game skill level

// Character Health & Damage
	float attackcopterHealth;
	float attackcopterShootDmg;

	float helicopterHealth;
	float helicopterGuardHealth;
	
	float ninjaHealth;
	float ninjaShootDmg;
	float ninjaSlashDmg;

	float osatovanHealth;
	float osatovanDmg;

	float enemyHealth;
	float enemyKickDmg;
	float enemyGspeed;
	float enemyRocketDmg;

	float npcDumbHealth;

	float npcHealth;

	float turretHealth;
	float miniturretHealth;
	float sentryHealth;
	float turretDmg;

	float scubasteveShakeDmg;
	float scubasteveHealth;

	float rookHealth;

	float drakeHealth;
	float drakeSlashDmg;

// Player Weapons
	float plr_fist;
	float plr_pp9_bullet;
	float plr_kowloon_bullet;
	float plr_raptor_bullet;
	float plr_commando_bullet;
	float plr_minigun_bullet;
	float plr_pdw90_bullet;
	float plr_mp9_bullet;
	float plr_buckshot_bullet;
	float plr_sniper_bullet;
	float plr_hand_grenade;
	float plr_tripmine;
	float plr_rocket;
	float plr_ronin_charge;
	float plr_ronin_shoot;
	float plr_ronin_health;
	float plr_taser;
	float plr_laser;
	float plr_eigrenade;
	
// weapons shared by monsters
	float pp9_bullet;
	float kowloon_bullet;
	float commando_bullet;
	float mp9_bullet;
	float pdw90_bullet;
	float buckshot_bullet;
	float sniper_bullet;
	float alerted_bullet;
	float raptor_bullet;
	float minigun_bullet;
	float laser_bolt;
	float ei_grenade;

// health/suit charge
	float suitchargerCapacity;
	float batteryCapacity;
	float healthchargerCapacity;
	float healthkitCapacity;

// character damage adj
	float characterHead;
	float characterChest;
	float characterStomach;
	float characterLeg;
	float characterArm;

// player damage adj
	float plrHead;
	float plrChest;
	float plrStomach;
	float plrLeg;
	float plrArm;
};

extern	DLL_GLOBAL	skilldata_t	gSkillData;
float GetSkillCvar( const char *pName );

extern DLL_GLOBAL int		g_iSkillLevel;

#define SKILL_EASY		1
#define SKILL_MEDIUM	2
#define SKILL_HARD		3
