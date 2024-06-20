#include "brad.h"

// =====================================================================================
//  CheckVisBit
// =====================================================================================
#ifdef HLRAD_HULLU
static bool     CheckVisBitNoVismatrix(unsigned patchnum1, unsigned patchnum2, vec3_t &transparency_out, unsigned int &)
#else
static bool     CheckVisBitNoVismatrix(unsigned patchnum1, unsigned patchnum2)
#endif
{
#ifdef HLRAD_HULLU
    // This fix was in vismatrix and sparse methods but not in nomatrix
    // Without this nomatrix causes SwapTransfers output lots of errors
    if (patchnum1 > patchnum2)
    {
        const unsigned a = patchnum1;
        const unsigned b = patchnum2;
        patchnum1 = b;
        patchnum2 = a;
    }
    
    if (patchnum1 > g_num_patches)
    {
        Warning("in CheckVisBit(), patchnum1 > num_patches");
    }
    if (patchnum2 > g_num_patches)
    {
        Warning("in CheckVisBit(), patchnum2 > num_patches");
    }
#endif
	
    patch_t*        patch = &g_patches[patchnum1];
    patch_t*        patch2 = &g_patches[patchnum2];
	vec3_t			transparency;

#ifdef HLRAD_HULLU
    VectorFill(transparency_out, 1.0);
#endif

    // if emitter is behind that face plane, skip all patches

    if (patch2)
    {
        const dplane_t* plane2 = getPlaneFromFaceNumber(patch2->faceNumber);

        if (DotProduct(patch->origin, plane2->normal) > (PatchPlaneDist(patch2) + MINIMUM_PATCH_DISTANCE))
        {
            // we need to do a real test

            const dplane_t* plane = getPlaneFromFaceNumber(patch->faceNumber);

            // check vis between patch and patch2
            //  if v2 is not behind light plane
            //  && v2 is visible from v1
#ifdef HLRAD_HULLU
			int facenum = TestSegmentAgainstOpaqueList(patch->origin, patch2->origin, transparency);
#else
			int facenum = TestSegmentAgainstOpaqueList(patch->origin, patch2->origin);
#endif
            if ((facenum < 0 || facenum == patch2->faceNumber)
                && (DotProduct(patch2->origin, plane->normal) > (PatchPlaneDist(patch) + MINIMUM_PATCH_DISTANCE))
                && (TestLine_r(0, patch->origin, patch2->origin) == CONTENTS_EMPTY))
            {
#ifdef HLRAD_HULLU            	
            	if(g_customshadow_with_bouncelight)
            	{
            		VectorCopy(transparency, transparency_out);
            	}
#endif
                return true;
            }
        }
    }

    return false;
}

//
// end old vismat.c
////////////////////////////

void            MakeScalesNoVismatrix()
{
    char            transferfile[_MAX_PATH];

    hlassume(g_num_patches < MAX_PATCHES, assume_MAX_PATCHES);

    safe_strncpy(transferfile, g_source, _MAX_PATH);
    StripExtension(transferfile);
    DefaultExtension(transferfile, ".inc");

    if (!g_incremental || !readtransfers(transferfile, g_num_patches))
    {
        g_CheckVisBit = CheckVisBitNoVismatrix;
#ifndef HLRAD_HULLU
        NamedRunThreadsOn(g_num_patches, g_estimate, MakeScales);
#else
	if(g_rgb_transfers)
		{NamedRunThreadsOn(g_num_patches, g_estimate, MakeRGBScales);}
	else
		{NamedRunThreadsOn(g_num_patches, g_estimate, MakeScales);}
#endif

        // invert the transfers for gather vs scatter
#ifndef HLRAD_HULLU
        NamedRunThreadsOnIndividual(g_num_patches, g_estimate, SwapTransfers);
#else
	if(g_rgb_transfers)
		{NamedRunThreadsOnIndividual(g_num_patches, g_estimate, SwapRGBTransfers);}
	else
		{NamedRunThreadsOnIndividual(g_num_patches, g_estimate, SwapTransfers);}
#endif
        if (g_incremental)
        {
            writetransfers(transferfile, g_num_patches);
        }
        else
        {
            _unlink(transferfile);
        }
        DumpTransfersMemoryUsage();
    }
}
