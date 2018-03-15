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
    int max_cells_per_box;      ///< maximum number of cells in box *should be calculated autmatically!*
    float force_r_cut;          ///< attraction forces r_cut [um]



    float force_r_peak;         ///< attraction forces peak

    int max_tube_chains;       ///< max number of tube chains
    int max_tube_merge;        ///< max number of tube pairs merged in one simulation step

    unsigned long sim_phases;  ///< enabled simulation phases/processes

    // output...
    int save_statistics;       ///< statistics saving frequency
    int save_povray;           ///< povray saving frequency
    int save_ag;               ///< ag saving frequency

    // Simple description of tumor cells mechanism, when medicine is used:
    //
    // 1. We have state P (proliferative) in which tumor cell can divide
    // 2. We have state Q (quiescent) in which CR (consumption rate) of O2 is decreased and CR of medicine is the same.
    // 3. Transition from P to Q is governed by O2 concentration in cell.
    // 4. If not enough O2 - tumor cell enter necrosis state.
    //
    // After medicine injection:
    //
    // 1. Additional Qp state (mutated Q) introduced.
    // 2. Transition Q -> Qp is governed by medicine concentration in cell.
    // 3. Cell P can either die or become Q.
    // 4. Cell Q can either die or become Qp.
    // 5. If cell is in Qp - it can either turn back to proliferative state or die.
    // 6. Effects of O2 are the same as without medicine (not enought - tumor necrosis).

    // medicine...

    float proliferative_o2;         ///< threshold of O2, when tumor cell becomes Q (from P)
    float quiescent_medicine;   ///< threshold of medicine, when it will affect Q tumor cell (becomes Qp)
    float medicine_threshold;   ///< threshold of medicine, when it will affect tumor cell

    int add_medicine;          ///< step when medicine is added to blood vessels
    int remove_medicine;       ///< step when medicine is remove from blood vessels
    int activation_steps;         ///< number of steps, when medicine will activate

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
