#ifndef ANYBARRIER_H
#define ANYBARRIER_H

#include "anyeditable.h"
#include "config.h"

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
    sat::BarrierType type;          ///< type

    anyBarrier();

    virtual ~anyBarrier() {}

#ifdef QT_CORE_LIB
    virtual void display_properties(QTextBrowser *tb);
    virtual void prepare_dialog();
    virtual void update_from_dialog();
    virtual bool validate_properties();
#endif

    virtual char *get_name();

    virtual void update_bounding_box(anyVector &from, anyVector &to, bool &first);

    virtual void rotation_event(anyVector /*d*/, bool /*x_axe*/, bool /*y_axe*/, bool /*z_axe*/)
    {
        // barrier cannot be rotated!
    };
    virtual void read_defaults();

    virtual bool validate_removal() { return true; }
    virtual void add_itself_to_scene();
    virtual void remove_itself_from_scene();
    virtual bool is_point_inside(anyVector const &p, float r);

};


#endif // ANYBARRIER_H
