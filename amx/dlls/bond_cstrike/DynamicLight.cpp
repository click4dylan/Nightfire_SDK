#include "DynamicLight.h"
#include "Dylans_Functions.h"
#include "engine.h"

void DLight_HandleClientUserInfoChanged ( edict_t *pEntity, char *infobuffer ) {
	//If player's game version is 5.82 or newer then they support dynamic lights
	char *versionstr = INFOKEY_VALUE(infobuffer, "gamever");
	int index = ENTINDEX(pEntity);
	if ( strlen(versionstr) > 0 ) {
		StringList *list = SplitChar(versionstr, " ");
		if ( atof(list->Get(0)) >= 5.82f) {
			plinfo[index].bSupportsDynamicLights = true;
		} else {
			//version too old to support Dlights
			if ( plinfo[index].bSupportsDynamicLights ) { //at connect we default to true so we can increment the counter properly
				glinfo.NumPlayersWithoutDynamicLightSupport++;
				plinfo[index].bSupportsDynamicLights = false;
			}
		}
		delete list;
	} else {
		//1.0 or really old player
		if ( plinfo[index].bSupportsDynamicLights ) { //at connect we default to true so we can increment the counter properly
			glinfo.NumPlayersWithoutDynamicLightSupport++;
			plinfo[index].bSupportsDynamicLights = false;
		}
	}
}

//don't use this or you will commit suicide on old clients
void Create_TE_DLIGHT( int MSGTYPE, edict_t *pEntity, Vector origin, float exponent, float radius, Color rgb, int life, float decayRate ) {

	MESSAGE_BEGIN(MSGTYPE, SVC_TEMPENTITY, NULL, pEntity );
		WRITE_BYTE( TE_DLIGHT );
		WRITE_SHORT( ENTINDEX(pEntity) );
		WRITE_COORD( origin.x );  // origin
		WRITE_COORD( origin.y );
		WRITE_COORD( origin.z );
		WRITE_COORD( exponent ); //brightness
		WRITE_COORD( radius ); //radius
		WRITE_BYTE( rgb.red ); // R
		WRITE_BYTE( rgb.green ); // G
		WRITE_BYTE( rgb.blue ); // B
		WRITE_BYTE ( life ); //life
		WRITE_COORD ( decayRate ); //decay
	MESSAGE_END();	
}

//wrapper, handles game versions
void UTIL_Create_TE_DLIGHT(edict_t *pEntity, Vector origin, float exponent, float radius, Color rgb, int life, float decayRate ) {
	if ( !pEntity ){
		if ( glinfo.NumPlayersWithoutDynamicLightSupport == 0) {
			//All players support dynamic lights, send a single message notifying everyone about this light
			Create_TE_DLIGHT( MSG_BROADCAST, pEntity, origin, exponent, radius, rgb, life, decayRate);
		}else{
			//send individual messages to each client that DOES support dynamic lights, ignoring those who don't so they don't crash
			for(int i=0; i < gpGlobals->maxClients; i++) {
				client_t *pPlayer = GetClientPointerFromIndex(i);
				if ( pPlayer->active && pPlayer->connected && !pPlayer->fakeclient /*MF_IsPlayerIngame(i) && !MF_IsPlayerBot(i)*/ && plinfo[i + 1].bSupportsDynamicLights ) {
					Create_TE_DLIGHT( MSG_ONE_UNRELIABLE, pPlayer->edict, origin, exponent, radius, rgb, life, decayRate);
				}
			}
		}
	}else{ //send a private message to a specific person notifying them of this light, but only if they support dynamic lights
		int index = ENTINDEX(pEntity);
		if ( MF_IsPlayerIngame(index) && !MF_IsPlayerBot(index) && plinfo[index].bSupportsDynamicLights ){
			Create_TE_DLIGHT( MSG_ONE_UNRELIABLE, pEntity, origin, exponent, radius, rgb, life, decayRate);
		}
	}
}

void UTIL_Create_TE_ELIGHT(edict_t *pEntity, Vector origin, int radius, Color rgb, int life, int decayRate ) {
	//All game versions support entity lights
	edict_t *NOTHING = 0;

	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY, NULL, NOTHING );
		WRITE_BYTE( TE_ELIGHT );
		WRITE_SHORT( ENTINDEX( pEntity ) + 0x1000 );		// entity, attachment
		WRITE_COORD( origin.x );		// origin
		WRITE_COORD( origin.y );
		WRITE_COORD( origin.z );
		WRITE_COORD( radius );	// radius
		WRITE_BYTE( rgb.red );	// R
		WRITE_BYTE( rgb.green );	// G
		WRITE_BYTE( rgb.blue );	// B
		WRITE_BYTE( life );	// life * 10
		WRITE_COORD( decayRate ); // decay
	MESSAGE_END();
}