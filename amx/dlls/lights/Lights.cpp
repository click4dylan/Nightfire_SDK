#include "Lights.h"
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
void OnModuleInitialize(void) {
	cvar_t_small setlights = {CVAR_STRING, "setlights_amt", "Sets light brightness a-z", "none", FCVAR_EXTDLL | FCVAR_SERVER };
	CVAR_REGISTER(setlights);
	REG_SVR_COMMAND("setlights", DoSetLights);
	//REG_SVR_COMMAND("hook", Hook);
	printf("SetLights Initialized\n");
}

void DoSetLights() {
	const char *value = CVAR_GET_STRING("setlights_amt");
	if ( !value || !strcmp(value, "") || !strcmpi(value, "none") ) {
		SetLights("", true);
	} else {
		SetLights(value);
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

void SetLights(const char *szLights, bool setdefault) { 
								  		//optional
	if (setdefault) {
		glinfo.bCheckLights = false;
		g_pFunctionTable_Post->pfnStartFrame = NULL;
		memset(glinfo.szLastLights, 0x0, 128);
		(g_engfuncs.pfnLightStyle)(0, (char *)glinfo.szRealLights);
	} else {
		g_pFunctionTable_Post->pfnStartFrame = StartFrame_Post;
		glinfo.bCheckLights = true;

		//Reset LastLights
		memset(glinfo.szLastLights, 0x0, 128);
		//Store the previous lighting.
		memcpy(glinfo.szLastLights, szLights, strlen(szLights));

		(g_engfuncs.pfnLightStyle)(0, szLights);

		// These make it so that players/weaponmodels look like whatever the lighting is
		// at. otherwise it would color players under the skybox to these values.
		SERVER_COMMAND("sv_skycolor_r 0\n");
		SERVER_COMMAND("sv_skycolor_g 0\n");
		SERVER_COMMAND("sv_skycolor_b 0\n");
	}
}