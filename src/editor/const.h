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
