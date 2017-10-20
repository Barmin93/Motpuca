#ifndef ANYEDITABLEDIALOG_H
#define ANYEDITABLEDIALOG_H

#include <QDialog>
#include <QColorDialog>
#include <QFileDialog>
#include <QStandardPaths>

#include "color.h"
#include "mainwindow.h"
#include "config.h"
#include "parser.h"

#ifdef QT_CORE_LIB
#include "ui_dialogEditable.h"
#endif

class anyEditable;

#ifdef QT_CORE_LIB
class anyEditableDialog: public QDialog
/**
  Class wraping 'Properties dialog' of all editables.
*/
{
    Q_OBJECT

public:
    Ui_DialogEditable *dialog;
    anyEditable *editable;
    anyColor color;
    bool add_new;                ///< is dialog executed to add new editable to scene or to edit existing editable

    anyEditableDialog();

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
#endif // ANYEDITABLEDIALOG_H
