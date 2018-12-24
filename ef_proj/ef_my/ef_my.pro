#-------------------------------------------------
#
# Project created by QtCreator 2017-09-04T10:55:10
#
#-------------------------------------------------
QMAKE_CXXFLAGS += -std=c++0x

QT       -= core gui

TARGET = ef_my
TEMPLATE = lib

DEFINES += EF_MY_LIBRARY

SOURCES += ef_my.cpp

HEADERS += ef_my.h

symbian {
    MMP_RULES += EXPORTUNFROZEN
    TARGET.UID3 = 0xE1B11D7B
    TARGET.CAPABILITY = 
    TARGET.EPOCALLOWDLLDATA = 1
    addFiles.sources = ef_my.dll
    addFiles.path = !:/sys/bin
    DEPLOYMENT += addFiles
}

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
LIBS += -lmysqlclient
