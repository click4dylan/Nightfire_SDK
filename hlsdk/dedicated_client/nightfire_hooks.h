#pragma once

struct model_s;

namespace nf_hooks
{
	extern model_s* R_StudioSetupPlayerModel(int playerindex);
	extern bool UTIL_CheckForWater(float startx, float starty, float startz, float endx, float endy, float endz, float* dest);
}