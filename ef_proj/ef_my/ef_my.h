#ifndef EF_MY_H
#define EF_MY_H

#include <../ef.hpp>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#define MY_EXPORT __declspec(dllexport)
#else
#define MY_EXPORT
#endif

extern "C" {
    MY_EXPORT void init(ef_Content &ec);
}

#endif // EF_MY_H
