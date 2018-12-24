#ifndef EF_HPP
#define EF_HPP


//#define __GXX_EXPERIMENTAL_CXX0X__
//#undef _GLIBCXX_INCLUDE_AS_TR1

#include <string>
#include <iostream>

#include <map>    //подключили библиотеку для работы с map
//#include <unordered_map>    //подключили библиотеку для работы с map
#include <queue>
#include <vector>
#include <list>
#include <algorithm>    // std::sort
#include <cstdarg>
#include <mutex>
//#include <va-ppc.h>


class hVal;
class ef_Content;

typedef  hVal* (*EF_FNC)(ef_Content &);

typedef  void (*DEL_FNC)(void *);
typedef std::map<int, DEL_FNC> HHMapIDF_T;
typedef HHMapIDF_T::iterator HHMapIDF_TI;

typedef std::map<std::string, hVal*> HHMapSV_T;
typedef HHMapSV_T::iterator HHMapSV_TI;

//typedef std::unordered_map<std::string, hVal> HHMapSV_T;
//typedef HHMapSV_T::iterator HHMapSV_TI;

typedef std::deque<hVal*> HHDeqV_T;
typedef HHDeqV_T::iterator HHDeqV_TI;

typedef std::vector<hVal*> HHVecV_T;
typedef HHVecV_T::iterator HHVecV_TI;

typedef std::vector<int> HHVecI_T;
typedef HHVecI_T::iterator HHVecI_TI;

typedef std::deque<HHDeqV_T*> EF_STACK_T;
typedef EF_STACK_T::iterator EF_STACK_TI;

//typedef std::vector<HHDeqV_TI> EF_STACKV_T;
typedef std::deque<HHDeqV_TI> EF_STACKV_T;
//typedef std::list<HHDeqV_TI> EF_STACKV_T;

typedef EF_STACKV_T::iterator EF_STACKV_TI;

class hLock
{
public:
    long mc;
    std::mutex mtx;

    hLock();//{mc=0;}
    void rl();
    void wl();
    void ru();
    void wu();
};

class hVal
{
public:

    enum {_TRUE=0,_FALSE,NIL, RETURN,BREAK,REPEATE,
          CDOUBLE,CSTRING,CMAP,CDEQ,
          DOUBLE,STRING,MAP,DEQ,FNC,FNCC,BUF,
          LOCK,DB,SQL,UDP,RE
         };

/*
    enum {NIL, RETURN,BREAK,REPEATE,
          CDOUBLE,CSTRING,CMAP,CDEQ,
          DOUBLE,STRING,MAP,DEQ,FNC,FNCC,BUF,
          LOCK,DB,SQL,UDP,RE
         };
*/
    enum {G,L,GE,LE,NE,E};

    union V{
        std::string *s;
        double d;
        unsigned long long u;
        long long l;
        HHMapSV_T *m;
        HHDeqV_T *q;
//        std::vector<unsigned char> *f;

        void *vl;
    };

    V v;

//    std::mutex m;

    struct VBK {
    unsigned tp: 32;
    int c: 32;
    };
    struct VBK b;

    //------------------

    //- DEQ

    inline hVal*  get(int pos);
    void  set(HHDeqV_T *v);
    inline void  set(int pos, hVal* v);
    inline void  push(hVal* v);
    inline hVal*  pop();
    void  _push(hVal* v);

    //-- MAP

    inline hVal*  get(char *k);
    inline hVal*  get(std::string k);
    inline void  del(std::string k);
    inline void  del(char* k);
    void  set(HHMapSV_T *v);
    inline void  set(std::string k,hVal* v);
    void  setMV(char *k,hVal* v);

    //- some...

    inline void  set(std::string *s);
    inline void  set(char *s);
    inline void  set(double d);

    inline void  inc(){
        if( this->b.tp != hVal::DOUBLE) return;

        ++this->v.d;
    }

    inline void  dec(){
        if( this->b.tp != hVal::DOUBLE) return;

        --this->v.d;
    }

    //----------

    hVal(int typ,void *value,double d);
    void operator delete(void *p);
};

class ef_Fac
{
public:

    hVal *new_hVal(int typ,void *value,double d);
    std::string *new_str(std::string s);
    std::string *new_str(char *s);
    std::string *new_str(char *s,int length);

    HHMapSV_T *new_M();
    HHDeqV_T *new_Q();

    void del(std::string *v);
    void del(HHMapSV_T *v);
    void del(HHDeqV_T *v);
    void del(hVal *v);
};

typedef void (*MyPFNC)();

class ef_Content
{
public:
    HHMapSV_T *g /* глобали */;
    HHMapSV_T *f /* с-функции */;
    HHMapSV_T *c /* дерективы компилятора */;

    EF_STACKV_T *svi; /* стек возвратов итераторы */
    EF_STACK_T *svq;  /* стек возвратов исполняемые очереди */

    HHVecV_T *sd; /* стек данных */
    HHVecI_T *sdv; /* начала RUN_ов*/

    hVal *G; // доступ к Глобали

    hVal *T;
    hVal *F;
    hVal *N;

    hVal *gN();
    hVal *gZS();

    EF_STACK_T *st;

    ef_Fac ef;
    HHMapIDF_T *dm; //del map

    long mc;
//    std::mutex mtx;
    void rl();void ru();void wl();void wu();

    ef_Content();
    ef_Content(HHMapSV_T *g,HHMapSV_T *f,HHMapSV_T *c,EF_STACKV_T *svi,EF_STACK_T *svq,HHVecV_T *sd);

    ~ef_Content();

    void dI(int id,DEL_FNC v);
    void fI(std::string k,hVal*v);
    void sP(hVal*v);
    void sB(hVal*v);
    void sPB();

    void ts(hVal *v,std::string &o);

    void tst();
/*
    MyPFNC t;// = tst;

    struct {
        void (*rl)();void (*ru)();void (*wl)();void (*wu)();

    } m;
*/
};


//--------

void tst_ef();

std::string *ef_err(char *f,char *l,char*er);
hVal* ef_compile(ef_Content &ec,ef_Fac &ef,char *s,char *pref);
void ef_init(ef_Content &ec,ef_Fac &ef,int argc,char *argv[]);
//hVal* ef_run(ef_Content &ec,ef_Fac &ef);
hVal* ef_run(ef_Content &ec,ef_Fac &ef,hVal *v);
//void getTID(char *s);

std::string url_encode(const std::string &value);
std::string urlDecode(std::string &SRC);


#endif // EF_HPP
