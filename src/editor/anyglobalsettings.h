#ifndef ANYGLOBALSETTINGS_H
#define ANYGLOBALSETTINGS_H

#include "const.h"

#ifdef QT_CORE_LIB
#include "ui_dialogGlobals.h"
#endif

class anyGlobalSettings
/**
  Global application settings.
*/
{
public:
    bool debug;                ///< debug mode?
    char app_dir[P_MAX_PATH];    ///< application directory (with trailing '/')
    char user_dir[P_MAX_PATH];   ///< user data
    char temp_dir[P_MAX_PATH];   ///< temp dir
    char input_file[P_MAX_PATH]; ///< input file name
    char output_dir[P_MAX_PATH]; ///< output file name
    sat::anyRunEnv run_env;    ///< environment
    bool simulation_allocated; ///< simulation is allocated?
    bool save_needed;          ///< save needed?

#ifdef QT_CORE_LIB
    Qt::HANDLE app_thread_id;  ///< main thread ID
#else
    int app_thread_id;
#endif

    anyGlobalSettings();
};

extern anyGlobalSettings GlobalSettings;

#endif // ANYGLOBALSETTINGS_H
