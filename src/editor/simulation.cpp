#ifdef QT_CORE_LIB
#include <QtDebug>
#endif

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <algorithm>
#include <iostream>
#include <cstring>

#include "types.h"
#include "parser.h"
#include "config.h"
#include "log.h"
#include "timers.h"
#include "scene.h"

#include "anytube.h"
#include "anybarrier.h"



/*

The human body is made up of approximately 100 trillion (100, 000, 000, 000, 000) cells

  from http://en.wikipedia.org/wiki/Cell_cycle:
A disregulation of the cell cycle components may lead to tumor formation.
As mentioned above, some genes like the cell cycle inhibitors, RB, p53 etc.,
when they mutate, may cause the cell to multiply uncontrollably, forming a tumor.
Although the duration of cell cycle in tumor cells is equal to or longer than
that of normal cell cycle, the proportion of cells that are in active cell
division (versus quiescent cells in G0 phase) in tumors is much higher than
that in normal tissue. Thus there is a net increase in cell number as the
number of cells that die by apoptosis or senescence remains the same.

The cells which are actively undergoing cell cycle are targeted in cancer
therapy as the DNA is relatively exposed during cell division and hence
susceptible to damage by drugs or radiation. This fact is made use of
in cancer treatment; by a process known as debulking, a significant mass
of the tumor is removed which pushes a significant number of the remaining
tumor cells from G0 to G1 phase (due to increased availability of nutrients,
oxygen, growth factors etc.). Radiation or chemotherapy following the debulking
procedure kills these cells which have newly entered the cell cycle. [12]

The fastest cycling mammalian cells in culture, and crypt cells in the
intestinal epithelium {nablonek}, have a cycle time as short as 9 to 10 hours.
Stem cells in resting mouse skin may have a cycle time of more than
200 hours. Most of this difference is due to the varying length of G1,
the most variable phase of the cycle. M and S do not vary much.

In general, cells are most radiosensitive in late M and G2 phases and
most resistant in late S.

For cells with a longer cell cycle time and a significantly long
G1 phase, there is a second peak of resistance late in G1

The pattern of resistance and sensitivity correlates with the level of
sulfhydryl compounds in the cell. Sulfhydryls are natural radioprotectors
and tend to be at their highest levels in S and at their lowest near mitosis.
*/


// SPH kernel functions
real W_poly6(real r_sq, real h_sq, real h)
{
    static real coefficient = 315.0f / (64.0f*c::PIf*pow(h, 9));

    return coefficient * pow(h_sq - r_sq, 3);
}

anyVector GradW_poly6(real r, real h)
{
    static real coefficient = -945.0f / (32.0f*c::PIf*pow(h, 9));

    return anyVector(coefficient * pow(pow(h, 2) - pow(r, 2), 2));// * r
}

// do surface tension color field
real LapW_poly6(real r, real h)
{
    static real coefficient = -945.0f / (32.0f*c::PIf*pow(h, 9));

    return coefficient * (pow(h, 2) - pow(r, 2)) * (3.0f*pow(h, 2) - 7.0f*pow(r, 2));
}

anyVector GradW_spiky(real r, real h)
{
    static real coefficient = -45.0f / (c::PIf*pow(h, 6));

    return anyVector(coefficient * pow((h - r), 2) / r);
}

real LapW_viscosity(real r, real h)
{
    static real coefficient = 45.0f / (c::PIf*pow(h, 6));

    return coefficient * (h - r);
}

anyVector Grad_BicubicSpline(anyVector x, real h)
{
    auto const r = x.length();
    auto const q = r / h;
    auto coefficient = 6.0f * (8.0f/c::PIf) / pow(h, 3);

    if(0.0f <= q && q <= 0.5f)
        coefficient *= 3.0f * pow(q, 2) - 2.0f * q;
    else if(0.5f < q && q <= 1.0f)
        coefficient *= -1.0f * pow(1.0f - q, 2);
    else
        coefficient *= 0.0f;
    x.normalize();
    return anyVector(coefficient) * x / h;
}


inline
void change_cell_state(anyCell *c, sat::CellState new_state)
/**
  Changes state of cell and resets its timer.

  \param c -- pointer to cell
  \param new_state -- new state of cell
*/
{
    c->state = new_state;
    c->state_age = 0;
}


inline
void change_tube_state(anyTube *v, sat::CellState new_state)
/**
  Changes state of tube and resets its timer.

  \param v -- pointer to tube
  \param new_state -- new state of tube
*/
{
    v->state = new_state;
    v->state_age = 0;
}


inline
int conc_step_current()
/**
  Returns index in concentations[] array associated with current simulation step.

  \returns index in concentations[] array associated with current simulation step.
*/
{
    return SimulationSettings.step % 2;
}


inline
int conc_step_prev()
/**
  Returns index in concentations[] array associated with previous simulation step.

  \returns index in concentations[] array associated with previous simulation step.
*/
{
    return !(SimulationSettings.step % 2);
}


static
void normalize_conc(real &conc)
/**
  Normalizes concentration value to range [0, 1].

  \param conc -- concentration to normalize
*/
{
    if (conc < 0) conc = 0;
    else if (conc > 1) conc = 1;
}


static
void limit_velocity(anyVector &v, real limit)
/**
  Limits velocity (or any other) vector.

  \param v -- vector to normalize
  \param limit -- maximum vector length
*/
{
    if (v.length2() > limit*limit)
    {
        v.normalize();
        v = v*limit;
    }
}


void GrowCell(anyCell *c)
/**
  Growth of cell.

  State changes:
   - csAlive ---(time_to_apoptosis)---> csApoptosis
   - csAlive ---(O2 shortage)---> csHypoxia
   - csApoptisis | csHypoxia ---(time_to_necrosis)---> csNecrosis
   - csNecrosis ---(time_in_necrosis)---> csRemoved

  Cell grows in csAlive until it reaches cell_r radius.
  Cell shrinks in csNecrosis until it reaches dead_r radius.

  \param c -- pointer to cell
*/
{
    static bool mutation = true;
    anyTissueSettings *tissue = c->tissue;

    if (SimulationSettings.dimensions == 2)
        c->force.z = 0;

    // move...

    // dv = F/m*dt...
    anyVector dv(c->force.x*SimulationSettings.time_step*c->one_by_mass,
                 c->force.y*SimulationSettings.time_step*c->one_by_mass,
                 c->force.z*SimulationSettings.time_step*c->one_by_mass);

    // v += dv...
    c->velocity += dv;

    //limit_velocity(c->velocity, SimulationSettings.time_step);

    // dr = v*dt
    anyVector dr(c->velocity.x*SimulationSettings.time_step,
                 c->velocity.y*SimulationSettings.time_step,
                 c->velocity.z*SimulationSettings.time_step);

    if (dr.length2() > 1) dr.normalize();

    //r += dr...
    c->pos += dr;

    // velocity dumping...
    c->velocity *= 0.5; //@@@


    if (SimulationSettings.step % 10 == 0)
    {
        c->pos_h2 = c->pos_h1;
        c->pos_h1 = c->pos;
    }


    // concentration change...

    // O2 consumption/production...
//    if (tissue->type == ttNormal && 2*c->nei_cnt[ttTumor] > c->nei_cnt[ttNormal])
//        ;
//    else
        c->concentrations[sat::dsO2][conc_step_current()] -= c->tissue->o2_consumption * c->tissue->density / 10e18 * SimulationSettings.time_step / SimulationSettings.max_o2_concentration / 10;
    normalize_conc(c->concentrations[sat::dsO2][conc_step_current()]);

    // TAF production...
    if (c->state == sat::csHypoxia)
    {
        // TAF production...
        c->concentrations[sat::dsTAF][conc_step_current()] = 1;
    }

    // Pericytes production....
    if (c->state == sat::csAlive)
    {
        c->concentrations[sat::dsPericytes][conc_step_current()] += c->tissue->pericyte_production * SimulationSettings.time_step;
        normalize_conc(c->concentrations[sat::dsPericytes][conc_step_current()]);
    }

    //mitosis

    if(strcmp(c->tissue->name, "membrane") == 0){
        if (SimulationSettings.sim_phases & sat::spMitosis
            && SimulationSettings.step > 1  //< pressures are calculated in steps 0 & 1
            && c->state == sat::csAlive
            && c->age > tissue->minimum_interphase_time
            && c->r >= tissue->minimum_mitosis_r
            && c->pressure_prev < tissue->max_pressure
            && rand() % 100==23
            )
        {
            // change age...
            c->age = 0;
            change_cell_state(c, sat::csAdded);

            // clone cell...
            anyCell *nc = new anyCell;
            *nc = *c;

            //double time_step = (double)SimulationSettings.step;
            //auto cos = (time_step*time_step*time_step* 100000) /216000000000 ;
            anyTissueSettings *ts = scene::FindTissueSettings("epidermis");
            nc->tissue = ts;
            nc->pos += anyVector(0,c->r,0);
            c->pos_h1.x = c->pos_h2.x = nc->pos_h1.x = nc->pos_h2.x = -1000000000;
            scene::AddCell(nc);
        }
    }else if (SimulationSettings.sim_phases & sat::spMitosis
        && SimulationSettings.step > 1  //< pressures are calculated in steps 0 & 1
        && c->state == sat::csAlive
        && c->age > tissue->minimum_interphase_time
        && c->r >= tissue->minimum_mitosis_r
        && c->pressure_prev < tissue->max_pressure
        && (strcmp(c->tissue->name, "epidermis") != 0)
        && rand() % 1000==23
        )
    {
        // displacement...
        anyVector d;
        d.set_random(SimulationSettings.dimensions, 0.5*c->r);

        // shrink cell...
        c->r *= 0.79;
        scene::SetCellMass(c);

        // change age...
        c->age = 0;
        change_cell_state(c, sat::csAdded);

        // clone cell...
        anyCell *nc = new anyCell;
        *nc = *c;

        double time_step = (double)SimulationSettings.step;
        auto cos = (time_step*time_step*time_step* 100000) /216000000000 ;
//        std::cout << c->tissue->name << std::endl;
//        std::cout << (strcmp(c->tissue->name, "epidermis") == 0) << std::endl;

        if(c->tissue->type == sat::ttTumor
           && mutation
           && (rand() % 100000 < cos)
        ){

            anyTissueSettings *ts = scene::FindTissueSettings("melanoma1");
            nc->tissue = ts;
            mutation = false;
        }

        // move cells...
        c->pos  += d;
        nc->pos -= d;
        c->pos_h1.x = c->pos_h2.x = nc->pos_h1.x = nc->pos_h2.x = -1000000000;

        scene::AddCell(nc);
    }

    // tissue change...


    // state and radius change...
    switch (c->state)
    {
    case sat::csAdded:
        break;
    case sat::csAlive:
/*        if (tissue->type == ttNormal && c->nei_cnt[ttTumor] > c->nei_cnt[ttNormal])
            change_cell_state(c, csApoptosis);
        else*/ if (c->state_age > tissue->time_to_apoptosis)
            change_cell_state(c, sat::csApoptosis);
        else if (c->tissue->o2_hypoxia > 0 && c->concentrations[sat::dsO2][conc_step_current()] < c->tissue->o2_hypoxia)
        {
            change_cell_state(c, sat::csHypoxia);
        }
        else if(c->r < tissue->cell_r
                && c->pressure_prev < tissue->max_pressure
                )
        {
            // growing...
            c->r += c->tissue->cell_grow_speed*SimulationSettings.time_step;
            if (c->r > tissue->cell_r)
                c->r = tissue->cell_r;
            scene::SetCellMass(c);
        }
        break;
    case sat::csApoptosis:
    case sat::csHypoxia:
        if (c->state_age > c->time_to_necrosis)
            change_cell_state(c, sat::csNecrosis);
        break;
    case sat::csNecrosis:
        if (c->state_age > tissue->time_in_necrosis)
        {
            change_cell_state(c, sat::csRemoved);
        }
        else if(c->r > tissue->dead_r)
        {
            // shrinking...
            c->r -= tissue->cell_shrink_speed*SimulationSettings.time_step;
            if (c->r < tissue->dead_r)
                c->r = tissue->dead_r;
            scene::SetCellMass(c);
        }
        break;
    default:
        throw new Error(__FILE__, __LINE__, "Unexpected state of cell");
    }


    // update timers...
    c->age += SimulationSettings.time_step;
    c->state_age += SimulationSettings.time_step;
}


void GrowAllCells()
/**
  Growth of all cells.
*/
{
    if (SimulationSettings.sim_phases & sat::spGrow)
    {
        StartTimer(TimerCellGrowId);

        // loop over all cells...
        int first_cell = 0;
        for (int box_id = 0; box_id < SimulationSettings.no_boxes; box_id++)
        {
            int no_cells = scene::Cells[first_cell].no_cells_in_box;
            for (int i = 0; i < no_cells; i++)
                // grow only active cells...
                if (scene::Cells[first_cell + i].state != sat::csRemoved)
                    GrowCell(scene::Cells + first_cell + i);

            first_cell += SimulationSettings.max_cells_per_box;
        }

        StopTimer(TimerCellGrowId);
    }
}


void UpdateTubes()
/**
  Promotes tubes from csAdded to csAlive, assigns tubes to boxes.
*/
{
    StartTimer(TimerTubeUpdateId);

    // reset boxes...
    for (int i = 0; i < SimulationSettings.no_boxes; i++)
        scene::BoxedTubes[i].no_tubes = 0;

    // loop over all tubes...
    for (int i = 0; i < scene::NoTubeChains; i++)
    {
        anyTube *v = scene::TubeChains[i];
        while (v)
        {
            // assign to box...
            int box_x_1 = floor((v->pos1.x - SimulationSettings.comp_box_from.x)/SimulationSettings.box_size);
            int box_y_1 = floor((v->pos1.y - SimulationSettings.comp_box_from.y)/SimulationSettings.box_size);
            int box_z_1 = floor((v->pos1.z - SimulationSettings.comp_box_from.z)/SimulationSettings.box_size);
            int box_x_2 = floor((v->pos2.x - SimulationSettings.comp_box_from.x)/SimulationSettings.box_size);
            int box_y_2 = floor((v->pos2.y - SimulationSettings.comp_box_from.y)/SimulationSettings.box_size);
            int box_z_2 = floor((v->pos2.z - SimulationSettings.comp_box_from.z)/SimulationSettings.box_size);

            if (box_x_1 > box_x_2) SWAP(int, box_x_1, box_x_2);
            if (box_y_1 > box_y_2) SWAP(int, box_y_1, box_y_2);
            if (box_z_1 > box_z_2) SWAP(int, box_z_1, box_z_2);

            v->nx = box_x_1;
            v->ny = box_y_1;
            v->nz = box_z_1;

            box_x_1--;
            box_y_1--;
            box_z_1--;
            box_x_2++;
            box_y_2++;
            box_z_2++;

            if (box_x_2 >= 0 && box_x_1 < SimulationSettings.no_boxes_x
                && box_y_2 >= 0 && box_y_1 < SimulationSettings.no_boxes_y
                && box_z_2 >= 0 && box_z_1 < SimulationSettings.no_boxes_z)
            {

                box_x_1 = MIN(MAX(0, box_x_1), SimulationSettings.no_boxes_x - 1);
                box_y_1 = MIN(MAX(0, box_y_1), SimulationSettings.no_boxes_y - 1);
                box_z_1 = MIN(MAX(0, box_z_1), SimulationSettings.no_boxes_z - 1);
                box_x_2 = MIN(MAX(0, box_x_2), SimulationSettings.no_boxes_x - 1);
                box_y_2 = MIN(MAX(0, box_y_2), SimulationSettings.no_boxes_y - 1);
                box_z_2 = MIN(MAX(0, box_z_2), SimulationSettings.no_boxes_z - 1);

                int cnt = 0;
                for (int box_x = box_x_1; box_x <= box_x_2; box_x++)
                    for (int box_y = box_y_1; box_y <= box_y_2; box_y++)
                        for (int box_z = box_z_1; box_z <= box_z_2; box_z++)
                        {
                    int box_id = BOX_ID(box_x, box_y, box_z);
                    if (scene::BoxedTubes[box_id].no_tubes < SimulationSettings.max_cells_per_box)
                        scene::BoxedTubes[box_id].tubes[scene::BoxedTubes[box_id].no_tubes++] = v;
                    cnt++;
                }
            }

            // change state...
            if (v->state == sat::csAdded)
                v->state = sat::csAlive;

            v = v->next;
        }
    }
    StopTimer(TimerTubeUpdateId);
}


void RearrangeCells()
/**
  Removes cells in csRemove state, promotes cells from csAdded to csAlive,
  moves cells to correct boxes.
*/
{
    StartTimer(TimerRearangeId);

    // loop over all cells...
    SimulationSettings.max_max_cells_per_box = 0;
    int first_cell = 0;
    int box_id = 0;
    for (int box_z = 0; box_z < SimulationSettings.no_boxes_z; box_z++)
        for (int box_y = 0; box_y < SimulationSettings.no_boxes_y; box_y++)
            for (int box_x = 0; box_x < SimulationSettings.no_boxes_x; box_x++, box_id++)
            {
                int no_cells = scene::Cells[first_cell].no_cells_in_box;
                if (no_cells > SimulationSettings.max_max_cells_per_box)
                    SimulationSettings.max_max_cells_per_box = no_cells;
                for (int i = 0; i < no_cells; i++)
                {
                    // promote cell?...
                    if (scene::Cells[first_cell + i].state == sat::csAdded)
                        scene::Cells[first_cell + i].state = sat::csAlive;
                    // remove cell?
                    else if (scene::Cells[first_cell + i].state == sat::csRemoved)
                    {
                        // remove cell...
                        scene::Cells[first_cell + i].tissue->no_cells[0]--;
                        if (i != no_cells - 1)
                            scene::Cells[first_cell + i] = scene::Cells[first_cell + no_cells - 1];
                        scene::Cells[first_cell].no_cells_in_box = no_cells - 1;
                        i--;
                        no_cells--;
                        continue;
                    }

                    // move to other box?...
                    if (floor((scene::Cells[first_cell + i].pos.x - SimulationSettings.comp_box_from.x)/SimulationSettings.box_size) != box_x
                        || floor((scene::Cells[first_cell + i].pos.y - SimulationSettings.comp_box_from.y)/SimulationSettings.box_size) != box_y
                        || floor((scene::Cells[first_cell + i].pos.z - SimulationSettings.comp_box_from.z)/SimulationSettings.box_size) != box_z)
                    {
                        // add cell to proper box...
                        scene::AddCell(scene::Cells + first_cell + i);

                        scene::Cells[first_cell + i].tissue->no_cells[0]--;
                        if (i != no_cells - 1)
                            scene::Cells[first_cell + i] = scene::Cells[first_cell + no_cells - 1];
                        scene::Cells[first_cell].no_cells_in_box = no_cells - 1;
                        i--;
                        no_cells--;
                        continue;
                    }
                }

                first_cell += SimulationSettings.max_cells_per_box;
            }

    if (SimulationSettings.max_max_max_cells_per_box < SimulationSettings.max_max_cells_per_box)
        SimulationSettings.max_max_max_cells_per_box = SimulationSettings.max_max_cells_per_box;
    StopTimer(TimerRearangeId);
}


void ConnectTubeChains()
{
    StartTimer(TimerConnectTubeChainsId);

    // find all tubes which start in last tube in the other chain...
    for (int i = 0; i < scene::NoTubeChains; i++)
    {
        anyTube *vl = scene::FindLastTube(scene::TubeChains[i]);

        // case #1...
        if (vl->top && !vl->top->next && !vl->top->top)
        {
            // connect at tip...
            scene::AddTubesToMerge(vl, vl->top);

            // disconnect at current position...
            vl->top->jab = 0;
            vl->top = 0;
        }

        // case #2...
        if (scene::TubeChains[i]->base && !scene::TubeChains[i]->base->next && !scene::TubeChains[i]->base->top)
        {
            scene::TubeChains[i]->base->next = scene::TubeChains[i];
            scene::TubeChains[i]->base->fork = 0;
            scene::TubeChains[i]->prev = scene::TubeChains[i]->base;
            scene::TubeChains[i]->base = 0;

            scene::TubeChains[i] = scene::TubeChains[--scene::NoTubeChains];
            i--;
        }
    }

    StopTimer(TimerConnectTubeChainsId);
}


double normal()
{
    double v1, v2, s;
//    const double x = 67108864.0;

    do
    {
        v1 = 2*double(rand())/RAND_MAX - 1;
        v2 = 2*double(rand())/RAND_MAX - 1;
        s = v1*v1 + v2*v2;
    }
    while (s >= 1);
    return(v1*sqrt((-2.0*log(s))/s));
}


static
void calc_force_dissipative_and_random(anyVector const &p1, anyVector const &p2, real radius, anyVector const &v1, anyVector const &v2, real force_dpd_factor, real dpd_temperature, anyVector &force)
{
    const real Boltzmann = 1.380648813131313e-23 * 1e2;

    if (force_dpd_factor == 0 && dpd_temperature == 0)
        return;

    anyVector r = p2 - p1;
    anyVector v = v2 - v1;
    real r_len2 = r.length2();

    if (r_len2 == 0) return;

    real r_len = sqrt(r_len2);
    real omega = 1 - r_len/(SimulationSettings.force_r_cut + radius);
    real w = omega/r_len2;

    // dissipative force...
    if (force_dpd_factor > 0)
        force -= r*force_dpd_factor*w*(r|v);

    // random force...
    if (dpd_temperature > 0)
        force += r*(sqrt(2*force_dpd_factor*Boltzmann*dpd_temperature*omega)*normal()/r_len);
}


static
bool calc_force(anyVector const &p1, anyVector const &p2, anyVector &force, real &dp, real r, real force_rep_factor, real force_attr1_factor, real force_attr2_factor, bool do_r_cut)
/**
  Calculates forces between two spheres.

  \param p1 -- center of sphere #1
  \param p2 -- center of sphere #2
  \param r  -- sum of radiuses
  \param force_rep_factor -- repulsion factor
  \param force_atr1_factor -- near attraction factor
  \param force_atr2_factor -- far attraction factor
  \param dp -- pressure (output parameter)

  \returns false if spheres are too far to interact.
*/
{
    anyVector d_c1c2 = p2 - p1;
    real d_c1c2_len2 = d_c1c2.length2();

    force.set(0, 0, 0);
    dp = 0;

    if (d_c1c2_len2 == r)
        return true;

    // points in exactly same location...
    if (d_c1c2_len2 == 0)
    {
        d_c1c2.set_random(SimulationSettings.dimensions, r*0.05);
        d_c1c2_len2 = d_c1c2.length2();
    }

    real d_c1c2_len = sqrt(d_c1c2_len2);
    real dr_len = d_c1c2_len - r;

    if (do_r_cut && dr_len > SimulationSettings.force_r_cut)
        return false;

    if (SimulationSettings.sim_phases & sat::spForces)
    {

        anyVector dr = d_c1c2*(dr_len/d_c1c2_len);

        if (dr_len < 0)
        {
            // repulsion...
            force = dr * force_rep_factor;

            dp = force.length();
        }
        else if (dr_len < SimulationSettings.force_r_peak || !do_r_cut)
        {
            // attraction (near)...
            force = dr * force_attr1_factor;

            dp = -force.length();
        }
        else
        {
            // attraction (far)...
            force = dr * (-force_attr2_factor * (dr_len - SimulationSettings.force_r_cut)/dr_len);

            dp = -force.length();
        }
    }

    return true;
}

real vol(real r) {
    return 4.19 * r * r * r;
}


static
void concentration_exchange(real conc1[sat::dsLast][2], real conc2[sat::dsLast][2], real r1, real r2, real dist2, bool bidirectional = true)
{
    int current_frame = conc_step_current();
    int prev_frame = conc_step_prev();
    const real max_exchange_ratio = 1/14.0 / 200; // okolo 14 kul tej samej wielkosci zmiesci sie obok danej kuli;
                                            //przez 2 zeby wartosci sie nie zamienily, zamienione na 50 zeby bylo stabilne, uzasadnic

    //exchange oxygen and TAF concentrations
    //zuzycie jak w modelu z siecia
    //produkcja powinna byc rowna - jak ja zdefiniowac - musi tu zalezec od boxu, zeby sie dalo porownac
    //odleglosc przeplywu - wlasciwie powinna zalezec od zuzycia w poszczegolnych komorkach
    //szybkosc wymiany miedzy komorkami powinna zalezec od: ich wzajemnej odleglosci, ich mas,
    real min_r = MIN(r1, r2);
    real min_vol = vol(min_r);

    for (int sub = 0; sub < sat::dsLast; sub++)
    {
        real diffLevel = conc1[sub][prev_frame] - conc2[sub][prev_frame];
        if (diffLevel != 0 && dist2 < (r1+r2)*(r1+r2)*1.2)
        {
            if (dist2 < (r1+r2)*(r1+r2)) dist2 = (r1+r2)*(r1+r2);
//            if (dist2 < (r1+r2)*(r1+r2)) qDebug("%.2f %.2f", sqrt(dist2), (r1+r2));
            real movingMass = diffLevel * min_vol * max_exchange_ratio;
            //@@@
            real diffSpeed = SimulationSettings.diffusion_coeff[sub] * SimulationSettings.time_step / dist2;
//            qDebug("%f", dist2);

            if (bidirectional)
            {
                conc1[sub][current_frame] -= movingMass / vol(r1) * diffSpeed;
                normalize_conc(conc1[sub][current_frame]);
            }

            conc2[sub][current_frame] += movingMass / vol(r2) * diffSpeed;
            normalize_conc(conc2[sub][current_frame]);
        }
    }
}


static
void cell_cell_force(anyCell *c1, anyCell *c2)
/**
  Calculates forces between two cells.

  \param c1 -- pointer to first cell
  \param c2 -- pointer to second cell
*/
{
    anyVector force;
    real dp;


    if (!calc_force(c1->pos, c2->pos,
                   force, dp,
                   c1->r + c2->r,
                   (c1->tissue->force_rep_factor + c2->tissue->force_rep_factor)*0.5,
                   (c1->tissue->force_atr1_factor + c2->tissue->force_atr1_factor)*0.5,
                   (c1->tissue->force_atr2_factor + c2->tissue->force_atr2_factor)*0.5,
                   true))
        return;

    c1->nei_cnt[c2->tissue->type]++;
    c2->nei_cnt[c1->tissue->type]++;

    if (SimulationSettings.sim_phases & sat::spForces)
    {
        calc_force_dissipative_and_random(c1->pos, c2->pos,
                 c1->r + c2->r,
                 c1->velocity, c2->velocity,
                 (c1->tissue->force_dpd_factor + c2->tissue->force_dpd_factor)*0.5,
                 (c1->tissue->dpd_temperature + c2->tissue->dpd_temperature)*0.5,
                 force);

        c1->force += force;
        c2->force -= force;

        c1->pressure += dp;
        c2->pressure += dp;
        c1->pressure_sum += c2->pressure_prev;
        c2->pressure_sum += c1->pressure_prev;
    }

    if (SimulationSettings.sim_phases & sat::spDiffusion)
    {
        concentration_exchange(c1->concentrations, c2->concentrations,
                               c1->r, c2->r,
                               (c2->pos - c1->pos).length2());
    }
}


static
void cell_cell_forces_box2(int box1_first_cell, int box1_no_cells, int box2_x, int box2_y, int box2_z)
/**
  Calculates forces between cells from two different boxes.

  \param box1_first_cell -- index of first cell of first box
  \param box1_no_cells -- number of cells in first box
  \param box2_x -- x of second box
  \param box2_y -- y of second box
  \param box2_z -- z of second box
*/
{
    if (!VALID_BOX(box2_x, box2_y, box2_z))
        return;

    int box2_box_id = BOX_ID(box2_x, box2_y, box2_z);
    int box2_first_cell = box2_box_id*SimulationSettings.max_cells_per_box;
    int box2_no_cells = scene::Cells[box2_first_cell].no_cells_in_box;

    if (!box2_no_cells) return;

    for (int i = 0; i < box1_no_cells; i++)
        for (int j = 0; j < box2_no_cells; j++)
            cell_cell_force(scene::Cells + box1_first_cell + i, scene::Cells + box2_first_cell + j);
}


void CellCellForces()
/**
  Calculates forces between cells.
*/
{
    StartTimer(TimerCellCellForcesId);


    // calculate forces...
    int box_id = 0;
    int first_cell = 0;
    int no_cells;
    for (int box_z = 0; box_z < SimulationSettings.no_boxes_z; box_z++)
        for (int box_y = 0; box_y < SimulationSettings.no_boxes_y; box_y++)
            for (int box_x = 0; box_x < SimulationSettings.no_boxes_x; box_x++, box_id++)
            {
                no_cells = scene::Cells[first_cell].no_cells_in_box;

                if (no_cells)
                {
                    // inner-box forces...
                    for (int i = 0; i < no_cells - 1; i++)
                        for (int j = i + 1; j < no_cells; j++)
                            cell_cell_force(scene::Cells + first_cell + i, scene::Cells + first_cell + j);

                    // inter-box forces...
                    // (+1, 0, 0)...
                    cell_cell_forces_box2(first_cell, no_cells, box_x + 1, box_y, box_z);

                    // (+1, +1, 0)...
                    cell_cell_forces_box2(first_cell, no_cells, box_x + 1, box_y + 1, box_z);

                    // (0, +1, 0)...
                    cell_cell_forces_box2(first_cell, no_cells, box_x, box_y + 1, box_z);

                    // (-1, +1, 0)...
                    cell_cell_forces_box2(first_cell, no_cells, box_x - 1, box_y + 1, box_z);

                    if (box_z < SimulationSettings.no_boxes_z - 1)
                        for (int dx = -1; dx <= 1; dx++)
                            for (int dy = -1; dy <= 1; dy++)
                                // (dx, dy, +1)...
                                cell_cell_forces_box2(first_cell, no_cells, box_x + dx, box_y + dy, box_z + 1);
                }
                first_cell += SimulationSettings.max_cells_per_box;
            }

    StopTimer(TimerCellCellForcesId);
}


static
void cell_barrier_out_force(anyBarrier *b, anyCell *c)
{
    real dr_len; ///< distance from cell to wall
    real f;

    // left...
    dr_len = (c->pos.x - c->r) - b->to.x;
    if (dr_len < 0
        && -dr_len < (b->to.x - b->from.x)*0.5
        && c->pos.y > b->from.y
        && c->pos.y < b->to.y
        && c->pos.z > b->from.z
        && c->pos.z < b->to.z)
    {
        f = -c->tissue->force_rep_factor * dr_len;
        c->force.x += f;
        c->pressure += fabs(f);
    }

    // right...
    dr_len = b->from.x - (c->pos.x + c->r);
    if (dr_len < 0
        && -dr_len < (b->to.x - b->from.x)*0.5
        && c->pos.y > b->from.y
        && c->pos.y < b->to.y
        && c->pos.z > b->from.z
        && c->pos.z < b->to.z)
    {
        f = c->tissue->force_rep_factor * dr_len;
        c->force.x += f;
        c->pressure += fabs(f);
    }

    // top...
    dr_len = (c->pos.y - c->r) - b->to.y;
    if (dr_len < 0
        && -dr_len < (b->to.y - b->from.y)*0.5
        && c->pos.x > b->from.x
        && c->pos.x < b->to.x
        && c->pos.z > b->from.z
        && c->pos.z < b->to.z)
    {
        f = -c->tissue->force_rep_factor * dr_len;
        c->force.y += f;
        c->pressure += fabs(f);
    }

    // bottom...
    dr_len = b->from.y - (c->pos.y + c->r);
    if (dr_len < 0
        && -dr_len < (b->to.y - b->from.y)*0.5
        && c->pos.x > b->from.x
        && c->pos.x < b->to.x
        && c->pos.z > b->from.z
        && c->pos.z < b->to.z)
    {
        f = c->tissue->force_rep_factor * dr_len;
        c->force.y += f;
        c->pressure += fabs(f);
    }

    // near...
    dr_len = (c->pos.z - c->r) - b->to.z;
    if (dr_len < 0
        && -dr_len < (b->to.z - b->from.z)*0.5
        && c->pos.x > b->from.x
        && c->pos.x < b->to.x
        && c->pos.y > b->from.y
        && c->pos.y < b->to.y)
    {
        f = -c->tissue->force_rep_factor * dr_len;
        c->force.z += f;
        c->pressure += fabs(f);
    }

    // far...
    dr_len = b->from.z - (c->pos.z + c->r);
    if (dr_len < 0
        && -dr_len < (b->to.z - b->from.z)*0.5
        && c->pos.x > b->from.x
        && c->pos.x < b->to.x
        && c->pos.y > b->from.y
        && c->pos.y < b->to.y)
    {
        f = c->tissue->force_rep_factor * dr_len;
        c->force.z += f;
        c->pressure += fabs(f);
    }
}


static
void cell_barrier_in_force(anyBarrier *b, anyCell *c)
/**
  Calculates forces from barrier.

  \param b -- pointer to barrier
  \param c -- pointer to cell
*/
{
    real dr_len; ///< distance from cell to wall
    real f;

    // left...
    dr_len = c->pos.x - c->r - b->from.x;
    if (dr_len < 0)
    {
        f = -c->tissue->force_rep_factor * dr_len;
        c->force.x += f;
        c->pressure += fabs(f);
    }

    // right...
    dr_len = b->to.x - c->pos.x - c->r;
    if (dr_len < 0)
    {
        f = c->tissue->force_rep_factor * dr_len;
        c->force.x += f;
        c->pressure += fabs(f);
    }

    // top...
    dr_len = c->pos.y - c->r - b->from.y;
    if (dr_len < 0)
    {
        f = -c->tissue->force_rep_factor * dr_len;
        c->force.y += f;
        c->pressure += fabs(f);
    }

    // bottom...
    dr_len = b->to.y - c->pos.y - c->r;
    if (dr_len < 0)
    {
        f = c->tissue->force_rep_factor * dr_len;
        c->force.y += f;
        c->pressure += fabs(f);
    }

    // near...
    dr_len = c->pos.z - c->r - b->from.z;
    if (dr_len < 0)
    {
        f = -c->tissue->force_rep_factor * dr_len;
        c->force.z += f;
        c->pressure += fabs(f);
    }

    // far...
    dr_len = b->to.z - c->pos.z - c->r;
    if (dr_len < 0)
    {
        f = c->tissue->force_rep_factor * dr_len;
        c->force.z += f;
        c->pressure += fabs(f);
    }

}


void CellBarrierForces()
/**
  Highly UNEFFICIENT bariers - cells interactions.
*/
{
    if (SimulationSettings.sim_phases & sat::spForces)
    {
        StartTimer(TimerCellBarrierForcesId);

        anyBarrier *b = scene::FirstBarrier;

        while (b)
        {
            // loop over all cells...
            int first_cell = 0;
            for (int box_id = 0; box_id < SimulationSettings.no_boxes; box_id++)
            {
                int no_cells = scene::Cells[first_cell].no_cells_in_box;
                for (int i = 0; i < no_cells; i++)
                    // grow only active cells...
                    if (scene::Cells[first_cell + i].state != sat::csRemoved)
                    {
                    if (b->type == sat::btIn)
                        cell_barrier_in_force(b, scene::Cells + first_cell + i);
                    else
                        cell_barrier_out_force(b, scene::Cells + first_cell + i);
                }

                first_cell += SimulationSettings.max_cells_per_box;
            }

            b = (anyBarrier *)b->next;
        }
        StopTimer(TimerCellBarrierForcesId);
    }
}


void TissueProperties()
/**
  Calculates tissue properties.
*/
{
    StartTimer(TimerTissuePropertiesId);

    anyTissueSettings *ts = scene::FirstTissueSettings;

    // reset pressures...
    while (ts)
    {
        ts->pressure_sum = 0;
        ts = ts->next;
    }

    // add pressures...
    // loop over all cells...
    int first_cell = 0;
    for (int box_id = 0; box_id < SimulationSettings.no_boxes; box_id++)
    {
        int no_cells = scene::Cells[first_cell].no_cells_in_box;
        for (int i = 0; i < no_cells; i++)
            if (scene::Cells[first_cell + i].state != sat::csRemoved)
            {
               scene::Cells[first_cell + i].tissue->pressure_sum += scene::Cells[first_cell + i].pressure_avg;
            }
        first_cell += SimulationSettings.max_cells_per_box;
    }


    // calculate average pressures...
    ts = scene::FirstTissueSettings;
    while (ts)
    {
        if (ts->no_cells[0] > 0)
            ts->pressure = ts->pressure_sum/ts->no_cells[0];
        else
            ts->pressure = 0;
        ts = ts->next;
    }

    StopTimer(TimerTissuePropertiesId);
}


static
void tube_tube_in_chain_force(anyTube *v)
/**
  Calculates attraction forces between two joined tubes.

  \param v -- pointer to tube (is tip-tip connected with v->next)
*/
{
    anyTube *v_n = v->next;

    anyVector dr = v_n->pos1 - v->pos2;

    anyVector force = dr * TubularSystemSettings.force_chain_attr_factor;

    v->force2 += force;
    v_n->force1 -= force;
}


static
void tube_tube_sprout_force_base(anyTube *v)
/**
  Calculates attraction forces between tubes in sprout.

  \param v -- pointer to tube (is base-connected with v->base)
*/
{
    anyTube *v_b = v->base;
    anyVector force1, force2;
    real dp, v1l, v2l;

    // ortogonalization...
    anyVector v1 = (v->pos2 - v->pos1);
    anyVector v2 = (v_b->pos2 - v_b->pos1);

    v1l = v1.length();
    v2l = v2.length();

    if (v1l == 0 || v2l == 0)
        return;

    real r_mod = v1l/v2l;

    // normalization...
    v1 *= (1/v1l);
    v2 *= (1/v2l);

    real cc = (v1|v2); // - cos(90.0*M_PI/180.0);

    anyVector vv = v1*v2;
    vv.normalize();
    force1 = vv*v1*cc*TubularSystemSettings.force_angle_factor;
    v->force1 += force1;
    v->force2 -= force1;

    force2 = vv*v2*cc*TubularSystemSettings.force_angle_factor*r_mod;
    v_b->force1 -= force2;
    v_b->force2 += force2;

    // connection...
    calc_force((v_b->pos1 + v_b->pos2)*0.5, v->pos1, force1, dp, 0, TubularSystemSettings.force_chain_attr_factor, TubularSystemSettings.force_chain_attr_factor, TubularSystemSettings.force_chain_attr_factor, false);
    v->force1 -= force1;
    v_b->force1 += force1*0.5;
    v_b->force2 += force1*0.5;
}


static
void tube_tube_sprout_force_top(anyTube *v)
/**
  Calculates attraction forces between tubes in sprout.

  \param v -- pointer to tube (is top-connected with v->top)
*/
{
    anyTube *v_b = v->top;
    anyVector force1, force2;
    real dp, v1l, v2l;

    // ortogonalization...
    anyVector v1 = (v->pos1 - v->pos2);
    anyVector v2 = (v_b->pos2 - v_b->pos1);

    v1l = v1.length();
    v2l = v2.length();

    if (v1l == 0 || v2l == 0)
        return;

    real r_mod = v1l/v2l;

    // normalization...
    v1 *= (1/v1l);
    v2 *= (1/v2l);

    real cc = (v1|v2); // - cos(90.0*M_PI/180.0);

    anyVector vv = v1*v2;
    vv.normalize();
    force1 = vv*v1*cc*TubularSystemSettings.force_angle_factor;
    v->force2 += force1;
    v->force1 -= force1;

    force2 = vv*v2*cc*TubularSystemSettings.force_angle_factor*r_mod;
    v_b->force1 -= force2;
    v_b->force2 += force2;


    // connection...
    calc_force((v_b->pos1 + v_b->pos2)*0.5, v->pos2, force1, dp, 0, TubularSystemSettings.force_chain_attr_factor, TubularSystemSettings.force_chain_attr_factor, TubularSystemSettings.force_chain_attr_factor, false);
    v->force2 -= force1;
    v_b->force1 += force1*0.5;
    v_b->force2 += force1*0.5;
}



static
void min_dist_line_to_line(anyVector const &p1, anyVector const &u1,
                           anyVector const &p2, anyVector const &u2,
                           real &t1, real &t2)
{
    anyVector d = p2 - p1;
    anyVector M = u2*u1;
    real m = M|M;
    if (!m)
        t1 = t2 = 0.5;
    else
    {
        anyVector R = d*M*(1/m);

        t1 = R|u2;
        t2 = R|u1;
    }
}


static
void min_dist_point_to_line(anyVector const &c, anyVector const &p1, anyVector const &p2,
                            real &t2)
{
    // http://mathworld.wolfram.com/Point-LineDistance3-Dimensional.html

    real dx = p2.x - p1.x;
    real dy = p2.y - p1.y;
    real dz = p2.z - p1.z;
    real m  = dx*dx + dy*dy + dz*dz;
    if (m == 0)
        t2 = 0;
    else
        t2 = -(dx*(p1.x - c.x) + dy*(p1.y - c.y) + dz*(p1.z - c.z))/m;
}


static
bool is_out(real x)
{
    return x < 0 || x > 1;
}

static
void norm_t(real &x)
{
    if (x < 0) x = 0;
    else if (x > 1) x = 1;
}


static
void tube_tube_force(anyTube *v1, anyTube *v2)
{
    real t1, t2;
    anyVector p1;
    anyVector p2;

    min_dist_line_to_line(v1->pos1, v1->pos2 - v1->pos1,
                          v2->pos1, v2->pos2 - v2->pos1,
                          t1, t2);

    if (is_out(t1))
    {
        norm_t(t1);
        p1 = v1->pos1*(1-t1) + v1->pos2*t1;

        min_dist_point_to_line(p1, v2->pos1, v2->pos2, t2);
        if (is_out(t2))
        {
            norm_t(t2);
            p2 = v2->pos1*(1-t2) + v2->pos2*t2;

            min_dist_point_to_line(p2, v1->pos1, v1->pos2, t1);
            norm_t(t1);
            p1 = v1->pos1*(1-t1) + v1->pos2*t1;
        }
        else
            p2 = v2->pos1*(1-t2) + v2->pos2*t2;
    }
    else if (is_out(t2))
    {
        norm_t(t2);
        p2 = v2->pos1*(1-t2) + v2->pos2*t2;

        min_dist_point_to_line(p2, v1->pos1, v1->pos2, t1);
        if (is_out(t1))
        {
            norm_t(t1);
            p1 = v1->pos1*(1-t1) + v1->pos2*t1;

            min_dist_point_to_line(p1, v2->pos1, v2->pos2, t2);
            norm_t(t2);
            p2 = v2->pos1*(1-t2) + v2->pos2*t2;
        }
        else
            p1 = v1->pos1*(1-t1) + v1->pos2*t1;
    }

    // force...
    anyVector force;
    real dp;
    if (!calc_force(p1, p2,
                   force, dp,
                   v1->r + v2->r,
                   TubularSystemSettings.force_rep_factor,
                   TubularSystemSettings.force_atr1_factor,
                   TubularSystemSettings.force_atr2_factor,
                   true))
        return;
    //LOG(llDebug, QString("Force! p1=") + p1.to_string() + ", p2=" + p2.to_string() + ", force=" + force.to_string());

    v1->nei_cnt++;
    v2->nei_cnt++;

    // tube connecting...
    if (!v1->fixed_blood_pressure && !v2->fixed_blood_pressure)
    {
        // tube connecting (case #1 - top->top)...
        if (!v1->next && !v1->top && !v2->next && !v2->top)
        {
            if ((v1->pos2 - v2->pos2).length2() < SimulationSettings.force_r_cut2)
                scene::AddTubesToMerge(v1, v2);
        }

        // tube connecting (case #2 - top->middle)
        else if (!v1->next && !v1->top && !v2->fork && !v2->jab)
        {
            if ((v1->pos2 - (v2->pos2 + v2->pos1)*0.5).length2() < SimulationSettings.force_r_cut2)
            {
                v1->top = v2;
                v2->jab = v1;
            }

        }

        // tube connecting (case #3 - middle->top)
        else if (!v2->next && !v2->top && !v1->fork && !v1->jab)
        {
            if ((v2->pos2 - (v1->pos2 + v1->pos1)*0.5).length2() < SimulationSettings.force_r_cut2)
            {
                v2->top = v1;
                v1->jab = v2;
            }

        }
    }



    if (SimulationSettings.sim_phases & sat::spForces)
    {
        v1->force1 += force*(1 - t1);
        v1->force2 += force*t1;
        v2->force1 -= force*(1 - t2);
        v2->force2 -= force*t2;
    }
}


void TubeTubeInChainsForces()
/**
  Calculates forces between joined tubes.
*/
{
    for (int i = 0; i < scene::NoTubeChains; i++)
    {
        anyTube *v = scene::TubeChains[i];
        while (v)
        {
            if (v->next)
                tube_tube_in_chain_force(v);
            if (v->base)
                tube_tube_sprout_force_base(v);
            if (v->top)
                tube_tube_sprout_force_top(v);
            v = v->next;
        }
    }
}


void TubeTubeOutChainsForces()
/**
  Calculates forces between not joined tubes.
*/
{
    // loop over all boxes...
    anyTube *v1, *v2;
    int box_id = 0;
    for (int box_z = 0; box_z < SimulationSettings.no_boxes_z; box_z++)
        for (int box_y = 0; box_y < SimulationSettings.no_boxes_y; box_y++)
            for (int box_x = 0; box_x < SimulationSettings.no_boxes_x; box_x++, box_id++)
            {
                // loop over all pairs of tubes in box...
                for (int i = 0; i < scene::BoxedTubes[box_id].no_tubes - 1; i++)
                    for (int j = i + 1; j < scene::BoxedTubes[box_id].no_tubes; j++)
                    {
                        v1 = scene::BoxedTubes[box_id].tubes[i];
                        v2 = scene::BoxedTubes[box_id].tubes[j];
                        if (!scene::TubesJoined(v1, v2) && box_x == MAX(v1->nx, v2->nx) && box_y == MAX(v1->ny, v2->ny) && box_z == MAX(v1->nz, v2->nz))
                            tube_tube_force(v1, v2);
                    }
    }
}


static
void tube_length_force(anyTube *v)
/**
  Keeps defined distance between tube's ends.

  \param v -- pointer to tube
*/
{
    anyVector dr = v->pos2 - v->pos1;
    real dl = dr.length() - v->length;

    dr.normalize();
    anyVector force = dr*dl*TubularSystemSettings.force_length_keep_factor;

    v->force1 += force;
    v->force2 -= force;
}


void TubeLengthForces()
/**
  Keeps defined length of all tubes.
*/
{
    for (int i = 0; i < scene::NoTubeChains; i++)
    {
        anyTube *v = scene::TubeChains[i];
        while (v)
        {
            tube_length_force(v);
            v = v->next;
        }
    }
}


static
void tube_cell_force(anyTube *v, anyCell *c)
/**
  Calculates forces between cell and tube.

  \param c -- pointer to cell
  \param v -- pointer to tube
*/
{
    real p = 0.5;

    if (SimulationSettings.sim_phases & sat::spForces)
    {
        anyVector p12 = v->pos2 - v->pos1;
        anyVector pc = c->pos - v->pos1;
        real p12_len = p12.length();
        anyVector p12_n = p12;
        p12_n.normalize();

        real d = pc|p12_n;

        if (d <= 0)
        {
            // near p1...
            p = 0;
        }
        else if (d >= p12_len)
        {
            // near p2...
            p = 1;
        }
        else
        {
            // between p1 and p2...
            p = d/p12_len;
        }

        anyVector force;
        real dp;
        if (!calc_force(c->pos, v->pos1*(1 - p) + v->pos2*p,
                        force, dp,
                        v->r + c->r,
                        c->tissue->force_rep_factor,
                        c->tissue->force_atr1_factor,
                        c->tissue->force_atr2_factor,
                        true
                        ))
            return;

        c->nei_cnt[sat::ttNormal]++;
        v->nei_cnt++;

        c->force += force;
        v->force1 -= force*(1 - p);
        v->force2 -= force*p;

        c->pressure += dp;
        v->pressure += dp;
        c->pressure_sum += v->pressure_prev;
        v->pressure_sum += c->pressure_prev;
    }

    if ((SimulationSettings.sim_phases & sat::spDiffusion) && v->blood_flow)
    {
        const real vessel_conc_accel = 0.25;
        concentration_exchange(c->concentrations, v->concentrations,
                               c->r*vessel_conc_accel, c->r*vessel_conc_accel,
                               (c->r+c->r)*(c->r+c->r)*vessel_conc_accel*vessel_conc_accel,
//                               c->r*c->r*0.25,
                               true);
    }
}


void TubeCellForces()
{
    if (SimulationSettings.sim_phases & sat::spForces)
    {
        int x1, y1, z1, x2, y2, z2;

        StartTimer(TimerTubeCellForcesId);

        // loop for every tube...
        for (int i = 0; i < scene::NoTubeChains; i++)
        {
            anyTube *v = scene::TubeChains[i];
            while (v)
            {
                // corner boxes...
                x1 = floor((v->pos1.x - SimulationSettings.comp_box_from.x)/SimulationSettings.box_size);
                y1 = floor((v->pos1.y - SimulationSettings.comp_box_from.y)/SimulationSettings.box_size);
                z1 = floor((v->pos1.z - SimulationSettings.comp_box_from.z)/SimulationSettings.box_size);

                x2 = floor((v->pos2.x - SimulationSettings.comp_box_from.x)/SimulationSettings.box_size);
                y2 = floor((v->pos2.y - SimulationSettings.comp_box_from.y)/SimulationSettings.box_size);
                z2 = floor((v->pos2.z - SimulationSettings.comp_box_from.z)/SimulationSettings.box_size);

                if (x1 > x2) SWAP(int, x1, x2);
                if (y1 > y2) SWAP(int, y1, y2);
                if (z1 > z2) SWAP(int, z1, z2);

                //  loop over all boxes...
                /// \todo: optimalization!!!
                for (int x = x1 - 1; x <= x2 + 1; x++)
                    for (int y = y1 - 1; y <= y2 + 1; y++)
                        for (int z = z1 - 1; z <= z2 + 1; z++)
                            if (VALID_BOX(x, y, z))
                            {
                                // loop over all particles in box...
                                int first_cell = BOX_ID(x, y, z)*SimulationSettings.max_cells_per_box;
                                int no_cells = scene::Cells[first_cell].no_cells_in_box;
                                for (int j = 0; j < no_cells; j++)
                                // only active cells...
                                if (scene::Cells[first_cell + j].state != sat::csRemoved)
                                {
                                    tube_cell_force(v, scene::Cells + first_cell + j);
                                }
                            }
                v = v->next;
            }
        }
        StopTimer(TimerTubeCellForcesId);
    }
}


void TubeTubeForces()
/**
  Calculates forces between tubes.
*/
{
    if (SimulationSettings.sim_phases & sat::spForces)
    {
        StartTimer(TimerTubeTubeForcesId);

        // in chains...
        TubeTubeInChainsForces();

        // out of chains...
        TubeTubeOutChainsForces();

        // lengths...
        TubeLengthForces();

        StopTimer(TimerTubeTubeForcesId);
    }
}




void GrowTube(anyTube *v)
{
    if (SimulationSettings.dimensions == 2)
        v->force1.z = v->force2.z = 0;

    // move...

    // dv = F/m*dt...
    anyVector dv1(v->force1.x*SimulationSettings.time_step*v->one_by_mass*2,
                  v->force1.y*SimulationSettings.time_step*v->one_by_mass*2,
                  v->force1.z*SimulationSettings.time_step*v->one_by_mass*2);
    anyVector dv2(v->force2.x*SimulationSettings.time_step*v->one_by_mass*2,
                  v->force2.y*SimulationSettings.time_step*v->one_by_mass*2,
                  v->force2.z*SimulationSettings.time_step*v->one_by_mass*2);
    // v += dv...
    v->velocity1 += dv1;
    v->velocity2 += dv2;

    limit_velocity(v->velocity1, SimulationSettings.time_step);
    limit_velocity(v->velocity2, SimulationSettings.time_step);

    // dr = v*dt
    anyVector dr1(v->velocity1.x*SimulationSettings.time_step,
                  v->velocity1.y*SimulationSettings.time_step,
                  v->velocity1.z*SimulationSettings.time_step);
    anyVector dr2(v->velocity2.x*SimulationSettings.time_step,
                  v->velocity2.y*SimulationSettings.time_step,
                  v->velocity2.z*SimulationSettings.time_step);

    // r += dr...
    v->pos1 += dr1;
    v->pos2 += dr2;

    // velocity dumping...
    v->velocity1 *= 0.5;
    v->velocity2 *= 0.5;

    // tip division...
    if (SimulationSettings.sim_phases & sat::spTubeDiv
        && !v->next
        && !v->top
        && (v->pos1 - v->pos2).length2() >= (v->final_length)*(v->final_length)*0.99
        && (v->age > TubularSystemSettings.minimum_interphase_time)
        && (v->taf_triggered)
       )
    {
        // create new tube and copy data...
        anyTube *v2 = new anyTube;
        *v2 = *v;
        v2->base = v2->fork = 0;

        change_tube_state(v2, sat::csAdded);
        v2->age = 0;

        // new ending points...
        anyVector mid_point = (v->pos1 + v->pos2)*0.5;
        v2->pos1 = v->pos2 = mid_point;
        v2->length = v->length = v->length*0.5;
        scene::SetTubeMass(v);
        v2->one_by_mass = v->one_by_mass;

        scene::AddTube(v2, false, false);

        // linkage...
        v->next = v2;
        v2->prev = v;
        v2->base = 0;
        v2->fork = 0;
    }

    // sprout division...
    else if (SimulationSettings.sim_phases & sat::spTubeDiv
        && (v->next)
        && (!v->fork)
        && (!v->jab)
        && (v->pos1 - v->pos2).length2() >= (v->final_length)*(v->final_length)*0.99
        && (v->age > TubularSystemSettings.minimum_interphase_time)
        && (v->taf_triggered)
       )
    {
        // create new tube and copy data...
        anyTube *v2 = new anyTube;
        *v2 = *v;

        change_tube_state(v2, sat::csAdded);
        v2->age = 0;

        // new ending points...
        anyVector end_point;
        if (SimulationSettings.dimensions == 3)
        {
            // 3d...
            end_point.set_random(3, 1);
            end_point = (v->pos2 - v->pos1)*end_point;
        }
        else
        {
            // 2d...
            end_point = v->pos2 - v->pos1;
            end_point.set(-end_point.y, end_point.x, end_point.z);
            if (rand()%2)
                end_point = end_point*-1;
        }
        end_point.normalize();
        end_point = end_point*v->final_length*0.1;
        anyVector mid_point = (v->pos1 + v->pos2)*0.5;
        v2->pos1 = mid_point;
        v2->pos2 = mid_point + end_point;

        v2->length = v->final_length*0.1;
        v2->r = v->final_r*0.5;
        scene::SetTubeMass(v2);
        //v2->one_by_mass *= 10;

        scene::AddTube(v2, false, true);

        // linkage...
        v->fork = v2;
        v2->base = v;
        v2->next = 0;
        v2->prev = 0;
    }

    // state, length and radius change...
    switch (v->state)
    {
    case sat::csAdded:
    case sat::csRemoved:
        break;
    case sat::csAlive:
        // death?...
        if (!v->fixed_blood_pressure &&
            SimulationSettings.time - v->flow_time > TubularSystemSettings.time_to_degradation &&
            !v->next && !v->top && !v->fork && !v->jab)
            v->state = sat::csRemoved;

        // lengthening...
        if (v->length < v->final_length)
        {
            v->length += TubularSystemSettings.lengthening_speed*SimulationSettings.time_step;
            if (v->length > v->final_length)
                v->length = v->final_length;
            scene::SetTubeMass(v);
        }

        // thickening...
        if (v->r < v->final_r)
        {
            v->r += TubularSystemSettings.thickening_speed*SimulationSettings.time_step;
            if (v->r > v->final_r)
                v->r = v->final_r;
            scene::SetTubeMass(v);
        }

        break;
    default:
        throw new Error(__FILE__, __LINE__, "Unexpected state of cell");
    }

    v->taf_triggered = v->concentrations[sat::dsTAF][conc_step_current()] > TubularSystemSettings.TAFtrigger;

    // concentration changes...
    v->concentrations[sat::dsTAF][conc_step_current()] = 0;
    if (v->blood_flow != 0)
        v->concentrations[sat::dsO2][conc_step_current()] = 1;

    // update timers...
    v->age += SimulationSettings.time_step;
    v->state_age += SimulationSettings.time_step;

    // flow timer...
    if (ABS(v->blood_flow) > 0)
        v->flow_time = SimulationSettings.time;
}


void GrowAllTubes()
/**
  Growth of all tubes.
*/
{
    if (SimulationSettings.sim_phases & sat::spGrow)
    {
        StartTimer(TimerTubeGrowId);

        for (int i = 0; i < scene::NoTubeChains; i++)
        {
            anyTube *v = scene::TubeChains[i];
            while (v)
            {
                if (v->state != sat::csAdded)
                    GrowTube(v);
                v = v->next;
            }
        }
        StopTimer(TimerTubeGrowId);
    }
}


void CopyConcentrations()
{
    StartTimer(TimerCopyConcentrationsId);

    int first_cell = 0;
    for (int box_id = 0; box_id < SimulationSettings.no_boxes; box_id++)
    {
        int no_cells = scene::Cells[first_cell].no_cells_in_box;
        for (int i = 0; i < no_cells; i++)
        {
            anyCell &currentCell = scene::Cells[first_cell + i];

            for (int k = 0; k < sat::dsLast; k++)
              currentCell.concentrations[k][conc_step_prev()] = currentCell.concentrations[k][conc_step_current()];
        }
        first_cell += SimulationSettings.max_cells_per_box;
    }

    StopTimer(TimerCopyConcentrationsId);
}


void ResetForces()
{
    StartTimer(TimerResetForcesId);

    // cells...
    int first_cell = 0;
    for (int box_id = 0; box_id < SimulationSettings.no_boxes; box_id++)
    {
        int no_cells = scene::Cells[first_cell].no_cells_in_box;
        for (int i = 0; i < no_cells; i++)
        {
            anyCell& currentCell = scene::Cells[first_cell + i];
            currentCell.force.set(0, 0, 0);
            currentCell.nei_cnt[sat::ttNormal] = currentCell.nei_cnt[sat::ttTumor] = 0;
        }
        first_cell += SimulationSettings.max_cells_per_box;
    }

    // tubes...
    for (int i = 0; i < scene::NoTubeChains; i++)
    {
        anyTube *v = scene::TubeChains[i];
        while (v)
        {
            v->force1.set(0, 0, 0);
            v->force2.set(0, 0, 0);
            v->nei_cnt = 0;

            v = v->next;
        }
    }


    StopTimer(TimerResetForcesId);
}


void UpdatePressures()
{
    StartTimer(TimerUpdatePressuresId);

    // cells...
    int first_cell = 0;
    for (int box_id = 0; box_id < SimulationSettings.no_boxes; box_id++)
    {
        int no_cells = scene::Cells[first_cell].no_cells_in_box;
        for (int i = 0; i < no_cells; i++)
        {
            anyCell &currentCell = scene::Cells[first_cell + i];
            /* Pressure is counted from forces and should conform following conditions:
            * -when system is in stable state, pressure should have similiar values in all cells
            * -when pressure is not equall, system should move to position in which it will be equall
            * -pressure do not have indirect influence on forcess or cells position only on internal state of cell
            * -pressure is calculated directly from forces acting on this cell and is not in direct way connected with pressures of other cells
            */

            if (SimulationSettings.dimensions == 3)
                currentCell.pressure = (currentCell.pressure) / (currentCell.r * sqrt(currentCell.r));
            else
                currentCell.pressure = (currentCell.pressure) / currentCell.r;

            currentCell.pressure_avg = currentCell.pressure_sum/(currentCell.nei_cnt[sat::ttNormal] + currentCell.nei_cnt[sat::ttTumor] + 1);
            currentCell.pressure_prev = currentCell.pressure;
            currentCell.pressure_sum = currentCell.pressure_prev;
        }

        first_cell += SimulationSettings.max_cells_per_box;
    }

    // tubes...
    for (int i = 0; i < scene::NoTubeChains; i++)
    {
        anyTube *v = scene::TubeChains[i];
        while (v)
        {
            if (SimulationSettings.dimensions == 3)
                v->pressure = (v->pressure) / (v->r*sqrt(v->r));
            else
                v->pressure = (v->pressure) / v->r;

            v->pressure_avg = v->pressure_sum/(v->nei_cnt + 1);
            v->pressure_prev = v->pressure;
            v->pressure_sum = v->pressure_prev;
            v = v->next;
        }
    }

    StopTimer(TimerUpdatePressuresId);
}


void RemoveTubes()
{
    StartTimer(TimerRemoveTubesId);

    for (int i = 0; i < scene::NoTubeChains; i++)
    {
        anyTube *v = scene::TubeChains[i];
        while (v)
        {
            if (v->state == sat::csRemoved)
            {
                // forked?...
                if (v->base)
                    v->base->fork = 0;

                // in chain?...
                if (v->prev)
                    v->prev->next = 0;
                else
                {
                    // first in chain...
                    scene::TubeChains[i] = scene::TubeChains[--scene::NoTubeChains];
                    i--;
                }

                delete v;
                scene::NoTubes--;
                break;
            }
            v = v->next;
        }
    }

    StopTimer(TimerRemoveTubesId);
}


void BloodFlow()
{
    if (SimulationSettings.sim_phases & sat::spBloodFlow)
    {
        StartTimer(TimerBloodFlowId);

        // pressure recalculation...
        for (int i = 0; i < scene::NoTubeChains; i++)
        {
            anyTube *v = scene::TubeChains[i];
            while (v)
            {
                if (!v->fixed_blood_pressure)
                {
                    real np = 0;
                    int np_cnt = 0;
                    if (v->next)
                    {
                        np += v->next->blood_pressure;
                        np_cnt++;
                    }
                    if (v->prev)
                    {
                        np += v->prev->blood_pressure;
                        np_cnt++;
                    }
                    if (v->base)
                    {
                        np += v->base->blood_pressure;
                        np_cnt++;
                    }
                    if (v->fork)
                    {
                        np += v->fork->blood_pressure;
                        np_cnt++;
                    }
                    if (v->top)
                    {
                        np += v->top->blood_pressure;
                        np_cnt++;
                    }
                    if (v->jab)
                    {
                        np += v->jab->blood_pressure;
                        np_cnt++;
                    }

                    if (np_cnt)
                    {
                        v->blood_pressure = np/np_cnt;
                    }
                }

                v = v->next;
            }
        }



        for (int i = 0; i < scene::NoTubeChains; i++)
        {
            anyTube *v = scene::TubeChains[i];
            while (v)
            {
                real p1 = 0, p2 = 0;
                bool p1p, p2p;

                p1p = true;
                if (v->prev)
                    p1 = v->prev->blood_pressure;
                else if (v->base)
                    p1 = v->base->blood_pressure;
                else if (v->fork)
                    p1 = v->fork->blood_pressure;
                else
                    p1p = false;

                p2p = true;
                if (v->next)
                    p2 = v->next->blood_pressure;
                else if (v->top)
                    p2 = v->top->blood_pressure;
                else if (v->jab)
                    p2 = v->jab->blood_pressure;
                else
                    p2p = false;

                if (p1p && p2p && ABS(p2-p1) > TubularSystemSettings.minimum_blood_flow)
                    v->blood_flow = p2 - p1;
                else
                    v->blood_flow = 0;

//                if (v->id == 97) qDebug("%g %g %g", p1, p2, p2-p1);


                v = v->next;
            }
        }

        StopTimer(TimerBloodFlowId);
    }
}


void TimeStep()
/**
  Time step of simulation.
*/
{
    StartTimer(TimerSimulationId);

    // update tubes..., timer: TimerTubeUpdateId
    UpdateTubes();

    // Reset forces in cells and tubes..., timer: TimerResetForcesId
    ResetForces();

    // forces, part I (cells)..., timer: TimerCellCellForcesId
    CellCellForces();

    // forces, part II (barriers)..., timer: TimerCellBarrierForcesId
    CellBarrierForces();

    // forces, part III (tubes)..., timer: TimerTubeTubeForcesId
    TubeTubeForces();

    // forces, part IV (tubes-cells)..., timer: TimerTubeCellForcesId
    TubeCellForces();

    // growth of cells..., timer: TimerCellGrowId
    GrowAllCells();

    // growth of tubes..., timer: TimerTubeGrowId
    GrowAllTubes();

    // rearange cells array..., timer: TimerRearangeId
    RearrangeCells();

    // remove csRemoved tubes..., timer: TimerRemoveTubesId
    RemoveTubes();

    // connect tube chains..., timer: TimerConnectTubeChainsId
    ConnectTubeChains();

    // merge tube chains..., timer: TimerMergeTubesId
    scene::MergeTubes();

    // update pressures..., timer: TimerUpdatePressuresId
    UpdatePressures();

    // copy concentrations for next step & visualization..., timer: TimerCopyConcentrationsId
    CopyConcentrations();

    // blood flow..., timer: TimerBloodFlowId
    BloodFlow();

    // tissue properties..., timer: TimerTissuePropertiesId
    TissueProperties();

    // update time...
    SimulationSettings.time += SimulationSettings.time_step;
    SimulationSettings.step++;

    StopTimer(TimerSimulationId);

}


