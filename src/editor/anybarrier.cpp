#include "anybarrier.h"

anyBarrier::anyBarrier(): type(sat::btIn){}

char* anyBarrier::get_name()
{
    static char buff[50];
    if (type == sat::btIn)
        snprintf(buff, 50, "barrier (keeps cells inside)");
    else
        snprintf(buff, 50, "barrier (keeps cells outside)");
    return buff;
}

void anyBarrier::update_bounding_box(anyVector &from, anyVector &to, bool &first)
{
    // barriers influence on simulation bounding box only if (type == btIn)

    if (type == sat::btIn)
        anyEditable::update_bounding_box(from, to, first);
}

void anyBarrier::add_itself_to_scene()
{
    scene::AddBarrier(this);
}
void anyBarrier::remove_itself_from_scene()
{
    LOG(llDebug, "Removing barrier");
    scene::RemoveBarrier(this);
}

bool anyBarrier::is_point_inside(anyVector const &p, float r)
{
    if (type == sat::btIn)
        return anyEditable::is_point_inside(p, r);
    else
        return !anyEditable::is_point_inside(p, -r);
}

void anyBarrier::read_defaults()
{
    char fname[P_MAX_PATH];

    snprintf(fname, P_MAX_PATH, "%s%sbarrier.ag", GlobalSettings.app_dir, FOLDER_DEFAULTS);

    try
    {
        LOG2(llInfo, "Reading defaults for Barrier: ", fname);

        FILE *f = fopen(fname, "r");
        if (!f)
            throw new Error(__FILE__, __LINE__, "Cannot open file", fname);

        scene::ParseBarrier(f, this, false);

        fclose(f);
    }
    catch (Error *err)
    {
        LogError(err);
    }
}


#ifdef QT_CORE_LIB
void anyBarrier::display_properties(QTextBrowser *tb)
{
    tb->clear();
    tb->append(QObject::tr("BARRIER"));
    tb->append("");
    if (type == sat::btIn)
        tb->append(QObject::tr("type: ") + "<b>" + QObject::tr("in") + "</b>");
    else
        tb->append(QObject::tr("type: ") + "<b>" + QObject::tr("out") + "</b>");
    tb->append(QObject::tr("width: ") + "<b>" + float_to_str(to.x - from.x) + "</b>");
    tb->append(QObject::tr("height: ") + "<b>" + float_to_str(to.y - from.y) + "</b>");
    tb->append(QObject::tr("depth: ") + "<b>" + float_to_str(to.z - from.z) + "</b>");
}

void anyBarrier::prepare_dialog()
{
    dialog->dialog->groupBox_size->setVisible(true);
    dialog->dialog->groupBox_barriertype->setVisible(true);

    anyEditable::prepare_dialog();

    // set barrier type...
    if (type == sat::btIn)
        dialog->dialog->radioButton_in->setChecked(true);
    else
        dialog->dialog->radioButton_out->setChecked(true);
}

bool anyBarrier::validate_properties()
{
    return true;
}

void anyBarrier::update_from_dialog()
{
    anyEditable::update_from_dialog();

    // get barrier type...
    if (dialog->dialog->radioButton_in->isChecked())
        type = sat::btIn;
    else
        type = sat::btOut;

    LOG(llDebug, "Barrier updated from dialog");
}

#endif
