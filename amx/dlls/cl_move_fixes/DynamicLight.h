#ifndef DYNAMIC_LIGHT_H
#define DYNAMIC_LIGHT_H
#include "engine.h"

struct Color { 
	int red;
	int green;
	int blue;
	Color(int Red, int Green, int Blue): red(Red), green(Green), blue(Blue) { }
};

void DLight_HandleClientUserInfoChanged( edict_t *pEntity, char *infobuffer );
void Create_TE_DLIGHT( int MSGTYPE, edict_t *pEntity, Vector origin, float exponent, float radius, Color rgb, int life, float decayRate ); //don't use this or you will commit suicide on old clients
void UTIL_Create_TE_DLIGHT(edict_t *pEntity, Vector origin, float exponent, float radius, Color rgb, int life, float decayRate ); //wrapper, handles game versions
void UTIL_Create_TE_ELIGHT(edict_t *pEntity, Vector origin, int radius, Color rgb, int life, int decayRate );
#endif