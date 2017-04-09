#ifndef STATISTICS_H
#define STATISTICS_H

class anyStatData
{
public:
    int step;

    int *counter; ///< NoTissues + 1
    // for every tissue:
    //  0 - total number of cells
    //  ( 1 - not used )
    //  2 - csAlive
    //  3 - csHypoxia
    //  4 - csApoptosis
    //  5 - csNecrosis

    anyStatData *next;

    anyStatData(): counter(0), next(0) {}
};

extern anyStatData *Statistics;
extern anyStatData *LastStatistics;
extern anyStatData MaxStatistics;

void DeallocStatistics();
void AddStatistics(int step, int tissue_id, int state_id, int count);
void AddAllStatistics();
void SaveStatistics(char const *fname);


#endif // STATISTICS_H
