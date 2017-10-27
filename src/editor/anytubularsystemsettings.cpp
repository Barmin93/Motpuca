#include "anytubularsystemsettings.h"

#include "config.h"

anyTubularSystemSettings::anyTubularSystemSettings(){}
void anyTubularSystemSettings::reset()
{
    char fname[P_MAX_PATH];
    snprintf(fname, P_MAX_PATH, "%s%stubular_settings.ag", GlobalSettings.app_dir, FOLDER_DEFAULTS);
    Slashify(fname, false);
    LOG2(llInfo, "Reading defaults from ", fname);
    FILE *f = fopen(fname, "r");
    if (!f)
        throw new Error(__FILE__, __LINE__, "Cannot open file for reading", 0, fname);
    else
    {
        ParseTubularSystemSettings(f);
        fclose(f);
    }

}
