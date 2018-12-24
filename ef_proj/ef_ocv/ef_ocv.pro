#-------------------------------------------------
#
# Project created by QtCreator 2017-09-14T13:45:21
#
#-------------------------------------------------
QMAKE_CXXFLAGS += -std=c++0x
QT       -= core gui

TARGET = ef_ocv
TEMPLATE = lib

DEFINES += EF_OCV_LIBRARY

SOURCES += ef_ocv.cpp

HEADERS += ef_ocv.h

INCLUDEPATH += C:\CAFFE_OPENCV\OpenCV\opencv-2.4.13.4\inc\

symbian {
    MMP_RULES += EXPORTUNFROZEN
    TARGET.UID3 = 0xE054E6F5
    TARGET.CAPABILITY = 
    TARGET.EPOCALLOWDLLDATA = 1
    addFiles.sources = ef_ocv.dll
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

#unix:LIBS +=   -lopencv_core -lopencv_gpu -lopencv_imgproc -lopencv_objdetect -lopencv_video
#LIBS += -LC:\CAFFE_OPENCV\OpenCV\opencv-2.4.13.4\build\lib\Release\ -lopencv_core2413 -lopencv_gpu2413 -lopencv_imgproc2413 -lopencv_objdetect2413 -lopencv_video2413 -lopencv_ml2413
LIBS += -LC:\CAFFE_OPENCV\OpenCV\opencv-2.4.13.4\build\lib\Release\ -lopencv_core2413 -lopencv_gpu2413 -lopencv_ml2413
