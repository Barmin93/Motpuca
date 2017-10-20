#include "anyeditabledialog.h"
#include "anyeditable.h"

anyEditableDialog::anyEditableDialog(){
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

#ifdef QT_CORE_LIB
void anyEditableDialog::slot_ok_clicked()
{
    if (slot_apply_clicked())
        accept();
}


bool anyEditableDialog::slot_apply_clicked()
{
    if (!editable->validate_properties())
        return false;

    editable->update_from_dialog();

    if (add_new)
    {
        editable->add_itself_to_scene();

        MainWindowPtr->display_tree_objects(editable);
    }
    scene::UpdateSimulationBox();
    MainWindowPtr->update_selected_object_info();
    MainWindowPtr->slot_gl_repaint();

    SaveNeeded(true);

    return true;
}


void anyEditableDialog::slot_cancel_clicked()
{
    reject();
    if (add_new)
        delete editable;
}


void anyEditableDialog::slot_delete_clicked()
{
    if (!editable->validate_removal())
        return;

    if (QMessageBox::question(0, QObject::tr("Confirmation"),
                             QObject::tr("Do you really want to delete this ") + editable->get_name() + "?",
                             QObject::tr("Yes"),
                             QObject::tr("No")))
        return;

    editable->remove_itself_from_scene();

    MainWindowPtr->display_tree_objects();

    delete editable;

    SaveNeeded(true);

    accept();
}


void anyEditableDialog::slot_color_clicked()
{
    QColorDialog cdialog;

    cdialog.setCurrentColor(QColor(color.r255(), color.g255(), color.b255()));

    if (!cdialog.exec())
        return;

    char style[200];

    color.set(cdialog.currentColor().redF(), cdialog.currentColor().greenF(), cdialog.currentColor().blueF());
    snprintf(style, 200, "background-color: rgb(%d,%d,%d)", cdialog.currentColor().red(), cdialog.currentColor().green(), cdialog.currentColor().blue());
    dialog->pushButton_color->setStyleSheet(style);
}


void anyEditableDialog::slot_add_tissue_clicked()
{
    anyTissueSettings *ts = new anyTissueSettings;
    ts->read_defaults();

    if (ts->display_dialog(true) == Accepted)
    {
        // rebuild tissue combo... (version #1)
        dialog->comboBox_tissue->clear();
        ts = scene::FirstTissueSettings;
        while (ts)
        {
            dialog->comboBox_tissue->addItem(ts->name);
            ts = ts->next;
        }
    }
}


void anyEditableDialog::slot_load_tissue_clicked()
{
    QFileDialog fileopendialog;
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation));
    urls << QUrl::fromLocalFile(QString(GlobalSettings.user_dir) + QString(FOLDER_LIB_TISSUES));
    fileopendialog.setSidebarUrls(urls);
    fileopendialog.setNameFilter(tr("Input files (*.ag);;All files (*)"));
    fileopendialog.setDirectory(QString(GlobalSettings.user_dir) + QString(FOLDER_LIB_TISSUES));

    if (!fileopendialog.exec())
        return;

    try
    {
        FILE *f = fopen(fileopendialog.selectedFiles().at(0).toLatin1(), "r");
        if (!f)
            throw new Error(__FILE__, __LINE__, (QString("Cannot open file: ") + fileopendialog.selectedFiles().at(0)).toLatin1());

        char basefile[P_MAX_PATH];
        snprintf(basefile, P_MAX_PATH, "%sinclude/base.ag", GlobalSettings.app_dir);
        ParseFile(basefile, false);
        scene::ParseTissueSettings(f, (anyTissueSettings *)editable, false);
        fclose(f);
    }
    catch (Error *err)
    {
        LogError(err);
    }

    editable->prepare_dialog();

}


void anyEditableDialog::slot_save_tissue_clicked()
{
    QFileDialog filesavedialog;
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation));
    urls << QUrl::fromLocalFile(QString(GlobalSettings.user_dir) + QString(FOLDER_LIB_TISSUES));
    filesavedialog.setSidebarUrls(urls);
    filesavedialog.setAcceptMode(QFileDialog::AcceptSave);
    filesavedialog.setFileMode(QFileDialog::AnyFile);

    filesavedialog.setNameFilter(tr("Input files (*.ag);;All files (*)"));
    filesavedialog.setDefaultSuffix("ag");
    filesavedialog.setDirectory(QString(GlobalSettings.user_dir) + QString(FOLDER_LIB_TISSUES));

    if (!filesavedialog.exec())
        return;

    try
    {
        FILE *f = fopen(filesavedialog.selectedFiles().at(0).toLatin1(), "w");
        if (!f)
            throw new Error(__FILE__, __LINE__, (QString("Cannot open file: ") + filesavedialog.selectedFiles().at(0)).toLatin1());

        // store tissue properties...
        anyTissueSettings ts = *(anyTissueSettings *)editable;
        editable->update_from_dialog();

        scene::SaveTissueSettings_ag(f, (anyTissueSettings *)editable, false);

        // restore tissue properties...
        *(anyTissueSettings *)editable = ts;

        fclose(f);
    }
    catch (Error *err)
    {
        LogError(err);
    }
}

#endif
