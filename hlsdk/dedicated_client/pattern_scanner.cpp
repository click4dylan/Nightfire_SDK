#include "pattern_scanner.h"
#include <memory>
#define PSAPI_VERSION 1
#include <Psapi.h>

void GetModuleStartEndPoints(HANDLE ModuleHandle, uintptr_t& start, uintptr_t& end) {
	MODULEINFO dllinfo;
	GetModuleInformation(GetCurrentProcess(), (HMODULE)ModuleHandle, &dllinfo, sizeof(MODULEINFO));
	start = (uintptr_t)ModuleHandle;
	end = (uintptr_t)ModuleHandle + dllinfo.SizeOfImage;
}

uintptr_t FindMemoryPattern(uintptr_t start, uintptr_t end, const char* strpattern, size_t length, bool double_wide) {
	uintptr_t	   adrafterfirstmatch = 0;
	size_t		   indextofind = 0;
	size_t		   numhexvalues = 0;
	std::unique_ptr<unsigned char[]>  hexvalues(new unsigned char[length + 1]);
	std::unique_ptr<bool[]>			  shouldskip(new bool[length + 1]);

	if (double_wide) { //DOUBLE SPACES AND QUESTION MARKS, THIS IS FASTER TO RUN
		for (size_t i = 0; i < length - 1; i += 2) {
			//Get the ascii version of the hex values out of the pattern
			char ascii[4];
			*(short*)ascii = *(short*)&strpattern[i];

			//Filter out spaces
			if (ascii[0] != ' ') {
				//Filter out wildcards
				if (ascii[0] == '?') {
					shouldskip[numhexvalues] = true;
				}
				else {
					//Convert ascii to hex
					ascii[2] = NULL; //add null terminator
					hexvalues[numhexvalues] = (unsigned char)std::stoul(ascii, nullptr, 16);
					shouldskip[numhexvalues] = false;
				}
				numhexvalues++;
			}
		}
	}
	else {
		for (size_t i = 0, maxlength = length - 1; i < maxlength; i++) {
			//Get the ascii version of the hex values out of the pattern
			char ascii[4];
			*(short*)ascii = *(short*)&strpattern[i];

			//Filter out spaces
			if (ascii[0] != ' ') {
				//Filter out wildcards
				if (ascii[0] == '?') {
					shouldskip[numhexvalues] = true;
				}
				else {
					//Convert ascii to hex
					ascii[2] = NULL; //add null terminator
					hexvalues[numhexvalues] = (unsigned char)std::stoul(ascii, nullptr, 16);
					shouldskip[numhexvalues] = false;
				}
				i++;
				numhexvalues++;
			}
		}
	}

	//Search for the hex signature in memory	
	for (uintptr_t adr = start; adr < end; adr++)
	{
		if (shouldskip[indextofind] || *(char*)adr == hexvalues[indextofind] || *(unsigned char*)adr == hexvalues[indextofind]) {
			if (indextofind++ == 0)
				adrafterfirstmatch = adr + 1;

			if (indextofind >= numhexvalues)
				return adr - (numhexvalues - 1); //FOUND PATTERN!

		}
		else if (adrafterfirstmatch) {
			adr = adrafterfirstmatch;
			indextofind = 0;
			adrafterfirstmatch = 0;
		}
	}
	return NULL; //NOT FOUND!
}


uintptr_t FindMemoryPattern(DWORD ModuleHandle, std::string strpattern, bool double_wide)
{
	uintptr_t start, end;
	GetModuleStartEndPoints((HANDLE)ModuleHandle, start, end);
	return FindMemoryPattern(start, end, strpattern.c_str(), strpattern.length(), double_wide);
}