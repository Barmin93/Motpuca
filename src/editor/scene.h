#ifndef SCENE_H
#define SCENE_H

#include "model.h"
#include "types.h"
#include "anyvector.h"
#include "color.h"
#include "transform.h"
#include "func.h"
#include "log.h"

#include "anycell.h"
#include "anyboundingbox.h"
#include "anyeditable.h"
#include "anyeditabledialog.h"

#include "anytissuesettings.h"
#include "anytube.h"
#include "anytubemerge.h"
#include "anytubebox.h"


class anyCellBlock;
class anyBarrier;
class anyTubeBundle;
class anyTubeLine;

namespace scene {
    extern anyBarrier *FirstBarrier;
    extern anyBarrier *LastBarrier;
    extern anyCellBlock *FirstCellBlock;
    extern anyCellBlock *LastCellBlock;
    extern anyTubeLine *FirstTubeLine;
    extern anyTubeLine *LastTubeLine;
    extern anyTubeBundle *FirstTubeBundle;
    extern anyTubeBundle *LastTubeBundle;
    extern anyTissueSettings *FirstTissueSettings;
    extern anyTissueSettings *LastTissueSettings;
    extern int NoTissueSettings;
    extern anyCell *Cells;
    extern anyTube **TubeChains;
    extern real ***Concentrations;
    extern int NoTubeChains;
    extern int NoTubes;
    extern int LastTubeId;
    extern anyTubeBox *BoxedTubes;
    extern anyTissueSettings *FindTissueSettings(char const *name);

    void AddTissueSettings(anyTissueSettings *ts);
    void RemoveTissueSettings(anyTissueSettings *ts);
    void ParseTissueSettings(FILE *f, anyTissueSettings *ts, bool add_to_scene);
    void SaveTissueSettings_ag(FILE *f, anyTissueSettings const *ts, bool save_header);
    void SaveAllTissueSettings_ag(FILE *f);
    void DeallocateTissueSettings();
    anyTissueSettings *FindTissueSettingById(int id);

    void AddBarrier(anyBarrier *b);
    void RemoveBarrier(anyBarrier *b);
    void ParseBarrier(FILE *f, anyBarrier *b, bool add_to_scene);
    void ParseBarrierValue(FILE *f, anyBarrier *b);
    void SaveBarrier_ag(FILE *f, anyBarrier const *b);
    void SaveAllBarriers_ag(FILE *f);
    void DeallocateBarriers();

    void AddCellBlock(anyCellBlock *b);
    void RemoveCellBlock(anyCellBlock *cb);
    void ParseCellBlockValue(FILE *f, anyCellBlock *b);
    void ParseCellBlock(FILE *f, anyCellBlock *b, bool add_to_scene);
    void SaveCellBlock_ag(FILE *f, anyCellBlock const *b);
    void SaveAllCellBlocks_ag(FILE *f);
    void DeallocateCellBlocks();
    void GenerateCellsInBlock(anyCellBlock *b);
    void GenerateCellsInAllBlocks();

    int GetBoxId(anyVector const pos);
    void SetCellMass(anyCell *c);
    void AddCell(anyCell *c);
    void ParseCellValue(FILE *f, anyCell *c);
    void ParseCell(FILE *f);
    void SaveCell_ag(FILE *f, anyCell *c);
    void SaveAllCells_ag(FILE *f);

    void AddTubeLine(anyTubeLine *vl);
    void RemoveTubeLine(anyTubeLine *vl);
    void ParseTubeLineValue(FILE *f, anyTubeLine *vl);
    void ParseTubeLine(FILE *f, anyTubeLine *vl, bool add_to_scene);
    void SaveTubeLine_ag(FILE *f, anyTubeLine const *vl);
    void SaveAllTubeLines_ag(FILE *f);
    void DeallocateTubeLines();
    void GenerateTubesInTubeLine(anyTubeLine *vl);
    void GenerateTubesInAllTubeLines();

    void AddTubeBundle(anyTubeBundle *vb);
    void RemoveTubeBundle(anyTubeBundle *vb);
    void ParseTubeBundle(FILE *f, anyTubeBundle *vb, bool add_to_scene);
    void SaveTubeBundle_ag(FILE *f, anyTubeBundle const *vb);
    void SaveAllTubeBundles_ag(FILE *f);
    void DeallocateTubeBundles();
    void GenerateTubesInAllTubeBundles();
    void GenerateTubesInTubeBundle(anyTubeBundle *vb);

    void AddTube(anyTube *v, bool attach_to_previous, bool start_new_chain);
    void SetTubeMass(anyTube *v);
    void ParseTubeValue(FILE *f, anyTube *v);
    void ParseTube(FILE *f);
    void SaveTube_ag(FILE *f, anyTube *v);
    void SaveAllTubes_ag(FILE *f);
    void SmoothTubeTips(anyTube const *v, anyVector &pos1, anyVector &pos2);
    bool TubesJoined(anyTube *v1, anyTube *v2);
    anyTube *FindTubeById(int id, bool current);
    void RelinkTubes();
    anyTube *FindFirstTube(anyTube *v);
    anyTube *FindLastTube(anyTube *v);

    void AddTubesToMerge(anyTube *v1, anyTube *v2);
    void MergeTubes();
    void SavePovRay(char const *povfname, bool save_ani);
    void SaveVTK();
    void SaveAG(char const *fname, bool save_cells_and_tubes);

    void AllocSimulation();
    void DeallocSimulation();
    void ReallocSimulation();

    void UpdateSimulationBox();

}



#endif // SCENE_H
