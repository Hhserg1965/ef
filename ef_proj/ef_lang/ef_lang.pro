#-------------------------------------------------
#
# Project created by QtCreator 2017-08-03T13:51:07
#
#-------------------------------------------------

QT       -= core gui

TARGET = ../../bin/ef_lang
TEMPLATE = lib
QMAKE_CXXFLAGS += -std=c++0x

#QMAKE_CXXFLAGS += -std=c++0x

INCLUDEPATH += ../../inc
INCLUDEPATH += ../ef_proj/utf
DEFINES += EF_LANG_LIBRARY

SOURCES += ef_lang.cpp \
    ../utf/translators.c \
    ../utf/stringutils.c \
    ../utf/chartables.c \
    uued.cpp \
    ../../rpi_sock.cpp \
    ../utf/utf8uclc.c \
    ef_spawn.cpp

HEADERS += ef_lang.h \
    ../ef.hpp \
    ../utf/translators.h \
    ../utf/stringutils.h \
    ../utf/chartables.h \
    ../utf/chacu.h \
    ../../rpi_sock.h

symbian {
    MMP_RULES += EXPORTUNFROZEN
    TARGET.UID3 = 0xE0028B59
    TARGET.CAPABILITY = 
    TARGET.EPOCALLOWDLLDATA = 1
    addFiles.sources = ef_lang.dll
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
#LIBS +=   -LC:\ef\lib -luuid -liconv -lpcre2-8
#LIBS +=   -LC:\ef\lib\ -llibuv -lpcre2-8 -lws2_32 -llibpq -lAdvapi32 -lPsapi -lIphlpapi -lUserenv -lUser32
# /NODEFAULTLIB:library
unix: LIBS += -L../../lib -lpcre2-8  -lz -luv -lpq
