#include "voting.h"

cvar_t* sv_enablemapvoting = nullptr;
cvar_t* sv_votetime = nullptr;

enum VOTE_TYPES
{
	VOTE_MAP = 0,
};

bool bVoteInProgress = false;
VOTE_TYPES iVoteType = VOTE_MAP;
float flVoteEndTime = 0.0f;
float flNextVoteReminder = 0.0f;
float flChangeLevelTime = 0.0f;
int iVoteCount = 0;
edict_t *pEdictStartedVote = nullptr;
#define MAX_MAP_STRING_LENGTH 72
char strMap[MAX_MAP_STRING_LENGTH];
unsigned int WONIDsVoted[33] = {0};
unsigned int NumWONIDsVoted = 0;
unsigned int WONIDInstances[33] = {0};
unsigned int NumWONIDs = 0;

void ClearVotes()
{
	for (int i = 0; i < 33; i++)
	{
		plinfo[i].bVoted = false;
	}
	memset(WONIDsVoted, 0, sizeof(WONIDsVoted) * 33);
	NumWONIDsVoted = 0;
}

void StopVote()
{
	ClearVotes();
	bVoteInProgress = false;
	flVoteEndTime = 0.0f;
	flNextVoteReminder = 0.0f;
	flChangeLevelTime = 0.0f;
	iVoteCount = 0;
	pEdictStartedVote = nullptr;
}

void OnModuleInitialize(void) {
	static bool regged = false;
	if (!regged)
	{
		cvar_t_small votetime = {CVAR_FLOAT, "sv_votetime", "none", "30.0", FCVAR_EXTDLL | FCVAR_SERVER};
		cvar_t_small enablemapvoting = {CVAR_BOOL, "sv_enablemapvoting", "none", "1", FCVAR_EXTDLL | FCVAR_SERVER };
		sv_enablemapvoting = (cvar_t*)CVAR_REGISTER(enablemapvoting);
		sv_votetime = (cvar_t*)CVAR_REGISTER(votetime);
		printf("Map Voting Initialized\n");
	}
	NumWONIDs = 0;
	StopVote();
}

bool AlreadyHasInstance(unsigned int WONID)
{
	for (int i = 0; i < NumWONIDs; i++)
	{
		if (WONIDInstances[i] == WONID)
			return true;
	}
	return false;
}

int GetNumPlayers(bool IncludeBots, bool AllowMultipleInstances)
{
	int numplayers = 0;
	NumWONIDs = 0;
	for (int i = 0; i < gpGlobals->maxClients; i++) 
	{
		client_t *pClient = GetClientPointerFromIndex(i);
		if (pClient->active && pClient->connected && pClient->spawned && (!pClient->fakeclient || IncludeBots))
		{
			unsigned int WONID = g_engfuncs.pfnGetPlayerWONId(pClient->edict);
			if (!AlreadyHasInstance(WONID))
			{
				WONIDInstances[NumWONIDs++] = WONID;
				numplayers++;
			}
			else if (AllowMultipleInstances)
			{
				numplayers++;
			}
		}
	}
	return numplayers;
}

int GetNumVotesRequired()
{
	int numplayers = GetNumPlayers(false, false);
	
	if (numplayers == 1)
		return 1;
	if (numplayers == 2)
		return 2;
	
	return (int)ceilf((float)numplayers * 0.5f);
}

void OnVoteFailed()
{
	char msg[128];
	sprintf(msg, "Vote failed, not enough votes! (%i/%i)", iVoteCount, GetNumVotesRequired());
	SayTextAll(msg);
	StopVote();
}

void CheckChangeLevelTime()
{
	if (flChangeLevelTime != 0.0f)
	{
		if (gpGlobals->time >= flChangeLevelTime)
		{
			flChangeLevelTime = 0.0f;
			g_engfuncs.pfnChangeLevel(strMap, "");
		}
	}
}

void OnVoteReminder()
{
	char msg[512];
	char tmp[256];
	sprintf(msg, "Type #vote if you wish to vote\n");

	sprintf(tmp, "Time remaining: %.1f seconds\n", flVoteEndTime - gpGlobals->time);
	strcat(msg, tmp);

	switch (iVoteType)
	{
		case VOTE_MAP:
			sprintf(tmp, "Map: %s\n", strMap); 
			strcat(msg, tmp);
	}

	sprintf(tmp, "Vote In Progress (Votes: %i/%i)\n", iVoteCount, GetNumVotesRequired());
	strcat(msg, tmp);

	SayTextAll(msg);
}

void OnStartFrame()
{
	switch (iVoteType)
	{
		case VOTE_MAP:
			CheckChangeLevelTime();	
			break;
	}

	if (!bVoteInProgress)
		return;

	if (gpGlobals->time >= flVoteEndTime)
		OnVoteFailed();
	else if (gpGlobals->time >= flNextVoteReminder)
	{
		OnVoteReminder();
		flNextVoteReminder = gpGlobals->time + 10.0f;
	}
}

void OnVoteSucceeded()
{
	char msg[512];
	char tmp[256];

	sprintf(msg, "Vote succeeded! (%i/%i)\n", iVoteCount, GetNumVotesRequired());
	StopVote();
			
	switch (iVoteType)
	{
		case VOTE_MAP:
		sprintf(tmp, "Changing level to %s in 4 seconds", strMap);
		strcat(msg, tmp);
		flChangeLevelTime = gpGlobals->time + 4.0f;
		break;
	}

	SayTextAll(msg);
}

void OnVoteStarted()
{
	char msg[512];
	char tmp[256];
	sprintf(msg, "Votes Required: %i\nType #vote to vote or #cancel to cancel\nVote ends in %.1f seconds\n", GetNumVotesRequired(), sv_votetime->GetFloat());

	switch (iVoteType)
	{
		case VOTE_MAP:
		sprintf(tmp, "Map Vote Started: %s", strMap);
		strcat(msg, tmp);
		break;
	}

	SayTextAll(msg);

	flNextVoteReminder = gpGlobals->time + 10.0f;
}

void OnChangeLevelReceived(edict_t *pEntity, int argl, const char **str)
{
	if (bVoteInProgress)
	{
		SayTextAll("Changelevel failed: vote already in progress");
		return;
	}

	if (argl == 0)
	{
		SayTextAll("Syntax: #changelevel ctf_mapname");
		return;
	}

	char mapstring[512];
	mapstring[0] = 0x0;
	int i = 0;
	while (i < argl)
	{
		const char *newstr = str[i];
		strcat(mapstring, newstr);
		if (++i < argl)
			strcat(mapstring, " ");
	}

	if (strlen(mapstring) > MAX_MAP_STRING_LENGTH)
	{
		SayTextAll("Error: length of map name too long!");
		return;
	}
	
	if (!g_engfuncs.pfnIsMapValid(mapstring))
	{
		char err[512];
		sprintf(err, "Error: %s not found on server", mapstring);
		SayTextAll(err);
		return;
	}

	//Set globals
	iVoteType = VOTE_TYPES::VOTE_MAP;
	bVoteInProgress = true;
	flVoteEndTime = gpGlobals->time + sv_votetime->GetFloat();
	strcpy(strMap, mapstring);
	pEdictStartedVote = pEntity;
	iVoteCount = 1;

	if (GetNumPlayers(false, false) == 1)
	{
		OnVoteSucceeded();
		return;
	}

	WONIDsVoted[NumWONIDsVoted++] = g_engfuncs.pfnGetPlayerWONId(pEntity);
	plinfo[ENTINDEX(pEntity)].bVoted = true;

	OnVoteStarted();
}

void OnVoteCancelReceived(edict_t *pEntity)
{
	if (!bVoteInProgress)
	{
		SayTextAll("Error: no vote in progress!");
		return;
	}

	if (pEdictStartedVote != pEntity)
	{
		SayTextAll("Denied: you didn't start the vote!");
		return;
	}

	SayTextAll("Vote cancelled!");
	StopVote();
}

void OnVoteReceived(edict_t *pEntity)
{
	char msg[512];

	if (!bVoteInProgress)
	{
		SayTextAll("Error: no vote in progress!");
		return;
	}

	int index = ENTINDEX(pEntity);
	if (plinfo[index].bVoted)
	{
		SayTextAll("Denied: you already voted!");
		return;
	}

	unsigned int WONID = g_engfuncs.pfnGetPlayerWONId(pEntity);
	for (int i = 0; i < NumWONIDsVoted; i++)
	{
		if (WONIDsVoted[i] == WONID)
		{
			SayTextAll("Denied: vote disqualified!");
			return;
		}
	}

	WONIDsVoted[NumWONIDsVoted++] = WONID;
	plinfo[index].bVoted = true;
	iVoteCount++;
	int iNumVotesRequired = GetNumVotesRequired();
	if (iVoteCount < iNumVotesRequired)
	{
		sprintf(msg, "Vote counted (%i/%i)", iVoteCount, iNumVotesRequired);
		SayTextAll(msg);
		return;
	}

	OnVoteSucceeded();
}

void OnClientCommand(edict_t *pEntity, int argl, const char **str)
{
	//argl is 1 extra than the real value with ClientCommand/subtract by 1 to get actual value
	argl--;

	if (argl > 1)
	{
		if (!strcmpi(str[0], "say"))
		{
			//filter prefixes
			if (str[2][0] == '#' || str[2][0] == '!')
			{
				char *stringnoprefix = (char*)(str[2] + 1);
				if (!strcmpi(stringnoprefix, "changelevel") || !strcmpi(stringnoprefix, "map"))
				{
					OnChangeLevelReceived(pEntity, argl - 2, &str[3]);
				}
				else if (strstr(stringnoprefix, "cancel"))
				{
					OnVoteCancelReceived(pEntity);
				}
				else if (!strcmpi(stringnoprefix, "vote"))
				{
					OnVoteReceived(pEntity);
				}
			}
		}
	}
}

void ConsoleCommand(edict_t *pEntity, int argl) {
	/*
	if ( argl > 2 ) { 
		SetLights(CMD_ARGV(1));
	} else if ( glinfo.bCheckLights ) {
		SetLights("", true);
	}
	*/
}