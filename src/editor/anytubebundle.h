#ifndef ANYTUBEBUNDLE_H
#define ANYTUBEBUNDLE_H

#include "anyeditable.h"
#include "types.h"
#include <qtextbrowser.h>
#include "model.h"
#include "const.h"
#include "log.h"
#include "config.h"

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

    anyTubeBundle();

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
    virtual void add_itself_to_scene();
    virtual void remove_itself_from_scene();
    virtual char *get_name();

#ifdef QT_CORE_LIB
    virtual void display_properties(QTextBrowser *tb);
#endif
    real billion_to_inf(real x);
    QString real_to_str(real x);

};

#endif // ANYTUBEBUNDLE_H
