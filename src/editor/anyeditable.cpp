#include "anyeditable.h"
#include "anyeditabledialog.h"

anyEditable::anyEditable(): next(0), dialog(0) {}

int anyEditable::display_dialog(bool add_new)
{
    if (dialog) delete dialog;
    dialog = new anyEditableDialog;
    dialog->editable = this;
    dialog->add_new = add_new;

    // model tube names...
    dialog->dialog->groupBox_tube->setTitle(MODEL_TUBE_NAME_PL.left(1).toUpper() + MODEL_TUBE_NAME_PL.mid(1));
    dialog->dialog->groupBox_bundle->setTitle(MODEL_TUBEBUNDLE_NAME.left(1).toUpper() + MODEL_TUBEBUNDLE_NAME.mid(1));

    // align columns...
    ColumnResizer* resizer = new ColumnResizer(dialog);
    resizer->addWidgetsFromLayout(dialog->dialog->groupBox_size->layout(), 0);
    resizer->addWidgetsFromLayout(dialog->dialog->groupBox_bundle->layout(), 0);
    resizer->addWidgetsFromLayout(dialog->dialog->groupBox_tissueparams->layout(), 0);
    resizer->addWidgetsFromLayout(dialog->dialog->groupBox_conc->layout(), 0);
    resizer->addWidgetsFromLayout(dialog->dialog->groupBox_tube->layout(), 0);
    resizer->addWidgetsFromLayout(dialog->dialog->groupBox_blood->layout(), 0);

    // set up buttons...
    dialog->dialog->pushButton_Apply->setVisible(!add_new);
    dialog->dialog->pushButton_Delete->setVisible(!add_new);
    if (add_new)
        dialog->dialog->pushButton_Ok->setText(QObject::tr("Add"));
    else
        dialog->dialog->pushButton_Ok->setText(QObject::tr("Ok"));

    // by default hide all group boxes...
    dialog->dialog->groupBox_size->setVisible(false);
    dialog->dialog->groupBox_ends->setVisible(false);
    dialog->dialog->groupBox_tissue->setVisible(false);
    dialog->dialog->groupBox_barriertype->setVisible(false);
    dialog->dialog->groupBox_tube->setVisible(false);
    dialog->dialog->groupBox_bundle->setVisible(false);
    dialog->dialog->groupBox_tissueparams->setVisible(false);
    dialog->dialog->groupBox_msg->setVisible(false);
    dialog->dialog->groupBox_Library->setVisible(false);
    dialog->dialog->groupBox_conc->setVisible(false);
    dialog->dialog->groupBox_blood->setVisible(false);

    prepare_dialog();
    int result = dialog->exec();
    delete resizer;
    return result;
}

void anyEditable::scale_event(int amount, bool x_axe, bool y_axe, bool z_axe)
{
    float step = amount < 0 ? -10 : 10;
    if (x_axe)
    {
        from.x -= step;
        to.x += step;
    }
    if (y_axe)
    {
        from.y -= step;
        to.y += step;
    }
    if (z_axe)
    {
        from.z -= step;
        to.z += step;
    }
    fix();
}

void anyEditable::trans_event(anyVector d, bool x_axe, bool y_axe, bool z_axe)
{
    if (!x_axe) d.x = 0;
    if (!y_axe) d.y = 0;
    if (!z_axe) d.z = 0;

    from += d;
    to += d;
}

void anyEditable::rotation_event(anyVector d, bool x_axe, bool y_axe, bool z_axe)
{
    if (x_axe)
        trans.rotateX(-d.x);

    if (y_axe)
        trans.rotateY(d.y);

    if (z_axe)
        trans.rotateZ(d.z);
}

float billion_to_inf(float x)
{
    return x == 1000000000 ? MAX_float : x;
}


QString float_to_str(float x)
{
    if (x == MAX_float)
        return QString("inf");
    else
        return QString::number(x);
}

void anyEditable::prepare_dialog()
{
    dialog->resize(dialog->geometry().width(), 100);

    // name...
    dialog->setWindowTitle(QObject::tr("Properties: ") + get_name());

    // size...
    if (dialog->dialog->groupBox_size->isVisibleTo(dialog))
    {
        dialog->dialog->doubleSpinBox_size_x->setValue(to.x - from.x);
        dialog->dialog->doubleSpinBox_size_y->setValue(to.y - from.y);
        dialog->dialog->doubleSpinBox_size_z->setValue(to.z - from.z);
    }

    // end points...
    if (dialog->dialog->groupBox_ends->isVisibleTo(dialog))
    {
        dialog->dialog->doubleSpinBox_start_x->setValue(from.x);
        dialog->dialog->doubleSpinBox_start_y->setValue(from.y);
        dialog->dialog->doubleSpinBox_start_z->setValue(from.z);
        dialog->dialog->doubleSpinBox_end_x->setValue(to.x);
        dialog->dialog->doubleSpinBox_end_y->setValue(to.y);
        dialog->dialog->doubleSpinBox_end_z->setValue(to.z);
    }

    // build tissue combo... (version #1)
    anyTissueSettings *ts = scene::FirstTissueSettings;
    while (ts)
    {
        dialog->dialog->comboBox_tissue->addItem(ts->name);
        ts = ts->next;
    }
}

void anyEditable::update_from_dialog()
{
    if (!dialog) return;

    // size...
    if (dialog->dialog->groupBox_size->isVisibleTo(dialog))
    {
        double w = dialog->dialog->doubleSpinBox_size_x->value();
        from.x = (to.x + from.x - w)*0.5;
        to.x = from.x + w;
        w = dialog->dialog->doubleSpinBox_size_y->value();
        from.y = (to.y + from.y - w)*0.5;
        to.y = from.y + w;
        w = dialog->dialog->doubleSpinBox_size_z->value();
        from.z = (to.z + from.z - w)*0.5;
        to.z = from.z + w;
    }

    // end points...
    if (dialog->dialog->groupBox_ends->isVisibleTo(dialog))
    {
        from.x = dialog->dialog->doubleSpinBox_start_x->value();
        from.y = dialog->dialog->doubleSpinBox_start_y->value();
        from.z = dialog->dialog->doubleSpinBox_start_z->value();
        to.x = dialog->dialog->doubleSpinBox_end_x->value();
        to.y = dialog->dialog->doubleSpinBox_end_y->value();
        to.z = dialog->dialog->doubleSpinBox_end_z->value();
    }
}

