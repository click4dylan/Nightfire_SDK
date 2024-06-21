#include "globals.h"
#include "helpers.h"
#include "log.h"
#include "threads.h"
#include "cmdlib.h"
#include "bspfile.h"
#include "mapfile.h"

void __declspec(noreturn) Usage()
{
    Banner();
    Log("\n-= %s Options =-\n\n", g_Program);
    Log("    -onlyents      : do an entity update from .map to .bsp\n");
    Log("    -leakonly      : Run BSP only enough to check for LEAKs\n");
    Log("    -subdivide #   : Sets the face subdivide size\n");
#ifdef SUBDIVIDE
    Log("    -nosubdiv      : Disables face subdivision\n");
#endif
#ifdef MERGE
    Log("    -nomerge       : Disables face merging\n");
#endif
    Log("    -maxnodesize # : Sets the maximum portal node size\n\n");
#ifdef VARIABLE_LIGHTING_MAX_NODE_SIZE
    Log("    -lightingmaxnodesize # : Sets the maximum lighting portal node size\n\n");
#endif
    Log("    -showbevels    : export a .map with the expand brush generated bevels\n");
    Log("    -notjunc       : Don't break edges on t-junctions     (not for final runs)\n");
    Log("    -noclip        : Don't process the clipping hull      (not for final runs)\n");
    Log("    -nofill        : Don't fill outside (will mask LEAKs) (not for final runs)\n\n");
    Log("    -nolighting    : Do not build lighting bsp\n\n");
    Log("    -chart         : display bsp statitics\n");
    Log("    -low | -high   : run program an altered priority level\n");
    Log("    -nolog         : don't generate the compile logfiles\n");
    Log("    -blscale       : base lightmap scale\n");
    Log("    -ilscale       : incremental lightmap scale\n");
    Log("    -nowater       : do not build water models\n");
    Log("    -threads #     : manually specify the number of threads to run\n");
    Log("    -estimate      : display estimated time during compile\n");
    Log("    -verbose       : compile with verbose messages\n");
    Log("    -noinfo        : Do not show tool configuration information\n");
    Log("    -dev #         : compile with developer message\n\n");
    Log("    mapfile        : The mapfile to compile\n\n");
    exit(1);
}

void ProcessArguments(int argc, const char** argv)
{
    int i = 1;

    while (i < argc)
    {
        if (!_stricmp(argv[i], "-threads"))
        {
            if (++i >= argc)
            {
                Usage();
            }
            if (atol(argv[i]) < 1)
            {
                Log("Expected value of at least 1 for '-threads'\n");
                Usage();
            }
        }
        else if (!_stricmp(argv[i], "-notjunc"))
        {
            g_notjunc = true;
        }
#ifdef SUBDIVIDE
        else if (!_stricmp(argv[i], "-nosubdiv") || !_stricmp(argv[i], "-nosubdivide"))
        {
            g_nosubdiv = true;
        }
        else if (!_stricmp(argv[i], "-subdivide"))
        {
            if (++i >= argc)
            {
                Usage();
            }
            g_subdivide_size = atol(argv[i]);
            if (g_subdivide_size < 1)
            {
                Log("Expected value of at least 1 for '-subdivide'\n");
                Usage();
            }
        }
#endif
#ifdef MERGE
        else if (!_stricmp(argv[i], "-nomerge"))
        {
            g_nomerge = true;
        }
#endif
        else if (!_stricmp(argv[i], "-noclip"))
        {
            g_noclip = true;
        }
        else if (!_stricmp(argv[i], "-nofill"))
        {
            g_nofill = true;
        }
        else if (!_stricmp(argv[i], "-estimate"))
        {
            g_estimate = true;
        }
        else if (!_stricmp(argv[i], "-dev"))
        {
            if (++i >= argc)
            {
                Usage();
            }
            g_developer = (developer_level_t)atol(argv[i]);
        }
        else if (!_stricmp(argv[i], "-verbose"))
        {
            g_verbose = true;
        }
        else if (!_stricmp(argv[i], "-noinfo"))
        {
            g_info = false;
        }
        else if (!_stricmp(argv[i], "-leakonly"))
        {
            g_bLeakOnly = true;
        }
        else if (!_stricmp(argv[i], "-chart"))
        {
            g_chart = true;
        }
        else if (!_stricmp(argv[i], "-low"))
        {
            g_threadpriority = q_threadpriority::eThreadPriorityLow;
        }
        else if (!_stricmp(argv[i], "-high"))
        {
            g_threadpriority = q_threadpriority::eThreadPriorityHigh;
        }
        else if (!_stricmp(argv[i], "-nolog"))
        {
            g_log = false;
        }
        else if (!_stricmp(argv[i], "-nolighting"))
        {
            g_lighting = false;
        }
#ifdef VARIABLE_LIGHTING_MAX_NODE_SIZE
        else if (!_stricmp(argv[i], "-lightingmaxnodesize"))
        {
            if (++i >= argc)
            {
                Usage();
            }
            g_LightingMaxNodeSize = atol(argv[i]);
#ifdef UNCLAMP_MAX_NODE_SIZE
            if (g_LightingMaxNodeSize < 64 || g_LightingMaxNodeSize > 65536)
            {
                Warning("Lighting node size must be between 64 and 65536. Using default value 65536.");
                g_LightingMaxNodeSize = 65536;
            }
#else
            if (g_LightingMaxNodeSize < 64 || g_LightingMaxNodeSize > 4096)
            {
                Warning("Lighting node size must be between 64 and 4096. Using default value 4096.");
                g_LightingMaxNodeSize = 4096;
            }
#endif
        }
#endif
        else if (!_stricmp(argv[i], "-maxnodesize"))
        {
            if (++i >= argc)
            {
                Usage();
            }
            g_MaxNodeSize = atol(argv[i]);
#ifdef UNCLAMP_MAX_NODE_SIZE
            if (g_MaxNodeSize < 64 || g_MaxNodeSize > 65536)
            {
                Warning("Node size must be between 64 and 65536. Using default value 65536.");
                g_MaxNodeSize = 65536;
            }
#else
            if (g_MaxNodeSize < 64 || g_MaxNodeSize > 4096)
            {
                Warning("Node size must be between 64 and 4096. Using default value 4096.");
                g_MaxNodeSize = 4096;
            }
#endif
        }
        else if (!_stricmp(argv[i], "-blscale"))
        {
            if (++i >= argc)
            {
                Usage();
            }
            g_blscale = atol(argv[i]);
#ifdef UNCLAMP_LIGHTMAP_SCALE
            if (g_blscale < 1)
            {
                Log("Expected value of at least 1 for '-blscale'\n");
                Usage();
            }
#else
            if (g_blscale < 16)
            {
                Log("Expected value of at least 16 for '-blscale'\n");
                Usage();
            }
#endif
        }
        else if (!_stricmp(argv[i], "-ilscale"))
        {
            if (++i >= argc)
            {
                Usage();
            }
            g_ilscale = atol(argv[i]);

#ifdef UNCLAMP_LIGHTMAP_SCALE
            if (g_ilscale < 1)
            {
                Log("Expected value of at least 1 for '-ilscale'\n");
                Usage();
            }
#else
            if (g_ilscale < 16)
            {
                Log("Expected value of at least 16 for '-ilscale'\n");
                Usage();
            }
#endif
        }
        else if (!_stricmp(argv[i], "-onlyents"))
        {
            g_onlyents = true;
        }
        else if (!_stricmp(argv[i], "-nowater"))
        {
            g_water = false;
        }
        else if (!_stricmp(argv[i], "-showbevels"))
        {
            g_showbevels = true;
        }
        else
        {
            if (*argv[i] == '-')
            {
                Log("Unknown option \"%s\"\n", argv[i]);
                Usage();
            }
            safe_strncpy(g_Mapname, argv[i], MAX_PATH);
        }
        ++i;
    }
}

void UnlinkFiles()
{
    char FileName[MAX_PATH];

    if (g_log)
    {
        safe_snprintf(FileName, MAX_PATH, "%s.bsp", g_Mapname);
        _unlink(FileName);
        safe_snprintf(FileName, MAX_PATH, "%s.inc", g_Mapname);
        _unlink(FileName);
        safe_snprintf(FileName, MAX_PATH, "%s.p0", g_Mapname);
        _unlink(FileName);
        safe_snprintf(FileName, MAX_PATH, "%s.p1", g_Mapname);
        _unlink(FileName);
        safe_snprintf(FileName, MAX_PATH, "%s.p2", g_Mapname);
        _unlink(FileName);
        safe_snprintf(FileName, MAX_PATH, "%s.p3", g_Mapname);
        _unlink(FileName);
        safe_snprintf(FileName, MAX_PATH, "%s.prt", g_Mapname);
        _unlink(FileName);
        safe_snprintf(FileName, MAX_PATH, "%s.pts", g_Mapname);
        _unlink(FileName);
        safe_snprintf(FileName, MAX_PATH, "%s.lin", g_Mapname);
        _unlink(FileName);
        safe_snprintf(FileName, MAX_PATH, "%s.wic", g_Mapname);
        _unlink(FileName);
    }
}

int main(int argc, const char** argv, const char** envp)
{
#ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_CHECK_ALWAYS_DF);
#endif

    g_Program = "bbsp";

    if (argc == 1)
    {
        Usage();
    }

    ProcessArguments(argc, argv);

    if (!g_Mapname[0])
    {
        Log("No mapfile specified\n");
        Usage();
    }

    //safe_snprintf(g_Mapname, sizeof(g_Mapname), g_Mapname);

    FlipSlashes(g_Mapname);
    StripExtension(g_Mapname);

    if (!g_onlyents)
        UnlinkFiles();

    ResetErrorLog();
    ResetLog();
    OpenLog(g_clientid);
    atexit(CloseLog);
    ThreadSetDefault();
    ThreadSetPriority(g_threadpriority);
    LogStart(argc, argv);
    Settings();

    // delete existing files
    safe_snprintf(g_portfilename, sizeof(g_portfilename), "%s.prt", g_Mapname);
    _unlink(g_portfilename);

    safe_snprintf(g_pointfilename, sizeof(g_pointfilename), "%s.pts", g_Mapname);
    _unlink(g_pointfilename);

    safe_snprintf(g_linefilename, sizeof(g_linefilename), "%s.lin", g_Mapname);
    _unlink(g_linefilename);

    safe_snprintf(g_MapFileName, sizeof(g_MapFileName), "%s.map", g_Mapname);
    safe_snprintf(g_bspfilename, sizeof(g_bspfilename), "%s.bsp", g_Mapname);

    double startTime = I_FloatTime();

    mapinfo_t* mapfile;
    if (g_onlyents)
    {
        LoadBSPFile(g_bspfilename);
        mapfile = LoadMapFile(g_MapFileName);
        if (g_chart)
            PrintBSPFileSizes();
    }
    else
    {
        mapfile = LoadMapFile(g_MapFileName);
        StartCreatingBSP(mapfile);
    }

    WriteBSP(mapfile, g_bspfilename);

#ifdef FIX_NORMALS_LUMP
    //temporary 
#endif

    delete mapfile;

    LogTimeElapsed(I_FloatTime() - startTime);
    PrintAllocationData("Final allocation status (should all be 0)\n");

    return 0;
}