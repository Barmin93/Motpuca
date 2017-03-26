#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#ifdef QT_CORE_LIB
#include <QtDebug>
#include <QColorDialog>
#include <QFileDialog>
#include <QDesktopServices>
#include <QMessageBox>
#endif

#include "const.h"
#include "log.h"
#include "parser.h"
#include "config.h"
#include "timers.h"

#ifdef QT_CORE_LIB
#include "mainwindow.h"
#endif

char const *CellState_names[] = { "-added-", "-removed-", "ALIVE", "HYPOXIA", "APOPTOSIS", "NECROSIS" };
char const *TissueType_names[] = { "NORMAL", "TUMOR" };
char const *BarrierType_names[] = { "KEEP_IN", "KEEP_OUT" };
char const *DiffundingSubstances_names[] = { "O2", "TAF", "Pericytes" };

anyBarrier *FirstBarrier = 0;
anyBarrier *LastBarrier = 0;

anyCellBlock *FirstCellBlock = 0;
anyCellBlock *LastCellBlock = 0;

anyTubeLine *FirstTubeLine = 0;
anyTubeLine *LastTubeLine = 0;

anyTubeBundle *FirstTubeBundle = 0;
anyTubeBundle *LastTubeBundle = 0;

anyTissueSettings *FirstTissueSettings = 0;
anyTissueSettings *LastTissueSettings = 0;
int NoTissueSettings = 0;

anyCell *Cells = 0;

anyTubeBox *BoxedTubes = 0;    ///< boxed tube array
anyTube **TubeChains = 0;      ///< tube chains array
int NoTubeChains = 0;          ///< no of tube chains
int NoTubes = 0;               ///< no of tubes
int LastTubeId = 0;            ///< id of last added tube

anyTubeMerge *TubelMerge = 0;  ///< array of tube pairs to merge after tip-tip collision
int NoTubeMerge = 0;           ///< number of tube pairs to merge

real ***Concentrations = 0;


void AddTissueSettings(anyTissueSettings *ts)
/**
  Creates new tissue settings and adds to linked list.
*/
{
    if (!FirstTissueSettings)
    {
        FirstTissueSettings = LastTissueSettings = ts;
        ts->id = 0;
    }
    else
    {
        ts->id = LastTissueSettings->id + 1;
        LastTissueSettings->next = ts;
        LastTissueSettings = ts;
    }
    NoTissueSettings++;
}


void RemoveTissueSettings(anyTissueSettings *ts)
/**
  Removes tissue settings from linked list.
*/
{
    // find tissue settings...
    anyTissueSettings *tsb = FirstTissueSettings;
    anyTissueSettings *tsp = 0;
    while (tsb)
    {
        if (tsb == ts)
        {
            // found...
            if (!tsp)
                FirstTissueSettings = (anyTissueSettings *)tsb->next;
            else
                tsp->next = tsb->next;

            if (LastTissueSettings == ts)
                LastTissueSettings = tsp;

            return;
        }
        tsp = tsb;
        tsb = (anyTissueSettings *)tsb->next;
    }
    NoTissueSettings--;
}


anyTissueSettings *FindTissueSettings(char const *name)
/**
  Finds tissue with given name.

  \param name -- name to search

  \returns pointer to tissue settings or 0
*/
{
    anyTissueSettings *ts = FirstTissueSettings;

    while (ts)
    {
        if (ts->name && !StrCmp(ts->name, name))
            return ts;
        ts = ts->next;
    }
    return 0;
}


void ParseTissueSettingsValue(FILE *f, anyTissueSettings *ts, bool check_name)
/**
  Parses 'tissue' block.

  \param f -- input file
  \param ts -- pointer to tissue settings
*/
{
    anyToken tv;

    // store value name...
    tv = Token;

    // get '='...
    GetNextToken(f, false);
    if (Token.type != TT_Symbol || Token.symbol != '=')
        throw new Error(__FILE__, Token.type, "Syntax error ('=' expected)", TokenToString(Token), ParserFile, ParserLine);

    // get value...
    GetNextToken(f, true);

    // assign value...
    if (!StrCmp(tv.str, "name"))
    {
        if (Token.type != TT_String)
            throw new Error(__FILE__, __LINE__, "Invalid tissue name", TokenToString(Token), ParserFile, ParserLine);
        if (check_name && FindTissueSettings(Token.str))
            throw new Error(__FILE__, __LINE__, "Duplicate tissue name", TokenToString(Token), ParserFile, ParserLine);

        ts->name = new char[strlen(Token.str) + 1];
        strcpy(ts->name, Token.str);
    }
    PARSE_VALUE_ENUM((*ts), TissueType, type)
    PARSE_VALUE_COLOR((*ts), color)
    PARSE_VALUE_REAL((*ts), cell_r)
    PARSE_VALUE_REAL((*ts), density)
    PARSE_VALUE_REAL((*ts), cell_grow_speed)
    PARSE_VALUE_REAL((*ts), minimum_interphase_time)
    PARSE_VALUE_REAL((*ts), time_to_apoptosis)
    PARSE_VALUE_REAL((*ts), time_to_necrosis)
    PARSE_VALUE_REAL((*ts), time_in_necrosis)
    PARSE_VALUE_REAL((*ts), dead_r)
    PARSE_VALUE_REAL((*ts), cell_shrink_speed)
    PARSE_VALUE_REAL((*ts), minimum_mitosis_r)
    PARSE_VALUE_REAL((*ts), force_rep_factor)
    PARSE_VALUE_REAL((*ts), force_atr1_factor)
    PARSE_VALUE_REAL((*ts), force_atr2_factor)
    PARSE_VALUE_REAL((*ts), force_dpd_factor)
    PARSE_VALUE_REAL((*ts), dpd_temperature)
    PARSE_VALUE_REAL((*ts), max_pressure)
    PARSE_VALUE_REAL((*ts), o2_consumption)
    PARSE_VALUE_REAL((*ts), o2_hypoxia)
    PARSE_VALUE_REAL((*ts), pericyte_production)
    PARSE_VALUE_REAL((*ts), time_to_necrosis_var)

  else
      throw new Error(__FILE__, __LINE__, "Unknown token in 'tissue'", TokenToString(tv), ParserFile, ParserLine);
}


void ParseTissueSettings(FILE *f, anyTissueSettings *ts, bool add_to_scene)
/**
  Parses tissue settings.

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
            ParseTissueSettingsValue(f, ts, add_to_scene);

        // end of 'tissue' body?...
        else if (Token.type == TT_Symbol && Token.symbol == '}')
            break;

        // end of input file?...
        else if (Token.type == TT_Eof)
            throw new Error(__FILE__, __LINE__, "Unexpected end of file", TokenToString(Token), ParserFile, ParserLine);

        else
            throw new Error(__FILE__, __LINE__, "Unexpected token (not string)", TokenToString(Token), ParserFile, ParserLine);
    }

    if (add_to_scene)
        AddTissueSettings(ts);
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


void SaveTissueSettings_ag(FILE *f, anyTissueSettings const *ts, bool save_header)
/**
  Saves tissue settings to *.ag file.

  \param f -- output file
  \param ts -- pointer to tissue settings
*/
{
    if (save_header)
        fprintf(f, "\nTissue\n");

    fprintf(f, " {\n");

    // counters...
    fprintf(f, "  // cells: %d\n", ts->no_cells[0]);

    // save all fields...
    if (ts->name && ts->name[0])
        SAVE_STRING(f, ts, name);
    SAVE_ENUM(f, ts, type, TissueType_names);
    SAVE_COLOR(f, ts, color);
    SAVE_REAL(f, ts, cell_r);
    SAVE_REAL(f, ts, density);
    SAVE_REAL(f, ts, cell_grow_speed);
    SAVE_REAL(f, ts, minimum_interphase_time);
    SAVE_REAL(f, ts, time_to_apoptosis);
    SAVE_REAL(f, ts, time_to_necrosis);
    SAVE_REAL(f, ts, time_in_necrosis);
    SAVE_REAL(f, ts, dead_r);
    SAVE_REAL(f, ts, cell_shrink_speed);
    SAVE_REAL(f, ts, minimum_mitosis_r);
    SAVE_REAL(f, ts, force_rep_factor);
    SAVE_REAL(f, ts, force_atr1_factor);
    SAVE_REAL(f, ts, force_atr2_factor);
    SAVE_REAL(f, ts, max_pressure);
    SAVE_REAL(f, ts, o2_consumption);
    SAVE_REAL(f, ts, o2_hypoxia);
    SAVE_REAL(f, ts, pericyte_production);
    SAVE_REAL(f, ts, time_to_necrosis_var);
    SAVE_REAL(f, ts, force_dpd_factor);
    SAVE_REAL(f, ts, dpd_temperature);

    fprintf(f, " }\n");
}


void SaveAllTissueSettings_ag(FILE *f)
/**
  Saves all tissue settings to *.ag file.

  \param f -- output file
*/
{
    anyTissueSettings *ts = FirstTissueSettings;

    while (ts)
    {
        SaveTissueSettings_ag(f, ts, true);
        ts = ts->next;
    }
}


void DeallocateTissueSettings()
/**
  Deallocates all tissue settings.
*/
{
    anyTissueSettings *ts = FirstTissueSettings, *tsn = 0;

    while (ts)
    {
        tsn = ts->next;
        delete ts;
        ts = tsn;
    }
    FirstTissueSettings = LastTissueSettings = 0;
    NoTissueSettings = 0;
}


void AddBarrier(anyBarrier *b)
/**
  Adds barrier to linked list.
*/
{
    if (!FirstBarrier)
        FirstBarrier = LastBarrier = b;
    else
    {
        LastBarrier->next = b;
        LastBarrier = b;
    }
}


void RemoveBarrier(anyBarrier *b)
/**
  Removes barrier from linked list.
*/
{
    // find barrier...
    anyBarrier *bb = FirstBarrier;
    anyBarrier *bp = 0;
    while (bb)
    {
        if (bb == b)
        {
            // found...
            if (!bp)
                FirstBarrier = (anyBarrier *)bb->next;
            else
                bp->next = bb->next;

            if (LastBarrier == b)
                LastBarrier = bp;

            return;
        }
        bp = bb;
        bb = (anyBarrier *)bb->next;
    }
}


void ParseBarrierValue(FILE *f, anyBarrier *b)
/**
  Parses 'barrier' block.

  \param f -- input file
  \param b -- pointer to barrier
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
    PARSE_VALUE_ENUM((*b), BarrierType, type)
    PARSE_VALUE_VECTOR((*b), from)
    PARSE_VALUE_VECTOR((*b), to)
    PARSE_VALUE_TRANSFORMATION((*b), trans)

  else
      throw new Error(__FILE__, __LINE__, "Unknown token in 'barrier'", TokenToString(tv), ParserFile, ParserLine);
}


void ParseBarrier(FILE *f, anyBarrier *b, bool add_to_scene)
/**
  Parses barrier settings.

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
            ParseBarrierValue(f, b);

        // end of 'barrier' body?...
        else if (Token.type == TT_Symbol && Token.symbol == '}')
            break;

        // end of input file?...
        else if (Token.type == TT_Eof)
            throw new Error(__FILE__, __LINE__, "Unexpected end of file", TokenToString(Token), ParserFile, ParserLine);

        else
            throw new Error(__FILE__, __LINE__, "Unexpected token (not string)", TokenToString(Token), ParserFile, ParserLine);
    }
    b->fix();

    if (add_to_scene)
        AddBarrier(b);
}


void SaveBarrier_ag(FILE *f, anyBarrier const *b)
/**
  Saves barrier to *.ag file.

  \param f -- output file
  \param b -- pointer to barrier
*/
{
    fprintf(f, "\n");
    fprintf(f, "Barrier\n");
    fprintf(f, " {\n");

    // save all fields...
    SAVE_ENUM(f, b, type, BarrierType_names);
    SAVE_VECT(f, b, from);
    SAVE_TRANSFORMATION(f, b, trans);
    SAVE_VECT(f, b, to);

    fprintf(f, " }\n");
}


void SaveAllBarriers_ag(FILE *f)
/**
  Saves all barriers to *.ag file.

  \param f -- output file
*/
{
    anyBarrier *b = FirstBarrier;

    while (b)
    {
        SaveBarrier_ag(f, b);
        b = (anyBarrier *)b->next;
    }
}


void DeallocateBarriers()
/**
  Deallocates all barriers.
*/
{
    anyBarrier *b = FirstBarrier, *bn = 0;

    while (b)
    {
        bn = (anyBarrier *)b->next;
        delete b;
        b = bn;
    }
    FirstBarrier = LastBarrier = 0;
}



void AllocSimulation()
/**
  Allocates memory for simulation.
*/
{
    LOG(llInfo, "Memory allocation");

    // calculate number of boxes...
    SimulationSettings.no_boxes_x = ceil((SimulationSettings.comp_box_to.x - SimulationSettings.comp_box_from.x)/SimulationSettings.box_size);
    SimulationSettings.no_boxes_y = ceil((SimulationSettings.comp_box_to.y - SimulationSettings.comp_box_from.y)/SimulationSettings.box_size);
    SimulationSettings.no_boxes_z = ceil((SimulationSettings.comp_box_to.z - SimulationSettings.comp_box_from.z)/SimulationSettings.box_size);
    SimulationSettings.no_boxes_xy = SimulationSettings.no_boxes_x*SimulationSettings.no_boxes_y;
    SimulationSettings.no_boxes = SimulationSettings.no_boxes_xy*SimulationSettings.no_boxes_z;

    // recalculate derived values...
    SimulationSettings.calculate_derived_values();

    // alloc Cells and TubeChains array...
    try
    {
        Cells = new anyCell[SimulationSettings.no_boxes*SimulationSettings.max_cells_per_box];

        TubeChains = new anyTube *[SimulationSettings.max_tube_chains];

        TubelMerge = new anyTubeMerge[SimulationSettings.max_tube_merge];
        NoTubeMerge = 0;

        BoxedTubes = new anyTubeBox[SimulationSettings.no_boxes];
        for (int i = 0; i < SimulationSettings.no_boxes; i++)
            BoxedTubes[i].tubes = new anyTube *[SimulationSettings.max_cells_per_box];

        Concentrations = new real**[2];
        for(int frame = 0; frame < 2; frame++)
        {
            Concentrations[frame] = new real*[dsLast];
            for (int i = 0; i < dsLast; i++)
            {
                Concentrations[frame][i] = new real[SimulationSettings.no_boxes];
                for (int j = 0; j < SimulationSettings.no_boxes; j++)
                {
                    Concentrations[frame][i][j]=0;
                }
            }
        }
    }
    catch (...)
    {
        throw new Error(__FILE__, __LINE__, "Memory allocation failed");
    }

    GlobalSettings.simulation_allocated = true;
}


void ReallocSimulation()
{
    LOG(llError, "Reallocation not implemented YET!");

    // workaround...
    DeallocSimulation();
    AllocSimulation();
}


void DeallocSimulation()
/**
  Deallocates simulation memory.
*/
{
    delete [] Cells;
    Cells = 0;

    for (int i = 0; i < NoTubeChains; i++)
    {
        anyTube *v = TubeChains[i], *nv;
        while (v)
        {
         nv = v->next;
         delete v;
         v = nv;
        }
    }
    delete [] TubeChains;
    delete [] TubelMerge;
    NoTubeMerge = 0;

    for (int i = 0; i < SimulationSettings.no_boxes; i++)
        delete [] BoxedTubes[i].tubes;
    delete [] BoxedTubes;
    TubeChains = 0;
    NoTubes = 0;
    NoTubeChains = 0;
    LastTubeId = 0;

    if (Concentrations != 0)
    {
        for (int frame = 0; frame < 2; frame++)
        {
            for (int i = 0; i < dsLast; i++)
            {
                delete[] Concentrations[frame][i];
                Concentrations[frame][i] = 0;
            }
            delete [] Concentrations[frame];
            Concentrations[frame] = 0;
        }
    }
    delete [] Concentrations;
    Concentrations = 0;

    GlobalSettings.simulation_allocated = false;
}


int GetBoxId(anyVector const pos)
/**
  Calculates box id for given point.

  \param pos -- point

  \returns box id or -1 if outside computational box
*/
{
    int box_x = floor((pos.x - SimulationSettings.comp_box_from.x)/SimulationSettings.box_size);
    int box_y = floor((pos.y - SimulationSettings.comp_box_from.y)/SimulationSettings.box_size);
    int box_z = floor((pos.z - SimulationSettings.comp_box_from.z)/SimulationSettings.box_size);

    if (!VALID_BOX(box_x, box_y, box_z))
        return -1;

    return BOX_ID(box_x, box_y, box_z);
}


void AddCell(anyCell *c)
/**
  Adds cell to Cells array.

  \param c -- pointer to cell to add
*/
{
    int box_no = GetBoxId(c->pos);

    if (box_no == -1)
        return;

    // current number of cells in box...
    int no_cells = Cells[box_no*SimulationSettings.max_cells_per_box].no_cells_in_box;
    if (no_cells >= SimulationSettings.max_cells_per_box)
    {
        LOG(llError, "Too many cells in box");
        //        throw new Error(__FILE__, __LINE__, "Too many cells in box");
//        if (c->tissue->type == ttNormal)
//            qDebug("%s cannot be added! %s", TissueType_names[c->tissue->type], c->pos.to_string());

        return;
    }

    // add cell..
    Cells[box_no*SimulationSettings.max_cells_per_box + no_cells] = *c;
    Cells[box_no*SimulationSettings.max_cells_per_box].no_cells_in_box = no_cells + 1;
    c->tissue->no_cells[0]++;
}


void AddTube(anyTube *v, bool attach_to_previous, bool start_new_chain)
/**
  Adds tube to scene.

  \param v -- pointer to tube to add
  \param attach_to_previous -- attach to previously added tube (chain creation)?
*/
{
    static anyTube *pv = 0;

    if (start_new_chain || !pv)
    {
        if (NoTubeChains >= SimulationSettings.max_tube_chains)
            throw new Error(__FILE__, __LINE__, "Too many tube chains");
        TubeChains[NoTubeChains++] = v;
        v->next = v->prev = 0;
    }
    else if (attach_to_previous && pv)
    {
        pv->next = v;
        v->prev = pv;
        v->next = 0;
    }


    pv = v;
    v->id = ++LastTubeId;
    NoTubes++;
}


bool TubesJoined(anyTube *v1, anyTube *v2)
{
    return v1->next == v2 || v2->next == v1 || v1->fork == v2 || v1->base == v2 || v1->top == v2 || v1->jab == v2;
}


void SetCellMass(anyCell *c)
/**
  Sets 1/mass of cell.

  \param c -- pointer to cell
*/
{
    // conversion of density from [kg/m^3] to [kg/um^3] gives 1e-18...
    c->one_by_mass = 1 / ((4*M_PI/3)*(c->r*c->r*c->r)*c->tissue->density*1e-18);
    // WRONG:    c->one_by_mass = 1 / ((4*M_PI/3)*(c->tissue->cell_r*c->tissue->cell_r*c->tissue->cell_r)*c->tissue->density*1e-18);
}


void ParseCellValue(FILE *f, anyCell *c)
/**
  Parses 'cell' block.

  \param f -- input file
  \param c -- pointer to cell
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
    if (!StrCmp(tv.str, "tissue"))
    {
        if (Token.type != TT_String)
            throw new Error(__FILE__, __LINE__, "Invalid tissue name", TokenToString(Token), ParserFile, ParserLine);
        anyTissueSettings *ts = FindTissueSettings(Token.str);
        if (!ts)
            throw new Error(__FILE__, __LINE__, "Unknown tissue name in 'cell'", TokenToString(Token), ParserFile, ParserLine);
        c->tissue = ts;
    }
    else if (!StrCmp(tv.str, "conc_O2"))
    {
        if (Token.type != TT_Number)
            throw new Error(__FILE__, __LINE__, "Invalid concentration value", TokenToString(Token), ParserFile, ParserLine);
        if (Token.number < 0 || Token.number > 1)
            throw new Error(__FILE__, __LINE__, "Invalid concentration value", TokenToString(Token), ParserFile, ParserLine);
        c->concentrations[dsO2][0] = c->concentrations[dsO2][1] = Token.number;
    }
    else if (!StrCmp(tv.str, "conc_TAF"))
    {
        if (Token.type != TT_Number)
            throw new Error(__FILE__, __LINE__, "Invalid concentration value", TokenToString(Token), ParserFile, ParserLine);
        if (Token.number < 0 || Token.number > 1)
            throw new Error(__FILE__, __LINE__, "Invalid concentration value", TokenToString(Token), ParserFile, ParserLine);
        c->concentrations[dsTAF][0] = c->concentrations[dsTAF][1] = Token.number;
    }
    else if (!StrCmp(tv.str, "conc_Pericytes"))
    {
        if (Token.type != TT_Number)
            throw new Error(__FILE__, __LINE__, "Invalid concentration value", TokenToString(Token), ParserFile, ParserLine);
        if (Token.number < 0 || Token.number > 1)
            throw new Error(__FILE__, __LINE__, "Invalid concentration value", TokenToString(Token), ParserFile, ParserLine);
        c->concentrations[dsPericytes][0] = c->concentrations[dsPericytes][1] = Token.number;
    }
    PARSE_VALUE_VECTOR((*c), pos)
    PARSE_VALUE_REAL((*c), r)
    PARSE_VALUE_REAL((*c), age)
    PARSE_VALUE_ENUM((*c), CellState, state)
    PARSE_VALUE_REAL((*c), state_age)
    PARSE_VALUE_REAL((*c), time_to_necrosis)

  else
      throw new Error(__FILE__, __LINE__, "Unknown token in 'cell'", TokenToString(tv), ParserFile, ParserLine);
}


void ParseCell(FILE *f)
/**
  Parses cell.

  \param f -- input file
*/
{
    // get '{'...
    GetNextToken(f, false);
    if (Token.type != TT_Symbol || Token.symbol != '{')
        throw new Error(__FILE__, __LINE__, "Bad block start ('{' expected)", TokenToString(Token), ParserFile, ParserLine);

    static anyCell c;

    // parse...
    while (23)
    {
        GetNextToken(f, false);

        if (Token.type == TT_Ident)
            ParseCellValue(f, &c);

        // end of 'cell' body?...
        else if (Token.type == TT_Symbol && Token.symbol == '}')
            break;

        // end of input file?...
        else if (Token.type == TT_Eof)
            throw new Error(__FILE__, __LINE__, "Unexpected end of file", TokenToString(Token), ParserFile, ParserLine);

        else
            throw new Error(__FILE__, __LINE__, "Unexpected token (not string)", TokenToString(Token), ParserFile, ParserLine);
    }
    if (c.r == 0)
        c.r = c.tissue->cell_r;
    SetCellMass(&c);

    if (!GlobalSettings.simulation_allocated)
        AllocSimulation();

    AddCell(&c);
}


void SaveCell_ag(FILE *f, anyCell *c)
{
    fprintf(f, "\n");
    fprintf(f, "Cell\n");
    fprintf(f, " {\n");

    fprintf(f, "  // mass = %g\n", 1/c->one_by_mass);

    fprintf(f, "  tissue = \"%s\"\n", c->tissue->name);
    SAVE_ENUM(f, c, state, CellState_names);
    SAVE_VECT(f, c, pos);
    SAVE_REAL(f, c, r);
    SAVE_REAL(f, c, age);
    SAVE_REAL(f, c, state_age);
    SAVE_REAL(f, c, time_to_necrosis);

    fprintf(f, "  conc_O2 = %g\n", c->concentrations[dsO2][SimulationSettings.step % 2]);
    fprintf(f, "  conc_TAF = %g\n", c->concentrations[dsTAF][SimulationSettings.step % 2]);
    fprintf(f, "  conc_Pericytes = %g\n", c->concentrations[dsPericytes][SimulationSettings.step % 2]);

    fprintf(f, " }\n");
}


void SaveAllCells_ag(FILE *f)
{
    int first_cell = 0;
    for (int box_id = 0; box_id < SimulationSettings.no_boxes; box_id++)
    {
        int no_cells = Cells[first_cell].no_cells_in_box;
        for (int i = 0; i < no_cells; i++)
            // save only active cells...
            if (Cells[first_cell + i].state > csRemoved)
                SaveCell_ag(f, Cells + first_cell + i);

        first_cell += SimulationSettings.max_cells_per_box;
    }
}


void AddCellBlock(anyCellBlock *b)
/**
 Adds block of cells to linked list.

 \param b -- pointer to cell block
*/
{
    if (!FirstCellBlock)
        FirstCellBlock = LastCellBlock = b;
    else
    {
        LastCellBlock->next = b;
        LastCellBlock = b;
    }
}


void RemoveCellBlock(anyCellBlock *cb)
/**
  Removes cell block from linked list.
*/
{
    // find cell block...
    anyCellBlock *cbb = FirstCellBlock;
    anyCellBlock *cbp = 0;
    while (cbb)
    {
        if (cbb == cb)
        {
            // found...
            if (!cbp)
                FirstCellBlock = (anyCellBlock *)cbb->next;
            else
                cbp->next = cbb->next;

            if (LastCellBlock == cb)
                LastCellBlock = cbp;

            return;
        }
        cbp = cbb;
        cbb = (anyCellBlock *)cbb->next;
    }
}



void ParseCellBlockValue(FILE *f, anyCellBlock *b)
/**
  Parses 'block' value.

  \param f -- input file
  \param b -- pointer to cell block
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
    if (!StrCmp(tv.str, "tissue"))
    {
        if (Token.type != TT_String)
            throw new Error(__FILE__, __LINE__, "Invalid tissue name", TokenToString(Token), ParserFile, ParserLine);
        anyTissueSettings *ts = FindTissueSettings(Token.str);
        if (!ts)
            throw new Error(__FILE__, __LINE__, "Unknown tissue name in 'block'", TokenToString(Token), ParserFile, ParserLine);
        b->tissue = ts;
    }
    else if (!StrCmp(tv.str, "conc_O2"))
    {
        if (Token.type != TT_Number)
            throw new Error(__FILE__, __LINE__, "Invalid concentration value", TokenToString(Token), ParserFile, ParserLine);
        if (Token.number < 0 || Token.number > 1)
            throw new Error(__FILE__, __LINE__, "Invalid concentration value", TokenToString(Token), ParserFile, ParserLine);
        b->concentrations[dsO2] = Token.number;
    }
    else if (!StrCmp(tv.str, "conc_TAF"))
    {
        if (Token.type != TT_Number)
            throw new Error(__FILE__, __LINE__, "Invalid concentration value", TokenToString(Token), ParserFile, ParserLine);
        if (Token.number < 0 || Token.number > 1)
            throw new Error(__FILE__, __LINE__, "Invalid concentration value", TokenToString(Token), ParserFile, ParserLine);
        b->concentrations[dsTAF] = Token.number;
    }
    else if (!StrCmp(tv.str, "conc_Pericytes"))
    {
        if (Token.type != TT_Number)
            throw new Error(__FILE__, __LINE__, "Invalid concentration value", TokenToString(Token), ParserFile, ParserLine);
        if (Token.number < 0 || Token.number > 1)
            throw new Error(__FILE__, __LINE__, "Invalid concentration value", TokenToString(Token), ParserFile, ParserLine);
        b->concentrations[dsPericytes] = Token.number;
    }
    PARSE_VALUE_VECTOR((*b), from)
    PARSE_VALUE_VECTOR((*b), to)
    PARSE_VALUE_TRANSFORMATION((*b), trans)
    PARSE_VALUE_INT((*b), generated)

  else
      throw new Error(__FILE__, __LINE__, "Unknown token in 'block'", TokenToString(tv), ParserFile, ParserLine);
}


void ParseCellBlock(FILE *f, anyCellBlock *b, bool add_to_scene)
/**
  Parses block of cells settings.

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
            ParseCellBlockValue(f, b);

        // end of 'block' body?...
        else if (Token.type == TT_Symbol && Token.symbol == '}')
            break;

        // end of input file?...
        else if (Token.type == TT_Eof)
            throw new Error(__FILE__, __LINE__, "Unexpected end of file", TokenToString(Token), ParserFile, ParserLine);

        else
            throw new Error(__FILE__, __LINE__, "Unexpected token (not string)", TokenToString(Token), ParserFile, ParserLine);
    }
    b->fix();

    if (add_to_scene)
        AddCellBlock(b);
}


void SaveCellBlock_ag(FILE *f, anyCellBlock const *b)
/**
  Saves block of cells to *.ag file.

  \param f -- output file
  \param b -- pointer to cell block
*/
{
    fprintf(f, "\n");
    fprintf(f, "CellBlock\n");
    fprintf(f, " {\n");

    // save all fields...
    fprintf(f, "  tissue = \"%s\"\n", b->tissue->name);
    SAVE_INT(f, b, generated);
    SAVE_TRANSFORMATION(f, b, trans);
    SAVE_VECT(f, b, from);
    SAVE_VECT(f, b, to);
    fprintf(f, "  conc_o2 = %g\n", b->concentrations[dsO2]);
    fprintf(f, "  conc_taf = %g\n", b->concentrations[dsTAF]);
    fprintf(f, "  conc_pericytes = %g\n", b->concentrations[dsPericytes]);

    fprintf(f, " }\n");
}


void SaveAllCellBlocks_ag(FILE *f)
/**
  Saves all blocks of cells to *.ag file.

  \param f -- output file
*/
{
    anyCellBlock *b = FirstCellBlock;

    while (b)
    {
        SaveCellBlock_ag(f, b);
        b = (anyCellBlock *)b->next;
    }
}


void AddTubeLine(anyTubeLine *vl)
/**
 Adds tube line to linked list.

 \param vl -- pointer to tube line
*/
{
    if (!FirstTubeLine)
        FirstTubeLine = LastTubeLine = vl;
    else
    {
        LastTubeLine->next = vl;
        LastTubeLine = vl;
    }
}


void RemoveTubeLine(anyTubeLine *vl)
/**
  Removes tube line from linked list.
*/
{
    // find tube line...
    anyTubeLine *vlb = FirstTubeLine;
    anyTubeLine *vlp = 0;
    while (vlb)
    {
        if (vlb == vl)
        {
            // found...
            if (!vlp)
                FirstTubeLine = (anyTubeLine *)vlb->next;
            else
                vlp->next = vlb->next;

            if (LastTubeLine == vl)
                LastTubeLine = vlp;

            return;
        }
        vlp = vlb;
        vlb = (anyTubeLine *)vlb->next;
    }
}


void ParseTubeLineValue(FILE *f, anyTubeLine *vl)
/**
  Parses 'tubeLine' value.

  \param f -- input file
  \param vl -- pointer to tube line
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
    if (false) ;
    PARSE_VALUE_VECTOR((*vl), from)
    PARSE_VALUE_VECTOR((*vl), to)
    PARSE_VALUE_TRANSFORMATION((*vl), trans)
    PARSE_VALUE_REAL((*vl), tube_length)
    PARSE_VALUE_REAL((*vl), r)
    PARSE_VALUE_REAL((*vl), min_blood_pressure)
    PARSE_VALUE_REAL((*vl), max_blood_pressure)
    PARSE_VALUE_BOOL((*vl), fixed_blood_pressure)
    PARSE_VALUE_INT((*vl), generated)

    else
      throw new Error(__FILE__, __LINE__, "Unknown token in 'tubeLine'", TokenToString(tv), ParserFile, ParserLine);
}


void ParseTubeLine(FILE *f, anyTubeLine *vl, bool add_to_scene)
/**
  Parses tube line settings.

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
            ParseTubeLineValue(f, vl);

        // end of 'tubeLine' body?...
        else if (Token.type == TT_Symbol && Token.symbol == '}')
            break;

        // end of input file?...
        else if (Token.type == TT_Eof)
            throw new Error(__FILE__, __LINE__, "Unexpected end of file", TokenToString(Token), ParserFile, ParserLine);

        else
            throw new Error(__FILE__, __LINE__, "Unexpected token (not string)", TokenToString(Token), ParserFile, ParserLine);
    }

    if (add_to_scene)
        AddTubeLine(vl);
}


void SaveTubeLine_ag(FILE *f, anyTubeLine const *vl)
/**
  Saves tube line to *.ag file.

  \param f -- output file
  \param vl -- pointer to tube line
*/
{
    fprintf(f, "\n");
    fprintf(f, "TubeLine\n");
    fprintf(f, " {\n");

    // save all fields...
    SAVE_VECT(f, vl, from);
    SAVE_VECT(f, vl, to);
    SAVE_TRANSFORMATION(f, vl, trans);
    SAVE_REAL(f, vl, tube_length);
    SAVE_REAL(f, vl, r);

    SAVE_REAL(f, vl, min_blood_pressure);
    SAVE_REAL(f, vl, max_blood_pressure);
    SAVE_INT(f, vl, fixed_blood_pressure);

    SAVE_INT(f, vl, generated);

    fprintf(f, " }\n");
}


void SaveAllTubeLines_ag(FILE *f)
/**
  Saves all tube lines to *.ag file.

  \param f -- output file
*/
{
    anyTubeLine *vl = FirstTubeLine;

    while (vl)
    {
        SaveTubeLine_ag(f, vl);
        vl = (anyTubeLine *)vl->next;
    }
}


void DeallocateTubeLines()
/**
  Deallocates all tube lines.
*/
{
    anyTubeLine *vl = FirstTubeLine, *vn = 0;

    while (vl)
    {
        vn = (anyTubeLine *)vl->next;
        delete vl;
        vl = vn;
    }
    FirstTubeLine = LastTubeLine = 0;
}



void AddTubeBundle(anyTubeBundle *vb)
/**
 Adds tube bundle to linked list.

 \param vb -- pointer to tube bundle
*/
{
    if (!FirstTubeBundle)
        FirstTubeBundle = LastTubeBundle = vb;
    else
    {
        LastTubeBundle->next = vb;
        LastTubeBundle = vb;
    }
}


void RemoveTubeBundle(anyTubeBundle *vb)
/**
  Removes tube bundle from linked list.
*/
{
    // find tube bundle...
    anyTubeBundle *vbb = FirstTubeBundle;
    anyTubeBundle *vbp = 0;
    while (vbb)
    {
        if (vbb == vb)
        {
            // found...
            if (!vbp)
                FirstTubeBundle = (anyTubeBundle *)vbb->next;
            else
                vbp->next = vbb->next;

            if (LastTubeBundle == vb)
                LastTubeBundle = vbp;

            return;
        }
        vbp = vbb;
        vbb = (anyTubeBundle *)vbb->next;
    }
}


void ParseTubeBundleValue(FILE *f, anyTubeBundle *vb)
/**
  Parses 'tubeBundle' value.

  \param f -- input file
  \param vb -- pointer to tube bundle
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
    if (false) ;
    PARSE_VALUE_VECTOR((*vb), from)
    PARSE_VALUE_VECTOR((*vb), to)
    PARSE_VALUE_TRANSFORMATION((*vb), trans)
    PARSE_VALUE_REAL((*vb), r)
    PARSE_VALUE_REAL((*vb), tube_length)
    PARSE_VALUE_REAL((*vb), extent_x)
    PARSE_VALUE_REAL((*vb), spacing_y)
    PARSE_VALUE_REAL((*vb), spacing_z)
    PARSE_VALUE_REAL((*vb), shift_y)
    PARSE_VALUE_REAL((*vb), shift_z)
    PARSE_VALUE_REAL((*vb), min_blood_pressure)
    PARSE_VALUE_REAL((*vb), max_blood_pressure)
    PARSE_VALUE_BOOL((*vb), fixed_blood_pressure)
    PARSE_VALUE_INT((*vb), generated)

  else
      throw new Error(__FILE__, __LINE__, "Unknown token in 'tubeBundle'", TokenToString(tv), ParserFile, ParserLine);
}


void ParseTubeBundle(FILE *f, anyTubeBundle *vb, bool add_to_scene)
/**
  Parses tube block settings.

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
            ParseTubeBundleValue(f, vb);

        // end of 'tubeBunde' body?...
        else if (Token.type == TT_Symbol && Token.symbol == '}')
            break;

        // end of input file?...
        else if (Token.type == TT_Eof)
            throw new Error(__FILE__, __LINE__, "Unexpected end of file", TokenToString(Token), ParserFile, ParserLine);

        else
            throw new Error(__FILE__, __LINE__, "Unexpected token (not string)", TokenToString(Token), ParserFile, ParserLine);
    }

    if (add_to_scene)
        AddTubeBundle(vb);
}


void SaveTubeBundle_ag(FILE *f, anyTubeBundle const *vb)
/**
  Saves tube bundle to *.ag file.

  \param f -- output file
  \param vb -- pointer to tube bundle
*/
{
    fprintf(f, "\n");
    fprintf(f, "TubeBundle\n");
    fprintf(f, " {\n");

    // save all fields...
    SAVE_VECT(f, vb, from);
    SAVE_VECT(f, vb, to);
    SAVE_TRANSFORMATION(f, vb, trans);
    SAVE_REAL(f, vb, extent_x);
    SAVE_REAL(f, vb, spacing_y);
    SAVE_REAL(f, vb, spacing_z);
    SAVE_REAL(f, vb, shift_y);
    SAVE_REAL(f, vb, shift_z);
    SAVE_REAL(f, vb, tube_length);
    SAVE_REAL(f, vb, r);

    SAVE_REAL(f, vb, min_blood_pressure);
    SAVE_REAL(f, vb, max_blood_pressure);
    SAVE_INT(f, vb, fixed_blood_pressure);

    SAVE_INT(f, vb, generated);

    fprintf(f, " }\n");
}


void SaveAllTubeBundles_ag(FILE *f)
/**
  Saves all tube bundles to *.ag file.

  \param f -- output file
*/
{
    anyTubeBundle *vb = FirstTubeBundle;

    while (vb)
    {
        SaveTubeBundle_ag(f, vb);
        vb = (anyTubeBundle *)vb->next;
    }
}


void DeallocateTubeBundles()
/**
  Deallocates all tube bundles.
*/
{
    anyTubeBundle *vb = FirstTubeBundle, *vn = 0;

    while (vb)
    {
        vn = (anyTubeBundle *)vb->next;
        delete vb;
        vb = vn;
    }
    FirstTubeBundle = LastTubeBundle = 0;
}



void SetTubeMass(anyTube *v)
/**
  Sets 1/mass of tube.

  \param v -- pointer to tube
*/
{
    real len = v->final_length;
//    real len = (v->pos2 - v->pos1).length();
    // conversion of density from [kg/m^3] to [kg/um^3] gives 1e-18...
    v->one_by_mass = 1 / (M_PI*len*v->final_r*v->final_r*TubularSystemSettings.density*1e-18);
}


void ParseTubeValue(FILE *f, anyTube *v)
/**
  Parses 'tube' values.

  \param f -- input file
  \param v -- pointer to tube
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
    if (false) ;
    PARSE_VALUE_VECTOR((*v), pos1)
    PARSE_VALUE_VECTOR((*v), pos2)
    PARSE_VALUE_REAL((*v), r)
    PARSE_VALUE_REAL((*v), final_r)
    PARSE_VALUE_REAL((*v), length)
    PARSE_VALUE_REAL((*v), final_length)
    PARSE_VALUE_ENUM((*v), CellState, state)
    PARSE_VALUE_REAL((*v), age)
    PARSE_VALUE_REAL((*v), state_age)
    PARSE_VALUE_INT((*v),  base_id)
    PARSE_VALUE_INT((*v),  top_id)

    PARSE_VALUE_REAL((*v), blood_pressure)
    PARSE_VALUE_BOOL((*v), fixed_blood_pressure)

    else if (!StrCmp(tv.str, "id"))
    {
        if (Token.type == TT_Number)
            v->parsed_id = Token.number;
        else
            throw new Error(__FILE__, __LINE__, "Syntax error (number expected)", TokenToString(Token), ParserFile, ParserLine);
    }

    else
        throw new Error(__FILE__, __LINE__, "Unknown token in 'tube'", TokenToString(tv), ParserFile, ParserLine);
}


void ParseTube(FILE *f)
/**
  Parses tube.

  \param f -- input file
*/
{
    // get '{'...
    GetNextToken(f, false);
    if (Token.type != TT_Symbol || Token.symbol != '{')
        throw new Error(__FILE__, __LINE__, "Bad block start ('{' expected)", TokenToString(Token), ParserFile, ParserLine);

    anyTube *v = new anyTube;
    v->final_r = 0;
    bool first_in_chain = false;

    // parse...
    while (23)
    {
        GetNextToken(f, false);

        if (Token.type == TT_Ident)
        {
            if (!StrCmp(Token.str, "first"))
                first_in_chain = true;
            else
                ParseTubeValue(f, v);
        }

        // end of 'tube' body?...
        else if (Token.type == TT_Symbol && Token.symbol == '}')
            break;

        // end of input file?...
        else if (Token.type == TT_Eof)
            throw new Error(__FILE__, __LINE__, "Unexpected end of file", TokenToString(Token), ParserFile, ParserLine);

        else
            throw new Error(__FILE__, __LINE__, "Unexpected token (not string)", TokenToString(Token), ParserFile, ParserLine);
    }
    if (v->length == 0)
        v->length = (v->pos2 - v->pos1).length();
    if (v->final_length == 0)
        v->final_length = v->length;
    if (v->final_r == 0)
        v->final_r = v->r;

    SetTubeMass(v);

    if (!GlobalSettings.simulation_allocated)
        AllocSimulation();

    AddTube(v, !first_in_chain, first_in_chain);
}


void SaveTube_ag(FILE *f, anyTube *v)
/**
  Saves tube to *.ag file.

  \param f -- output file
  \param v -- pointer to tube
*/
{
    fprintf(f, "\n");
    fprintf(f, "Tube\n");
    fprintf(f, " {\n");
    fprintf(f, "  id = %d\n", v->id);
    if (!v->prev)
        fprintf(f, "  first\n");
    if (v->base)
        fprintf(f, "  base_id = %d\n", v->base->id);
    if (v->top)
        fprintf(f, "  top_id = %d\n", v->top->id);
    SAVE_ENUM(f, v, state, CellState_names);
    SAVE_VECT(f, v, pos1);
    SAVE_VECT(f, v, pos2);
    SAVE_REAL(f, v, length);
    if (v->length != v->final_length)
        SAVE_REAL(f, v, final_length);
    fprintf(f, "  // current length = %g\n", (v->pos2 - v->pos1).length());
    SAVE_REAL(f, v, r);
    if (v->r != v->final_r)
        SAVE_REAL(f, v, final_r);
    SAVE_REAL(f, v, age);
    SAVE_REAL(f, v, state_age);

    SAVE_REAL(f, v, blood_pressure);
    SAVE_INT(f, v, fixed_blood_pressure);

    fprintf(f, " }\n");
}


void SaveAllTubes_ag(FILE *f)
/**
  Saves all tubes to *.ag file.

  \param f -- output file
*/
{
    for (int i = 0; i < NoTubeChains; i++)
    {
        anyTube *v = TubeChains[i];
        while (v)
        {
            SaveTube_ag(f, v);
            v = v->next;
        }
    }
}


void DeallocateCellBlocks()
/**
  Deallocates all blocks of cells.
*/
{
    anyCellBlock *b = FirstCellBlock, *bn = 0;

    while (b)
    {
        bn = (anyCellBlock *)b->next;
        delete b;
        b = bn;
    }
    FirstCellBlock = LastCellBlock = 0;
}


static
real cell_tube_dist(anyCell const *c, anyTube const *v)
/**
  Calculates distance between cell and tube.

  \param c -- pointer to cell
  \param v -- pointer to tube
*/
{
    anyVector p12 = v->pos2 - v->pos1;
    anyVector pc = c->pos - v->pos1;
    real p12_len = p12.length();
    anyVector p12_n = p12;
    p12_n.normalize();

    real d = pc|p12_n;

    real p;

    if (d <= 0)
    {
        // near p1...
        p = 0;
    }
    else if (d >= p12_len)
    {
        // near p2...
        p = 1;
    }
    else
    {
        // between p1 and p2...
        p = d/p12_len;
    }

    return (c->pos - (v->pos1*(1 - p) + v->pos2*p)).length();
}


void GenerateCellsInBlock(anyCellBlock *b)
/**
  Generates all cells in block.
*/
{
    LOG3(llDebug, "Generating cells for tissue '", b->tissue->name, "'");

    anyCell c;
    real r = b->tissue->cell_r;
    real r_pack = r*0.9;

    c.r = r;
    c.tissue = b->tissue;

    for (int i = 0; i < dsLast; i++)
        c.concentrations[i][0] = c.concentrations[i][1] = b->concentrations[i];

    SetCellMass(&c);

    if (SimulationSettings.dimensions == 3)
    {
        // 3D...
        real x_shift = 0;
        real y_shift = r_pack;
        real y_shift_z = 0;
        real dz = 1.6329931618554520654648560498039*r_pack; // 2 * (sqrt(6)/3)
        real dy = 2*r_pack;
        real dx = 1.7320508075688772935274463415059*r_pack;  // 2 * (sqrt(3)/2)

        int x_cnt = floor((b->to.x - b->from.x - 2*r_pack)/dx);
        int y_cnt = floor((b->to.y - b->from.y - 2*r_pack)/dy);
        int z_cnt = floor((b->to.z - b->from.z - 2*r_pack)/dz);

        // d* correction...
        dx = (b->to.x - b->from.x - 2*r_pack)/x_cnt;
        dy = (b->to.y - b->from.y - 2*r_pack)/y_cnt;
        dz = (b->to.z - b->from.z - 2*r_pack)/z_cnt;

        for(real z = b->from.z + r; z <= b->to.z; z += dz)
        {
            for(real x = b->from.x + r + x_shift; x < b->to.x; x += dx)
            {
                for(real y = b->from.y + r + y_shift + y_shift_z; y < b->to.y; y += dy)
                {
                    c.pos = b->trans*anyVector(x, y, z);
                    c.pos += anyVector(float(rand())/RAND_MAX*c.r - c.r*0.5,
                                       float(rand())/RAND_MAX*c.r - c.r*0.5,
                                       0)*0.5;

                    c.age = real(rand())/real(RAND_MAX) * b->tissue->minimum_interphase_time;
                    c.state_age = c.age;

                    c.time_to_necrosis = b->tissue->time_to_necrosis + (2*double(rand())/double(RAND_MAX) - 1.0)*b->tissue->time_to_necrosis_var;

                    bool add = true;

                    // barriers...
                    anyBarrier *br = FirstBarrier;
                    while (add && br)
                    {
                        if (!br->is_point_inside(c.pos, c.r))
                            add = false;
                        br = (anyBarrier *)br->next;
                    }

                    // blocks...
                    anyCellBlock *bc = FirstCellBlock;
                    while (add && bc)
                    {
                        if (bc->generated
                            && bc->is_point_inside(c.pos, c.r*0.5))
                            add = false;
                        bc = (anyCellBlock *)bc->next;
                    }

                    // tubes...
                    for (int i = 0; add && i < NoTubeChains; i++)
                    {
                        anyTube *v = TubeChains[i];
                        while (v && add)
                        {
                            if (cell_tube_dist(&c, v) < c.r)
                                add = false;
                            v = v->next;
                        }
                    }

                    if (add)
                        AddCell(&c);
                }
                y_shift = r_pack - y_shift;
            }
            y_shift_z = r_pack - y_shift_z;
            x_shift = dx*0.5 - x_shift;
        }
    }
    else
    {
        // 2D...
        real y_shift = r;
        real dy = 2*r;
        real dx = 1.7320508075688772935274463415059*r;  // 2 * (sqrt(3)/2)
        for(real x = b->from.x + r; x < b->to.x - r; x += dx)
        {
            for(real y = b->from.y + r + y_shift; y < b->to.y - r; y += dy)
            {
                c.pos = b->trans*anyVector(x, y, 0);

                c.pos += anyVector(float(rand())/RAND_MAX*c.r - c.r*0.5,
                                   float(rand())/RAND_MAX*c.r - c.r*0.5,
                                   0)*0.5;

                c.age = real(rand())/real(RAND_MAX) * b->tissue->minimum_interphase_time;
                c.state_age = c.age;
                c.time_to_necrosis = b->tissue->time_to_necrosis + (2*double(rand())/double(RAND_MAX) - 1.0)*b->tissue->time_to_necrosis_var;

                bool add = true;

                // barriers...
                anyBarrier *br = FirstBarrier;
                while (add && br)
                {
                    if (!br->is_point_inside(c.pos, c.r))
                        add = false;
                    br = (anyBarrier *)br->next;
                }

                // blocks...
                anyCellBlock *bc = FirstCellBlock;
                while (add && bc)
                {
                    if (bc->generated
                        && bc->is_point_inside(c.pos, c.r*0.5))
                        add = false;
                    bc = (anyCellBlock *)bc->next;
                }

                // tubes...
                for (int i = 0; add && i < NoTubeChains; i++)
                {
                    anyTube *v = TubeChains[i];
                    while (v && add)
                    {
                        if (cell_tube_dist(&c, v) < c.r)
                            add = false;
                        v = v->next;
                    }
                }

                if (add)
                    AddCell(&c);
            }
            y_shift = r - y_shift;
        }
    }

    b->generated = true;
}


void GenerateCellsInAllBlocks()
/**
  Generates cells in all blocks.
*/
{
    SimulationSettings.max_max_max_cells_per_box = 0;
    bool gen;
    do
    {
        anyCellBlock *b = FirstCellBlock;
        gen = false;
        while (b)
        {
            if (!b->generated && b->should_be_generated_next())
            {
                GenerateCellsInBlock(b);
                gen = true;
            }
            b = (anyCellBlock *)b->next;
        }
    }
    while (gen);
}


void GenerateTubesInTubeLine(anyTubeLine *vl)
{
    // length of tube line...
    real l = (vl->to - vl->from).length();

    // number of tubes to generate...
    int n = round(l/vl->tube_length);
    real ll = l/n;

    anyVector vv = (vl->to - vl->from)*(1.0/n);

    for (int i = 0; i < n; i++)
    {
        anyTube *v = new anyTube;
        v->r = vl->r;
        v->final_r = vl->r;
        v->final_length = v->length = ll;

        v->pos1 = vl->from + vv*i;
        v->pos2 = vl->from + vv*(i + 1);

        v->pos1 = vl->trans*v->pos1;
        v->pos2 = vl->trans*v->pos2;
        SetTubeMass(v);

        if (vl->fixed_blood_pressure && i == 0)
        {
            v->fixed_blood_pressure = true;
            v->blood_pressure = vl->min_blood_pressure;
            v->one_by_mass = 0;
        }
        else if (vl->fixed_blood_pressure && i == n - 1)
        {
            v->fixed_blood_pressure = true;
            v->blood_pressure = vl->max_blood_pressure;
            v->one_by_mass = 0;
        }

        AddTube(v, i > 0, i == 0);
    }

    vl->generated = true;
}


void GenerateTubesInTubeBundle(anyTubeBundle *vb)
{
    real y_shift = (vb->from.y + vb->to.y)*0.5 + vb->shift_y;
    real z_shift = (vb->from.z + vb->to.z)*0.5 + vb->shift_z;
    for (real y = y_shift + vb->spacing_y; y <= vb->to.y; y += vb->spacing_y)
    {
        for (real z = z_shift + vb->spacing_z; z <= vb->to.z; z += vb->spacing_z)
        {
            anyTubeLine vl;
            vl.trans = vb->trans;
            vl.from.set(vb->from.x - vb->extent_x, y, z);
            vl.to.set(vb->to.x + vb->extent_x, y, z);
            vl.r = vb->r;
            vl.tube_length = vb->tube_length;
            vl.fixed_blood_pressure = vb->fixed_blood_pressure;
            vl.min_blood_pressure = vb->min_blood_pressure;
            vl.max_blood_pressure = vb->max_blood_pressure;
            GenerateTubesInTubeLine(&vl);
        }
        for (real z = z_shift; z >= vb->from.z; z -= vb->spacing_z)
        {
            anyTubeLine vl;
            vl.trans = vb->trans;
            vl.from.set(vb->from.x - vb->extent_x, y, z);
            vl.to.set(vb->to.x + vb->extent_x, y, z);
            vl.r = vb->r;
            vl.tube_length = vb->tube_length;
            vl.fixed_blood_pressure = vb->fixed_blood_pressure;
            vl.min_blood_pressure = vb->min_blood_pressure;
            vl.max_blood_pressure = vb->max_blood_pressure;
            GenerateTubesInTubeLine(&vl);
        }
    }
    for (real y = y_shift; y >= vb->from.y; y -= vb->spacing_y)
    {
        for (real z = z_shift + vb->spacing_z; z <= vb->to.z; z += vb->spacing_z)
        {
            anyTubeLine vl;
            vl.trans = vb->trans;
            vl.from.set(vb->from.x - vb->extent_x, y, z);
            vl.to.set(vb->to.x + vb->extent_x, y, z);
            vl.r = vb->r;
            vl.tube_length = vb->tube_length;
            vl.fixed_blood_pressure = vb->fixed_blood_pressure;
            vl.min_blood_pressure = vb->min_blood_pressure;
            vl.max_blood_pressure = vb->max_blood_pressure;
            GenerateTubesInTubeLine(&vl);
        }
        for (real z = z_shift; z >= vb->from.z; z -= vb->spacing_z)
        {
            anyTubeLine vl;
            vl.trans = vb->trans;
            vl.from.set(vb->from.x - vb->extent_x, y, z);
            vl.to.set(vb->to.x + vb->extent_x, y, z);
            vl.r = vb->r;
            vl.tube_length = vb->tube_length;
            vl.fixed_blood_pressure = vb->fixed_blood_pressure;
            vl.min_blood_pressure = vb->min_blood_pressure;
            vl.max_blood_pressure = vb->max_blood_pressure;
            GenerateTubesInTubeLine(&vl);
        }
    }

    vb->generated = true;
}


void GenerateTubesInAllTubeLines()
/**
  Generates tubes in all tube lines.
*/
{
    anyTubeLine *vl = FirstTubeLine;
    while (vl)
    {
        if (!vl->generated)
            GenerateTubesInTubeLine(vl);
        vl = (anyTubeLine *)vl->next;
    }
}


void GenerateTubesInAllTubeBundles()
/**
  Generates tubes in all tube bundles.
*/
{
    anyTubeBundle *vb = FirstTubeBundle;
    while (vb)
    {
        if (!vb->generated)
            GenerateTubesInTubeBundle(vb);
        vb = (anyTubeBundle *)vb->next;
    }
}



static
void save_povray_tissues(FILE *f)
{
    fprintf(f, "#declare TissueColor = array[%d]\n", NoTissueSettings);
    fprintf(f, "{\n");
    anyTissueSettings *ts = FirstTissueSettings;
    while (ts)
    {
        fprintf(f, "  rgb %s%s\n", ts->color.to_string_rgb(), ts->next ? "," : "");
        ts = ts->next;
    }
    fprintf(f, "}\n");
    fprintf(f, "#declare TissueType = array[%d]\n", NoTissueSettings);
    fprintf(f, "{\n");
    ts = FirstTissueSettings;
    while (ts)
    {
        fprintf(f, "  %d%s\n", int(ts->type), ts->next ? "," : "");
        ts = ts->next;
    }
    fprintf(f, "}\n");
    fprintf(f, "#declare TubeColor = rgb %s;\n", VisualSettings.tube_color.to_string_rgb());
    fprintf(f, "#declare InBarrierColor = rgb %s;\n", VisualSettings.in_barrier_color.to_string_rgb());
    fprintf(f, "#declare OutBarrierColor = rgb %s;\n", VisualSettings.out_barrier_color.to_string_rgb());
}


static
void save_povray_cells(FILE *f)
{
    if (!Cells) return;

    int first_cell;
    int t_id = -1;
    fprintf(f, "\n");

    // tumor...
    fprintf(f, "#if (draw_tumor)\n");
    first_cell = 0;
    for (int box_id = 0; box_id < SimulationSettings.no_boxes; box_id++)
    {
        int no_cells = Cells[first_cell].no_cells_in_box;
        for (int i = 0; i < no_cells; i++)
            // draw only active, tumor cells...
            if (Cells[first_cell + i].state != csRemoved && Cells[first_cell + i].tissue->type == ttTumor)
            {
                int clipped =
                        Cells[first_cell + i].pos.x*VisualSettings.clip_plane[0] +
                        Cells[first_cell + i].pos.y*VisualSettings.clip_plane[1] +
                        Cells[first_cell + i].pos.z*VisualSettings.clip_plane[2] +
                        VisualSettings.clip_plane[3] > 0;

                t_id = Cells[first_cell + i].tissue->id;
                fprintf(f, "c(%d, %s, %g, %d, %g, %g, %d)\n",
                        t_id,
                        Cells[first_cell + i].pos.toString(),
                        Cells[first_cell + i].r,
                        int(Cells[first_cell + i].state),
                        Cells[first_cell + i].concentrations[dsO2][SimulationSettings.step % 2],
                        Cells[first_cell + i].concentrations[dsTAF][SimulationSettings.step % 2],
                        clipped);
            }

        first_cell += SimulationSettings.max_cells_per_box;
    }
    fprintf(f, "#end\n");

    // normal...
    fprintf(f, "#if (draw_normal)\n");
    first_cell = 0;
    for (int box_id = 0; box_id < SimulationSettings.no_boxes; box_id++)
    {
        int no_cells = Cells[first_cell].no_cells_in_box;
        for (int i = 0; i < no_cells; i++)
            // draw only active, normal cells...
            if (Cells[first_cell + i].state != csRemoved && Cells[first_cell + i].tissue->type == ttNormal)
            {
                int clipped =
                        Cells[first_cell + i].pos.x*VisualSettings.clip_plane[0] +
                        Cells[first_cell + i].pos.y*VisualSettings.clip_plane[1] +
                        Cells[first_cell + i].pos.z*VisualSettings.clip_plane[2] +
                        VisualSettings.clip_plane[3] > 0;

                t_id = Cells[first_cell + i].tissue->id;
                fprintf(f, "c(%d, %s, %g, %d, %g, %g, %d)\n",
                        t_id,
                        Cells[first_cell + i].pos.toString(),
                        Cells[first_cell + i].r,
                        int(Cells[first_cell + i].state),
                        Cells[first_cell + i].concentrations[dsO2][SimulationSettings.step % 2],
                        Cells[first_cell + i].concentrations[dsTAF][SimulationSettings.step % 2],
                        clipped);
            }

        first_cell += SimulationSettings.max_cells_per_box;
    }
    fprintf(f, "#end\n");
}


void SmoothTubeTips(anyTube const *v, anyVector &pos1, anyVector &pos2)
{
    if (!v->prev)
        pos1 = v->pos1;
    else
        pos1 = (v->pos1 + v->prev->pos2)*0.5;

    if (!v->next)
        pos2 = v->pos2;
    else
        pos2 = (v->pos2 + v->next->pos1)*0.5;
}


static
void save_povray_tubes(FILE *f)
{
    if (!NoTubes) return;

    fprintf(f, "\n");
    fprintf(f, "#if (draw_tubes)\n");

    for (int i = 0; i < NoTubeChains; i++)
    {
        anyTube *v = TubeChains[i];
        fprintf(f, "merge\n");
        fprintf(f, "{\n");
        while (v)
        {
            anyVector p1, p2;
            SmoothTubeTips(v, p1, p2);
            fprintf(f, " b(%s, %s, %g, %d)\n", p1.toString(), p2.toString(), v->r, !v->next);
            v = v->next;
        }
        fprintf(f, " bc()\n");
        fprintf(f, "}\n");
    }
    fprintf(f, "#end\n");
}


static
void save_povray_barriers(FILE *f)
{
    anyBarrier *b = FirstBarrier;

    if (!b) return;

    fprintf(f, "\n");
    fprintf(f, "#if (draw_barriers)\n");
    while (b)
    {
        fprintf(f, "ba(%s, %s, %d)\n", b->from.toString(), b->to.toString(), int(b->type));
        b = (anyBarrier *)b->next;
    }
    fprintf(f, "#end\n");
}


void SavePovRay(char const *povfname, bool save_ani)
/**
  Saves povray file(s).

  \param povfname -- name of output file, if NULL then name is generated.
  \param save_ani -- save suplementary animation files?
*/
{
    static int frame = 0;

    frame++;

    FILE *f;
    char fname[P_MAX_PATH];

    if (save_ani)
    {
        // animation file...
        snprintf(fname, P_MAX_PATH, "%spovray", GlobalSettings.output_dir);

        mkdir(fname);

        snprintf(fname, P_MAX_PATH, "%spovray/pov.ini", GlobalSettings.output_dir);
        f = fopen(fname, "w");
        if (f)
        {

            fprintf(f, "Initial_Frame=1\n");
            fprintf(f, "Final_Frame=%d\n\n", frame);

            fprintf(f, "Initial_Clock=1\n");
            fprintf(f, "Final_Clock=%d\n\n", frame);

            fprintf(f, "Subset_start_frame=1\n");
            fprintf(f, "Subset_end_frame=%d\n\n", frame);

            fprintf(f, "Input_File_Name=\"pov.pov\"\n");
            fprintf(f, "Output_File_Name=\"frames/\"\n");
            fclose(f);
        }

        // suplementary animation file...
        snprintf(fname, P_MAX_PATH, "%spovray/pov.pov", GlobalSettings.output_dir);
        f = fopen(fname, "w");
        if (f)
        {
            fprintf(f, "#include concat(str(clock, -6, 0), \".pov\")\n");
            fclose(f);
        }
    }

    // main povray file...
    if (povfname)
        strncpy(fname, povfname, P_MAX_PATH);
    else
        snprintf(fname, P_MAX_PATH, "%spovray/%06d.pov", GlobalSettings.output_dir, frame);

    LOG2(llInfo, "Saving PovRay file: ", fname);

    f = fopen(fname, "w");
    if (f)
    {
        fprintf(f, "// STEP: %d\n", SimulationSettings.step);
        fprintf(f, "\n");
        fprintf(f, "#declare BackgroundColor=rgb %s;\n", VisualSettings.bkg_color.to_string_rgb());
        fprintf(f, "#declare LightSourcePos=<%g, %g, %g>;\n", SimulationSettings.comp_box_to.x, SimulationSettings.comp_box_to.y, SimulationSettings.comp_box_to.z*3);
        fprintf(f, "#declare EyePos=%s;\n", VisualSettings.eye.toString());
        fprintf(f, "#declare ViewMatrix=array[16] %s\n", VisualSettings.v_matrix.toString("{", "}"));

        fprintf(f, "\n");
        fprintf(f, "#include \"%s%smotpuca.pov\"\n", GlobalSettings.user_dir, FOLDER_LIB_POVRAY);

        fprintf(f, "\n");

        // tissues...
        save_povray_tissues(f);

        fprintf(f, "\n");
        fprintf(f, "union\n{\n");

        // barriers...
        save_povray_barriers(f);

        // cells...
        save_povray_cells(f);

        // tubes...
        save_povray_tubes(f);

        fprintf(f, "\ntransformation()\n}\n");

        fclose(f);
    }
}


void SaveVTK()
{
    LOG(llInfo, "Saving VTK file: data.vtk");

    FILE *f = fopen("data.vtk", "w");
    if (f)
    {
        fprintf(f, "# vtk DataFile Version 2.0\n");
        fprintf(f, "Test data\n");
        fprintf(f, "ASCII\n");
        fprintf(f, "DATASET UNSTRUCTURED_GRID\n");

        // count cells...
        int total_no_cells = 0;
        int first_cell = 0;
        for (int box_id = 0; box_id < SimulationSettings.no_boxes; box_id++)
        {
            total_no_cells += Cells[first_cell].no_cells_in_box;
            first_cell += SimulationSettings.max_cells_per_box;
        }

        // save coordinates...
        fprintf(f, "POINTS %d float\n", total_no_cells);
        first_cell = 0;
        int no_cells;
        for (int box_id = 0; box_id < SimulationSettings.no_boxes; box_id++)
        {
            no_cells = Cells[first_cell].no_cells_in_box;
            for (int i = 0; i < no_cells; i++)
                fprintf(f, "%.2f %.2f %.2f\n", Cells[first_cell + i].pos.x, Cells[first_cell + i].pos.y, Cells[first_cell + i].pos.z);
            first_cell += SimulationSettings.max_cells_per_box;
        }

        // save radiuses...
        fprintf(f, "POINT_DATA %d\n", total_no_cells);
        fprintf(f, "SCALARS radius float 1\n");
        fprintf(f, "LOOKUP_TABLE default\n");
        first_cell = 0;
        for (int box_id = 0; box_id < SimulationSettings.no_boxes; box_id++)
        {
            no_cells = Cells[first_cell].no_cells_in_box;
            for (int i = 0; i < no_cells; i++)
                fprintf(f, "%g\n", Cells[first_cell + i].r);
            first_cell += SimulationSettings.max_cells_per_box;
        }

        fclose(f);
    }
}


void anyBoundingBox::update_bounding_box_by_point(anyVector v, anyVector &u_from, anyVector &u_to, bool &first)
{
    v = trans*v;
    if (first)
    {
        u_from = v;
        u_to = v;
        first = false;
    }
    else
    {
        if (v.x < u_from.x)
            u_from.x = v.x;
        if (v.x > u_to.x)
            u_to.x = v.x;

        if (v.y < u_from.y)
            u_from.y = v.y;
        if (v.y > u_to.y)
            u_to.y = v.y;

        if (v.z < u_from.z)
            u_from.z = v.z;
        if (v.z > u_to.z)
            u_to.z = v.z;
    }
}


void anyBoundingBox::update_bounding_box(anyVector &u_from, anyVector &u_to, bool &first)
{
    anyVector v;

    v.set(from.x, from.y, from.z);
    update_bounding_box_by_point(v, u_from, u_to, first);
    v.set(from.x, from.y, to.z);
    update_bounding_box_by_point(v, u_from, u_to, first);
    v.set(from.x, to.y,   from.z);
    update_bounding_box_by_point(v, u_from, u_to, first);
    v.set(from.x, to.y,   to.z);
    update_bounding_box_by_point(v, u_from, u_to, first);
    v.set(to.x,   from.y, from.z);
    update_bounding_box_by_point(v, u_from, u_to, first);
    v.set(to.x,   from.y, to.z);
    update_bounding_box_by_point(v, u_from, u_to, first);
    v.set(to.x,   to.y,   from.z);
    update_bounding_box_by_point(v, u_from, u_to, first);
    v.set(to.x,   to.y,   to.z);
    update_bounding_box_by_point(v, u_from, u_to, first);
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
    UpdateSimulationBox();
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
        ts = FirstTissueSettings;
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
        ParseTissueSettings(f, (anyTissueSettings *)editable, false);
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

        SaveTissueSettings_ag(f, (anyTissueSettings *)editable, false);

        // restore tissue properties...
        *(anyTissueSettings *)editable = ts;

        fclose(f);
    }
    catch (Error *err)
    {
        LogError(err);
    }
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
    anyTissueSettings *ts = FirstTissueSettings;
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

#endif


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

        ParseCellBlock(f, this, false);

        fclose(f);
    }
    catch (Error *err)
    {
        LogError(err);
    }
}


#ifdef QT_CORE_LIB
void anyCellBlock::prepare_dialog()
{
    dialog->dialog->groupBox_size->setVisible(true);
    dialog->dialog->groupBox_tissue->setVisible(true);
    dialog->dialog->groupBox_conc->setVisible(true);

    anyEditable::prepare_dialog();

    dialog->dialog->doubleSpinBox_concO2->setValue(concentrations[dsO2]);
    dialog->dialog->doubleSpinBox_concTAF->setValue(concentrations[dsTAF]);
    dialog->dialog->doubleSpinBox_concPericytes->setValue(concentrations[dsPericytes]);

    // select tissue in combo...
    if (tissue)
        for (int i = 0; i < dialog->dialog->comboBox_tissue->count(); i++)
            if (dialog->dialog->comboBox_tissue->itemText(i) == QString(tissue->name))
            {
                dialog->dialog->comboBox_tissue->setCurrentIndex(i);
                break;
            }
}
#endif

#ifdef QT_CORE_LIB
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
#endif


#ifdef QT_CORE_LIB
void anyCellBlock::update_from_dialog()
{
    anyEditable::update_from_dialog();

    // find and update tissue...
    anyTissueSettings *ts = FirstTissueSettings;
    while (ts)
    {
        if (QString(ts->name) == dialog->dialog->comboBox_tissue->currentText())
        {
            tissue = ts;
            break;
        }
        ts = ts->next;
    }

    concentrations[dsO2] = dialog->dialog->doubleSpinBox_concO2->value();
    concentrations[dsTAF] = dialog->dialog->doubleSpinBox_concTAF->value();
    concentrations[dsPericytes] = dialog->dialog->doubleSpinBox_concPericytes->value();

    LOG(llDebug, "CellBlock updated from dialog");
}
#endif

bool anyCellBlock::can_move_up()
{
    return LastCellBlock != this;
}


bool anyCellBlock::can_move_down()
{
    return FirstCellBlock != this;
}


void anyCellBlock::move_up()
{
    if (!can_move_up())
        return;

    // find previous element...
    anyEditable *pe;
    if (this == FirstCellBlock)
        pe = 0;
    else
    {
        pe = FirstCellBlock;
        while (pe && pe->next != this)
            pe = pe->next;
    }

    // find next element...
    anyEditable *ne = next;

    // remove 'this' from list...
    if (pe)
        pe->next = ne;
    else
        FirstCellBlock = (anyCellBlock *)ne;

    // add 'this' one element ahead...
    next = ne->next;
    if (LastCellBlock == ne)
        LastCellBlock = this;
    ne->next = this;
}


void anyCellBlock::move_down()
{
    if (!can_move_down())
        return;

    // find previous element...
    anyEditable *pe;
    if (this == FirstCellBlock)
        pe = 0;
    else
    {
        pe = FirstCellBlock;
        while (pe && pe->next != this)
            pe = pe->next;
    }

    if (pe)
        pe->move_up();
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

        ParseBarrier(f, this, false);

        fclose(f);
    }
    catch (Error *err)
    {
        LogError(err);
    }
}


#ifdef QT_CORE_LIB
void anyBarrier::prepare_dialog()
{
    dialog->dialog->groupBox_size->setVisible(true);
    dialog->dialog->groupBox_barriertype->setVisible(true);

    anyEditable::prepare_dialog();

    // set barrier type...
    if (type == btIn)
        dialog->dialog->radioButton_in->setChecked(true);
    else
        dialog->dialog->radioButton_out->setChecked(true);
}
#endif


#ifdef QT_CORE_LIB
bool anyBarrier::validate_properties()
{
    return true;
}
#endif


#ifdef QT_CORE_LIB
void anyBarrier::update_from_dialog()
{
    anyEditable::update_from_dialog();

    // get barrier type...
    if (dialog->dialog->radioButton_in->isChecked())
        type = btIn;
    else
        type = btOut;

    LOG(llDebug, "Barrier updated from dialog");
}
#endif

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

        ParseTubeBundle(f, this, false);

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
    extent_x = billion_to_inf(dialog->dialog->doubleSpinBox_extension->value());
    r = billion_to_inf(dialog->dialog->doubleSpinBox_tube_radius->value());
    tube_length = billion_to_inf(dialog->dialog->doubleSpinBox_tube_length->value());
    spacing_y = billion_to_inf(dialog->dialog->doubleSpinBox_spacing_y->value());
    spacing_z = billion_to_inf(dialog->dialog->doubleSpinBox_spacing_z->value());
    shift_y = billion_to_inf(dialog->dialog->doubleSpinBox_shift_y->value());
    shift_z = billion_to_inf(dialog->dialog->doubleSpinBox_shift_z->value());
    min_blood_pressure = dialog->dialog->doubleSpinBox_blood_min_pressure->value();
    max_blood_pressure = dialog->dialog->doubleSpinBox_blood_max_pressure->value();
    fixed_blood_pressure = dialog->dialog->checkBox_blood_fixed->isChecked();

    LOG(llDebug, "TubeBundle updated from dialog");
}
#endif


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

        ParseTubeLine(f, this, false);

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
#endif


#ifdef QT_CORE_LIB
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
#endif


#ifdef QT_CORE_LIB
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
#endif


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

        ParseTissueSettings(f, this, false);

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
    dialog->dialog->radioButton_t_type_normal->setChecked(type == ttNormal);
    dialog->dialog->radioButton_t_type_tumor->setChecked(type == ttTumor);
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
    anyTissueSettings *ts = FindTissueSettings(dialog->dialog->lineEdit_name->text().toLatin1());
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
    anyCellBlock *cb = FirstCellBlock;
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
        int no_cells = Cells[first_cell].no_cells_in_box;
        for (int i = 0; i < no_cells; i++)
            if (Cells[first_cell + i].tissue == this)
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
        type = ttNormal;
    else if (dialog->dialog->radioButton_t_type_tumor->isChecked())
        type = ttTumor;
    strcpy(name, dialog->dialog->lineEdit_name->text().toLatin1());
    color = dialog->color;
    cell_r = billion_to_inf(dialog->dialog->doubleSpinBox_t_radius->value());
    density = billion_to_inf(dialog->dialog->doubleSpinBox_t_density->value());
    cell_grow_speed = billion_to_inf(dialog->dialog->doubleSpinBox_t_growth->value());
    minimum_interphase_time = billion_to_inf(dialog->dialog->doubleSpinBox_t_mininttime->value());
    time_to_apoptosis = billion_to_inf(dialog->dialog->doubleSpinBox_t_timetoapop->value());
    time_to_necrosis = billion_to_inf(dialog->dialog->doubleSpinBox_t_timetonecr->value());
    time_to_necrosis_var = billion_to_inf(dialog->dialog->doubleSpinBox_t_timetonecrvar->value());
    time_in_necrosis = billion_to_inf(dialog->dialog->doubleSpinBox_t_timeinnecr->value());
    dead_r = billion_to_inf(dialog->dialog->doubleSpinBox_t_deadradius->value());
    cell_shrink_speed = billion_to_inf(dialog->dialog->doubleSpinBox_t_shrink->value());
    minimum_mitosis_r = billion_to_inf(dialog->dialog->doubleSpinBox_t_minmitradius->value());
    max_pressure = billion_to_inf(dialog->dialog->lineEdit_t_maxpressure->text().toDouble());
    force_rep_factor = billion_to_inf(dialog->dialog->lineEdit_t_repfactor->text().toDouble());
    force_atr1_factor = billion_to_inf(dialog->dialog->lineEdit_t_attrfactor1->text().toDouble());
    force_atr2_factor = billion_to_inf(dialog->dialog->lineEdit_t_attrfactor2->text().toDouble());
    force_dpd_factor = billion_to_inf(dialog->dialog->lineEdit_t_dpdfactor->text().toDouble());
    dpd_temperature= billion_to_inf(dialog->dialog->lineEdit_t_dpdtemp->text().toDouble());
    o2_consumption = billion_to_inf(dialog->dialog->lineEdit_t_o2cons->text().toDouble());
    o2_hypoxia = dialog->dialog->doubleSpinBox_t_o2hypoxia->value();
    pericyte_production = dialog->dialog->lineEdit_t_per_prod->text().toDouble();

    LOG(llDebug, "Tissue updated from dialog");
}
#endif


void UpdateSimulationBox()
{
    if (GlobalSettings.simulation_allocated)
        return;

    bool first = true;
    anyVector m1, m2;
    anyBarrier *b = FirstBarrier;
    while (b)
    {
        b->update_bounding_box(m1, m2, first);
        b = (anyBarrier *)b->next;
    }

    anyCellBlock *cb = FirstCellBlock;
    while (cb)
    {
        cb->update_bounding_box(m1, m2, first);
        cb = (anyCellBlock *)cb->next;
    }

    anyVector newfrom, newto;

    newfrom = m1 - anyVector(10, 10, 10);
    newto = m2 + anyVector(10, 10, 10);

    if (newfrom != SimulationSettings.comp_box_from ||
        newto != SimulationSettings.comp_box_to)
    {
        SimulationSettings.comp_box_from = newfrom;
        SimulationSettings.comp_box_to = newto;
    }
}


void SaveAG(char const *fname, bool save_cells_and_tubes)
{
    LOG2(llInfo, "Saving ag file: ", fname);
    FILE *f = fopen(fname, "w");
    if (f)
    {
        SaveDefinitions_ag(f);
        fprintf(f, "\n");
        SaveVisualSettings_ag(f, &VisualSettings);
        SaveSimulationSettings_ag(f, &SimulationSettings);
        SaveTubularSystemSettings_ag(f, &TubularSystemSettings);
        fprintf(f, "\n//---[ BARRIERS ]---------------------------------------------------------------\n");
        SaveAllBarriers_ag(f);
        fprintf(f, "\n//---[ TISSUES ]----------------------------------------------------------------\n");
        SaveAllTissueSettings_ag(f);
        fprintf(f, "\n//---[ BLOCKS ]-----------------------------------------------------------------\n");
        SaveAllCellBlocks_ag(f);
        fprintf(f, "\n//---[ TUBE BUNDLES ]---------------------------------------------------------\n");
        SaveAllTubeBundles_ag(f);
        fprintf(f, "\n//---[ TUBE LINES ]-----------------------------------------------------------\n");
        SaveAllTubeLines_ag(f);

        if (save_cells_and_tubes)
        {
            fprintf(f, "\n//---[ TUBES ]----------------------------------------------------------------\n");
            SaveAllTubes_ag(f);
            fprintf(f, "\n//---[ CELLS ]------------------------------------------------------------------\n");
            SaveAllCells_ag(f);
        }
        fclose(f);
    }
}


bool anyCellBlock::should_be_generated_next()
{
    // kiedy mozemy generowac? kiedy nie ma mniejszych niewygenerowanych...

    // bounding box...
    anyVector bb_from, bb_to;
    bool first = true;
    update_bounding_box(bb_from, bb_to, first);

    anyCellBlock *cb = FirstCellBlock;
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


anyTube *FindTubeById(int id, bool current)
{
    // loop over all tubes...
    for (int i = 0; i < NoTubeChains; i++)
    {
        anyTube *v = TubeChains[i];
        while (v)
        {
            if ((v->id == id && current) || (v->parsed_id == id && !current))
                return v;
            v = v->next;
        }
    }
    return 0;
}


void RelinkTubes()
{
    // loop over all tubes...
    for (int i = 0; i < NoTubeChains; i++)
    {
        anyTube *v = TubeChains[i];
        while (v)
        {
            if (v->base_id)
            {
              v->base = FindTubeById(v->base_id, false);
              v->base->fork = v;
              v->base_id = 0;
            }
            if (v->top_id)
            {
              v->top = FindTubeById(v->top_id, false);
              v->top->jab = v;
              v->top_id = 0;
            }
            v = v->next;
        }
    }

}


anyTube *FindFirstTube(anyTube *v)
{
    while (v->prev)
        v = v->prev;
    return v;
}

anyTube *FindLastTube(anyTube *v)
{
    while (v->next)
        v = v->next;
    return v;
}


void AddTubesToMerge(anyTube *v1, anyTube *v2)
{
    if (NoTubeMerge < SimulationSettings.max_tube_merge)
    {
        v1 = FindFirstTube(v1);
        v2 = FindFirstTube(v2);

        // tube cannot be merged more than once in one simulation step...
        for (int i = 0; i < NoTubeMerge; i++)
            if (TubelMerge[i].t1 == v1 || TubelMerge[i].t2 == v1 || TubelMerge[i].t1 == v2 || TubelMerge[i].t2 == v2)
                return;

        TubelMerge[NoTubeMerge].t1 = v1;
        TubelMerge[NoTubeMerge].t2 = v2;

        NoTubeMerge++;
    }
}


void MergeTubes()
{
    StartTimer(TimerMergeTubesId);

    for (int i = 0; i < NoTubeMerge; i++)
    {
        // revert chain #1...
        anyTube *v = TubelMerge[i].t1;
        anyTube *vn = v->next;
        anyTube *t1 = FindLastTube(TubelMerge[i].t1);
        anyTube *t2 = FindLastTube(TubelMerge[i].t2);
        while (v)
        {
            SWAP(anyTube *, v->next, v->prev);
            SWAP(anyVector, v->velocity1, v->velocity2);
            SWAP(anyVector, v->force1, v->force2);
            SWAP(anyVector, v->pos1, v->pos2);

            v = vn;
            if (v)
                vn = v->next;
        }

        if (TubelMerge[i].t1->base)
        {
            TubelMerge[i].t1->base->fork = 0;
            TubelMerge[i].t1->base->jab = TubelMerge[i].t1;
        }
        TubelMerge[i].t1->top = TubelMerge[i].t1->base;
        TubelMerge[i].t1->base = 0;

        // remove chain #1 from list of chains...
        for (int j = 0; j < NoTubeChains; j++)
        {
            if (TubeChains[j] == TubelMerge[i].t1)
            {
                TubeChains[j] = TubeChains[NoTubeChains - 1];
                NoTubeChains--;
                break;
            }
        }

        // connect chains...
        t2->next = t1;
        t1->prev = t2;

        LOG(llDebug, "Chains merged");
    }

    NoTubeMerge = 0;


    StopTimer(TimerMergeTubesId);
}


anyTissueSettings *FindTissueSettingById(int id)
{
    anyTissueSettings *ts = FirstTissueSettings;
    while (ts)
    {
        if (ts->id == id)
            return ts;
        ts = ts->next;
    }
    return 0;
}
