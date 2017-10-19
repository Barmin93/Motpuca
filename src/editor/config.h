#ifndef CONFIG_H
#define CONFIG_H

#ifdef QT_CORE_LIB
#include <QDialog>
#endif

#include "func.h"
#include "types.h"
#include "vector.h"
#include "color.h"
#include "scene.h"
#include "parser.h"
#include "statistics.h"

#ifdef QT_CORE_LIB
#include "columnresizer.h"
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

    anyGlobalSettings()
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
};

extern anyGlobalSettings GlobalSettings;



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

    anyVisualSettings() {}

    void reset()
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

    void calculate_derived_values()
    {

    }

    void comp_clip_plane()
    {
        clip_plane[0] = -clip.matrix[8];
        clip_plane[1] = -clip.matrix[9];
        clip_plane[2] = -clip.matrix[10];
        clip_plane[3] = clip.matrix[8]*clip.matrix[12] +
                        clip.matrix[9]*clip.matrix[13] +
                        clip.matrix[10]*clip.matrix[14];

    }

    void comp_light_dir()
    {
        light_dir_r = r_matrix.inverted()*light_dir;
        light_dir_r.normalize();
    }
};


class anySimulationSettings
/**
  Global simulation settings.

  Adding new field requires adding of:
    - default value in reset
    - section to SaveSimulationSettings_ag()
    - section to ParseSimulationSettingsValue()
    - section to documentation
*/
{
public:
    int dimensions;            ///< number of dimensions (2 or 3)
    real time_step;            ///< simulation time step [s]
    real time;                 ///< simulation time [s]
    real stop_time;            ///< time to stop simulation [s]
    int step;                  ///< simulation step
    anyVector comp_box_from;   ///< minimal vertex of simulation box
    anyVector comp_box_to;     ///< maximal vertex of simulation box

    real box_size;             ///< box size [um] *should be calculated autmatically!*
    int max_cells_per_box;     ///< maximum number of cells in box *should be calculated autmatically!*
    real force_r_cut;          ///< attraction forces r_cut [um]
    real force_r_peak;         ///< attraction forces peak

    int max_tube_chains;       ///< max number of tube chains
    int max_tube_merge;        ///< max number of tube pairs merged in one simulation step

    unsigned long sim_phases;  ///< enabled simulation phases/processes

    // output...
    int save_statistics;       ///< statistics saving frequency
    int save_povray;           ///< povray saving frequency
    int save_ag;               ///< ag saving frequency

    // graph...
    int graph_sampling;        ///< graph samplimg rate

    // derived values...
    int no_boxes_x; ///< number of boxes in x direction
    int no_boxes_y; ///< number of boxes in y direction
    int no_boxes_z; ///< number of boxes in z direction
    int no_boxes_xy; /// no_boxes_x*no_boxes_y
    int no_boxes;   ///< overall number of boxes
    int max_max_cells_per_box; ///< actual maximum number of cells in box
    int max_max_max_cells_per_box; ///< all-time maximum number of cells in box
    real farest_point;   ///< farest distance from (0, 0, 0)
    real force_r_cut2;   ///< force_r_cut*force_r_cut
    real diffusion_coeff[sat::dsLast];   ///< oxygen, TAF etc diffusion coefficient ( TODO: stability condition)
    real max_o2_concentration;   ///< maximum oxygen in kg/um^3 in cell represented by concentration == 1
    anySimulationSettings() {}

    void reset()
    {
        step = 0;
        max_o2_concentration = 1e-26;

        char fname[P_MAX_PATH];
        snprintf(fname, P_MAX_PATH, "%s%ssimulation_settings.ag", GlobalSettings.app_dir, FOLDER_DEFAULTS);
        Slashify(fname, false);
        LOG2(llInfo, "Reading defaults from ", fname);
        FILE *f = fopen(fname, "r");
        if (!f)
            throw new Error(__FILE__, __LINE__, "Cannot open file for reading", 0, fname);
        else
        {
            ParseSimulationSettings(f);
            fclose(f);
        }

        calculate_derived_values();
    }

    void calculate_derived_values()
    {
        force_r_peak = force_r_cut/5;
        force_r_cut2 = force_r_cut*force_r_cut;
        farest_point = MAX(comp_box_from.length(), comp_box_to.length());
    }
};


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

    anyTubularSystemSettings() { }

    void reset()
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

        calculate_derived_values();
    }

    void calculate_derived_values()
    {

    }
};


#ifdef QT_CORE_LIB
class anyGlobalsDialog: public QDialog
/**
  Class wraping 'Global parametrs dialog'.
*/
{
    Q_OBJECT

public:
    Ui_DialogGlobals *dialog;

    anyGlobalsDialog()
    {
        dialog = new Ui_DialogGlobals;
        dialog->setupUi(this);
        connect(dialog->pushButton_Ok, SIGNAL(clicked()), this, SLOT(slot_ok_clicked()));
        connect(dialog->pushButton_Apply, SIGNAL(clicked()), this, SLOT(slot_apply_clicked()));
        connect(dialog->pushButton_Cancel, SIGNAL(clicked()), this, SLOT(slot_cancel_clicked()));

        connect(dialog->pushButton_colorbackground, SIGNAL(clicked()), this, SLOT(slot_color_clicked()));
        connect(dialog->pushButton_colortubes, SIGNAL(clicked()), this, SLOT(slot_color_clicked()));
        connect(dialog->pushButton_colorbarrier_in, SIGNAL(clicked()), this, SLOT(slot_color_clicked()));
        connect(dialog->pushButton_colorbarrier_out, SIGNAL(clicked()), this, SLOT(slot_color_clicked()));
        connect(dialog->pushButton_colornavigator, SIGNAL(clicked()), this, SLOT(slot_color_clicked()));
        connect(dialog->pushButton_colorclip, SIGNAL(clicked()), this, SLOT(slot_color_clicked()));
        connect(dialog->pushButton_colorcellalive, SIGNAL(clicked()), this, SLOT(slot_color_clicked()));
        connect(dialog->pushButton_colorcellhypoxia, SIGNAL(clicked()), this, SLOT(slot_color_clicked()));
        connect(dialog->pushButton_colorcellnecrosis, SIGNAL(clicked()), this, SLOT(slot_color_clicked()));
        connect(dialog->pushButton_colorcellapoptosis, SIGNAL(clicked()), this, SLOT(slot_color_clicked()));

        dialog->doubleSpinBox_boxsize->setEnabled(!GlobalSettings.simulation_allocated);
        dialog->spinBox_cellsperbox->setEnabled(!GlobalSettings.simulation_allocated);
        dialog->spinBox_max_tube_chains->setEnabled(!GlobalSettings.simulation_allocated);

        dialog->spinBox_graphrate->setEnabled(!Statistics);

        // align columns...
        ColumnResizer* resizer = new ColumnResizer(this);
        resizer->addWidgetsFromLayout(dialog->groupBox_colors_general->layout(), 0);
        resizer->addWidgetsFromLayout(dialog->groupBox_colors_cellstates->layout(), 0);
    }

    void prepare_dialog();
    void update_from_dialog();
    bool validate_settings();
    void set_color_of_button(QPushButton *button, anyColor const &color);
    anyColor get_color_from_button(QPushButton *b) const;

    real billion_to_inf(real x)
    {
        return x == 1000000000 ? MAX_REAL : x;
    }


private slots:
        void slot_ok_clicked();
        bool slot_apply_clicked();
        void slot_cancel_clicked();
        void slot_color_clicked();
};
#endif


extern anyVisualSettings VisualSettings;
extern anySimulationSettings SimulationSettings;
extern anyTubularSystemSettings TubularSystemSettings;

void SaveVisualSettings_ag(FILE *f, anyVisualSettings const *vs);
void SaveSimulationSettings_ag(FILE *f, anySimulationSettings const *ss);
void SaveTubularSystemSettings_ag(FILE *f, anyTubularSystemSettings const *vs);
void ParseSimulationSettingsValue(FILE *f);
void ParseVisualSettingsValue(FILE *f);
void ParseTubularSystemSettingsValue(FILE *f);

void displayGlobalsDialog();
void SaveNeeded(bool needed);

#define VALID_BOX(box_x, box_y, box_z) ((box_x) >= 0 && (box_x) < SimulationSettings.no_boxes_x && (box_y) >= 0 && (box_y) < SimulationSettings.no_boxes_y && (box_z) >= 0 && (box_z) < SimulationSettings.no_boxes_z)
#define BOX_ID(box_x, box_y, box_z) (box_x + box_y*SimulationSettings.no_boxes_x + box_z*SimulationSettings.no_boxes_x*SimulationSettings.no_boxes_y)



#endif // CONFIG_H
