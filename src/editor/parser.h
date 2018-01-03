#ifndef PARSER_H
#define PARSER_H

#include "types.h"
#include "anyvector.h"
#include "transform.h"
#include "color.h"


typedef enum { TT_Ident, TT_String, TT_Number, TT_Vector, TT_Color, TT_Transformation, TT_Symbol, TT_Eof } aTokenType; ///< token type

#define MAX_IDENT_LEN 1024

struct anyToken
 {
  aTokenType type;  ///< type of token
  float number;      ///< number (set if type == TT_Number)
  anyVector vector; ///< vector (set if type == TT_Vector)
  anyColor color;   ///< color (set if type == TT_Color)
  anyTransform transformation; ///< transformation matrix (set if type == TT_Transformation)
  char str[MAX_IDENT_LEN];   ///< string (set if type == TT_String)
  int symbol;       ///< symbol (set if type == TT_Symbol)
 };

struct anyDefinition
{
    char *name;     ///< name
    anyToken value; ///< value

    anyDefinition *next;
};

extern anyDefinition *FirstDefinition;
extern anyDefinition *LastDefinition;

extern anyToken Token;
extern int ParserLine;
extern char const *ParserFile;

int StrCmp(char const *s1, char const *s2);
char *TokenToString(anyToken const &t);
void GetNextToken(FILE *f, bool replace);
void ParseFile(char const *fname, bool store_filename);
void ParseBlock(FILE *f, void val_fun(FILE *));
void SaveDefinitions_ag(FILE *f);
void ReplaceToken(anyToken &t);
void DeallocateDefinitions();
void Slashify(char *s, bool add_slash_at_end);

#define SAVE_float(f, s, fld_name) fprintf(f, s->fld_name < MAX_float ? "  " #fld_name " = %g\n" : "  " #fld_name " = inf\n", s->fld_name)
#define SAVE_float_N(f, s, fld_name, save_name) fprintf(f, s->fld_name < MAX_float ? "  " #save_name " = %g\n" : "  " #save_name " = inf\n", s->fld_name)
#define SAVE_INT(f, s, fld_name) fprintf(f, "  " #fld_name " = %d\n", int(s->fld_name))
#define SAVE_ENUM(f, s, fld_name, valname) fprintf(f, "  " #fld_name " = %s\n", valname[(int)(s->fld_name)])
#define SAVE_VECT(f, s, fld_name) fprintf(f, "  " #fld_name " = <%g, %g, %g>\n", s->fld_name.x, s->fld_name.y, s->fld_name.z)
#define SAVE_COLOR(f, s, fld_name) fprintf(f, "  " #fld_name " = <%g, %g, %g, %g>\n", s->fld_name.rgba_array[0], s->fld_name.rgba_array[1], s->fld_name.rgba_array[2], s->fld_name.rgba_array[3])
#define SAVE_TRANSFORMATION(f, s, fld_name) fprintf(f, "  " #fld_name " = %s\n", s->fld_name.toString())
//#define SAVE_PCHAR(f, s, fld_name) fprintf(f, "  " #fld_name " = %s\n", s->fld_name)
#define SAVE_STRING(f, s, fld_name) fprintf(f, "  " #fld_name " = \"%s\"\n", s->fld_name)


#define PARSE_VALUE_float(object, name) \
     else if (!StrCmp(tv.str, #name)) \
      { if (Token.type == TT_Number) object.name = Token.number; \
        else throw new Error(__FILE__, __LINE__, "Syntax error (number expected)", TokenToString(Token), ParserFile, ParserLine); }

#define PARSE_VALUE_float_N(object, name, save_name) \
     else if (!StrCmp(tv.str, #save_name)) \
      { if (Token.type == TT_Number) object.name = Token.number; \
        else throw new Error(__FILE__, __LINE__, "Syntax error (number expected)", TokenToString(Token), ParserFile, ParserLine); }

#define PARSE_VALUE_INT(object, name) \
     else if (!StrCmp(tv.str, #name)) \
      { if (Token.type == TT_Number) object.name = int(Token.number); \
        else throw new Error(__FILE__, __LINE__, "Syntax error (number expected)", TokenToString(Token), ParserFile, ParserLine); }

#define PARSE_VALUE_BOOL(object, name) \
     else if (!StrCmp(tv.str, #name)) \
      { if (Token.type == TT_Number) object.name = int(Token.number) != 0; \
        else throw new Error(__FILE__, __LINE__, "Syntax error (0 or 1 expected)", TokenToString(Token), ParserFile, ParserLine); }

#define PARSE_VALUE_ENUM(object, casttype, name) \
     else if (!StrCmp(tv.str, #name)) \
      { if (Token.type == TT_Number) object.name = (casttype)int(Token.number); \
        else throw new Error(__FILE__, __LINE__, "Syntax error (number expected)", TokenToString(Token), ParserFile, ParserLine); }

#define PARSE_VALUE_VECTOR(object, name) \
     else if (!StrCmp(tv.str, #name)) \
      { if (Token.type == TT_Vector) object.name = Token.vector; \
        else throw new Error(__FILE__, __LINE__, "Syntax error (vector expected)", TokenToString(Token), ParserFile, ParserLine); }

#define PARSE_VALUE_COLOR(object, name) \
     else if (!StrCmp(tv.str, #name)) \
      { if (Token.type == TT_Vector || Token.type == TT_Color) object.name = Token.color; \
        else throw new Error(__FILE__, __LINE__, "Syntax error (color or vector expected)", TokenToString(Token), ParserFile, ParserLine); }

#define PARSE_VALUE_TRANSFORMATION(object, name) \
     else if (!StrCmp(tv.str, #name)) \
      { if (Token.type == TT_Transformation) object.name = Token.transformation; \
        else throw new Error(__FILE__, __LINE__, "Syntax error (transformation matrix expected)", TokenToString(Token), ParserFile, ParserLine); }

#endif // PARSER_H
