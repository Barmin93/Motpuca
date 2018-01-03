#ifndef CONST_H
#define CONST_H

#define APP_NAME "Motpuca" // version defined in version.h

#define FOLDER_USER "Motpuca/"
#define FOLDER_INPUT_FILES "data/"
#define FOLDER_OUTPUT "output/"
#define FOLDER_DEFAULTS "defaults/"
#define FOLDER_LIB_TISSUES "lib/tissues/"
#define FOLDER_LIB_POVRAY "lib/povray/"

#define P_MAX_PATH 1024

#include <map>
#include <iostream>
#include <string>
#include "types.h"

namespace c{
    // kernel radius (promien odciecia)
    const float H                = 0.03125f; //def = 0.03125f
    const float gasStiffness     = 20.0f; // incompressibility can only be obtained as k -> infinity.   [N*m/kg]
    const float restDensity      = 200.0f; //115.f   [kg/m^3]
    const float particleMass     = 0.0018f; // m = ρ*(0.66*H)^3   [kg]
    const float viscosity        = 20.0f; //0.005f; def = 1.5f  // [N*s / m^2]
    const float surfaceTension   = 0.25f;
    const float interfaceTension = 0.15f;
    const float surfaceThreshold = 0.00001f;
    const float gravityAcc       = -9.80665f;
    const float PIf = 3.14159265358979323846f;

}

namespace sat {
    enum CellState { csAdded, csRemoved, csAlive, csHypoxia, csApoptosis, csNecrosis, csLast };  ///< states of cell. csAdded and csRemoved must be at the beginnig
    enum TissueType { ttNormal, ttTumor };
    enum BarrierType { btIn, btOut };
    enum DiffundingSubstances {dsO2, dsTAF, dsPericytes, dsLast};
    enum anyRunEnv { reUnknown, reProduction, reDebug, reRelease };
    enum anySimPhase { spForces = 0x0001, spGrow = 0x0002, spMitosis = 0x0004, spDiffusion = 0x0008, spTubeDiv = 0x0010, spBloodFlow = 0x0020,
                       spALL = 0xFFFF };
}


#ifdef unix
#define DIRSLASH '/'
#else
#define DIRSLASH '\\'
#endif

#endif // CONST_H
