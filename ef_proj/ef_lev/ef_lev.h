#ifndef EF_LEV_H
#define EF_LEV_H
#include <../ef.hpp>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <cassert>
#include "leveldb/db.h"
#include "leveldb/cache.h"
#include "leveldb/filter_policy.h"
#include "leveldb/write_batch.h"
#include "leveldb/options.h"
#include "leveldb/comparator.h"

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

#ifdef _WIN32
#define MY_EXPORT __declspec(dllexport)
#else
#define MY_EXPORT
#endif


class efKV_C
{
public:

    leveldb::DB* db;
    leveldb::Options db_op;

    leveldb::DB* lg;
    leveldb::Options lg_op;

    long lg_max;

    std::string nm;
    std::string fnm;

    std::string kf;
    std::string kl;

    efKV_C(){
        lg_max = 0;
        db = lg = NULL;
        kf="\0377\0377\0377\0377\0377\0377\0377\0377\0377\0377\0377\0377\0377\0377\0377\0377\0377\0377\0377\0377\0377\0377\0377\0377\0377\0377\0377\0377\0377\0377";
        kl="";
    }

    hLock l;
};


class levC
{
public:

    leveldb::DB* db;
    leveldb::Cache* block_cache;
    const leveldb::FilterPolicy* filter_policy;

    levC(){
        db = 0;
        block_cache = 0;
        filter_policy = 0;
    }
};

class BlevC
{
public:

    leveldb::DB* db;
    leveldb::WriteBatch *batch;

    BlevC(){
        db = 0;
        batch = 0;
    }
};

class IlevC
{
public:

    leveldb::DB* db;
    leveldb::Iterator* it;
    const leveldb::Snapshot* ss;

    IlevC(){
        db = 0;
        it = 0;
        ss = 0;
    }
};

class lComparator : public leveldb::Comparator {
 public:
  // Three-way comparison function:
  //   if a < b: negative result
  //   if a > b: positive result
  //   else: zero result
  int Compare(const leveldb::Slice& a, const leveldb::Slice& b) const {
    long al,bl;
    al = *((long*)a.data());
    bl = *((long*)b.data());

    return al - bl;
  }

  // Ignore the following methods for now:
  const char* Name() const { return "longComparator"; }
  void FindShortestSeparator(std::string*, const leveldb::Slice&) const { }
  void FindShortSuccessor(std::string*) const { }
};

extern "C" {
    MY_EXPORT void init(ef_Content &ec);
}

#endif // EF_LEV_H
