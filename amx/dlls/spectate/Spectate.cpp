#include "Spectate.h"
/*
#define HOOK_Spawn 0//x420E57A0
void *pOriginalFunction = NULL;
__declspec (naked) void Hook_Spawn()
{
    //save this pointer
    //void *pthis;
    //__asm mov pthis, ecx;
    //pre hook
    //SERVER_PRINT("::Spawn(void) prehook!\n");
    //call original function
    //__asm mov ecx, pthis;
	__asm mov eax, pOriginalFunction;
	__asm call eax
	__asm retn
    //__asm call [pOriginalFunction];
    //post hook
    //SERVER_PRINT("::Spawn(void) posthook!\n");
}
void Hook(void) {
  //get classname of the entity you want to hook
    const char *classname = CMD_ARGV(1);
    edict_t *pEdict = CREATE_ENTITY();
    CALL_GAME_ENTITY(PLID, classname, &pEdict->v.classname);
    if(pEdict->v.pvPrivateData == NULL)
    {
        REMOVE_ENTITY(pEdict);
        return;
    }
    //get this pointer
    void *pthis = pEdict->v.pvPrivateData;
    //get vtable
    void **vtbl = *(void ***)pthis;
    REMOVE_ENTITY(pEdict);
    if(vtbl == NULL)
        return;
    
    int **ivtbl = (int **)vtbl;
    //get the original function
    pOriginalFunction = (void *)ivtbl[HOOK_Spawn];
    
    //patch original function to our function
    DWORD oldflags;
    if(VirtualProtect(&ivtbl[HOOK_Spawn], sizeof(int *), PAGE_READWRITE, &oldflags))
    {
        ivtbl[HOOK_Spawn] = (int *)Hook_Spawn;
    }
}
*/
bool init = 0;
void OnModuleInitialize(void) {
	if (!init)
	{
		cvar_t_small allow_spectators = { CVAR_BOOL, "allow_spectators", "Toggle allowing spectators", "1", FCVAR_EXTDLL | FCVAR_SERVER };
		CVAR_REGISTER(allow_spectators);
		init = 1;
	}
	//REG_SVR_COMMAND("hook", Hook);
	printf("Spectate Initialized\n");
}

void SpectateConsoleCommand(edict_t *pEntity, int argl) {
	bool allow_spectators = CVAR_GET_BOOL("allow_spectators");
	if ( CVAR_GET_BOOL("allow_spectators") ) {
		C_BasePlayer* pPlayer = C_BasePlayer::Instance(pEntity);
		DropFlag(pPlayer);
		DropPowerups(pPlayer);
		if ( pEntity->v.deadflag != DEAD_DYING && pEntity->v.deadflag != DEAD_DEAD ) {
			if ( argl - 1 > 1 ) {
				if ( allow_spectators ) {
					int targetid = atoi(CMD_ARGV(1));
					int index = ENTINDEX(pEntity);
					int OurUserID = GetClientPointerByIndex(index - 1)->userid;
					plinfo[index].iLastTeam = C_BasePlayer::Instance(pEntity)->team;
					for ( int i = 1; i <= gpGlobals->maxClients; i++ ) {
						client_t *pPlayerToSpectate = GetClientPointerFromIndex(i - 1);
						//int actualdatagram = (int)pPlayerToSpectate + 0x25a0;
						//int *wrongdatagram = (int*)&pPlayerToSpectate->datagram;
						if ( pPlayerToSpectate->connected && pPlayerToSpectate->active ) {
							if ( plinfo[ENTINDEX(pPlayerToSpectate->edict)].bSpectating ) {
								MESSAGE_BEGIN(MSG_ONE, GET_USER_MSG_ID(PLID, "HudText", NULL), 0, pEntity);
									WRITE_STRING( "ERROR: Target player is already spectating someone!" );
								MESSAGE_END();
								break;
							}
							if ( (OurUserID != targetid) && (targetid == pPlayerToSpectate->userid) ) {
								plinfo[index].pSpectateTargetEdict = pPlayerToSpectate->edict;
								plinfo[index].pSpectateLastTargetEdict = pPlayerToSpectate->edict;
								if ( !plinfo[index].bSpectating ) {
									//If we are following something don't allow us to go into spectate
									if (!pPlayer->pev->aiment || pPlayer->pev->aiment == pEntity)
									{
										UpdatePlayerTeam(pEntity, "Spectators", 3);
										if ( !plinfo[index].bCodeEnabledSpectator ) {
											char msg[200];
											sprintf(msg, "%s has changed to team Spectators", GetClientPointerFromIndex(index - 1)->name);
											SayTextAll(msg);
										}
										MakePlayerInvisible(pEntity);
										MakePlayerNonSolid(pEntity);
										plinfo[index].iSpectateMode = MODE_THIRDPERSON;
										//CLIENT_COMMAND(pEntity, "togglehud\n"); //disable HUD
										plinfo[index].vecOriginBeforeSpectating = pEntity->v.origin;
										CreateCamera(plinfo[index].pCameraEdict);
										plinfo[index].pCameraEdict->v.owner = pEntity;
										plinfo[index].pCameraEdict->v.origin = pPlayerToSpectate->edict->v.origin;
										char str[50];
										sprintf(str, "Spectating: %s", pPlayerToSpectate->name);
										CLIENT_PRINTF(pEntity, print_center, str);
										SET_VIEW(pEntity, plinfo[index].pCameraEdict);
										plinfo[index].bSpectating = true;
									}
								}
								break;
							}
						}
					}
				}
			} else if ( plinfo[ENTINDEX(pEntity)].pCameraEdict ) {
				//we are already spectating someone, disable spectator mode
				ChangePlayerTeamCTF(pPlayer, "Spectators", plinfo[ENTINDEX(pEntity)].iLastTeam/*GetTeamchoice(pPlayer)*/);// , true);
				ExitSpectate(pEntity);
				//CLIENT_COMMAND(pEntity, "togglehud\n"); //enable HUD
			} else if ( allow_spectators ) {
				//free camera mode
				int index = ENTINDEX(pEntity);
				if ( plinfo[index].iSpectateMode == MODE_FREELOOK ) {
					//disable free camera and exit spectate
					DisableFreeLook(pEntity);
					plinfo[index].iSpectateMode = MODE_THIRDPERSON;
					//pEntity->v.origin = plinfo[index].vecOriginBeforeSpectating;
					pEntity->v.origin = Vector(16000, 16000, -16000); //When exiting spectate, hide dead body
					plinfo[index].bSpectating = false;
					//CLIENT_COMMAND(pEntity, "togglehud\n"); //enable HUD
					ChangePlayerTeamCTF(pPlayer, "Spectators" , plinfo[index].iLastTeam /*GetTeamchoice(pPlayer)*/);//, true);
					SpectatorKill(pEntity);
				} else {
					//If we are following something don't allow us to go into spectate
					if (!pPlayer->pev->aiment || pPlayer->pev->aiment == pEntity)
					{
						//enable free camera
						plinfo[index].iLastTeam = C_BasePlayer::Instance(pEntity)->team;
						if ( !plinfo[index].bCodeEnabledSpectator ) {
							char msg[200];
							UpdatePlayerTeam(pEntity, "Spectators", 3);
							sprintf(msg, "%s has changed to team Spectators", GetClientPointerFromIndex(index - 1)->name);
							SayTextAll(msg);
						}
						plinfo[index].vecOriginBeforeSpectating = pEntity->v.origin;
						EnableFreeLook(pEntity);
						plinfo[index].bSpectating = true;
						//CLIENT_COMMAND(pEntity, "togglehud\n"); //disable HUD
					}
				}
			}
		}
	}
}

void ExitSpectate(edict_t *pEntity) {
	DeleteCamera(pEntity);
	DisableFreeLook(pEntity);
	MakePlayerVisible(pEntity);
	if ( !plinfo[ENTINDEX(pEntity)].bCodeEnabledSpectator ) {
		MakePlayerSolid(pEntity);
	}
	plinfo[ENTINDEX(pEntity)].bSpectating = false;
	plinfo[ENTINDEX(pEntity)].iSpectateMode = MODE_THIRDPERSON;
	pEntity->v.origin = Vector(16000, 16000, -16000); //When exiting spectate, hide dead body
	SpectatorKill(pEntity);
}

bool SpectateLastPlayer(edict_t *pEntity) {
	int index = ENTINDEX(pEntity);
	int OurUserID = GetClientPointerByIndex(index - 1)->userid;
	if ( plinfo[index].pSpectateLastTargetEdict ) {
		client_t *pPlayerToSpectate = GetClientPointerFromIndex(ENTINDEX(plinfo[index].pSpectateLastTargetEdict) - 1);
		if ( pPlayerToSpectate->connected && pPlayerToSpectate->active ) {
			if ( plinfo[ENTINDEX(pPlayerToSpectate->edict)].bSpectating == FALSE ) {
				if ( (OurUserID != pPlayerToSpectate->userid) ) {
					plinfo[index].pSpectateTargetEdict = pPlayerToSpectate->edict;
					plinfo[index].pSpectateLastTargetEdict = pPlayerToSpectate->edict;
					if ( !plinfo[index].pCameraEdict ) {
						MakePlayerInvisible(pEntity);
						MakePlayerNonSolid(pEntity);
						CreateCamera(plinfo[index].pCameraEdict);
						plinfo[index].pCameraEdict->v.owner = pEntity;
						plinfo[index].pCameraEdict->v.origin = pPlayerToSpectate->edict->v.origin;
						//plinfo[index].pCameraEdict->v.aiment = pPlayerToSpectate->edict;
						char str[50];
						sprintf(str, "Spectating: %s", pPlayerToSpectate->name);
						CLIENT_PRINTF(pEntity, print_center, str);
						SET_VIEW(pEntity, plinfo[index].pCameraEdict);
						plinfo[index].bSpectating = true;
						return true;
					}
				}
			}
		}
	}
	//Last player was invalid, find a new one

	//Spectates the first valid index it finds
	for ( int i = 1; i <= gpGlobals->maxClients; i++ ) {
		client_t *pPlayerToSpectate = GetClientPointerFromIndex(i - 1);
		if ( pPlayerToSpectate->connected && pPlayerToSpectate->active ) {
			if ( plinfo[ENTINDEX(pPlayerToSpectate->edict)].bSpectating )
				continue;
			if ( (OurUserID != pPlayerToSpectate->userid) ) {
				plinfo[index].pSpectateTargetEdict = pPlayerToSpectate->edict;
				plinfo[index].pSpectateLastTargetEdict = pPlayerToSpectate->edict;
				if ( !plinfo[index].pCameraEdict ) {
					MakePlayerInvisible(pEntity);
					MakePlayerNonSolid(pEntity);
					CreateCamera(plinfo[index].pCameraEdict);
					plinfo[index].pCameraEdict->v.owner = pEntity;
					plinfo[index].pCameraEdict->v.origin = pPlayerToSpectate->edict->v.origin;
					char str[50];
					sprintf(str, "Spectating: %s", pPlayerToSpectate->name);
					CLIENT_PRINTF(pEntity, print_center, str);
					SET_VIEW(pEntity, plinfo[index].pCameraEdict);
					plinfo[index].bSpectating = true;
					return true;
				}
				break;
			}
		}
	}
	return false;
}

void EnableFreeLook(edict_t *pEntity) {
	StripWeapons(pEntity, 1);
	MakePlayerNonSolid(pEntity);
	MakePlayerInvisible(pEntity);
	plinfo[ENTINDEX(pEntity)].iSpectateMode = MODE_FREELOOK;
	CLIENT_PRINTF(pEntity, print_center, "Free Look Mode");
	MESSAGE_BEGIN(MSG_ONE, GET_USER_MSG_ID(PLID, "ToggleHud", NULL), 0, pEntity);
		WRITE_BYTE(0);
	MESSAGE_END();
}

void DisableFreeLook(edict_t *pEntity) {
	int index = ENTINDEX(pEntity);
	if ( !plinfo[index].bCodeEnabledSpectator ) {
		MakePlayerSolid(pEntity);
		MakePlayerVisible(pEntity);
	}
	MESSAGE_BEGIN(MSG_ONE, GET_USER_MSG_ID(PLID, "ToggleHud", NULL), 0, pEntity);
		WRITE_BYTE(1);
	MESSAGE_END();
	//pEntity->v.origin = plinfo[index].vecOriginBeforeSpectating;
}

void CreateCamera(edict_t *& pCamera) {
	//g_pFunctionTable->pfnAddToFullPack=AddToFullPack;
	//g_pFunctionTable_Post->pfnPlayerPostThink=PlayerPostThink_Post;

	pCamera = CREATE_NAMED_ENTITY(MAKE_STRING("cycler_sprite"));
	
	//pNewCamera->v.classname = MAKE_STRING("SpectatorCamera");

	//int modelindex = PRECACHE_MODEL("models/shell.mdl");
	SET_MODEL(pCamera, "models/player.mdl");
	SET_SIZE(pCamera, Vector(0, 0, 0), Vector(0, 0, 0));

	pCamera->v.movetype = MOVETYPE_NOCLIP;
	//pCamera->v.movetype = MOVETYPE_FOLLOW;
	pCamera->v.solid = SOLID_NOT;
	pCamera->v.takedamage = DAMAGE_NO;
	pCamera->v.gravity = 0;
	
	pCamera->v.rendermode = kRenderTransColor;
	pCamera->v.renderamt = 0;
	pCamera->v.renderfx = kRenderFxNone;
}

void DeleteCamera(edict_t *pEntity) {
	int index = ENTINDEX(pEntity);
	if ( plinfo[index].pCameraEdict ) {
		REMOVE_ENTITY(plinfo[index].pCameraEdict);
	}
	plinfo[index].pCameraEdict = NULL;
	plinfo[index].pSpectateTargetEdict = NULL;
	//plinfo[index].pSpectateLastTargetEdict = NULL;
	SET_VIEW(pEntity, pEntity);
}

void UpdateCamera(int index, edict_t *pEntity) {
	if ( plinfo[index].pCameraEdict && plinfo[index].pSpectateTargetEdict ) {
		edict_t *pCamera = plinfo[index].pCameraEdict;
		edict_t *pTargetPlayer = plinfo[index].pSpectateTargetEdict;
		client_t *pTargetClientPointer = GetClientPointerFromIndex(ENTINDEX(pTargetPlayer) - 1);

		if ( plinfo[ENTINDEX(pTargetPlayer)].bSpectating || !pTargetClientPointer->connected ) {
			DeleteCamera(pEntity);
			EnableFreeLook(pEntity);
			return;
		}
		if ( pTargetPlayer->v.health <= 0 )
			pEntity->v.health = 0.1f;
		else
			pEntity->v.health = pTargetPlayer->v.health;
		char message[200];
		
		if ( gpGlobals->teamplay == 1.0f ) {
			/*
			int actual = (int)ourptr + 2908;
			int wrong = (int)&ourptr->netchan.team;
			int difference = wrong - (int)ourptr;
			*/
			int targetteam = GetPlayerTeam((C_BasePlayer*)CBaseEntity::Instance(pTargetPlayer));
			//SetPlayerHudColor(pEntity, targetteam);
			//UpdatePlayerTeam(pEntity, "Spectators", 0);
			sprintf(message, "Spectating: %s\nHealth: %.1f\nTeam: %s", pTargetClientPointer->name, pTargetPlayer->v.health / 2, (targetteam == 1) ? "MI6" : "Phoenix" );
		} else {
			sprintf(message, "Spectating: %s\nHealth: %.1f", pTargetClientPointer->name, pTargetPlayer->v.health / 2  );
		}

		MESSAGE_BEGIN(MSG_ONE, GET_USER_MSG_ID(PLID, "HudText", NULL), 0, pEntity);
			WRITE_STRING( message );
		MESSAGE_END();

		if ( plinfo[index].iSpectateMode == MODE_THIRDPERSON ) {
			MAKE_VECTORS(pTargetPlayer->v.v_angle + pTargetPlayer->v.punchangle);
			Vector vecSrc	 = pTargetPlayer->v.origin + pTargetPlayer->v.view_ofs;
			Vector vecAiming = gpGlobals->v_forward;
			TraceResult tr;
			TRACE_LINE(vecSrc, vecSrc - (vecAiming * 72), 0, ignore_monsters, pTargetPlayer/*ENT(pTargetPlayer)*/, &tr);
			SET_MODEL(pCamera, "models/3rd_person.mdl");
			pCamera->v.origin = tr.vecEndPos;
			pCamera->v.angles = pTargetPlayer->v.v_angle;
			pCamera->v.movetype = MOVETYPE_NOCLIP;
			pCamera->v.aiment = NULL;
			pEntity->v.origin = pCamera->v.origin;
		} else {
			//FIRST PERSON
			//TRACE_LINE(vecSrc, vecSrc + (vecAiming * 20), 0, ignore_monsters, ENT(pTargetPlayer), &tr);
			//pCamera->v.origin = tr.vecEndPos;
			SET_MODEL(pCamera, "models/shell.mdl");
			pCamera->v.angles = pTargetPlayer->v.v_angle;
			pCamera->v.movetype = MOVETYPE_FOLLOW;
			pCamera->v.aiment = pTargetPlayer;
			pEntity->v.origin = pCamera->v.origin;
		}
	}
}

void CheckCameraButtons(int index, edict_t *pEntity) {
	client_t *cptr = GetClientPointerFromIndex(index - 1);

	edict_t *pCamera = plinfo[index].pCameraEdict;
	edict_t *pTargetPlayer = plinfo[index].pSpectateTargetEdict;

	int buttons = pEntity->v.button;
	if ( buttons & IN_JUMP && fabsf(gpGlobals->time - plinfo[index].flLastSpectateButtonPress) > 0.5f ) {
		//switch between free look and spectating the first player we find
		plinfo[index].flLastSpectateButtonPress = gpGlobals->time;
		if ( plinfo[index].iSpectateMode == MODE_FREELOOK ) {
			DisableFreeLook(pEntity);
			plinfo[index].iSpectateMode = MODE_THIRDPERSON;
			if ( !SpectateLastPlayer(pEntity) ) {
				EnableFreeLook(pEntity); //stay in free look mode if there's no players to spectate
			} else {
				CLIENT_PRINTF(pEntity, print_center, "Third Person Mode");
			}
		} else if ( plinfo[index].iSpectateMode == MODE_THIRDPERSON ) {
			plinfo[index].iSpectateMode = MODE_FIRSTPERSON;
			CLIENT_PRINTF(pEntity, print_center, "First Person Mode");
		} else {
			DeleteCamera(pEntity);
			EnableFreeLook(pEntity);
		}
	} else if ( buttons & IN_DUCK && fabsf(gpGlobals->time - plinfo[index].flLastSpectateButtonPress) > 0.5f ) {
		if ( !plinfo[index].bCodeEnabledSpectator ) {
			//disable spectate mode when pressing crouch and they haven't been forced into spectator mode
			plinfo[index].flLastSpectateButtonPress = gpGlobals->time;
			//CLIENT_COMMAND(pEntity, "togglehud\n"); //enable HUD
			C_BasePlayer *pPlayer = C_BasePlayer::Instance(pEntity);
			ChangePlayerTeamCTF(pPlayer, "Spectators", plinfo[index].iLastTeam /*GetTeamchoice(pPlayer)*/);// , true);
			ExitSpectate(pEntity);
			pEntity->v.origin = plinfo[index].vecOriginBeforeSpectating;
		}
	} 
	//if ( plinfo[index].pSpectateTargetEdict ) {
		if ( buttons & IN_ATTACK && fabsf(gpGlobals->time - plinfo[index].flLastSpectateButtonPress) > 0.5f ) {
			plinfo[index].flLastSpectateButtonPress = gpGlobals->time;
			if ( plinfo[index].iSpectateMode == MODE_FREELOOK ) {
				//If we are in free look and clicking the mouse, exit out of free look and start spectating
				DisableFreeLook(pEntity);
				plinfo[index].iSpectateMode = MODE_THIRDPERSON;
				//CLIENT_COMMAND(pEntity, "togglehud\n"); //disable HUD
				if ( !SpectateLastPlayer(pEntity) ) {
					DeleteCamera(pEntity);
					EnableFreeLook(pEntity);
				}
			} else {
				//cycle to next player
				if (!CycleToNextPlayer(pEntity, plinfo[index].pSpectateTargetEdict, index)) {
					DeleteCamera(pEntity);
					EnableFreeLook(pEntity);
				}
			}
		} else if ( buttons & IN_ATTACK2 && fabsf(gpGlobals->time - plinfo[index].flLastSpectateButtonPress) > 0.5f ) {
			plinfo[index].flLastSpectateButtonPress = gpGlobals->time;
			if ( plinfo[index].iSpectateMode == MODE_FREELOOK ) {
				//If we are in free look and clicking the mouse, exit out of free look and start spectating
				DisableFreeLook(pEntity);
				plinfo[index].iSpectateMode = MODE_THIRDPERSON;
				//CLIENT_COMMAND(pEntity, "togglehud\n"); //disable HUD
				if ( !SpectateLastPlayer(pEntity) ) {
					DeleteCamera(pEntity);
					EnableFreeLook(pEntity);
				}
			} else {
				//cycle to previous player
				if ( !CycleToPreviousPlayer(pEntity, plinfo[index].pSpectateTargetEdict, index) ) {
					DeleteCamera(pEntity);
					EnableFreeLook(pEntity);
				}
			}
		}
	//}
}

bool CycleToNextPlayer ( edict_t *pEntity, edict_t *pCurrentTargetPlayer, int ourindex ) {
	int indexoftarget = ENTINDEX(pCurrentTargetPlayer);
	for ( int i = 1; i <= gpGlobals->maxClients; i++ ) {
		indexoftarget++;
		if ( indexoftarget > gpGlobals->maxClients )
			indexoftarget = 1;

		if ( ourindex == indexoftarget)
			continue;

		client_t *NewTargetClient = GetClientPointerFromIndex(indexoftarget - 1);
		if ( NewTargetClient->connected && NewTargetClient->active ) {
			if ( plinfo[ENTINDEX(NewTargetClient->edict)].bSpectating )
				continue;
			plinfo[ourindex].pSpectateTargetEdict = NewTargetClient->edict;
			plinfo[ourindex].pSpectateLastTargetEdict = NewTargetClient->edict;
			char str[50];
			sprintf(str, "Spectating: %s", NewTargetClient->name);
			CLIENT_PRINTF(pEntity, print_center, str);
			return true;
		}
	}
	return false;
}

bool CycleToPreviousPlayer ( edict_t *pEntity, edict_t *pCurrentTargetPlayer, int ourindex ) {
	int indexoftarget = ENTINDEX(pCurrentTargetPlayer);
	for ( int i = 1; i <= gpGlobals->maxClients; i++ ) {
		indexoftarget--;
		if ( indexoftarget < 1 )
			indexoftarget = gpGlobals->maxClients;

		if ( ourindex == indexoftarget)
			continue;

		client_t *NewTargetClient = GetClientPointerFromIndex(indexoftarget - 1);
		if ( NewTargetClient->connected && NewTargetClient->active ) {
			if ( plinfo[ENTINDEX(NewTargetClient->edict)].bSpectating )
				continue;
			plinfo[ourindex].pSpectateTargetEdict = NewTargetClient->edict;
			plinfo[ourindex].pSpectateLastTargetEdict = NewTargetClient->edict;
			char str[50];
			sprintf(str, "Spectating: %s", NewTargetClient->name);
			CLIENT_PRINTF(pEntity, print_center, str);
			return true;
		}
	}
	return false;
}