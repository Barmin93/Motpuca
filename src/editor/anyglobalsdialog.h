#ifndef ANYGLOBALSDIALOG_H
#define ANYGLOBALSDIALOG_H

#include "color.h"
#include "const.h"

#ifdef QT_CORE_LIB
#include "columnresizer.h"
#include "ui_dialogGlobals.h"
#endif


#ifdef QT_CORE_LIB
class anyGlobalsDialog: public QDialog
/**
  Class wraping 'Global parametrs dialog'.
*/
{
    Q_OBJECT

public:
    Ui_DialogGlobals *dialog;

    anyGlobalsDialog();
    void prepare_dialog();
    void update_from_dialog();
    bool validate_settings();
    void set_color_of_button(QPushButton *button, anyColor const &color);
    anyColor get_color_from_button(QPushButton *b) const;

    real billion_to_inf(real x);

private slots:
        void slot_ok_clicked();
        bool slot_apply_clicked();
        void slot_cancel_clicked();
        void slot_color_clicked();
};
#endif

#endif // ANYGLOBALSDIALOG_H
