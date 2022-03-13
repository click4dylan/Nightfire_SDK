#include <Windows.h>
#include "globals.h"
#include "pattern_scanner.h"

DWORD GUI_GetAction_JmpBack;
__declspec(naked) void GUI_GetAction_Return()
{
	__asm
	{
		test eax, eax
		jz isnullptr
		mov edx, [eax]
		push ebx
		mov ecx, eax
		call [edx + 8]
		isnullptr:
		jmp GUI_GetAction_JmpBack
	}
}

// this fix is not ideal as audio files will still leak if they are changed during runtime,
// but this will allow us to have more than 768 sounds, and stops the CGBAudio::m_registerSoundFX array being overflowed all the time
// also fixes crash when reconnecting to servers multiple times
//
	DWORD loadSound_JmpBack;
	__declspec(naked) void loadSound_CrashFix()
	{
		__asm
		{
			mov eax, [esp + 4]
			test eax, eax
			jz isnullptr
			sub esp, 0x354
			jmp loadSound_JmpBack
			isnullptr :
			xor eax, eax
				ret 8
		}
	}
	void Fix_Sound_Overflow()
	{
		//0x4307EFF6
		DWORD find_name = FindMemoryPattern(g_engineDllHinst, "7E 27 8B F7 8D 45 48 8B D6 90", false);
		if (find_name)
		{
			// prevent CGBAudio::findName from reusing old slot when AudioAPIGetSoundID did not find any crc matching loaded sound filenames
			PlaceShort((BYTE*)find_name, 0x27EB);

			//0x430813D0
			DWORD constructor = FindMemoryPattern(g_engineDllHinst, "68 00 FC 00 00 89 1D", false);
			if (constructor)
			{
				// increase max audio files limit in CGBAudio::findName from 768 to 4096
				PlaceInt((BYTE*)(find_name + 0x2B), 4096);
				PlaceInt((BYTE*)(find_name + 0x32), 4096);

				// increase allocation size for audio files in CGBAudio::CGBAudio()
				// note: nightfire alpha demo struct size is 80 bytes
				PlaceInt((BYTE*)(constructor + 1), 4096 * 84); //alloc
				PlaceInt((BYTE*)(constructor + 0x14), 4096 * 84); //memset
			}

		}

		//0x4307ED40
		DWORD end_precaching = FindMemoryPattern(g_engineDllHinst, "C6 41 05 00 A1 ?? ?? ?? ?? 57 33 FF 85 C0", false);
		if (end_precaching)
		{
			// prevent CGBAudio::endPrecaching from unloading audio files when servercount changes
			PlaceShort((BYTE*)(end_precaching + 0xE), 0x69EB);
		}

		//0x43080190
		DWORD load_sound = FindMemoryPattern(g_engineDllHinst, "81 EC 54 03 00 00 53", false);
		if (load_sound)
		{
			// stop CGBAudio::loadSound from dereferencing nullptr on first argument due to CGBAudio::findName returning nullptr
			//PlaceJMP((BYTE*)load_sound, (DWORD)&loadSound_CrashFix, 5);
			loadSound_JmpBack = load_sound + 6;
		}
	}
//

void Fix_Engine_Bugs()
{
	Fix_Sound_Overflow();
}

// apply nullptr check from mac version
void Fix_GUI_GetAction_Crash()
{
	if (!g_guiDllHinst)
		return;

	DWORD adr = FindMemoryPattern(g_guiDllHinst, "8B 10 53 8B C8 FF 52 08", false);
	if (!adr)
		return;

	PlaceJMP((BYTE*)adr, (DWORD)&GUI_GetAction_Return, 5);
	GUI_GetAction_JmpBack = adr + 8;
}