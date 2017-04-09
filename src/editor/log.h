#ifndef LOG_H
#define LOG_H

enum LogLevel { llDebug, llInfo , llError};

#define LOG_LEVEL llDebug

class Error
{
public:
    char *msg;       ///< error message
    char *msg_more;  ///< additional info
    char *filename;  ///< name of file cousing error
    int line;        ///< line in file where error occured

    char *src_src;   ///< name of source file where error was detected (__FILE__)
    int src_line;      ///< line if source file where error was detected (__LINE__)

    Error(char const *_src_src, int _src_line, char const *_msg, char const *_filename = 0, char const *_msg_more = 0, int _line = 0);
    ~Error();
};

void Log(LogLevel log_level, int line, char const *file, char const *msg, char const *msg2 = 0, char const *msg3 = 0);
void LogError(Error *err);
char *SecToString(int s);

#define LOG(ll, msg) Log(ll, __LINE__, __FILE__, msg)
#define LOG2(ll, msg, msg2) Log(ll, __LINE__, __FILE__, msg, msg2)
#define LOG3(ll, msg, msg2, msg3) Log(ll, __LINE__, __FILE__, msg, msg2, msg3)

#endif // LOG_H
