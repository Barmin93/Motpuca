#include "anycellblock.h"

anyCellBlock::anyCellBlock(): tissue(0), generated(false)
{
    for (int i = 0; i < sat::dsLast; i++)
    concentrations[i] = 0;
}

real anyCellBlock::billion_to_inf(real x)
{
    return x == 1000000000 ? MAX_REAL : x;
}


QString anyCellBlock::real_to_str(real x)
{
    if (x == MAX_REAL)
        return QString("inf");
    else
        return QString::number(x);
}

bool anyCellBlock::should_be_generated_next()
{
    // kiedy mozemy generowac? kiedy nie ma mniejszych niewygenerowanych...

    // bounding box...
    anyVector bb_from, bb_to;
    bool first = true;
    update_bounding_box(bb_from, bb_to, first);

    anyCellBlock *cb = scene::FirstCellBlock;
    while (cb)
    {
        if (cb != this && !cb->generated)
        {
            anyVector bb_from2, bb_to2;
            first = true;
            cb->update_bounding_box(bb_from2, bb_to2, first);

            int bc = 0;

            bc += bb_from2.x > bb_from.x;
            bc += bb_from2.y > bb_from.y;
            bc += bb_from2.z > bb_from.z;
            bc += bb_to2.x < bb_to.x;
            bc += bb_to2.y < bb_to.y;
            bc += bb_to2.z < bb_to.z;
            if (bc > 3)
                return false;

        }
        cb = (anyCellBlock *)cb->next;
    }

    return true;
}
void anyCellBlock::read_defaults()
{
    char fname[P_MAX_PATH];

    snprintf(fname, P_MAX_PATH, "%s%scell_block.ag", GlobalSettings.app_dir, FOLDER_DEFAULTS);

    try
    {
        LOG2(llInfo, "Reading defaults for CellBlock: ", fname);

        FILE *f = fopen(fname, "r");
        if (!f)
            throw new Error(__FILE__, __LINE__, "Cannot open file", fname);

        scene::ParseCellBlock(f, this, false);

        fclose(f);
    }
    catch (Error *err)
    {
        LogError(err);
    }
}
void anyCellBlock::prepare_dialog()
{
    dialog->dialog->groupBox_size->setVisible(true);
    dialog->dialog->groupBox_tissue->setVisible(true);
    dialog->dialog->groupBox_conc->setVisible(true);

    anyEditable::prepare_dialog();

    dialog->dialog->doubleSpinBox_concO2->setValue(concentrations[sat::dsO2]);
    dialog->dialog->doubleSpinBox_concTAF->setValue(concentrations[sat::dsTAF]);
    dialog->dialog->doubleSpinBox_concPericytes->setValue(concentrations[sat::dsPericytes]);

    // select tissue in combo...
    if (tissue)
        for (int i = 0; i < dialog->dialog->comboBox_tissue->count(); i++)
            if (dialog->dialog->comboBox_tissue->itemText(i) == QString(tissue->name))
            {
                dialog->dialog->comboBox_tissue->setCurrentIndex(i);
                break;
            }
}
bool anyCellBlock::validate_properties()
{
    // tissue selected in combo?...
    if (dialog->dialog->comboBox_tissue->currentIndex() == -1)
    {
        dialog->dialog->groupBox_msg->setVisible(true);
        dialog->dialog->label_msg->setText(QObject::tr("Tissue missing! Press \'Add new tissue\' button and define a tissue."));
        return false;
    }

    return true;
}
void anyCellBlock::update_from_dialog()
{
    anyEditable::update_from_dialog();

    // find and update tissue...
    anyTissueSettings *ts = scene::FirstTissueSettings;
    while (ts)
    {
        if (QString(ts->name) == dialog->dialog->comboBox_tissue->currentText())
        {
            tissue = ts;
            break;
        }
        ts = ts->next;
    }

    concentrations[sat::dsO2] = dialog->dialog->doubleSpinBox_concO2->value();
    concentrations[sat::dsTAF] = dialog->dialog->doubleSpinBox_concTAF->value();
    concentrations[sat::dsPericytes] = dialog->dialog->doubleSpinBox_concPericytes->value();

    LOG(llDebug, "CellBlock updated from dialog");
}
bool anyCellBlock::can_move_up()
{
    return scene::LastCellBlock != this;
}
bool anyCellBlock::can_move_down()
{
    return scene::FirstCellBlock != this;
}
void anyCellBlock::move_up()
{
    if (!can_move_up())
        return;

    // find previous element...
    anyEditable *pe;
    if (this == scene::FirstCellBlock)
        pe = 0;
    else
    {
        pe = scene::FirstCellBlock;
        while (pe && pe->next != this)
            pe = pe->next;
    }

    // find next element...
    anyEditable *ne = next;

    // remove 'this' from list...
    if (pe)
        pe->next = ne;
    else
        scene::FirstCellBlock = (anyCellBlock *)ne;

    // add 'this' one element ahead...
    next = ne->next;
    if (scene::LastCellBlock == ne)
        scene::LastCellBlock = this;
    ne->next = this;
}
void anyCellBlock::move_down()
{
    if (!can_move_down())
        return;

    // find previous element...
    anyEditable *pe;
    if (this == scene::FirstCellBlock)
        pe = 0;
    else
    {
        pe = scene::FirstCellBlock;
        while (pe && pe->next != this)
            pe = pe->next;
    }

    if (pe)
        pe->move_up();
}
char* anyCellBlock::get_name()
{
    static char buff[1000];
    if (tissue)
        snprintf(buff, 1000, "cell block (%s)", tissue->name);
    else
        snprintf(buff, 1000, "cell block");
    return buff;
}
void anyCellBlock::display_properties(QTextBrowser *tb)
{
    tb->clear();
    tb->append(QObject::tr("CELL BLOCK"));
    tb->append("");
    tb->append(QObject::tr("tissue name: ") + " <b>" + tissue->name + "</b>");
    tb->append(QObject::tr("width: ") + "<b>" + anyCellBlock::real_to_str(to.x - from.x) + "</b>");
    tb->append(QObject::tr("height: ") + "<b>" + anyCellBlock::real_to_str(to.y - from.y) + "</b>");
    tb->append(QObject::tr("depth: ") + "<b>" + anyCellBlock::real_to_str(to.z - from.z) + "</b>");
    if (!generated)
    {
        tb->append(QObject::tr("init O2 conc: ") + "<b>" + anyCellBlock::real_to_str(concentrations[sat::dsO2]) + "</b>");
        tb->append(QObject::tr("init TAF conc: ") + "<b>" + anyCellBlock::real_to_str(concentrations[sat::dsTAF]) + "</b>");
        tb->append(QObject::tr("init Pericytes conc: ") + "<b>" + anyCellBlock::real_to_str(concentrations[sat::dsPericytes]) + "</b>");
    }
}
void anyCellBlock::add_itself_to_scene()
{
    scene::AddCellBlock(this);
}
void anyCellBlock::remove_itself_from_scene()
{
    LOG(llDebug, "Removing cell block");
    scene::RemoveCellBlock(this);
}
