#ifndef SCENE_H
#define SCENE_H

#ifdef QT_CORE_LIB
#include <QObject>
#include <QTextBrowser>
#endif

#include "model.h"
#include "types.h"
#include "vector.h"
#include "color.h"
#include "transform.h"
#include "func.h"
#include "log.h"

#ifdef QT_CORE_LIB
#include "columnresizer.h"
#include "ui_dialogEditable.h"
#endif


enum CellState { csAdded, csRemoved, csAlive, csHypoxia, csApoptosis, csNecrosis, csLast };  ///< states of cell. csAdded and csRemoved must be at the beginnig
extern char const *CellState_names[];

enum TissueType { ttNormal, ttTumor };
extern char const *TissueType_names[];

enum BarrierType { btIn, btOut };
extern char const *BarrierType_names[];

enum DiffundingSubstances {dsO2, dsTAF, dsPericytes, dsLast};
extern char const *DiffundingSubstances_names[];

class anyTissueSettings;
extern anyTissueSettings *FindTissueSettings(char const *name);


#ifdef QT_CORE_LIB
class anyEditableDialog: public QDialog
/**
  Class wraping 'Properties dialog' of all editables.
*/
{
    Q_OBJECT

public:
    Ui_DialogEditable *dialog;
    class anyEditable *editable;
    anyColor color;
    bool add_new;                ///< is dialog executed to add new editable to scene or to edit existing editable

    anyEditableDialog()
    {
        dialog = new Ui_DialogEditable;
        dialog->setupUi(this);
        connect(dialog->pushButton_Ok, SIGNAL(clicked()), this, SLOT(slot_ok_clicked()));
        connect(dialog->pushButton_Apply, SIGNAL(clicked()), this, SLOT(slot_apply_clicked()));
        connect(dialog->pushButton_Cancel, SIGNAL(clicked()), this, SLOT(slot_cancel_clicked()));
        connect(dialog->pushButton_Delete, SIGNAL(clicked()), this, SLOT(slot_delete_clicked()));
        connect(dialog->pushButton_color, SIGNAL(clicked()), this, SLOT(slot_color_clicked()));
        connect(dialog->pushButton_add_tissue, SIGNAL(clicked()), this, SLOT(slot_add_tissue_clicked()));
        connect(dialog->pushButton_TissueLoad, SIGNAL(clicked()), this, SLOT(slot_load_tissue_clicked()));
        connect(dialog->pushButton_TissueSave, SIGNAL(clicked()), this, SLOT(slot_save_tissue_clicked()));
    }

private slots:
    void slot_ok_clicked();
    void slot_cancel_clicked();
    bool slot_apply_clicked();
    void slot_delete_clicked();
    void slot_color_clicked();
    void slot_add_tissue_clicked();
    void slot_load_tissue_clicked();
    void slot_save_tissue_clicked();
};
#endif


class anyBoundingBox
{
public:
    anyTransform trans;    ///< transformation matrix: rotation + translation
    anyVector from;        ///< start point
    anyVector to;          ///< end point

    anyBoundingBox(): from(0, 0, 0), to(0, 0, 0) { trans.setToIdentity(); }
    void update_bounding_box_by_point(anyVector v, anyVector &u_from, anyVector &u_to, bool &first);
    virtual void update_bounding_box(anyVector &from, anyVector &to, bool &first);
    virtual bool is_point_inside(anyVector const &p, real r)
    {
        anyTransform trans_inv = trans.inverted();
        anyVector p2 = trans_inv*p;
        return p2.x + r >= from.x && p2.x - r <= to.x
            && p2.y + r >= from.y && p2.y - r <= to.y
            && p2.z + r >= from.z && p2.z - r <= to.z;
    }
    void fix()
    {
        if (from.x > to.x)
            from.x = to.x = (from.x + to.x)*0.5;
        if (from.y > to.y)
            from.y = to.y = (from.y + to.y)*0.5;
        if (from.z > to.z)
            from.z = to.z = (from.z + to.z)*0.5;
    }

};


#ifdef QT_CORE_LIB

class anyEditable: public anyBoundingBox
{
public:
    anyEditable *next;

    anyEditableDialog *dialog; ///< Qt dialog used to display/modify properties

    anyEditable(): next(0), dialog(0) {}
    virtual ~anyEditable() {}


    virtual void display_properties(QTextBrowser *tb) = 0;
    virtual bool validate_properties() = 0;
    virtual bool validate_removal() = 0;
    int display_dialog(bool add_new = false)
    {
        if (dialog) delete dialog;
        dialog = new anyEditableDialog;
        dialog->editable = this;
        dialog->add_new = add_new;

        // model tube names...
        dialog->dialog->groupBox_tube->setTitle(MODEL_TUBE_NAME_PL.left(1).toUpper() + MODEL_TUBE_NAME_PL.mid(1));
        dialog->dialog->groupBox_bundle->setTitle(MODEL_TUBEBUNDLE_NAME.left(1).toUpper() + MODEL_TUBEBUNDLE_NAME.mid(1));

        // align columns...
        ColumnResizer* resizer = new ColumnResizer(dialog);
        resizer->addWidgetsFromLayout(dialog->dialog->groupBox_size->layout(), 0);
        resizer->addWidgetsFromLayout(dialog->dialog->groupBox_bundle->layout(), 0);
        resizer->addWidgetsFromLayout(dialog->dialog->groupBox_tissueparams->layout(), 0);
        resizer->addWidgetsFromLayout(dialog->dialog->groupBox_conc->layout(), 0);
        resizer->addWidgetsFromLayout(dialog->dialog->groupBox_tube->layout(), 0);
        resizer->addWidgetsFromLayout(dialog->dialog->groupBox_blood->layout(), 0);

        // set up buttons...
        dialog->dialog->pushButton_Apply->setVisible(!add_new);
        dialog->dialog->pushButton_Delete->setVisible(!add_new);
        if (add_new)
            dialog->dialog->pushButton_Ok->setText(QObject::tr("Add"));
        else
            dialog->dialog->pushButton_Ok->setText(QObject::tr("Ok"));

        // by default hide all group boxes...
        dialog->dialog->groupBox_size->setVisible(false);
        dialog->dialog->groupBox_ends->setVisible(false);
        dialog->dialog->groupBox_tissue->setVisible(false);
        dialog->dialog->groupBox_barriertype->setVisible(false);
        dialog->dialog->groupBox_tube->setVisible(false);
        dialog->dialog->groupBox_bundle->setVisible(false);
        dialog->dialog->groupBox_tissueparams->setVisible(false);
        dialog->dialog->groupBox_msg->setVisible(false);
        dialog->dialog->groupBox_Library->setVisible(false);
        dialog->dialog->groupBox_conc->setVisible(false);
        dialog->dialog->groupBox_blood->setVisible(false);

        prepare_dialog();
        int result = dialog->exec();
        delete resizer;
        return result;
    }

    virtual char *get_name() = 0;
    virtual void read_defaults() = 0;
    virtual void prepare_dialog();
    virtual void update_from_dialog();
    virtual void add_itself_to_scene() = 0;
    virtual void remove_itself_from_scene() {}
    virtual bool can_move_up() { return false; }
    virtual bool can_move_down() { return false; }
    virtual void move_up() { }
    virtual void move_down() { }

    virtual void scale_event(int amount, bool x_axe, bool y_axe, bool z_axe)
    {
        real step = amount < 0 ? -10 : 10;
        if (x_axe)
        {
            from.x -= step;
            to.x += step;
        }
        if (y_axe)
        {
            from.y -= step;
            to.y += step;
        }
        if (z_axe)
        {
            from.z -= step;
            to.z += step;
        }
        fix();
    }

    virtual void trans_event(anyVector d, bool x_axe, bool y_axe, bool z_axe)
    {
        if (!x_axe) d.x = 0;
        if (!y_axe) d.y = 0;
        if (!z_axe) d.z = 0;

        from += d;
        to += d;
    }

    virtual void rotation_event(anyVector d, bool x_axe, bool y_axe, bool z_axe)
    {
        if (x_axe)
            trans.rotateX(-d.x);

        if (y_axe)
            trans.rotateY(d.y);

        if (z_axe)
            trans.rotateZ(d.z);
    }

    real billion_to_inf(real x)
    {
        return x == 1000000000 ? MAX_REAL : x;
    }

    QString real_to_str(real x)
    {
        if (x == MAX_REAL)
            return QString("inf");
        else
            return QString::number(x);
    }
};

#else

class anyEditable: public anyBoundingBox
{
public:
    anyEditable *next;
    anyEditable(): next(0) {}

    virtual void move_up() { }
    virtual void move_down() { }
};

#endif



class anyBarrier: public anyEditable
/**
  Structure defining barrier properties.

  Adding new field requires adding of:
    - default value in constructor
    - section to SaveBarrier_ag()
    - section to ParseBarrierValue()
    - new widget in properties dialog and
        - section in prepare_dialog()
        - section in update_from_dialog()
    - section to documentation
*/
{
public:
    BarrierType type;          ///< type

    anyBarrier(): type(btIn)
    {}

    virtual ~anyBarrier() {}

#ifdef QT_CORE_LIB
    virtual void display_properties(QTextBrowser *tb)
    {
        tb->clear();
        tb->append(QObject::tr("BARRIER"));
        tb->append("");
        if (type == btIn)
            tb->append(QObject::tr("type: ") + "<b>" + QObject::tr("in") + "</b>");
        else
            tb->append(QObject::tr("type: ") + "<b>" + QObject::tr("out") + "</b>");
        tb->append(QObject::tr("width: ") + "<b>" + real_to_str(to.x - from.x) + "</b>");
        tb->append(QObject::tr("height: ") + "<b>" + real_to_str(to.y - from.y) + "</b>");
        tb->append(QObject::tr("depth: ") + "<b>" + real_to_str(to.z - from.z) + "</b>");
    }
#endif

    virtual char *get_name()
    {
        static char buff[50];
        if (type == btIn)
            snprintf(buff, 50, "barrier (keeps cells inside)");
        else
            snprintf(buff, 50, "barrier (keeps cells outside)");
        return buff;
    }

    virtual void update_bounding_box(anyVector &from, anyVector &to, bool &first)
    {
        // barriers influence on simulation bounding box only if (type == btIn)

        if (type == btIn)
            anyEditable::update_bounding_box(from, to, first);
    }

    virtual void rotation_event(anyVector /*d*/, bool /*x_axe*/, bool /*y_axe*/, bool /*z_axe*/)
    {
        // barrier cannot be rotated!
    };
    virtual void read_defaults();

#ifdef QT_CORE_LIB
    virtual void prepare_dialog();
    virtual void update_from_dialog();
    virtual bool validate_properties();
#endif

    virtual bool validate_removal() { return true; }
    virtual void add_itself_to_scene()
    {
        void AddBarrier(anyBarrier *b);

        AddBarrier(this);
    }
    virtual void remove_itself_from_scene()
    {
        void RemoveBarrier(anyBarrier *b);

        LOG(llDebug, "Removing barrier");
        RemoveBarrier(this);
    }

    virtual bool is_point_inside(anyVector const &p, real r)
    {
        if (type == btIn)
            return anyEditable::is_point_inside(p, r);
        else
            return !anyEditable::is_point_inside(p, -r);
    }

};


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
    TissueType type;           ///< type
    char *name;                ///< unique name of tissue settings
    real cell_r;               ///< radius of mature cell [um]
    real density;              ///< tissue density [kg/m^3]
    real cell_grow_speed;      ///< speed of cell growing [um/s]
    real minimum_interphase_time; ///< miniumum interphase time [s]
    real time_to_apoptosis;    ///< maximal age of cell [s]
    real time_to_necrosis;     ///< time in apoptosis or hypoxia before switching to necrosis [s]
    real time_to_necrosis_var; ///< variance of time_to_necrosis [s]
    real time_in_necrosis;     ///< time in necrosis state [s]
    real dead_r;               ///< size of dead cell [um]
    real cell_shrink_speed;    ///< speed of shrinking of dead cell [um/s]
    real minimum_mitosis_r;    ///< minimum radius for mitosis [um]

    real force_rep_factor;     ///< repulsion force factor
    real force_atr1_factor;    ///< first attraction force factor
    real force_atr2_factor;    ///< second attraction force factor
    real force_dpd_factor;     ///< PDP gamma factor
    real dpd_temperature;      ///< T in DPD random force
    real max_pressure;         ///< maximum pressure allowing for growth and proliferation
    real o2_consumption;       ///< oxygen consumption ratio in o2 kg/ cell kg /s
    real pericyte_production;  ///< pericyte production
    real o2_hypoxia;           ///< o2 concentration that leads to hypoxia
    anyTissueSettings *next;   ///< pointer to next tissue

    // aux fields...
    int id;                    ///< id of tissue (0, 1,...)
    int no_cells[csLast];      ///< number of cells (0 - all; 2...5 - for every state)
    real pressure_sum;         ///< sum of cell pressures
    real pressure;             ///< average pressure

    anyTissueSettings(): color(0, 0, 0), type(ttNormal), name(0), cell_r(10), density(1),
                         cell_grow_speed(0), minimum_interphase_time(0), time_to_apoptosis(MAX_REAL), time_to_necrosis(0),
                         time_to_necrosis_var(0),
                         time_in_necrosis(0), dead_r(1),
                         cell_shrink_speed(0), minimum_mitosis_r(10),
                         force_rep_factor(0), force_atr1_factor(0), force_atr2_factor(0), force_dpd_factor(0), dpd_temperature(0),
                         max_pressure(0), o2_consumption(7.5e-9), pericyte_production(0), o2_hypoxia(0),
                         next(0), id(0), pressure_sum(0), pressure(0)
    {
        for (int i = 0; i < csLast; i++)
            no_cells[i] = 0;
    }

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

    virtual void add_itself_to_scene()
    {
        void AddTissueSettings(anyTissueSettings *);

        AddTissueSettings(this);
    }
    virtual void remove_itself_from_scene()
    {
        void RemoveTissueSettings(anyTissueSettings *ts);

        LOG(llDebug, "Removing tissue");
        RemoveTissueSettings(this);
    }

    virtual char *get_name()
    {
        static char buff[1000];
        if (!name)
            snprintf(buff, 1000, "tissue");
        else
            snprintf(buff, 1000, "tissue '%s'", name);
        return buff;
    }

#ifdef QT_CORE_LIB
    virtual void display_properties(QTextBrowser *tb)
    {
        tb->clear();
        tb->append(QObject::tr("TISSUE"));
        tb->append("");
        tb->append(QObject::tr("name: ") + " <b>" + name + "</b>");
        switch (type)
        {
        case ttNormal:
            tb->append(QObject::tr("type: ") + " <b>" + QObject::tr("NORMAL") + "</b>");
            break;
        case ttTumor:
            tb->append(QObject::tr("type: ") + " <b>" + QObject::tr("TUMOR") + "</b>");
            break;
        }
        tb->append(QObject::tr("cell radius: ") + " <b>" + real_to_str(cell_r) + "</b>");
        tb->append(QObject::tr("dead cell radius: ") + " <b>" + real_to_str(dead_r) + "</b>");
        tb->append(QObject::tr("density: ") + " <b>" + real_to_str(density) + "</b>");
        tb->append(QObject::tr("growth speed: ") + " <b>" + real_to_str(cell_grow_speed) + "</b>");
        tb->append(QObject::tr("shinking speed: ") + " <b>" + real_to_str(cell_shrink_speed) + "</b>");
        tb->append(QObject::tr("minimum interphase time: ") + " <b>" + real_to_str(minimum_interphase_time) + "</b>");
        tb->append(QObject::tr("time to apoptosis: ") + " <b>" + real_to_str(time_to_apoptosis) + "</b>");
        tb->append(QObject::tr("time to necrosis: ") + " <b>" + real_to_str(time_to_necrosis) + "</b>");
        tb->append(QObject::tr("time in necrosis: ") + " <b>" + real_to_str(time_in_necrosis) + "</b>");
        tb->append(QObject::tr("minimum mitosis radius: ") + " <b>" + real_to_str(minimum_mitosis_r) + "</b>");
        tb->append(QObject::tr("repulsion factor: ") + " <b>" + real_to_str(force_rep_factor) + "</b>");
        tb->append(QObject::tr("attraction factor #1: ") + " <b>" + real_to_str(force_atr1_factor) + "</b>");
        tb->append(QObject::tr("attraction factor #2: ") + " <b>" + real_to_str(force_atr2_factor) + "</b>");
        tb->append(QObject::tr("maximum pressure: ") + " <b>" + real_to_str(max_pressure) + "</b>");
        tb->append(QObject::tr("O2 consumption: ") + " <b>" + real_to_str(o2_consumption) + "</b>");
        tb->append(QObject::tr("Pericytes production: ") + " <b>" + real_to_str(pericyte_production) + "</b>");
    }
#endif
};


class anyCellBlock: public anyEditable
/**
  Structure defining block of cells.

  Adding new field requires adding of:
    - default value in constructor
    - section to SaveBlock_ag()
    - section to ParseBlockValue()
    - new widget in properties dialog and
        - section in prepare_dialog()
        - section in update_from_dialog()
    - section to documentation
*/
{
public:
    anyTissueSettings *tissue; ///< tissue
    bool generated;            ///< are cells generated?
    real concentrations[dsLast]; ///< initial concentrations

    anyCellBlock(): tissue(0), generated(false)
    {
        for (int i = 0; i < dsLast; i++)
        concentrations[i] = 0;
    }

    virtual ~anyCellBlock() {}

    virtual void read_defaults();

#ifdef QT_CORE_LIB
    virtual void prepare_dialog();
    virtual void update_from_dialog();
    virtual bool validate_properties();
#endif
    virtual bool validate_removal() { return true; }
    virtual void add_itself_to_scene()
    {
        void AddCellBlock(anyCellBlock *);

        AddCellBlock(this);
    }
    virtual void remove_itself_from_scene()
    {
        void RemoveCellBlock(anyCellBlock *cb);

        LOG(llDebug, "Removing cell block");
        RemoveCellBlock(this);
    }
    virtual bool can_move_up();
    virtual bool can_move_down();
    virtual void move_up();
    virtual void move_down();

    bool should_be_generated_next();

    virtual char *get_name()
    {
        static char buff[1000];
        if (tissue)
            snprintf(buff, 1000, "cell block (%s)", tissue->name);
        else
            snprintf(buff, 1000, "cell block");
        return buff;
    }

#ifdef QT_CORE_LIB
    void virtual display_properties(QTextBrowser *tb)
    {
        tb->clear();
        tb->append(QObject::tr("CELL BLOCK"));
        tb->append("");
        tb->append(QObject::tr("tissue name: ") + " <b>" + tissue->name + "</b>");
        tb->append(QObject::tr("width: ") + "<b>" + real_to_str(to.x - from.x) + "</b>");
        tb->append(QObject::tr("height: ") + "<b>" + real_to_str(to.y - from.y) + "</b>");
        tb->append(QObject::tr("depth: ") + "<b>" + real_to_str(to.z - from.z) + "</b>");
        if (!generated)
        {
            tb->append(QObject::tr("init O2 conc: ") + "<b>" + real_to_str(concentrations[dsO2]) + "</b>");
            tb->append(QObject::tr("init TAF conc: ") + "<b>" + real_to_str(concentrations[dsTAF]) + "</b>");
            tb->append(QObject::tr("init Pericytes conc: ") + "<b>" + real_to_str(concentrations[dsPericytes]) + "</b>");
        }
    }
#endif
};


class anyCell
/**
  Structure defining cell properties.
*/
{
public:
    anyTissueSettings *tissue; ///< tissue
    anyVector pos;             ///< position
    anyVector pos_h1, pos_h2;  ///< historical positions (for displacement drawing)
                               // every N steps: pos_h2 := pos_h1; pos_h1 := pos;
    real r;                    ///< current radius
    CellState state;           ///< state
    real age;                  ///< age
    real state_age;            ///< age in current state

    int no_cells_in_box;       ///< number of cells in box (valid only for first cell in every box)

    // aux fields...
    real one_by_mass;          ///< 1/mass
    anyVector velocity;        ///< velocity
    anyVector force;           ///< force
    real pressure;             ///< pressure - real value of pressure in cell, may not be used in visualization or any calculations inside CellCellForces
    real pressure_prev;        ///< pressure in previous step (for calculating pressure_avg in nei. cells)
    real pressure_avg;         ///< average pressure - may not be used for visualization  or any calculations inside CellCellForces
    real pressure_sum;         ///< pressure sum for averaging
    int  nei_cnt[2];           ///< number of neighbouring cells (0 - normal, 1 - tumor)
    real time_to_necrosis;     ///< individual time to necrosis (in hypoxia or apoptosis)
    real concentrations[dsLast][2];

    bool mark;                 ///< marker (for debugging)

    anyCell(): tissue(0), pos(0, 0, 0), pos_h1(-1000000000, 0, 0), pos_h2(-1000000000, 0, 0), r(0), state(csAlive), age(0), state_age(0), no_cells_in_box(0),
        one_by_mass(0), velocity(0, 0, 0), force(0, 0, 0), pressure(0), pressure_prev(0), pressure_avg(0), pressure_sum(0), time_to_necrosis(0),
               mark(false)
    {
        for (int i =0; i < dsLast; i++)
            concentrations[i][0] = concentrations[i][1] = 0;
        nei_cnt[ttNormal] = nei_cnt[ttTumor] = 0;
    }
};


class anyTubeBundle: public anyEditable
/**
  Structure defining bundle of pipes.

  Adding new field requires adding of:
    - default value in constructor
    - section to SaveBundle_ag()
    - section to ParseBundleValue()
    - new widget in properties dialog and
        - section in prepare_dialog()
        - section in update_from_dialog()
    - section to documentation
*/
{
public:
    real extent_x;          ///< extension
    real spacing_y;         ///< spacing in Y direction
    real spacing_z;         ///< spacing in Z direction
    real shift_y;           ///< shift in Y direction
    real shift_z;           ///< shift in Z direction
    real tube_length;       ///< length of single tube
    real r;                 ///< tube radius

    real min_blood_pressure;  ///< minimum blood pressure
    real max_blood_pressure;  ///< maximum blood pressure
    bool fixed_blood_pressure; ///< is blood pressure fixed on ends?

    bool generated;         ///< are tube lines generated?

    anyTubeBundle(): extent_x(0), spacing_y(0), spacing_z(0),
    shift_y(0), shift_z(0), r(0), min_blood_pressure(-100), max_blood_pressure(100), fixed_blood_pressure(true), generated(false)
    {}

    virtual ~anyTubeBundle() {}

    virtual void update_bounding_box(anyVector &/*from*/, anyVector &/*to*/, bool &/*first*/)
    {
        // tube bundles do not influence on simulation bounding box
    }
    virtual void read_defaults();

#ifdef QT_CORE_LIB
    virtual void prepare_dialog();
    virtual void update_from_dialog();
    virtual bool validate_properties();
#endif

    virtual bool validate_removal() { return true; }
    virtual void add_itself_to_scene()
    {
        void AddTubeBundle(anyTubeBundle *);

        AddTubeBundle(this);
    }
    virtual void remove_itself_from_scene()
    {
        void RemoveTubeBundle(anyTubeBundle *vb);

        LOG(llDebug, "Removing tube bundle");
        RemoveTubeBundle(this);
    }

    virtual char *get_name()
    {
        static char buff[100];
        #ifdef QT_CORE_LIB
          snprintf(buff, 30, "%s", MODEL_TUBEBUNDLE_NAME.toLatin1().constData());
        #else
          snprintf(buff, 30, "%s", MODEL_TUBEBUNDLE_NAME_PCHAR);
        #endif
        return buff;
    }

#ifdef QT_CORE_LIB
    virtual void display_properties(QTextBrowser *tb)
    {
        tb->clear();
        tb->append(MODEL_TUBEBUNDLE_NAME.toUpper());
        tb->append("");
        tb->append(QObject::tr("width: ") + "<b>" + real_to_str(to.x - from.x) + " + 2*" + real_to_str(extent_x) + "</b>");
        tb->append(QObject::tr("height: ") + "<b>" + real_to_str(to.y - from.y) + "</b>");
        tb->append(QObject::tr("depth: ") + "<b>" + real_to_str(to.z - from.z) + "</b>");
        tb->append(QObject::tr("tube radius: ") + "<b>" + real_to_str(r) + "</b>");
        tb->append(QObject::tr("tube length: ") + "<b>" + real_to_str(tube_length) + "</b>");
        tb->append(QObject::tr("extension: ") + "<b>" + real_to_str(extent_x) + "</b>");
        tb->append(QObject::tr("y spacing: ") + "<b>" + real_to_str(spacing_y) + "</b>");
        tb->append(QObject::tr("z spacing: ") + "<b>" + real_to_str(spacing_z) + "</b>");
        tb->append(QObject::tr("y shift: ") + "<b>" + real_to_str(shift_y) + "</b>");
        tb->append(QObject::tr("z shift: ") + "<b>" + real_to_str(shift_z) + "</b>");
        if (fixed_blood_pressure)
        {
            tb->append(QObject::tr("fixed blood pressure:"));
            tb->append(QObject::tr("- min pressure: ") + "<b>" + real_to_str(min_blood_pressure) + "</b>");
            tb->append(QObject::tr("- max pressure: ") + "<b>" + real_to_str(max_blood_pressure) + "</b>");
        }
    }
#endif
};


class anyTubeLine: public anyEditable
/**
  Structure defining line of tubes.

  Adding new field requires adding of:
    - default value in constructor
    - section to SaveTubeLine_ag()
    - section to ParseTubeLineValue()
    - new widget in properties dialog and
        - section in prepare_dialog()
        - section in update_from_dialog()
        - section in display_properties()
    - section to documentation
*/
{
public:
    real tube_length;         ///< tube length
    real r;                   ///< radius

    real min_blood_pressure;  ///< minimum blood pressure
    real max_blood_pressure;  ///< maximum blood pressure
    bool fixed_blood_pressure;///< is blood pressure fixed on ends?

    bool generated;           ///< are tubes generated?

    anyTubeLine(): tube_length(50), r(0), min_blood_pressure(-100), max_blood_pressure(100), fixed_blood_pressure(true), generated(false)
    {}

    virtual ~anyTubeLine() {}

    virtual void update_bounding_box(anyVector &/*from*/, anyVector &/*to*/, bool &/*first*/)
    {
        // tube lines do not influence on simulation bounding box
    }
    virtual void read_defaults();

#ifdef QT_CORE_LIB
    virtual void prepare_dialog();
    virtual void update_from_dialog();
    virtual bool validate_properties();
#endif

    virtual bool validate_removal() { return true; }
    virtual void add_itself_to_scene()
    {
        void AddTubeLine(anyTubeLine *);

        AddTubeLine(this);
    }
    virtual void remove_itself_from_scene()
    {
        void RemoveTubeLine(anyTubeLine *vl);

        LOG(llDebug, "Removing tube line");
        RemoveTubeLine(this);
    }


    virtual char *get_name()
    {
        static char buff[100];
        #ifdef QT_CORE_LIB
          snprintf(buff, 30, "%s", MODEL_TUBELINE_NAME.toLatin1().constData());
        #else
          snprintf(buff, 30, "%s", MODEL_TUBELINE_NAME_PCHAR);
        #endif
        return buff;
    }

#ifdef QT_CORE_LIB
    virtual void display_properties(QTextBrowser *tb)
    {
        tb->clear();
        tb->append(MODEL_TUBELINE_NAME.toUpper());
        tb->append("");
        tb->append(QObject::tr("tube radius: ") + "<b>" + real_to_str(r) + "</b>");
        tb->append(QObject::tr("tube length: ") + "<b>" + real_to_str(tube_length) + "</b>");
        if (fixed_blood_pressure)
        {
            tb->append(QObject::tr("fixed blood pressure:"));
            tb->append(QObject::tr("- min pressure: ") + "<b>" + real_to_str(min_blood_pressure) + "</b>");
            tb->append(QObject::tr("- max pressure: ") + "<b>" + real_to_str(max_blood_pressure) + "</b>");
        }
    }
#endif
};


class anyTube
/**
  Structure defining tube.
*/
{
public:
    anyVector pos1;    ///< tip #1 of tube
    anyVector pos2;    ///< tip #2 of tube
    real length;       ///< length of tube
    real final_length; ///< final length of tube
    real r;            ///< radius
    real final_r;      ///< final radius
    CellState state;   ///< state of tube
    real age;          ///< age
    real state_age;    ///< age in current state
    real flow_time;    ///< time of last blood flow

    anyVector velocity1; /// velocity in tip #1
    anyVector velocity2; /// velocity in tip #2
    anyVector force1;  ///< force in tip #1
    anyVector force2;  ///< force in tip #2

    anyTube *next;   ///< next tube in chain
    anyTube *prev;   ///< previous tube in chain
    anyTube *fork;   ///< link to first tube in forking chain
    anyTube *base;   ///< link to base tube of first forked tube
    anyTube *top;    ///< link to base tube of last forked tube
    anyTube *jab;    ///< link to attached tube

    real concentrations[dsLast][2];

    bool fixed_blood_pressure; ///< is blood pressure fixed?
    real blood_pressure; ///< blood pressure

    // aux values...
    bool taf_triggered; ///< enough TAF for sprouting?
    real blood_flow;    ///< blood flows

    int id;            ///< tube id
    int parsed_id;     ///< id read from input file
    int base_id;       ///< id of read base tube
    int top_id;        ///< id of read top tube
    int nx, ny, nz;    ///< minimum bounding box
    real one_by_mass;  ///< 1/mass
    real pressure;             ///< pressure - real value of pressure in cell, may not be used in visualization or any calculations inside CellCellForces
    real pressure_prev;        ///< pressure in previous step (for calculating pressure_avg in nei. cells)
    real pressure_avg;         ///< average pressure - may not be used for visualization  or any calculations inside CellCellForces
    real pressure_sum;         ///< pressure sum for averaging
    int  nei_cnt;              ///< number of neighbouring cells

    anyTube(): pos1(0, 0, 0), pos2(0, 0, 0), length(0), final_length(0), r(0), final_r(0), state(csAlive), age(0), state_age(0), flow_time(0),
                 velocity1(0, 0, 0), velocity2(0, 0, 0), force1(0, 0, 0), force2(0, 0, 0),
                 next(0), prev(0), fork(0), base(0), top(0), jab(0),
                 fixed_blood_pressure(false), blood_pressure(0),
                 taf_triggered(false), blood_flow(0), id(0), parsed_id(0), base_id(0), top_id(0), one_by_mass(0),
                 pressure(0), pressure_prev(0), pressure_avg(0), pressure_sum(0), nei_cnt(0)
    {
        for (int i = 0; i < dsLast; i++)
            concentrations[i][0] = concentrations[i][1] = 0;
    }

    anyVector smooth_pos1() const
    {
        anyVector p = pos1;
        if (prev)
            p = (p + prev->pos2)*0.5;
        else if (base)
            p = (p + base->smooth_pos1() + base->smooth_pos2())*0.333333333333;

        return p;
    }

    anyVector smooth_pos2() const
    {
        anyVector p = pos2;
        if (next)
            p = (p + next->pos1)*0.5;
        else if (top)
            p = (p + top->smooth_pos1() + top->smooth_pos2())*0.333333333333;

        return p;
    }

};


class anyTubeBox
{
public:
    int no_tubes;
    anyTube **tubes;

    anyTubeBox(): no_tubes(0), tubes(0) {}
};


class anyTubeMerge
{
public:
    anyTube *t1, *t2;
};


extern anyBarrier *FirstBarrier;
extern anyBarrier *LastBarrier;
extern anyCellBlock *FirstCellBlock;
extern anyCellBlock *LastCellBlock;
extern anyTubeLine *FirstTubeLine;
extern anyTubeLine *LastTubeLine;
extern anyTubeBundle *FirstTubeBundle;
extern anyTubeBundle *LastTubeBundle;
extern anyTissueSettings *FirstTissueSettings;
extern anyTissueSettings *LastTissueSettings;
extern int NoTissueSettings;
extern anyCell *Cells;
extern anyTube **TubeChains;
extern real ***Concentrations;
extern int NoTubeChains;
extern int NoTubes;
extern int LastTubeId;
extern anyTubeBox *BoxedTubes;


void ParseSimulationSettings(FILE *f);
void ParseVisualSettings(FILE *f);
void ParseTubularSystemSettings(FILE *f);

void AddTissueSettings(anyTissueSettings *ts);
void RemoveTissueSettings(anyTissueSettings *ts);
void ParseTissueSettings(FILE *f, anyTissueSettings *ts, bool add_to_scene);
void SaveTissueSettings_ag(FILE *f, anyTissueSettings const *ts, bool save_header);
void SaveAllTissueSettings_ag(FILE *f);
void DeallocateTissueSettings();
anyTissueSettings *FindTissueSettingById(int id);

void AddBarrier(anyBarrier *b);
void RemoveBarrier(anyBarrier *b);
void ParseBarrier(FILE *f, anyBarrier *b, bool add_to_scene);
void ParseBarrierValue(FILE *f, anyBarrier *b);
void SaveBarrier_ag(FILE *f, anyBarrier const *b);
void SaveAllBarriers_ag(FILE *f);
void DeallocateBarriers();

void AddCellBlock(anyCellBlock *b);
void RemoveCellBlock(anyCellBlock *cb);
void ParseCellBlockValue(FILE *f, anyCellBlock *b);
void ParseCellBlock(FILE *f, anyCellBlock *b, bool add_to_scene);
void SaveCellBlock_ag(FILE *f, anyCellBlock const *b);
void SaveAllCellBlocks_ag(FILE *f);
void DeallocateCellBlocks();
void GenerateCellsInBlock(anyCellBlock *b);
void GenerateCellsInAllBlocks();

int GetBoxId(anyVector const pos);
void SetCellMass(anyCell *c);
void AddCell(anyCell *c);
void ParseCellValue(FILE *f, anyCell *c);
void ParseCell(FILE *f);
void SaveCell_ag(FILE *f, anyCell *c);
void SaveAllCells_ag(FILE *f);

void AddTubeLine(anyTubeLine *vl);
void RemoveTubeLine(anyTubeLine *vl);
void ParseTubeLineValue(FILE *f, anyTubeLine *vl);
void ParseTubeLine(FILE *f, anyTubeLine *vl, bool add_to_scene);
void SaveTubeLine_ag(FILE *f, anyTubeLine const *vl);
void SaveAllTubeLines_ag(FILE *f);
void DeallocateTubeLines();
void GenerateTubesInTubeLine(anyTubeLine *vl);
void GenerateTubesInAllTubeLines();

void AddTubeBundle(anyTubeBundle *vb);
void RemoveTubeBundle(anyTubeBundle *vb);
void ParseTubeBundle(FILE *f, anyTubeBundle *vb, bool add_to_scene);
void SaveTubeBundle_ag(FILE *f, anyTubeBundle const *vb);
void SaveAllTubeBundles_ag(FILE *f);
void DeallocateTubeBundles();
void GenerateTubesInAllTubeBundles();
void GenerateTubesInTubeBundle(anyTubeBundle *vb);

void AddTube(anyTube *v, bool attach_to_previous, bool start_new_chain);
void SetTubeMass(anyTube *v);
void ParseTubeValue(FILE *f, anyTube *v);
void ParseTube(FILE *f);
void SaveTube_ag(FILE *f, anyTube *v);
void SaveAllTubes_ag(FILE *f);
void SmoothTubeTips(anyTube const *v, anyVector &pos1, anyVector &pos2);
bool TubesJoined(anyTube *v1, anyTube *v2);
anyTube *FindTubeById(int id, bool current);
void RelinkTubes();
anyTube *FindFirstTube(anyTube *v);
anyTube *FindLastTube(anyTube *v);
void AddTubesToMerge(anyTube *v1, anyTube *v2);
void MergeTubes();

void SavePovRay(char const *povfname, bool save_ani);
void SaveVTK();
void SaveAG(char const *fname, bool save_cells_and_tubes);

void AllocSimulation();
void DeallocSimulation();
void ReallocSimulation();

void UpdateSimulationBox();

#endif // SCENE_H
