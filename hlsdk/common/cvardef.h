/***
*
*	Copyright (c) 1999, 2000, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
#ifndef CVARDEF_H
#define CVARDEF_H

//cl_name = archive, userinfo, printableonly

#define FCVAR_NF_ALWAYSDEFINED (1 << 0) // internally defined by the game
#define FCVAR_NF_GAMEDLL (1 << 1) // defined by RegisterGame
#define FCVAR_NF_CLIENTDLL (1 << 2) // defined by RegisterClient
#define FCVAR_ARCHIVE (1 << 3) // set to cause it to be saved to config.cfg
#define FCVAR_USERINFO (1 << 4) // changes the client's info string
#define FCVAR_SERVER (1 << 5) // notifies players when changed
#define FCVAR_PROTECTED (1 << 6) // It's a server cvar, but we don't send the data since it's a password, etc.  Sends 1 if it's not bland/zero, 0 otherwise as value
#define FCVAR_SPONLY (1 << 7) // This cvar cannot be changed by clients connected to a multiplayer server.
#define FCVAR_UNLOGGED (1 << 8) // If this is a FCVAR_SERVER, don't log changes to the log file / console if we are creating a log
#define FCVAR_PRINTABLEONLY (1 << 9) // This cvar's string cannot contain unprintable characters ( e.g., used for player name etc ).
/*
typedef struct cvar_s
{
char	*name;
char	*string;
int		flags;
float	value;
struct cvar_s *next;
} cvar_t;
*/

//#define BOOL_FALSE 0x00000030 //useful to avoid string compares
//#define BOOL_TRUE 0x00000031 //useful to avoid string compares


typedef struct cvar_s_s
{
	int type; //bool, int, float, string

	const char *name;
	const char *string;
	const char *value;
	unsigned int flags;

	/*
	const char *name;
	const char *string;
	int flags;
	float value;
	struct cvar_s *next;
	*/
} cvar_t_s;

struct IConsoleFunction
{
public:
	const char* name; //4
	const char* description; //8
	unsigned int flags; //12

public:
	virtual void run(unsigned int numargs, const char** args) = 0;
	virtual const char* getName() = 0;
	virtual const char* getDescription() = 0;
	virtual unsigned int getFlags() = 0;
	virtual void setFlags(unsigned int flags) = 0;
	virtual void Delete() = 0;
	virtual ~IConsoleFunction() {};
};

class ConsoleFunction : public IConsoleFunction
{
public:

	virtual void run(unsigned int numargs, const char** args) = 0;
	virtual const char* getName() { return name; };
	virtual const char* getDescription() { return description; };
	virtual unsigned int getFlags() { return flags; };
	virtual void setFlags(unsigned int _flags) { flags = _flags; }
	virtual void Delete() { delete this; };
	virtual ~ConsoleFunction() {};

	ConsoleFunction(const char* _name, const char* _description, unsigned int _flags)
	{
		name = _name;
		description = _description;
		flags = _flags;
	}
};

#if 0
class TestFunction : public ConsoleFunction
{
public:
	using ConsoleFunction::ConsoleFunction; // Inherit constructors from base class, C++11 requirement

	void run(unsigned int numargs, const char** args)
	{

	}
};
#endif

//allow allocating dynamic console functions, such as from amx plugins
class DynamicConsoleFunction : public ConsoleFunction
{
public:
	DynamicConsoleFunction(const char* _name, void (*_custom_run_func)(unsigned int, const char**), const char* _description = "", unsigned int _flags = 0)
		:ConsoleFunction(_name, _description, _flags), custom_run_func(_custom_run_func)
	{
	}

	void run(unsigned int numargs, const char** args)
	{
		custom_run_func(numargs, args);
	}

	void (*custom_run_func)(unsigned int numargs, const char** args);
};

enum VariableTypes : int 
{
	CVAR_BOOL = 0,
	CVAR_INT,
	CVAR_FLOAT,
	CVAR_STRING
};

class IConsoleVariable
{
public:

	VariableTypes type; //bool, int, float, string
	const char* name;
	const char* description;
	const char* value;
	unsigned int flags;

	IConsoleVariable() {};
	IConsoleVariable(VariableTypes _type, const char* _name, const char* _description, const char* _value, unsigned int _flags)
		: type(_type), name(_name), description(_description), value(_value), flags(_flags)
	{}

};

class ConsoleVariable
{
private:
	const char* name; //4
	const char* description; //8
	unsigned int flags; //12
	int type; //16
	void* value; //20
	void* default_value; //24
	char* current_value_str; //28
	const char* default_value_str; //32
public:
	virtual void run(unsigned int numargs, const char** args);
	virtual const char* getName();
	virtual const char* getDescription();
	virtual unsigned int getFlags();
	virtual void setFlags(unsigned int flags);
	virtual void Delete();
	virtual ~ConsoleVariable(/*char deletingdestructor = true*/);
	virtual BOOLEAN isType(VariableTypes type);
	virtual void toDefault();
	virtual BOOLEAN toString(char* dest, int dest_size);
	virtual BOOLEAN toStringFriendly(char* dest, int dest_size);
	virtual void setFromString(const char* value);
	virtual void setValueBool(bool value);
	virtual void setValueInt(int value);
	virtual void setValueFloat(float value);
	virtual void setValueString(const char* value);
	virtual float getValue();
	virtual bool getValueBool();
	virtual int getValueInt();
	virtual float getValueFloat();
	virtual const char* getValueString();
	virtual bool getDefaultBool();
	virtual int getDefaultInt();
	virtual float getDefaultFloat();
	virtual const char* getDefaultString();
};


typedef struct cvar_s
{
public:
	//int *value;
	void *functiontocall; //0 //for toggling, and printing values, etc

	const char *name; //4
	const char *string; //8
	int flags; //12
	int type; //bool, int, float, string //16
	int value; //for int, bool, and float 	 //20
	float defaultvalue; //0; //24 //can also be int, bool, float, etc
	char* szvalue; //28

#if 0

	//dylan's added stuff
	float GetFloat()
	{
		if (!&functiontocall)
			return 0.0f;

		return *(float*)&value;

#if 0
		int adr = (int)&value;
		float retval;
		__asm {
			mov ecx, adr
				mov eax, [ecx]
				mov retval, eax
		}
		return retval;
#endif
	}

	void SetFloat(float val)
	{
		if (!&functiontocall)
			return;
		int adr = (int)&value;
		__asm {
			mov ecx, adr
				mov eax, val
				mov[ecx], eax
		}
	}

	int GetInt()
	{
		if (!&functiontocall)
			return 0;
		int adr = (int)&this->value;
		int retval;
		__asm {
			mov eax, adr
				mov eax, [eax]
				mov retval, eax
		}
		return retval;
	}

	void SetInt(int val)
	{
		if (!&functiontocall)
			return;
		int adr = (int)&value;
		__asm {
			mov eax, val
				mov ecx, adr
				mov[ecx], eax
		}
	}

	BOOLEAN GetBool()
	{
		if (!&functiontocall)
			return 0;
		int adr = (int)&value;
		BOOLEAN retval;
		__asm {
			mov ecx, adr
				mov al, byte ptr ds : [ecx]
				mov retval, al
		}
		return retval;
	}

	void SetBool(BOOLEAN val)
	{
		if (!&functiontocall)
			return;
		int adr = (int)&value;
		__asm {
			mov ecx, adr
				mov al, val
				mov byte ptr ds : [ecx], al
		}
	}

	char* GetString()
	{
		if (!&functiontocall)
			return nullptr;
		return szvalue;
	}

	void SetString(char* szval)
	{
		if (!&functiontocall)
			return;
		const void *Cvar_DirectSet_internal = (const void*)0x43003BB0; //0x43003A40;
		int pointer = (int)&functiontocall; //pointer to this cvar
		__asm
		{
			mov ecx, pointer
				push szval
				call Cvar_DirectSet_internal
		}
	}

	void SetValue(char* szval)
	{
		SetString(szval);
	}

#endif

	/*
	const char *name;
	const char *string;
	int flags;
	float value;
	struct cvar_s *next;
	*/
}cvar_t;

typedef struct cvar_s_small
{
	int type;
	const char *name;
	const char *string;
	char* value;
	int flags;
}cvar_t_small;

inline cvar_t_small cvar_t_TO_cvar_t_small(cvar_t src)
{
	cvar_t_small dest;
	dest.flags = src.flags;
	dest.name = src.name;
	dest.string = src.string;
	dest.type = src.type;
	dest.value = src.szvalue;
	return dest;
}

inline cvar_s_small cvar_s_TO_cvar_s_small(cvar_s src)
{
	cvar_s_small dest;
	dest.flags = src.flags;
	dest.name = src.name;
	dest.string = src.string;
	dest.type = src.type;
	dest.value = src.szvalue;
	return dest;
}



#endif
