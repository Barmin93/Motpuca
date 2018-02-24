#include "anycell.h"

anyCell::anyCell() : tissue(0), pos(0.0f, 0.0f, 0.0f), pos_h1(-1000000000, 0.0f, 0.0f), pos_h2(-1000000000, 0.0f, 0.0f), r(0), state(sat::csAlive), age(0.0f), 
					 state_age(0.0f), no_cells_in_box(0), one_by_mass(0.0f), velocity(0.0f, 0.0f, 0.0f), force(0.0f, 0.0f, 0.0f), pressure(0.0f), pressure_prev(0.0f),
					 pressure_avg(0.0f), pressure_sum(0.0f), time_to_necrosis(0.0f), density(200.0f), viscosity(20.0f), mark(false)
{
    for (int i =0; i < sat::dsLast; i++)
        concentrations[i][0] = concentrations[i][1] = 0.0f;
    nei_cnt[sat::ttNormal] = nei_cnt[sat::ttTumor] = 0;
}

