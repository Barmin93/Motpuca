#ifdef QT_CORE_LIB
#include <QColorDialog>
#endif

#include <stdio.h>

#include "config.h"
#include "log.h"
#include "parser.h"

#include "mainwindow.h"


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

    SAVE_float(f, ss, time_step);
    SAVE_float(f, ss, time);
    SAVE_float(f, ss, stop_time);
    SAVE_VECT(f, ss, comp_box_from);
    SAVE_VECT(f, ss, comp_box_to);
    SAVE_float(f, ss, box_size);
    SAVE_INT(f, ss, max_cells_per_box);
    SAVE_float(f, ss, force_r_cut);
    SAVE_float(f, ss, quiescent_o2);
    SAVE_float(f, ss, proliferative_o2);

    SAVE_INT(f, ss, max_tube_chains);
    SAVE_INT(f, ss, max_tube_merge);

    SAVE_INT(f, ss, save_statistics);
    SAVE_INT(f, ss, save_povray);
    SAVE_INT(f, ss, save_ag);

    SAVE_INT(f, ss, graph_sampling);

    SAVE_float_N(f, ss, diffusion_coeff[sat::dsO2], diffusion_coeff_O2);
    SAVE_float_N(f, ss, diffusion_coeff[sat::dsTAF], diffusion_coeff_TAF);
    SAVE_float_N(f, ss, diffusion_coeff[sat::dsPericytes], diffusion_coeff_Pericytes);
    SAVE_float_N(f, ss, diffusion_coeff[sat::dsMedicine], diffusion_coeff_Medicine);

    fprintf(f, " }\n");
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
    SAVE_float(f, vs, force_chain_attr_factor);
    SAVE_float(f, vs, force_length_keep_factor);
    SAVE_float(f, vs, force_angle_factor);
    SAVE_float(f, vs, force_rep_factor);
    SAVE_float(f, vs, force_atr1_factor);
    SAVE_float(f, vs, force_atr2_factor);
    SAVE_float(f, vs, density);
    SAVE_float(f, vs, lengthening_speed);
    SAVE_float(f, vs, thickening_speed);
    SAVE_float(f, vs, minimum_interphase_time);
    SAVE_float(f, vs, TAFtrigger);
    SAVE_float(f, vs, minimum_blood_flow);
    SAVE_float(f, vs, time_to_degradation);

    SAVE_float(f, vs, o2_production);

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



void SaveNeeded(bool needed)
{
    GlobalSettings.save_needed = needed;
    MainWindowPtr->set_save_needed(needed);
}


void ParseSimulationSettings(FILE *f)
/**
  Parses global simulation settings.

  \param f -- input file
*/
{
    // get '{'...
    GetNextToken(f, false);
    if (Token.type != TT_Symbol || Token.symbol != '{')
        throw new Error(__FILE__, __LINE__, "Bad block start ('{' expected)", TokenToString(Token), ParserFile, ParserLine);

    // parse...
    while (23)
    {
        GetNextToken(f, false);

        if (Token.type == TT_Ident)
            ParseSimulationSettingsValue(f);

        // end of 'tissue' body?...
        else if (Token.type == TT_Symbol && Token.symbol == '}')
            break;

        // end of input file?...
        else if (Token.type == TT_Eof)
            throw new Error(__FILE__, __LINE__, "Unexpected end of file", TokenToString(Token), ParserFile, ParserLine);

        else
            throw new Error(__FILE__, __LINE__, "Unexpected token (not string)", TokenToString(Token), ParserFile, ParserLine);
    }

    SimulationSettings.calculate_derived_values();
}


void ParseTubularSystemSettings(FILE *f)
/**
  Parses tubular system settings.

  \param f -- input file
*/
{
    // get '{'...
    GetNextToken(f, false);
    if (Token.type != TT_Symbol || Token.symbol != '{')
        throw new Error(__FILE__, __LINE__, "Bad block start ('{' expected)", TokenToString(Token), ParserFile, ParserLine);

    // parse...
    while (23)
    {
        GetNextToken(f, false);

        if (Token.type == TT_Ident)
            ParseTubularSystemSettingsValue(f);

        // end of 'TubularSystemSettings' body?...
        else if (Token.type == TT_Symbol && Token.symbol == '}')
            break;

        // end of input file?...
        else if (Token.type == TT_Eof)
            throw new Error(__FILE__, __LINE__, "Unexpected end of file", TokenToString(Token), ParserFile, ParserLine);

        else
            throw new Error(__FILE__, __LINE__, "Unexpected token (not string)", TokenToString(Token), ParserFile, ParserLine);
    }
}


void ParseVisualSettings(FILE *f)
/**
  Parses global visual settings.

  \param f -- input file
*/
{
    // get '{'...
    GetNextToken(f, false);
    if (Token.type != TT_Symbol || Token.symbol != '{')
        throw new Error(__FILE__, __LINE__, "Bad block start ('{' expected)", TokenToString(Token), ParserFile, ParserLine);

    // parse...
    while (23)
    {
        GetNextToken(f, false);

        if (Token.type == TT_Ident)
            ParseVisualSettingsValue(f);

        // end of 'tissue' body?...
        else if (Token.type == TT_Symbol && Token.symbol == '}')
            break;

        // end of input file?...
        else if (Token.type == TT_Eof)
            throw new Error(__FILE__, __LINE__, "Unexpected end of file", TokenToString(Token), ParserFile, ParserLine);

        else
            throw new Error(__FILE__, __LINE__, "Unexpected token (not string)", TokenToString(Token), ParserFile, ParserLine);
    }

    VisualSettings.comp_light_dir();
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
    PARSE_VALUE_float(SimulationSettings, time_step)
    PARSE_VALUE_float(SimulationSettings, stop_time)
    PARSE_VALUE_float(SimulationSettings, time)
    PARSE_VALUE_VECTOR(SimulationSettings, comp_box_from)
    PARSE_VALUE_VECTOR(SimulationSettings, comp_box_to)
    PARSE_VALUE_float(SimulationSettings, box_size)
    PARSE_VALUE_INT(SimulationSettings, max_cells_per_box)

    PARSE_VALUE_float(SimulationSettings, force_r_cut)
    PARSE_VALUE_float(SimulationSettings, quiescent_o2)
    PARSE_VALUE_float(SimulationSettings, proliferative_o2)

    PARSE_VALUE_INT(SimulationSettings, max_tube_chains)
    PARSE_VALUE_INT(SimulationSettings, max_tube_merge)

    PARSE_VALUE_INT(SimulationSettings, save_statistics)
    PARSE_VALUE_INT(SimulationSettings, save_povray)
    PARSE_VALUE_INT(SimulationSettings, save_ag)

    PARSE_VALUE_INT(SimulationSettings, graph_sampling)

    PARSE_VALUE_INT(SimulationSettings, add_medicine)
    PARSE_VALUE_INT(SimulationSettings, remove_medicine)

    PARSE_VALUE_float_N(SimulationSettings, diffusion_coeff[sat::dsO2], diffusion_coeff_o2)
    PARSE_VALUE_float_N(SimulationSettings, diffusion_coeff[sat::dsTAF], diffusion_coeff_taf)
    PARSE_VALUE_float_N(SimulationSettings, diffusion_coeff[sat::dsPericytes], diffusion_coeff_pericytes)
    PARSE_VALUE_float_N(SimulationSettings, diffusion_coeff[sat::dsMedicine], diffusion_coeff_medicine)

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
    PARSE_VALUE_float(TubularSystemSettings, force_chain_attr_factor)
    PARSE_VALUE_float(TubularSystemSettings, force_length_keep_factor)
    PARSE_VALUE_float(TubularSystemSettings, force_angle_factor)
    PARSE_VALUE_float(TubularSystemSettings, force_rep_factor)
    PARSE_VALUE_float(TubularSystemSettings, force_atr1_factor)
    PARSE_VALUE_float(TubularSystemSettings, force_atr2_factor)
    PARSE_VALUE_float(TubularSystemSettings, density)
    PARSE_VALUE_float(TubularSystemSettings, o2_production)
    PARSE_VALUE_float(TubularSystemSettings, lengthening_speed)
    PARSE_VALUE_float(TubularSystemSettings, thickening_speed)
    PARSE_VALUE_float(TubularSystemSettings, minimum_interphase_time)
    PARSE_VALUE_float(TubularSystemSettings, TAFtrigger)
    PARSE_VALUE_float(TubularSystemSettings, minimum_blood_flow)
    PARSE_VALUE_float(TubularSystemSettings, time_to_degradation)

    else
        throw new Error(__FILE__, __LINE__, "Unknown token in 'TubularSystem'", TokenToString(tv), ParserFile, ParserLine);
 }


#endif
















