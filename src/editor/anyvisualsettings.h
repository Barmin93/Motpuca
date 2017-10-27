#ifndef ANYVISUALSETTINGS_H
#define ANYVISUALSETTINGS_H

#include "color.h"
#include "transform.h"
#include "anyglobalsettings.h"
#include "log.h"
#include "parser.h"

class anyVisualSettings
/**
  Global visual settings.

  Adding new field requires adding of:
    - default value in reset
    - section to SaveVisualSettings_ag()
    - section to ParseVisualSettingsValue()
    - section to documentation
*/
{
public:
    anyTransform v_matrix; ///< view matrix
    anyTransform r_matrix; ///< rotation only matrix
    anyTransform p_matrix; ///< perspective martrix

    int window_width;      ///< initial width of program window
    int window_height;     ///< initial height of program window

    // colors...
    anyColor bkg_color; ///< background color
    anyColor axis_x_color;     ///< color of X axis
    anyColor axis_y_color;     ///< color of Y axis
    anyColor axis_z_color;     ///< color of Z axis
    anyColor comp_box_color;   ///< color of computational box
    anyColor in_barrier_color; ///< color of 'in' barrier
    anyColor out_barrier_color; ///< color of 'out' barrier
    anyColor navigator_color;  ///< color of navigator
    anyColor boxes_color;      ///< color of boxes
    anyColor selection_color;  ///< selection color

    anyColor cell_alive_color;     ///< color of alive cells
    anyColor cell_hypoxia_color;   ///< color of cells in hypoxia
    anyColor cell_apoptosis_color; ///< color of cells in apoptosis
    anyColor cell_necrosis_color;  ///< color of cells in necrosis
    anyColor clip_plane_color;     ///< color of clipping plane marker
    anyColor tube_color;           ///< color of tubes

    // light...
    anyVector light_dir;  ///< light direction
    anyVector light_dir_r;  ///< rotated light direction

    // clipping plane...
    anyTransform clip;    ///< clipping matrix
    double clip_plane[4]; ///< clipping plane equation

    // aux...
    anyVector eye;

    anyVisualSettings();

    void reset();

    void calculate_derived_values();

    void comp_clip_plane();

    void comp_light_dir();

};

extern anyVisualSettings VisualSettings;

#endif // ANYVISUALSETTINGS_H
