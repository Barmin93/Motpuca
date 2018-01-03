#include "anyglobalsdialog.h"

#include "config.h"

anyGlobalsDialog::anyGlobalsDialog()
{
    dialog = new Ui_DialogGlobals;
    dialog->setupUi(this);
    connect(dialog->pushButton_Ok, SIGNAL(clicked()), this, SLOT(slot_ok_clicked()));
    connect(dialog->pushButton_Apply, SIGNAL(clicked()), this, SLOT(slot_apply_clicked()));
    connect(dialog->pushButton_Cancel, SIGNAL(clicked()), this, SLOT(slot_cancel_clicked()));

    connect(dialog->pushButton_colorbackground, SIGNAL(clicked()), this, SLOT(slot_color_clicked()));
    connect(dialog->pushButton_colortubes, SIGNAL(clicked()), this, SLOT(slot_color_clicked()));
    connect(dialog->pushButton_colorbarrier_in, SIGNAL(clicked()), this, SLOT(slot_color_clicked()));
    connect(dialog->pushButton_colorbarrier_out, SIGNAL(clicked()), this, SLOT(slot_color_clicked()));
    connect(dialog->pushButton_colornavigator, SIGNAL(clicked()), this, SLOT(slot_color_clicked()));
    connect(dialog->pushButton_colorclip, SIGNAL(clicked()), this, SLOT(slot_color_clicked()));
    connect(dialog->pushButton_colorcellalive, SIGNAL(clicked()), this, SLOT(slot_color_clicked()));
    connect(dialog->pushButton_colorcellhypoxia, SIGNAL(clicked()), this, SLOT(slot_color_clicked()));
    connect(dialog->pushButton_colorcellnecrosis, SIGNAL(clicked()), this, SLOT(slot_color_clicked()));
    connect(dialog->pushButton_colorcellapoptosis, SIGNAL(clicked()), this, SLOT(slot_color_clicked()));

    dialog->doubleSpinBox_boxsize->setEnabled(!GlobalSettings.simulation_allocated);
    dialog->spinBox_cellsperbox->setEnabled(!GlobalSettings.simulation_allocated);
    dialog->spinBox_max_tube_chains->setEnabled(!GlobalSettings.simulation_allocated);

    dialog->spinBox_graphrate->setEnabled(!Statistics);

    // align columns...
    ColumnResizer* resizer = new ColumnResizer(this);
    resizer->addWidgetsFromLayout(dialog->groupBox_colors_general->layout(), 0);
    resizer->addWidgetsFromLayout(dialog->groupBox_colors_cellstates->layout(), 0);
}

float anyGlobalsDialog::billion_to_inf(float x)
{
    return x == 1000000000 ? MAX_float : x;
}

void anyGlobalsDialog::slot_ok_clicked()
/**
  Handles 'Ok' button on global settings dialog.
*/
{
    if (slot_apply_clicked())
        accept();
}

bool anyGlobalsDialog::slot_apply_clicked()
/**
  Handles 'Apply' button on global settings dialog.
*/
{
    if (!validate_settings())
        return false;

    update_from_dialog();

    SaveNeeded(true);

    MainWindowPtr->slot_gl_repaint();

    return true;
}

void anyGlobalsDialog::slot_cancel_clicked()
/**
  Handles 'Cancel' button on global settings dialog.
*/
{
    reject();
}

void anyGlobalsDialog::slot_color_clicked()
{
    QColorDialog cdialog;
    anyColor color = get_color_from_button((QPushButton *)QObject::sender());
    cdialog.setCurrentColor(QColor(color.r255(), color.g255(), color.b255()));

    if (!cdialog.exec())
        return;

    color.set(cdialog.currentColor().redF(), cdialog.currentColor().greenF(), cdialog.currentColor().blueF());
    set_color_of_button((QPushButton *)QObject::sender(), color);
}

void anyGlobalsDialog::prepare_dialog()
{
    resize(geometry().width(), 100);

    // model tube names...
    dialog->checkBox_sim_tube_div->setText(tr("proliferation") + " (" + MODEL_TUBE_SHORTNAME_PL + ")");
    dialog->tabWidget->setTabText(1, MODEL_TUBULARSYS_NAME.left(1).toUpper() + MODEL_TUBULARSYS_NAME.mid(1));

    // set up fields...

    // simulation settings...
    dialog->doubleSpinBox_timestep->setValue(SimulationSettings.time_step);
    dialog->radioButton_2d->setChecked(SimulationSettings.dimensions == 2);
    dialog->radioButton_3d->setChecked(SimulationSettings.dimensions == 3);
    dialog->doubleSpinBox_boxsize->setValue(SimulationSettings.box_size);
    dialog->spinBox_cellsperbox->setValue(SimulationSettings.max_cells_per_box);
    dialog->doubleSpinBox_forcercut->setValue(SimulationSettings.force_r_cut);
    dialog->spinBox_max_tube_chains->setValue(SimulationSettings.max_tube_chains);
    dialog->doubleSpinBox_diffcoefO2->setValue(SimulationSettings.diffusion_coeff[sat::dsO2]);
    dialog->doubleSpinBox_diffcoefTAF->setValue(SimulationSettings.diffusion_coeff[sat::dsTAF]);
    dialog->doubleSpinBox_diffcoefPeri->setValue(SimulationSettings.diffusion_coeff[sat::dsPericytes]);
    dialog->doubleSpinBox_stop_time->setValue(SimulationSettings.stop_time);

    dialog->spinBox_savepovray->setValue(SimulationSettings.save_povray);
    dialog->spinBox_saveag->setValue(SimulationSettings.save_ag);
    dialog->spinBox_savestats->setValue(SimulationSettings.save_statistics);
    dialog->spinBox_graphrate->setValue(SimulationSettings.graph_sampling);

    dialog->checkBox_sim_diffusion->setChecked(SimulationSettings.sim_phases & sat::spDiffusion);
    dialog->checkBox_sim_forces->setChecked(SimulationSettings.sim_phases & sat::spForces);
    dialog->checkBox_sim_mitosis->setChecked(SimulationSettings.sim_phases & sat::spMitosis);
    dialog->checkBox_sim_tube_div->setChecked(SimulationSettings.sim_phases & sat::spTubeDiv);
    dialog->checkBox_sim_flow->setChecked(SimulationSettings.sim_phases & sat::spBloodFlow);
    dialog->checkBox_sim_growth->setChecked(SimulationSettings.sim_phases & sat::spGrow);

    // tubular system settings...
    dialog->lineEdit_o2prod->setText(QString::number(TubularSystemSettings.o2_production));
    dialog->doubleSpinBox_tube_density->setValue(TubularSystemSettings.density);
    dialog->lineEdit_tube_force_length->setText(QString::number(TubularSystemSettings.force_length_keep_factor));
    dialog->lineEdit_tube_force_angle->setText(QString::number(TubularSystemSettings.force_angle_factor));
    dialog->lineEdit_tube_force_bind->setText(QString::number(TubularSystemSettings.force_chain_attr_factor));
    dialog->lineEdit_lengthening_speed->setText(QString::number(TubularSystemSettings.lengthening_speed));
    dialog->lineEdit_thickening_speed->setText(QString::number(TubularSystemSettings.thickening_speed));
    dialog->lineEdit_tube_rep_factor->setText(QString::number(TubularSystemSettings.force_rep_factor));
    dialog->lineEdit_tube_atr1_factor->setText(QString::number(TubularSystemSettings.force_atr1_factor));
    dialog->lineEdit_tube_atr2_factor->setText(QString::number(TubularSystemSettings.force_atr2_factor));
    dialog->doubleSpinBox_tube_min_interphase_time->setValue(TubularSystemSettings.minimum_interphase_time);
    dialog->doubleSpinBox_tube_TAF_trigger->setValue(TubularSystemSettings.TAFtrigger);
    dialog->doubleSpinBox_tube_min_blood_flow->setValue(TubularSystemSettings.minimum_blood_flow);
    dialog->doubleSpinBox_tube_time_degradation->setValue(TubularSystemSettings.time_to_degradation);

    // visual settings...
    set_color_of_button(dialog->pushButton_colorbackground, VisualSettings.bkg_color);
    dialog->label_colortubes->setText(MODEL_TUBELINE_SHORTNAME_PL.left(1).toUpper() + MODEL_TUBELINE_SHORTNAME_PL.mid(1) + ":");
    set_color_of_button(dialog->pushButton_colortubes, VisualSettings.tube_color);
    set_color_of_button(dialog->pushButton_colorbarrier_in, VisualSettings.in_barrier_color);
    set_color_of_button(dialog->pushButton_colorbarrier_out, VisualSettings.out_barrier_color);
    set_color_of_button(dialog->pushButton_colornavigator, VisualSettings.navigator_color);
    set_color_of_button(dialog->pushButton_colorclip, VisualSettings.clip_plane_color);
    set_color_of_button(dialog->pushButton_colorcellalive, VisualSettings.cell_alive_color);
    set_color_of_button(dialog->pushButton_colorcellhypoxia, VisualSettings.cell_hypoxia_color);
    set_color_of_button(dialog->pushButton_colorcellapoptosis, VisualSettings.cell_apoptosis_color);
    set_color_of_button(dialog->pushButton_colorcellnecrosis, VisualSettings.cell_necrosis_color);
    set_color_of_button(dialog->pushButton_colorselection, VisualSettings.selection_color);
}

bool anyGlobalsDialog::validate_settings()
{
    return true;
}

void anyGlobalsDialog::set_color_of_button(QPushButton *button, anyColor const &color)
{
    char style[200];
    snprintf(style, 200, "background-color: rgb(%d,%d,%d)", int(floor(color.rgba_array[0]*255.0)), int(floor(color.rgba_array[1]*255.0)), int(floor(color.rgba_array[2]*255.0)));
    button->setStyleSheet(style);
}

anyColor anyGlobalsDialog::get_color_from_button(QPushButton *button) const
{
    int r, g, b;

    sscanf(button->styleSheet().toLatin1(), "background-color: rgb(%d, %d, %d)", &r, &g, &b);
    return anyColor(r/255.0, g/255.0, b/255.0);
}

void anyGlobalsDialog::update_from_dialog()
{
    // simulation settings...
    SimulationSettings.time_step = dialog->doubleSpinBox_timestep->value();
    SimulationSettings.dimensions = 2 + dialog->radioButton_3d->isChecked();
    SimulationSettings.box_size = dialog->doubleSpinBox_boxsize->value();
    SimulationSettings.max_cells_per_box = dialog->spinBox_cellsperbox->value();
    SimulationSettings.force_r_cut = dialog->doubleSpinBox_forcercut->value();
    SimulationSettings.max_tube_chains = dialog->spinBox_max_tube_chains->value();
    SimulationSettings.diffusion_coeff[sat::dsO2] = dialog->doubleSpinBox_diffcoefO2->value();
    SimulationSettings.diffusion_coeff[sat::dsTAF] = dialog->doubleSpinBox_diffcoefTAF->value();
    SimulationSettings.diffusion_coeff[sat::dsPericytes] = dialog->doubleSpinBox_diffcoefPeri->value();

    SimulationSettings.save_povray = dialog->spinBox_savepovray->value();
    SimulationSettings.save_ag = dialog->spinBox_saveag->value();
    SimulationSettings.save_statistics = dialog->spinBox_savestats->value();
    SimulationSettings.graph_sampling = dialog->spinBox_graphrate->value();

    SimulationSettings.sim_phases = dialog->checkBox_sim_diffusion->isChecked()*sat::spDiffusion +
                                    dialog->checkBox_sim_forces->isChecked()*sat::spForces +
                                    dialog->checkBox_sim_growth->isChecked()*sat::spGrow +
                                    dialog->checkBox_sim_mitosis->isChecked()*sat::spMitosis +
                                    dialog->checkBox_sim_tube_div->isChecked()*sat::spTubeDiv +
                                    dialog->checkBox_sim_flow->isChecked()*sat::spBloodFlow;
    SimulationSettings.stop_time = billion_to_inf(dialog->doubleSpinBox_stop_time->value());

    // tubular system settings...
    TubularSystemSettings.o2_production = dialog->lineEdit_o2prod->text().toDouble();
    TubularSystemSettings.density = dialog->doubleSpinBox_tube_density->value();
    TubularSystemSettings.force_length_keep_factor = dialog->lineEdit_tube_force_length->text().toDouble();
    TubularSystemSettings.force_angle_factor = dialog->lineEdit_tube_force_angle->text().toDouble();
    TubularSystemSettings.force_chain_attr_factor = dialog->lineEdit_tube_force_bind->text().toDouble();
    TubularSystemSettings.lengthening_speed = dialog->lineEdit_lengthening_speed->text().toDouble();
    TubularSystemSettings.thickening_speed = dialog->lineEdit_thickening_speed->text().toDouble();
    TubularSystemSettings.force_rep_factor = dialog->lineEdit_tube_rep_factor->text().toDouble();
    TubularSystemSettings.force_atr1_factor = dialog->lineEdit_tube_atr1_factor->text().toDouble();
    TubularSystemSettings.force_atr2_factor = dialog->lineEdit_tube_atr2_factor->text().toDouble();
    TubularSystemSettings.minimum_interphase_time = dialog->doubleSpinBox_tube_min_interphase_time->value();
    TubularSystemSettings.TAFtrigger = dialog->doubleSpinBox_tube_TAF_trigger->value();
    TubularSystemSettings.minimum_blood_flow = dialog->doubleSpinBox_tube_min_blood_flow->value();
    TubularSystemSettings.time_to_degradation = billion_to_inf(dialog->doubleSpinBox_tube_time_degradation->value());

    // visual settings...
    VisualSettings.bkg_color.set_skip_alpha(get_color_from_button(dialog->pushButton_colorbackground));
    VisualSettings.tube_color.set_skip_alpha(get_color_from_button(dialog->pushButton_colortubes));
    VisualSettings.in_barrier_color.set_skip_alpha(get_color_from_button(dialog->pushButton_colorbarrier_in));
    VisualSettings.out_barrier_color.set_skip_alpha(get_color_from_button(dialog->pushButton_colorbarrier_out));
    VisualSettings.navigator_color.set_skip_alpha(get_color_from_button(dialog->pushButton_colornavigator));
    VisualSettings.clip_plane_color.set_skip_alpha(get_color_from_button(dialog->pushButton_colorclip));
    VisualSettings.cell_alive_color.set_skip_alpha(get_color_from_button(dialog->pushButton_colorcellalive));
    VisualSettings.cell_hypoxia_color.set_skip_alpha(get_color_from_button(dialog->pushButton_colorcellhypoxia));
    VisualSettings.cell_necrosis_color.set_skip_alpha(get_color_from_button(dialog->pushButton_colorcellnecrosis));
    VisualSettings.cell_apoptosis_color.set_skip_alpha(get_color_from_button(dialog->pushButton_colorcellapoptosis));
    VisualSettings.selection_color.set_skip_alpha(get_color_from_button(dialog->pushButton_colorselection));

    LOG(llDebug, "Global settings updated from dialog.");
}

