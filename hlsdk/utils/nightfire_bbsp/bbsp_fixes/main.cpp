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
    Log("    -maxnodesize # : Sets the maximum portal node size\n\n");
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
            g_notjunc = 1;
        }
        else if (!_stricmp(argv[i], "-noclip"))
        {
            g_noclip = 1;
        }
        else if (!_stricmp(argv[i], "-nofill"))
        {
            g_nofill = 1;
        }
        else if (!_stricmp(argv[i], "-estimate"))
        {
            g_estimate = 1;
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
            g_verbose = 1;
        }
        else if (!_stricmp(argv[i], "-noinfo"))
        {
            g_info = 0;
        }
        else if (!_stricmp(argv[i], "-leakonly"))
        {
            g_bLeakOnly = 1;
        }
        else if (!_stricmp(argv[i], "-chart"))
        {
            g_chart = 1;
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
            g_log = 0;
        }
        else if (!_stricmp(argv[i], "-nolighting"))
        {
            g_lighting = 0;
        }
        else if (!_stricmp(argv[i], "-maxnodesize"))
        {
            if (++i >= argc)
            {
                Usage();
            }
            g_MaxNodeSize = atol(argv[i]);
            if (g_MaxNodeSize < 64 || g_MaxNodeSize > 4096)
            {
                Warning("Node size must be between 64 and 4096. Using default value 4096.");
                g_MaxNodeSize = 4096;
            }
        }
        else if (!_stricmp(argv[i], "-blscale"))
        {
            if (++i >= argc)
            {
                Usage();
            }
            g_blscale = atol(argv[i]);
            if (g_blscale < 16)
            {
                Log("Expected value of at least 16 for '-blscale'\n");
                Usage();
            }
        }
        else if (!_stricmp(argv[i], "-ilscale"))
        {
            if (++i >= argc)
            {
                Usage();
            }
            g_ilscale = atol(argv[i]);
            if (g_ilscale < 16)
            {
                Log("Expected value of at least 16 for '-ilscale'\n");
                Usage();
            }
        }
        else if (!_stricmp(argv[i], "-onlyents"))
        {
            g_onlyents = 1;
        }
        else if (!_stricmp(argv[i], "-nowater"))
        {
            g_water = 0;
        }
        else if (!_stricmp(argv[i], "-showbevels"))
        {
            g_showbevels = 1;
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

void __stdcall UnlinkFiles()
{
    char FileName[260]; // [esp+0h] [ebp-104h] BYREF

    if (g_log)
    {
        safe_snprintf(FileName, 260u, "%s.bsp", g_Mapname);
        _unlink(FileName);
        safe_snprintf(FileName, 260u, "%s.inc", g_Mapname);
        _unlink(FileName);
        safe_snprintf(FileName, 260u, "%s.p0", g_Mapname);
        _unlink(FileName);
        safe_snprintf(FileName, 260u, "%s.p1", g_Mapname);
        _unlink(FileName);
        safe_snprintf(FileName, 260u, "%s.p2", g_Mapname);
        _unlink(FileName);
        safe_snprintf(FileName, 260u, "%s.p3", g_Mapname);
        _unlink(FileName);
        safe_snprintf(FileName, 260u, "%s.prt", g_Mapname);
        _unlink(FileName);
        safe_snprintf(FileName, 260u, "%s.pts", g_Mapname);
        _unlink(FileName);
        safe_snprintf(FileName, 260u, "%s.lin", g_Mapname);
        _unlink(FileName);
        safe_snprintf(FileName, 260u, "%s.wic", g_Mapname);
        _unlink(FileName);
    }
}

int __cdecl main(int argc, const char** argv, const char** envp)
{
#ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_CHECK_ALWAYS_DF);
#endif

    double startTime, endTime;
    entinfo_t* entinfo;

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

    FixSlashes(g_Mapname);
    StripFileExtension(g_Mapname);

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

    safe_snprintf(g_portfilename, sizeof(g_portfilename), "%s.prt", g_Mapname);
    _unlink(g_portfilename);
    safe_snprintf(g_pointfilename, sizeof(g_pointfilename), "%s.pts", g_Mapname);
    _unlink(g_pointfilename);
    safe_snprintf(g_linefilename, sizeof(g_linefilename), "%s.lin", g_Mapname);
    _unlink(g_linefilename);
    safe_snprintf(g_MapFileName, sizeof(g_MapFileName), "%s.map", g_Mapname);
    safe_snprintf(g_bspfilename, sizeof(g_bspfilename), "%s.bsp", g_Mapname);

    startTime = I_FloatTime();

    if (g_onlyents)
    {
        LoadBSPFile(g_bspfilename);
        entinfo = LoadMapFile(g_MapFileName);
        if (g_chart)
        {
            PrintBSPFileSizes();
        }
    }
    else
    {
        entinfo = LoadMapFile(g_MapFileName);
        StartCreatingBSP(entinfo);
    }

    WriteBSP(entinfo, g_bspfilename);
    delete entinfo;

    endTime = I_FloatTime();
    LogTimeElapsed(endTime - startTime);
    PrintAllocationData("Final allocation status (should all be 0)\n");

    return 0;
}