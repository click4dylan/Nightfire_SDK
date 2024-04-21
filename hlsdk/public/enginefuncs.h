#pragma once

typedef struct enginefuncs_s
{
	class ConsoleVariable* (*pfnCreateConsoleVariableGame) (IConsoleVariable pCvar);
	qboolean	(*pfnDestroyConsoleVariableGame)  (class IConsoleVariable*pCvar);
	class ConsoleVariable* (*pfnGetConsoleVariableGame) (const char *szVarName);
	void		(*pfnRegisterGame)			(struct IConsoleFunction* pFunction);
	void		(*pfnUnregisterGame)		(struct IConsoleFunction* pFunction);
	int			(*pfnPrecacheModel)			(const char* s);
	int			(*pfnPrecacheSound)			(const char* s);
	int			(*pfnPrecacheGeneric)		(const char* s); //moved
	void		(*pfnSetModel)				(struct edict_t *e, const char *m);
	int			(*pfnModelIndex)			(const char *m);
	int			(*pfnModelFrames)			(int modelIndex);
	void		(*pfnSetSize)				(struct edict_t *e, const float *rgflMin, const float *rgflMax);
	void		(*pfnChangeLevel)			(const char* s1, const char* s2);
	void		(*pfnGetSpawnParms)			(struct edict_t *ent);
	void		(*pfnSaveSpawnParms)		(struct edict_t *ent);
	float		(*pfnVecToYaw)				(const float *rgflVector);
	void		(*pfnVecToAngles)			(const float *rgflVectorIn, float *rgflVectorOut);
	void		(*pfnMoveToOrigin)			(struct edict_t *ent, const float *pflGoal, float dist, int iMoveType);
	void		(*pfnChangeYaw)				(struct edict_t* ent);
	void		(*pfnChangePitch)			(struct edict_t* ent);
	edict_t*	(*pfnFindEntityByString)	(struct edict_t *pEdictStartSearchAfter, const char *pszField, const char *pszValue);
	int			(*pfnGetEntityIllum)		(struct edict_t* pEnt, bool unknown);
	int			(*pfnGetPointIllum)			(const float* rgflVector); //added, used in CTempEnts::Sprite_Spray
	edict_t*	(*pfnFindEntityInSphere)	(struct edict_t *pEdictStartSearchAfter, const float* org, float rad);
	edict_t*	(*pfnFindClientInPVS)		(struct edict_t *pEdict);
	edict_t* 	(*pfnEntitiesInPVS)			(struct edict_t *pplayer);
	void		(*pfnMakeVectors)			(const float *rgflVector);
	void		(*pfnAngleVectors)			(const float *rgflVector, float *forward, float *right, float *up);
	edict_t*	(*pfnCreateEntity)			(void);
	void		(*pfnRemoveEntity)			(struct edict_t* e);
	edict_t*	(*pfnCreateNamedEntity)		(int className);
	void		(*pfnMakeStatic)			(struct edict_t *ent);
	int			(*pfnCheckBottom)			(struct edict_t *pEdict); //added
	int			(*pfnDropToFloor)			(struct edict_t* e);
	int			(*pfnWalkMove)				(struct edict_t *ent, float yaw, float dist, int iMode);
	void		(*pfnSetOrigin)				(struct edict_t *e, const float *rgflOrigin);
	void		(*pfnEmitSound)				(struct edict_t *entity, int channel, const char *sample, /*int*/float volume, float attenuation, int fFlags, int pitch);
	void		(*pfnEmitAmbientSound)		(struct edict_t *entity, float *pos, const char *samp, float vol, float attenuation, int fFlags, int pitch);
	void		(*pfnTraceLine)				(const float *v1, const float *v2, int fNoMonsters, int brushflags, edict_t *pentToSkip, struct TraceResult *ptr);
	void		(*pfnTraceToss)				(struct edict_t* pent, edict_t* pentToIgnore, struct TraceResult *ptr);
	int			(*pfnTraceCharacterHull)	(struct edict_t *pEdict, const float *v1, const float *v2, int fNoMonsters, edict_t *pentToSkip, struct TraceResult *ptr);
	void		(*pfnTraceHull)				(const float *v1, const float *v2, int fNoMonsters, int hullNumber, int brushflags, edict_t *pentToSkip, struct TraceResult *ptr);
	void		(*pfnTraceModel)			(const float *v1, const float *v2, int hullNumber, int brushflags, edict_t *pent, struct TraceResult *ptr);
	const char *(*pfnTraceTexture)			(struct edict_t *pTextureEntity, const float *v1, const float *v2 );
	void		(*pfnGetAimVector)			(struct edict_t* ent, float speed, float *rgflReturn);
	void		(*pfnServerCommand)			(const char* str);
	void		(*pfnServerExecute)			(void);
	void		(*pfnClientCommand)			(struct edict_t* pEdict, const char* szFmt, ...);
	void		(*pfnParticleEffect)		(const float *org, const float *dir, float red, float green, float blue, float count);
	void		(*pfnLightStyle)			(int style, const char* val);
	int			(*pfnAllocateDecal)			(const char *name); //added
	int			(*pfnDecalIndex)			(const char *name);
	int			(*pfnPointContents)			(const float *rgflVector, int brushflags);
	BOOLEAN		(*pfnWaterCheck)			(const float *pos);
	void		(*pfnMessageBegin)			(int msg_dest, int msg_type, const float *pOrigin, edict_t *ed);
	void		(*pfnMessageEnd)			(void);
	void		(*pfnWriteByte)				(int iValue);
	void		(*pfnWriteChar)				(int iValue);
	void		(*pfnWriteShort)			(int iValue);
	void		(*pfnWriteLong)				(int iValue);
	void		(*pfnWriteAngle)			(float flValue);
	void		(*pfnWriteCoord)			(float flValue);
	void		(*pfnWriteString)			(const char *sz);
	void		(*pfnWriteEntity)			(int iValue);
	void		(*pfnAlertMessage)			(enum ALERT_TYPE atype, const char *szFmt, ...);
	void		(*pfnEngineFprintf)			(void *pfile, const char *szFmt, ...);
	void*		(*pfnPvAllocEntPrivateData)	(struct edict_t *pEdict, int cb);
	void*		(*pfnPvEntPrivateData)		(struct edict_t *pEdict);
	void		(*pfnFreeEntPrivateData)	(struct edict_t *pEdict);
	const char*	(*pfnSzFromIndex)			(int iString);
	int			(*pfnAllocString)			(const char *szValue);
	struct entvars_s*	(*pfnGetVarsOfEnt)			(struct edict_t *pEdict);
	struct edict_t*	(*pfnPEntityOfEntOffset)	(int iEntOffset);
	int			(*pfnEntOffsetOfPEntity)	(const edict_t *pEdict);
	int			(*pfnIndexOfEdict)			(const edict_t *pEdict);
	struct edict_t*	(*pfnPEntityOfEntIndex)		(int iEntIndex);
	struct edict_t*	(*pfnFindEntityByVars)		(struct entvars_s* pvars);
	void*		(*pfnGetModelPtr)			(struct edict_t* pEdict);
	int			(*pfnRegUserMsg)			(const char *pszName, int iSize);
	void		(*pfnUnregUserMsgs)			(void);
	void		(*pfnGetBonePosition)		(const edict_t* pEdict, int iBone, float *rgflOrigin, float *rgflAngles );
	BOOLEAN		(*pfnGetBoneCoord)			(struct edict_t *ed, const char *bone, float *rgflOrigin, float *rgflAngles);
	unsigned int 		(*pfnFunctionFromName)		( const char *pName );
	const char *(*pfnNameForFunction)		( unsigned int  function );
	void		(*pfnClientPrintf)			( struct edict_t* pEdict, enum PRINT_TYPE ptype, const char *szMsg ); // JOHN: engine callbacks so game DLL can print messages to individual clients
	void		(*pfnServerPrint)			( const char *szMsg );
	void		(*pfnGetAttachment)			(const edict_t *pEdict, int iAttachment, float *rgflOrigin, float *rgflAngles );
	void		(*pfnCRC32_Init)			(struct CRC32_t *pulCRC);
	void        (*pfnCRC32_ProcessBuffer)   (struct CRC32_t *pulCRC, void *p, int len);
	void		(*pfnCRC32_ProcessByte)     (struct CRC32_t *pulCRC, unsigned char ch);
	CRC32_t		(*pfnCRC32_Final)			(struct CRC32_t pulCRC);
	int		(*pfnRandomLong)			(int  lLow,  int  lHigh);
	float		(*pfnRandomFloat)			(float flLow, float flHigh);
	void		(*pfnSetView)				(const edict_t *pClient, const edict_t *pViewent );
	float		(*pfnTime)					( void );
	void		(*pfnCrosshairAngle)		(const edict_t *pClient, float pitch, float yaw);
	void		(*pfnCOM_Log)         (char *pszFile, char *fmt, ...); //added
	const char *(*pfnCOM_FileBase)			(const char *in, char *out, unsigned int size);
	byte *      (*pfnCOM_LoadHeapFile)         (const char *filename, int *pLength);
	const char* (*pfnCOM_ParseFile)          (const char *file, char *adr, unsigned int length);
	char*       (*pfnCOM_Token)              (void); //added
	void        (*pfnCOM_FreeFile)           (void *buffer);
	void        (*pfnEndSection)            (const char *pszSectionName); // trigger_endsection
	int 		(*pfnCompareFileTime)       (char *filename1, char *filename2, int *iCompare);
	void        (*pfnGetGameDir)            (char *szGetGameDir, unsigned int length);
	void        (*pfnFadeClientVolume)      (const edict_t *pEdict, int fadePercent, int fadeOutSeconds, int holdTime, int fadeInSeconds);
	void        (*pfnSetClientMaxspeed)     (const edict_t *pEdict, float fNewMaxspeed);
	edict_t *	(*pfnCreateFakeClient)		(const char *netname);	// returns NULL if fake client can't be created
	void		(*pfnRunPlayerMove)			(struct edict_t *fakeclient, const float *viewangles, float forwardmove, float sidemove, float upmove, unsigned short buttons, byte impulse, byte msec );
	int			(*pfnNumberOfEntities)		(void);
	char*		(*pfnGetInfoKeyBuffer)		(struct edict_t *e);	// passing in NULL gets the serverinfo
	char*		(*pfnInfoKeyValue)			(char *infobuffer, const char *key);
	void		(*pfnSetKeyValue)			(char *infobuffer, const char *key, const char *value);
	void		(*pfnSetClientKeyValue)		(int clientIndex, char *infobuffer, const char *key, const char *value);
	int			(*pfnIsMapValid)			(const char *filename);
	void		(*pfnStaticDecal)			( const float *origin, int decalIndex, int entityIndex, int modelIndex );
	int			(*pfnGetPlayerUserId)		(struct edict_t *e ); // returns the server assigned userid for this player.  useful for logging frags, etc.  returns -1 if the edict couldn't be found in the list of clients
	void		(*pfnBuildSoundMsg)			(struct edict_t *entity, int channel, const char *sample, /*int*/float volume, float attenuation, int fFlags, int pitch, int msg_dest, int msg_type, const float *pOrigin, edict_t *ed);
	int			(*pfnIsDedicatedServer)		(void);// is this a dedicated server?
	unsigned int (*pfnGetPlayerWONId)		(struct edict_t *e); // returns the server assigned WONid for this player.  useful for logging frags, etc.  returns -1 if the edict couldn't be found in the list of clients

	// YWB 8/1/99 TFF Physics additions
	void		(*pfnInfo_RemoveKey)		( char *s, const char *key );
	const char *(*pfnGetPhysicsKeyValue)	( const edict_t *pClient, const char *key );
	void		(*pfnSetPhysicsKeyValue)	( const edict_t *pClient, const char *key, const char *value );
	const char *(*pfnGetPhysicsInfoString)	( const edict_t *pClient );
	unsigned short (*pfnPrecacheEvent)		( int type, const char* psz );
	void		(*pfnPlaybackEvent)			( int flags, const edict_t *pInvoker, unsigned short eventindex, float delay, float *origin, float *angles, float fparam1, float fparam2, int iparam1, int iparam2, int bparam1, int bparam2 );

	unsigned char *(*pfnSetFatPVS)			( float *org );
	unsigned char *(*pfnSetFatPAS)			( float *org );

	BOOLEAN			(*pfnCheckVisibility )		( const edict_t *entity, unsigned char *pset );

	void		(*pfnDeltaSetField)			( struct delta_s *pFields, const char *fieldname );
	void		(*pfnDeltaUnsetField)		( struct delta_s *pFields, const char *fieldname );
	void		(*pfnDeltaAddEncoder)		( const char *name, void (*conditionalencode)( struct delta_s *pFields, const unsigned char *from, const unsigned char *to ) );
	int			(*pfnGetCurrentPlayer)		( void );
	int			(*pfnCanSkipPlayer)			( const edict_t *player );
	int			(*pfnDeltaFindField)		( struct delta_s *pFields, const char *fieldname );
	void		(*pfnDeltaSetFieldByIndex)	( struct delta_s *pFields, int fieldNumber );
	void		(*pfnDeltaUnsetFieldByIndex)( struct delta_s *pFields, int fieldNumber );

	void		(*pfnSetGroupMask)			( int mask, int op );

	int			(*pfnCreateInstancedBaseline) ( int classname, struct entity_state_s *baseline );

	// Forces the client and server to be running with the same version of the specified file
	//  ( e.g., a player model ).
	// Calling this has no effect in single player
	void		(*pfnForceUnmodified)	( enum FORCE_TYPE type, float *mins, float *maxs, const char *filename );

	void		(*pfnGetPlayerStats)	( const edict_t *pClient, int *ping, int *packet_loss );
	
	void		(*pfnPM_FindSafeSpot)	( struct edict_t* pClient );
	int			(*pfnIndexForSurface)	( const msurface_t* pSurface );
	const msurface_t* (*pfnSurfaceForIndex)	( int index ); //added
	const msurface_t* (*pfnSV_ClosestSurfaceToPoint)		(struct edict_t const* pClient, float const* pPoint);
} enginefuncs_t;