#include "anysimulationsettings.h"

#include "config.h"

anySimulationSettings::anySimulationSettings(){}

void anySimulationSettings::reset()
{
    step = 0;
    max_o2_concentration = 1e-26f;

    char fname[P_MAX_PATH];
    snprintf(fname, P_MAX_PATH, "%s%ssimulation_settings.ag", GlobalSettings.app_dir, FOLDER_DEFAULTS);
    Slashify(fname, false);
    LOG2(llInfo, "Reading defaults from ", fname);
    FILE *f = fopen(fname, "r");
    if (!f)
        throw new Error(__FILE__, __LINE__, "Cannot open file for reading", 0, fname);
    else
    {
        ParseSimulationSettings(f);
        fclose(f);
    }

    calculate_derived_values();
}

void anySimulationSettings::calculate_derived_values()
{
    force_r_peak = force_r_cut/5;
    force_r_cut2 = force_r_cut*force_r_cut;
    farest_point = MAX(comp_box_from.length(), comp_box_to.length());
}
