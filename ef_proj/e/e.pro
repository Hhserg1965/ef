#-------------------------------------------------
#
# Project created by QtCreator 2017-08-03T13:58:53
#
#-------------------------------------------------

QT       -= core
QMAKE_CXXFLAGS += -std=c++0x
QT       -= gui

TARGET = e
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app



SOURCES += main.cpp \
    ../../rpi_sock.cpp \
    ../sha1.c

DEFINES += _REENTRANT

INCLUDEPATH += C:\ef\inc\

#QMAKE_CXXFLAGS += -fPIC
#QMAKE_CFLAGS += -fPIC

#LIBS += -lmemcached -lmemcachedutil -L/usr/lib/nptl -L../ef_lang -lpthread -lef_lang
LIBS +=  -L../ef_lang -lef_lang

DEFINES += __GXX_EXPERIMENTAL_CXX0X__

unix: LIBS += -lz -luv -lpq

HEADERS += \
    ../../rpi_sock.h \
    ../sha1.h
 FORCE
before_build.commands = chcp 1251
