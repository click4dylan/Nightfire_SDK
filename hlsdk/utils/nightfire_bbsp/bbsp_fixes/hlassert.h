#ifndef HLASSERT_H__
#define HLASSERT_H__

#if _MSC_VER >= 1000
#pragma once
#endif

#ifdef _DEBUG

#include "log.h"

#define assume(exp, message) {if (!(exp)) {Log("\n***** ERROR *****\nAssume '%s' failed\n at %s:%d\n %s\n\n", #exp, __FILE__, __LINE__, message);  __asm{int 3} }}
#define hlassert(exp) assume(exp, "")

#else // _DEBUG

#define assume(exp, message) {if (!(exp)) {Error("\nAssume '%s' failed\n at %s:%d\n %s\n\n", #exp, __FILE__, __LINE__, message);}}
#define hlassert(exp)

#endif // _DEBUG

#endif // SYSTEM_POSIX HLASSERT_H__