#include "anyglobalsettings.h"

anyGlobalSettings::anyGlobalSettings()
{
    debug = false;
    app_dir[0] = 0;
    user_dir[0] = 0;
    input_file[0] = 0;
    output_dir[0] = 0;
    temp_dir[0] = 0;
    save_needed = false;
    simulation_allocated = false;
    run_env = sat::reUnknown;
    debug = false;
    app_thread_id = 0;
}
