#ifndef MYQAPP_H
#define MYQAPP_H

#include <QApplication>

class MyQApplication: public QApplication
{
    Q_OBJECT

public:
    MyQApplication(int &argc, char **argv);
    ~MyQApplication() {}

//    void register_3Dconnexion(HWND hwndMessageWindow);

//protected:
//    bool winEventFilter(MSG *, long *result);
};

#endif // MYQAPP_H
