QT += opengl
DEFINES += _WIN32_WINNT=0x501
DEFINES += GUI=1

TEMPLATE = app
HEADERS += mainwindow.h \
    glwidget.h \
    config.h \
    types.h \
    vector.h \
    scene.h \
    log.h \
    const.h \
    func.h \
    simulation.h \
    timers.h \
    transform.h \
    version.h \
    color.h \
    graphwidget.h \
    statistics.h \
    model.h \
    columnresizer.h \
    myqapp.h \
    parser.h \
    glmodel.h \
    openglstaff.h \
    glshaderprogram.h \
    anytube.h
SOURCES += mainwindow.cpp \
    glwidget.cpp \
    main.cpp \
    scene.cpp \
    config.cpp \
    vector.cpp \
    log.cpp \
    simulation.cpp \
    timers.cpp \
    statistics.cpp \
    color.cpp \
    columnresizer.cpp \
    myqapp.cpp \
    parser.cpp \
    glmodel.cpp \
    glshaderprogram.cpp \
    anytube.cpp
FORMS += mainwindow.ui \
    dialogEditable.ui \
    dialogGlobals.ui \
    dialogRun.ui
win32:RC_FILE = editor.rc

QMAKE_CFLAGS_RELEASE = -O3
QMAKE_CXXFLAGS_RELEASE = -O3
QMAKE_CXXFLAGS_WARN_OFF -= -Wunused-parameter

OTHER_FILES +=

DISTFILES += \
    ../../include/shadersInstanced.glsl \
    ../../include/shadersOverlay.glsl \
    ../../include/shadersWireframe.glsl \
    ../../include/shadersSimple.glsl
