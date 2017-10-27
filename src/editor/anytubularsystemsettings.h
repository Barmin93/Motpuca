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
    real force_chain_attr_factor;  ///< in chain attraction force factor
    real force_length_keep_factor; ///< length keeping force factor
    real force_rep_factor;         ///< tube-tube repulsion factor
    real force_angle_factor;       ///< sprout angle force factor
    real force_atr1_factor;        ///< tube-tube attraction factor #1
    real force_atr2_factor;        ///< tube-tube attraction factor #2
    real density;                  ///< density of tube [kg/m^3]
    real lengthening_speed;        ///< tube lengthening speed [m/s]
    real thickening_speed;         ///< tube thickening speed [m/s]
    real minimum_interphase_time;  ///< minimum time between tube division [s]

    // vasculat specific attributes...
    real TAFtrigger;               ///< TAF concentration triggering angiogenesis [%]
    real minimum_blood_flow;       ///< minimum blood flow which is considered as blood flow
    real o2_production;            ///< how much oxygen produce one tube per box
    real time_to_degradation;      ///< time to tube degradation

    anyTubularSystemSettings();

    void reset();

};

extern anyTubularSystemSettings TubularSystemSettings;

#endif // ANYTUBULARSYSTEMSETTINGS_H
