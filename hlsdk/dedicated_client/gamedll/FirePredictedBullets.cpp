#include "..\nightfire_hooks.h"
#include "..\pattern_scanner.h"
#include "nightfire_pointers.h"

#include <extdll.h>
#include <pm_defs.h>
#include "util.h"

#include "cbase.h"
#include "player.h"
#include "trains.h"
#include "nodes.h"
#include "weapons.h"
#include "soundent.h"
#include "monsters.h"
#include "shake.h"
#include "decals.h"
#include "gamerules.h"
#include "game.h"
#include "pm_shared.h"
#include "hltv.h"

#include "..\globals.h"



Vector*(__thiscall* g_oFirePredictedBullets_Server)(CBasePlayer* player, Vector* returnval, ULONG cShots, Vector vecSrc, Vector vecDirShooting, float flSpread, float flDistance, int iBulletType) = nullptr;

Vector* __fastcall nf_hooks::FirePredictedBullets_ServerHook(CBasePlayer* player, void* edx, Vector* returnval, ULONG cShots, Vector vecSrc, Vector vecDirShooting, float flSpread, float flDistance, int iBulletType)
{
	return g_oFirePredictedBullets_Server(player, returnval, cShots, vecSrc, vecDirShooting, flSpread, flDistance, iBulletType);

#if 0
	TraceResult tr;
	Vector vecRight = g_Pointers.gpGlobals->v_right;
	Vector vecUp = g_Pointers.gpGlobals->v_up;
	float x = 0.0, y = 0.0;
	float z;

	ClearMultiDamage();
	gMultiDamage.type = DMG_BULLET | DMG_NEVERGIB;

	entvars_t* pevAttacker = player->pev;

	CBaseEntity* shooter = nullptr;
	if (pevAttacker && g_Pointers.g_pEngineFuncs->pfnEntOffsetOfPEntity(pevAttacker->pContainingEntity))
		shooter = CBaseEntity::Instance(pevAttacker);

	if (shooter && shooter->IsPlayer())
		shooter->IncrementNumShots();
	else
		shooter = nullptr;

	int entity_type_hit = 0;

	for (ULONG iShot = 1; iShot <= cShots; iShot++)
	{
		CBaseEntity* pEntity = (CBaseEntity*)player;
		int num_walls_hit = 0;
		bool bullet_blocked = false;

		//Use player's random seed.
		// get circular gaussian spread

		do {
			x = UTIL_SharedRandomFloat(player->random_seed, -0.5, 0.5) + UTIL_SharedRandomFloat(player->random_seed + 1, -0.5, 0.5);
			player->random_seed += 2;
			y = UTIL_SharedRandomFloat(player->random_seed, -0.5, 0.5) + UTIL_SharedRandomFloat(player->random_seed + 1, -0.5, 0.5);
			player->random_seed += 2;

			z = x * x + y * y;
		} while (z > 1);

		Vector vecDir = vecDirShooting +
			x * flSpread * vecRight +
			y * flSpread * vecUp;
		Vector vecEnd = vecSrc + vecDir * flDistance;				
		
		float recoil_force = 0.0f;

		while (num_walls_hit < 2 && !bullet_blocked)
		{
			UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, 0, pEntity->pev->pContainingEntity, &tr);

			if (tr.flFraction != 1.0)
			{
				if (tr.pHit && g_Pointers.g_pEngineFuncs->pfnEntOffsetOfPEntity(tr.pHit))
					pEntity = CBaseEntity::Instance(tr.pHit);

				if (entindex() == pEntity->entindex())
					continue;

				// do damage, paint decals
				switch (iBulletType)
				{
				default:
				case BULLET_PLAYER_P99:
					recoil_force = 32.0f;
					pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgP99, vecDir, &tr, DMG_BULLET);
					TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
					break;

				case BULLET_PLAYER_KOWLOON:
					recoil_force = 32.0f;
					pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgKowloon, vecDir, &tr, DMG_BULLET);
					TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
					break;

				case BULLET_PLAYER_RAPTOR:
					recoil_force = 32.0f;
					pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgRaptor, vecDir, &tr, DMG_BULLET);
					TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
					break;

				case BULLET_PLAYER_MP9:
					recoil_force = 32.0f;
					pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgMP9, vecDir, &tr, DMG_BULLET);
					break;

				case BULLET_PLAYER_SNIPER:
					recoil_force = 64.0f;
					pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgSniper, vecDir, &tr, DMG_BULLET);
					break;

				case BULLET_PLAYER_BUCKSHOT:
				{
					float fraction = tr.flFraction <= 1.0 ? tr.flFraction : 1.0;
					float scale = 1.0 - fraction;
					float dmg = min(1.0f, gSkillData.plrDmgBuckshot * (scale * scale));
					recoil_force = 48.0f;

					pEntity->TraceAttack(pevAttacker, dmg, vecDir, &tr, DMG_BULLET);
					break;
				}

				case BULLET_PLAYER_COMMANDO:
					recoil_force = 48.0f;
					pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgCommando, vecDir, &tr, DMG_BULLET);
					break;

				case BULLET_PLAYER_PDW90:
					recoil_force = 48.0f;
					pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgPDW90, vecDir, &tr, DMG_BULLET);
					break;

				case BULLET_PLAYER_MINIGUN:
					recoil_force = 48.0f;
					pEntity->TraceAttack(pevAttacker, gSkillData.plrDmgMinigun, vecDir, &tr, DMG_BULLET);
					break;

				case BULLET_NONE: // FIX 
					//recoil_force unknown?
					pEntity->TraceAttack(pevAttacker, 50, vecDir, &tr, DMG_CLUB);
					//TEXTURETYPE_PlaySound(&tr, vecSrc, vecEnd, iBulletType);
					// only decal glass
					//if (!FNullEnt(tr.pHit) && VARS(tr.pHit)->rendermode != 0)
					//{
					//	UTIL_DecalTrace(&tr, DECAL_GLASSBREAK1 + RANDOM_LONG(0, 2));
					//}
					break;
				}
			}

			// some gearbox force knockback mechanism
			if (pev->movetype == 2)
			{
				pev->velocity[0] += vecDir * -recoil_force;
				pev->velocity[1] += vecDir * -recoil_force;
				pev->velocity[2] += vecDir * -recoil_force;
			}

			// check what we hit
			if (pEntity->MyCharacterPointer())
				entity_type_hit = 1; // hit npc
			else if (entity_type_hit != 1 && pEntity->pev->takedamage != 0)
				entity_type_hit = 2; // hit breakable object

			// notify enemies at endpos
			if (m_iWeaponVolume == NORMAL_WEAPON_VOLUME)
			{
				Vector angles = UTIL_VecToAngles(tr.vecPlaneNormal);
				CSoundEnt::InsertSound(4, tr.vecEndPos, angles, 256, 1.0f);
			}

			// allow bullet penetration
			if (pEntity->pev->solid == SOLID_BSP && (pEntity->pev->spawnflags & SF_ALLOW_BULLETS) != 0)
			{
				bullet_blocked = false;

				vecSrc[0] = tr.vecEndPos[0] + vecDir[0];
				vecSrc[1] = tr.vecEndPos[1] + vecDir[1];
				vecSrc[2] = tr.vecEndPos[2] + vecDir[2];
			}
			else
			{
				bullet_blocked = true;

				vecSrc[0] = tr.vecEndPos[0];
				vecSrc[1] = tr.vecEndPos[1];
				vecSrc[2] = tr.vecEndPos[2];
			}

			// make bullet trails
			UTIL_BubbleTrail(vecSrc, tr.vecEndPos, static_cast<int>((flDistance * tr.flFraction) / 64.0));

			++num_walls_hit;
		}
	}

	if (shooter && entity_type_hit)
	{
		if (entity_type_hit == 2)
			shooter->DecrementNumShots();
		else
			shooter->IncrementNumShotsHitEnemies();
	}

	g_pGameRules->ShotFired(player->pev);
	ApplyMultiDamage(player->pev, pevAttacker);

	*returnval = Vector(x * flSpread, y * flSpread, 0.0);
	return returnval;
#endif
}

Vector*(__thiscall* g_oFirePredictedBullets_Client)(CBasePlayer* player, Vector* returnval, ULONG cShots, Vector vecSrc, Vector vecDirShooting, float flSpread, float flDistance, int iBulletType) = nullptr;

Vector* __fastcall nf_hooks::FirePredictedBullets_ClientHook(CBasePlayer* player, void* edx, Vector* returnval, ULONG cShots, Vector vecSrc, Vector vecDirShooting, float flSpread, float flDistance, int iBulletType)
{
	return g_oFirePredictedBullets_Client(player, returnval, cShots, vecSrc, vecDirShooting, flSpread, flDistance, iBulletType);

#if 0

	float x = 0.0, y = 0.0, z;
	//Use player's random seed.
	// get circular gaussian spread
	do {
		x = UTIL_SharedRandomFloat(random_seed, -0.5, 0.5) + UTIL_SharedRandomFloat(random_seed + 1, -0.5, 0.5);
		random_seed += 2;
		y = UTIL_SharedRandomFloat(random_seed, -0.5, 0.5) + UTIL_SharedRandomFloat(random_seed + 1, -0.5, 0.5);
		random_seed += 2;

		z = x * x + y * y;
	} while (z > 1);

	*returnval = Vector(x * flSpread, y * flSpread, 0.0);
	return returnval;
#endif
}

void Fix_FirePredictedBullets_GameDLL()
{
	DWORD adr;
	if (!FindMemoryPattern(pattern_t(adr, g_gameDllHinst, "E8 ? ? ? ? 8B 86 ? ? ? ? 8B 4C 24 2C", false, false, 0, 1, true, "FirePredictedBullets_GameDLL", true)))
		return;

	if (!HookFunctionWithMinHook(adr, nf_hooks::FirePredictedBullets_ServerHook, (void**)&g_oFirePredictedBullets_Server))
		return;
}

void Fix_FirePredictedBullets_ClientDLL()
{
	DWORD adr;
	if (!FindMemoryPattern(pattern_t(adr, *g_clientDllHinst, "E8 ? ? ? ? 8B 86 ? ? ? ? 8B 4C 24 2C", false, false, 0, 1, true, "FirePredictedBullets_ClientDLL", true)))
		return;

	if (!HookFunctionWithMinHook(adr, nf_hooks::FirePredictedBullets_ClientHook, (void**)&g_oFirePredictedBullets_Client))
		return;
}