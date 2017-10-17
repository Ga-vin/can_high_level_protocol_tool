FORMS += \
    maingui.ui

HEADERS += \
    maingui.h \
    sockcandata.h \
    sockcanframe.h \
    rxtask.h

SOURCES += \
    maingui.cpp \
    main.cpp \
    sockcandata.cpp \
    sockcanframe.cpp \
    rxtask.cpp
CONFIG += \
    thread

QT     += \
    network

RESOURCES += \
    maingui.qrc
