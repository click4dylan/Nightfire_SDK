#include "Dylans_Functions.h"

BOOL IsCTF(void) {
#ifdef MICROSOFT_SUCKS
	edict_t *flag = FIND_ENTITY_BY_CLASSNAME((edict_t*)0, "item_ctfflag");
	if ( !FNullEnt(flag)/*flag && flag->v.origin != Vector(0,0,0)*/ ) //origin is required because for some reason non-existing entities still return an edict
		return TRUE;
	return FALSE;
#else
	void *g_pGameRules = (void*)*(int*)*(int*)0x4217959C;
	BOOL *isCTFFunc = (BOOL*) *(int*)((int)g_pGameRules + 0x28);
	return ((BOOL(__stdcall*)(void))isCTFFunc)();
	/*
	__asm {
		mov ecx, dword ptr ds:[0x4217959C] //g_pGameRules
		mov ecx, dword ptr ds:[ecx]
		call dword ptr ds:[ecx + 0x28] //set's EAX to 1 or 0 depending on gamemode
	}
	*/
#endif
}



//-----------------------------------------------------------------------------
// Purpose: Fade an entity (player) screen to a specified color.
// Input  : *pEntity - Entity to tell to screen fade on.
// Input  : &color  - Color to fade to
// Input  : fadeTime - Time it takes to fade
// Input  : fadeHold - Time the fade holds for
// Input  : flags - Fade in, Fade Out, Fade_Modulate (don't blend), Fade_StayOut, Fade_Purge (Replaces others)
// Output : 
void UTIL_ScreenFade( edict_t *pEntity, byte r, byte g, byte b, byte a, float fadeTime, float fadeHold, int flags ) { //UTIL_ScreenFade(int playerid, short duration, short holdtime, short fadeflags, byte r, byte g, byte b, byte a) {
	int newflags = 0;
	client_t *pClient = EDICT_TO_CLIENT(pEntity);

	__asm {
		mov eax, flags
		xor edi, edi //set edi to 0
		test al, 0x01
		jnz one
		mov edi, 0x1
	one:
		test al, 0x02
		jz two
		or edi,0x00000003
	two:
		test al, 0x04
		mov newflags, edi
		jz fadeall
	}
	if ( !pClient->fakeclient )
		UTIL_ScreenFadeOne(pEntity, r, g, b, a, fadeTime, fadeHold, newflags);
	return;
	fadeall:
	UTIL_ScreenFadeAll(r, g, b, a, fadeTime, fadeHold, newflags);
}

void UTIL_ScreenFadeAll( byte r, byte g, byte b, byte a, float fadeTime, float fadeHold, int flags ) {
	int unknown, unknown2;
	MESSAGE_BEGIN(MSG_BROADCAST, GET_USER_MSG_ID(PLID, "ScreenFade", NULL));
		WRITE_COORD(fadeTime);
		WRITE_COORD(fadeHold);
		WRITE_BYTE(r);
		WRITE_BYTE(g);
		WRITE_BYTE(b);
		__asm {
			mov ebx, flags
			mov edi, ebx
			and edi, 0x00000001
			mov eax, 0x0
			setnz al
			dec eax
			and eax, 0x000000FF
			mov unknown, eax
			mov unknown2, edi
		}
		WRITE_BYTE(unknown);
		WRITE_BYTE(r);
		WRITE_BYTE(g);
		WRITE_BYTE(b);
		if ( unknown2 == 0 ) 
			a = 255 - a;
		WRITE_BYTE(a);
		__asm {
			mov ebx, flags //might be unnecessary, but keep it just in case
			shr ebx, 0x1
			not bl
			and ebx, 0x00000001
			mov unknown, ebx
		}
		WRITE_BYTE(unknown);
	MESSAGE_END();
}

void UTIL_ScreenFadeOne( edict_t *pEntity, byte r, byte g, byte b, byte a, float fadeTime, float fadeHold, int flags ) {
	int unknown, unknown2;
	MESSAGE_BEGIN(MSG_ONE_UNRELIABLE, GET_USER_MSG_ID(PLID, "ScreenFade", NULL), 0, pEntity);
		WRITE_COORD(fadeTime);
		WRITE_COORD(fadeHold);
		WRITE_BYTE(r);
		WRITE_BYTE(g);
		WRITE_BYTE(b);
		__asm {
			mov ebx, flags
			mov edi, ebx
			and edi, 0x00000001
			mov eax, 0x0
			setnz al
			dec eax
			and eax, 0x000000FF
			mov unknown, eax
			mov unknown2, edi
		}
		WRITE_BYTE(unknown);
		WRITE_BYTE(r);
		WRITE_BYTE(g);
		WRITE_BYTE(b);
		if ( unknown2 == 0 ) 
			a = 255 - a;
		WRITE_BYTE(a);
		__asm{
			mov ebx, flags //might be unnecessary, but keep it just in case
			shr ebx, 0x1
			not bl
			and ebx, 0x00000001
			mov unknown, ebx
		}
		WRITE_BYTE(unknown);
	MESSAGE_END();
}


//Noel's char functions
StringList::StringList(int Length = 0, int initialmemreserve = 5){
	strings = new std::vector<const char*>(Length);
	strings->reserve(initialmemreserve);
	editabletext = nullptr;
}

StringList::~StringList(){
	delete strings;
	if(editabletext != nullptr) delete[] editabletext;
}

StringList * SplitChar(const char * text, char delimiters[]){
	char * editabletext = new char[strlen(text) + 1]; // For Const Char Strings Passed To This Function
	strcpy(editabletext, text);
	char * token = strtok(editabletext, delimiters);
	if(token == nullptr){
		delete editabletext;
		return new StringList(0);
	}
	StringList * sl = new StringList(1);
	sl->editabletext = editabletext;
	sl->Set(0, token);
	while(69){
		token = strtok(0, delimiters);
		if(token == nullptr)
			break;
		sl->Add(token);
	}
	return sl;
}

__declspec (naked) C_BasePlayer* PLAYER_BY_INDEX(int index) {
	__asm {
		push dword ptr ss:[esp + 4]
		mov eax, 0x42059F10
		call eax
		add esp, 0x4
		retn
	}
}

/*
__declspec (naked) void UTIL_SET_ORIGIN(C_BaseEntity *entity, const Vector &vecOrigin, bool bFireTriggers) {
	//original function: 0x4306B690 but it has no check for bFireTriggers, actually i'm not sure if we can even fire triggers at all... this may be useless
	__asm {
		mov eax, dword ptr ss:[esp + 4]
		test eax, eax
		jz Exit //Invalid Entity, return
		MOV ECX,DWORD PTR SS:[ESP+8]
		MOV EDX,DWORD PTR DS:[ECX]
		MOV DWORD PTR DS:[EAX+0x128],EDX
		MOV EDX,DWORD PTR DS:[ECX+4]
		MOV DWORD PTR DS:[EAX+0x12C],EDX
		MOV ECX,DWORD PTR DS:[ECX+8]
		MOV DWORD PTR DS:[EAX+0x130],ECX
		cmp byte ptr ss:[esp + 16], 0
		jne Exit
			MOV DWORD PTR SS:[ESP+8],0
			MOV DWORD PTR SS:[ESP+4],EAX
			mov eax, 0x430994D0 //SV_LinkEdict
			jmp eax
		Exit:
		retn
	}
}
*/