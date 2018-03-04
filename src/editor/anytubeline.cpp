#include "anytubeline.h"

anyTubeLine::anyTubeLine(): tube_length(50), r(0), min_blood_pressure(-100), max_blood_pressure(100), fixed_blood_pressure(true), generated(false)
{

}

float anyTubeLine::billion_to_inf(float x)
{
    return x == 1000000000 ? MAX_float : x;
}



void anyTubeLine::read_defaults()
{
    char fname[P_MAX_PATH];

    snprintf(fname, P_MAX_PATH, "%s%stube_line.ag", GlobalSettings.app_dir, FOLDER_DEFAULTS);

    try
    {
        LOG2(llInfo, "Reading defaults for TubeLine: ", fname);

        FILE *f = fopen(fname, "r");
        if (!f)
            throw new Error(__FILE__, __LINE__, "Cannot open file", fname);

        scene::ParseTubeLine(f, this, false);

        fclose(f);
    }
    catch (Error *err)
    {
        LogError(err);
    }
}

#ifdef QT_CORE_LIB

void anyTubeLine::prepare_dialog()
{
    dialog->dialog->groupBox_ends->setVisible(true);
    dialog->dialog->groupBox_tube->setVisible(true);
    dialog->dialog->groupBox_blood->setVisible(true);

    anyEditable::prepare_dialog();

    // set properties...
    dialog->dialog->doubleSpinBox_tube_radius->setValue(r);
    dialog->dialog->doubleSpinBox_tube_length->setValue(tube_length);
    dialog->dialog->doubleSpinBox_blood_min_pressure->setValue(min_blood_pressure);
    dialog->dialog->doubleSpinBox_blood_max_pressure->setValue(max_blood_pressure);
    dialog->dialog->checkBox_blood_fixed->setChecked(fixed_blood_pressure);
}

bool anyTubeLine::validate_properties()
{
    // tube length?...
    anyVector v(dialog->dialog->doubleSpinBox_end_x->value() - dialog->dialog->doubleSpinBox_start_x->value(),
                dialog->dialog->doubleSpinBox_end_y->value() - dialog->dialog->doubleSpinBox_start_y->value(),
                dialog->dialog->doubleSpinBox_end_z->value() - dialog->dialog->doubleSpinBox_start_z->value());
    if (v.length() < 10)
    {
        dialog->dialog->groupBox_msg->setVisible(true);
        dialog->dialog->label_msg->setText(QObject::tr("Tube to short. Minimum length is 10."));
        return false;
    }

    return true;
}

void anyTubeLine::update_from_dialog()
{
    anyEditable::update_from_dialog();

    // get properties...
    r = dialog->dialog->doubleSpinBox_tube_radius->value();
    tube_length = dialog->dialog->doubleSpinBox_tube_length->value();
    min_blood_pressure = dialog->dialog->doubleSpinBox_blood_min_pressure->value();
    max_blood_pressure = dialog->dialog->doubleSpinBox_blood_max_pressure->value();
    fixed_blood_pressure = dialog->dialog->checkBox_blood_fixed->isChecked();

    LOG(llDebug, "TubeLine updated from dialog");
}

void anyTubeLine::display_properties(QTextBrowser *tb)
{
    tb->clear();
    tb->append(MODEL_TUBELINE_NAME.toUpper());
    tb->append("");
    tb->append(QObject::tr("tube radius: ") + "<b>" + anyTubeLine::float_to_str(r) + "</b>");
    tb->append(QObject::tr("tube length: ") + "<b>" + anyTubeLine::float_to_str(tube_length) + "</b>");
    if (fixed_blood_pressure)
    {
        tb->append(QObject::tr("fixed blood pressure:"));
        tb->append(QObject::tr("- min pressure: ") + "<b>" + anyTubeLine::float_to_str(min_blood_pressure) + "</b>");
        tb->append(QObject::tr("- max pressure: ") + "<b>" + anyTubeLine::float_to_str(max_blood_pressure) + "</b>");
    }
}

QString anyTubeLine::float_to_str(float x)
{
    if (x == MAX_float)
        return QString("inf");
    else
        return QString::number(x);
}

#endif

void anyTubeLine::add_itself_to_scene()
{
    scene::AddTubeLine(this);
}

void anyTubeLine::remove_itself_from_scene()
{
    LOG(llDebug, "Removing tube line");
    scene::RemoveTubeLine(this);
}

char* anyTubeLine::get_name()
{
    static char buff[100];
    #ifdef QT_CORE_LIB
      snprintf(buff, 30, "%s", MODEL_TUBELINE_NAME.toLatin1().constData());
    #else
      snprintf(buff, 30, "%s", MODEL_TUBELINE_NAME_PCHAR);
    #endif
    return buff;
}
