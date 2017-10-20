#include "anytube.h"

#include "const.h"
#include "vector.h"
#include "anyeditable.h"
#include "log.h"
#include "model.h"
#include "scene.h"
#include "types.h"


anyTube::anyTube(): pos1(0, 0, 0), pos2(0, 0, 0), length(0), final_length(0), r(0), final_r(0), state(sat::csAlive), age(0), state_age(0), flow_time(0),
    velocity1(0, 0, 0), velocity2(0, 0, 0), force1(0, 0, 0), force2(0, 0, 0),
    next(0), prev(0), fork(0), base(0), top(0), jab(0),
    fixed_blood_pressure(false), blood_pressure(0),
    taf_triggered(false), blood_flow(0), id(0), parsed_id(0), base_id(0), top_id(0), one_by_mass(0),
    pressure(0), pressure_prev(0), pressure_avg(0), pressure_sum(0), nei_cnt(0)
{
    for (int i = 0; i < sat::dsLast; i++)
    concentrations[i][0] = concentrations[i][1] = 0;
}



