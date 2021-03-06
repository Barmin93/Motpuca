#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <direct.h>

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

#include "anytube.h"
#include "anybarrier.h"
#include "anycellblock.h"
#include "anytubebundle.h"
#include "anytubeline.h"

#ifdef QT_CORE_LIB
#include "mainwindow.h"
#endif

char const *CellState_names[] = { "-added-", "-removed-", "ALIVE", "HYPOXIA", "APOPTOSIS", "NECROSIS" };
char const *TissueType_names[] = { "NORMAL", "TUMOR" };
char const *BarrierType_names[] = { "KEEP_IN", "KEEP_OUT" };
char const *DiffundingSubstances_names[] = { "O2", "TAF", "Pericytes" };

namespace scene {
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

    float ***Concentrations = 0;


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
        PARSE_VALUE_ENUM((*ts), sat::TissueType, type)
        PARSE_VALUE_COLOR((*ts), color)
        PARSE_VALUE_float((*ts), cell_r)
        PARSE_VALUE_float((*ts), density)
        PARSE_VALUE_float((*ts), cell_grow_speed)
        PARSE_VALUE_float((*ts), minimum_interphase_time)
        PARSE_VALUE_float((*ts), time_to_apoptosis)
        PARSE_VALUE_float((*ts), time_to_necrosis)
        PARSE_VALUE_float((*ts), time_in_necrosis)
        PARSE_VALUE_float((*ts), dead_r)
        PARSE_VALUE_float((*ts), cell_shrink_speed)
        PARSE_VALUE_float((*ts), minimum_mitosis_r)
        PARSE_VALUE_float((*ts), force_rep_factor)
        PARSE_VALUE_float((*ts), force_atr1_factor)
        PARSE_VALUE_float((*ts), force_atr2_factor)
        PARSE_VALUE_float((*ts), force_dpd_factor)
        PARSE_VALUE_float((*ts), dpd_temperature)
        PARSE_VALUE_float((*ts), max_pressure)
        PARSE_VALUE_float((*ts), o2_consumption)
        PARSE_VALUE_float((*ts), medicine_consumption)
        PARSE_VALUE_float((*ts), o2_hypoxia)
        PARSE_VALUE_float((*ts), pericyte_production)
        PARSE_VALUE_float((*ts), time_to_necrosis_var)

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
        SAVE_float(f, ts, cell_r);
        SAVE_float(f, ts, density);
        SAVE_float(f, ts, cell_grow_speed);
        SAVE_float(f, ts, minimum_interphase_time);
        SAVE_float(f, ts, time_to_apoptosis);
        SAVE_float(f, ts, time_to_necrosis);
        SAVE_float(f, ts, time_in_necrosis);
        SAVE_float(f, ts, dead_r);
        SAVE_float(f, ts, cell_shrink_speed);
        SAVE_float(f, ts, minimum_mitosis_r);
        SAVE_float(f, ts, force_rep_factor);
        SAVE_float(f, ts, force_atr1_factor);
        SAVE_float(f, ts, force_atr2_factor);
        SAVE_float(f, ts, max_pressure);
        SAVE_float(f, ts, o2_consumption);
        SAVE_float(f, ts, medicine_consumption);
        SAVE_float(f, ts, o2_hypoxia);
        SAVE_float(f, ts, pericyte_production);
        SAVE_float(f, ts, time_to_necrosis_var);
        SAVE_float(f, ts, force_dpd_factor);
        SAVE_float(f, ts, dpd_temperature);

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
        PARSE_VALUE_ENUM((*b), sat::BarrierType, type)
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

            Concentrations = new float**[2];
            for(int frame = 0; frame < 2; frame++)
            {
                Concentrations[frame] = new float*[sat::dsLast];
                for (int i = 0; i < sat::dsLast; i++)
                {
                    Concentrations[frame][i] = new float[SimulationSettings.no_boxes];
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
        LOG(llError, "floatlocation not implemented YET!");

        // workaround...
        scene::DeallocSimulation();
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
                for (int i = 0; i < sat::dsLast; i++)
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
            c->concentrations[sat::dsO2][0] = c->concentrations[sat::dsO2][1] = Token.number;
        }
        else if (!StrCmp(tv.str, "conc_TAF"))
        {
            if (Token.type != TT_Number)
                throw new Error(__FILE__, __LINE__, "Invalid concentration value", TokenToString(Token), ParserFile, ParserLine);
            if (Token.number < 0 || Token.number > 1)
                throw new Error(__FILE__, __LINE__, "Invalid concentration value", TokenToString(Token), ParserFile, ParserLine);
            c->concentrations[sat::dsTAF][0] = c->concentrations[sat::dsTAF][1] = Token.number;
        }
        else if (!StrCmp(tv.str, "conc_Pericytes"))
        {
            if (Token.type != TT_Number)
                throw new Error(__FILE__, __LINE__, "Invalid concentration value", TokenToString(Token), ParserFile, ParserLine);
            if (Token.number < 0 || Token.number > 1)
                throw new Error(__FILE__, __LINE__, "Invalid concentration value", TokenToString(Token), ParserFile, ParserLine);
            c->concentrations[sat::dsPericytes][0] = c->concentrations[sat::dsPericytes][1] = Token.number;
        }
        else if (!StrCmp(tv.str, "conc_Medicine"))
        {
            if (Token.type != TT_Number)
                throw new Error(__FILE__, __LINE__, "Invalid concentration value", TokenToString(Token), ParserFile, ParserLine);
            if (Token.number < 0 || Token.number > 1)
                throw new Error(__FILE__, __LINE__, "Invalid concentration value", TokenToString(Token), ParserFile, ParserLine);
            c->concentrations[sat::dsMedicine][0] = c->concentrations[sat::dsMedicine][1] = Token.number;
        }
        PARSE_VALUE_VECTOR((*c), pos)
        PARSE_VALUE_float((*c), r)
        PARSE_VALUE_float((*c), age)
        PARSE_VALUE_ENUM((*c), sat::CellState, state)
        PARSE_VALUE_float((*c), state_age)
        PARSE_VALUE_float((*c), time_to_necrosis)

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
        SAVE_float(f, c, r);
        SAVE_float(f, c, age);
        SAVE_float(f, c, state_age);
        SAVE_float(f, c, time_to_necrosis);

        fprintf(f, "  conc_O2 = %g\n", c->concentrations[sat::dsO2][SimulationSettings.step % 2]);
        fprintf(f, "  conc_TAF = %g\n", c->concentrations[sat::dsTAF][SimulationSettings.step % 2]);
        fprintf(f, "  conc_Pericytes = %g\n", c->concentrations[sat::dsPericytes][SimulationSettings.step % 2]);
        fprintf(f, "  conc_Medicine = %g\n", c->concentrations[sat::dsMedicine][SimulationSettings.step % 2]);

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
                if (Cells[first_cell + i].state > sat::csRemoved)
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
            b->concentrations[sat::dsO2] = Token.number;
        }
        else if (!StrCmp(tv.str, "conc_TAF"))
        {
            if (Token.type != TT_Number)
                throw new Error(__FILE__, __LINE__, "Invalid concentration value", TokenToString(Token), ParserFile, ParserLine);
            if (Token.number < 0 || Token.number > 1)
                throw new Error(__FILE__, __LINE__, "Invalid concentration value", TokenToString(Token), ParserFile, ParserLine);
            b->concentrations[sat::dsTAF] = Token.number;
        }
        else if (!StrCmp(tv.str, "conc_Pericytes"))
        {
            if (Token.type != TT_Number)
                throw new Error(__FILE__, __LINE__, "Invalid concentration value", TokenToString(Token), ParserFile, ParserLine);
            if (Token.number < 0 || Token.number > 1)
                throw new Error(__FILE__, __LINE__, "Invalid concentration value", TokenToString(Token), ParserFile, ParserLine);
            b->concentrations[sat::dsPericytes] = Token.number;
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
        fprintf(f, "  conc_o2 = %g\n", b->concentrations[sat::dsO2]);
        fprintf(f, "  conc_taf = %g\n", b->concentrations[sat::dsTAF]);
        fprintf(f, "  conc_pericytes = %g\n", b->concentrations[sat::dsPericytes]);
        fprintf(f, "  conc_medicine = %g\n", b->concentrations[sat::dsMedicine]);
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
        PARSE_VALUE_float((*vl), tube_length)
        PARSE_VALUE_float((*vl), r)
        PARSE_VALUE_float((*vl), min_blood_pressure)
        PARSE_VALUE_float((*vl), max_blood_pressure)
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
        SAVE_float(f, vl, tube_length);
        SAVE_float(f, vl, r);

        SAVE_float(f, vl, min_blood_pressure);
        SAVE_float(f, vl, max_blood_pressure);
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
        PARSE_VALUE_float((*vb), r)
        PARSE_VALUE_float((*vb), tube_length)
        PARSE_VALUE_float((*vb), extent_x)
        PARSE_VALUE_float((*vb), spacing_y)
        PARSE_VALUE_float((*vb), spacing_z)
        PARSE_VALUE_float((*vb), shift_y)
        PARSE_VALUE_float((*vb), shift_z)
        PARSE_VALUE_float((*vb), min_blood_pressure)
        PARSE_VALUE_float((*vb), max_blood_pressure)
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
        SAVE_float(f, vb, extent_x);
        SAVE_float(f, vb, spacing_y);
        SAVE_float(f, vb, spacing_z);
        SAVE_float(f, vb, shift_y);
        SAVE_float(f, vb, shift_z);
        SAVE_float(f, vb, tube_length);
        SAVE_float(f, vb, r);

        SAVE_float(f, vb, min_blood_pressure);
        SAVE_float(f, vb, max_blood_pressure);
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
        float len = v->final_length;
    //    float len = (v->pos2 - v->pos1).length();
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
        PARSE_VALUE_float((*v), r)
        PARSE_VALUE_float((*v), final_r)
        PARSE_VALUE_float((*v), length)
        PARSE_VALUE_float((*v), final_length)
        PARSE_VALUE_ENUM((*v), sat::CellState, state)
        PARSE_VALUE_float((*v), age)
        PARSE_VALUE_float((*v), state_age)
        PARSE_VALUE_INT((*v),  base_id)
        PARSE_VALUE_INT((*v),  top_id)

        PARSE_VALUE_float((*v), blood_pressure)
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
        SAVE_float(f, v, length);
        if (v->length != v->final_length)
            SAVE_float(f, v, final_length);
        fprintf(f, "  // current length = %g\n", (v->pos2 - v->pos1).length());
        SAVE_float(f, v, r);
        if (v->r != v->final_r)
            SAVE_float(f, v, final_r);
        SAVE_float(f, v, age);
        SAVE_float(f, v, state_age);

        SAVE_float(f, v, blood_pressure);
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
    float cell_tube_dist(anyCell const *c, anyTube const *v)
    /**
      Calculates distance between cell and tube.

      \param c -- pointer to cell
      \param v -- pointer to tube
    */
    {
        anyVector p12 = v->pos2 - v->pos1;
        anyVector pc = c->pos - v->pos1;
        float p12_len = p12.length();
        anyVector p12_n = p12;
        p12_n.normalize();

        float d = pc|p12_n;

        float p;

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
        float r = b->tissue->cell_r;
        float r_pack = r*0.9;

        c.r = r;
        c.tissue = b->tissue;

        for (int i = 0; i < sat::dsLast; i++)
            c.concentrations[i][0] = c.concentrations[i][1] = b->concentrations[i];

        SetCellMass(&c);

        if (SimulationSettings.dimensions == 3)
        {
            // 3D...
            float x_shift = 0;
            float y_shift = r_pack;
            float y_shift_z = 0;
            float dz = 1.6329931618554520654648560498039*r_pack; // 2 * (sqrt(6)/3)
            float dy = 2*r_pack;
            float dx = 1.7320508075688772935274463415059*r_pack;  // 2 * (sqrt(3)/2)

            int x_cnt = floor((b->to.x - b->from.x - 2*r_pack)/dx);
            int y_cnt = floor((b->to.y - b->from.y - 2*r_pack)/dy);
            int z_cnt = floor((b->to.z - b->from.z - 2*r_pack)/dz);

            // d* correction...
            dx = (b->to.x - b->from.x - 2*r_pack)/x_cnt;
            dy = (b->to.y - b->from.y - 2*r_pack)/y_cnt;
            dz = (b->to.z - b->from.z - 2*r_pack)/z_cnt;

            for(float z = b->from.z + r; z <= b->to.z; z += dz)
            {
                for(float x = b->from.x + r + x_shift; x < b->to.x; x += dx)
                {
                    for(float y = b->from.y + r + y_shift + y_shift_z; y < b->to.y; y += dy)
                    {
                        c.pos = b->trans*anyVector(x, y, z);
                        c.pos += anyVector(float(rand())/RAND_MAX*c.r - c.r*0.5,
                                           float(rand())/RAND_MAX*c.r - c.r*0.5,
                                           0)*0.5;

                        c.age = float(rand())/float(RAND_MAX) * b->tissue->minimum_interphase_time;
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
            float y_shift = r;
            float dy = 2*r;
            float dx = 1.7320508075688772935274463415059*r;  // 2 * (sqrt(3)/2)
            for(float x = b->from.x + r; x < b->to.x - r; x += dx)
            {
                for(float y = b->from.y + r + y_shift; y < b->to.y - r; y += dy)
                {
                    c.pos = b->trans*anyVector(x, y, 0);

                    c.pos += anyVector(float(rand())/RAND_MAX*c.r - c.r*0.5,
                                       float(rand())/RAND_MAX*c.r - c.r*0.5,
                                       0)*0.5;

                    c.age = float(rand())/float(RAND_MAX) * b->tissue->minimum_interphase_time;
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
        float l = (vl->to - vl->from).length();

        // number of tubes to generate...
        int n = round(l/vl->tube_length);
        float ll = l/n;

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
        float y_shift = (vb->from.y + vb->to.y)*0.5 + vb->shift_y;
        float z_shift = (vb->from.z + vb->to.z)*0.5 + vb->shift_z;
        for (float y = y_shift + vb->spacing_y; y <= vb->to.y; y += vb->spacing_y)
        {
            for (float z = z_shift + vb->spacing_z; z <= vb->to.z; z += vb->spacing_z)
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
            for (float z = z_shift; z >= vb->from.z; z -= vb->spacing_z)
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
        for (float y = y_shift; y >= vb->from.y; y -= vb->spacing_y)
        {
            for (float z = z_shift + vb->spacing_z; z <= vb->to.z; z += vb->spacing_z)
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
            for (float z = z_shift; z >= vb->from.z; z -= vb->spacing_z)
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
                if (Cells[first_cell + i].state != sat::csRemoved && Cells[first_cell + i].tissue->type == sat::ttTumor)
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
                            Cells[first_cell + i].concentrations[sat::dsO2][SimulationSettings.step % 2],
                            Cells[first_cell + i].concentrations[sat::dsTAF][SimulationSettings.step % 2],
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
                if (Cells[first_cell + i].state != sat::csRemoved && Cells[first_cell + i].tissue->type == sat::ttNormal)
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
                            Cells[first_cell + i].concentrations[sat::dsO2][SimulationSettings.step % 2],
                            Cells[first_cell + i].concentrations[sat::dsTAF][SimulationSettings.step % 2],
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
}
