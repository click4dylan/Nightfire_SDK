#pragma once
#include <Windows.h>

extern char* Languages[8];
extern unsigned int* KeyboardLayout;
extern unsigned int* KeyboardLayouts[8];

extern int MapKey(int key);
extern BOOLEAN SetKeyboardLayout(char* region);
