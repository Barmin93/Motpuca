#ifndef ANYTUBULARSYSTEMSETTINGS_H
#define ANYTUBULARSYSTEMSETTINGS_H

#include "const.h"

class anyTubularSystemSettings
/**
  Global tube settings.

  Adding new field requires adding of:
    - default value in reset
    - section to SaveTubularSettings_ag()
    - section to ParseTubeValue()
    - section to documentation
*/
{
public:
    // general attributes...
    float force_chain_attr_factor;  ///< in chain attraction force factor
    float force_length_keep_factor; ///< length keeping force factor
    float force_rep_factor;         ///< tube-tube repulsion factor
    float force_angle_factor;       ///< sprout angle force factor
    float force_atr1_factor;        ///< tube-tube attraction factor #1
    float force_atr2_factor;        ///< tube-tube attraction factor #2
    float density;                  ///< density of tube [kg/m^3]
    float lengthening_speed;        ///< tube lengthening speed [m/s]
    float thickening_speed;         ///< tube thickening speed [m/s]
    float minimum_interphase_time;  ///< minimum time between tube division [s]

    // vasculat specific attributes...
    float TAFtrigger;               ///< TAF concentration triggering angiogenesis [%]
    float minimum_blood_flow;       ///< minimum blood flow which is considered as blood flow
    float o2_production;            ///< how much oxygen produce one tube per box
    float time_to_degradation;      ///< time to tube degradation

    anyTubularSystemSettings();

    void reset();

};

extern anyTubularSystemSettings TubularSystemSettings;

#endif // ANYTUBULARSYSTEMSETTINGS_H
