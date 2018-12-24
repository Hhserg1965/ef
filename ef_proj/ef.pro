TEMPLATE = app
CONFIG += console
CONFIG -= qt

QMAKE_CXXFLAGS += -std=c++0x
INCLUDEPATH += ../inc
TARGET = ../bin/ef

SOURCES += \
    sha1.c \
    b64.cpp \
    el/main.cpp

#unix: LIBS += -lcurl -lz -luv -lv8

#LIBS += -lmc18
#CFLAGS=-D_REENTRANT
#LDFLAGS=-lpthread
DEFINES += _REENTRANT

#QMAKE_CXXFLAGS += -fPIC
#QMAKE_CFLAGS += -fPIC

#LIBS += -lmemcached -lmemcachedutil -L/usr/lib/nptl -lpthread

DEFINES += __GXX_EXPERIMENTAL_CXX0X__

unix: LIBS += -L../bin -lef_lang -lz -luv -lpq -L../../lib -lpcre2-8

HEADERS += \
    ../hh_shmem.h \
    sha1.h \
    ef.hpp
