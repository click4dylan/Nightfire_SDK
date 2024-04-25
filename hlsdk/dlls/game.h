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

#ifndef GAME_H
#define GAME_H

extern void GameDLLInit( void );

extern ConsoleVariable* gMissionName;

extern ConsoleVariable*	displaysoundlist;

// multiplayer server rules
extern ConsoleVariable*	mp_teamplay;
extern ConsoleVariable*	mp_fraglimit;
extern ConsoleVariable* mp_fragsleft;
extern ConsoleVariable*	mp_timelimit;
extern ConsoleVariable* mp_timeleft;
extern ConsoleVariable*	mp_friendlyfire;
extern ConsoleVariable*	mp_falldamage;
extern ConsoleVariable*	mp_weaponstay;
extern ConsoleVariable* ctf_capture;
extern ConsoleVariable*	mp_forcerespawn;
extern ConsoleVariable*	mp_flashlight;
extern ConsoleVariable*	mp_teamlist;
extern ConsoleVariable*	mp_teamoverride;
extern ConsoleVariable*	mp_defaultteam;
extern ConsoleVariable*	mp_allowcharacters;
extern ConsoleVariable* mp_chattime;
extern ConsoleVariable* mp_autoaim;
extern ConsoleVariable* mp_multipower;
extern ConsoleVariable* mp_spamdelay;
extern ConsoleVariable* ctf_autoteam;
extern ConsoleVariable* sv_waypoint;
extern ConsoleVariable* sp_playerbody;
extern ConsoleVariable* sp_nodefloatout;
extern ConsoleVariable* sp_showaistate;
extern ConsoleVariable* sp_showcamlaser;
extern ConsoleVariable* showtriggers;

extern ConsoleVariable* sk_agrunt_health1;
extern ConsoleVariable* sk_agrunt_health2;
extern ConsoleVariable* sk_agrunt_health3;

extern ConsoleVariable* sk_agrunt_dmg_punch1;
extern ConsoleVariable* sk_agrunt_dmg_punch2;
extern ConsoleVariable* sk_agrunt_dmg_punch3;

extern ConsoleVariable* sk_attackcopter_health1;
extern ConsoleVariable* sk_attackcopter_health2;
extern ConsoleVariable* sk_attackcopter_health3;

extern ConsoleVariable* sk_attackcopter_shoot1;
extern ConsoleVariable* sk_attackcopter_shoot2;
extern ConsoleVariable* sk_attackcopter_shoot3;

extern ConsoleVariable* sk_helicopter_health1;
extern ConsoleVariable* sk_helicopter_health2;
extern ConsoleVariable* sk_helicopter_health3;

extern ConsoleVariable* sk_helicopter_guard_health1;
extern ConsoleVariable* sk_helicopter_guard_health2;
extern ConsoleVariable* sk_helicopter_guard_health3;

extern ConsoleVariable* sk_ninja_health1;
extern ConsoleVariable* sk_ninja_health2;
extern ConsoleVariable* sk_ninja_health3;

extern ConsoleVariable* sk_ninja_shoot1;
extern ConsoleVariable* sk_ninja_shoot2;
extern ConsoleVariable* sk_ninja_shoot3;

extern ConsoleVariable* sk_ninja_slash1;
extern ConsoleVariable* sk_ninja_slash2;
extern ConsoleVariable* sk_ninja_slash3;

extern ConsoleVariable* sk_rook_health1;
extern ConsoleVariable* sk_rook_health2;
extern ConsoleVariable* sk_rook_health3;

extern ConsoleVariable* sk_drake_health1;
extern ConsoleVariable* sk_drake_health2;
extern ConsoleVariable* sk_drake_health3;

extern ConsoleVariable* sk_drake_slash1;
extern ConsoleVariable* sk_drake_slash2;
extern ConsoleVariable* sk_drake_slash3;

extern ConsoleVariable* sk_osatovan_health1;
extern ConsoleVariable* sk_osatovan_health2;
extern ConsoleVariable* sk_osatovan_health3;

extern ConsoleVariable* sk_osatovan_shoot1;
extern ConsoleVariable* sk_osatovan_shoot2;
extern ConsoleVariable* sk_osatovan_shoot3;

extern ConsoleVariable* sk_enemy_health1;
extern ConsoleVariable* sk_enemy_health2;
extern ConsoleVariable* sk_enemy_health3;

extern ConsoleVariable* sk_enemy_kick1;
extern ConsoleVariable* sk_enemy_kick2;
extern ConsoleVariable* sk_enemy_kick3;

extern ConsoleVariable* sk_enemy_pellets1;
extern ConsoleVariable* sk_enemy_pellets2;
extern ConsoleVariable* sk_enemy_pellets3;

extern ConsoleVariable* sk_enemy_gspeed1;
extern ConsoleVariable* sk_enemy_gspeed2;
extern ConsoleVariable* sk_enemy_gspeed3;

extern ConsoleVariable* sk_enemy_rocket1;
extern ConsoleVariable* sk_enemy_rocket2;
extern ConsoleVariable* sk_enemy_rocket3;

extern ConsoleVariable* sk_npc_dumb_health1;
extern ConsoleVariable* sk_npc_dumb_health2;
extern ConsoleVariable* sk_npc_dumb_health3;

extern ConsoleVariable* sk_npc_health1;
extern ConsoleVariable* sk_npc_health2;
extern ConsoleVariable* sk_npc_health3;

extern ConsoleVariable* sk_turret_health1;
extern ConsoleVariable* sk_turret_health2;
extern ConsoleVariable* sk_turret_health3;

extern ConsoleVariable* sk_miniturret_health1;
extern ConsoleVariable* sk_miniturret_health2;
extern ConsoleVariable* sk_miniturret_health3;

extern ConsoleVariable* sk_sentry_health1;
extern ConsoleVariable* sk_sentry_health2;
extern ConsoleVariable* sk_sentry_health3;

extern ConsoleVariable* sk_turret_damage1;
extern ConsoleVariable* sk_turret_damage2;
extern ConsoleVariable* sk_turret_damage3;


extern ConsoleVariable* sk_scubasteve_dmg_shake1;
extern ConsoleVariable* sk_scubasteve_dmg_shake2;
extern ConsoleVariable* sk_scubasteve_dmg_shake3;

extern ConsoleVariable* sk_scubasteve_health1;
extern ConsoleVariable* sk_scubasteve_health2;
extern ConsoleVariable* sk_scubasteve_health3;

extern ConsoleVariable* sk_plr_fist1;
extern ConsoleVariable* sk_plr_fist2;
extern ConsoleVariable* sk_plr_fist3;

extern ConsoleVariable* sk_plr_pp9_bullet1;
extern ConsoleVariable* sk_plr_pp9_bullet2;
extern ConsoleVariable* sk_plr_pp9_bullet3;

extern ConsoleVariable* sk_plr_kowloon_bullet1;
extern ConsoleVariable* sk_plr_kowloon_bullet2;
extern ConsoleVariable* sk_plr_kowloon_bullet3;

extern ConsoleVariable* sk_plr_raptor_bullet1;
extern ConsoleVariable* sk_plr_raptor_bullet2;
extern ConsoleVariable* sk_plr_raptor_bullet3;

extern ConsoleVariable* sk_plr_commando_bullet1;
extern ConsoleVariable* sk_plr_commando_bullet2;
extern ConsoleVariable* sk_plr_commando_bullet3;

extern ConsoleVariable* sk_plr_minigun_bullet1;
extern ConsoleVariable* sk_plr_minigun_bullet2;
extern ConsoleVariable* sk_plr_minigun_bullet3;

extern ConsoleVariable* sk_plr_pdw90_bullet1;
extern ConsoleVariable* sk_plr_pdw90_bullet2;
extern ConsoleVariable* sk_plr_pdw90_bullet3;

extern ConsoleVariable* sk_plr_mp9_bullet1;
extern ConsoleVariable* sk_plr_mp9_bullet2;
extern ConsoleVariable* sk_plr_mp9_bullet3;

extern ConsoleVariable* sk_plr_buckshot_bullet1;
extern ConsoleVariable* sk_plr_buckshot_bullet2;
extern ConsoleVariable* sk_plr_buckshot_bullet3;

extern ConsoleVariable* sk_plr_sniper_bullet1;
extern ConsoleVariable* sk_plr_sniper_bullet2;
extern ConsoleVariable* sk_plr_sniper_bullet3;

extern ConsoleVariable* sk_plr_hand_grenade1;
extern ConsoleVariable* sk_plr_hand_grenade2;
extern ConsoleVariable* sk_plr_hand_grenade3;

extern ConsoleVariable* sk_plr_tripmine1;
extern ConsoleVariable* sk_plr_tripmine2;
extern ConsoleVariable* sk_plr_tripmine3;

extern ConsoleVariable* sk_plr_rocket1;
extern ConsoleVariable* sk_plr_rocket2;
extern ConsoleVariable* sk_plr_rocket3;

extern ConsoleVariable* sk_plr_ronin_charge1;
extern ConsoleVariable* sk_plr_ronin_charge2;
extern ConsoleVariable* sk_plr_ronin_charge3;

extern ConsoleVariable* sk_plr_ronin_shoot1;
extern ConsoleVariable* sk_plr_ronin_shoot2;
extern ConsoleVariable* sk_plr_ronin_shoot3;

extern ConsoleVariable* sk_plr_ronin_health1;
extern ConsoleVariable* sk_plr_ronin_health2;
extern ConsoleVariable* sk_plr_ronin_health3;

extern ConsoleVariable* sk_plr_taser1;
extern ConsoleVariable* sk_plr_taser2;
extern ConsoleVariable* sk_plr_taser3;

extern ConsoleVariable* sk_plr_laser1;
extern ConsoleVariable* sk_plr_laser2;
extern ConsoleVariable* sk_plr_laser3;

extern ConsoleVariable* sk_plr_eigrenade1;
extern ConsoleVariable* sk_plr_eigrenade2;
extern ConsoleVariable* sk_plr_eigrenade3;

extern ConsoleVariable* sk_commando_bullet1;
extern ConsoleVariable* sk_commando_bullet2;
extern ConsoleVariable* sk_commando_bullet3;

extern ConsoleVariable* sk_mp9_bullet1;
extern ConsoleVariable* sk_mp9_bullet2;
extern ConsoleVariable* sk_mp9_bullet3;

extern ConsoleVariable* sk_kowloon_bullet1;
extern ConsoleVariable* sk_kowloon_bullet2;
extern ConsoleVariable* sk_kowloon_bullet3;

extern ConsoleVariable* sk_pdw90_bullet1;
extern ConsoleVariable* sk_pdw90_bullet2;
extern ConsoleVariable* sk_pdw90_bullet3;

extern ConsoleVariable* sk_buckshot_bullet1;
extern ConsoleVariable* sk_buckshot_bullet2;
extern ConsoleVariable* sk_buckshot_bullet3;

extern ConsoleVariable* sk_sniper_bullet1;
extern ConsoleVariable* sk_sniper_bullet2;
extern ConsoleVariable* sk_sniper_bullet3;

extern ConsoleVariable* sk_alerted_bullet1;
extern ConsoleVariable* sk_alerted_bullet2;
extern ConsoleVariable* sk_alerted_bullet3;

extern ConsoleVariable* sk_raptor_bullet1;
extern ConsoleVariable* sk_raptor_bullet2;
extern ConsoleVariable* sk_raptor_bullet3;

extern ConsoleVariable* sk_minigun_bullet1;
extern ConsoleVariable* sk_minigun_bullet2;
extern ConsoleVariable* sk_minigun_bullet3;

extern ConsoleVariable* sk_laser_bolt1;
extern ConsoleVariable* sk_laser_bolt2;
extern ConsoleVariable* sk_laser_bolt3;

extern ConsoleVariable* sk_ei_grenade1;
extern ConsoleVariable* sk_ei_grenade2;
extern ConsoleVariable* sk_ei_grenade3;

extern ConsoleVariable* sk_suitcharger1;
extern ConsoleVariable* sk_suitcharger2;
extern ConsoleVariable* sk_suitcharger3;

extern ConsoleVariable* sk_battery1;
extern ConsoleVariable* sk_battery2;
extern ConsoleVariable* sk_battery3;

extern ConsoleVariable* sk_healthcarger1;
extern ConsoleVariable* sk_healthcarger2;
extern ConsoleVariable* sk_healthcharger3;

extern ConsoleVariable* sk_healthkit1;
extern ConsoleVariable* sk_healthkit2;
extern ConsoleVariable* sk_healthkit3;

extern ConsoleVariable* sk_character_head1;
extern ConsoleVariable* sk_character_head2;
extern ConsoleVariable* sk_character_head3;

extern ConsoleVariable* sk_character_chest1;
extern ConsoleVariable* sk_character_chest2;
extern ConsoleVariable* sk_character_chest3;

extern ConsoleVariable* sk_character_stomach1;
extern ConsoleVariable* sk_character_stomach2;
extern ConsoleVariable* sk_character_stomach3;

extern ConsoleVariable* sk_character_arm1;
extern ConsoleVariable* sk_character_arm2;
extern ConsoleVariable* sk_character_arm3;

extern ConsoleVariable* sk_character_leg1;
extern ConsoleVariable* sk_character_leg2;
extern ConsoleVariable* sk_character_leg3;

extern ConsoleVariable* sk_player_head1;
extern ConsoleVariable* sk_player_head2;
extern ConsoleVariable* sk_player_head3;

extern ConsoleVariable* sk_player_chest1;
extern ConsoleVariable* sk_player_chest2;
extern ConsoleVariable* sk_player_chest3;

extern ConsoleVariable* sk_player_stomach1;
extern ConsoleVariable* sk_player_stomach2;
extern ConsoleVariable* sk_player_stomach3;

extern ConsoleVariable* sk_player_arm1;
extern ConsoleVariable* sk_player_arm2;
extern ConsoleVariable* sk_player_arm3;

extern ConsoleVariable* sk_player_leg1;
extern ConsoleVariable* sk_player_leg2;
extern ConsoleVariable* sk_player_leg3;



// Engine Cvars
extern ConsoleVariable	*g_psv_gravity;
//extern IConsoleVariable	*g_psv_aim;
extern ConsoleVariable	*g_psv_footsteps;

#endif		// GAME_H
