#ifndef ANYTUBELINE_H
#define ANYTUBELINE_H

#include "anyeditable.h"
#include "types.h"
#include "vector.h"
#include "const.h"
#include "config.h"
#include "log.h"
#include <QTextBrowser>

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

    anyTubeLine();

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
    virtual void add_itself_to_scene();
    virtual void remove_itself_from_scene();
    virtual char *get_name();

#ifdef QT_CORE_LIB
    virtual void display_properties(QTextBrowser *tb);
#endif
    real billion_to_inf(real x);
    QString real_to_str(real x);
};
#endif // ANYTUBELINE_H