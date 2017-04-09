//#include <windows.h>
//#include <winuser.h>

#include "myqapp.h"
#include "mainwindow.h"

MyQApplication::MyQApplication(int &argc, char **argv): QApplication(argc, argv)
{

}


//bool MyQApplication::winEventFilter(MSG *msg, long */*result*/)
//{
//    if (MainWindowPtr)
//        MainWindowPtr->mouse_3d_event(msg);

//    return false;
//}


//void MyQApplication::register_3Dconnexion(HWND hwndMessageWindow)
//{
//    RAWINPUTDEVICE sRawInputDevices[] = { {0x01, 0x08, 0x00, 0x00} };

//    UINT uiNumDevices = sizeof(sRawInputDevices)/sizeof(sRawInputDevices[0]);

//    UINT cbSize = sizeof (sRawInputDevices[0]);
//    for (size_t i=0; i<uiNumDevices; i++)
//        sRawInputDevices[i].hwndTarget = hwndMessageWindow;

//    if (::RegisterRawInputDevices(sRawInputDevices, uiNumDevices, cbSize))
//        qDebug("Raw input device registered");
//}

