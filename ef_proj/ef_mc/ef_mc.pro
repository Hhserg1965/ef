#-------------------------------------------------
#
# Project created by QtCreator 2017-08-31T15:49:37
#
#-------------------------------------------------
QMAKE_CXXFLAGS += -std=c++0x

QT       -= core gui

TARGET = ef_mc
TEMPLATE = lib

DEFINES += EF_MC_LIBRARY

SOURCES += ef_mc.cpp

HEADERS += ef_mc.h

symbian {
    MMP_RULES += EXPORTUNFROZEN
    TARGET.UID3 = 0xE15C6989
    TARGET.CAPABILITY = 
    TARGET.EPOCALLOWDLLDATA = 1
    addFiles.sources = ef_mc.dll
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
LIBS += -lmemcached -lmemcachedutil
