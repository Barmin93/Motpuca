#ifndef ANYSIMULATIONSETTINGS_H
#define ANYSIMULATIONSETTINGS_H

#include "anyglobalsettings.h"
#include "anyvector.h"

class anySimulationSettings
/**
  Global simulation settings.

  Adding new field requires adding of:
    - default value in reset
    - section to SaveSimulationSettings_ag()
    - section to ParseSimulationSettingsValue()
    - section to documentation
*/
{
public:
    int dimensions;            ///< number of dimensions (2 or 3)
    float time_step;            ///< simulation time step [s]
    float time;                 ///< simulation time [s]
    float stop_time;            ///< time to stop simulation [s]
    int step;                  ///< simulation step
    anyVector comp_box_from;   ///< minimal vertex of simulation box
    anyVector comp_box_to;     ///< maximal vertex of simulation box

    float box_size;             ///< box size [um] *should be calculated autmatically!*
    int max_cells_per_box;     ///< maximum number of cells in box *should be calculated autmatically!*
    float force_r_cut;          ///< attraction forces r_cut [um]
    float force_r_peak;         ///< attraction forces peak

    int max_tube_chains;       ///< max number of tube chains
    int max_tube_merge;        ///< max number of tube pairs merged in one simulation step

    unsigned long sim_phases;  ///< enabled simulation phases/processes

    // output...
    int save_statistics;       ///< statistics saving frequency
    int save_povray;           ///< povray saving frequency
    int save_ag;               ///< ag saving frequency

    // graph...
    int graph_sampling;        ///< graph samplimg rate

    // derived values...
    int no_boxes_x; ///< number of boxes in x direction
    int no_boxes_y; ///< number of boxes in y direction
    int no_boxes_z; ///< number of boxes in z direction
    int no_boxes_xy; /// no_boxes_x*no_boxes_y
    int no_boxes;   ///< overall number of boxes
    int max_max_cells_per_box; ///< actual maximum number of cells in box
    int max_max_max_cells_per_box; ///< all-time maximum number of cells in box
    float farest_point;   ///< farest distance from (0, 0, 0)
    float force_r_cut2;   ///< force_r_cut*force_r_cut
    float diffusion_coeff[sat::dsLast];   ///< oxygen, TAF etc diffusion coefficient ( TODO: stability condition)
    float max_o2_concentration;   ///< maximum oxygen in kg/um^3 in cell represented by concentration == 1
    anySimulationSettings();

    void reset();
    void calculate_derived_values();
};

extern anySimulationSettings SimulationSettings;

#endif // ANYSIMULATIONSETTINGS_H
