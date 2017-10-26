#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/timeb.h>

#include "log.h"
#include "timers.h"
#include <QTextStream>
QTextStream out(stdout);


static struct timeb ProgramStart = {0,0,0,0};  ///< Time of program start in seconds and milliseconds (set by first call to Time())

static anyTimer Timers[MAX_TIMERS];        ///< Array of timers
static int TimerCnt = 0;                   ///< Number of defined timers

int   TimerSimulationId;        ///< id of simulation timer
int   TimerTubeUpdateId;        ///< tube array rearangement
int   TimerResetForcesId;       ///< id of reset forces timer
int   TimerCellCellForcesId;    ///< cell-cell forces calculation timer
int   TimerCellBarrierForcesId; ///< cell-barrier forces calculation timer
int   TimerTubeTubeForcesId;    ///< tube-tube forces
int   TimerTubeCellForcesId;    ///< tube-cell forces
int   TimerCellGrowId;          ///< cell growing timer
int   TimerTubeGrowId;          ///< tube growing timer
int   TimerRearangeId;          ///< cells array rearangement timer
int   TimerRemoveTubesId;       ///< tubes removal timer
int   TimerConnectTubeChainsId; ///< chains connecting timer
int   TimerMergeTubesId;        ///< chains merging timer
int   TimerUpdatePressuresId;   ///< pressure updating timer
int   TimerCopyConcentrationsId; ///< copy concentration timer
int   TimerTissuePropertiesId;  ///< tissue properties calculation timer
int   TimerBloodFlowId;         ///< blood flow timer

void DefineAllTimers()
/**
 Defines all timers.
*/
{
    TimerSimulationId = DefineTimer("Simulation", -1);

    TimerTubeUpdateId = DefineTimer("UpdateTubes", TimerSimulationId);
    TimerResetForcesId = DefineTimer("ResetForces", TimerSimulationId);
    TimerCellCellForcesId = DefineTimer("CellCellForces", TimerSimulationId);
    TimerCellBarrierForcesId = DefineTimer("CellBarrierForces", TimerSimulationId);
    TimerTubeTubeForcesId = DefineTimer("TubeTubeForces", TimerSimulationId);
    TimerTubeCellForcesId = DefineTimer("TubeCellForces", TimerSimulationId);
    TimerCellGrowId = DefineTimer("GrowAllCells", TimerSimulationId);
    TimerTubeGrowId = DefineTimer("GrowAllTubes", TimerSimulationId);
    TimerRearangeId = DefineTimer("RearangeCells", TimerSimulationId);
    TimerRemoveTubesId = DefineTimer("RemoveTubes", TimerSimulationId);
    TimerConnectTubeChainsId = DefineTimer("ConnectTubeChains", TimerSimulationId);
    TimerMergeTubesId = DefineTimer("MergeTubes", TimerSimulationId);
    TimerUpdatePressuresId = DefineTimer("UpdatePressures", TimerSimulationId);
    TimerCopyConcentrationsId = DefineTimer("CopyConcentrations", TimerSimulationId);
    TimerTissuePropertiesId = DefineTimer("TissueProperties", TimerSimulationId);
    TimerBloodFlowId = DefineTimer("BloodFlow", TimerSimulationId);
}


int DefineTimer(char const *name, int parent_id)
/**
 Defines new timer. Timer is added to Timers[] array. Timer is not started.

 \param name -- name of timer
 \param parent_id -- id of parent timer (-1 if no parent)
*/
{
    if (TimerCnt < MAX_TIMERS)
    {
        // Add new timer...
        Timers[TimerCnt].name = new char[strlen(name) + 1];
        strcpy(Timers[TimerCnt].name, name);
        Timers[TimerCnt].parent_id = parent_id;
        Timers[TimerCnt].depth = parent_id == -1 ? 0 : Timers[parent_id].depth + 1;
        Timers[TimerCnt].time = 0;
        Timers[TimerCnt].running = 0;

        return TimerCnt++;
    }
    else
    {
        // Array too short...
        throw new Error(__FILE__, __LINE__, "Too many timers");
    }
}


long Time()
/**
 Returns time since program start in milliseconds.
*/
 {
  struct timeb tb;
  ftime(&tb);

  if (!ProgramStart.time)
    ProgramStart = tb;

  return (tb.time - ProgramStart.time)*1000 + tb.millitm - ProgramStart.millitm;
 }


void  StartTimer(int id)
/**
 Starts timer.

 \param id -- timer id
*/
 {
  Timers[id].time_start = Time();
  Timers[id].running = 1;
 }


long GetTimer(int id)
/**
 Returns current timer time.

 \param id -- timer id
*/
 {
  if (Timers[id].running)
    return Timers[id].time + (Time() - Timers[id].time_start);
  else
    return Timers[id].time;
 }



void  StopTimer(int id)
/**
 Stops timer.

 \param id -- timer id
*/
 {
  Timers[id].time += Time() - Timers[id].time_start;
  Timers[id].running = 0;
 }


static char *timer_to_str(long t, bool bold)
{
    static char s[201];
    long hours, minutes, seconds, miliseconds;

    miliseconds = t%1000;
    t = t/1000;

    seconds = t%60;
    t = t/60;

    minutes = t%60;
    t = t/60;

    hours = t;

    char bold_start[4], bold_end[5];
    strcpy(bold_start, bold ? "<b>" : "");
    strcpy(bold_end, bold ? "</b>" : "");

    if (!hours && !minutes)
        snprintf(s, 200, "%s%ld.%03ld s%s", bold_start, seconds, miliseconds, bold_end);
    else
        snprintf(s, 200, "%s%02ld:%02ld:%02ld%s", bold_start, hours, minutes, seconds, bold_end);
    return s;
}


char *ReportTimer(int id, bool bold)
/**
  Reports timer.

 \param id -- timer id
*/
{
    static char ret[200], spc[MAX_TIMERS + 1];

    for (int i = 0; i < Timers[id].depth; i++)
        spc[i] = '-';
    spc[Timers[id].depth] = ' ';
    spc[Timers[id].depth + 1] = 0;

    long t = GetTimer(id);

    if (Timers[id].parent_id == -1)
        snprintf(ret, 200, "%s%s: %s", spc, Timers[id].name, timer_to_str(t, bold));
    else
    {
        long pt = GetTimer(Timers[id].parent_id);
        if (!pt)
            snprintf(ret, 200, "%s%s: %s", spc, Timers[id].name, timer_to_str(t, bold));
        else
        {
            float prc = 100.0*t/pt;
            if (prc < 10)
                snprintf(ret, 200, "%s%s: %s (%.1f%%)", spc, Timers[id].name, timer_to_str(t, bold), prc);
            else
                snprintf(ret, 200, "%s%s: %s (%.0f%%)", spc, Timers[id].name, timer_to_str(t, bold), prc);
        }
    }

    return ret;
}


void ResetTimer(int id)
/**
  Reset timer and its children.
*/
{
    StopTimer(id);
    Timers[id].time = Timers[id].time_start = 0;

    for (int i = 0; i < TimerCnt; i++)
        if (Timers[i].parent_id == id)
            ResetTimer(i);
}
