#ifndef ANYTUBE_H
#define ANYTUBE_H

#include "const.h"
#include "anyvector.h"
#include "anyeditable.h"
#include "log.h"
#include "model.h"
#include "scene.h"
#include "types.h"

class anyTube
/**
  Structure defining tube.
*/
{
public:
    anyVector pos1;    ///< tip #1 of tube
    anyVector pos2;    ///< tip #2 of tube
    float length;       ///< length of tube
    float final_length; ///< final length of tube
    float r;            ///< radius
    float final_r;      ///< final radius
    sat::CellState state;   ///< state of tube
    float age;          ///< age
    float state_age;    ///< age in current state
    float flow_time;    ///< time of last blood flow

    anyVector velocity1; /// velocity in tip #1
    anyVector velocity2; /// velocity in tip #2
    anyVector force1;  ///< force in tip #1
    anyVector force2;  ///< force in tip #2

    anyTube *next;   ///< next tube in chain
    anyTube *prev;   ///< previous tube in chain
    anyTube *fork;   ///< link to first tube in forking chain
    anyTube *base;   ///< link to base tube of first forked tube
    anyTube *top;    ///< link to base tube of last forked tube
    anyTube *jab;    ///< link to attached tube

    float concentrations[sat::dsLast][2];

    bool fixed_blood_pressure; ///< is blood pressure fixed?
    float blood_pressure; ///< blood pressure

    // aux values...
    bool taf_triggered; ///< enough TAF for sprouting?
    float blood_flow;    ///< blood flows

    int id;            ///< tube id
    int parsed_id;     ///< id read from input file
    int base_id;       ///< id of read base tube
    int top_id;        ///< id of read top tube
    int nx, ny, nz;    ///< minimum bounding box
    float one_by_mass;  ///< 1/mass
    float pressure;             ///< pressure - float value of pressure in cell, may not be used in visualization or any calculations inside CellCellForces
    float pressure_prev;        ///< pressure in previous step (for calculating pressure_avg in nei. cells)
    float pressure_avg;         ///< average pressure - may not be used for visualization  or any calculations inside CellCellForces
    float pressure_sum;         ///< pressure sum for averaging
    int  nei_cnt;              ///< number of neighbouring cells

    anyTube();

    anyVector smooth_pos1() const
    {
        anyVector p = pos1;
        if (prev)
            p = (p + prev->pos2)*0.5;
        else if (base)
            p = (p + base->smooth_pos1() + base->smooth_pos2())*0.3333f;

        return p;
    }

    anyVector smooth_pos2() const
    {
        anyVector p = pos2;
        if (next)
            p = (p + next->pos1)*0.5;
        else if (top)
            p = (p + top->smooth_pos1() + top->smooth_pos2())*0.3333f;

        return p;
    }

};

#endif // ANYTUBE_H
