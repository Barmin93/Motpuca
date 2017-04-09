#ifdef QT_CORE_LIB
#include <QColorDialog>
#endif

#include <stdio.h>

#include "config.h"
#include "log.h"
#include "parser.h"

#ifdef QT_CORE_LIB
#include "mainwindow.h"
#endif


anyGlobalSettings GlobalSettings;               ///< global settings
anyVisualSettings VisualSettings;               ///< all visual settings
anySimulationSettings SimulationSettings;       ///< all simulation settings
anyTubularSystemSettings TubularSystemSettings; ///< tubular network settings


void SaveVisualSettings_ag(FILE *f, anyVisualSettings const *vs)
/**
  Saves visual settings to *.ag file.

  \param f -- output file
  \param vs -- pointer to visual settings
*/
{
    fprintf(f, "\n");
    fprintf(f, "Visual\n");
    fprintf(f, " {\n");

    // save all fields...
    SAVE_COLOR(f, vs, bkg_color);
    SAVE_COLOR(f, vs, axis_x_color);
    SAVE_COLOR(f, vs, axis_y_color);
    SAVE_COLOR(f, vs, axis_z_color);
    SAVE_COLOR(f, vs, comp_box_color);
    SAVE_COLOR(f, vs, in_barrier_color);
    SAVE_COLOR(f, vs, out_barrier_color);

    SAVE_COLOR(f, vs, cell_alive_color);
    SAVE_COLOR(f, vs, cell_hypoxia_color);
    SAVE_COLOR(f, vs, cell_apoptosis_color);
    SAVE_COLOR(f, vs, cell_necrosis_color);
    SAVE_COLOR(f, vs, tube_color);

    SAVE_COLOR(f, vs, clip_plane_color);
    SAVE_COLOR(f, vs, navigator_color);
    SAVE_COLOR(f, vs, boxes_color);

    fprintf(f, " }\n");
}


void SaveSimulationSettings_ag(FILE *f, anySimulationSettings const *ss)
/**
  Saves simulation settings to *.ag file.

  \param f -- output file
  \param ss -- pointer to simulation settings
*/
{
    fprintf(f, "\n");
    fprintf(f, "Simulation\n");
    fprintf(f, " {\n");

    // save all fields...
    SAVE_INT(f, ss, dimensions);
    SAVE_INT(f, ss, sim_phases);

    SAVE_REAL(f, ss, time_step);
    SAVE_REAL(f, ss, time);
    SAVE_REAL(f, ss, stop_time);
    SAVE_VECT(f, ss, comp_box_from);
    SAVE_VECT(f, ss, comp_box_to);
    SAVE_REAL(f, ss, box_size);
    SAVE_INT(f, ss, max_cells_per_box);
    SAVE_REAL(f, ss, force_r_cut);

    SAVE_INT(f, ss, max_tube_chains);
    SAVE_INT(f, ss, max_tube_merge);

    SAVE_INT(f, ss, save_statistics);
    SAVE_INT(f, ss, save_povray);
    SAVE_INT(f, ss, save_ag);

    SAVE_INT(f, ss, graph_sampling);

    SAVE_REAL_N(f, ss, diffusion_coeff[dsO2], diffusion_coeff_O2);
    SAVE_REAL_N(f, ss, diffusion_coeff[dsTAF], diffusion_coeff_TAF);
    SAVE_REAL_N(f, ss, diffusion_coeff[dsPericytes], diffusion_coeff_Pericytes);

    fprintf(f, " }\n");
}


void ParseVisualSettingsValue(FILE *f)
/**
  Parses 'visual' block.

  \param f -- input file
*/
 {
    anyToken tv;

    // store value name...
    tv = Token;

    // get '='...
    GetNextToken(f, false);
    if (Token.type != TT_Symbol || Token.symbol != '=')
        throw new Error(__FILE__, __LINE__, "Syntax error ('=' expected)", TokenToString(Token), ParserFile, ParserLine);

    // get value...
    GetNextToken(f, true);

    // assign value...
    if (0) ;
    PARSE_VALUE_INT(VisualSettings, window_width)
    PARSE_VALUE_INT(VisualSettings, window_height)

    PARSE_VALUE_COLOR(VisualSettings, bkg_color)
    PARSE_VALUE_COLOR(VisualSettings, axis_x_color)
    PARSE_VALUE_COLOR(VisualSettings, axis_y_color)
    PARSE_VALUE_COLOR(VisualSettings, axis_z_color)
    PARSE_VALUE_COLOR(VisualSettings, comp_box_color)
    PARSE_VALUE_COLOR(VisualSettings, in_barrier_color)
    PARSE_VALUE_COLOR(VisualSettings, out_barrier_color)
    PARSE_VALUE_COLOR(VisualSettings, selection_color)

    PARSE_VALUE_COLOR(VisualSettings, cell_alive_color)
    PARSE_VALUE_COLOR(VisualSettings, cell_hypoxia_color)
    PARSE_VALUE_COLOR(VisualSettings, cell_apoptosis_color)
    PARSE_VALUE_COLOR(VisualSettings, cell_necrosis_color)
    PARSE_VALUE_COLOR(VisualSettings, tube_color)

    PARSE_VALUE_COLOR(VisualSettings, clip_plane_color)
    PARSE_VALUE_COLOR(VisualSettings, navigator_color)
    PARSE_VALUE_COLOR(VisualSettings, boxes_color)
    PARSE_VALUE_VECTOR(VisualSettings, light_dir)

  else
      throw new Error(__FILE__, __LINE__, "Unknown token in 'visual'", TokenToString(tv), ParserFile, ParserLine);
 }


void ParseSimulationSettingsValue(FILE *f)
/**
  Parses 'simulation' block.

  \param f -- input file
*/
 {
    anyToken tv;

    // store value name...
    tv = Token;

    // get '='...
    GetNextToken(f, false);
    if (Token.type != TT_Symbol || Token.symbol != '=')
        throw new Error(__FILE__, __LINE__, "Syntax error ('=' expected)", TokenToString(Token), ParserFile, ParserLine);

    // get value...
    GetNextToken(f, true);

    // assign value...
    if (0) ;
    PARSE_VALUE_INT(SimulationSettings, dimensions)
    PARSE_VALUE_INT(SimulationSettings, sim_phases)
    PARSE_VALUE_REAL(SimulationSettings, time_step)
    PARSE_VALUE_REAL(SimulationSettings, stop_time)
    PARSE_VALUE_REAL(SimulationSettings, time)
    PARSE_VALUE_VECTOR(SimulationSettings, comp_box_from)
    PARSE_VALUE_VECTOR(SimulationSettings, comp_box_to)
    PARSE_VALUE_REAL(SimulationSettings, box_size)
    PARSE_VALUE_INT(SimulationSettings, max_cells_per_box)
    PARSE_VALUE_REAL(SimulationSettings, force_r_cut)
    PARSE_VALUE_INT(SimulationSettings, max_tube_chains)
    PARSE_VALUE_INT(SimulationSettings, max_tube_merge)

    PARSE_VALUE_INT(SimulationSettings, save_statistics)
    PARSE_VALUE_INT(SimulationSettings, save_povray)
    PARSE_VALUE_INT(SimulationSettings, save_ag)

    PARSE_VALUE_INT(SimulationSettings, graph_sampling)

    PARSE_VALUE_REAL_N(SimulationSettings, diffusion_coeff[dsO2], diffusion_coeff_o2)
    PARSE_VALUE_REAL_N(SimulationSettings, diffusion_coeff[dsTAF], diffusion_coeff_taf)
    PARSE_VALUE_REAL_N(SimulationSettings, diffusion_coeff[dsPericytes], diffusion_coeff_pericytes)

    else
        throw new Error(__FILE__, __LINE__, "Unknown token in 'simulation'", TokenToString(tv), ParserFile, ParserLine);
 }


void ParseTubularSystemSettingsValue(FILE *f)
/**
  Parses 'TubularSystem' block.

  \param f -- input file
*/
 {
    anyToken tv;

    // store value name...
    tv = Token;

    // get '='...
    GetNextToken(f, false);
    if (Token.type != TT_Symbol || Token.symbol != '=')
        throw new Error(__FILE__, __LINE__, "Syntax error ('=' expected)", TokenToString(Token), ParserFile, ParserLine);

    // get value...
    GetNextToken(f, true);

    // assign value...
    if (0) ;
    PARSE_VALUE_REAL(TubularSystemSettings, force_chain_attr_factor)
    PARSE_VALUE_REAL(TubularSystemSettings, force_length_keep_factor)
    PARSE_VALUE_REAL(TubularSystemSettings, force_angle_factor)
    PARSE_VALUE_REAL(TubularSystemSettings, force_rep_factor)
    PARSE_VALUE_REAL(TubularSystemSettings, force_atr1_factor)
    PARSE_VALUE_REAL(TubularSystemSettings, force_atr2_factor)
    PARSE_VALUE_REAL(TubularSystemSettings, density)
    PARSE_VALUE_REAL(TubularSystemSettings, o2_production)
    PARSE_VALUE_REAL(TubularSystemSettings, lengthening_speed)
    PARSE_VALUE_REAL(TubularSystemSettings, thickening_speed)
    PARSE_VALUE_REAL(TubularSystemSettings, minimum_interphase_time)
    PARSE_VALUE_REAL(TubularSystemSettings, TAFtrigger)
    PARSE_VALUE_REAL(TubularSystemSettings, minimum_blood_flow)
    PARSE_VALUE_REAL(TubularSystemSettings, time_to_degradation)

    else
        throw new Error(__FILE__, __LINE__, "Unknown token in 'TubularSystem'", TokenToString(tv), ParserFile, ParserLine);
 }


void SaveTubularSystemSettings_ag(FILE *f, anyTubularSystemSettings const *vs)
/**
  Saves tubular network settings to *.ag file.

  \param f -- output file
  \param vs -- pointer to tubular settings
*/
{
    fprintf(f, "\n");
    fprintf(f, "TubularSystem\n");
    fprintf(f, " {\n");

    // save all fields...
    SAVE_REAL(f, vs, force_chain_attr_factor);
    SAVE_REAL(f, vs, force_length_keep_factor);
    SAVE_REAL(f, vs, force_angle_factor);
    SAVE_REAL(f, vs, force_rep_factor);
    SAVE_REAL(f, vs, force_atr1_factor);
    SAVE_REAL(f, vs, force_atr2_factor);
    SAVE_REAL(f, vs, density);
    SAVE_REAL(f, vs, lengthening_speed);
    SAVE_REAL(f, vs, thickening_speed);
    SAVE_REAL(f, vs, minimum_interphase_time);
    SAVE_REAL(f, vs, TAFtrigger);
    SAVE_REAL(f, vs, minimum_blood_flow);
    SAVE_REAL(f, vs, time_to_degradation);

    SAVE_REAL(f, vs, o2_production);

    fprintf(f, " }\n");
}


#ifdef QT_CORE_LIB
void displayGlobalsDialog()
/**
  Displays dialog with global settings.
*/
{
    anyGlobalsDialog *dialog = new anyGlobalsDialog;

    dialog->dialog->groupBox_Error->setVisible(false);
    dialog->prepare_dialog();
    dialog->exec();

    delete dialog;
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
    dialog->doubleSpinBox_diffcoefO2->setValue(SimulationSettings.diffusion_coeff[dsO2]);
    dialog->doubleSpinBox_diffcoefTAF->setValue(SimulationSettings.diffusion_coeff[dsTAF]);
    dialog->doubleSpinBox_diffcoefPeri->setValue(SimulationSettings.diffusion_coeff[dsPericytes]);
    dialog->doubleSpinBox_stop_time->setValue(SimulationSettings.stop_time);

    dialog->spinBox_savepovray->setValue(SimulationSettings.save_povray);
    dialog->spinBox_saveag->setValue(SimulationSettings.save_ag);
    dialog->spinBox_savestats->setValue(SimulationSettings.save_statistics);
    dialog->spinBox_graphrate->setValue(SimulationSettings.graph_sampling);

    dialog->checkBox_sim_diffusion->setChecked(SimulationSettings.sim_phases & spDiffusion);
    dialog->checkBox_sim_forces->setChecked(SimulationSettings.sim_phases & spForces);
    dialog->checkBox_sim_mitosis->setChecked(SimulationSettings.sim_phases & spMitosis);
    dialog->checkBox_sim_tube_div->setChecked(SimulationSettings.sim_phases & spTubeDiv);
    dialog->checkBox_sim_flow->setChecked(SimulationSettings.sim_phases & spBloodFlow);
    dialog->checkBox_sim_growth->setChecked(SimulationSettings.sim_phases & spGrow);

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
    SimulationSettings.diffusion_coeff[dsO2] = dialog->doubleSpinBox_diffcoefO2->value();
    SimulationSettings.diffusion_coeff[dsTAF] = dialog->doubleSpinBox_diffcoefTAF->value();
    SimulationSettings.diffusion_coeff[dsPericytes] = dialog->doubleSpinBox_diffcoefPeri->value();

    SimulationSettings.save_povray = dialog->spinBox_savepovray->value();
    SimulationSettings.save_ag = dialog->spinBox_saveag->value();
    SimulationSettings.save_statistics = dialog->spinBox_savestats->value();
    SimulationSettings.graph_sampling = dialog->spinBox_graphrate->value();

    SimulationSettings.sim_phases = dialog->checkBox_sim_diffusion->isChecked()*spDiffusion +
                                    dialog->checkBox_sim_forces->isChecked()*spForces +
                                    dialog->checkBox_sim_growth->isChecked()*spGrow +
                                    dialog->checkBox_sim_mitosis->isChecked()*spMitosis +
                                    dialog->checkBox_sim_tube_div->isChecked()*spTubeDiv +
                                    dialog->checkBox_sim_flow->isChecked()*spBloodFlow;
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


void SaveNeeded(bool needed)
{
    GlobalSettings.save_needed = needed;
    MainWindowPtr->set_save_needed(needed);
}
#endif
















