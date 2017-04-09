QT -= core gui
CONFIG   += console

SOURCES += \
    main.cpp \
    ../Editor/scene.cpp \
    ../Editor/config.cpp \
    ../Editor/vector.cpp \
    ../Editor/log.cpp \
    ../Editor/simulation.cpp \
    ../Editor/timers.cpp \
    ../Editor/statistics.cpp \
    ../Editor/color.cpp \
    ../Editor/parser.cpp

INCLUDEPATH += ../Editor

OTHER_FILES += \
    Makefile

HEADERS += \
    ../Editor/config.h \
    ../Editor/types.h \
    ../Editor/vector.h \
    ../Editor/scene.h \
    ../Editor/log.h \
    ../Editor/const.h \
    ../Editor/func.h \
    ../Editor/simulation.h \
    ../Editor/timers.h \
    ../Editor/transform.h \
    ../Editor/version.h \
    ../Editor/color.h \
    ../Editor/statistics.h \
    ../Editor/model.h \
    ../Editor/parser.h
