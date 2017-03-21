/**
  Parser.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include "const.h"
#include "parser.h"
#include "log.h"
#include "config.h"

static const char *char_newline      = "\n"; ///< newline characters
static const char *char_ident_start  = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"; ///< ident starting characters
static const char *char_ident        = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789"; ///< ident characters
static const char *char_string_startend = "\""; ///< string start/end character
static const char *char_number_start = "0123456789.-+"; ///< number starting characters
static const char *char_number       = "0123456789.-+eE"; ///< nymber characters
static const char *char_symbol       = ",<>{}="; ///< symbol characters
static const char *char_comment_start = "/#"; ///< comment starting characters

int ParserLine = 1;   ///< line number of currently parsed file
char const *ParserFile = 0; ///< name of currently parsed file
anyToken Token;              ///< parsed token filled by GetNextToken()
anyDefinition *FirstDefinition = 0, *LastDefinition = 0;

static
char lowercase(char c)
{
    return (c >= 'A' && c <= 'Z') ? c - 'A' + 'a': c;
}


int StrCmp(char const *s1, char const *s2)
/**
  Case insensitive string compare.

  \param s1 -- first string
  \param s2 -- second string
*/
{
    if (!s1 && s2) return -1;
    else if (s1 && !s2) return 1;
    else if (!s1 && !s2) return 0;

    while (*s1 && *s2 && lowercase(*s1) == lowercase(*s2))
    {
        s1++;
        s2++;
    }
    return *s1 - *s2;
}



anyDefinition *FindDefinition(char const *name)
/**
  Finds definition.

  \param name -- name of definition
*/
{
    anyDefinition *d = FirstDefinition;

    while (d)
    {
        if (!StrCmp(d->name, name))
            return d;
        d = d->next;
    }
    return 0;
}


void AddDefinition(char const *name, anyToken const &t)
/**
  Adds definition to list of definitions.

  \param name -- name
  \param t -- value
*/
{
    anyDefinition *d = FindDefinition(name);

    if (d)
    {
        // replace value...
        d->value = t;
    }
    else
    {
        // new definition...
        d = new anyDefinition;

        d->name = new char[strlen(name) + 1];
        strcpy(d->name, name);
        d->value = t;

        if (!FirstDefinition)
            FirstDefinition = LastDefinition = d;
        else
        {
            LastDefinition->next = d;
            LastDefinition = d;
        }
        d->next = 0;
    }
}


void ReplaceToken(anyToken &t)
/**
  Replaces token if 'set' definition is found for ident.
*/
{
    if (t.type == TT_Ident)
    {
        anyDefinition *d = FindDefinition(t.str);
        if (d)
            t = d->value;
    }
}


void SaveDefinitions_ag(FILE *f)
/**
  Saves all definitions.

  \param f -- output file
*/
{
    anyDefinition *d = FirstDefinition;
    while (d)
    {
        fprintf(f, "set %s = %s\n", d->name, TokenToString(d->value));
        d = d->next;
    }
}


void DeallocateDefinitions()
/**
  Dealocates all definitions.
*/
{
    anyDefinition *d = FirstDefinition, *dn;

    while (d)
    {
        dn = d->next;
        delete d;
        d = dn;
    }
    FirstDefinition = LastDefinition = 0;
}


char *TokenToString(anyToken const &t)
/**
 Returns char * token representation.

 \param t -- token to convert
*/
{
    static char s[MAX_IDENT_LEN + 11];
    switch (t.type)
    {
    case TT_Eof:
        snprintf(s, 100, "EOF"); break;
    case TT_Number:
        snprintf(s, 100, "%g", t.number); break;
    case TT_String:
        snprintf(s, 100, "\"%s\"", t.str); break;
    case TT_Ident:
        snprintf(s, 100, "%s", t.str); break;
    case TT_Vector:
        snprintf(s, 100, "<%g, %g, %g>", t.vector.x, t.vector.y, t.vector.z); break;
    case TT_Color:
        snprintf(s, 100, "<%g, %g, %g, %g>", t.color.rgba_array[0], t.color.rgba_array[1], t.color.rgba_array[2], t.color.rgba_array[3]); break;
    case TT_Transformation:
        snprintf(s, 300, "%s", t.transformation.toString()); break;
    case TT_Symbol:
        snprintf(s, 100, "%c", t.symbol); break;
    }
    return s;
}


static
int p_getc(FILE *f)
/**
 Gets one character form input file and increases line counter if necessery.

 \param f -- input file
*/
{
    int c = getc(f);
    if (strchr(char_newline, c))
        ParserLine++;
    return c;
}


static
void p_ungetc(int c, FILE *f)
/**
 Ungets one character to input file and decreases line counter if necessary.

 \param c -- character to unget
 \param f -- input file
*/
{
    ungetc(c, f);
    if (strchr(char_newline, c))
        ParserLine--;
}



static
void skip_white_space(FILE *f)
/**
 Skips white space (including comment). If comment is encountered
 it is skipped and then function is called recursively to skip
 white space after comment.

 \param f -- input file
*/
{
    int c;

    // white space...
    while ((c = p_getc(f)) != EOF && c <= ' ')
        ;

    // comment?...
    if (strchr(char_comment_start, c))
    {
        while ((c = p_getc(f)) != EOF && !strchr(char_newline, c))
            ;
        skip_white_space(f);
    }
    else if (c != EOF) p_ungetc(c, f);
}


static
void get_next_token_number(FILE *f)
/**
 Gets next token as number (first character must be from char_number_start).

 \param f -- input file
*/
{
    int c, i = 0;
    char str[100];

    // collect characters...
    while ((c = p_getc(f)) != EOF && strchr(char_number, c))
        if (i < 100)
            str[i++] = c;
    str[i] = 0;
    if (c != EOF)
        p_ungetc(c, f);

    // store token...
    Token.type = TT_Number;
    Token.number = atof(str);
}


static
void get_next_token_vector_or_color_or_transformation(FILE *f)
/**
 Gets next token as vector or transformation matrix (first character must be '<' and be already skipped).

 \param f -- input file
*/
{
    anyTransform trans;

    int i = 0;
    while (i < 16)
    {
        // get next value...
        GetNextToken(f, true);
        if (Token.type != TT_Number)
            throw new Error(__FILE__, __LINE__, "Bad vector definition (number expected)", TokenToString(Token), ParserFile, ParserLine);
        trans.matrix[i] = Token.number;

        // get comma or '>'...
        GetNextToken(f, false);
        if (Token.type == TT_Symbol && Token.symbol == '>')
            break;

        if (Token.type != TT_Symbol || Token.symbol != ',')
            throw new Error(__FILE__, __LINE__, "Bad vector definition (comma expected)", TokenToString(Token), ParserFile, ParserLine);

        i++;
    }
    if (i < 3)
    {
        Token.type = TT_Vector;
        Token.vector.x = trans.matrix[0];
        Token.vector.y = trans.matrix[1];
        Token.vector.z = trans.matrix[2];
        for (int i = 0; i < 3; i++)
          Token.color.rgba_array[i] = trans.matrix[i];
        Token.color.rgba_array[3] = 1;
    }
    else if (i < 4)
    {
        Token.type = TT_Color;
        for (int i = 0; i < 4; i++)
          Token.color.rgba_array[i] = trans.matrix[i];
    }
    else
    {
        Token.type = TT_Transformation;
        Token.transformation = trans;
    }
}


static
void get_next_token_ident(FILE *f, bool replace)
/**
 Gets next token as ident (first character must be from char_ident_start).

 \param f -- input file
 \param replace -- replace token with defined value?
*/
{
    int c, i = 0;

    while ((c = p_getc(f)) != EOF && strchr(char_ident, c))
        if (i < 1024)
            Token.str[i++] = c;
    Token.str[i] = 0;
    if (c != EOF)
        p_ungetc(c, f);

    if (!StrCmp(Token.str, "inf"))
    {
        Token.number = MAX_REAL;
        Token.type = TT_Number;
    }
    else
    {
        Token.type = TT_Ident;
        if (replace)
            ReplaceToken(Token);
    }
}


static
void get_next_token_string(FILE *f)
/**
 Gets next token as string (first character must be from char_string_startend).

 \param f -- input file
*/
{
    int c, i = 0;

    while ((c = p_getc(f)) != EOF && !strchr(char_string_startend, c))
        if (i < 1024)
            Token.str[i++] = c;
    Token.str[i] = 0;
    Token.type = TT_String;
}


void GetNextToken(FILE *f, bool replace)
/**
 Gets next token.

 \param f -- input file
 \param replace -- replace token with defined value?
*/
{
    int c;
    skip_white_space(f);

    c = p_getc(f);

    if (c == EOF)
        Token.type = TT_Eof;
    else if (strchr(char_number_start, c))
    {
        p_ungetc(c, f);
        get_next_token_number(f);
    }
    else if (strchr(char_ident_start, c))
    {
        p_ungetc(c, f);
        get_next_token_ident(f, replace);
    }
    else if (strchr(char_string_startend, c))
    {
        get_next_token_string(f);
    }
    else if (c == '<')
        get_next_token_vector_or_color_or_transformation(f);
    else if (strchr(char_symbol, c))
    {
        Token.type = TT_Symbol;
        Token.symbol = c;
    }
    else
        throw new Error(__FILE__, __LINE__, "Unexpected symbol", TokenToString(Token), ParserFile, ParserLine);
}



void ParseBlock(FILE *f, void val_fun(FILE *))
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
            val_fun(f);

        // end of body?...
        else if (Token.type == TT_Symbol && Token.symbol == '}')
            break;

        // end of input file?...
        else if (Token.type == TT_Eof)
            throw new Error(__FILE__, __LINE__, "Unexpected end of file", TokenToString(Token), ParserFile, ParserLine);

        else
            throw new Error(__FILE__, __LINE__, "Unexpected token (not string)", TokenToString(Token), ParserFile, ParserLine);
    }
}


void ParseDefinedValue(FILE *f)
/**
  Parses 'set name = value' phrase.

  \param f -- input file
*/
{
    // get name...
    GetNextToken(f, false);
    if (Token.type != TT_Ident)
        throw new Error(__FILE__, __LINE__, "Syntax error (name of defined value expected)", TokenToString(Token), ParserFile, ParserLine);
    char *name = Token.str;

    // get '='...
    GetNextToken(f, false);
    if (Token.type != TT_Symbol || Token.symbol != '=')
        throw new Error(__FILE__, __LINE__, "Syntax error ('=' expected)", TokenToString(Token), ParserFile, ParserLine);

    // get value...
    GetNextToken(f, true);

    AddDefinition(name, Token);
}


void ParseFile(char const *fname, bool store_filename)
/**
 Parses input file.

 \param fname -- input file name
 \param store_filename -- store file name in global settings?
*/
{
    LOG2(llInfo, "Reading input file: ", fname);

    // open file...
    ParserFile = fname;
    FILE *f = fopen(fname, "r");
    if (!f)
    {
        char wd[1001];
        getcwd(wd, 1000);
        throw new Error(__FILE__, __LINE__, "Cannot open file for reading", fname /*ParserFile*/);
    }

    try
    {
        // parse file...
        ParserLine = 1;
        while (23)
        {
            GetNextToken(f, false);

            if (Token.type == TT_Eof) break;

            if (Token.type != TT_Ident)
                throw new Error(__FILE__, __LINE__, "Unexpected token (not string)", TokenToString(Token), ParserFile, ParserLine);
            else if (!StrCmp(Token.str, "set"))
                ParseDefinedValue(f);

            else if (!StrCmp(Token.str, "visual"))
                ParseVisualSettings(f);

            else if (!StrCmp(Token.str, "tubularsystem"))
                ParseTubularSystemSettings(f);

            else if (!StrCmp(Token.str, "simulation"))
            {
                ParseSimulationSettings(f);
                DeallocSimulation();
//                AllocSimulation();
            }

            else if (!StrCmp(Token.str, "tissue"))
                ParseTissueSettings(f, new anyTissueSettings, true);

            else if (!StrCmp(Token.str, "barrier"))
                ParseBarrier(f, new anyBarrier, true);

            else if (!StrCmp(Token.str, "cellblock"))
                ParseCellBlock(f, new anyCellBlock, true);

            else if (!StrCmp(Token.str, "cell"))
                ParseCell(f);

            else if (!StrCmp(Token.str, "tube"))
                ParseTube(f);

            else if (!StrCmp(Token.str, "tubeline"))
                ParseTubeLine(f, new anyTubeLine, true);

            else if (!StrCmp(Token.str, "tubebundle"))
                ParseTubeBundle(f, new anyTubeBundle, true);

            else
                throw new Error(__FILE__, __LINE__, "Unexpected keyword", TokenToString(Token), ParserFile, ParserLine);
        }
    }
    catch (...)
    {
        // close file and rethrow...
        if (f) fclose(f);
        throw;
    }

    fclose(f);

    RelinkTubes();

    // store filename and output directory...
    if (store_filename)
    {
        strncpy(GlobalSettings.input_file, fname, P_MAX_PATH);
        LOG2(llInfo, "Input file set to: ", GlobalSettings.input_file);

        // find file name and store as output dir...
        char fname[P_MAX_PATH];
        int i = strlen(GlobalSettings.input_file) - 1;
        while (i && GlobalSettings.input_file[i] != '/' && GlobalSettings.input_file[i] != '\\')
            i--;
        strncpy(fname, GlobalSettings.input_file + i + (!!i), P_MAX_PATH);

        // remove extension from output dir...
        i = strlen(fname) - 1;
        while (i && fname[i] != '.')
            i--;
        if (i)
            fname[i] = 0;

        // store ouput dir and create it...
        snprintf(GlobalSettings.output_dir, P_MAX_PATH, "%s%s%s/", GlobalSettings.user_dir, FOLDER_OUTPUT, fname);
        Slashify(GlobalSettings.output_dir, false);

        mkdir(GlobalSettings.output_dir, 0777);

        LOG2(llInfo, "Output directory set to: ", GlobalSettings.output_dir);
    }
}


void Slashify(char *s, bool add_slash_at_end)
{
    int l = strlen(s);
    for (int i = 0; i < l; i++)
        if (s[i] == '/' || s[i] == '\\')
            s[i] = DIRSLASH;

    if (add_slash_at_end && s[l - 1] != DIRSLASH)
    {
        s[l] = DIRSLASH;
        s[l + 1] = 0;
    }
}
