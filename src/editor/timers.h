#ifndef TIMERS_H
#define TIMERS_H

#define MAX_TIMERS 30

extern int TimerSimulationId;
extern int TimerTubeUpdateId;
extern int TimerResetForcesId;
extern int TimerCellCellForcesId;
extern int TimerCellBarrierForcesId;
extern int TimerTubeTubeForcesId;
extern int TimerTubeCellForcesId;
extern int TimerCellGrowId;
extern int TimerTubeGrowId;
extern int TimerRearangeId;
extern int TimerRemoveTubesId;
extern int TimerConnectTubeChainsId;
extern int TimerMergeTubesId;
extern int TimerUpdatePressuresId;
extern int TimerCopyConcentrationsId;
extern int TimerTissuePropertiesId;
extern int TimerBloodFlowId;


struct anyTimer
 {
  char *name;      ///< name of timer
  int parent_id;   ///< id (index in Timers[] array) of parent timer (or -1 if no parent)
  int depth;       ///< depth of timer
  long time;       ///< summary time in milliseconds
  long time_start; ///< time in milliseconds when timer was started
  int running;     ///< is timer running?
 };


void DefineAllTimers();
int DefineTimer(char const *name, int parent_id);
long Time();
void StartTimer(int id);
void StopTimer(int id);
long GetTimer(int id);
char *ReportTimer(int id, bool bold);
void ResetTimer(int id);


#endif // TIMERS_H
