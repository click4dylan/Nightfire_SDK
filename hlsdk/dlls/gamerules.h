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
// GameRules
//=========================================================

//#include "weapons.h"
//#include "items.h"
class CBasePlayerItem;
class CBasePlayer;
class CItem;
class CBasePlayerAmmo;

// weapon respawning return codes
enum
{	
	GR_NONE = 0,
	
	GR_WEAPON_RESPAWN_YES,
	GR_WEAPON_RESPAWN_NO,
	
	GR_AMMO_RESPAWN_YES,
	GR_AMMO_RESPAWN_NO,
	
	GR_ITEM_RESPAWN_YES,
	GR_ITEM_RESPAWN_NO,

	GR_PLR_DROP_GUN_ALL,
	GR_PLR_DROP_GUN_ACTIVE,
	GR_PLR_DROP_GUN_NO,

	GR_PLR_DROP_AMMO_ALL,
	GR_PLR_DROP_AMMO_ACTIVE,
	GR_PLR_DROP_AMMO_NO,
};

// Player relationship return codes
enum
{
	GR_NOTTEAMMATE = 0,
	GR_TEAMMATE,
	GR_ENEMY,
	GR_ALLY,
	GR_NEUTRAL,
};

//TODO : place in respective files
#define TEAM_MI6 0
#define TEAM_PHOENIX 1
#define TEAM_COUNT 2
#define CHARACTER_SLOTS 20
#define MAX_NAME_SIZE 32

extern const char team_names[TEAM_COUNT][MAX_NAME_SIZE];

extern const char team_chars[TEAM_COUNT][CHARACTER_SLOTS][MAX_NAME_SIZE];

#if 0
__declspec(noinline) const char* GetCharacterType(int team, int charactermodel_slot) {
	if (charactermodel_slot >= CHARACTER_SLOTS)
		return "";  // Return empty string if charactermodel_slot is out of bounds

	return team_chars[team][charactermodel_slot];
}
#endif

extern const char* CTF_GetIndexedTeamName(int index);

class CGameRules
{
public:
	virtual void RefreshSkillData( void );// fill skill data struct with proper values
	virtual void Think( void ) = 0;// GR_Think - runs every server frame, should handle any timer tasks, periodic events, etc.
	virtual BOOL IsAllowedToSpawn( CBaseEntity *pEntity ) = 0;  // Can this item spawn (eg monsters don't spawn in deathmatch).

	virtual BOOL FAllowFlashlight( void ) = 0;// Are players allowed to switch on their flashlight?
	virtual BOOL FShouldSwitchWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pWeapon ) = 0;// should the player switch to this weapon?
	virtual BOOL GetNextBestWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pCurrentWeapon ) = 0;// I can't use this weapon anymore, get me the next best one.

// Functions to verify the single/multiplayer status of a game
	virtual BOOL IsMultiplayer( void ) = 0;// is this a multiplayer game? (either coop or deathmatch)
	virtual BOOL IsDeathmatch( void ) = 0;//is this a deathmatch game?
	virtual BOOL IsTeamplay( void ) { return FALSE; };// is this deathmatch game being played with team rules?
	virtual BOOL IsCoOp( void ) = 0;// is this a coop game?
	virtual BOOL IsCTF(void) = 0;// is this a coop game?
	virtual const char *GetGameDescription( void ) { return "Vega"; }  // this is the game name that gets seen in the server browser
	
// Client connection/disconnection
	virtual BOOL ClientConnected( edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[ 128 ] ) = 0;// a client just connected to the server (player hasn't spawned yet)
	virtual void InitHUD( CBasePlayer *pl ) = 0;		// the client dll is ready for updating
	virtual void ClientDisconnected( edict_t *pClient ) = 0;// a client just disconnected from the server
	virtual void UpdateGameMode( CBasePlayer *pPlayer ) {}  // the client needs to be informed of the current game mode

// Client damage rules
	virtual float FlPlayerFallDamage( CBasePlayer *pPlayer ) = 0;// this client just hit the ground after a fall. How much damage?
	virtual BOOL  FPlayerCanTakeDamage( CBasePlayer *pPlayer, CBaseEntity *pAttacker ) {return TRUE;};// can this player take damage from this attacker?
	virtual BOOL ShouldAutoAim( CBasePlayer *pPlayer, edict_t *target ) { return TRUE; }

// Client spawn/respawn control
	virtual void PlayerSpawn( CBasePlayer *pPlayer ) = 0;// called by CBasePlayer::Spawn just before releasing player into the game
	virtual void PlayerThink( CBasePlayer *pPlayer ) = 0; // called by CBasePlayer::PreThink every frame, before physics are run and after keys are accepted
	virtual BOOL FPlayerCanRespawn( CBasePlayer *pPlayer ) = 0;// is this player allowed to respawn now?
	virtual float FlPlayerSpawnTime( CBasePlayer *pPlayer ) = 0;// When in the future will this player be able to spawn?
	virtual edict_t *GetPlayerSpawnSpot( CBasePlayer *pPlayer );// Place this player on their spawnspot and face them the proper direction.

	virtual BOOL AllowAutoTargetCrosshair( void ) { return TRUE; };
	virtual BOOL ClientCommand( CBasePlayer *pPlayer, const char *pcmd, unsigned int numargs, const char** args ) { return FALSE; };  // handles the user commands;  returns TRUE if command handled properly
	virtual void ClientUserInfoChanged( CBasePlayer *pPlayer, char *infobuffer ) {}		// the player has changed userinfo;  can change it now

// Client kills/scoring
	virtual int IPointsForKill( CBasePlayer *pAttacker, CBasePlayer *pKilled ) = 0;// how many points do I award whoever kills this player?
	virtual void PlayerKilled( CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor ) = 0;// Called each time a player dies
	virtual void PlayerFailed(CBasePlayer* pVictim) { };
	virtual void CharacterKilled(CBaseCharacter* pVictim, entvars_t* pKiller) { };
	virtual void DeathNotice( CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor )=  0;// Call this from within a GameRules class to report an obituary.
	virtual void ShotFired(CBasePlayer* pAttacker) { };
	virtual void PlayerTakeDamage(CBasePlayer* pMessageTarget, entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) { };
// Weapon retrieval
	virtual BOOL CanHavePlayerItem( CBasePlayer *pPlayer, CBasePlayerItem *pWeapon );// The player is touching an CBasePlayerItem, do I give it to him?
	virtual void PlayerGotWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pWeapon ) = 0;// Called each time a player picks up a weapon from the ground

// Weapon spawn/respawn control
	virtual int WeaponShouldRespawn( CBasePlayerItem *pWeapon ) = 0;// should this weapon respawn?
	virtual float FlWeaponRespawnTime( CBasePlayerItem *pWeapon ) = 0;// when may this weapon respawn?
	virtual float FlWeaponTryRespawn( CBasePlayerItem *pWeapon ) = 0; // can i respawn now,  and if not, when should i try again?
	virtual Vector VecWeaponRespawnSpot( CBasePlayerItem *pWeapon ) = 0;// where in the world should this weapon respawn?

// Item retrieval
	virtual BOOL CanHaveItem( CBasePlayer *pPlayer, CItem *pItem ) = 0;// is this player allowed to take this item?
	virtual void PlayerGotItem( CBasePlayer *pPlayer, CItem *pItem ) = 0;// call each time a player picks up an item (battery, healthkit, longjump)

// Item spawn/respawn control
	virtual int ItemShouldRespawn( CItem *pItem ) = 0;// Should this item respawn?
	virtual float FlItemRespawnTime( CItem *pItem ) = 0;// when may this item respawn?
	virtual Vector VecItemRespawnSpot( CItem *pItem ) = 0;// where in the world should this item respawn?

// Ammo retrieval
	virtual BOOL CanHaveAmmo( CBasePlayer *pPlayer, const char *pszAmmoName, int iMaxCarry );// can this player take more of this ammo?
	virtual void PlayerGotAmmo( CBasePlayer *pPlayer, char *szName, int iCount ) = 0;// called each time a player picks up some ammo in the world

// Ammo spawn/respawn control
	virtual int AmmoShouldRespawn( CBasePlayerAmmo *pAmmo ) = 0;// should this ammo item respawn?
	virtual float FlAmmoRespawnTime( CBasePlayerAmmo *pAmmo ) = 0;// when should this ammo item respawn?
	virtual Vector VecAmmoRespawnSpot( CBasePlayerAmmo *pAmmo ) = 0;// where in the world should this ammo item respawn?
																			// by default, everything spawns

// Healthcharger respawn control
	virtual float FlHealthChargerRechargeTime( void ) = 0;// how long until a depleted HealthCharger recharges itself?
	virtual float FlHEVChargerRechargeTime( void ) { return 0; }// how long until a depleted HealthCharger recharges itself?

// What happens to a dead player's weapons
	virtual int DeadPlayerWeapons( CBasePlayer *pPlayer ) = 0;// what do I do with a player's weapons when he's killed?

// What happens to a dead player's ammo	
	virtual int DeadPlayerAmmo( CBasePlayer *pPlayer ) = 0;// Do I drop ammo when the player dies? How much?

// Teamplay stuff
	virtual const char *GetTeamID( CBaseEntity *pEntity ) = 0;// what team is this entity on?
	virtual int PlayerRelationship( CBaseEntity *pPlayer, CBaseEntity *pTarget ) = 0;// What is the player's relationship with this entity?
	virtual int GetTeamIndex( const char *pTeamName ) { return -1; }
	virtual const char *GetIndexedTeamName( int teamIndex ) { return ""; }
	virtual BOOL IsValidTeam( const char *pTeamName ) { return TRUE; }
	virtual void ChangePlayerTeam( CBasePlayer *pPlayer, const char *pTeamName, BOOL bKill, BOOL bGib ) {}
	virtual const char *SetDefaultPlayerTeam( CBasePlayer *pPlayer ) { return ""; }

	virtual const char* GetCharacterType(int team, int charactermodel_slot) { return ""; }

	virtual int GetNumTeams() { return 0; }
	virtual const char* TeamWithFewestPlayers() { return 0; }
	virtual BOOL TeamsBalanced() { return TRUE; }

// Sounds
	virtual BOOL PlayTextureSounds( void ) { return TRUE; }
	virtual BOOL PlayFootstepSounds( CBasePlayer *pl, float fvol ) { return TRUE; }

// Monsters
	virtual BOOL FAllowCharacters( void ) = 0;//are monsters allowed

	// Immediately end a multiplayer game
	virtual void EndMultiplayerGame( void ) {}
};

extern CGameRules *InstallGameRules( void );


//=========================================================
// CBondRules - rules for the single player Half-Life 
// game.
//=========================================================
class CBondRules : public CGameRules
{
public:
	CBondRules ( void );

// GR_Think
	virtual void Think( void );
	virtual BOOL IsAllowedToSpawn( CBaseEntity *pEntity );
	virtual BOOL FAllowFlashlight( void );

	virtual BOOL FShouldSwitchWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pWeapon );
	virtual BOOL GetNextBestWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pCurrentWeapon );
	
// Functions to verify the single/multiplayer status of a game
	virtual BOOL IsMultiplayer( void );
	virtual BOOL IsDeathmatch( void );
	virtual BOOL IsCoOp( void );
	virtual BOOL IsCTF(void) { return FALSE; }

// Client connection/disconnection
	virtual BOOL ClientConnected( edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[ 128 ] );
	virtual void InitHUD( CBasePlayer *pl );		// the client dll is ready for updating
	virtual void ClientDisconnected( edict_t *pClient );
	virtual void UpdateGameMode(CBasePlayer* pPlayer);

// Client damage rules
	virtual float FlPlayerFallDamage( CBasePlayer *pPlayer );
	
// Client spawn/respawn control
	virtual void PlayerSpawn( CBasePlayer *pPlayer );
	virtual void PlayerThink( CBasePlayer *pPlayer );
	virtual BOOL FPlayerCanRespawn( CBasePlayer *pPlayer );
	virtual float FlPlayerSpawnTime( CBasePlayer *pPlayer );

	virtual BOOL AllowAutoTargetCrosshair( void );

// Client kills/scoring
	virtual int IPointsForKill( CBasePlayer *pAttacker, CBasePlayer *pKilled );
	virtual void PlayerKilled( CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor );
	virtual void PlayerFailed(CBasePlayer* pVictim);
	virtual void CharacterKilled(CBaseCharacter* pVictim, entvars_t* pKiller);
	virtual void DeathNotice( CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor );
	virtual void ShotFired(CBasePlayer* pAttacker);
	virtual void PlayerTakeDamage(CBasePlayer* pMessageTarget, entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);

// Weapon retrieval
	virtual void PlayerGotWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pWeapon );

// Weapon spawn/respawn control
	virtual int WeaponShouldRespawn( CBasePlayerItem *pWeapon );
	virtual float FlWeaponRespawnTime( CBasePlayerItem *pWeapon );
	virtual float FlWeaponTryRespawn( CBasePlayerItem *pWeapon );
	virtual Vector VecWeaponRespawnSpot( CBasePlayerItem *pWeapon );

// Item retrieval
	virtual BOOL CanHaveItem( CBasePlayer *pPlayer, CItem *pItem );
	virtual void PlayerGotItem( CBasePlayer *pPlayer, CItem *pItem );

// Item spawn/respawn control
	virtual int ItemShouldRespawn( CItem *pItem );
	virtual float FlItemRespawnTime( CItem *pItem );
	virtual Vector VecItemRespawnSpot( CItem *pItem );

// Ammo retrieval
	virtual void PlayerGotAmmo( CBasePlayer *pPlayer, char *szName, int iCount );

// Ammo spawn/respawn control
	virtual int AmmoShouldRespawn( CBasePlayerAmmo *pAmmo );
	virtual float FlAmmoRespawnTime( CBasePlayerAmmo *pAmmo );
	virtual Vector VecAmmoRespawnSpot( CBasePlayerAmmo *pAmmo );

// Healthcharger respawn control
	virtual float FlHealthChargerRechargeTime( void );
	virtual float FlHEVChargerRechargeTime(void); // how long until a depleted HealthCharger recharges itself?

// What happens to a dead player's weapons
	virtual int DeadPlayerWeapons( CBasePlayer *pPlayer );

// What happens to a dead player's ammo	
	virtual int DeadPlayerAmmo( CBasePlayer *pPlayer );

// Teamplay stuff	
	virtual const char* GetTeamID(CBaseEntity* pEntity);
	virtual int PlayerRelationship( CBaseEntity *pPlayer, CBaseEntity *pTarget );

// Sounds

// Monsters
	virtual BOOL FAllowCharacters(void);
};

//=========================================================
// CBondMultiplay - rules for the basic half life multiplayer
// competition
//=========================================================
class CBondMultiplay : public CGameRules
{
public:
	CBondMultiplay();

// GR_Think
	virtual void Think( void );
	virtual void RefreshSkillData( void );
	virtual BOOL IsAllowedToSpawn( CBaseEntity *pEntity );
	virtual BOOL FAllowFlashlight( void );

	virtual BOOL FShouldSwitchWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pWeapon );
	virtual BOOL GetNextBestWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pCurrentWeapon );

// Functions to verify the single/multiplayer status of a game
	virtual BOOL IsMultiplayer( void );
	virtual BOOL IsDeathmatch( void );
	virtual BOOL IsCoOp( void );
	virtual BOOL IsCTF(void) { return FALSE; };

// Client connection/disconnection
	// If ClientConnected returns FALSE, the connection is rejected and the user is provided the reason specified in
	//  svRejectReason
	// Only the client's name and remote address are provided to the dll for verification.
	virtual BOOL ClientConnected( edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[ 128 ] );
	virtual void InitHUD( CBasePlayer *pl );		// the client dll is ready for updating
	virtual void ClientDisconnected( edict_t *pClient );
	virtual void UpdateGameMode( CBasePlayer *pPlayer );  // the client needs to be informed of the current game mode

// Client damage rules
	virtual float FlPlayerFallDamage( CBasePlayer *pPlayer );
	virtual BOOL  FPlayerCanTakeDamage( CBasePlayer *pPlayer, CBaseEntity *pAttacker );

// Client spawn/respawn control
	virtual void PlayerSpawn( CBasePlayer *pPlayer );
	virtual void PlayerThink( CBasePlayer *pPlayer );
	virtual BOOL FPlayerCanRespawn( CBasePlayer *pPlayer );
	virtual float FlPlayerSpawnTime( CBasePlayer *pPlayer );
	virtual edict_t *GetPlayerSpawnSpot( CBasePlayer *pPlayer );

	virtual BOOL AllowAutoTargetCrosshair( void );
	virtual BOOL ClientCommand(CBasePlayer* pPlayer, const char* pcmd, unsigned int numargs, const char** args);  // handles the user commands;  returns TRUE if command handled properly
	virtual void ClientUserInfoChanged(CBasePlayer* pPlayer, char* infobuffer);		// the player has changed userinfo;  can change it now

// Client kills/scoring
	virtual int IPointsForKill( CBasePlayer *pAttacker, CBasePlayer *pKilled );
	virtual void PlayerKilled( CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor );
	virtual void DeathNotice( CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor );

// Weapon retrieval
	virtual BOOL CanHavePlayerItem( CBasePlayer *pPlayer, CBasePlayerItem *pWeapon );// The player is touching an CBasePlayerItem, do I give it to him?
	virtual void PlayerGotWeapon(CBasePlayer* pPlayer, CBasePlayerItem* pWeapon);

// Weapon spawn/respawn control
	virtual int WeaponShouldRespawn( CBasePlayerItem *pWeapon );
	virtual float FlWeaponRespawnTime( CBasePlayerItem *pWeapon );
	virtual float FlWeaponTryRespawn( CBasePlayerItem *pWeapon );
	virtual Vector VecWeaponRespawnSpot( CBasePlayerItem *pWeapon );

// Item retrieval
	virtual BOOL CanHaveItem( CBasePlayer *pPlayer, CItem *pItem );
	virtual void PlayerGotItem( CBasePlayer *pPlayer, CItem *pItem );

// Item spawn/respawn control
	virtual int ItemShouldRespawn( CItem *pItem );
	virtual float FlItemRespawnTime( CItem *pItem );
	virtual Vector VecItemRespawnSpot( CItem *pItem );

// Ammo retrieval
	virtual void PlayerGotAmmo( CBasePlayer *pPlayer, char *szName, int iCount );

// Ammo spawn/respawn control
	virtual int AmmoShouldRespawn( CBasePlayerAmmo *pAmmo );
	virtual float FlAmmoRespawnTime( CBasePlayerAmmo *pAmmo );
	virtual Vector VecAmmoRespawnSpot( CBasePlayerAmmo *pAmmo );

// Healthcharger respawn control
	virtual float FlHealthChargerRechargeTime( void );
	virtual float FlHEVChargerRechargeTime( void );

// What happens to a dead player's weapons
	virtual int DeadPlayerWeapons( CBasePlayer *pPlayer );

// What happens to a dead player's ammo	
	virtual int DeadPlayerAmmo( CBasePlayer *pPlayer );

// Teamplay stuff	
	virtual const char *GetTeamID( CBaseEntity *pEntity ) { return ""; }
	virtual int PlayerRelationship( CBaseEntity *pPlayer, CBaseEntity *pTarget );
	virtual const char* SetDefaultPlayerTeam(CBasePlayer* pPlayer);

	virtual BOOL PlayTextureSounds( void ) { return FALSE; }
	virtual BOOL PlayFootstepSounds( CBasePlayer *pl, float fvol );

// Monsters
	virtual BOOL FAllowCharacters( void );

	// Immediately end a multiplayer game
	virtual void EndMultiplayerGame( void ) { GoToIntermission(); }

protected:
	virtual void ChangeLevel( void );
	virtual void GoToIntermission( void );

	float m_flIntermissionStartTime = 0;
	float m_flIntermissionEndTime = 0;
	bool_nightfire m_iEndIntermissionButtonHit;
	void SendMOTDToClient( edict_t *client );
	void BotGameRulesThink();
};


//=========================================================
// CBondMultiplayCTF - rules for the basic half life multiplayer ctf
// competition
//=========================================================
class CBondCTFPlay : public CBondMultiplay
{
private:
	enum class StatsPhase
	{
		Nothing,
		SendTeam0,
		SendTeam1,
		SendTeam2,
		SendPlayers,
		OpenMenu
	};

public:
	CBondCTFPlay();

	// GR_Think
	virtual void Think(void);

	// Functions to verify the single/multiplayer status of a game
	virtual BOOL IsTeamplay(void) { return TRUE; }
	virtual BOOL IsCTF(void) { return TRUE; }
	virtual const char* GetGameDescription(void) { return "Bond CTF"; }  // this is the game name that gets seen in the server browser

	// Client connection/disconnection
		// If ClientConnected returns FALSE, the connection is rejected and the user is provided the reason specified in
		//  svRejectReason
		// Only the client's name and remote address are provided to the dll for verification.
	virtual BOOL ClientConnected(edict_t* pEntity, const char* pszName, const char* pszAddress, char szRejectReason[128]);
	virtual void InitHUD(CBasePlayer* pl);		// the client dll is ready for updating
	virtual void ClientDisconnected(edict_t* pClient);
	virtual void UpdateGameMode(CBasePlayer* pPlayer);  // the client needs to be informed of the current game mode

// Client damage rules
	virtual BOOL  FPlayerCanTakeDamage(CBasePlayer* pPlayer, CBaseEntity* pAttacker);

	// Client spawn/respawn control
	virtual void PlayerSpawn(CBasePlayer* pPlayer);

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
	virtual BOOL CanHaveAmmo(CBasePlayer* pPlayer, const char* pszAmmoName, int iMaxCarry);// can this player take more of this ammo?

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

	virtual const char* GetCharacterType(int team, int charactermodel_slot);

	virtual int GetNumTeams();
	virtual const char* TeamWithFewestPlayers();
	virtual BOOL TeamsBalanced();

protected:
	virtual void GoToIntermission(void);

private:

	void SendTeamStatInfo(int team);
	void SendPlayerStatInfo(CBasePlayer* pPlayer);

	void RecountTeams(bool bResendInfo = FALSE);

	const char* getTeamChoice(CBasePlayer* pPlayer);
	void setPlayerToTeam(CBasePlayer* pPlayer, const char* teamname, int teamchoice);
	void notifyTeamChange(CBasePlayer* pPlayer, const char* teamname, const char* character_model_name);

	bool_nightfire m_DisableDeathMessages = false;
	bool_nightfire m_DisableDeathPenalty = false;
	bool_nightfire m_fRefreshScores = false;
	float m_flNextStatsSend;
	StatsPhase m_iStatsPhase = StatsPhase::Nothing;
	//Use a sane default to avoid lockups
	int m_iStatsPlayer = 1;
};

extern DLL_GLOBAL CGameRules*	g_pGameRules;
