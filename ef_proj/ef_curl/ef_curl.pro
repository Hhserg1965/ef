#-------------------------------------------------
#
# Project created by QtCreator 2017-09-06T14:47:34
#
#-------------------------------------------------
QMAKE_CXXFLAGS += -std=c++0x
QT       -= core gui

TARGET = ef_curl
TEMPLATE = lib

DEFINES += EF_CURL_LIBRARY

SOURCES += ef_curl.cpp

HEADERS += ef_curl.h \
    ../ef.hpp

symbian {
    MMP_RULES += EXPORTUNFROZEN
    TARGET.UID3 = 0xE0FE19DF
    TARGET.CAPABILITY = 
    TARGET.EPOCALLOWDLLDATA = 1
    addFiles.sources = ef_curl.dll
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
unix: LIBS += -L./ -lcurl -lz
