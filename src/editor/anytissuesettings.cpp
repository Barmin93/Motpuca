#include "anytissuesettings.h"

anyTissueSettings::anyTissueSettings(): color(0, 0, 0), type(sat::ttNormal), name(0), cell_r(10), density(1),
    cell_grow_speed(0), minimum_interphase_time(0), time_to_apoptosis(MAX_float), time_to_necrosis(0),
    time_to_necrosis_var(0),
    time_in_necrosis(0), dead_r(1),
    cell_shrink_speed(0), minimum_mitosis_r(10.0f),
    force_rep_factor(0), force_atr1_factor(0), force_atr2_factor(0), force_dpd_factor(0), dpd_temperature(0),
    max_pressure(0), o2_consumption(7.5e-9f), pericyte_production(0), o2_hypoxia(0),
    next(0), id(0), pressure_sum(0), pressure(0)
{
for (int i = 0; i < sat::csLast; i++)
	no_cells[i] = 0;
}

float anyTissueSettings::billion_to_inf(float x)
{
    return x == 1000000000 ? MAX_float : x;
}


QString anyTissueSettings::float_to_str(float x)
{
    if (x == MAX_float)
        return QString("inf");
    else
        return QString::number(x);
}


void anyTissueSettings::add_itself_to_scene()
{
    scene::AddTissueSettings(this);
}
void anyTissueSettings::remove_itself_from_scene()
{

    LOG(llDebug, "Removing tissue");
    scene::RemoveTissueSettings(this);
}
char* anyTissueSettings::get_name()
{
    static char buff[1000];
    if (!name)
        snprintf(buff, 1000, "tissue");
    else
        snprintf(buff, 1000, "tissue '%s'", name);
    return buff;
}
void anyTissueSettings::display_properties(QTextBrowser *tb)
{
    tb->clear();
    tb->append(QObject::tr("TISSUE"));
    tb->append("");
    tb->append(QObject::tr("name: ") + " <b>" + name + "</b>");
    switch (type)
    {
    case sat::ttNormal:
        tb->append(QObject::tr("type: ") + " <b>" + QObject::tr("NORMAL") + "</b>");
        break;
    case sat::ttTumor:
        tb->append(QObject::tr("type: ") + " <b>" + QObject::tr("TUMOR") + "</b>");
        break;
    }
    tb->append(QObject::tr("cell radius: ") + " <b>" + anyTissueSettings::float_to_str(cell_r) + "</b>");
    tb->append(QObject::tr("dead cell radius: ") + " <b>" + anyTissueSettings::float_to_str(dead_r) + "</b>");
    tb->append(QObject::tr("density: ") + " <b>" + anyTissueSettings::float_to_str(density) + "</b>");
    tb->append(QObject::tr("growth speed: ") + " <b>" + anyTissueSettings::float_to_str(cell_grow_speed) + "</b>");
    tb->append(QObject::tr("shinking speed: ") + " <b>" + anyTissueSettings::float_to_str(cell_shrink_speed) + "</b>");
    tb->append(QObject::tr("minimum interphase time: ") + " <b>" + anyTissueSettings::float_to_str(minimum_interphase_time) + "</b>");
    tb->append(QObject::tr("time to apoptosis: ") + " <b>" + anyTissueSettings::float_to_str(time_to_apoptosis) + "</b>");
    tb->append(QObject::tr("time to necrosis: ") + " <b>" + anyTissueSettings::float_to_str(time_to_necrosis) + "</b>");
    tb->append(QObject::tr("time in necrosis: ") + " <b>" + anyTissueSettings::float_to_str(time_in_necrosis) + "</b>");
    tb->append(QObject::tr("minimum mitosis radius: ") + " <b>" + anyTissueSettings::float_to_str(minimum_mitosis_r) + "</b>");
    tb->append(QObject::tr("repulsion factor: ") + " <b>" + anyTissueSettings::float_to_str(force_rep_factor) + "</b>");
    tb->append(QObject::tr("attraction factor #1: ") + " <b>" + anyTissueSettings::float_to_str(force_atr1_factor) + "</b>");
    tb->append(QObject::tr("attraction factor #2: ") + " <b>" + anyTissueSettings::float_to_str(force_atr2_factor) + "</b>");
    tb->append(QObject::tr("maximum pressure: ") + " <b>" + anyTissueSettings::float_to_str(max_pressure) + "</b>");
    tb->append(QObject::tr("O2 consumption: ") + " <b>" + anyTissueSettings::float_to_str(o2_consumption) + "</b>");
    tb->append(QObject::tr("Pericytes production: ") + " <b>" + anyTissueSettings::float_to_str(pericyte_production) + "</b>");
}


void anyTissueSettings::read_defaults()
{
    char fname[P_MAX_PATH];

    snprintf(fname, P_MAX_PATH, "%s%stissue.ag", GlobalSettings.app_dir, FOLDER_DEFAULTS);

    try
    {
        LOG2(llInfo, "Reading defaults for Tissue: ", fname);

        FILE *f = fopen(fname, "r");
        if (!f)
            throw new Error(__FILE__, __LINE__, "Cannot open file", fname);

        scene::ParseTissueSettings(f, this, false);

        fclose(f);
    }
    catch (Error *err)
    {
        LogError(err);
    }
}


#ifdef QT_CORE_LIB
void anyTissueSettings::prepare_dialog()
{
    dialog->dialog->groupBox_tissueparams->setVisible(true);
    dialog->dialog->groupBox_Library->setVisible(true);

    anyEditable::prepare_dialog();

    dialog->dialog->pushButton_color->setAutoFillBackground(true);

    // set properties...
    dialog->dialog->lineEdit_name->setText(name);
    char style[200];
    dialog->color = color;
    snprintf(style, 200, "background-color: rgb(%d,%d,%d)", int(floor(color.rgba_array[0]*255.0)), int(floor(color.rgba_array[1]*255.0)), int(floor(color.rgba_array[2]*255.0)));
    dialog->dialog->pushButton_color->setStyleSheet(style);
    dialog->dialog->radioButton_t_type_normal->setChecked(type == sat::ttNormal);
    dialog->dialog->radioButton_t_type_tumor->setChecked(type == sat::ttTumor);
    dialog->dialog->doubleSpinBox_t_radius->setValue(cell_r);
    dialog->dialog->doubleSpinBox_t_density->setValue(density);
    dialog->dialog->doubleSpinBox_t_growth->setValue(cell_grow_speed);
    dialog->dialog->doubleSpinBox_t_mininttime->setValue(minimum_interphase_time);
    dialog->dialog->doubleSpinBox_t_timetoapop->setValue(time_to_apoptosis);
    dialog->dialog->doubleSpinBox_t_timetonecr->setValue(time_to_necrosis);
    dialog->dialog->doubleSpinBox_t_timetonecrvar->setValue(time_to_necrosis_var);
    dialog->dialog->doubleSpinBox_t_timeinnecr->setValue(time_in_necrosis);
    dialog->dialog->doubleSpinBox_t_deadradius->setValue(dead_r);
    dialog->dialog->doubleSpinBox_t_shrink->setValue(cell_shrink_speed);
    dialog->dialog->doubleSpinBox_t_minmitradius->setValue(minimum_mitosis_r);
    dialog->dialog->lineEdit_t_maxpressure->setText(QString::number(max_pressure));
    dialog->dialog->lineEdit_t_repfactor->setText(QString::number(force_rep_factor));
    dialog->dialog->lineEdit_t_attrfactor1->setText(QString::number(force_atr1_factor));
    dialog->dialog->lineEdit_t_attrfactor2->setText(QString::number(force_atr2_factor));
    dialog->dialog->lineEdit_t_dpdfactor->setText(QString::number(force_dpd_factor));
    dialog->dialog->lineEdit_t_dpdtemp->setText(QString::number(dpd_temperature));
    dialog->dialog->lineEdit_t_o2cons->setText(QString::number(o2_consumption));
    dialog->dialog->doubleSpinBox_t_o2hypoxia->setValue(o2_hypoxia);
    dialog->dialog->lineEdit_t_per_prod->setText(QString::number(pericyte_production));
}
#endif


#ifdef QT_CORE_LIB
bool anyTissueSettings::validate_properties()
{
    // tissue name given?...
    dialog->dialog->lineEdit_name->setText(dialog->dialog->lineEdit_name->text().trimmed());
    if (dialog->dialog->lineEdit_name->text() == "")
    {
        dialog->dialog->groupBox_msg->setVisible(true);
        dialog->dialog->label_msg->setText(QObject::tr("Tissue name missing."));
        dialog->dialog->lineEdit_name->setFocus();
        return false;
    }

    // is tissue name unique?...
    anyTissueSettings *ts = scene::FindTissueSettings(dialog->dialog->lineEdit_name->text().toLatin1());
    if (ts && ts != this)
    {
        dialog->dialog->groupBox_msg->setVisible(true);
        dialog->dialog->label_msg->setText(QObject::tr("Tissue with that name is already defined."));
        dialog->dialog->lineEdit_name->setFocus();
        return false;
    }

    return true;
}
#endif


#ifdef QT_CORE_LIB
bool anyTissueSettings::validate_removal()
{
    // is tissue used by any cell block?...
    anyCellBlock *cb = scene::FirstCellBlock;
    while (cb)
    {
        if (cb->tissue == this)
        {
            dialog->dialog->groupBox_msg->setVisible(true);
            dialog->dialog->label_msg->setText(QObject::tr("Tissue is used by one or more cell blocks."));
            return false;
        }
        cb = (anyCellBlock *)cb->next;
    }

    // is tissue used by any cell?...
    int first_cell = 0;
    for (int box_id = 0; box_id < SimulationSettings.no_boxes; box_id++)
    {
        int no_cells = scene::Cells[first_cell].no_cells_in_box;
        for (int i = 0; i < no_cells; i++)
            if (scene::Cells[first_cell + i].tissue == this)
            {
                dialog->dialog->groupBox_msg->setVisible(true);
                dialog->dialog->label_msg->setText(QObject::tr("Tissue is used by one or more cell."));
                return false;
            }

        first_cell += SimulationSettings.max_cells_per_box;
    }

    return true;
}
#endif


#ifdef QT_CORE_LIB
void anyTissueSettings::update_from_dialog()
{
    anyEditable::update_from_dialog();

    // get properties...
    delete name;
    name = new char[dialog->dialog->lineEdit_name->text().length() + 1];
    if (dialog->dialog->radioButton_t_type_normal->isChecked())
        type = sat::ttNormal;
    else if (dialog->dialog->radioButton_t_type_tumor->isChecked())
        type = sat::ttTumor;
    strcpy(name, dialog->dialog->lineEdit_name->text().toLatin1());
    color = dialog->color;
    cell_r = anyTissueSettings::billion_to_inf(dialog->dialog->doubleSpinBox_t_radius->value());
    density = anyTissueSettings::billion_to_inf(dialog->dialog->doubleSpinBox_t_density->value());
    cell_grow_speed = anyTissueSettings::billion_to_inf(dialog->dialog->doubleSpinBox_t_growth->value());
    minimum_interphase_time = anyTissueSettings::billion_to_inf(dialog->dialog->doubleSpinBox_t_mininttime->value());
    time_to_apoptosis = anyTissueSettings::billion_to_inf(dialog->dialog->doubleSpinBox_t_timetoapop->value());
    time_to_necrosis = anyTissueSettings::billion_to_inf(dialog->dialog->doubleSpinBox_t_timetonecr->value());
    time_to_necrosis_var = anyTissueSettings::billion_to_inf(dialog->dialog->doubleSpinBox_t_timetonecrvar->value());
    time_in_necrosis = anyTissueSettings::billion_to_inf(dialog->dialog->doubleSpinBox_t_timeinnecr->value());
    dead_r = anyTissueSettings::billion_to_inf(dialog->dialog->doubleSpinBox_t_deadradius->value());
    cell_shrink_speed = anyTissueSettings::billion_to_inf(dialog->dialog->doubleSpinBox_t_shrink->value());
    minimum_mitosis_r = anyTissueSettings::billion_to_inf(dialog->dialog->doubleSpinBox_t_minmitradius->value());
    max_pressure = anyTissueSettings::billion_to_inf(dialog->dialog->lineEdit_t_maxpressure->text().toDouble());
    force_rep_factor = anyTissueSettings::billion_to_inf(dialog->dialog->lineEdit_t_repfactor->text().toDouble());
    force_atr1_factor = anyTissueSettings::billion_to_inf(dialog->dialog->lineEdit_t_attrfactor1->text().toDouble());
    force_atr2_factor = anyTissueSettings::billion_to_inf(dialog->dialog->lineEdit_t_attrfactor2->text().toDouble());
    force_dpd_factor = anyTissueSettings::billion_to_inf(dialog->dialog->lineEdit_t_dpdfactor->text().toDouble());
    dpd_temperature= anyTissueSettings::billion_to_inf(dialog->dialog->lineEdit_t_dpdtemp->text().toDouble());
    o2_consumption = anyTissueSettings::billion_to_inf(dialog->dialog->lineEdit_t_o2cons->text().toDouble());
    o2_hypoxia = dialog->dialog->doubleSpinBox_t_o2hypoxia->value();
    pericyte_production = dialog->dialog->lineEdit_t_per_prod->text().toDouble();

    LOG(llDebug, "Tissue updated from dialog");
}
#endif

