#ifndef EF_OCV_H
#define EF_OCV_H

#include <../ef.hpp>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <cassert>


//#include <opencv2/core/core.hpp>
//#include <opencv2/features2d/features2d.hpp>
//#include <opencv2/nonfree/features2d.hpp>

//#include "opencv2/objdetect/objdetect.hpp"
//#include "opencv2/imgproc/imgproc.hpp"
//#include "opencv2/highgui/highgui.hpp"
//#include <opencv2/ml/ml.hpp>

//using namespace cv;

#ifdef _WIN32
#define MY_EXPORT __declspec(dllexport)
#else
#define MY_EXPORT
#endif


extern "C" {
    MY_EXPORT void init(ef_Content &ec);
}

#endif // EF_OCV_H
