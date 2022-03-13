//Thanks to https://github.com/A5-/Gamerfood_CSGO
#ifndef VTHOOK_H
#define VTHOOK_H
#pragma once
#include <Windows.h>
//DWORD HookGameFunc(DWORD** ppClassBase, int IndexToHook, DWORD ReplacementFunction);

inline DWORD* GetVT(PDWORD* baseclass)
{
	return (DWORD*)*(DWORD*)((DWORD)baseclass);
}

inline DWORD* GetVT(DWORD ptobaseclass)
{
	return (DWORD*)*(DWORD*)(*(DWORD*)(ptobaseclass));
}

class VTHook
{
public:
	VTHook()
	{
		memset(this, 0, sizeof(VTHook));
	}

	VTHook(PDWORD* ppdwClassBase)
	{
		bInitialize(ppdwClassBase);
	}

	~VTHook()
	{
		if (m_ClassBase)
		{
			//*m_ClassBase = m_OldVT;
			*(DWORD*)(*(DWORD*)((DWORD)m_ClassBase) =  (DWORD)m_OldVT);
		}
	}

	PDWORD* GetClassBase()
	{
		return m_ClassBase;
	}

	void ClearClassBase()
	{
		m_ClassBase = NULL;
	}

	PDWORD GetOldVT()
	{
		return m_OldVT;
	}

	PDWORD GetNewVT()
	{
		return (PDWORD)((DWORD)m_NewVT + 4);
	}

	PDWORD GetCurrentVT()
	{
		return (PDWORD)*(DWORD*)((DWORD)m_ClassBase);
	}

	void bInitialize(PDWORD* ppdwClassBase)
	{
		m_ClassBase = ppdwClassBase; // Get Pointer to the class base and store in member variable of current object
									 
		m_OldVT = (PDWORD)*(DWORD*)((DWORD)ppdwClassBase);
		m_VTSize = GetVTCount(m_OldVT);// Get the number of functions in the Virtual Address Table


		m_NewVT = new DWORD[m_VTSize + 1]; // Create A new virtual Address Table
										   

		memcpy((char*)((DWORD)m_NewVT + 4), (char*)m_OldVT, sizeof(DWORD) * m_VTSize);
		m_NewVT[0] = (DWORD)*(DWORD*)((DWORD)m_OldVT - 4);

		*(DWORD*)(*(DWORD*)((DWORD)&ppdwClassBase)) = (DWORD)((DWORD)m_NewVT + 4);
	}
	void bInitialize(PDWORD** pppdwClassBase) // fix for pp
	{
		bInitialize(*pppdwClassBase);
	}

	/*
	void ReHook()
	{
	if (m_ClassBase)
	{
	*m_ClassBase = m_NewVT;
	}
	}
	*/
	/*
	int iGetFuncCount()
	{
	return (int)m_VTSize;
	}
	*/
	/*
	DWORD GetFuncAddress(int Index)
	{
	if (Index >= 0 && Index <= (int)m_VTSize && m_OldVT != NULL)
	{
	return m_OldVT[Index];
	}
	return NULL;
	}
	*/

	/*
	PDWORD GetOldVT()
	{
	return m_OldVT;
	}
	*/

	DWORD HookFunction(DWORD dwNewFunc, unsigned int iIndex)
	{
		// Noel: Check if both the New Virtual Address Table and the the Old one have been allocated and exist,
		// also check if the function index into the New Virtual Address Table is within the bounds of the table
		if (m_NewVT && m_OldVT && iIndex <= m_VTSize && iIndex >= 0)
		{
			m_NewVT[iIndex + 1] = dwNewFunc; // Set the new function to be called in the new virtual address table
			return *(DWORD*)((DWORD)m_OldVT + (sizeof(DWORD*) * iIndex));
			//return m_OldVT[iIndex]; // Return the old function to be called in the virtual address table
		}
		// Return null and fail if the new Virtual Address Table or the old one hasn't been allocated with = new,
		// or if the index chosen is outside the bounds of the table.
		return NULL;
	}

private:

	//Use standard C++ functions instead
	BOOL CodePointerIsInvalid(DWORD* testptr) 
	{
		return IsBadCodePtr((FARPROC)testptr);
	}
	DWORD GetVTCount(PDWORD pdwVMT)
	{
		DWORD dwIndex = NULL;

		for (dwIndex = 0; pdwVMT[dwIndex]; dwIndex++)
			if (!pdwVMT[dwIndex])
				break;

		return dwIndex;
	}
	PDWORD*	m_ClassBase;
	PDWORD	m_NewVT, m_OldVT;
	DWORD	m_VTSize;
};

extern VTHook* DirectX;
//extern VTHook*  D3D9;
void SetupVMTHooks();
extern float(*RandomFloat) (float from, float to);
extern void(*RandomSeed) (unsigned int seed);

template <typename T>
T GetVFunc(void *vTable, int iIndex) {
	return (*(T**)vTable)[iIndex];
}


void HookFuncs(bool ENABLE_NVIDIA_FIX);
#endif