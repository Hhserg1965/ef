#-------------------------------------------------
#
# Project created by QtCreator 2017-08-02T10:29:35
#
#-------------------------------------------------

QT       -= core gui

TARGET = ../../bin/ef_lev
TEMPLATE = lib

QMAKE_CXXFLAGS += -std=c++0x
INCLUDEPATH += ../../inc
#DEFINES += EF_LEV_LIBRARY

SOURCES += ef_lev.cpp

HEADERS += ef_lev.h \
    ../ef.hpp \
    ../utf/chacu.h \
    ../utf/chartables.h \
    ../utf/stringutils.h \
    ../utf/translators.h

symbian {
    MMP_RULES += EXPORTUNFROZEN
    TARGET.UID3 = 0xE053DF3A
    TARGET.CAPABILITY = 
    TARGET.EPOCALLOWDLLDATA = 1
    addFiles.sources = ef_lev.dll
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
unix:LIBS += -L../../lib -lleveldb -lpcre2-8
