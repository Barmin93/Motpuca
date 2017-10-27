#include "anyvisualsettings.h"

#include "config.h"

anyVisualSettings::anyVisualSettings(){}

void anyVisualSettings::reset()
{
    char fname[P_MAX_PATH];
    snprintf(fname, P_MAX_PATH, "%s%svisual_settings.ag", GlobalSettings.app_dir, FOLDER_DEFAULTS);
    Slashify(fname, false);
    LOG2(llInfo, "Reading defaults from ", fname);
    FILE *f = fopen(fname, "r");
    if (!f)
        throw new Error(__FILE__, __LINE__, "Cannot open file for reading", 0, fname);
    else
    {
        ParseVisualSettings(f);
        fclose(f);
    }

    v_matrix.setToIdentity();
    p_matrix.setToIdentity();
    r_matrix.setToIdentity();
    clip.setToRotationZ(90);
    comp_clip_plane();
    comp_light_dir();
    eye.set(0, 0, -1000);

    calculate_derived_values();
}

void anyVisualSettings::calculate_derived_values()
{

}

void anyVisualSettings::comp_clip_plane()
{
    clip_plane[0] = -clip.matrix[8];
    clip_plane[1] = -clip.matrix[9];
    clip_plane[2] = -clip.matrix[10];
    clip_plane[3] = clip.matrix[8]*clip.matrix[12] +
                    clip.matrix[9]*clip.matrix[13] +
                    clip.matrix[10]*clip.matrix[14];

}

void anyVisualSettings::comp_light_dir()
{
    light_dir_r = r_matrix.inverted()*light_dir;
    light_dir_r.normalize();
}
