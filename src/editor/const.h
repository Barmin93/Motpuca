#ifndef CONST_H
#define CONST_H

#define APP_NAME "Motpuca" // version defined in version.h

#define FOLDER_USER "Motpuca/"
#define FOLDER_INPUT_FILES "data/"
#define FOLDER_OUTPUT "output/"
#define FOLDER_DEFAULTS "defaults/"
#define FOLDER_LIB_TISSUES "lib/tissues/"
#define FOLDER_LIB_POVRAY "lib/povray/"

#define P_MAX_PATH 1024

#ifdef unix
#define DIRSLASH '/'
#else
#define DIRSLASH '\\'
#endif

#endif // CONST_H
