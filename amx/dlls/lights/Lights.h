#ifndef LIGHTS_H
#define LIGHTS_H
#include "engine.h"
#include "Dylans_Functions.h"
#define MODE_THIRDPERSON 0
#define MODE_FREELOOK 1
#define MODE_FIRSTPERSON 2

void OnModuleInitialize(void);
void ConsoleCommand(edict_t *pEntity, int argl);
void DoSetLights();
void SetLights(const char *szLights, bool setdefault = false); 
#endif