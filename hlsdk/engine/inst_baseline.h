#pragma once

#include "entity_state.h"

const int NUM_BASELINES = 64;

typedef struct extra_baselines_s
{
	int number;
	int classname[NUM_BASELINES];
	entity_state_t baseline[NUM_BASELINES];
} extra_baselines_t;