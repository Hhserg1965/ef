#-------------------------------------------------
#
# Project created by QtCreator 2017-08-03T13:58:53
#
#-------------------------------------------------

QT       -= core
#QMAKE_CXXFLAGS += -std=c++0x
QT       -= gui
#CONFIG += c++11
#QMAKE_CXXFLAGS += /MD
#QMAKE_CXXFLAGS -= /MT

QMAKE_CXXFLAGS += -std=c++0x

#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../LIBRARYNAME/Lib/ -lLIBRARY /NODEFAULTLIB:library

TARGET = ../../bin/el
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    ../../rpi_sock.cpp \
    ../sha1.c \
    ../ef_lang/ef_lang.cpp \
    ../ef_lang/uued.cpp \
    ../utf/translators.c \
    ../utf/stringutils.c \
    ../utf/chartables.c \
    ../ef_lang/ef_spawn.cpp

#DEFINES += _REENTRANT

INCLUDEPATH += ../../inc

#INCLUDEPATH += C:\CAFFE_OPENCV\OpenCV\opencv-2.4.13.4\inc\
#INCLUDEPATH += C:\CAFFE_OPENCV\OpenCV\opencv_binary_2_4_13_4\build\include\

#QMAKE_CXXFLAGS += -fPIC
#QMAKE_CFLAGS += -fPIC

#LIBS += -lmemcached -lmemcachedutil -L/usr/lib/nptl -L../ef_lang -lpthread -lef_lang
#LIBS +=  -L../ef_lang -lef_lang

#LIBS += -LC:\CAFFE_OPENCV\OpenCV\opencv-2.4.13.4\build\lib\Release\ -lopencv_core2413 -lopencv_gpu2413 -lopencv_ml2413

win32:LIBS += -LC:\CAFFE_OPENCV\OpenCV\opencv_binary_2_4_13_4\build\x64\vc14\lib -lopencv_core2413 -lopencv_gpu2413 -lopencv_ml2413
win32:LIBS +=   -LC:\ef\lib\ -llibuv -lpcre2-8 -lws2_32 -llibpq -lAdvapi32 -lPsapi -lIphlpapi -lUserenv -lUser32

#LIBS += /NODEFAULTLIB:library

#DEFINES += __GXX_EXPERIMENTAL_CXX0X__

unix: LIBS += -L../../lib -lpcre2-8  -lz -luv -lpq

HEADERS += \
    ../../rpi_sock.h \
    ../sha1.h \
    ../ef_lang/ef_lang.h \
    ../utf/translators.h \
    ../utf/stringutils.h \
    ../utf/chartables.h
#FORCE
#before_build.commands = chcp 1251
