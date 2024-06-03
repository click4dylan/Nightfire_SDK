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