#include <stdio.h>
#include <math.h>
#include "helpers.h"
#include "winding.h"
#include "bsp_structs.h"
#include "face.h"
#include "globals.h"
#include "log.h"
#include "brush.h"
#include "threads.h"
#include <vector>

bool safe_strncat(char* dest, const char* src, size_t count)
{
    if (count)
    {
        strncat(dest, src, count);
        dest[count - 1] = 0;
        return 1;
    }
    else
    {
        Warning("_safe_strncat passed empty count");
        return 0;
    }
}