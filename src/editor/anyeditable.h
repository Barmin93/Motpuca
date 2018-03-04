#ifndef ANYEDITABLE_H
#define ANYEDITABLE_H

#if QT_CORE_LIB
#include <QString>
#include <QTextBrowser>
#endif

#include "anyboundingbox.h"

class anyEditableDialog;

#ifdef QT_CORE_LIB

class anyEditable: public anyBoundingBox
{
public:
    anyEditable *next;

    anyEditableDialog *dialog; ///< Qt dialog used to display/modify properties

    anyEditable();
    virtual ~anyEditable() {}


    virtual void display_properties(QTextBrowser *tb) = 0;
    virtual bool validate_properties() = 0;
    virtual bool validate_removal() = 0;

    int display_dialog(bool add_new = false);

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

    virtual void scale_event(int amount, bool x_axe, bool y_axe, bool z_axe);
    virtual void trans_event(anyVector d, bool x_axe, bool y_axe, bool z_axe);
    virtual void rotation_event(anyVector d, bool x_axe, bool y_axe, bool z_axe);

    float billion_to_inf(float x);
    QString float_to_str(float x);
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

#endif // ANYEDITABLE_H
