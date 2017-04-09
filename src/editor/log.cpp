#include <string.h>
#include <stdio.h>

#include "log.h"
#include "config.h"

#ifdef QT_CORE_LIB
#include "mainwindow.h"
#endif

Error::Error(const char *_src_src, int _src_line, const char *_msg, const char *_filename, char const *_msg_more, int _line):
        msg(0), msg_more(0), filename(0), src_src(0)
{
    if (_msg)
    {
        msg = new char[strlen(_msg) + 1];
        strcpy(msg, _msg);
    }

    if (_msg_more)
    {
        msg_more = new char[strlen(_msg_more) + 1];
        strcpy(msg_more, _msg_more);
    }

    if (_filename)
    {
        filename = new char[strlen(_filename) + 1];
        strcpy(filename, _filename);
    }

    if (_src_src)
    {
        src_src = new char[strlen(_src_src) + 1];
        strcpy(src_src, _src_src);
    }

    line = _line;
    src_line = _src_line;
}


Error::~Error()
{
    delete [] msg;
    delete [] msg_more;
    delete [] src_src;
}


void Log(LogLevel log_level, int line, char const *file, char const *msg, char const *msg2, char const *msg3)
/**
  Puts info to message window.

  \param log_level -- level of info; info is processed only in log_level >= LOG_LEVEL
  \param line -- line in source file (__LINE__)
  \param file -- source file (__FILE__)
  \param msg -- info
  \param msg2 -- more info
  \param msg3 -- even more info
*/
{
    if (log_level < LOG_LEVEL) return;

    char m[10240];

    snprintf(m, 10240, " %s%s%s", msg, msg2 ? msg2 : "", msg3 ? msg3 : "");

#ifdef QT_CORE_LIB
    MainWindowPtr->msg(line, file, m);
#else
    if (GlobalSettings.debug)
        printf("%s:%d: %s\n", file, line, m);
    else
        printf("%s\n", m);
#endif
}


void LogError(Error *err)
/**
  Puts error information to message window.

  \param err -- pointer to Error
*/
{
    char s[10240];

    snprintf(s, 10240, "ERROR: %s", err->msg);

    if (err->filename)
    {
        snprintf(s + strlen(s), 10240 - strlen(s), ": %s", err->filename);
        if (err->line)
            snprintf(s + strlen(s), 10240 - strlen(s), ", Line %d", err->line);
    }
    if (err->msg_more)
        snprintf(s + strlen(s), 10240 - strlen(s), ", At '%s'", err->msg_more);

#ifdef QT_CORE_LIB
    MainWindowPtr->msg(err->src_line, err->src_src, QObject::tr("ERROR: ") + s);
    QMessageBox::warning(0, "Error", s, "Dismiss");
#else
    printf("%s:%d: %s\n", err->src_src, err->src_line, s);
#endif
}


char *SecToString(int s)
/**
  Converts time in seconds to human notation.

  \param s -- number of seconds
*/
{
    int days = s/86400;
    s = s%86400;

    int hours = s/3600;
    s = s%3600;

    int min = s/60;
    s = s%60;

    static char ret[100];
    snprintf(ret, 100, "%dd, %02d:%02d:%02d", days, hours, min, s);

    return ret + 4*!days;
}
