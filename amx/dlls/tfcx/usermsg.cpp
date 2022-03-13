/*
 * TFCX 
 * Copyright (c) 2004 Lukasz Wlasinski
 *
 *
 *    This program is free software; you can redistribute it and/or modify it
 *    under the terms of the GNU General Public License as published by the
 *    Free Software Foundation; either version 2 of the License, or (at
 *    your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful, but
 *    WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software Foundation,
 *    Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *    In addition, as a special exception, the author gives permission to
 *    link the code of this program with the Half-Life Game Engine ("HL
 *    Engine") and Modified Game Libraries ("MODs") developed by Valve,
 *    L.L.C ("Valve").  You must obey the GNU General Public License in all
 *    respects for all of the code used other than the HL Engine and MODs
 *    from Valve.  If you modify this file, you may extend this exception
 *    to your version of the file, but you are not obligated to do so.  If
 *    you do not wish to do so, delete this exception statement from your
 *    version.
 *
 */

#include "amxxmodule.h"
#include "tfcx.h"


int damage;
int TA;
int weapon;
int aim;
CPlayer *pAttacker;
edict_t* tempEnt;
int tempInt;
bool ignoreDamage = false;

void Client_ResetHUD(void* mValue){
	if ( mPlayer ){
		mPlayer->clearStats = gpGlobals->time + 0.25f;
	}
}

/*
void Client_WeaponList(void* mValue){
  static int wpnList;
  static int iSlot;
  static const char* wpnName;

  switch (mState++) {
  case 0:
    wpnName = (const char*)mValue;
    break;
  case 1:
    iSlot = *(int*)mValue;
    break;
  case 7:
    int iId = *(int*)mValue;
    if ( (iId < 0 || iId >= MAX_WEAPONS ) || ( wpnList & (1<<iId) ) )
      break;

    wpnList |= (1<<iId);
    weaponData[iId].ammoSlot = iSlot;
    strcpy(weaponData[iId].fullName, wpnName );
	weaponData[iId].name = weaponData[iId].fullName;

	char* wpnPrefix = strstr( weaponData[iId].fullName ,"tf_weapon_");

	if ( wpnPrefix )
		weaponData[iId].name = wpnPrefix + 10;
  } 
}
*/

void Client_Damage(void* mValue){
  switch (mState++) {
  case 1: 
    damage = *(int*)mValue;
    break;

  case 2:
	edict_t* enemy;
    if ( !mPlayer || !damage )  
		break;

	enemy = mPlayer->pEdict->v.dmg_inflictor;

	if ( FNullEnt( enemy ) )
		break;

	if (enemy->v.flags & (FL_CLIENT | FL_FAKECLIENT) ) { // attacker is player and his active weapon

		pAttacker = GET_PLAYER_POINTER(enemy);
		
		aim = pAttacker->aiming;
		weapon = pAttacker->current;

		switch(weapon){
		case TFC_WPN_SPANNER:
		case TFC_WPN_AXE:
		case TFC_WPN_KNIFE:
			pAttacker->saveShot(weapon); // melee , save shot too
			break;

		case TFC_WPN_MEDIKIT:
			if ( pAttacker->teamId == mPlayer->teamId ) // ???
				ignoreDamage = true;
			pAttacker->saveShot(weapon); // melee , save shot too
			break;
		}
	
		pAttacker->saveHit( mPlayer , weapon , damage, aim);

	}
	else if ( !FNullEnt( enemy->v.owner ) ){
		if ( enemy->v.owner->v.flags & (FL_CLIENT | FL_FAKECLIENT)  ){ // caltrop, empgrenade, gasgrenade, napalmgrenade

			pAttacker = GET_PLAYER_POINTER(enemy->v.owner);

			const char *szClass = STRING(enemy->v.classname);
			
			switch(szClass[10]){
				case 'c':
					weapon = TFC_WPN_CALTROP;
					break;
				case 'e':
					weapon = TFC_WPN_EMPGRENADE;
					break;
				case 'g':
					weapon = TFC_WPN_GASGRENADE;
					break;
				case 'm':
					weapon = TFC_WPN_MEDIKIT;
					if ( pAttacker->teamId == mPlayer->teamId ) // ???
						ignoreDamage = true;
					pAttacker->saveShot(weapon);
					break;
				case 'n':
					weapon = TFC_WPN_NAPALMGRENADE;
					break;
				case 'a':
				case 's':
				case '_': // hmm...nailgun_nail, fastswitch and this one may not be true...
					//PRINT_CONSOLE("Nail! Class:%d wpn:%d\n",pAttacker->classId,pAttacker->current);
					weapon = pAttacker->current;
					aim = pAttacker->aiming;
					break;
				case 'r':
					weapon = TFC_WPN_FLAMETHROWER;
					break;
			}
			if ( !weapon ) {
				switch(szClass[3]){
				case 'e':
					weapon = TFC_WPN_TIMER; // TFC_WPN_MEDKIT ??

					tempInt = *( (int*)mPlayer->pEdict->pvPrivateData + pdTimerOwner );
					if (!tempInt)
						break;
					tempEnt = (edict_t*)tempInt;
					pAttacker = GET_PLAYER_POINTER(tempEnt);
					
					if ( pAttacker->teamId == mPlayer->teamId ) // ???
						ignoreDamage = true;
					
					pAttacker->saveShot(weapon); // ??? save shot too
					break;
				case 'f':
					weapon = TFC_WPN_FLAMES; // tf_fire
					break;
				}
			}

			if ( weapon )
				pAttacker->saveHit( mPlayer , weapon , damage, aim );
			//else 
			//	PRINT_CONSOLE("*** DMG! Att:%d Vic:%d unknown weapon %s\n",pAttacker->index,mPlayer->index,STRING(enemy->v.classname));
		}
		// nailgrenadenail->nailgrenade->player :))) I'm the best !
		else if ( !FNullEnt(enemy->v.owner->v.owner) && enemy->v.owner->v.owner->v.flags & (FL_CLIENT | FL_FAKECLIENT ) ){
			pAttacker = GET_PLAYER_POINTER(enemy->v.owner->v.owner);
			weapon = TFC_WPN_NAILGRENADE;
			pAttacker->saveHit( mPlayer , weapon , damage, aim );
		}
	}
    else { // nailgrenade , mirvgrenade , normalgrenade , rockets
		if ( strstr("sentrygun",STRING(enemy->v.classname)) ){
			tempInt = *( (int*)enemy->pvPrivateData + pdSentryGunOwner );
			if ( !tempInt )
				break;
			tempEnt = (edict_t*)tempInt;
			pAttacker = GET_PLAYER_POINTER(tempEnt);
			weapon = TFC_WPN_SENTRYGUN;
			pAttacker->saveShot(weapon); // save shot too
			pAttacker->saveHit( mPlayer , weapon , damage, aim );
		}
		else if( g_grenades.find(enemy , &pAttacker , &weapon ) ){
			if ( weapon == TFC_WPN_RPG && pAttacker->classId == TFC_PC_PYRO ){
				weapon = TFC_WPN_IC;
				pAttacker->saveShot(weapon);
			}
			
			pAttacker->saveHit( mPlayer , weapon , damage, aim );
		}
	}
	break;
  }
  
}

void Client_Damage_End(void* mValue){
	if ( !damage )
		return;

	if ( !pAttacker )
		pAttacker = mPlayer;

	if ( !ignoreDamage ){
		TA = 0;
		if ( (mPlayer->teamId == pAttacker->teamId) && (mPlayer != pAttacker) )
			TA = 1;
		if (g_damage_info > 0)
			MF_ExecuteForward(g_damage_info, pAttacker->index, mPlayer->index, damage, weapon, aim, TA);
	
		if( !mPlayer->IsAlive() ){
			pAttacker->saveKill(mPlayer,weapon,( aim == 1 ) ? 1:0 ,TA);
			
			if (g_death_info > 0)
				MF_ExecuteForward(g_death_info, pAttacker->index, mPlayer->index, weapon, aim, TA);
		}
	}

	damage = 0;
	aim = 0;
	weapon = 0;
	pAttacker = NULL;
	ignoreDamage = false;

}

void Client_CurWeapon(void* mValue){
  
  static int iState;
  static int iId;
  switch (mState++){
  case 0: 
    iState = *(int*)mValue;
    break;
  case 1:
    if (!iState) break; 
    iId = *(int*)mValue;
    break;
  case 2:
	if (!mPlayer || !iState ) break;
    int iClip = *(int*)mValue;
    
	if ((iClip > -1) && (iClip < mPlayer->weapons[iId].clip)){
		mPlayer->saveShot(iId);
	}
	
    mPlayer->weapons[iId].clip = iClip;
	mPlayer->current = iId;
  }
}

void Client_AmmoX(void* mValue){
  
  static int iAmmo;
  switch (mState++){
  case 0:
    iAmmo = *(int*)mValue;
    break;
  case 1:
	if (!mPlayer ) break;

	// SniperRifle, AC and AutoRifle ...
	if ( mPlayer->classId == TFC_PC_HWGUY ){
		if ( mPlayer->current == TFC_WPN_AC && mPlayer->weapons[mPlayer->current].ammo > *(int*)mValue  && iAmmo == weaponData[mPlayer->current].ammoSlot )
			mPlayer->saveShot(mPlayer->current);
	}
	else if ( mPlayer->classId == TFC_PC_SNIPER ){
		if ( (mPlayer->current == TFC_WPN_SNIPERRIFLE || mPlayer->current == TFC_WPN_AUTORIFLE) && mPlayer->weapons[mPlayer->current].ammo > *(int*)mValue && iAmmo == weaponData[mPlayer->current].ammoSlot )
			mPlayer->saveShot(mPlayer->current);
	}
	//

    for(int i = 1; i < MAX_WEAPONS ; ++i) 
      if (iAmmo == weaponData[i].ammoSlot)
        mPlayer->weapons[i].ammo = *(int*)mValue;
  }
}

void Client_AmmoPickup(void* mValue){
  
  static int iSlot;
  switch (mState++){
  case 0:
    iSlot = *(int*)mValue;
    break;
  case 1:
	if (!mPlayer ) break;
    for(int i = 1; i < MAX_WEAPONS ; ++i)
      if (weaponData[i].ammoSlot == iSlot)
        mPlayer->weapons[i].ammo += *(int*)mValue;
  }
}

void Client_ScoreInfo(void* mValue){
  static int iIndex;
  static int iClass;
  switch (mState++){
  case 0:
    iIndex = *(int*)mValue;
    break;
  case 3:
	  iClass = *(int*)mValue;
	  break;
  case 4:
	  if ( iIndex > 0 && iIndex <= gpGlobals->maxClients ){
		  GET_PLAYER_POINTER_I( iIndex )->teamId = *(int*)mValue;
		  GET_PLAYER_POINTER_I( iIndex )->classId = iClass;
	  }

  }
}

