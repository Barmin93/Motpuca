#ifndef ANYCELLBLOCK_H
#define ANYCELLBLOCK_H

#include "anyeditable.h"
#include "anytissuesettings.h"

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
    float concentrations[sat::dsLast]; ///< initial concentrations

    anyCellBlock();

    virtual ~anyCellBlock() {}

    virtual void read_defaults();

#ifdef QT_CORE_LIB
    virtual void prepare_dialog();
    virtual void update_from_dialog();
    virtual bool validate_properties();
#endif
    virtual bool validate_removal() { return true; }
    virtual void add_itself_to_scene();
    virtual void remove_itself_from_scene();
    virtual bool can_move_up();
    virtual bool can_move_down();
    virtual void move_up();
    virtual void move_down();

    bool should_be_generated_next();

    float billion_to_inf(float x);
    QString float_to_str(float x);

    virtual char *get_name();

#ifdef QT_CORE_LIB
    void virtual display_properties(QTextBrowser *tb);
#endif

};
#endif // ANYCELLBLOCK_H
