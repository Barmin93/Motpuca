#include "config.h"
#include "scene.h"
#include "statistics.h"
#include "log.h"

anyStatData *Statistics = 0;
anyStatData *LastStatistics = 0;
anyStatData MaxStatistics;

void DeallocStatistics()
{
   anyStatData *sd = Statistics;
   anyStatData *nsd;

   while (sd)
   {
       nsd = sd->next;

       delete [] sd->counter;
       delete sd;

       sd = nsd;
   }

   LastStatistics = Statistics = 0;
}


void AddStatistics(int step, int tissue_id, int state_id, int count)
{
    if (tissue_id > NoTissueSettings || state_id >= sat::csLast)
        return;

    anyStatData *sd;

    if (LastStatistics && LastStatistics->step == step)
        sd = LastStatistics;
    else
    {
        // alloc new...
        sd = new anyStatData;

        sd->step = step;
        sd->counter = new int[(NoTissueSettings + 1)*sat::csLast];
        for (int i = 0; i < (NoTissueSettings + 1)*sat::csLast; i++)
            sd->counter[i] = 0;

        // add to linked list...
        if (!Statistics)
        {
            Statistics = sd;

            // reset max values...
            delete [] MaxStatistics.counter;
            MaxStatistics.counter = new int[(NoTissueSettings + 1)*sat::csLast]; // 0,1,2,3,4    5,6,7,8,9    10,11,12,13,14
            for (int i = 0; i < (NoTissueSettings + 1)*sat::csLast; i++)
                MaxStatistics.counter[i] = 0;
        }
        else
            LastStatistics->next = sd;
        LastStatistics = sd;
    }

    sd->counter[tissue_id*sat::csLast + state_id] = count;
    if (count > MaxStatistics.counter[tissue_id*sat::csLast + state_id])
        MaxStatistics.counter[tissue_id*sat::csLast + state_id] = count;
}


void AddAllStatistics()
{
    anyTissueSettings *ts = FirstTissueSettings;

    // reset counters...
    while (ts)
    {
        for (int i = 1; i < sat::csLast; i++)
            ts->no_cells[i] = 0;
        ts = ts->next;
    }

    // count cells...
    int first_cell = 0;
    for (int box_id = 0; box_id < SimulationSettings.no_boxes; box_id++)
    {
        int no_cells = Cells[first_cell].no_cells_in_box;
        for (int i = 0; i < no_cells; i++)
            if (Cells[first_cell + i].state >= sat::csAlive)
                Cells[first_cell + i].tissue->no_cells[Cells[first_cell + i].state]++;

        first_cell += SimulationSettings.max_cells_per_box;
    }

    ts = FirstTissueSettings;
    while (ts)
    {
        for (int i = 0; i < sat::csLast; i++)
            AddStatistics(SimulationSettings.step, ts->id, i, ts->no_cells[i]);
        ts = ts->next;
    }

    // count tubes...
    AddStatistics(SimulationSettings.step, NoTissueSettings, 0, NoTubes);
    int fl = 0;
    for (int i = 0; i < NoTubeChains; i++)
    {
        anyTube *v = TubeChains[i];
        while (v)
        {
            if (v->blood_flow != 0) fl++;
            v = v->next;
        }
    }
    AddStatistics(SimulationSettings.step, NoTissueSettings, sat::csAlive, fl);
    AddStatistics(SimulationSettings.step, NoTissueSettings, sat::csHypoxia, NoTubeChains);
}


void SaveStatistics(char const *fname)
/**
  Saves statistics file.

  \param fname -- name of output file, if NULL then name is generated.
*/
{
    LOG2(llInfo, "Saving statistics file: ", fname);

    FILE *f = fopen(fname, "w");
    if (f)
    {
        // header...
        fprintf(f, "time; ");
        for (int i = 0; i < NoTissueSettings; i++)
        {
            anyTissueSettings *t = FindTissueSettingById(i);

            fprintf(f, "%s; ", t->name);
            fprintf(f, "%s (%s); ", t->name, "ALIVE");
            fprintf(f, "%s (%s); ", t->name, "HYPOXIA");
            fprintf(f, "%s (%s); ", t->name, "APOPTOSIS");
            fprintf(f, "%s (%s); ", t->name, "NECROSIS");
        }
        fprintf(f, "%s; ", MODEL_TUBE_SHORTNAME_PL_PCHAR);
        fprintf(f, "%s (with flow); ", MODEL_TUBE_SHORTNAME_PL_PCHAR);
        fprintf(f, "%s; ", MODEL_TUBECHAIN_SHORTNAME_PL_PCHAR);
        fprintf(f, "\n");

        // data...
        anyStatData *sd = Statistics;
        while (sd)
        {
            fprintf(f, "%.1f; ", sd->step*SimulationSettings.time_step);

            for (int i = 0; i < NoTissueSettings + 1; i++)
            {
                if (i < NoTissueSettings)
                {
                    fprintf(f, "%d; ", sd->counter[i*sat::csLast]);
                    fprintf(f, "%d; ", sd->counter[i*sat::csLast + sat::csAlive]);
                    fprintf(f, "%d; ", sd->counter[i*sat::csLast + sat::csHypoxia]);
                    fprintf(f, "%d; ", sd->counter[i*sat::csLast + sat::csApoptosis]);
                    fprintf(f, "%d; ", sd->counter[i*sat::csLast + sat::csNecrosis]);
                }
                else
                {
                    fprintf(f, "%d; ", sd->counter[i*sat::csLast]);
                    fprintf(f, "%d; ", sd->counter[i*sat::csLast + sat::csAlive]);
                    fprintf(f, "%d; ", sd->counter[i*sat::csLast + sat::csHypoxia]);
                }
            }

            fprintf(f, "\n");
            sd = sd->next;
        }

        fclose(f);
    }

}
