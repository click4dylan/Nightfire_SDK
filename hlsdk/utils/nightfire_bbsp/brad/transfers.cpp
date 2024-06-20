#pragma warning(disable: 4267) // amckern - 64bit - 'size_t' to 'unsigned int'                      // identifier was truncated to '255' characters in the browser information

#include "qrad.h"

#ifdef SYSTEM_WIN32
#include <sys/stat.h>
#include <fcntl.h>
#include "win32fix.h"
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

/*
 * =============
 * writetransfers
 * =============
 */

void            writetransfers(const char* const transferfile, const long total_patches)
{
    FILE           *file;

    file = fopen(transferfile, "w+b");
    if (file != NULL)
    {
        unsigned        amtwritten;
        patch_t*        patch;

        Log("Writing transfers file [%s]\n", transferfile);

        amtwritten = fwrite(&total_patches, sizeof(total_patches), 1, file);
        if (amtwritten != 1)
        {
            goto FailedWrite;
        }

        long patchcount = total_patches;
        for (patch = g_patches; patchcount-- > 0; patch++)
        {
            amtwritten = fwrite(&patch->iIndex, sizeof(patch->iIndex), 1, file);
            if (amtwritten != 1)
            {
                goto FailedWrite;
            }

            if (patch->iIndex)
            {
                amtwritten = fwrite(patch->tIndex, sizeof(transfer_index_t), patch->iIndex, file);
                if (amtwritten != patch->iIndex)
                {
                    goto FailedWrite;
                }
            }

            amtwritten = fwrite(&patch->iData, sizeof(patch->iData), 1, file);
            if (amtwritten != 1)
            {
                goto FailedWrite;
            }
            if (patch->iData)
            {
#ifdef HLRAD_HULLU
		if(g_rgb_transfers)
		{
			amtwritten = fwrite(patch->tRGBData, sizeof(rgb_transfer_data_t), patch->iData, file);
		}
		else
		{
			amtwritten = fwrite(patch->tData, sizeof(transfer_data_t), patch->iData, file);
		}
#else
                amtwritten = fwrite(patch->tData, sizeof(transfer_data_t), patch->iData, file);
#endif
                if (amtwritten != patch->iData)
                {
                    goto FailedWrite;
                }
            }
        }

        fclose(file);
    }
    else
    {
        Error("Failed to open incremenetal file [%s] for writing\n", transferfile);
    }
    return;

  FailedWrite:
    fclose(file);
    _unlink(transferfile);
    Warning("Failed to generate incremental file [%s] (probably ran out of disk space)\n");
}

/*
 * =============
 * readtransfers
 * =============
 */

bool            readtransfers(const char* const transferfile, const long numpatches)
{
    FILE*           file;
    long            total_patches;

    file = fopen(transferfile, "rb");
    if (file != NULL)
    {
        unsigned        amtread;
        patch_t*        patch;

        Log("Reading transfers file [%s]\n", transferfile);

        amtread = fread(&total_patches, sizeof(total_patches), 1, file);
        if (amtread != 1)
        {
            goto FailedRead;
        }
        if (total_patches != numpatches)
        {
            goto FailedRead;
        }

        long patchcount = total_patches;
        for (patch = g_patches; patchcount-- > 0; patch++)
        {
            amtread = fread(&patch->iIndex, sizeof(patch->iIndex), 1, file);
            if (amtread != 1)
            {
                goto FailedRead;
            }
            if (patch->iIndex)
            {
                patch->tIndex = (transfer_index_t*)AllocBlock(patch->iIndex * sizeof(transfer_index_t *));
                hlassume(patch->tIndex != NULL, assume_NoMemory);
                amtread = fread(patch->tIndex, sizeof(transfer_index_t), patch->iIndex, file);
                if (amtread != patch->iIndex)
                {
                    goto FailedRead;
                }
            }

            amtread = fread(&patch->iData, sizeof(patch->iData), 1, file);
            if (amtread != 1)
            {
                goto FailedRead;
            }
            if (patch->iData)
            {
#ifdef HLRAD_HULLU
		if(g_rgb_transfers)
		{
                    patch->tRGBData = (rgb_transfer_data_t*)AllocBlock(patch->iData * sizeof(rgb_transfer_data_t *));
                    hlassume(patch->tRGBData != NULL, assume_NoMemory);
                    amtread = fread(patch->tRGBData, sizeof(rgb_transfer_data_t), patch->iData, file);		    
		}
		else
		{
                    patch->tData = (transfer_data_t*)AllocBlock(patch->iData * sizeof(transfer_data_t *));
                    hlassume(patch->tData != NULL, assume_NoMemory);
                    amtread = fread(patch->tData, sizeof(transfer_data_t), patch->iData, file);		    
		}
#else
                patch->tData = (transfer_data_t*)AllocBlock(patch->iData * sizeof(transfer_data_t *));
                hlassume(patch->tData != NULL, assume_NoMemory);
                amtread = fread(patch->tData, sizeof(transfer_data_t), patch->iData, file);
#endif
                if (amtread != patch->iData)
                {
                    goto FailedRead;
                }
            }
        }

        fclose(file);
        Warning("Finished reading transfers file [%s] %d\n", transferfile);
        return true;
    }
    Warning("Failed to open transfers file [%s]\n", transferfile);
    return false;

  FailedRead:
    {
        unsigned        x;
        patch_t*        patch = g_patches;

        for (x = 0; x < g_num_patches; x++, patch++)
        {
            FreeBlock(patch->tData);
            FreeBlock(patch->tIndex);
            patch->iData = 0;
            patch->iIndex = 0;
            patch->tData = NULL;
            patch->tIndex = NULL;
        }
    }
    fclose(file);
    _unlink(transferfile);
    return false;
}
