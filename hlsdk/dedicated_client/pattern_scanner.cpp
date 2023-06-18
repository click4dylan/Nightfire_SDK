#include "pattern_scanner.h"
#include <memory>
#define PSAPI_VERSION 1
#include <Psapi.h>

DWORD old_protection = 0;
void* protection_address = 0;
DWORD protection_length = 0;