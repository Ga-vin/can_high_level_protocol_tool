FORMS += \
    maingui.ui

HEADERS += \
    maingui.h \
    sockcandata.h \
    sockcanframe.h \
    rxtask.h \
    cerr.h \
    errrxtask.h

SOURCES += \
    maingui.cpp \
    main.cpp \
    sockcandata.cpp \
    sockcanframe.cpp \
    rxtask.cpp \
    cerr.cpp \
    errrxtask.cpp
CONFIG += \
    thread

QT     += \
    network

RESOURCES += \
    maingui.qrc
