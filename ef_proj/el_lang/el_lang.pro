#-------------------------------------------------
#
# Project created by QtCreator 2017-08-03T13:49:24
#
#-------------------------------------------------

QT       -= core gui

TARGET = el_lang
TEMPLATE = lib

DEFINES += EL_LANG_LIBRARY

SOURCES += el_lang.cpp

HEADERS += el_lang.h

symbian {
    MMP_RULES += EXPORTUNFROZEN
    TARGET.UID3 = 0xE04C07B3
    TARGET.CAPABILITY = 
    TARGET.EPOCALLOWDLLDATA = 1
    addFiles.sources = el_lang.dll
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
