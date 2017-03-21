#include <unistd.h>
#include <string.h>
#include <time.h>

#include "../editor/version.h"
#include "../editor/scene.h"
#include "../editor/parser.h"
#include "../editor/config.h"
#include "../editor/timers.h"
#include "../editor/simulation.h"
#include "../editor/statistics.h"
#include "../editor/model.h"


#define PROG_DIR_WIN "/Motpuca"
#define PROG_DIR_WIN_DBG "/Moje dokumenty/Projekty/Motpuca/svn"
#define PROG_DIR_UNIX "/usr/local/Motpuca"
#define PROG_DIR_UNIX_DBG "/#motpuca/trunk/src/editor-build-desktop"

#define HOME_DIR_WIN "/Moje dokumenty/Motpuca"
#define HOME_DIR_WIN_DBG "/Moje dokumenty/Projekty/Motpuca/svn/home"
#define HOME_DIR_UNIX "/Motpuca"
#define HOME_DIR_UNIX_DBG "/#motpuca/trunk/src/editor-build-desktop/home-motpuca"


void setup_directories()
{
#ifdef unix
    strcpy(GlobalSettings.temp_dir, "/tmp/");
    if (GlobalSettings.run_env != reProduction)
    {
        snprintf(GlobalSettings.app_dir, P_MAX_PATH, "%s%s", getenv("HOME"), PROG_DIR_UNIX_DBG);
        snprintf(GlobalSettings.user_dir, P_MAX_PATH, "%s%s", getenv("HOME"), HOME_DIR_UNIX_DBG);
    }
    else
    {
        snprintf(GlobalSettings.app_dir, P_MAX_PATH, "%s", PROG_DIR_UNIX);
        snprintf(GlobalSettings.user_dir, P_MAX_PATH, "%s%s", getenv("HOME"), HOME_DIR_UNIX);
    }
#else
    snprintf(GlobalSettings.temp_dir, P_MAX_PATH, "%s", getenv("TEMP"));
    if (GlobalSettings.run_env != reProduction)
    {
        snprintf(GlobalSettings.app_dir, P_MAX_PATH, "%s%s", getenv("USERPROFILE"), PROG_DIR_WIN_DBG);
        snprintf(GlobalSettings.user_dir, P_MAX_PATH, "%s%s", getenv("USERPROFILE"), HOME_DIR_WIN_DBG);
    }
    else
    {
        snprintf(GlobalSettings.app_dir, P_MAX_PATH, "%s%s", getenv("PROGRAMFILES"), PROG_DIR_WIN);
        snprintf(GlobalSettings.user_dir, P_MAX_PATH, "%s%s", getenv("USERPROFILE"), HOME_DIR_WIN);
    }
#endif

    Slashify(GlobalSettings.app_dir, true);
    Slashify(GlobalSettings.user_dir, true);
    Slashify(GlobalSettings.temp_dir, true);

    LOG2(llDebug, "Home dir: ", GlobalSettings.user_dir);
    LOG2(llDebug, "Prog dir: ", GlobalSettings.app_dir);
    LOG2(llDebug, "Temp dir: ", GlobalSettings.temp_dir);
}


void load_scene(const char *fname)
{
    try
    {
        DeallocSimulation();
        DeallocateCellBlocks();
        DeallocateTubeLines();
        DeallocateTubeBundles();
        DeallocateTissueSettings();
        DeallocateBarriers();
        DeallocateDefinitions();

        SimulationSettings.reset();
        TubularSystemSettings.reset();
        VisualSettings.reset();

        char basefile[P_MAX_PATH];
        snprintf(basefile, P_MAX_PATH, "%sinclude/base.ag", GlobalSettings.app_dir);
        ParseFile(basefile, false);
        ParseFile(fname, true);
    }
    catch (Error *err)
    {
        LogError(err);
    }
}


void generate_scene()
{
    try
    {
        if (!GlobalSettings.simulation_allocated)
            AllocSimulation();

        GenerateTubesInAllTubeBundles();
        GenerateTubesInAllTubeLines();
        GenerateCellsInAllBlocks();
    }
    catch (Error *err)
    {
        LogError(err);
    }
}


void display_statistics()
{
    printf("Step: %d; Time: %s", SimulationSettings.step, SecToString(SimulationSettings.time));

    // loop over all tissues...
    anyTissueSettings *ts = FirstTissueSettings;
    while (ts)
    {
        printf("; %s: %d", ts->name, ts->no_cells[0]);
        ts = ts->next;
    }

    printf("; %s: %d (%d)\n", MODEL_TUBE_SHORTNAME_PL_PCHAR, NoTubes, NoTubeChains);
}


void report_timers()
{
    printf("\n");
    printf("%s\n", ReportTimer(TimerSimulationId, false));
    printf("%s\n", ReportTimer(TimerTubeUpdateId, false));
    printf("%s\n", ReportTimer(TimerResetForcesId, false));
    printf("%s\n", ReportTimer(TimerCellCellForcesId, false));
    printf("%s\n", ReportTimer(TimerCellBarrierForcesId, false));
    printf("%s\n", ReportTimer(TimerTubeTubeForcesId, false));
    printf("%s\n", ReportTimer(TimerTubeCellForcesId, false));
    printf("%s\n", ReportTimer(TimerCellGrowId, false));
    printf("%s\n", ReportTimer(TimerTubeGrowId, false));
    printf("%s\n", ReportTimer(TimerRearangeId, false));
    printf("%s\n", ReportTimer(TimerRemoveTubesId, false));
    printf("%s\n", ReportTimer(TimerConnectTubeChainsId, false));
    printf("%s\n", ReportTimer(TimerMergeTubesId, false));
    printf("%s\n", ReportTimer(TimerUpdatePressuresId, false));
    printf("%s\n", ReportTimer(TimerCopyConcentrationsId, false));
    printf("%s\n", ReportTimer(TimerTissuePropertiesId, false));
    printf("%s\n", ReportTimer(TimerBloodFlowId, false));
}


int main(int argc, char **argv)
{
    printf("%s, %d.%d.%d\n", APP_NAME, MOTPUCA_VERSION, MOTPUCA_SUBVERSION, MOTPUCA_RELEASE);

    GlobalSettings.run_env = reDebug;
    GlobalSettings.debug = (GlobalSettings.run_env != reProduction);

    setup_directories();

    if (argc < 2)
    {
        printf("Usage: %s <input-file>\n", argv[0]);
        return 1;
    }

    DefineAllTimers();

    load_scene(argv[1]);
    generate_scene();



    try
    {
        long t = time(0);
        for (int i = 0; SimulationSettings.time < SimulationSettings.stop_time; i++)
        {
            TimeStep();

            if (time(0) - t > 0)
            {
                display_statistics();
                t = time(0);
            }

            if (SimulationSettings.step % SimulationSettings.graph_sampling == 0)
                AddAllStatistics();

            if (SimulationSettings.save_statistics && SimulationSettings.step % SimulationSettings.save_statistics == 0)
            {
                char fname[P_MAX_PATH];
                snprintf(fname, P_MAX_PATH, "%sstatistics_%08d.csv", GlobalSettings.output_dir, SimulationSettings.step);
                SaveStatistics(fname);
            }

            if (SimulationSettings.save_povray && SimulationSettings.step % SimulationSettings.save_povray == 0)
                SavePovRay(0, true);
            if (SimulationSettings.save_ag && SimulationSettings.step % SimulationSettings.save_ag == 0)
            {
                char fname[P_MAX_PATH];
                snprintf(fname, P_MAX_PATH, "%sstep_%08d.ag", GlobalSettings.output_dir, SimulationSettings.step);
                SaveAG(fname, true);
            }

        }

        // save after last step...
        if (!SimulationSettings.save_ag || SimulationSettings.step % SimulationSettings.save_ag != 0)
        {
            char fname[P_MAX_PATH];
            snprintf(fname, P_MAX_PATH, "%sstep_%08d.ag", GlobalSettings.output_dir, SimulationSettings.step);
            SaveAG(fname, true);
        }
    }
    catch (Error *err)
    {
        LogError(err);
    }

    report_timers();

    return 0;
}
