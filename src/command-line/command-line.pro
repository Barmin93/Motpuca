QT -= core gui
CONFIG   += console

SOURCES += \
    main.cpp \
    ../editor/config.cpp \
    ../editor/anybarrier.cpp \
    ../editor/anyboundingbox.cpp \
    ../editor/anycell.cpp \
    ../editor/anycellblock.cpp \
    ../editor/anyglobalsettings.cpp \
    ../editor/anysimulationsettings.cpp \
    ../editor/anytissuesettings.cpp \
    ../editor/anytube.cpp \
    ../editor/anytubebundle.cpp \
    ../editor/anytubeline.cpp \
    ../editor/anytubularsystemsettings.cpp \
    ../editor/anyvector.cpp \
    ../editor/anyvisualsettings.cpp \
    ../editor/color.cpp \
    ../editor/log.cpp \
    ../editor/parser.cpp \
    ../editor/scene.cpp \
    ../editor/simulation.cpp \
    ../editor/statistics.cpp \
    ../editor/timers.cpp \
    ../editor/anyeditable.cpp

INCLUDEPATH += ../Editor

OTHER_FILES += \
    Makefile

HEADERS += \
    ../editor/config.h \
    ../editor/anybarrier.h \
    ../editor/anyboundingbox.h \
    ../editor/anycell.h \
    ../editor/anycellblock.h \
    ../editor/anyglobalsdialog.h \
    ../editor/anyglobalsettings.h \
    ../editor/anysimulationsettings.h \
    ../editor/anytissuesettings.h \
    ../editor/anytube.h \
    ../editor/anytubebox.h \
    ../editor/anytubebundle.h \
    ../editor/anytubeline.h \
    ../editor/anytubemerge.h \
    ../editor/anytubularsystemsettings.h \
    ../editor/anyvector.h \
    ../editor/anyvisualsettings.h \
    ../editor/color.h \
    ../editor/const.h \
    ../editor/func.h \
    ../editor/log.h \
    ../editor/parser.h \
    ../editor/scene.h \
    ../editor/simulation.h \
    ../editor/statistics.h \
    ../editor/timers.h \
    ../editor/transform.h \
    ../editor/types.h \
    ../editor/version.h \
    ../editor/anyeditable.h \
    ../editor/anyeditabledialog.h
