QT -= core
QT -= gui

CONFIG += c++11

TARGET = ocvtest
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp

INCLUDEPATH += C:\ef\inc\

INCLUDEPATH += C:\CAFFE_OPENCV\OpenCV\opencv_binary_2_4_13_4\build\include\

#QMAKE_CXXFLAGS += -fPIC
#QMAKE_CFLAGS += -fPIC

#LIBS += -lmemcached -lmemcachedutil -L/usr/lib/nptl -L../ef_lang -lpthread -lef_lang
#LIBS +=  -L../ef_lang -lef_lang

#LIBS += -LC:\CAFFE_OPENCV\OpenCV\opencv-2.4.13.4\build\lib\Release\ -lopencv_core2413 -lopencv_gpu2413 -lopencv_ml2413
LIBS += -LC:\CAFFE_OPENCV\OpenCV\opencv_binary_2_4_13_4\build\x64\vc14\lib -lopencv_core2413 -lopencv_gpu2413 -lopencv_ml2413
#LIBS +=   -LC:\ef\lib\ -llibuv -lpcre2-8 -lws2_32 -llibpq -lAdvapi32 -lPsapi -lIphlpapi -lUserenv -lUser32

#HEADERS +=

HEADERS +=

