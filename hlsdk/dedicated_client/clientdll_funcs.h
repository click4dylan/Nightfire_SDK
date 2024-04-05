#pragma once
//client.dll hardcodes..

#define g_FIELDOFVIEW *(int*)0x410A8ABC
#define m_flMouseSensitivity *(float*)0x410A8AD0
#define gViewPort (*(GenericViewport**)0x410A8AA0)
#define gHUD 0x410A8AA8

//CBasePanel = CHudBase

//https://github.com/alliedmodders/hlsdk/blob/a0edb7792a96998d349325bebab8ea41ec5cb239/cl_dll/hud.h#L66
class CHud
{

};

class CRocketOverlayPanel
{
public:
	void TurnOff()
	{
		reinterpret_cast<void(__thiscall*)(CRocketOverlayPanel*)>(0x41043790)(this);
	}
};

class CCrosshairOverlayPanel
{
public:
	void setCrosshair(int value)
	{
		reinterpret_cast<void(__thiscall*)(CCrosshairOverlayPanel*, int value)>(0x4103E5B0)(this, value);
	}
};

class CCrosshairPanel
{
public:
	void TurnOn()
	{
		reinterpret_cast<void(__thiscall*)(CCrosshairPanel*)>(0x4103EBD0)(this);
	}
};

struct GenericViewport
{
	int basepanel;
	int visible;
	int parent;
	int temp4;
	int temp5;
	int temp6;
	int temp7;
	int hud_is_on;
	int ammohudpanel;
	int healthhudpanel;
	int weaponselectionhudpanel;
	CCrosshairPanel* crosshairpanel;
	int visionoverlaypanel;
	int movementmodepanel;
	int stingerpanel;
	int fadeoverlaypanel;
	CCrosshairOverlayPanel* crosshairoverlaypanel;
	CRocketOverlayPanel* rocketoverlaypanel;
	int spacesuitoverlaypanel;
	int scubaoverlaypanel;
	int objectivepanel;
	int objectiveoverviewpanel;
	int multiplayerscoreboard;
	int playerhudid;
	int progresspanel;
	int gadgetbatterypanel;
	int mphudpanel;
	int mpchatpanel;
	int singleplayerscoreboard;
	int explosivehud;
	int savingoverlaypanel;
	int hudiconpanel;
};

void (*BEGIN_READ)(void* buf, int size) = (void (*)(void*, int))0x410387F0;
int (*READ_BYTE)() = (int (*)())0x41038850;
