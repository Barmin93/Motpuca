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
#include "anyglobalsettings.h"
#include "anyvisualsettings.h"
#include "anysimulationsettings.h"
#include "anytubularsystemsettings.h"

#ifdef QT_CORE_LIB
#include "columnresizer.h"
#include "ui_dialogGlobals.h"
#endif

#define VALID_BOX(box_x, box_y, box_z) ((box_x) >= 0 && (box_x) < SimulationSettings.no_boxes_x && (box_y) >= 0 && (box_y) < SimulationSettings.no_boxes_y && (box_z) >= 0 && (box_z) < SimulationSettings.no_boxes_z)
#define BOX_ID(box_x, box_y, box_z) (box_x + box_y*SimulationSettings.no_boxes_x + box_z*SimulationSettings.no_boxes_x*SimulationSettings.no_boxes_y)

void ParseSimulationSettingsValue(FILE *f);
void ParseVisualSettingsValue(FILE *f);
void ParseTubularSystemSettingsValue(FILE *f);
void ParseSimulationSettings(FILE *f);
void ParseVisualSettings(FILE *f);
void ParseTubularSystemSettings(FILE *f);



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



void displayGlobalsDialog();
void SaveNeeded(bool needed);

void SaveVisualSettings_ag(FILE *f, anyVisualSettings const *vs);
void SaveSimulationSettings_ag(FILE *f, anySimulationSettings const *ss);
void SaveTubularSystemSettings_ag(FILE *f, anyTubularSystemSettings const *vs);


#endif // CONFIG_H
