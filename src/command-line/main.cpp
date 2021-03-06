#include <unistd.h>
#include <string.h>
#include <time.h>

#include "../editor/version.h"
#include "../editor/scene.h"
#include "../editor/parser.h"
#include "../editor/config.h"
#include "../editor/timers.h"
#include "../editor/simulation.h"
#include "../editor/anysimulationsettings.h"
#include "../editor/anyglobalsdialog.h"
#include "../editor/anyglobalsettings.h"
#include "../editor/anytissuesettings.h"
#include "../editor/anytubularsystemsettings.h"
#include "../editor/statistics.h"
#include "../editor/model.h"
#include "../editor/log.h"

#define PROG_DIR_WIN "/Desktop/Motpuca"
#define PROG_DIR_WIN_DBG "/Desktop/Motpuca"

#define HOME_DIR_WIN "/Desktop/Motpuca"
#define HOME_DIR_WIN_DBG "/Desktop/Motpuca"


void setup_directories(const char *directory)
{

    snprintf(GlobalSettings.temp_dir, P_MAX_PATH, "%s", getenv("TEMP"));
    snprintf(GlobalSettings.app_dir, P_MAX_PATH, "%s%s", getenv("USERPROFILE"), PROG_DIR_WIN);
    snprintf(GlobalSettings.user_dir, P_MAX_PATH, "%s", directory);

    Slashify(GlobalSettings.app_dir, true);
    Slashify(GlobalSettings.user_dir, true);
    Slashify(GlobalSettings.temp_dir, true);

    LOG2(llDebug, "Home dir: ", GlobalSettings.user_dir);
    LOG2(llDebug, "Prog dir: ", GlobalSettings.app_dir);
    LOG2(llDebug, "Temp dir: ", GlobalSettings.temp_dir);
}


bool load_scene(const char *fname, const char *directory)
{
    setup_directories(directory);
    try
    {
        scene::DeallocSimulation();
        scene::DeallocateCellBlocks();
        scene::DeallocateTubeLines();
        scene::DeallocateTubeBundles();
        scene::DeallocateTissueSettings();
        scene::DeallocateBarriers();
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
        return false;
    }
    return true;
}


void generate_scene()
{
    try
    {
        if (!GlobalSettings.simulation_allocated)
            scene::AllocSimulation();

        scene::GenerateTubesInAllTubeBundles();
        scene::GenerateTubesInAllTubeLines();
        scene::GenerateCellsInAllBlocks();
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
    anyTissueSettings *ts = scene::FirstTissueSettings;
    while (ts)
    {
        printf("; %s: %d", ts->name, ts->no_cells[0]);
        ts = ts->next;
    }

    printf("; %s: %d (%d)\n", MODEL_TUBE_SHORTNAME_PL_PCHAR, scene::NoTubes, scene::NoTubeChains);
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

    GlobalSettings.run_env = sat::reDebug;
    GlobalSettings.debug = (GlobalSettings.run_env != sat::reProduction);



    if (argc < 3)
    {
        printf("Usage: %s <input-file> <output folder>\n", argv[0]);
        return 1;
    }

    DefineAllTimers();

    if (!load_scene(argv[1], argv[2])) return 1;
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
                scene::SavePovRay(0, true);
            if (SimulationSettings.save_ag && SimulationSettings.step % SimulationSettings.save_ag == 0)
            {
                char fname[P_MAX_PATH];
                snprintf(fname, P_MAX_PATH, "%sstep_%08d.ag", GlobalSettings.output_dir, SimulationSettings.step);
                scene::SaveAG(fname, true);
            }

        }

        // save after last step...
        if (!SimulationSettings.save_ag || SimulationSettings.step % SimulationSettings.save_ag != 0)
        {
            char fname[P_MAX_PATH];
            snprintf(fname, P_MAX_PATH, "%sstep_%08d.ag", GlobalSettings.output_dir, SimulationSettings.step);
            scene::SaveAG(fname, true);
        }
    }
    catch (Error *err)
    {
        LogError(err);
    }

    report_timers();

    return 0;
}
