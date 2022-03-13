#include "Flashlight.h"
#include "DynamicLight.h"
#include "DylansHooks.h"

void OnModuleInitialize(void) {
	ADD_DLL_HOOK(&callOnFireBullets, ADD_HOOK_TYPES::H_FIREBULLETS);
	ADD_DLL_HOOK(&callOnPlayerKilled, ADD_HOOK_TYPES::H_PLAYERKILLED);
	ADD_DLL_HOOK(&callOnHudInit, ADD_HOOK_TYPES::H_HUDINIT);
	printf("Flashlight Initialized\n");
}

void OnClientConnect(edict_t *pEntity)
{

}

void callOnHudInit()
{
	for (int i = 0; i < gpGlobals->maxClients; i++)
	{
		if (plinfo[i +1].bUsingFlashlight)
		{
			edict_t *pEntity = EDICT_NUM(i);
			EnableLightBeam(pEntity, ENTINDEX(pEntity));
		}
	}
}

void OnClientDisconnect(edict_t *pEntity)
{
	DisableFlashlight(pEntity);
}

void callOnPlayerKilled(entvars_t *pevAttacker, int iGib, C_BasePlayer *pVictim)
{
	DisableFlashlight(pVictim->edict());
}

void callOnFireBullets(edict_t *edict, Vector vecSrc, Vector vecShootDir, float dist) {
	if ( glinfo.TotalPlayersUsingFlashlight <= 24 ) {
		UTIL_Create_TE_DLIGHT(0, vecSrc, 2, 125, Color(255, 255, 170), 1, 100.0f );
		UTIL_Create_TE_ELIGHT(edict, vecSrc, 100, Color(255, 255, 170), 1, 100 );
	}
}

void DisableLightBeam(edict_t *pEntity, int index) {
	if ( plinfo[index].bUsingFlashlight ) {
		MESSAGE_BEGIN(MSG_ALL, GET_USER_MSG_ID(PLID, "LightBeam", NULL), 0, (edict_t*)0);
			WRITE_SHORT(index);
			WRITE_BYTE(4); //start attachment
			WRITE_BYTE(2); //end attachment
			WRITE_BYTE(0); //enable/disable
			WRITE_SHORT(128); //max distance
		MESSAGE_END();
	}
}

void EnableLightBeam(edict_t *pEntity, int index) {
	for ( int i = 0; i < gpGlobals->maxClients; i++ ) {
			client_t *pClient = GetClientPointerFromIndex(i);
			//edict_t *pEdict = C_BasePlayer::Instance(i)->edict();
			if ( pClient && pClient->connected && pClient->active && pClient->edict != pEntity && !pClient->fakeclient ) {
				MESSAGE_BEGIN(MSG_ONE, GET_USER_MSG_ID(PLID, "LightBeam", NULL), 0, pClient->edict);
					WRITE_SHORT(index);
					WRITE_BYTE(4); //start attachment //0
					WRITE_BYTE(2); //end attachment	  //2
					WRITE_BYTE(1); //enable/disable
					WRITE_SHORT(128); //max distance
				MESSAGE_END();
			}
	}
}


void ToggleFlashlight(edict_t *pEntity) {
	if ( CVAR_GET_BOOL("mp_flashlight") ) {
		int index = ENTINDEX(pEntity);
		if ( plinfo[index].bSupportsDynamicLights ) { //if ( index >= 1 && index <= gpGlobals->maxClients ) { //this check is pointless
			if ( plinfo[index].bUsingFlashlight ) {
				DisableLightBeam(pEntity, index);
				plinfo[index].bUsingFlashlight = false;
				glinfo.TotalPlayersUsingFlashlight--;
			} else {
				plinfo[index].bUsingFlashlight = true;
				glinfo.TotalPlayersUsingFlashlight++;
				plinfo[index].flNextFlashlightUpdate = gpGlobals->time;
				plinfo[index].flNextLightBeamUpdate = gpGlobals->time;
			}
		} else { 
			Bond_HudMsg(pEntity, "ERROR:\nO Cannot enable flashlight!\nO You must update to game version 5.82 or higher!", 4, SK_OPERATIVE);
		}
	}
}

void EnableFlashlight(edict_t *pEntity) {
	int index = ENTINDEX(pEntity);
	if ( plinfo[index].bSupportsDynamicLights ) { //if ( index >= 1 && index <= gpGlobals->maxClients ) { //this check is pointless
		if ( !plinfo[index].bUsingFlashlight ) {
			client_t *pClient = EDICT_TO_CLIENT(pEntity);
			if ( pClient->pViewEntity == pEntity) { //only enable flashlight if they aren't spectating
				plinfo[index].bUsingFlashlight = true;
				glinfo.TotalPlayersUsingFlashlight++;
				plinfo[index].flNextFlashlightUpdate = gpGlobals->time;
				plinfo[index].flNextLightBeamUpdate = gpGlobals->time;
			}
		}
	} else {
		Bond_HudMsg(pEntity, "ERROR:\nO Cannot enable flashlight!\nO You must update to game version 5.82 or higher!", 4, SK_OPERATIVE);
	}
}

void DisableFlashlight(edict_t *pEntity) {
	int index = ENTINDEX(pEntity);
	//if ( index >= 1 && index <= gpGlobals->maxClients ) { //this check is pointless
		if ( plinfo[index].bUsingFlashlight ) {
			DisableLightBeam(pEntity, index);
			plinfo[index].bUsingFlashlight = false;
			glinfo.TotalPlayersUsingFlashlight--;
		}
		plinfo[index].flNextFlashlightUpdate = gpGlobals->time;
	//}
}

#define CLOSEdeg 20.0f
#define CLOSEdist 300
#define CLOSEdistunrounded (CLOSEdist - 5)
#define flashlightlife 1.0f //100ms
#define flashlightlifesecs (flashlightlife / 10.0f ) //100ms

void FlashlightThink(edict_t *pEntity, int index) {
	//int index = ENTINDEX(pEntity); //g_engfuncs.pfnNUM_FOR_EDICT(pEntity);
		if ( plinfo[index].bUsingFlashlight && gpGlobals->time + gpGlobals->frametime >= plinfo[index].flNextFlashlightUpdate ) {
		double deltatime = (gpGlobals->time - plinfo[index].flNextFlashlightUpdate);
		if ( deltatime < 0 || deltatime > 1 )
			deltatime = 0;

		plinfo[index].flNextFlashlightUpdate = gpGlobals->time + gpGlobals->frametime + flashlightlifesecs - deltatime; /** 2.152)*/;

		Vector vecSrc = pEntity->v.origin;
		vecSrc.z += 32; //set height to head


		Vector eyeangles = ( pEntity->v.v_angle );
		//Vector weaponangles = eyeangles;
		//weaponangles.x /= 3;

		MAKE_VECTORS( eyeangles /*weaponangles*/ );
		Vector vecAiming = gpGlobals->v_forward;
#if 0
		if ( glinfo.TotalPlayersUsingFlashlight <= 16 ) {
			if ( glinfo.TotalPlayersUsingFlashlight <= 6 ) {
				//create up to 4 lights near around where the player is looking
				Vector vecAimingleft, vecAimingright, vecAimingup, vecAimingdown;
				Vector eyeanglesleft = eyeangles;
				Vector eyeanglesright = eyeangles;
				Vector eyeanglesup = eyeangles;
				Vector eyeanglesdown = eyeangles;
				eyeanglesleft.y += CLOSEdeg;
				eyeanglesright.y -= CLOSEdeg;
				eyeanglesup.x -= CLOSEdeg;
				eyeanglesdown.x += CLOSEdeg;
				MAKE_VECTORS( eyeanglesleft );
				vecAimingleft = gpGlobals->v_forward;
				MAKE_VECTORS( eyeanglesright );
				vecAimingright = gpGlobals->v_forward;
				MAKE_VECTORS( eyeanglesup );
				vecAimingup = gpGlobals->v_forward;
				MAKE_VECTORS( eyeanglesdown );
				vecAimingdown = gpGlobals->v_forward;

				TraceResult trleft;
				TraceResult trright;
				TraceResult trup;
				TraceResult trdown;

				TRACE_LINE(vecSrc, vecSrc + vecAimingleft * CLOSEdist, 0, dont_ignore_monsters, pEntity, &trleft);
				TRACE_LINE(vecSrc, vecSrc + vecAimingright * CLOSEdist, 0, dont_ignore_monsters, pEntity, &trright);
				TRACE_LINE(vecSrc, vecSrc + vecAimingup * CLOSEdist, 0, dont_ignore_monsters, pEntity, &trup);
				TRACE_LINE(vecSrc, vecSrc + vecAimingdown * CLOSEdist, 0, dont_ignore_monsters, pEntity, &trdown);

				float distancefromwall = (trleft.vecEndPos - vecSrc).Length();
				bool createdcloseElight = false;
				if ( distancefromwall < CLOSEdistunrounded ) {
					float flashlightsizeleft = distancefromwall / 1.5f;
					float brightnessleft = ((1 / flashlightsizeleft) * 36) / (distancefromwall * 0.0019f); //+ 1.5f;
						
					UTIL_Create_TE_DLIGHT(0, trleft.vecEndPos, brightnessleft, flashlightsizeleft, Color(200, 250, 255), flashlightlife, 0.0f );
					createdcloseElight = true;
					UTIL_Create_TE_ELIGHT(0, trleft.vecEndPos, flashlightsizeleft, Color(200, 250, 255), flashlightlife, 0.0f );
				}

				distancefromwall = (trright.vecEndPos - vecSrc).Length();
				if ( distancefromwall < CLOSEdistunrounded ) {
					float flashlightsizeright = distancefromwall / 1.5f;
					float brightnessright = ((1 / flashlightsizeright) * 36) / (distancefromwall * 0.0019f); //+ 1.5f;
						
					UTIL_Create_TE_DLIGHT(0, trright.vecEndPos, brightnessright, flashlightsizeright, Color(200, 250, 255), flashlightlife, 0.0f );
					if ( !createdcloseElight ) {
						createdcloseElight = true;
						UTIL_Create_TE_ELIGHT(0, trright.vecEndPos, flashlightsizeright, Color(200, 250, 255), flashlightlife, 0.0f );
					}
				}

				distancefromwall = (trup.vecEndPos - vecSrc).Length();
				if ( distancefromwall < CLOSEdistunrounded ) {
					float flashlightsizeup = distancefromwall / 1.5f;
					float brightnessup = ((1 / flashlightsizeup) * 36) / (distancefromwall * 0.0019f); //+ 1.5f;
						
					UTIL_Create_TE_DLIGHT(0, trup.vecEndPos, brightnessup, flashlightsizeup, Color(200, 250, 255), flashlightlife, 0.0f );
					if ( !createdcloseElight ) {
						createdcloseElight = true;
						UTIL_Create_TE_ELIGHT(0, trup.vecEndPos, flashlightsizeup, Color(200, 250, 255), flashlightlife, 0.0f );
					}
				}
					
				distancefromwall = (trdown.vecEndPos - vecSrc).Length();
				if ( distancefromwall < CLOSEdistunrounded ) {
					float flashlightsizedown = distancefromwall / 1.5f;
					float brightnessdown = ((1 / flashlightsizedown) * 36) / (distancefromwall * 0.0019f); //+ 1.5f;
						
					UTIL_Create_TE_DLIGHT(0, trdown.vecEndPos, brightnessdown, flashlightsizedown, Color(200, 250, 255), flashlightlife, 0.0f );
					if ( !createdcloseElight ) {
						createdcloseElight = true;
						UTIL_Create_TE_ELIGHT(0, trdown.vecEndPos, flashlightsizedown, Color(200, 250, 255), flashlightlife, 0.0f );
					}
				}
			} else {
				//# of players using flashlight > 6 but < 17
				//create 1 light near the player
				TraceResult trclose;
				TRACE_LINE(vecSrc, vecSrc + vecAiming * 100, 0, dont_ignore_monsters, pEntity, &trclose);
				float distancefromwallclose = (trclose.vecEndPos - vecSrc).Length();
				float flashlightsizeclose = distancefromwallclose / 0.75f;
				float brightnessclose = ((1 / flashlightsizeclose) * 25) / (distancefromwallclose * 0.0019f); //+ 1.5f;
				UTIL_Create_TE_DLIGHT(0, trclose.vecEndPos, brightnessclose, flashlightsizeclose, Color(200, 250, 255), flashlightlife, 0.0f );
				UTIL_Create_TE_ELIGHT(0, trclose.vecEndPos, flashlightsizeclose * 2, Color(200, 250, 255), flashlightlife, 0.0f );
			}
		}
#endif	

		if ( glinfo.TotalPlayersUsingFlashlight <= 4 ) { //could be 16, but take into account gunfire
			//create 1 light near the player
			UTIL_Create_TE_DLIGHT(0, vecSrc + vecAiming * 128, /*brightness*/1, /*size*/128, Color(200, 250, 255), (int)flashlightlife, 0.0f );
			//UTIL_Create_TE_ELIGHT(0, vecSrc + vecAiming * 64, 8, Color(200, 250, 255), flashlightlife, 0.0f );
		}
			//create the light where the gun is pointing
			TraceResult tr;
			TRACE_LINE(vecSrc, vecSrc + vecAiming * 3072, 0x100 /*ignore_glass*/, 0, pEntity, &tr);
			/*
			if ( !strcmp(STRING(tr.pHit->v.classname), "func_breakable") || !strcmp(STRING(tr.pHit->v.classname), "func_wall") ) {
				//trace through glass the bad way
				if ( tr.pHit->v.rendermode == kRenderTransTexture || tr.pHit->v.rendermode == kRenderTransAlpha || tr.pHit->v.rendermode == kRenderTransAdd ) {
					TRACE_LINE(tr.vecEndPos, tr.vecEndPos + vecAiming * 3072, dont_ignore_monsters | ignore_glass, 0, tr.pHit, &tr);
				}
			}
			*/

			float distancefromwall = (tr.vecEndPos - vecSrc).Length();
			float flashlightsize = distancefromwall / 2.75f; //3.75f
			float brightness = ((1 / flashlightsize) * 200) / (distancefromwall * 0.0015f); //+ 1.5f;
			UTIL_Create_TE_DLIGHT(0, tr.vecEndPos, brightness, flashlightsize, Color(200, 250, 255), (int)flashlightlife, 0.0f );
			UTIL_Create_TE_ELIGHT(0, tr.vecEndPos + -vecAiming * 48, max(75, flashlightsize / 2.2), Color(200, 250, 255), (int)flashlightlife, 0.0f );
													//reverse so entities are lit properly
	}
}

void OnPlayerPreThink(edict_t *pEntity, int index)
{
	FlashlightThink(pEntity, index);
}

int callOnAddToFullPack(struct entity_state_s *state, int e, edict_t *ent, edict_t *host, int hostflags, int player, unsigned char *pSet)
{
	int index = ENTINDEX(ent);
	if (/*index && */index <= 32)
	{
		if (plinfo[index].bUsingFlashlight)
		{
			return 1;
		}
	}
	return -1;
}