#ifndef ANYTISSUESETTINGS_H
#define ANYTISSUESETTINGS_H

#include "anyeditable.h"
#include "color.h"
#include "types.h"
#include "config.h"
#include "anycellblock.h"

class anyTissueSettings: public anyEditable
/**
  Structure defining tissue properties.

  Adding new field requires adding of:
    - default value in constructor                           [just below]
    - section to SaveTissueSettings_ag()                     [scene.cpp]
    - section to ParseTissueSettingsValue()                  [scene.cpp]
    - new widget in properties dialogEditable.ui and
        - section in anyTissueSettings::prepare_dialog()     [scene.cpp]
        - section in anyTissueSettings::update_from_dialog() [scene.cpp]
    - section to documentation
*/
{
public:
    anyColor color;            ///< color
    sat::TissueType type;           ///< type
    char *name;                ///< unique name of tissue settings
    float cell_r;               ///< radius of mature cell [um]
    float density;              ///< tissue density [kg/m^3]
    float cell_grow_speed;      ///< speed of cell growing [um/s]
    float minimum_interphase_time; ///< miniumum interphase time [s]
    float time_to_apoptosis;    ///< maximal age of cell [s]
    float time_to_necrosis;     ///< time in apoptosis or hypoxia before switching to necrosis [s]
    float time_to_necrosis_var; ///< variance of time_to_necrosis [s]
    float time_in_necrosis;     ///< time in necrosis state [s]
    float dead_r;               ///< size of dead cell [um]
    float cell_shrink_speed;    ///< speed of shrinking of dead cell [um/s]
    float minimum_mitosis_r;    ///< minimum radius for mitosis [um]

    float force_rep_factor;     ///< repulsion force factor
    float force_atr1_factor;    ///< first attraction force factor
    float force_atr2_factor;    ///< second attraction force factor
    float force_dpd_factor;     ///< PDP gamma factor
    float dpd_temperature;      ///< T in DPD random force
    float max_pressure;         ///< maximum pressure allowing for growth and proliferation
    float o2_consumption;       ///< oxygen consumption ratio in o2 kg/ cell kg /s
    float pericyte_production;  ///< pericyte production
    float o2_hypoxia;           ///< o2 concentration that leads to hypoxia
    anyTissueSettings *next;   ///< pointer to next tissue

    // aux fields...
    int id;                    ///< id of tissue (0, 1,...)
    int no_cells[sat::csLast];      ///< number of cells (0 - all; 2...5 - for every state)
    float pressure_sum;         ///< sum of cell pressures
    float pressure;             ///< average pressure

    anyTissueSettings();

    virtual ~anyTissueSettings() {}
    virtual void update_bounding_box(anyVector &/*from*/, anyVector &/*to*/, bool &/*first*/)
    {
        // tissue settings do not influence on simulation bounding box
    }
    virtual void read_defaults();

#ifdef QT_CORE_LIB
    virtual void prepare_dialog();
    virtual bool validate_properties();
    virtual bool validate_removal();
    virtual void update_from_dialog();
#endif

    virtual void add_itself_to_scene();
    virtual void remove_itself_from_scene();
    virtual char *get_name();
#ifdef QT_CORE_LIB
    virtual void display_properties(QTextBrowser *tb);
#endif
    float billion_to_inf(float x);
    QString float_to_str(float x);

};

#endif // ANYTISSUESETTINGS_H
