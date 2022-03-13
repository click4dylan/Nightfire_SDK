#ifndef MAPSPECIFICS_H
#define MAPSPECIFICS_H
#include "engine.h"
#include "Dylans_Functions.h"
#include "DynamicLight.h"

void OnModuleInitialize(void);
void MapSpecificsCommand(void);
void ConsoleCommand(edict_t *pEntity, int argl);
void OnMapSpecifics(void);
#endif