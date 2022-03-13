#ifndef DYLANSHOOKS_H
#define DYLANSHOOKS_H

#define MAX_HOOKS 128

struct hook_t {
	int numhooks;
	void* hooks[MAX_HOOKS];
};

/*
typedef const int HOOK_TYPE;

namespace ADD_HOOK_TYPES {
	HOOK_TYPE H_FIREBULLETS = (sizeof(hook_t*) * 0);
	HOOK_TYPE H_HUDINIT = (sizeof(hook_t*) * 1);
	HOOK_TYPE H_PLAYERKILLED = (sizeof(hook_t*) * 2);
	HOOK_TYPE H_ENTITYINIT = (sizeof(hook_t*) * 3);
	HOOK_TYPE H_BASEPLAYERSPAWN = (sizeof(hook_t*) * 4);
	HOOK_TYPE H_BASEPLAYERTAKEDAMAGE = (sizeof(hook_t*) * 5);
	HOOK_TYPE H_SPAWNBLOOD = (sizeof(hook_t*) * 6);
}
*/

enum ADD_HOOK_TYPES {
	H_FIREBULLETS = 0,
	H_HUDINIT,
	H_PLAYERKILLED,
	H_ENTITYINIT,
	H_BASEPLAYERSPAWN,
	H_BASEPLAYERTAKEDAMAGE,
	H_SPAWNBLOOD,
	H_FINDENTITYINSPHERE,
	H_FPLAYERCANRESPAWN//,
	//H_PM_INIT,
	//H_PM_MOVE
};

typedef BOOLEAN(__cdecl *ADD_HOOK_FUNC)(void*, int);

//returns true on hook success
//example: ADD_DLL_HOOK(&callOnGetEntityInit, ADD_HOOK_TYPES::H_ENTITYINIT);
inline BOOLEAN ADD_DLL_HOOK(void* address, ADD_HOOK_TYPES hooktype) {
	HMODULE hMod = GetModuleHandle("required_dll_hooks_amxx.dll");
	if (!hMod) {
		printf("ERROR: required_dll_hooks_amxx.dll was not loaded!\n");
		return FALSE;
	}
	ADD_HOOK_FUNC ADD_HOOK = (ADD_HOOK_FUNC)GetProcAddress(hMod, "ADD_HOOK");
	if (!ADD_HOOK) {
		printf("ERROR: Could not find ADD_HOOK in required_dll_hooks_amxx.dll!\n");
		return FALSE;
	}
	return ADD_HOOK(address, hooktype);
}

#endif