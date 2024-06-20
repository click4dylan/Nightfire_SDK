#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "cmdlib.h"

double          I_FloatTime()
{
    FILETIME        ftime;
    double rval;

    GetSystemTimeAsFileTime(&ftime);

    rval = ftime.dwLowDateTime;
    rval += ((__int64)ftime.dwHighDateTime) << 32;

    return (rval / 10000000.0);
}

int             LittleLong(const int l)
{
    return l;
}

inline void getFilePositions(char* path, int* extension_position, int* directory_position)
{
	char* ptr = strrchr(path, '.');
	if (ptr == 0)
	{
		*extension_position = -1;
	}
	else
	{
		*extension_position = ptr - path;
	}

	ptr = max(strrchr(path, '/'), strrchr(path, '\\'));
	if (ptr == 0)
	{
		*directory_position = -1;
	}
	else
	{
		*directory_position = ptr - path;
		if (*directory_position > *extension_position)
		{
			*extension_position = -1;
		}

		//cover the case where we were passed a directory - get 2nd-to-last slash
		if (*directory_position == strlen(path) - 1)
		{
			do
			{
				--(*directory_position);
			} while (*directory_position > -1 && path[*directory_position] != '/' && path[*directory_position] != '\\');
		}
	}
}

void DefaultExtension(char* path, const char* extension)
{
    int extension_pos, directory_pos;
    getFilePositions(path, &extension_pos, &directory_pos);
    if (extension_pos == -1)
    {
        strcat(path, extension);
    }
}