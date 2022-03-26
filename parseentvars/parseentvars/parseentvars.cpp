// parseentvars.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include <fstream>

typedef enum _fieldtypes
{
	FIELD_FLOAT = 0,		// Any floating point value
	FIELD_STRING,			// A string ID (return from ALLOC_STRING)
	FIELD_ENTITY,			// An entity offset (EOFFSET)
	FIELD_CLASSPTR,			// CBaseEntity *
	FIELD_EHANDLE,			// Entity handle
	FIELD_EVARS,			// EVARS *
	FIELD_EDICT,			// edict_t *, or edict_t *  (same thing)
	FIELD_VECTOR,			// Any vector
	FIELD_POSITION_VECTOR,	// A world coordinate (these are fixed up across level transitions automagically)
	FIELD_POINTER,			// Arbitrary data pointer... to be removed, use an array of FIELD_CHARACTER
	FIELD_INTEGER,			// Any integer or enum
	FIELD_FUNCTION,			// A class function pointer (Think, Use, etc)
	FIELD_BOOLEAN,			// boolean, implemented as an int, I may use this as a hint for compression
	FIELD_SHORT,			// 2 byte integer
	FIELD_CHARACTER,		// a byte
	FIELD_TIME,				// a floating point time (these are fixed up automatically too!)
	FIELD_MODELNAME,		// Engine string that is a model name (needs precache)
	FIELD_SOUNDNAME,		// Engine string that is a sound name (needs precache)

	FIELD_TYPECOUNT,		// MUST BE LAST
} FIELDTYPE;

#define FTYPEDESC_GLOBAL			0x0001
#define offsetof(s,m)	(size_t)&(((s *)0)->m)
#define _FIELD(type,name,fieldtype,count,flags)		{ fieldtype, #name, offsetof(type, name), count, flags }
#define DEFINE_FIELD(type,name,fieldtype)			_FIELD(type, name, fieldtype, 1, 0)
#define DEFINE_ARRAY(type,name,fieldtype,count)		_FIELD(type, name, fieldtype, count, 0)
#define DEFINE_ENTITY_FIELD(name,fieldtype)			_FIELD(entvars_t, name, fieldtype, 1, 0 )
#define DEFINE_ENTITY_GLOBAL_FIELD(name,fieldtype)	_FIELD(entvars_t, name, fieldtype, 1, FTYPEDESC_GLOBAL )
#define DEFINE_GLOBAL_FIELD(type,name,fieldtype)	_FIELD(type, name, fieldtype, 1, FTYPEDESC_GLOBAL )

typedef struct
{
	FIELDTYPE		fieldType;
	const char* fieldName;
	int				fieldOffset;
	short			fieldSize;
	short			flags;
} TYPEDESCRIPTION;

typedef struct
{
	const char* name;
	unsigned long function;
} CLIENTENTITY;

int main()
{
	SetCurrentDirectoryA("E:\\f\\games\\nightfire alpha");
    HMODULE mod = LoadLibraryA("E:\\f\\games\\nightfire alpha\\bond\\dlls\\game.dll");
    if (mod)
    {
        DWORD test = 0x421456D8;
		TYPEDESCRIPTION* desc = (TYPEDESCRIPTION*)test;
		std::ofstream out("E:\\alphatypedescriptions.txt");
		if (out.is_open())
		{
			for (int i = 0; i < 87; ++i)
			{
				TYPEDESCRIPTION* thedesc = &desc[i];
				out << thedesc->fieldType << "\n";
				out << thedesc->fieldName << "\n";
				out << thedesc->fieldOffset << "\n";
				out << thedesc->fieldSize << "\n";
				out << thedesc->flags << "\n\n";
			}
			out.close();
		}
		FreeLibrary(mod);

		SetCurrentDirectoryA("E:\\nightfire_11\\nightfire");
		mod = LoadLibraryA("E:\\nightfire_11\\nightfire\\bond\\dlls\\game.dll");
		if (mod)
		{
			test = 0x421501D0;
			desc = (TYPEDESCRIPTION*)test;
			out.open("E:\\finaltypedescriptions.txt");
			if (out.is_open())
			{
				for (int i = 0; i < 89; ++i)
				{
					TYPEDESCRIPTION* thedesc = &desc[i];
					out << thedesc->fieldType << "\n";
					out << thedesc->fieldName << "\n";
					out << thedesc->fieldOffset << "\n";
					out << thedesc->fieldSize << "\n";
					out << thedesc->flags << "\n\n";
				}
				out.close();
			}
			FreeLibrary(mod);
		}
		mod = LoadLibraryA("E:\\nightfire_11\\nightfire\\bond\\cl_dlls\\client.dll");
		if (mod)
		{
			out.open("E:\\client_entities.txt");
			if (out.is_open())
			{
				test = 0x410847C0;
				CLIENTENTITY* c = (CLIENTENTITY*)test;
				for (int i = 0; i < 7; ++i)
				{
					CLIENTENTITY* e = &c[i];
					out << e->name << "\n";
					out << std::hex << e->function << "\n\n";
				}
			}
			out.close();
		}
		FreeLibrary(mod);
    }
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
