#ifndef VOTING_H
#define VOTING_H
#include "engine.h"
#include "Dylans_Functions.h"
#include "DynamicLight.h"

void OnModuleInitialize(void);
void OnStartFrame();
void OnClientCommand(edict_t *pEntity, int argl, const char **str);
void ConsoleCommand(edict_t *pEntity, int argl);
#endif