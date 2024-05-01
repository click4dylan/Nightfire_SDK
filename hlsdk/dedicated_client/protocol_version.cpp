#include "pattern_scanner.h"
#include "MinHook/MinHook.h"
#include <event_api.h>
#include "MetaHook.h"
#include <usercmd.h>
#include <cvardef.h>
#include <clientsideentity.h>
#include <NightfireFileSystem.h>
//#include <r_studioint.h>

//#include <eiface.h>
#include <com_model.h>
#include <pmtrace.h>
#include "bzip2/bzlib.h"
#include <sizebuf.h>
#include <fragbuf.h>
#include <netchan.h>
#include <platformdll.h>
//#include <enginefuncs.h>
#include <globalvars.h>
#include <server.h>
#include <nightfire_pointers.h>
#include "nightfire_hooks.h"

//#include <pm_defs.h>
#include "fixes.h"
#include "globals.h"

// update protocol version number so old clients will get a proper translated connection rejection response from the server
void UpdateEngineProtocolVersion()
{
	DWORD CL_SendConnectPacket;
	if (!FindMemoryPattern(pattern_t(CL_SendConnectPacket, g_engineDllHinst, "6A ? 68 FF 00 00 00 68 FF 00 00 00 68 FF 00 00 00 68 FF 00 00 00 68 ? ? ? ? 89", false, "CL_SendConnectPacket", false)))
		return;

	DWORD SV_CheckProtocol;
	if (!FindMemoryPattern(pattern_t(SV_CheckProtocol, g_engineDllHinst, "57 8B 7C 24 08 85 FF 75 11", false, "SV_CheckProtocol", false)))
		return;

	DWORD Host_Version;
	if (!FindMemoryPattern(pattern_t(Host_Version, g_engineDllHinst, "81 EC ? ? ? ? 56 6A 20 68", false, "Host_Version", false)))
		return;

	DWORD CL_ParseServerInfo;
	if (!FindMemoryPattern(pattern_t(CL_ParseServerInfo, g_engineDllHinst, "81 EC ? ? ? ? 56 57 68", false, "CL_ParseServerInfo", false)))
		return;

	DWORD SV_SendServerInfo;
	if (!FindMemoryPattern(pattern_t(SV_SendServerInfo, g_engineDllHinst, "81 EC ? ? ? ? 56 57 B9 ? ? ? ? ", false, "SV_SendServerInfo", false)))
		return;

	DWORD SVC_InfoString;
	if (!FindMemoryPattern(pattern_t(SVC_InfoString, g_engineDllHinst, "81 EC ? ? ? ? 8D 84 24 ? ? ? ? 55", false, "SVC_InfoString", false)))
		return;

	PushProtection(CL_SendConnectPacket);
	*(unsigned char*)(CL_SendConnectPacket + 1) = PROTOCOL_VERSION;
	PopProtection();

	PushProtection(SV_CheckProtocol);
	*(unsigned char*)(SV_CheckProtocol + 0x21) = PROTOCOL_VERSION;
	*(unsigned char*)(SV_CheckProtocol + 0x4F) = PROTOCOL_VERSION;
	*(unsigned char*)(SV_CheckProtocol + 0x52) = PROTOCOL_VERSION;
	*(unsigned char*)(SV_CheckProtocol + 0x67) = PROTOCOL_VERSION;
	PopProtection();

	PushProtection(Host_Version);
	*(unsigned char*)(Host_Version + 0x108) = PROTOCOL_VERSION;
	PopProtection();

	PushProtection(CL_ParseServerInfo);
	*(unsigned char*)(CL_ParseServerInfo + 0x53) = PROTOCOL_VERSION;
	*(unsigned char*)(CL_ParseServerInfo + 0x57) = PROTOCOL_VERSION;
	PopProtection();

	PushProtection(SV_SendServerInfo);
	*(unsigned char*)(SV_SendServerInfo + 0x6E) = PROTOCOL_VERSION;
	PopProtection();

	PushProtection(SVC_InfoString);
	*(unsigned char*)(SVC_InfoString + 0x103) = PROTOCOL_VERSION;
	PopProtection();
}