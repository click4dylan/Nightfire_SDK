#pragma once

#if !defined(CACHE_USER) && !defined(QUAKEDEF_H)
#define CACHE_USER
typedef struct cache_user_s
{
	void* data = nullptr;
} cache_user_t;
#endif