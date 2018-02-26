QT += opengl
CONFIG   += console

TEMPLATE += app


SOURCES += \
    main.cpp \
    ../Editor/mainwindow.cpp \
    ../Editor/columnresizer.cpp \
    ../Editor/glwidget.cpp \
    ../Editor/glmodel.cpp \
    ../Editor/glshaderprogram.cpp \
    ../Editor/scene.cpp \
    ../Editor/config.cpp \
    ../Editor/anyvector.cpp \
    ../Editor/log.cpp \
    ../Editor/simulation.cpp \
    ../Editor/timers.cpp \
    ../Editor/statistics.cpp \
    ../Editor/color.cpp \
    ../Editor/parser.cpp \
    ../Editor/anytube.cpp \
    ../Editor/anycell.cpp \
    ../Editor/anyboundingbox.cpp \
    ../Editor/anyeditabledialog.cpp \
    ../Editor/anyeditable.cpp \
    ../Editor/anytissuesettings.cpp \
    ../Editor/anycellblock.cpp \
    ../Editor/anybarrier.cpp \
    ../Editor/anytubebundle.cpp \
    ../Editor/anytubeline.cpp

INCLUDEPATH += ../Editor

OTHER_FILES +=

HEADERS += \
    ../Editor/mainwindow.h \
    ../Editor/columnresizer.h \
    ../Editor/glwidget.h \
    ../Editor/glmodel.h \
    ../Editor/glshaderprogram.h \
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
    ../Editor/parser.h \
    ../Editor/anytube.h \
    ../Editor/anycell.h \
    ../Editor/anyboundingbox.h \
    ../Editor/anyeditabledialog.h \
    ../Editor/anyeditable.h \
    ../Editor/anytissuesettings.h \
    ../Editor/anycellblock.h \
    ../Editor/anybarrier.h \
    ../Editor/anytubebundle.h \
    ../Editor/anytubeline.h \
    ../Editor/anytubebox.h \
