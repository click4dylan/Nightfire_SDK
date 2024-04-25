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
#include "extdll.h"
#include "eiface.h"
#include "util.h"
#include "game.h"

ConsoleVariable* gMissionName = nullptr;
ConsoleVariable* displaysoundlist = nullptr;
ConsoleVariable* mp_teamplay = nullptr;
ConsoleVariable* mp_fraglimit = nullptr;
ConsoleVariable* mp_fragsleft = nullptr;
ConsoleVariable* mp_timelimit = nullptr;
ConsoleVariable* mp_timeleft = nullptr;
ConsoleVariable* mp_friendlyfire = nullptr;
ConsoleVariable* mp_falldamage = nullptr;
ConsoleVariable* mp_weaponstay = nullptr;
ConsoleVariable* ctf_capture = nullptr;
ConsoleVariable* mp_forcerespawn = nullptr;
ConsoleVariable* mp_flashlight = nullptr;
ConsoleVariable* mp_teamlist = nullptr;
ConsoleVariable* mp_teamoverride = nullptr;
ConsoleVariable* mp_defaultteam = nullptr;
ConsoleVariable* mp_allowcharacters = nullptr;
ConsoleVariable* mp_chattime = nullptr;
ConsoleVariable* mp_autoaim = nullptr;
ConsoleVariable* mp_multipower = nullptr;
ConsoleVariable* mp_spamdelay = nullptr;
ConsoleVariable* ctf_autoteam = nullptr;
ConsoleVariable* sv_waypoint = nullptr;
ConsoleVariable* sp_playerbody = nullptr;
ConsoleVariable* sp_nodefloatout = nullptr;
ConsoleVariable* sp_showaistate = nullptr;
ConsoleVariable* sp_showcamlaser = nullptr;
ConsoleVariable* showtriggers = nullptr;

ConsoleVariable* sk_agrunt_health1 = nullptr;
ConsoleVariable* sk_agrunt_health2 = nullptr;
ConsoleVariable* sk_agrunt_health3 = nullptr;

ConsoleVariable* sk_agrunt_dmg_punch1 = nullptr;
ConsoleVariable* sk_agrunt_dmg_punch2 = nullptr;
ConsoleVariable* sk_agrunt_dmg_punch3 = nullptr;

ConsoleVariable* sk_attackcopter_health1 = nullptr;
ConsoleVariable* sk_attackcopter_health2 = nullptr;
ConsoleVariable* sk_attackcopter_health3 = nullptr;

ConsoleVariable* sk_attackcopter_shoot1 = nullptr;
ConsoleVariable* sk_attackcopter_shoot2 = nullptr;
ConsoleVariable* sk_attackcopter_shoot3 = nullptr;

ConsoleVariable* sk_helicopter_health1 = nullptr;
ConsoleVariable* sk_helicopter_health2 = nullptr;
ConsoleVariable* sk_helicopter_health3 = nullptr;

ConsoleVariable* sk_helicopter_guard_health1 = nullptr;
ConsoleVariable* sk_helicopter_guard_health2 = nullptr;
ConsoleVariable* sk_helicopter_guard_health3 = nullptr;

ConsoleVariable* sk_ninja_health1 = nullptr;
ConsoleVariable* sk_ninja_health2 = nullptr;
ConsoleVariable* sk_ninja_health3 = nullptr;

ConsoleVariable* sk_ninja_shoot1 = nullptr;
ConsoleVariable* sk_ninja_shoot2 = nullptr;
ConsoleVariable* sk_ninja_shoot3 = nullptr;

ConsoleVariable* sk_ninja_slash1 = nullptr;
ConsoleVariable* sk_ninja_slash2 = nullptr;
ConsoleVariable* sk_ninja_slash3 = nullptr;

ConsoleVariable* sk_rook_health1 = nullptr;
ConsoleVariable* sk_rook_health2 = nullptr;
ConsoleVariable* sk_rook_health3 = nullptr;

ConsoleVariable* sk_drake_health1 = nullptr;
ConsoleVariable* sk_drake_health2 = nullptr;
ConsoleVariable* sk_drake_health3 = nullptr;

ConsoleVariable* sk_drake_slash1 = nullptr;
ConsoleVariable* sk_drake_slash2 = nullptr;
ConsoleVariable* sk_drake_slash3 = nullptr;

ConsoleVariable* sk_osatovan_health1 = nullptr;
ConsoleVariable* sk_osatovan_health2 = nullptr;
ConsoleVariable* sk_osatovan_health3 = nullptr;

ConsoleVariable* sk_osatovan_shoot1 = nullptr;
ConsoleVariable* sk_osatovan_shoot2 = nullptr;
ConsoleVariable* sk_osatovan_shoot3 = nullptr;

ConsoleVariable* sk_enemy_health1 = nullptr;
ConsoleVariable* sk_enemy_health2 = nullptr;
ConsoleVariable* sk_enemy_health3 = nullptr;

ConsoleVariable* sk_enemy_kick1 = nullptr;
ConsoleVariable* sk_enemy_kick2 = nullptr;
ConsoleVariable* sk_enemy_kick3 = nullptr;

ConsoleVariable* sk_enemy_pellets1 = nullptr;
ConsoleVariable* sk_enemy_pellets2 = nullptr;
ConsoleVariable* sk_enemy_pellets3 = nullptr;

ConsoleVariable* sk_enemy_gspeed1 = nullptr;
ConsoleVariable* sk_enemy_gspeed2 = nullptr;
ConsoleVariable* sk_enemy_gspeed3 = nullptr;

ConsoleVariable* sk_enemy_rocket1 = nullptr;
ConsoleVariable* sk_enemy_rocket2 = nullptr;
ConsoleVariable* sk_enemy_rocket3 = nullptr;

ConsoleVariable* sk_npc_dumb_health1 = nullptr;
ConsoleVariable* sk_npc_dumb_health2 = nullptr;
ConsoleVariable* sk_npc_dumb_health3 = nullptr;

ConsoleVariable* sk_npc_health1 = nullptr;
ConsoleVariable* sk_npc_health2 = nullptr;
ConsoleVariable* sk_npc_health3 = nullptr;

ConsoleVariable* sk_turret_health1 = nullptr;
ConsoleVariable* sk_turret_health2 = nullptr;
ConsoleVariable* sk_turret_health3 = nullptr;

ConsoleVariable* sk_miniturret_health1 = nullptr;
ConsoleVariable* sk_miniturret_health2 = nullptr;
ConsoleVariable* sk_miniturret_health3 = nullptr;

ConsoleVariable* sk_sentry_health1 = nullptr;
ConsoleVariable* sk_sentry_health2 = nullptr;
ConsoleVariable* sk_sentry_health3 = nullptr;

ConsoleVariable* sk_turret_damage1 = nullptr;
ConsoleVariable* sk_turret_damage2 = nullptr;
ConsoleVariable* sk_turret_damage3 = nullptr;


ConsoleVariable* sk_scubasteve_dmg_shake1 = nullptr;
ConsoleVariable* sk_scubasteve_dmg_shake2 = nullptr;
ConsoleVariable* sk_scubasteve_dmg_shake3 = nullptr;

ConsoleVariable* sk_scubasteve_health1 = nullptr;
ConsoleVariable* sk_scubasteve_health2 = nullptr;
ConsoleVariable* sk_scubasteve_health3 = nullptr;

ConsoleVariable* sk_plr_fist1 = nullptr;
ConsoleVariable* sk_plr_fist2 = nullptr;
ConsoleVariable* sk_plr_fist3 = nullptr;

ConsoleVariable* sk_plr_pp9_bullet1 = nullptr;
ConsoleVariable* sk_plr_pp9_bullet2 = nullptr;
ConsoleVariable* sk_plr_pp9_bullet3 = nullptr;

ConsoleVariable* sk_plr_kowloon_bullet1 = nullptr;
ConsoleVariable* sk_plr_kowloon_bullet2 = nullptr;
ConsoleVariable* sk_plr_kowloon_bullet3 = nullptr;

ConsoleVariable* sk_plr_raptor_bullet1 = nullptr;
ConsoleVariable* sk_plr_raptor_bullet2 = nullptr;
ConsoleVariable* sk_plr_raptor_bullet3 = nullptr;

ConsoleVariable* sk_plr_commando_bullet1 = nullptr;
ConsoleVariable* sk_plr_commando_bullet2 = nullptr;
ConsoleVariable* sk_plr_commando_bullet3 = nullptr;

ConsoleVariable* sk_plr_minigun_bullet1 = nullptr;
ConsoleVariable* sk_plr_minigun_bullet2 = nullptr;
ConsoleVariable* sk_plr_minigun_bullet3 = nullptr;

ConsoleVariable* sk_plr_pdw90_bullet1 = nullptr;
ConsoleVariable* sk_plr_pdw90_bullet2 = nullptr;
ConsoleVariable* sk_plr_pdw90_bullet3 = nullptr;

ConsoleVariable* sk_plr_mp9_bullet1 = nullptr;
ConsoleVariable* sk_plr_mp9_bullet2 = nullptr;
ConsoleVariable* sk_plr_mp9_bullet3 = nullptr;

ConsoleVariable* sk_plr_buckshot_bullet1 = nullptr;
ConsoleVariable* sk_plr_buckshot_bullet2 = nullptr;
ConsoleVariable* sk_plr_buckshot_bullet3 = nullptr;

ConsoleVariable* sk_plr_sniper_bullet1 = nullptr;
ConsoleVariable* sk_plr_sniper_bullet2 = nullptr;
ConsoleVariable* sk_plr_sniper_bullet3 = nullptr;

ConsoleVariable* sk_plr_hand_grenade1 = nullptr;
ConsoleVariable* sk_plr_hand_grenade2 = nullptr;
ConsoleVariable* sk_plr_hand_grenade3 = nullptr;

ConsoleVariable* sk_plr_tripmine1 = nullptr;
ConsoleVariable* sk_plr_tripmine2 = nullptr;
ConsoleVariable* sk_plr_tripmine3 = nullptr;

ConsoleVariable* sk_plr_rocket1 = nullptr;
ConsoleVariable* sk_plr_rocket2 = nullptr;
ConsoleVariable* sk_plr_rocket3 = nullptr;

ConsoleVariable* sk_plr_ronin_charge1 = nullptr;
ConsoleVariable* sk_plr_ronin_charge2 = nullptr;
ConsoleVariable* sk_plr_ronin_charge3 = nullptr;

ConsoleVariable* sk_plr_ronin_shoot1 = nullptr;
ConsoleVariable* sk_plr_ronin_shoot2 = nullptr;
ConsoleVariable* sk_plr_ronin_shoot3 = nullptr;

ConsoleVariable* sk_plr_ronin_health1 = nullptr;
ConsoleVariable* sk_plr_ronin_health2 = nullptr;
ConsoleVariable* sk_plr_ronin_health3 = nullptr;

ConsoleVariable* sk_plr_taser1 = nullptr;
ConsoleVariable* sk_plr_taser2 = nullptr;
ConsoleVariable* sk_plr_taser3 = nullptr;

ConsoleVariable* sk_plr_laser1 = nullptr;
ConsoleVariable* sk_plr_laser2 = nullptr;
ConsoleVariable* sk_plr_laser3 = nullptr;

ConsoleVariable* sk_plr_eigrenade1 = nullptr;
ConsoleVariable* sk_plr_eigrenade2 = nullptr;
ConsoleVariable* sk_plr_eigrenade3 = nullptr;

ConsoleVariable* sk_commando_bullet1 = nullptr;
ConsoleVariable* sk_commando_bullet2 = nullptr;
ConsoleVariable* sk_commando_bullet3 = nullptr;

ConsoleVariable* sk_mp9_bullet1 = nullptr;
ConsoleVariable* sk_mp9_bullet2 = nullptr;
ConsoleVariable* sk_mp9_bullet3 = nullptr;

ConsoleVariable* sk_kowloon_bullet1 = nullptr;
ConsoleVariable* sk_kowloon_bullet2 = nullptr;
ConsoleVariable* sk_kowloon_bullet3 = nullptr;

ConsoleVariable* sk_pdw90_bullet1 = nullptr;
ConsoleVariable* sk_pdw90_bullet2 = nullptr;
ConsoleVariable* sk_pdw90_bullet3 = nullptr;

ConsoleVariable* sk_buckshot_bullet1 = nullptr;
ConsoleVariable* sk_buckshot_bullet2 = nullptr;
ConsoleVariable* sk_buckshot_bullet3 = nullptr;

ConsoleVariable* sk_sniper_bullet1 = nullptr;
ConsoleVariable* sk_sniper_bullet2 = nullptr;
ConsoleVariable* sk_sniper_bullet3 = nullptr;

ConsoleVariable* sk_alerted_bullet1 = nullptr;
ConsoleVariable* sk_alerted_bullet2 = nullptr;
ConsoleVariable* sk_alerted_bullet3 = nullptr;

ConsoleVariable* sk_raptor_bullet1 = nullptr;
ConsoleVariable* sk_raptor_bullet2 = nullptr;
ConsoleVariable* sk_raptor_bullet3 = nullptr;

ConsoleVariable* sk_minigun_bullet1 = nullptr;
ConsoleVariable* sk_minigun_bullet2 = nullptr;
ConsoleVariable* sk_minigun_bullet3 = nullptr;

ConsoleVariable* sk_laser_bolt1 = nullptr;
ConsoleVariable* sk_laser_bolt2 = nullptr;
ConsoleVariable* sk_laser_bolt3 = nullptr;

ConsoleVariable* sk_ei_grenade1 = nullptr;
ConsoleVariable* sk_ei_grenade2 = nullptr;
ConsoleVariable* sk_ei_grenade3 = nullptr;

ConsoleVariable* sk_suitcharger1 = nullptr;
ConsoleVariable* sk_suitcharger2 = nullptr;
ConsoleVariable* sk_suitcharger3 = nullptr;

ConsoleVariable* sk_battery1 = nullptr;
ConsoleVariable* sk_battery2 = nullptr;
ConsoleVariable* sk_battery3 = nullptr;

ConsoleVariable* sk_healthcarger1 = nullptr;
ConsoleVariable* sk_healthcarger2 = nullptr;
ConsoleVariable* sk_healthcharger3 = nullptr;

ConsoleVariable* sk_healthkit1 = nullptr;
ConsoleVariable* sk_healthkit2 = nullptr;
ConsoleVariable* sk_healthkit3 = nullptr;

ConsoleVariable* sk_character_head1 = nullptr;
ConsoleVariable* sk_character_head2 = nullptr;
ConsoleVariable* sk_character_head3 = nullptr;

ConsoleVariable* sk_character_chest1 = nullptr;
ConsoleVariable* sk_character_chest2 = nullptr;
ConsoleVariable* sk_character_chest3 = nullptr;

ConsoleVariable* sk_character_stomach1 = nullptr;
ConsoleVariable* sk_character_stomach2 = nullptr;
ConsoleVariable* sk_character_stomach3 = nullptr;

ConsoleVariable* sk_character_arm1 = nullptr;
ConsoleVariable* sk_character_arm2 = nullptr;
ConsoleVariable* sk_character_arm3 = nullptr;

ConsoleVariable* sk_character_leg1 = nullptr;
ConsoleVariable* sk_character_leg2 = nullptr;
ConsoleVariable* sk_character_leg3 = nullptr;

ConsoleVariable* sk_player_head1 = nullptr;
ConsoleVariable* sk_player_head2 = nullptr;
ConsoleVariable* sk_player_head3 = nullptr;

ConsoleVariable* sk_player_chest1 = nullptr;
ConsoleVariable* sk_player_chest2 = nullptr;
ConsoleVariable* sk_player_chest3 = nullptr;

ConsoleVariable* sk_player_stomach1 = nullptr;
ConsoleVariable* sk_player_stomach2 = nullptr;
ConsoleVariable* sk_player_stomach3 = nullptr;

ConsoleVariable* sk_player_arm1 = nullptr;
ConsoleVariable* sk_player_arm2 = nullptr;
ConsoleVariable* sk_player_arm3 = nullptr;

ConsoleVariable* sk_player_leg1 = nullptr;
ConsoleVariable* sk_player_leg2 = nullptr;
ConsoleVariable* sk_player_leg3 = nullptr;

ConsoleVariable* g_psv_gravity = nullptr;
ConsoleVariable* g_psv_footsteps = nullptr;

#if 0
IConsoleVariable	displaysoundlist = {"displaysoundlist","0"};

// multiplayer server rules
IConsoleVariable	fragsleft	= { CVAR_INT, "mp_fragsleft", "", "0", FCVAR_SERVER | FCVAR_UNLOGGED };	  // Don't spam console/log files/users with this changing
IConsoleVariable	timeleft	= { CVAR_INT, "mp_timeleft", "","0" , FCVAR_SERVER | FCVAR_UNLOGGED };	  // "      "

// multiplayer server rules
IConsoleVariable	teamplay	= {"mp_teamplay","0", FCVAR_SERVER };
ConsoleVariable	*fraglimit	= {"mp_fraglimit","0", FCVAR_SERVER };
IConsoleVariable	timelimit	= {CVAR_INT, "mp_timelimit","0", FCVAR_SERVER };
IConsoleVariable	friendlyfire= {"mp_friendlyfire","0", FCVAR_SERVER };
IConsoleVariable	falldamage	= {"mp_falldamage","0", FCVAR_SERVER };
IConsoleVariable	weaponstay	= {"mp_weaponstay","0", FCVAR_SERVER };
IConsoleVariable	forcerespawn= {"mp_forcerespawn","1", FCVAR_SERVER };
IConsoleVariable	flashlight	= {"mp_flashlight","0", FCVAR_SERVER };
IConsoleVariable	aimcrosshair= {"mp_autocrosshair","1", FCVAR_SERVER };
IConsoleVariable	decalfrequency = {"decalfrequency","30", FCVAR_SERVER };
IConsoleVariable	teamlist = {"mp_teamlist","hgrunt;scientist", FCVAR_SERVER };
IConsoleVariable	teamoverride = {"mp_teamoverride","1" };
IConsoleVariable	defaultteam = {"mp_defaultteam","0" };
IConsoleVariable	allowcharacters={"mp_allowmonsters","0", FCVAR_SERVER };

IConsoleVariable  allow_spectators = { "allow_spectators", "0.0", FCVAR_SERVER };		// 0 prevents players from being spectators

IConsoleVariable  mp_chattime = {"mp_chattime","10", FCVAR_SERVER };

// Engine Cvars
IConsoleVariable 	*g_psv_gravity = NULL;
IConsoleVariable	*g_psv_aim = NULL;
IConsoleVariable	*g_psv_footsteps = NULL;

//CVARS FOR SKILL LEVEL SETTINGS
// Agrunt
IConsoleVariable	sk_agrunt_health1 = {"sk_agrunt_health1","0"};
IConsoleVariable	sk_agrunt_health2 = {"sk_agrunt_health2","0"};
IConsoleVariable	sk_agrunt_health3 = {"sk_agrunt_health3","0"};

IConsoleVariable	sk_agrunt_dmg_punch1 = {"sk_agrunt_dmg_punch1","0"};
IConsoleVariable	sk_agrunt_dmg_punch2 = {"sk_agrunt_dmg_punch2","0"};
IConsoleVariable	sk_agrunt_dmg_punch3 = {"sk_agrunt_dmg_punch3","0"};

// Apache
IConsoleVariable	sk_attackcopter_health1	= {"sk_attackcopter_health1","0"};
IConsoleVariable	sk_attackcopter_health2	= {"sk_attackcopter_health2","0"};
IConsoleVariable	sk_attackcopter_health3	= {"sk_attackcopter_health3","0"};

// Barney
IConsoleVariable	sk_barney_health1	= {"sk_barney_health1","0"};
IConsoleVariable	sk_barney_health2	= {"sk_barney_health2","0"};
IConsoleVariable	sk_barney_health3	= {"sk_barney_health3","0"};

// Bullsquid
IConsoleVariable	sk_bullsquid_health1 = {"sk_bullsquid_health1","0"};
IConsoleVariable	sk_bullsquid_health2 = {"sk_bullsquid_health2","0"};
IConsoleVariable	sk_bullsquid_health3 = {"sk_bullsquid_health3","0"};

IConsoleVariable	sk_bullsquid_dmg_bite1 = {"sk_bullsquid_dmg_bite1","0"};
IConsoleVariable	sk_bullsquid_dmg_bite2 = {"sk_bullsquid_dmg_bite2","0"};
IConsoleVariable	sk_bullsquid_dmg_bite3 = {"sk_bullsquid_dmg_bite3","0"};

IConsoleVariable	sk_bullsquid_dmg_whip1 = {"sk_bullsquid_dmg_whip1","0"};
IConsoleVariable	sk_bullsquid_dmg_whip2 = {"sk_bullsquid_dmg_whip2","0"};
IConsoleVariable	sk_bullsquid_dmg_whip3 = {"sk_bullsquid_dmg_whip3","0"};

IConsoleVariable	sk_bullsquid_dmg_spit1 = {"sk_bullsquid_dmg_spit1","0"};
IConsoleVariable	sk_bullsquid_dmg_spit2 = {"sk_bullsquid_dmg_spit2","0"};
IConsoleVariable	sk_bullsquid_dmg_spit3 = {"sk_bullsquid_dmg_spit3","0"};


// Big Momma
IConsoleVariable	sk_bigmomma_health_factor1 = {"sk_bigmomma_health_factor1","1.0"};
IConsoleVariable	sk_bigmomma_health_factor2 = {"sk_bigmomma_health_factor2","1.0"};
IConsoleVariable	sk_bigmomma_health_factor3 = {"sk_bigmomma_health_factor3","1.0"};

IConsoleVariable	sk_bigmomma_dmg_slash1 = {"sk_bigmomma_dmg_slash1","50"};
IConsoleVariable	sk_bigmomma_dmg_slash2 = {"sk_bigmomma_dmg_slash2","50"};
IConsoleVariable	sk_bigmomma_dmg_slash3 = {"sk_bigmomma_dmg_slash3","50"};

IConsoleVariable	sk_bigmomma_dmg_blast1 = {"sk_bigmomma_dmg_blast1","100"};
IConsoleVariable	sk_bigmomma_dmg_blast2 = {"sk_bigmomma_dmg_blast2","100"};
IConsoleVariable	sk_bigmomma_dmg_blast3 = {"sk_bigmomma_dmg_blast3","100"};

IConsoleVariable	sk_bigmomma_radius_blast1 = {"sk_bigmomma_radius_blast1","250"};
IConsoleVariable	sk_bigmomma_radius_blast2 = {"sk_bigmomma_radius_blast2","250"};
IConsoleVariable	sk_bigmomma_radius_blast3 = {"sk_bigmomma_radius_blast3","250"};

// Gargantua
IConsoleVariable	sk_gargantua_health1 = {"sk_gargantua_health1","0"};
IConsoleVariable	sk_gargantua_health2 = {"sk_gargantua_health2","0"};
IConsoleVariable	sk_gargantua_health3 = {"sk_gargantua_health3","0"};

IConsoleVariable	sk_gargantua_dmg_slash1	= {"sk_gargantua_dmg_slash1","0"};
IConsoleVariable	sk_gargantua_dmg_slash2	= {"sk_gargantua_dmg_slash2","0"};
IConsoleVariable	sk_gargantua_dmg_slash3	= {"sk_gargantua_dmg_slash3","0"};

IConsoleVariable	sk_gargantua_dmg_fire1 = {"sk_gargantua_dmg_fire1","0"};
IConsoleVariable	sk_gargantua_dmg_fire2 = {"sk_gargantua_dmg_fire2","0"};
IConsoleVariable	sk_gargantua_dmg_fire3 = {"sk_gargantua_dmg_fire3","0"};

IConsoleVariable	sk_gargantua_dmg_stomp1	= {"sk_gargantua_dmg_stomp1","0"};
IConsoleVariable	sk_gargantua_dmg_stomp2	= {"sk_gargantua_dmg_stomp2","0"};
IConsoleVariable	sk_gargantua_dmg_stomp3	= {"sk_gargantua_dmg_stomp3","0"};


// Hassassin
IConsoleVariable	sk_hassassin_health1 = {"sk_hassassin_health1","0"};
IConsoleVariable	sk_hassassin_health2 = {"sk_hassassin_health2","0"};
IConsoleVariable	sk_hassassin_health3 = {"sk_hassassin_health3","0"};


// Headcrab
IConsoleVariable	sk_headcrab_health1 = {"sk_headcrab_health1","0"};
IConsoleVariable	sk_headcrab_health2 = {"sk_headcrab_health2","0"};
IConsoleVariable	sk_headcrab_health3 = {"sk_headcrab_health3","0"};

IConsoleVariable	sk_headcrab_dmg_bite1 = {"sk_headcrab_dmg_bite1","0"};
IConsoleVariable	sk_headcrab_dmg_bite2 = {"sk_headcrab_dmg_bite2","0"};
IConsoleVariable	sk_headcrab_dmg_bite3 = {"sk_headcrab_dmg_bite3","0"};


// Hgrunt 
IConsoleVariable	sk_hgrunt_health1 = {"sk_hgrunt_health1","0"};
IConsoleVariable	sk_hgrunt_health2 = {"sk_hgrunt_health2","0"};
IConsoleVariable	sk_hgrunt_health3 = {"sk_hgrunt_health3","0"};

IConsoleVariable	sk_hgrunt_kick1 = {"sk_hgrunt_kick1","0"};
IConsoleVariable	sk_hgrunt_kick2 = {"sk_hgrunt_kick2","0"};
IConsoleVariable	sk_hgrunt_kick3 = {"sk_hgrunt_kick3","0"};

IConsoleVariable	sk_hgrunt_pellets1 = {"sk_hgrunt_pellets1","0"};
IConsoleVariable	sk_hgrunt_pellets2 = {"sk_hgrunt_pellets2","0"};
IConsoleVariable	sk_hgrunt_pellets3 = {"sk_hgrunt_pellets3","0"};

IConsoleVariable	sk_hgrunt_gspeed1 = {"sk_hgrunt_gspeed1","0"};
IConsoleVariable	sk_hgrunt_gspeed2 = {"sk_hgrunt_gspeed2","0"};
IConsoleVariable	sk_hgrunt_gspeed3 = {"sk_hgrunt_gspeed3","0"};

// Houndeye
IConsoleVariable	sk_houndeye_health1 = {"sk_houndeye_health1","0"};
IConsoleVariable	sk_houndeye_health2 = {"sk_houndeye_health2","0"};
IConsoleVariable	sk_houndeye_health3 = {"sk_houndeye_health3","0"};

IConsoleVariable	sk_houndeye_dmg_blast1 = {"sk_houndeye_dmg_blast1","0"};
IConsoleVariable	sk_houndeye_dmg_blast2 = {"sk_houndeye_dmg_blast2","0"};
IConsoleVariable	sk_houndeye_dmg_blast3 = {"sk_houndeye_dmg_blast3","0"};


// ISlave
IConsoleVariable	sk_islave_health1 = {"sk_islave_health1","0"};
IConsoleVariable	sk_islave_health2 = {"sk_islave_health2","0"};
IConsoleVariable	sk_islave_health3 = {"sk_islave_health3","0"};

IConsoleVariable	sk_islave_dmg_claw1 = {"sk_islave_dmg_claw1","0"};
IConsoleVariable	sk_islave_dmg_claw2 = {"sk_islave_dmg_claw2","0"};
IConsoleVariable	sk_islave_dmg_claw3 = {"sk_islave_dmg_claw3","0"};

IConsoleVariable	sk_islave_dmg_clawrake1	= {"sk_islave_dmg_clawrake1","0"};
IConsoleVariable	sk_islave_dmg_clawrake2	= {"sk_islave_dmg_clawrake2","0"};
IConsoleVariable	sk_islave_dmg_clawrake3	= {"sk_islave_dmg_clawrake3","0"};
	
IConsoleVariable	sk_islave_dmg_zap1 = {"sk_islave_dmg_zap1","0"};
IConsoleVariable	sk_islave_dmg_zap2 = {"sk_islave_dmg_zap2","0"};
IConsoleVariable	sk_islave_dmg_zap3 = {"sk_islave_dmg_zap3","0"};


// Icthyosaur
IConsoleVariable	sk_ichthyosaur_health1	= {"sk_ichthyosaur_health1","0"};
IConsoleVariable	sk_ichthyosaur_health2	= {"sk_ichthyosaur_health2","0"};
IConsoleVariable	sk_ichthyosaur_health3	= {"sk_ichthyosaur_health3","0"};

IConsoleVariable	sk_ichthyosaur_shake1	= {"sk_ichthyosaur_shake1","0"};
IConsoleVariable	sk_ichthyosaur_shake2	= {"sk_ichthyosaur_shake2","0"};
IConsoleVariable	sk_ichthyosaur_shake3	= {"sk_ichthyosaur_shake3","0"};


// Leech
IConsoleVariable	sk_leech_health1 = {"sk_leech_health1","0"};
IConsoleVariable	sk_leech_health2 = {"sk_leech_health2","0"};
IConsoleVariable	sk_leech_health3 = {"sk_leech_health3","0"};

IConsoleVariable	sk_leech_dmg_bite1 = {"sk_leech_dmg_bite1","0"};
IConsoleVariable	sk_leech_dmg_bite2 = {"sk_leech_dmg_bite2","0"};
IConsoleVariable	sk_leech_dmg_bite3 = {"sk_leech_dmg_bite3","0"};

// Controller
IConsoleVariable	sk_controller_health1 = {"sk_controller_health1","0"};
IConsoleVariable	sk_controller_health2 = {"sk_controller_health2","0"};
IConsoleVariable	sk_controller_health3 = {"sk_controller_health3","0"};

IConsoleVariable	sk_controller_dmgzap1 = {"sk_controller_dmgzap1","0"};
IConsoleVariable	sk_controller_dmgzap2 = {"sk_controller_dmgzap2","0"};
IConsoleVariable	sk_controller_dmgzap3 = {"sk_controller_dmgzap3","0"};

IConsoleVariable	sk_controller_speedball1 = {"sk_controller_speedball1","0"};
IConsoleVariable	sk_controller_speedball2 = {"sk_controller_speedball2","0"};
IConsoleVariable	sk_controller_speedball3 = {"sk_controller_speedball3","0"};

IConsoleVariable	sk_controller_dmgball1 = {"sk_controller_dmgball1","0"};
IConsoleVariable	sk_controller_dmgball2 = {"sk_controller_dmgball2","0"};
IConsoleVariable	sk_controller_dmgball3 = {"sk_controller_dmgball3","0"};

// Nihilanth
IConsoleVariable	sk_nihilanth_health1 = {"sk_nihilanth_health1","0"};
IConsoleVariable	sk_nihilanth_health2 = {"sk_nihilanth_health2","0"};
IConsoleVariable	sk_nihilanth_health3 = {"sk_nihilanth_health3","0"};

IConsoleVariable	sk_nihilanth_zap1 = {"sk_nihilanth_zap1","0"};
IConsoleVariable	sk_nihilanth_zap2 = {"sk_nihilanth_zap2","0"};
IConsoleVariable	sk_nihilanth_zap3 = {"sk_nihilanth_zap3","0"};

// Scientist
IConsoleVariable	sk_scientist_health1 = {"sk_scientist_health1","0"};
IConsoleVariable	sk_scientist_health2 = {"sk_scientist_health2","0"};
IConsoleVariable	sk_scientist_health3 = {"sk_scientist_health3","0"};


// Snark
IConsoleVariable	sk_snark_health1 = {"sk_snark_health1","0"};
IConsoleVariable	sk_snark_health2 = {"sk_snark_health2","0"};
IConsoleVariable	sk_snark_health3 = {"sk_snark_health3","0"};

IConsoleVariable	sk_snark_dmg_bite1 = {"sk_snark_dmg_bite1","0"};
IConsoleVariable	sk_snark_dmg_bite2 = {"sk_snark_dmg_bite2","0"};
IConsoleVariable	sk_snark_dmg_bite3 = {"sk_snark_dmg_bite3","0"};

IConsoleVariable	sk_snark_dmg_pop1 = {"sk_snark_dmg_pop1","0"};
IConsoleVariable	sk_snark_dmg_pop2 = {"sk_snark_dmg_pop2","0"};
IConsoleVariable	sk_snark_dmg_pop3 = {"sk_snark_dmg_pop3","0"};



// Zombie
IConsoleVariable	sk_zombie_health1 = {"sk_zombie_health1","0"};
IConsoleVariable	sk_zombie_health2 = {"sk_zombie_health2","0"};
IConsoleVariable	sk_zombie_health3 = {"sk_zombie_health3","0"};

IConsoleVariable	sk_zombie_dmg_one_slash1 = {"sk_zombie_dmg_one_slash1","0"};
IConsoleVariable	sk_zombie_dmg_one_slash2 = {"sk_zombie_dmg_one_slash2","0"};
IConsoleVariable	sk_zombie_dmg_one_slash3 = {"sk_zombie_dmg_one_slash3","0"};

IConsoleVariable	sk_zombie_dmg_both_slash1 = {"sk_zombie_dmg_both_slash1","0"};
IConsoleVariable	sk_zombie_dmg_both_slash2 = {"sk_zombie_dmg_both_slash2","0"};
IConsoleVariable	sk_zombie_dmg_both_slash3 = {"sk_zombie_dmg_both_slash3","0"};


//Turret
IConsoleVariable	sk_turret_health1 = {"sk_turret_health1","0"};
IConsoleVariable	sk_turret_health2 = {"sk_turret_health2","0"};
IConsoleVariable	sk_turret_health3 = {"sk_turret_health3","0"};


// MiniTurret
IConsoleVariable	sk_miniturret_health1 = {"sk_miniturret_health1","0"};
IConsoleVariable	sk_miniturret_health2 = {"sk_miniturret_health2","0"};
IConsoleVariable	sk_miniturret_health3 = {"sk_miniturret_health3","0"};


// Sentry Turret
IConsoleVariable	sk_sentry_health1 = {"sk_sentry_health1","0"};
IConsoleVariable	sk_sentry_health2 = {"sk_sentry_health2","0"};
IConsoleVariable	sk_sentry_health3 = {"sk_sentry_health3","0"};


// PLAYER WEAPONS

// Crowbar whack
IConsoleVariable	sk_plr_crowbar1 = {"sk_plr_crowbar1","0"};
IConsoleVariable	sk_plr_crowbar2 = {"sk_plr_crowbar2","0"};
IConsoleVariable	sk_plr_crowbar3 = {"sk_plr_crowbar3","0"};

// Glock Round
IConsoleVariable	sk_plr_9mm_bullet1 = {"sk_plr_9mm_bullet1","0"};
IConsoleVariable	sk_plr_9mm_bullet2 = {"sk_plr_9mm_bullet2","0"};
IConsoleVariable	sk_plr_9mm_bullet3 = {"sk_plr_9mm_bullet3","0"};

// 357 Round
IConsoleVariable	sk_plr_357_bullet1 = {"sk_plr_357_bullet1","0"};
IConsoleVariable	sk_plr_357_bullet2 = {"sk_plr_357_bullet2","0"};
IConsoleVariable	sk_plr_357_bullet3 = {"sk_plr_357_bullet3","0"};

// MP5 Round
IConsoleVariable	sk_plr_9mmAR_bullet1 = {"sk_plr_9mmAR_bullet1","0"};
IConsoleVariable	sk_plr_9mmAR_bullet2 = {"sk_plr_9mmAR_bullet2","0"};
IConsoleVariable	sk_plr_9mmAR_bullet3 = {"sk_plr_9mmAR_bullet3","0"};


// M203 grenade
IConsoleVariable	sk_plr_9mmAR_grenade1 = {"sk_plr_9mmAR_grenade1","0"};
IConsoleVariable	sk_plr_9mmAR_grenade2 = {"sk_plr_9mmAR_grenade2","0"};
IConsoleVariable	sk_plr_9mmAR_grenade3 = {"sk_plr_9mmAR_grenade3","0"};


// Shotgun buckshot
IConsoleVariable	sk_plr_buckshot1 = {"sk_plr_buckshot1","0"};
IConsoleVariable	sk_plr_buckshot2 = {"sk_plr_buckshot2","0"};
IConsoleVariable	sk_plr_buckshot3 = {"sk_plr_buckshot3","0"};


// Crossbow
IConsoleVariable	sk_plr_xbow_bolt_client1 = {"sk_plr_xbow_bolt_client1","0"};
IConsoleVariable	sk_plr_xbow_bolt_client2 = {"sk_plr_xbow_bolt_client2","0"};
IConsoleVariable	sk_plr_xbow_bolt_client3 = {"sk_plr_xbow_bolt_client3","0"};

IConsoleVariable	sk_plr_xbow_bolt_monster1 = {"sk_plr_xbow_bolt_monster1","0"};
IConsoleVariable	sk_plr_xbow_bolt_monster2 = {"sk_plr_xbow_bolt_monster2","0"};
IConsoleVariable	sk_plr_xbow_bolt_monster3 = {"sk_plr_xbow_bolt_monster3","0"};


// RPG
IConsoleVariable	sk_plr_rpg1 = {"sk_plr_rpg1","0"};
IConsoleVariable	sk_plr_rpg2 = {"sk_plr_rpg2","0"};
IConsoleVariable	sk_plr_rpg3 = {"sk_plr_rpg3","0"};


// Zero Point Generator
IConsoleVariable	sk_plr_gauss1 = {"sk_plr_gauss1","0"};
IConsoleVariable	sk_plr_gauss2 = {"sk_plr_gauss2","0"};
IConsoleVariable	sk_plr_gauss3 = {"sk_plr_gauss3","0"};


// Tau Cannon
IConsoleVariable	sk_plr_egon_narrow1 = {"sk_plr_egon_narrow1","0"};
IConsoleVariable	sk_plr_egon_narrow2 = {"sk_plr_egon_narrow2","0"};
IConsoleVariable	sk_plr_egon_narrow3 = {"sk_plr_egon_narrow3","0"};

IConsoleVariable	sk_plr_egon_wide1 = {"sk_plr_egon_wide1","0"};
IConsoleVariable	sk_plr_egon_wide2 = {"sk_plr_egon_wide2","0"};
IConsoleVariable	sk_plr_egon_wide3 = {"sk_plr_egon_wide3","0"};


// Hand Grendade
IConsoleVariable	sk_plr_hand_grenade1 = {"sk_plr_hand_grenade1","0"};
IConsoleVariable	sk_plr_hand_grenade2 = {"sk_plr_hand_grenade2","0"};
IConsoleVariable	sk_plr_hand_grenade3 = {"sk_plr_hand_grenade3","0"};


// Satchel Charge
IConsoleVariable	sk_plr_satchel1	= {"sk_plr_satchel1","0"};
IConsoleVariable	sk_plr_satchel2	= {"sk_plr_satchel2","0"};
IConsoleVariable	sk_plr_satchel3	= {"sk_plr_satchel3","0"};


// Tripmine
IConsoleVariable	sk_plr_tripmine1 = {"sk_plr_tripmine1","0"};
IConsoleVariable	sk_plr_tripmine2 = {"sk_plr_tripmine2","0"};
IConsoleVariable	sk_plr_tripmine3 = {"sk_plr_tripmine3","0"};


// WORLD WEAPONS
IConsoleVariable	sk_12mm_bullet1 = {"sk_12mm_bullet1","0"};
IConsoleVariable	sk_12mm_bullet2 = {"sk_12mm_bullet2","0"};
IConsoleVariable	sk_12mm_bullet3 = {"sk_12mm_bullet3","0"};

IConsoleVariable	sk_9mmAR_bullet1 = {"sk_9mmAR_bullet1","0"};
IConsoleVariable	sk_9mmAR_bullet2 = {"sk_9mmAR_bullet2","0"};
IConsoleVariable	sk_9mmAR_bullet3 = {"sk_9mmAR_bullet3","0"};

IConsoleVariable	sk_9mm_bullet1 = {"sk_9mm_bullet1","0"};
IConsoleVariable	sk_9mm_bullet2 = {"sk_9mm_bullet2","0"};
IConsoleVariable	sk_9mm_bullet3 = {"sk_9mm_bullet3","0"};


// HORNET
IConsoleVariable	sk_hornet_dmg1 = {"sk_hornet_dmg1","0"};
IConsoleVariable	sk_hornet_dmg2 = {"sk_hornet_dmg2","0"};
IConsoleVariable	sk_hornet_dmg3 = {"sk_hornet_dmg3","0"};

// HEALTH/CHARGE
IConsoleVariable	sk_suitcharger1	= { "sk_suitcharger1","0" };
IConsoleVariable	sk_suitcharger2	= { "sk_suitcharger2","0" };		
IConsoleVariable	sk_suitcharger3	= { "sk_suitcharger3","0" };		

IConsoleVariable	sk_battery1	= { "sk_battery1","0" };			
IConsoleVariable	sk_battery2	= { "sk_battery2","0" };			
IConsoleVariable	sk_battery3	= { "sk_battery3","0" };			

IConsoleVariable	sk_healthcharger1	= { "sk_healthcharger1","0" };		
IConsoleVariable	sk_healthcharger2	= { "sk_healthcharger2","0" };		
IConsoleVariable	sk_healthcharger3	= { "sk_healthcharger3","0" };		

IConsoleVariable	sk_healthkit1	= { "sk_healthkit1","0" };		
IConsoleVariable	sk_healthkit2	= { "sk_healthkit2","0" };		
IConsoleVariable	sk_healthkit3	= { "sk_healthkit3","0" };		

IConsoleVariable	sk_scientist_heal1	= { "sk_scientist_heal1","0" };	
IConsoleVariable	sk_scientist_heal2	= { "sk_scientist_heal2","0" };	
IConsoleVariable	sk_scientist_heal3	= { "sk_scientist_heal3","0" };	


// monster damage adjusters
IConsoleVariable	sk_monster_head1	= { "sk_monster_head1","2" };
IConsoleVariable	sk_monster_head2	= { "sk_monster_head2","2" };
IConsoleVariable	sk_monster_head3	= { "sk_monster_head3","2" };

IConsoleVariable	sk_monster_chest1	= { "sk_monster_chest1","1" };
IConsoleVariable	sk_monster_chest2	= { "sk_monster_chest2","1" };
IConsoleVariable	sk_monster_chest3	= { "sk_monster_chest3","1" };

IConsoleVariable	sk_monster_stomach1	= { "sk_monster_stomach1","1" };
IConsoleVariable	sk_monster_stomach2	= { "sk_monster_stomach2","1" };
IConsoleVariable	sk_monster_stomach3	= { "sk_monster_stomach3","1" };

IConsoleVariable	sk_monster_arm1	= { "sk_monster_arm1","1" };
IConsoleVariable	sk_monster_arm2	= { "sk_monster_arm2","1" };
IConsoleVariable	sk_monster_arm3	= { "sk_monster_arm3","1" };

IConsoleVariable	sk_monster_leg1	= { "sk_monster_leg1","1" };
IConsoleVariable	sk_monster_leg2	= { "sk_monster_leg2","1" };
IConsoleVariable	sk_monster_leg3	= { "sk_monster_leg3","1" };

// player damage adjusters
IConsoleVariable	sk_player_head1	= { "sk_player_head1","2" };
IConsoleVariable	sk_player_head2	= { "sk_player_head2","2" };
IConsoleVariable	sk_player_head3	= { "sk_player_head3","2" };

IConsoleVariable	sk_player_chest1	= { "sk_player_chest1","1" };
IConsoleVariable	sk_player_chest2	= { "sk_player_chest2","1" };
IConsoleVariable	sk_player_chest3	= { "sk_player_chest3","1" };

IConsoleVariable	sk_player_stomach1	= { "sk_player_stomach1","1" };
IConsoleVariable	sk_player_stomach2	= { "sk_player_stomach2","1" };
IConsoleVariable	sk_player_stomach3	= { "sk_player_stomach3","1" };

IConsoleVariable	sk_player_arm1	= { "sk_player_arm1","1" };
IConsoleVariable	sk_player_arm2	= { "sk_player_arm2","1" };
IConsoleVariable	sk_player_arm3	= { "sk_player_arm3","1" };

IConsoleVariable	sk_player_leg1	= { "sk_player_leg1","1" };
IConsoleVariable	sk_player_leg2	= { "sk_player_leg2","1" };
IConsoleVariable	sk_player_leg3	= { "sk_player_leg3","1" };
#endif

// END Cvars for Skill Level settings

// Register your console variables here
// This gets called one time when the game is initialied
void GameDLLInit( void )
{
	// Register cvars here:

	gMissionName = CVAR_REGISTER(IConsoleVariable(CVAR_STRING, "mission_name", "Name of the mission the current map belongs to.", "", 0);
	g_psv_gravity = CVAR_GET_POINTER( "sv_gravity" );
	g_psv_footsteps = CVAR_GET_POINTER( "sv_footsteps" );

	displaysoundlist = CVAR_REGISTER (IConsoleVariable(CVAR_BOOL, "displaysoundlist", "", "0", 0));
	
	mp_teamplay = CVAR_GET_POINTER("mp_teamplay");
	mp_fraglimit = CVAR_GET_POINTER("mp_fraglimit");
	mp_timelimit = CVAR_GET_POINTER("mp_timelimit");
	mp_friendlyfire = CVAR_GET_POINTER("mp_friendlyfire");
	mp_weaponstay = CVAR_GET_POINTER("mp_weaponstay");
	ctf_capture = CVAR_GET_POINTER("ctf_capture");

	mp_fragsleft = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "mp_fragsleft", "", "0", FCVAR_SERVER | FCVAR_UNLOGGED));
	mp_timeleft = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "mp_timeleft", "", "0", FCVAR_SERVER | FCVAR_UNLOGGED));

	mp_falldamage = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "mp_falldamage", "", "0", FCVAR_SERVER));
	mp_forcerespawn = CVAR_REGISTER(IConsoleVariable(CVAR_BOOL, "mp_forcerespawn", "", "0", FCVAR_SERVER));
	mp_flashlight = CVAR_REGISTER(IConsoleVariable(CVAR_BOOL, "mp_flashlight", "", "0", FCVAR_SERVER));
	mp_teamlist = CVAR_REGISTER(IConsoleVariable(CVAR_STRING, "mp_teamlist", "", "MI6;Phoenix", FCVAR_SERVER));
	mp_teamoverride = CVAR_REGISTER(IConsoleVariable(CVAR_BOOL, "mp_teamoverride", "", "1", 0));
	mp_defaultteam = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "mp_defaultteam", "", "0", 0));
	mp_allowcharacters = CVAR_REGISTER(IConsoleVariable(CVAR_BOOL, "mp_allowcharacters", "", "0", FCVAR_SERVER));
	mp_chattime = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "mp_chattime", "", "0", FCVAR_SERVER));
	mp_autoaim = CVAR_REGISTER(IConsoleVariable(CVAR_BOOL, "mp_autoaim", "", "0", FCVAR_SERVER));
	mp_multipower = CVAR_REGISTER(IConsoleVariable(CVAR_BOOL, "mp_multipower", "", "0", FCVAR_SERVER));
	mp_spamdelay = CVAR_REGISTER(IConsoleVariable(CVAR_FLOAT, "mp_multipower", "", "2.0", FCVAR_SERVER));
	ctf_autoteam = CVAR_REGISTER(IConsoleVariable(CVAR_BOOL, "ctf_autoteam", "", "0", FCVAR_SERVER));
	sv_waypoint = CVAR_REGISTER(IConsoleVariable(CVAR_BOOL, "sv_waypoint", "", "0", FCVAR_SERVER));
	sp_playerbody = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sp_playerbody", "", "0", FCVAR_SERVER));
	sp_nodefloatout = CVAR_REGISTER(IConsoleVariable(CVAR_BOOL, "sp_nodefloatout", "", "0", FCVAR_SERVER));
	sp_showaistate = CVAR_REGISTER(IConsoleVariable(CVAR_BOOL, "sp_showaistate", "", "0", FCVAR_SERVER));
	sp_showcamlaser = CVAR_REGISTER(IConsoleVariable(CVAR_BOOL, "sp_showcamlaser", "", "0", FCVAR_SERVER));

	showtriggers = CVAR_REGISTER(IConsoleVariable(CVAR_BOOL, "showtriggers", "Visible Show Triggers Dev Option", "0", FCVAR_SERVER | FCVAR_SPONLY));

	sk_agrunt_health1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_agrunt_health1", "", "0", 0));
	sk_agrunt_health2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_agrunt_health2", "", "0", 0));
	sk_agrunt_health3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_agrunt_health3", "", "0", 0));

	sk_agrunt_dmg_punch1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_agrunt_dmg_punch1", "", "0", 0));
	sk_agrunt_dmg_punch2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_agrunt_dmg_punch2", "", "0", 0));
	sk_agrunt_dmg_punch3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_agrunt_dmg_punch3", "", "0", 0));

	sk_attackcopter_health1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_attackcopter_health1", "", "0", 0));
	sk_attackcopter_health2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_attackcopter_health2", "", "0", 0));
	sk_attackcopter_health3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_attackcopter_health3", "", "0", 0));

	sk_attackcopter_shoot1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_attackcopter_shoot1", "", "0", 0));
	sk_attackcopter_shoot2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_attackcopter_shoot2", "", "0", 0));
	sk_attackcopter_shoot3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_attackcopter_shoot3", "", "0", 0));

	sk_helicopter_health1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_helicopter_health1", "", "0", 0));
	sk_helicopter_health2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_helicopter_health2", "", "0", 0));
	sk_helicopter_health3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_helicopter_health3", "", "0", 0));

	sk_helicopter_guard_health1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_helicopter_guard_health1", "", "0", 0));
	sk_helicopter_guard_health2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_helicopter_guard_health2", "", "0", 0));
	sk_helicopter_guard_health3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_helicopter_guard_health3", "", "0", 0));

	sk_ninja_health1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_ninja_health1", "", "0", 0));
	sk_ninja_health2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_ninja_health2", "", "0", 0));
	sk_ninja_health3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_ninja_health3", "", "0", 0));

	sk_ninja_shoot1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_ninja_shoot1", "", "0", 0));
	sk_ninja_shoot2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_ninja_shoot2", "", "0", 0));
	sk_ninja_shoot3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_ninja_shoot3", "", "0", 0));

	sk_ninja_slash1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_ninja_slash1", "", "0", 0));
	sk_ninja_slash2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_ninja_slash2", "", "0", 0));
	sk_ninja_slash3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_ninja_slash3", "", "0", 0));

	sk_rook_health1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_rook_health1", "", "0.0", 0));
	sk_rook_health2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_rook_health2", "", "0.0", 0));
	sk_rook_health3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_rook_health3", "", "0.0", 0));

	sk_drake_health1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_drake_health1", "", "0.0", 0));
	sk_drake_health2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_drake_health2", "", "0.0", 0));
	sk_drake_health3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_drake_health3", "", "0.0", 0));

	sk_drake_slash1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_drake_slash1", "", "0.0", 0));
	sk_drake_slash2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_drake_slash2", "", "0.0", 0));
	sk_drake_slash3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_drake_slash3", "", "0.0", 0));

	sk_osatovan_health1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_osatovan_health1", "", "0", 0));
	sk_osatovan_health2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_osatovan_health2", "", "0", 0));
	sk_osatovan_health3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_osatovan_health3", "", "0", 0));

	sk_osatovan_shoot1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_osatovan_shoot1", "", "0", 0));
	sk_osatovan_shoot2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_osatovan_shoot2", "", "0", 0));
	sk_osatovan_shoot3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_osatovan_shoot3", "", "0", 0));

	sk_enemy_health1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_enemy_health1", "", "0", 0));
	sk_enemy_health2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_enemy_health2", "", "0", 0));
	sk_enemy_health3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_enemy_health3", "", "0", 0));

	sk_enemy_kick1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_enemy_kick1", "", "0", 0));
	sk_enemy_kick2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_enemy_kick2", "", "0", 0));
	sk_enemy_kick3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_enemy_kick3", "", "0", 0));

	sk_enemy_pellets1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_enemy_pellets1", "", "0", 0));
	sk_enemy_pellets2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_enemy_pellets2", "", "0", 0));
	sk_enemy_pellets3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_enemy_pellets3", "", "0", 0));

	sk_enemy_gspeed1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_enemy_gspeed1", "", "0", 0));
	sk_enemy_gspeed2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_enemy_gspeed2", "", "0", 0));
	sk_enemy_gspeed3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_enemy_gspeed3", "", "0", 0));

	sk_enemy_rocket1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_enemy_rocket1", "", "0", 0));
	sk_enemy_rocket2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_enemy_rocket2", "", "0", 0));
	sk_enemy_rocket3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_enemy_rocket3", "", "0", 0));

	sk_npc_dumb_health1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_npc_dumb_health1", "", "0", 0));
	sk_npc_dumb_health2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_npc_dumb_health2", "", "0", 0));
	sk_npc_dumb_health3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_npc_dumb_health3", "", "0", 0));

	sk_npc_health1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_npc_health1", "", "0", 0));
	sk_npc_health2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_npc_health2", "", "0", 0));
	sk_npc_health3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_npc_health3", "", "0", 0));

	sk_turret_health1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_turret_health1", "", "0", 0));
	sk_turret_health2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_turret_health2", "", "0", 0));
	sk_turret_health3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_turret_health3", "", "0", 0));

	sk_miniturret_health1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_miniturret_health1", "", "0", 0));
	sk_miniturret_health2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_miniturret_health2", "", "0", 0));
	sk_miniturret_health3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_miniturret_health3", "", "0", 0));

	sk_sentry_health1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_sentry_health1", "", "0", 0));
	sk_sentry_health2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_sentry_health2", "", "0", 0));
	sk_sentry_health3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_sentry_health3", "", "0", 0));

	sk_turret_damage1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_turret_damage1", "", "0", 0));
	sk_turret_damage2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_turret_damage2", "", "0", 0));
	sk_turret_damage3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_turret_damage3", "", "0", 0));

	sk_scubasteve_dmg_shake1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_scubasteve_dmg_shake1", "", "0", 0));
	sk_scubasteve_dmg_shake2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_scubasteve_dmg_shake2", "", "0", 0));
	sk_scubasteve_dmg_shake3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_scubasteve_dmg_shake3", "", "0", 0));

	sk_scubasteve_health1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_scubasteve_health1", "", "0", 0));
	sk_scubasteve_health2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_scubasteve_health2", "", "0", 0));
	sk_scubasteve_health3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_scubasteve_health3", "", "0", 0));

	sk_plr_fist1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_fist1", "", "0", 0));
	sk_plr_fist2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_fist2", "", "0", 0));
	sk_plr_fist3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_fist3", "", "0", 0));

	sk_plr_pp9_bullet1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_pp9_bullet1", "", "0", 0));
	sk_plr_pp9_bullet2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_pp9_bullet2", "", "0", 0));
	sk_plr_pp9_bullet3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_pp9_bullet3", "", "0", 0));

	sk_plr_kowloon_bullet1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_kowloon_bullet1", "", "0", 0));
	sk_plr_kowloon_bullet2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_kowloon_bullet2", "", "0", 0));
	sk_plr_kowloon_bullet3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_kowloon_bullet3", "", "0", 0));

	sk_plr_raptor_bullet1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_raptor_bullet1", "", "0", 0));
	sk_plr_raptor_bullet2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_raptor_bullet2", "", "0", 0));
	sk_plr_raptor_bullet3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_raptor_bullet3", "", "0", 0));

	sk_plr_commando_bullet1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_commando_bullet1", "", "0", 0));
	sk_plr_commando_bullet2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_commando_bullet2", "", "0", 0));
	sk_plr_commando_bullet3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_commando_bullet3", "", "0", 0));

	sk_plr_minigun_bullet1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_minigun_bullet1", "", "0", 0));
	sk_plr_minigun_bullet2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_minigun_bullet2", "", "0", 0));
	sk_plr_minigun_bullet3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_minigun_bullet3", "", "0", 0));

	sk_plr_pdw90_bullet1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_pdw90_bullet1", "", "0", 0));
	sk_plr_pdw90_bullet2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_pdw90_bullet2", "", "0", 0));
	sk_plr_pdw90_bullet3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_pdw90_bullet3", "", "0", 0));

	sk_plr_mp9_bullet1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_mp9_bullet1", "", "0", 0));
	sk_plr_mp9_bullet2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_mp9_bullet2", "", "0", 0));
	sk_plr_mp9_bullet3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_mp9_bullet3", "", "0", 0));

	sk_plr_buckshot_bullet1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_buckshot_bullet1", "", "0", 0));
	sk_plr_buckshot_bullet2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_buckshot_bullet2", "", "0", 0));
	sk_plr_buckshot_bullet3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_buckshot_bullet3", "", "0", 0));

	sk_plr_sniper_bullet1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_sniper_bullet1", "", "0", 0));
	sk_plr_sniper_bullet2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_sniper_bullet2", "", "0", 0));
	sk_plr_sniper_bullet3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_sniper_bullet3", "", "0", 0));

	sk_plr_hand_grenade1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_hand_grenade1", "", "0", 0));
	sk_plr_hand_grenade2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_hand_grenade2", "", "0", 0));
	sk_plr_hand_grenade3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_hand_grenade3", "", "0", 0));

	sk_plr_tripmine1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_tripmine1", "", "0", 0));
	sk_plr_tripmine2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_tripmine2", "", "0", 0));
	sk_plr_tripmine3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_tripmine3", "", "0", 0));

	sk_plr_rocket1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_rocket1", "", "0", 0));
	sk_plr_rocket2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_rocket2", "", "0", 0));
	sk_plr_rocket3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_rocket3", "", "0", 0));

	sk_plr_ronin_charge1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_ronin_charge1", "", "0", 0));
	sk_plr_ronin_charge2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_ronin_charge2", "", "0", 0));
	sk_plr_ronin_charge3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_ronin_charge3", "", "0", 0));

	sk_plr_ronin_shoot1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_ronin_shoot1", "", "0", 0));
	sk_plr_ronin_shoot2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_ronin_shoot2", "", "0", 0));
	sk_plr_ronin_shoot3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_ronin_shoot3", "", "0", 0));

	sk_plr_ronin_health1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_ronin_health1", "", "0", 0));
	sk_plr_ronin_health2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_ronin_health2", "", "0", 0));
	sk_plr_ronin_health3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_ronin_health3", "", "0", 0));

	sk_plr_taser1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_taser1", "", "0", 0));
	sk_plr_taser2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_taser2", "", "0", 0));
	sk_plr_taser3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_taser3", "", "0", 0));

	sk_plr_laser1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_laser1", "", "0", 0));
	sk_plr_laser2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_laser2", "", "0", 0));
	sk_plr_laser3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_laser3", "", "0", 0));

	sk_plr_eigrenade1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_eigrenade1", "", "0", 0));
	sk_plr_eigrenade2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_eigrenade2", "", "0", 0));
	sk_plr_eigrenade3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_plr_eigrenade3", "", "0", 0));

	sk_commando_bullet1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_commando_bullet1", "", "0", 0));
	sk_commando_bullet2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_commando_bullet2", "", "0", 0));
	sk_commando_bullet3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_commando_bullet3", "", "0", 0));

	sk_mp9_bullet1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_mp9_bullet1", "", "0", 0));
	sk_mp9_bullet2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_mp9_bullet2", "", "0", 0));
	sk_mp9_bullet3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_mp9_bullet3", "", "0", 0));

	sk_kowloon_bullet1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_kowloon_bullet1", "", "0", 0));
	sk_kowloon_bullet2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_kowloon_bullet2", "", "0", 0));
	sk_kowloon_bullet3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_kowloon_bullet3", "", "0", 0));

	sk_pdw90_bullet1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_pdw90_bullet1", "", "0", 0));
	sk_pdw90_bullet2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_pdw90_bullet2", "", "0", 0));
	sk_pdw90_bullet3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_pdw90_bullet3", "", "0", 0));

	sk_buckshot_bullet1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_buckshot_bullet1", "", "0", 0));
	sk_buckshot_bullet2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_buckshot_bullet2", "", "0", 0));
	sk_buckshot_bullet3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_buckshot_bullet3", "", "0", 0));

	sk_sniper_bullet1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_sniper_bullet1", "", "0", 0));
	sk_sniper_bullet2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_sniper_bullet2", "", "0", 0));
	sk_sniper_bullet3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_sniper_bullet3", "", "0", 0));

	sk_alerted_bullet1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_alerted_bullet1", "", "0", 0));
	sk_alerted_bullet2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_alerted_bullet2", "", "0", 0));
	sk_alerted_bullet3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_alerted_bullet3", "", "0", 0));

	sk_raptor_bullet1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_raptor_bullet1", "", "0", 0));
	sk_raptor_bullet2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_raptor_bullet2", "", "0", 0));
	sk_raptor_bullet3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_raptor_bullet3", "", "0", 0));

	sk_minigun_bullet1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_minigun_bullet1", "", "0", 0));
	sk_minigun_bullet2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_minigun_bullet2", "", "0", 0));
	sk_minigun_bullet3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_minigun_bullet3", "", "0", 0));

	sk_laser_bolt1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_laser_bolt1", "", "0", 0));
	sk_laser_bolt2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_laser_bolt2", "", "0", 0));
	sk_laser_bolt3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_laser_bolt3", "", "0", 0));

	sk_ei_grenade1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_ei_grenade1", "", "0", 0));
	sk_ei_grenade2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_ei_grenade2", "", "0", 0));
	sk_ei_grenade3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_ei_grenade3", "", "0", 0));

	sk_suitcharger1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_suitcharger1", "", "0", 0));
	sk_suitcharger2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_suitcharger2", "", "0", 0));
	sk_suitcharger3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_suitcharger3", "", "0", 0));

	sk_battery1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_battery1", "", "0", 0));
	sk_battery2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_battery2", "", "0", 0));
	sk_battery3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_battery3", "", "0", 0));

	sk_healthcarger1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_healthcarger1", "", "0", 0));
	sk_healthcarger2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_healthcarger2", "", "0", 0));
	sk_healthcharger3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_healthcharger3", "", "0", 0));

	sk_healthkit1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_healthkit1", "", "0", 0));
	sk_healthkit2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_healthkit2", "", "0", 0));
	sk_healthkit3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_healthkit3", "", "0", 0));

	sk_character_head1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_character_head1", "", "2", 0));
	sk_character_head2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_character_head2", "", "2", 0));
	sk_character_head3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_character_head3", "", "2", 0));

	sk_character_chest1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_character_chest1", "", "1", 0));
	sk_character_chest2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_character_chest2", "", "1", 0));
	sk_character_chest3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_character_chest3", "", "1", 0));

	sk_character_stomach1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_character_stomach1", "", "1", 0));
	sk_character_stomach2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_character_stomach2", "", "1", 0));
	sk_character_stomach3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_character_stomach3", "", "1", 0));

	sk_character_arm1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_character_arm1", "", "1", 0));
	sk_character_arm2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_character_arm2", "", "1", 0));
	sk_character_arm3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_character_arm3", "", "1", 0));

	sk_character_leg1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_character_leg1", "", "1", 0));
	sk_character_leg2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_character_leg2", "", "1", 0));
	sk_character_leg3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_character_leg3", "", "1", 0));

	sk_player_head1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_player_head1", "", "2", 0));
	sk_player_head2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_player_head2", "", "2", 0));
	sk_player_head3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_player_head3", "", "2", 0));

	sk_player_chest1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_player_chest1", "", "1", 0));
	sk_player_chest2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_player_chest2", "", "1", 0));
	sk_player_chest3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_player_chest3", "", "1", 0));

	sk_player_stomach1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_player_stomach1", "", "1", 0));
	sk_player_stomach2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_player_stomach2", "", "1", 0));
	sk_player_stomach3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_player_stomach3", "", "1", 0));

	sk_player_arm1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_player_arm1", "", "1", 0));
	sk_player_arm2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_player_arm2", "", "1", 0));
	sk_player_arm3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_player_arm3", "", "1", 0));

	sk_player_leg1 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_player_leg1", "", "1", 0));
	sk_player_leg2 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_player_leg2", "", "1", 0));
	sk_player_leg3 = CVAR_REGISTER(IConsoleVariable(CVAR_INT, "sk_player_leg3", "", "1", 0));

	SERVER_COMMAND( "exec skill.cfg\n" );
}

