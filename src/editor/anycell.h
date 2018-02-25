#ifndef ANYCELL_H
#define ANYCELL_H

#include "anyvector.h"
#include "const.h"

class anyTissueSettings;

class anyCell
/**
  Structure defining cell properties.
*/
{
public:
    anyTissueSettings *tissue; ///< tissue
    anyVector pos;             ///< position
    anyVector pos_h1, pos_h2;  ///< historical positions (for displacement drawing)
                               // every N steps: pos_h2 := pos_h1; pos_h1 := pos;
    float r;                    ///< current radius
    sat::CellState state;      ///< state
    float age;                  ///< age
    float state_age;            ///< age in current state

    int no_cells_in_box;       ///< number of cells in box (valid only for first cell in every box)

    // aux fields...
    float one_by_mass;          ///< 1/mass
    anyVector velocity;        ///< velocity
    anyVector force;           ///< force
    float pressure;             ///< pressure - float value of pressure in cell, may not be used in visualization or any calculations inside CellCellForces
    float pressure_prev;        ///< pressure in previous step (for calculating pressure_avg in nei. cells)
    float pressure_avg;         ///< average pressure - may not be used for visualization  or any calculations inside CellCellForces
    float pressure_sum;         ///< pressure sum for averaging
    int  nei_cnt[2];           ///< number of neighbouring cells (0 - normal, 1 - tumor)
    float time_to_necrosis;     ///< individual time to necrosis (in hypoxia or apoptosis)
    float concentrations[sat::dsLast][2];
    float density;
    bool mark;                 ///< marker (for debugging)

    anyCell();
};

#endif // ANYCELL_H
