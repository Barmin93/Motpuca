#include "anytubebundle.h"

anyTubeBundle::anyTubeBundle(): extent_x(0), spacing_y(0), spacing_z(0),
    shift_y(0), shift_z(0), r(0), min_blood_pressure(-100), max_blood_pressure(100), fixed_blood_pressure(true), generated(false){

}

real anyTubeBundle::billion_to_inf(real x)
{
    return x == 1000000000 ? MAX_REAL : x;
}


QString anyTubeBundle::real_to_str(real x)
{
    if (x == MAX_REAL)
        return QString("inf");
    else
        return QString::number(x);
}

void anyTubeBundle::add_itself_to_scene()
{
    scene::AddTubeBundle(this);
}
void anyTubeBundle::remove_itself_from_scene()
{
    LOG(llDebug, "Removing tube bundle");
    scene::RemoveTubeBundle(this);
}
char* anyTubeBundle::get_name()
{
    static char buff[100];
    #ifdef QT_CORE_LIB
      snprintf(buff, 30, "%s", MODEL_TUBEBUNDLE_NAME.toLatin1().constData());
    #else
      snprintf(buff, 30, "%s", MODEL_TUBEBUNDLE_NAME_PCHAR);
    #endif
    return buff;
}
void anyTubeBundle::display_properties(QTextBrowser *tb)
{
    tb->clear();
    tb->append(MODEL_TUBEBUNDLE_NAME.toUpper());
    tb->append("");
    tb->append(QObject::tr("width: ") + "<b>" + anyTubeBundle::real_to_str(to.x - from.x) + " + 2*" + anyTubeBundle::real_to_str(extent_x) + "</b>");
    tb->append(QObject::tr("height: ") + "<b>" + anyTubeBundle::real_to_str(to.y - from.y) + "</b>");
    tb->append(QObject::tr("depth: ") + "<b>" + anyTubeBundle::real_to_str(to.z - from.z) + "</b>");
    tb->append(QObject::tr("tube radius: ") + "<b>" + anyTubeBundle::real_to_str(r) + "</b>");
    tb->append(QObject::tr("tube length: ") + "<b>" + anyTubeBundle::real_to_str(tube_length) + "</b>");
    tb->append(QObject::tr("extension: ") + "<b>" + anyTubeBundle::real_to_str(extent_x) + "</b>");
    tb->append(QObject::tr("y spacing: ") + "<b>" + anyTubeBundle::real_to_str(spacing_y) + "</b>");
    tb->append(QObject::tr("z spacing: ") + "<b>" + anyTubeBundle::real_to_str(spacing_z) + "</b>");
    tb->append(QObject::tr("y shift: ") + "<b>" + anyTubeBundle::real_to_str(shift_y) + "</b>");
    tb->append(QObject::tr("z shift: ") + "<b>" + anyTubeBundle::real_to_str(shift_z) + "</b>");
    if (fixed_blood_pressure)
    {
        tb->append(QObject::tr("fixed blood pressure:"));
        tb->append(QObject::tr("- min pressure: ") + "<b>" + anyTubeBundle::real_to_str(min_blood_pressure) + "</b>");
        tb->append(QObject::tr("- max pressure: ") + "<b>" + anyTubeBundle::real_to_str(max_blood_pressure) + "</b>");
    }
}
void anyTubeBundle::read_defaults()
{
    char fname[P_MAX_PATH];

    snprintf(fname, P_MAX_PATH, "%s%stube_bundle.ag", GlobalSettings.app_dir, FOLDER_DEFAULTS);

    try
    {
        LOG2(llInfo, "Reading defaults for TubeBundle: ", fname);

        FILE *f = fopen(fname, "r");
        if (!f)
            throw new Error(__FILE__, __LINE__, "Cannot open file", fname);

        scene::ParseTubeBundle(f, this, false);

        fclose(f);
    }
    catch (Error *err)
    {
        LogError(err);
    }
}

#ifdef QT_CORE_LIB
void anyTubeBundle::prepare_dialog()
{
    dialog->dialog->groupBox_size->setVisible(true);
    dialog->dialog->groupBox_tube->setVisible(true);
    dialog->dialog->groupBox_bundle->setVisible(true);
    dialog->dialog->groupBox_blood->setVisible(true);

    anyEditable::prepare_dialog();

    // set properties...
    dialog->dialog->doubleSpinBox_extension->setValue(extent_x);
    dialog->dialog->doubleSpinBox_tube_radius->setValue(r);
    dialog->dialog->doubleSpinBox_tube_length->setValue(tube_length);
    dialog->dialog->doubleSpinBox_spacing_y->setValue(spacing_y);
    dialog->dialog->doubleSpinBox_spacing_z->setValue(spacing_z);
    dialog->dialog->doubleSpinBox_shift_y->setValue(shift_y);
    dialog->dialog->doubleSpinBox_shift_z->setValue(shift_z);
    dialog->dialog->doubleSpinBox_blood_min_pressure->setValue(min_blood_pressure);
    dialog->dialog->doubleSpinBox_blood_max_pressure->setValue(max_blood_pressure);
    dialog->dialog->checkBox_blood_fixed->setChecked(fixed_blood_pressure);
}
#endif

#ifdef QT_CORE_LIB
bool anyTubeBundle::validate_properties()
{
    return true;
}
#endif

#ifdef QT_CORE_LIB
void anyTubeBundle::update_from_dialog()
{
    anyEditable::update_from_dialog();

    // get properties...
    extent_x = anyTubeBundle::billion_to_inf(dialog->dialog->doubleSpinBox_extension->value());
    r = anyTubeBundle::billion_to_inf(dialog->dialog->doubleSpinBox_tube_radius->value());
    tube_length = anyTubeBundle::billion_to_inf(dialog->dialog->doubleSpinBox_tube_length->value());
    spacing_y = anyTubeBundle::billion_to_inf(dialog->dialog->doubleSpinBox_spacing_y->value());
    spacing_z = anyTubeBundle::billion_to_inf(dialog->dialog->doubleSpinBox_spacing_z->value());
    shift_y = anyTubeBundle::billion_to_inf(dialog->dialog->doubleSpinBox_shift_y->value());
    shift_z = anyTubeBundle::billion_to_inf(dialog->dialog->doubleSpinBox_shift_z->value());
    min_blood_pressure = dialog->dialog->doubleSpinBox_blood_min_pressure->value();
    max_blood_pressure = dialog->dialog->doubleSpinBox_blood_max_pressure->value();
    fixed_blood_pressure = dialog->dialog->checkBox_blood_fixed->isChecked();

    LOG(llDebug, "TubeBundle updated from dialog");
}
#endif
