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
#include "anyglobalsdialog.h"

#define VALID_BOX(box_x, box_y, box_z) ((box_x) >= 0 && (box_x) < SimulationSettings.no_boxes_x && (box_y) >= 0 && (box_y) < SimulationSettings.no_boxes_y && (box_z) >= 0 && (box_z) < SimulationSettings.no_boxes_z)
#define BOX_ID(box_x, box_y, box_z) (box_x + box_y*SimulationSettings.no_boxes_x + box_z*SimulationSettings.no_boxes_x*SimulationSettings.no_boxes_y)

void ParseSimulationSettingsValue(FILE *f);
void ParseVisualSettingsValue(FILE *f);
void ParseTubularSystemSettingsValue(FILE *f);
void ParseSimulationSettings(FILE *f);
void ParseVisualSettings(FILE *f);
void ParseTubularSystemSettings(FILE *f);

void displayGlobalsDialog();
void SaveNeeded(bool needed);

void SaveVisualSettings_ag(FILE *f, anyVisualSettings const *vs);
void SaveSimulationSettings_ag(FILE *f, anySimulationSettings const *ss);
void SaveTubularSystemSettings_ag(FILE *f, anyTubularSystemSettings const *vs);


#endif // CONFIG_H
