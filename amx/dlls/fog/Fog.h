#ifndef FOG_H
#define FOG_H
#include "engine.h"
#include "Dylans_Functions.h"
#include "DynamicLight.h"

void OnModuleInitialize(void);
void ConsoleCommand(edict_t *pEntity, int argl);
void SetFog(Color col, float fogstart, float fogend, bool setdefault); 
void DoSetFog();
#endif