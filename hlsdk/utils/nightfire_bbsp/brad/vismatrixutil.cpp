#pragma warning(disable: 4018) //amckern - 64bit - '<' Singed/Unsigned Mismatch

#include "qrad.h"

#ifdef HLRAD_HULLU
#define HLRAD_HULLU_COMPRESSEDARRAY_TWEAK
//#undef HLRAD_HULLU_COMPRESSEDARRAY_TWEAK
#endif

funcCheckVisBit g_CheckVisBit = NULL;

unsigned        g_total_transfer = 0;
unsigned        g_transfer_index_bytes = 0;
unsigned        g_transfer_data_bytes = 0;

#define COMPRESSED_TRANSFERS
//#undef  COMPRESSED_TRANSFERS

int             FindTransferOffsetPatchnum(transfer_index_t* tIndex, const patch_t* const patch, const unsigned patchnum)
{
    //
    // binary search for match
    //
    int             low = 0;
    int             high = patch->iIndex - 1;
    int             offset;

    while (1)
    {
        offset = (low + high) / 2;

        if ((tIndex[offset].index + tIndex[offset].size) < patchnum)
        {
            low = offset + 1;
        }
        else if (tIndex[offset].index > patchnum)
        {
            high = offset - 1;
        }
        else
        {
            unsigned        x;
            unsigned int    rval = 0;
            transfer_index_t* pIndex = tIndex;

            for (x = 0; x < offset; x++, pIndex++)
            {
                rval += pIndex->size + 1;
            }
            rval += patchnum - tIndex[offset].index;
            return rval;
        }
        if (low > high)
        {
            return -1;
        }
    }
}

#ifdef COMPRESSED_TRANSFERS

static unsigned GetLengthOfRun(const transfer_raw_index_t* raw, const transfer_raw_index_t* const end)
{
    unsigned        run_size = 0;

    while (raw < end)
    {
        if (((*raw) + 1) == (*(raw + 1)))
        {
            raw++;
            run_size++;

            if (run_size >= MAX_COMPRESSED_TRANSFER_INDEX_SIZE)
            {
                return run_size;
            }
        }
        else
        {
            return run_size;
        }
    }
    return run_size;
}

#ifdef HLRAD_HULLU_COMPRESSEDARRAY_TWEAK
static transfer_index_t* CompressTransferIndicies(transfer_raw_index_t* tRaw, const unsigned rawSize, unsigned* iSize, transfer_index_t *CompressedArray)
#else
static transfer_index_t* CompressTransferIndicies(transfer_raw_index_t* tRaw, const unsigned rawSize, unsigned* iSize)
#endif
{
    unsigned        x;
    unsigned        size = rawSize;
    unsigned        compressed_count = 0;

    transfer_raw_index_t* raw = tRaw;
    transfer_raw_index_t* end = tRaw + rawSize - 1;        // -1 since we are comparing current with next and get errors when bumping into the 'end'

#ifndef HLRAD_HULLU_COMPRESSEDARRAY_TWEAK
    transfer_index_t CompressedArray[MAX_PATCHES];         // somewhat big stack object (1 Mb with 256k patches)
#endif
    transfer_index_t* compressed = CompressedArray;

    for (x = 0; x < size; x++, raw++, compressed++)
    {
        compressed->index = (*raw);
        compressed->size = GetLengthOfRun(raw, end);       // Zero based (count 0 still implies 1 item in the list, so 256 max entries result)
        raw += compressed->size;
        x += compressed->size;
        compressed_count++;                                // number of entries in compressed table
    }

    *iSize = compressed_count;

    if (compressed_count)
    {
        unsigned        compressed_array_size = sizeof(transfer_index_t) * compressed_count;
        transfer_index_t* rval = (transfer_index_t*)AllocBlock(compressed_array_size);

        ThreadLock();
        g_transfer_index_bytes += compressed_array_size;
        ThreadUnlock();

        memcpy(rval, CompressedArray, compressed_array_size);
        return rval;
    }
    else
    {
        return NULL;
    }
}

#else

#ifdef HLRAD_HULLU_COMPRESSEDARRAY_TWEAK
static transfer_index_t* CompressTransferIndicies(const transfer_raw_index_t* tRaw, const unsigned rawSize, unsigned* iSize, transfer_index_t *CompressedArray)
#else
static transfer_index_t* CompressTransferIndicies(const transfer_raw_index_t* tRaw, const unsigned rawSize, unsigned* iSize)
#endif
{
    unsigned        x;
    unsigned        size = rawSize;
    unsigned        compressed_count = 0;

    transfer_raw_index_t* raw = tRaw;
    transfer_raw_index_t* end = tRaw + rawSize;

#ifndef HLRAD_HULLU_COMPRESSEDARRAY_TWEAK
    transfer_index_t CompressedArray[MAX_PATCHES];         // somewhat big stack object (1 Mb with 256k patches)
#endif
    transfer_index_t* compressed = CompressedArray;

    for (x = 0; x < size; x++, raw++, compressed++)
    {
        compressed->index = (*raw);
        compressed->size = 0;
        compressed_count++;                                // number of entries in compressed table
    }

    *iSize = compressed_count;

    if (compressed_count)
    {
        unsigned        compressed_array_size = sizeof(transfer_index_t) * compressed_count;
        transfer_index_t* rval = AllocBlock(compressed_array_size);

        ThreadLock();
        g_transfer_index_bytes += compressed_array_size;
        ThreadUnlock();

        memcpy(rval, CompressedArray, compressed_array_size);
        return rval;
    }
    else
    {
        return NULL;
    }
}
#endif

/*
 * =============
 * MakeScales
 * 
 * This is the primary time sink.
 * It can be run multi threaded.
 * =============
 */
#ifdef SYSTEM_WIN32
#pragma warning(push)
#pragma warning(disable: 4100)                             // unreferenced formal parameter
#endif
void            MakeScales(const int threadnum)
{
    int             i;
    unsigned        j;
    vec3_t          delta;
    vec_t           dist;
    int             count;
    float           trans;
    patch_t*        patch;
    patch_t*        patch2;
    float           send;
    vec3_t          origin;
    vec_t           area;
    const vec_t*    normal1;
    const vec_t*    normal2;

#ifdef HLRAD_HULLU
    unsigned int    fastfind_index;
    vec3_t          transparency;
#endif

    vec_t           total;

    transfer_raw_index_t* tIndex;
    transfer_data_t* tData;

    transfer_raw_index_t* tIndex_All = (transfer_raw_index_t*)AllocBlock(sizeof(transfer_index_t) * MAX_PATCHES);
    transfer_data_t* tData_All = (transfer_data_t*)AllocBlock(sizeof(transfer_data_t) * MAX_PATCHES);

#ifdef HLRAD_HULLU_COMPRESSEDARRAY_TWEAK
    //Simple optimization.. CompressTransferIndicies stack allocs 1MB object on ever run .. that's slooow :)
    //and some compilers cannot even make it work (Segfaults). Create array here and pass to CompressTransferIndicies
    transfer_index_t *CompressedArray = (transfer_index_t *)AllocBlock(sizeof(transfer_index_t) * MAX_PATCHES);
#endif

    count = 0;

#ifdef HLRAD_HULLU
    fastfind_index = 0;
#endif

    while (1)
    {
        i = GetThreadWork();
        if (i == -1)
            break;

        patch = g_patches + i;
        patch->iIndex = 0;
        patch->iData = 0;

        total = 0.0;

        tIndex = tIndex_All;
        tData = tData_All;

        VectorCopy(patch->origin, origin);
        normal1 = getPlaneFromFaceNumber(patch->faceNumber)->normal;

        area = patch->area;

        // find out which patch2's will collect light
        // from patch

        for (j = 0, patch2 = g_patches; j < g_num_patches; j++, patch2++)
        {
            vec_t           dot1;
            vec_t           dot2;

#ifdef HLRAD_HULLU
            VectorFill(transparency,1.0);
            if (!g_CheckVisBit(i, j, transparency, fastfind_index) || (i == j))
#else
            if (!g_CheckVisBit(i, j) || (i == j))
#endif
            {
                continue;
            }

            normal2 = getPlaneFromFaceNumber(patch2->faceNumber)->normal;

            // calculate transferemnce
            VectorSubtract(patch2->origin, origin, delta);

            dist = VectorNormalize(delta);
            dot1 = DotProduct(delta, normal1);
            dot2 = -DotProduct(delta, normal2);

            trans = (dot1 * dot2) / (dist * dist);         // Inverse square falloff factoring angle between patch normals

#ifdef HLRAD_HULLU
            trans = trans * VectorAvg(transparency); //hullu: add transparency effect
#endif

            if (trans >= 0)
            {
                send = trans * patch2->area;

                // Caps light from getting weird
                if (send > 0.4f)
                {
                    trans = 0.4f / patch2->area;
                    send = 0.4f;
                }

                total += send;

                // scale to 16 bit (black magic)
                trans = trans * area * INVERSE_TRANSFER_SCALE;
                if (trans >= TRANSFER_SCALE_MAX)
                {
                    trans = TRANSFER_SCALE_MAX;
                }
            }
            else
            {
#if 0
                Warning("transfer < 0 (%f): dist=(%f)\n"
                        "   dot1=(%f) patch@(%4.3f %4.3f %4.3f) normal(%4.3f %4.3f %4.3f)\n"
                        "   dot2=(%f) patch@(%4.3f %4.3f %4.3f) normal(%4.3f %4.3f %4.3f)\n",
                        trans, dist,
                        dot1, patch->origin[0], patch->origin[1], patch->origin[2], patch->normal[0], patch->normal[1],
                        patch->normal[2], dot2, patch2->origin[0], patch2->origin[1], patch2->origin[2],
                        patch2->normal[0], patch2->normal[1], patch2->normal[2]);
#endif
                trans = 0.0;
            }

            *tData = trans;
            *tIndex = j;
            tData++;
            tIndex++;
            patch->iData++;
            count++;
        }

        // copy the transfers out
        if (patch->iData)
        {
            unsigned        data_size = patch->iData * sizeof(transfer_data_t);

            patch->tData = (transfer_data_t*)AllocBlock(data_size);

#ifdef HLRAD_HULLU_COMPRESSEDARRAY_TWEAK
            patch->tIndex = CompressTransferIndicies(tIndex_All, patch->iData, &patch->iIndex, CompressedArray);
#else
            patch->tIndex = CompressTransferIndicies(tIndex_All, patch->iData, &patch->iIndex);
#endif

            hlassume(patch->tData != NULL, assume_NoMemory);
            hlassume(patch->tIndex != NULL, assume_NoMemory);

            ThreadLock();
            g_transfer_data_bytes += data_size;
            ThreadUnlock();

            //
            // normalize all transfers so exactly 50% of the light
            // is transfered to the surroundings
            //

            total = 0.5 / total;
            {
                unsigned        x;
                transfer_data_t* t1 = patch->tData;
                transfer_data_t* t2 = tData_All;

                for (x = 0; x < patch->iData; x++, t1++, t2++)
                {
                    (*t1) = (*t2) * total;
                }
            }
        }
    }

    FreeBlock(tIndex_All);
    FreeBlock(tData_All);

#ifdef HLRAD_HULLU_COMPRESSEDARRAY_TWEAK
    FreeBlock( CompressedArray );
#endif

    ThreadLock();
    g_total_transfer += count;
    ThreadUnlock();
}

#ifdef SYSTEM_WIN32
#pragma warning(pop)
#endif

/*
 * =============
 * SwapTransfersTask
 * 
 * Change transfers from light sent out to light collected in.
 * In an ideal world, they would be exactly symetrical, but
 * because the form factors are only aproximated, then normalized,
 * they will actually be rather different.
 * =============
 */
void            SwapTransfers(const int patchnum)
{
    patch_t*        patch = &g_patches[patchnum];
    transfer_index_t* tIndex = patch->tIndex;
    transfer_data_t* tData = patch->tData;
    unsigned        x;

    for (x = 0; x < patch->iIndex; x++, tIndex++)
    {
        unsigned        size = (tIndex->size + 1);
        unsigned        patchnum2 = tIndex->index;
        unsigned        y;

        for (y = 0; y < size; y++, tData++, patchnum2++)
        {
            patch_t*        patch2 = &g_patches[patchnum2];

            if (patchnum2 > patchnum)
            {                                              // done with this list
                return;
            }
            else if (!patch2->iData)
            {                                              // Set to zero in this impossible case
                Log("patch2 has no iData\n");
                (*tData) = 0;
                continue;
            }
            else
            {
                transfer_index_t* tIndex2 = patch2->tIndex;
                transfer_data_t* tData2 = patch2->tData;
                int             offset = FindTransferOffsetPatchnum(tIndex2, patch2, patchnum);

                if (offset >= 0)
                {
                    transfer_data_t tmp = *tData;

                    *tData = tData2[offset];
                    tData2[offset] = tmp;
                }
                else
                {                                          // Set to zero in this impossible case
                    Log("FindTransferOffsetPatchnum returned -1 looking for patch %d in patch %d's transfer lists\n",
                        patchnum, patchnum2);
                    (*tData) = 0;
                    return;
                }
            }
        }
    }
}

#ifdef HLRAD_HULLU
/*
 * =============
 * MakeScales
 * 
 * This is the primary time sink.
 * It can be run multi threaded.
 * =============
 */
#ifdef SYSTEM_WIN32
#pragma warning(push)
#pragma warning(disable: 4100)                             // unreferenced formal parameter
#endif
void            MakeRGBScales(const int threadnum)
{
    int             i;
    unsigned        j;
    vec3_t          delta;
    vec_t           dist;
    int             count;
    float           trans[3];
    float           trans_one;
    patch_t*        patch;
    patch_t*        patch2;
    float           send;
    vec3_t          origin;
    vec_t           area;
    const vec_t*    normal1;
    const vec_t*    normal2;

    unsigned int    fastfind_index;
    vec3_t          transparency;

    vec_t           total;

    transfer_raw_index_t* tIndex;
    rgb_transfer_data_t* tRGBData;

    transfer_raw_index_t* tIndex_All = (transfer_raw_index_t*)AllocBlock(sizeof(transfer_index_t) * MAX_PATCHES);
    rgb_transfer_data_t* tRGBData_All = (rgb_transfer_data_t*)AllocBlock(sizeof(rgb_transfer_data_t) * MAX_PATCHES);

#ifdef HLRAD_HULLU_COMPRESSEDARRAY_TWEAK
    //Simple optimization.. CompressTransferIndicies stack allocs 1MB object on ever run .. that's slooow :)
    //and some compilers cannot even make it work (Segfaults). Create array here and pass to CompressTransferIndicies
    transfer_index_t *CompressedArray = (transfer_index_t *)AllocBlock(sizeof(transfer_index_t) * MAX_PATCHES);
#endif

    count = 0;

	fastfind_index = 0;


    while (1)
    {
        i = GetThreadWork();
        if (i == -1)
            break;

        patch = g_patches + i;
        patch->iIndex = 0;
        patch->iData = 0;

        total = 0.0;

        tIndex = tIndex_All;
        tRGBData = tRGBData_All;

        VectorCopy(patch->origin, origin);
        normal1 = getPlaneFromFaceNumber(patch->faceNumber)->normal;

        area = patch->area;

        // find out which patch2's will collect light
        // from patch

        for (j = 0, patch2 = g_patches; j < g_num_patches; j++, patch2++)
        {
            vec_t           dot1;
            vec_t           dot2;
            VectorFill( transparency, 1.0 );

            if (!g_CheckVisBit(i, j, transparency, fastfind_index) || (i == j))
            {
                continue;
            }

            normal2 = getPlaneFromFaceNumber(patch2->faceNumber)->normal;

            // calculate transferemnce
            VectorSubtract(patch2->origin, origin, delta);

            dist = VectorNormalize(delta);
            dot1 = DotProduct(delta, normal1);
            dot2 = -DotProduct(delta, normal2);

            trans_one = (dot1 * dot2) / (dist * dist);         // Inverse square falloff factoring angle between patch normals
            
            VectorFill(trans, trans_one);
            VectorMultiply(trans, transparency, trans); //hullu: add transparency effect

            if (VectorAvg(trans) >= 0)
            {
            	/////////////////////////////////////////RED
                send = trans[0] * patch2->area;
                // Caps light from getting weird
                if (send > 0.4f) 
                {
                    trans[0] = 0.4f / patch2->area;
                    send = 0.4f;
                }
                total += send / 3.0f;
                
            	/////////////////////////////////////////GREEN
                send = trans[1] * patch2->area;
                // Caps light from getting weird
                if (send > 0.4f) 
                {
                    trans[1] = 0.4f / patch2->area;
                    send = 0.4f;
                }
                total += send / 3.0f;

            	/////////////////////////////////////////BLUE
                send = trans[2] * patch2->area;
                // Caps light from getting weird
                if (send > 0.4f) 
                {
                    trans[2] = 0.4f / patch2->area;
                    send = 0.4f;
                }
                total += send / 3.0f;

                // scale to 16 bit (black magic)
                VectorScale(trans, area * INVERSE_TRANSFER_SCALE, trans);

                if (trans[0] >= TRANSFER_SCALE_MAX)
                {
                    trans[0] = TRANSFER_SCALE_MAX;
                }
                if (trans[1] >= TRANSFER_SCALE_MAX)
                {
                    trans[1] = TRANSFER_SCALE_MAX;
                }
                if (trans[2] >= TRANSFER_SCALE_MAX)
                {
                    trans[2] = TRANSFER_SCALE_MAX;
                }
            }
            else
            {
#if 0
                Warning("transfer < 0 (%4.3f %4.3f %4.3f): dist=(%f)\n"
                        "   dot1=(%f) patch@(%4.3f %4.3f %4.3f) normal(%4.3f %4.3f %4.3f)\n"
                        "   dot2=(%f) patch@(%4.3f %4.3f %4.3f) normal(%4.3f %4.3f %4.3f)\n",
                        trans[0], trans[1], trans[2], dist,
                        dot1, patch->origin[0], patch->origin[1], patch->origin[2], patch->normal[0], patch->normal[1],
                        patch->normal[2], dot2, patch2->origin[0], patch2->origin[1], patch2->origin[2],
                        patch2->normal[0], patch2->normal[1], patch2->normal[2]);
#endif
                VectorFill(trans,0.0);
            }

            VectorCopy(trans, *tRGBData);
            *tIndex = j;
            tRGBData++;
            tIndex++;
            patch->iData++;
            count++;
        }

        // copy the transfers out
        if (patch->iData)
        {
            unsigned data_size = patch->iData * sizeof(rgb_transfer_data_t);

            patch->tRGBData = (rgb_transfer_data_t*)AllocBlock(data_size);

#ifdef HLRAD_HULLU_COMPRESSEDARRAY_TWEAK
            patch->tIndex = CompressTransferIndicies(tIndex_All, patch->iData, &patch->iIndex, CompressedArray);
#else
            patch->tIndex = CompressTransferIndicies(tIndex_All, patch->iData, &patch->iIndex);
#endif

            hlassume(patch->tRGBData != NULL, assume_NoMemory);
            hlassume(patch->tIndex != NULL, assume_NoMemory);

            ThreadLock();
            g_transfer_data_bytes += data_size;
            ThreadUnlock();

            //
            // normalize all transfers so exactly 50% of the light
            // is transfered to the surroundings
            //

            total = 0.5 / total;
            {
                unsigned        x;
                rgb_transfer_data_t* t1 = patch->tRGBData;
                rgb_transfer_data_t* t2 = tRGBData_All;

                for (x = 0; x < patch->iData; x++, t1++, t2++)
                {
                     VectorScale( *t2, total, *t1 );
                }
            }
        }
    }

    FreeBlock(tIndex_All);
    FreeBlock(tRGBData_All);

#ifdef HLRAD_HULLU_COMPRESSEDARRAY_TWEAK
    FreeBlock( CompressedArray );
#endif

    ThreadLock();
    g_total_transfer += count;
    ThreadUnlock();
}

#ifdef SYSTEM_WIN32
#pragma warning(pop)
#endif

/*
 * =============
 * SwapTransfersTask
 * 
 * Change transfers from light sent out to light collected in.
 * In an ideal world, they would be exactly symetrical, but
 * because the form factors are only aproximated, then normalized,
 * they will actually be rather different.
 * =============
 */
void            SwapRGBTransfers(const int patchnum)
{
    patch_t*        		patch	= &g_patches[patchnum];
    transfer_index_t*		tIndex	= patch->tIndex;
    rgb_transfer_data_t* 	tRGBData= patch->tRGBData;
    unsigned        x;

    for (x = 0; x < patch->iIndex; x++, tIndex++)
    {
        unsigned        size = (tIndex->size + 1);
        unsigned        patchnum2 = tIndex->index;
        unsigned        y;

        for (y = 0; y < size; y++, tRGBData++, patchnum2++)
        {
            patch_t*        patch2 = &g_patches[patchnum2];

            if (patchnum2 > patchnum)
            {                                              // done with this list
                return;
            }
            else if (!patch2->iData)
            {                                              // Set to zero in this impossible case
                Log("patch2 has no iData\n");
                VectorFill(*tRGBData, 0);
                continue;
            }
            else
            {
                transfer_index_t* tIndex2 = patch2->tIndex;
                rgb_transfer_data_t* tRGBData2 = patch2->tRGBData;
                int             offset = FindTransferOffsetPatchnum(tIndex2, patch2, patchnum);

                if (offset >= 0)
                {
                    rgb_transfer_data_t tmp;
                    VectorCopy(*tRGBData, tmp)

                    VectorCopy(tRGBData2[offset], *tRGBData);
                    VectorCopy(tmp, tRGBData2[offset]);
                }
                else
                {                                          // Set to zero in this impossible case
                    Log("FindTransferOffsetPatchnum returned -1 looking for patch %d in patch %d's transfer lists\n",
                        patchnum, patchnum2);
                    VectorFill(*tRGBData, 0);
                    return;
                }
            }
        }
    }
}

#endif /*HLRAD_HULLU*/


#ifndef HLRAD_HULLU

void            DumpTransfersMemoryUsage()
{
    Log("Transfer Lists : %u transfers\n       Indices : %u bytes\n          Data : %u bytes\n",
        g_total_transfer, g_transfer_index_bytes, g_transfer_data_bytes);
}

#else

//More human readable numbers
void            DumpTransfersMemoryUsage()
{
	if(g_total_transfer > 1000*1000)
		Log("Transfer Lists : %11u : %7.2fM transfers\n", g_total_transfer, g_total_transfer/(1000.0f*1000.0f));
	else if(g_total_transfer > 1000)
		Log("Transfer Lists : %11u : %7.2fk transfers\n", g_total_transfer, g_total_transfer/1000.0f);
	else
		Log("Transfer Lists : %11u transfers\n", g_total_transfer);
	
	if(g_transfer_index_bytes > 1024*1024)
		Log("       Indices : %11u : %7.2fM bytes\n", g_transfer_index_bytes, g_transfer_index_bytes/(1024.0f * 1024.0f));
	else if(g_transfer_index_bytes > 1024)
		Log("       Indices : %11u : %7.2fk bytes\n", g_transfer_index_bytes, g_transfer_index_bytes/1024.0f);
	else
		Log("       Indices : %11u bytes\n", g_transfer_index_bytes);
	
	if(g_transfer_data_bytes > 1024*1024)
		Log("          Data : %11u : %7.2fM bytes\n", g_transfer_data_bytes, g_transfer_data_bytes/(1024.0f * 1024.0f));
	else if(g_transfer_data_bytes > 1024)
		Log("          Data : %11u : %7.2fk bytes\n", g_transfer_data_bytes, g_transfer_data_bytes/1024.0f);
	else
		Log("       Indices : %11u bytes\n", g_transfer_data_bytes);
}

#endif

