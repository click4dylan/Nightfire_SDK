#include "VTHook.h"
#include "dedicated.h"
#include "enginecallback.h"

typedef struct client_static_s
{
	cactive_t state;
} client_static_t;

static client_static_t  *g_pcls = (client_static_t*)0x431191B0;

VTHook* DirectX = nullptr;

typedef HRESULT(__stdcall* TestCooperativeLevelFn)(void*);
TestCooperativeLevelFn oTestCooperativeLevel;

//#define D3DERR_DEVICELOST 0x88760868
//#define D3DERR_DEVICENOTRESET 0x88760869
#define D3D_OK 0x0

#define _FACD3D  0x876
#define MAKE_D3DHRESULT( code )  MAKE_HRESULT( 1, _FACD3D, code )

/*
* Direct3D Errors
*/

#define D3DERR_WRONGTEXTUREFORMAT               MAKE_D3DHRESULT(2072)
#define D3DERR_UNSUPPORTEDCOLOROPERATION        MAKE_D3DHRESULT(2073)
#define D3DERR_UNSUPPORTEDCOLORARG              MAKE_D3DHRESULT(2074)
#define D3DERR_UNSUPPORTEDALPHAOPERATION        MAKE_D3DHRESULT(2075)
#define D3DERR_UNSUPPORTEDALPHAARG              MAKE_D3DHRESULT(2076)
#define D3DERR_TOOMANYOPERATIONS                MAKE_D3DHRESULT(2077)
#define D3DERR_CONFLICTINGTEXTUREFILTER         MAKE_D3DHRESULT(2078)
#define D3DERR_UNSUPPORTEDFACTORVALUE           MAKE_D3DHRESULT(2079)
#define D3DERR_CONFLICTINGRENDERSTATE           MAKE_D3DHRESULT(2081)
#define D3DERR_UNSUPPORTEDTEXTUREFILTER         MAKE_D3DHRESULT(2082)
#define D3DERR_CONFLICTINGTEXTUREPALETTE        MAKE_D3DHRESULT(2086)
#define D3DERR_DRIVERINTERNALERROR              MAKE_D3DHRESULT(2087)

#define D3DERR_NOTFOUND                         MAKE_D3DHRESULT(2150)
#define D3DERR_MOREDATA                         MAKE_D3DHRESULT(2151)
#define D3DERR_DEVICELOST                       MAKE_D3DHRESULT(2152)
#define D3DERR_DEVICENOTRESET                   MAKE_D3DHRESULT(2153)
#define D3DERR_NOTAVAILABLE                     MAKE_D3DHRESULT(2154)
#define D3DERR_OUTOFVIDEOMEMORY                 MAKE_D3DHRESULT(380)
#define D3DERR_INVALIDDEVICE                    MAKE_D3DHRESULT(2155)
#define D3DERR_INVALIDCALL                      MAKE_D3DHRESULT(2156)
#define D3DERR_DRIVERINVALIDCALL MAKE_D3DHRESULT(2157)

HMODULE NVD3DUM;



HRESULT __stdcall Hooked_TestCooperativeLevel(void* pDevice)
{
	HRESULT result = oTestCooperativeLevel(pDevice);
	
	static bool bDeviceWasInvalid = false;

	if (!result)
	{
		//Device is OK
		static HRESULT forcestate = D3D_OK;
		register cactive_t state = g_pcls->state;

		if (state == ca_active)
		{
			//If the device was invalid already, then don't bother resetting again
			if (bDeviceWasInvalid)
			{
				bDeviceWasInvalid = false;
				forcestate = D3D_OK;
				return D3D_OK;
			}

			switch (forcestate)
			{
				case D3D_OK:
					return D3D_OK;
				case D3DERR_DEVICELOST:
					forcestate = D3DERR_DEVICENOTRESET;
					return D3DERR_DEVICELOST;
				case D3DERR_DEVICENOTRESET:
					forcestate = D3D_OK;
					return D3DERR_DEVICENOTRESET;
			}
		}
		else if (state == ca_connecting || state == ca_uninitialized)
		{
			forcestate = D3DERR_DEVICELOST;
		}
	}
	else
	{
		//Device is INVALID
		bDeviceWasInvalid = true;
	}
	return result;
}

void HookFuncs(bool ENABLE_NVIDIA_FIX)
{
#ifndef SWDS
	static bool Hooked_DirectX = false;
	if (!g_bDedicated && !Hooked_DirectX)
	{
		if (g_hRenderDLL && render)
		{
			NVD3DUM = GetModuleHandleA("nvd3dum.dll");
			if (NVD3DUM)
			{
				if (ENABLE_NVIDIA_FIX)
				{
					DWORD dx_ptr;
					__asm {
						MOV EAX, render
						MOV ECX, DWORD PTR DS : [EAX + 4]
						MOV EDX, DWORD PTR DS : [ECX + 4]
						LEA ECX, [EAX + EDX + 4]

						SUB ECX, DWORD PTR DS : [ECX - 4]
						SUB ECX, 0x340

						MOV EAX, DWORD PTR DS : [ECX - 0x176C]

						MOV EAX, DWORD PTR DS : [EAX + 0x1834] //DirectX pointer
						//MOV ECX, DWORD PTR DS:[EAX] //DX

						mov dx_ptr, EAX
					}
					DirectX = new VTHook((DWORD**)dx_ptr);

					oTestCooperativeLevel = (TestCooperativeLevelFn)DirectX->HookFunction((DWORD)Hooked_TestCooperativeLevel, 3);
				}
			}
			Hooked_DirectX = true;
		}
	}
#endif
}