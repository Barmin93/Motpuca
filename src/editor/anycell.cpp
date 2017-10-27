#include "anycell.h"

anyCell::anyCell() : tissue(0), pos(0, 0, 0), pos_h1(-1000000000, 0, 0), pos_h2(-1000000000, 0, 0), r(0), state(sat::csAlive), age(0), state_age(0), no_cells_in_box(0),
    one_by_mass(0), velocity(0, 0, 0), force(0, 0, 0), pressure(0), pressure_prev(0), pressure_avg(0), pressure_sum(0), time_to_necrosis(0), density(0), mark(false)
{
    for (int i =0; i < sat::dsLast; i++)
        concentrations[i][0] = concentrations[i][1] = 0;
    nei_cnt[sat::ttNormal] = nei_cnt[sat::ttTumor] = 0;
}

