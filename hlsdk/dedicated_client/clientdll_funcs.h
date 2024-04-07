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

class CBasePanel
{
public:
	virtual void setVisible(bool visible);
	virtual bool isVisible();
	virtual void Draw();
	virtual void handleResChange();
	virtual void doDraw();
	virtual ~CBasePanel();

	//todo: member variables
	bool is_visible;
};

class CGUIPanel : public CBasePanel
{
public:
	virtual void setVisible(bool visible);
	virtual bool isVisible();
	virtual void doDraw();
	virtual ~CGUIPanel();

	//todo: member variables and functions
};

class CAmmoCountPanel : public CBasePanel
{
public:
	virtual void handleResChange();
	virtual void doDraw();
	virtual ~CAmmoCountPanel();
	virtual void setAmmoAmt(int amount);
	virtual void setAmmoTag();
	virtual float getAmmoWidth();

	//todo: member variables and functions
};

class CAmmoHudPanel : public CBasePanel
{
public:
	virtual void handleResChange();
	virtual void doDraw();
	virtual ~CAmmoHudPanel();

	//todo: member variables and functions
};

class CAmmoImagePanel : public CBasePanel
{
public:
	virtual void handleResChange();
	virtual void doDraw();
	virtual ~CAmmoImagePanel();
	virtual void setAmmoAmt(int ammo);
	virtual float getAmmoWidth();
	virtual void setBulletsPerRow(int bullets_per_row);

	//todo: member variables and functions
};

class CBatteryAmmoPanel : public CBasePanel
{
public:
	virtual void handleResChange();
	virtual void doDraw();
	virtual ~CBatteryAmmoPanel();
	virtual void setAmmoAmt(int amount);
	virtual void setBulletsPerRow(int bullets_per_row);

	//todo: member variables and functions
};

class CClipAmmoPanel : public CBasePanel
{
public:
	virtual void handleResChange();
	virtual void doDraw();
	virtual ~CClipAmmoPanel();
	virtual void setAmmoAmt(int amount);
	virtual float getAmmoWidth();
	virtual void setBulletsPerRow(int bullets_per_row);

	//todo: member variables and functions
};

class CCrosshairOverlayPanel : public CBasePanel
{
public:
	virtual void handleResChange();
	virtual void doDraw();
	virtual ~CCrosshairOverlayPanel();

	//todo: member variables and functions

	void setCrosshair(int value)
	{
		reinterpret_cast<void(__thiscall*)(CCrosshairOverlayPanel*, int value)>(0x4103E5B0)(this, value);
	}
};

class CCrosshairPanel : public CBasePanel
{
public:
	virtual void handleResChange();
	virtual void doDraw();
	virtual ~CCrosshairPanel();

	//todo: member variables and functions

	void TurnOn()
	{
		reinterpret_cast<void(__thiscall*)(CCrosshairPanel*)>(0x4103EBD0)(this);
	}
};

class CCTFHudPanel : public CBasePanel
{
public:
	virtual void handleResChange();
	virtual void doDraw();
	virtual ~CCTFHudPanel();

	//todo: member variables and functions
};


class CCylinderAmmoPanel : public CBasePanel
{
public:
	virtual void handleResChange();
	virtual void doDraw();
	virtual ~CCylinderAmmoPanel();
	virtual void setAmmoAmt(int amount);
	virtual float getAmmoWidth();
	virtual void setBulletsPerRow(int bullets_per_row);

	//todo: member variables and functions
};

class CExplosiveHud : public CBasePanel
{
public:
	virtual void handleResChange();
	virtual void doDraw();
	virtual ~CExplosiveHud();

	//todo: member variables and functions
};

class CFadeOverlayPanel : public CBasePanel
{
public:
	virtual void handleResChange();
	virtual void doDraw();
	virtual ~CFadeOverlayPanel();

	//todo: member variables and functions
};

class CGadgetBatteryPanel : public CBasePanel
{
public:
	virtual void handleResChange();
	virtual void doDraw();
	virtual ~CGadgetBatteryPanel();
	virtual void setBatteryAmt(int amount);

	//todo: member variables and functions
};

//CGUIPanel was here

class CHealthHudPanel : public CBasePanel
{
public:
	virtual void handleResChange();
	virtual void doDraw();
	virtual ~CHealthHudPanel();

	//todo: member variables and functions
};

class CHudIconPanel : public CBasePanel
{
public:
	virtual void handleResChange();
	virtual void doDraw();
	virtual ~CHudIconPanel();

	//todo: member variables and functions
};

class CMovementModePanel : public CBasePanel
{
public:
	virtual void handleResChange();
	virtual void doDraw();
	virtual ~CMovementModePanel();

	//todo: member variables and functions
};

class CMPChatPanel : public CBasePanel
{
public:
	virtual void handleResChange();
	virtual void doDraw();
	virtual ~CMPChatPanel();

	//todo: member variables and functions
};

class CMPHudPanel : public CBasePanel
{
public:
	virtual void handleResChange();
	virtual void doDraw();
	virtual ~CMPHudPanel();

	//todo: member variables and functions
};

class CSimpleScoreBoard : public CBasePanel
{
public:
	virtual void handleResChange();
	virtual ~CSimpleScoreBoard();

	//todo: member variables and functions
};

class CSinglePlayerScoreboard : public CGUIPanel
{
public:
	virtual void setVisible(bool visible);
	virtual ~CSinglePlayerScoreboard();

	//todo: member variables and functions
};

class CMultiPlayerScoreboard : public CSimpleScoreBoard
{
public:
	virtual void doDraw();
	virtual ~CMultiPlayerScoreboard();
	virtual void update();

	//todo: member variables and functions
};

class CObjectiveOverviewPanel : public CBasePanel
{
public:
	virtual void handleResChange();
	virtual void doDraw();
	virtual ~CObjectiveOverviewPanel();

	//todo: member variables and functions
};

class CObjectivePanel : public CBasePanel
{
public:
	virtual void handleResChange();
	virtual void doDraw();
	virtual ~CObjectivePanel();

	//todo: member variables and functions
};

class CPlayerHUDID : public CBasePanel
{
public:
	virtual void doDraw();
	virtual ~CPlayerHUDID();

	//todo: member variables and functions
};

class CProgressPanel : public CBasePanel
{
public:
	virtual void handleResChange();
	virtual void doDraw();
	virtual ~CProgressPanel();

	//todo: member variables and functions
};

class CRocketOverlayPanel : public CBasePanel
{
public:
	virtual void handleResChange();
	virtual void doDraw();
	virtual ~CRocketOverlayPanel();

	//todo: member variables and functions

	void TurnOff()
	{
		reinterpret_cast<void(__thiscall*)(CRocketOverlayPanel*)>(0x41043790)(this);
	}
};

class CSavingOverlayPanel : public CBasePanel
{
public:
	virtual void handleResChange();
	virtual void doDraw();
	virtual ~CSavingOverlayPanel();

	//todo: member variables and functions
};

class CScubaOverlayPanel : public CBasePanel
{
public:
	virtual void handleResChange();
	virtual void doDraw();
	virtual ~CScubaOverlayPanel();

	//todo: member variables and functions
};

class CSpaceSuitOverlayPanel : public CBasePanel
{
public:
	virtual void handleResChange();
	virtual void doDraw();
	virtual ~CSpaceSuitOverlayPanel();

	//todo: member variables and functions
};

class CStaggeredClipAmmoPanel : public CBasePanel
{
public:
	virtual void handleResChange();
	virtual void doDraw();
	virtual ~CStaggeredClipAmmoPanel();

	virtual void setAmmoAmt(int amount);
	virtual float getAmmoWidth();
	virtual void setBulletsPerRow(int bullets_per_row);

	//todo: member variables and functions
};

class CStingerPanel : public CBasePanel
{
public:
	virtual void handleResChange();
	virtual void doDraw();
	virtual ~CStingerPanel();

	//todo: member variables and functions
};

class CVisionOverlayPanel : public CBasePanel
{
public:
	virtual void handleResChange();
	virtual void doDraw();
	virtual ~CVisionOverlayPanel();

	//todo: member variables and functions
};

class CWeaponSelectionHudPanel : public CBasePanel
{
public:
	virtual void handleResChange();
	virtual void doDraw();
	virtual ~CWeaponSelectionHudPanel();

	//todo: member variables and functions
};

class GenericViewport : public CBasePanel
{
public:

	virtual void Draw();
	virtual ~GenericViewport();

	//todo: member variables and functions

	//int basepanel; //vtable
	//int visible; // in vtable
	int parent;
	int temp4;
	int temp5;
	int temp6;
	int temp7;
	int hud_is_on;
	CAmmoHudPanel* ammohudpanel;
	CHealthHudPanel* healthhudpanel;
	CWeaponSelectionHudPanel* weaponselectionhudpanel;
	CCrosshairPanel* crosshairpanel;
	CVisionOverlayPanel* visionoverlaypanel;
	CMovementModePanel* movementmodepanel;
	CStingerPanel* stingerpanel;
	CFadeOverlayPanel* fadeoverlaypanel;
	CCrosshairOverlayPanel* crosshairoverlaypanel;
	CRocketOverlayPanel* rocketoverlaypanel;
	CSpaceSuitOverlayPanel* spacesuitoverlaypanel;
	CScubaOverlayPanel* scubaoverlaypanel;
	CObjectivePanel* objectivepanel;
	CObjectiveOverviewPanel* objectiveoverviewpanel;
	CMultiPlayerScoreboard* multiplayerscoreboard;
	CPlayerHUDID* playerhudid;
	CProgressPanel* progresspanel;
	CGadgetBatteryPanel* gadgetbatterypanel;
	CMPHudPanel* mphudpanel;
	CMPChatPanel* mpchatpanel;
	CSinglePlayerScoreboard* singleplayerscoreboard;
	CExplosiveHud* explosivehud;
	CSavingOverlayPanel* savingoverlaypanel;
	CHudIconPanel* hudiconpanel;
};


//void (*BEGIN_READ)(void* buf, int size) = (void (*)(void*, int))0x410387F0;
//int (*READ_BYTE)() = (int (*)())0x41038850;