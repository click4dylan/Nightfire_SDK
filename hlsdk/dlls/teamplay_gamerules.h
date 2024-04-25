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
//
// teamplay_gamerules.h
//

#define MAX_TEAMNAME_LENGTH	16
#define MAX_TEAMS			32

#define TEAMPLAY_TEAMLISTLENGTH		MAX_TEAMS*MAX_TEAMNAME_LENGTH

//=========================================================
// CBondTeamplay - rules for the basic half life multiplayer teamplay
// competition
//=========================================================
class CBondTeamplay : public CBondMultiplay
{
public:
	CBondTeamplay();

	// GR_Think
	virtual void Think(void);

	// Functions to verify the single/multiplayer status of a game
	virtual BOOL IsTeamplay(void) { return TRUE; };// is this deathmatch game being played with team rules?
	virtual BOOL IsCTF(void) { return FALSE; }
	virtual const char* GetGameDescription(void) { return "HL Teamplay"; }  // this is the game name that gets seen in the server browser

	// Client connection/disconnection
		// If ClientConnected returns FALSE, the connection is rejected and the user is provided the reason specified in
		//  svRejectReason
		// Only the client's name and remote address are provided to the dll for verification.
	virtual void InitHUD(CBasePlayer* pl);		// the client dll is ready for updating
	virtual void UpdateGameMode(CBasePlayer* pPlayer);  // the client needs to be informed of the current game mode

// Client damage rules
	virtual BOOL  FPlayerCanTakeDamage(CBasePlayer* pPlayer, CBaseEntity* pAttacker);
	virtual BOOL ShouldAutoAim(CBasePlayer* pPlayer, edict_t* target);

	// Client spawn/respawn control

	virtual BOOL ClientCommand(CBasePlayer* pPlayer, const char* pcmd, unsigned int numargs, const char** args);  // handles the user commands;  returns TRUE if command handled properly
	virtual void ClientUserInfoChanged(CBasePlayer* pPlayer, char* infobuffer);		// the player has changed userinfo;  can change it now

	// Client kills/scoring
	virtual int IPointsForKill(CBasePlayer* pAttacker, CBasePlayer* pKilled);
	virtual void PlayerKilled(CBasePlayer* pVictim, entvars_t* pKiller, entvars_t* pInflictor);
	virtual void DeathNotice(CBasePlayer* pVictim, entvars_t* pKiller, entvars_t* pInflictor);

	// Weapon retrieval

// Weapon spawn/respawn control

	// Item retrieval

	// Item spawn/respawn control

	// Ammo retrieval

	// Ammo spawn/respawn control

	// Healthcharger respawn control

	// What happens to a dead player's weapons

	// What happens to a dead player's ammo	

	// Teamplay stuff	
	virtual const char* GetTeamID(CBaseEntity* pEntity);
	virtual int PlayerRelationship(CBaseEntity* pPlayer, CBaseEntity* pTarget);
	virtual int GetTeamIndex(const char* pTeamName);
	virtual const char* GetIndexedTeamName(int teamIndex);
	virtual BOOL IsValidTeam(const char* pTeamName);
	virtual void ChangePlayerTeam(CBasePlayer* pPlayer, const char* pTeamName, BOOL bKill, BOOL bGib);
	virtual const char* SetDefaultPlayerTeam(CBasePlayer* pPlayer);

	virtual const char* TeamWithFewestPlayers();

private:
	void RecountTeams(bool bResendInfo = FALSE);

	bool_nightfire m_DisableDeathMessages;
	bool_nightfire m_DisableDeathPenalty;
	bool_nightfire m_teamLimit;				// This means the server set only some teams as valid
	char m_szTeamList[TEAMPLAY_TEAMLISTLENGTH]; //512
};