#include "ef.hpp"
#include "string.h"
#include <time.h>
#include "uv.h"
#include "unistd.h"
#include <libpq-fe.h>

#include "../rpi_sock.h"


extern "C" {
//#include <libmemcached/memcached.h>
//#include <libmemcached/util.h>
#include "sha1.h"
}

#ifdef _WIN32
#include <direct.h>
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>

#define MSLEEP(x) Sleep(x)

#else

#define MSLEEP(x) usleep(x*1000)

#endif

#define MAX_UDP_B   20000
class UDP_T
{
public:
    int s;
    int sc;
    struct sockaddr_in adr_bc;  /* AF_INET */
    struct sockaddr_in adr_bc_rc;  /* AF_INET */
    char b[MAX_UDP_B+1];
};

class SqlAdd
{
public:
    PGresult   *res;
//    char*   sql;
    hVal *sql;
    int flg_exec;
//    int con;
    hVal *con;
    int pos;
//    char stnm[100];
    std::string stnm;

    SqlAdd(){
        res = NULL;
        sql = NULL;
        flg_exec = 0;
        con = NULL;
        pos = 0;
//        stnm[0] = 0;
    }
    ~SqlAdd(){
        if( res) PQclear(res);
        if( sql) delete sql;
        if( con) delete con;
    }
};
#define QBOOLOID 16
#define QINT8OID 20
#define QINT2OID 21
#define QINT4OID 23
#define QNUMERICOID 1700
#define QFLOAT4OID 700
#define QFLOAT8OID 701
#define QABSTIMEOID 702
#define QRELTIMEOID 703
#define QDATEOID 1082
#define QTIMEOID 1083
#define QTIMETZOID 1266
#define QTIMESTAMPOID 1114
#define QTIMESTAMPTZOID 1184
#define QOIDOID 2278
#define QBYTEAOID 17
#define QREGPROCOID 24
#define QXIDOID 28
#define QCIDOID 29


#define PREF_MAIN "MAIN"

extern int argc_;
extern char **argv_;
extern uv_thread_t tprc;
extern int quit_flg;
char * get_file_text_utf8(char * fn,int *fsz);

//hVal G(hVal::CMAP,0,0);
hVal *G = NULL;//(hVal::CMAP,0,0);

ef_Fac ef;
ef_Content ec;
HHMapIDF_T del_map;

hVal *vT = new hVal(hVal::TRUE, 0,0);
hVal *vF = new hVal(hVal::FALSE, 0,0);
hVal *vN = new hVal(hVal::NIL, 0,0);

hVal *vR = new hVal(hVal::RETURN, 0,0);
hVal *vB = new hVal(hVal::BREAK, 0,0);
hVal *vRP = new hVal(hVal::REPEATE, 0,0);
hVal *vZS = new hVal(hVal::CSTRING, new std::string(""),0);
hVal *vNL = new hVal(hVal::CSTRING, new std::string("\n"),0);

//hVal *vD1 = new hVal(hVal::CDOUBLE, 0,1);


char *tps[] ={
    "true","false","null", "R","B","RP","CDOUBLE","CSTRING", "CMAP","CDEQ",
    "DOUBLE","STRING","MAP","DEQ","FNC","FNCC","BUF",
    "LOCK","DB","SQL","UDP"
};
#define TPS_LENGTH sizeof(tps)/sizeof(char*)

void tdel_f(void *v)
{
    fprintf(stderr,"DEL_FNC tdel_f !!!\n"); fflush(stderr);
}

void tttst()
{
    std::cerr << "<--TTTTT ES TTTTT +++\n"; std::cerr.flush();
}


std::string tp_f(int i)
{
    std::string o;

    if( i >= TPS_LENGTH) {
        o = "UT_";
        o += std::to_string((long long )i);
        return o;
    }

    o = tps[i];
    return o;
}


//--------------------------

#define BUF_INT_SIZE 50000
static char	b[BUF_INT_SIZE] = "";
static char	b_in[BUF_INT_SIZE] = "";
static int bin_flg = 0;

void getsTrun(void* p)
{
    while( gets(b) && !quit_flg) {
        while( bin_flg) {
            if( quit_flg) return;
            MSLEEP(20);
        }

        strcpy(b_in,b);
        bin_flg = 1;

//        fprintf(stderr,"=====================%s\n",b);fflush(stderr);
    }
}

int c_scselect( int sock)
{

    fd_set				scArr;
    timeval				tmW;

    tmW.tv_sec = 0;
    tmW.tv_usec = 10;

#ifdef _WIN32
    scArr.fd_count = 1;
    scArr.fd_array[0] = sock;
    int r = select(0,&scArr,0,0,&tmW);

#else
    FD_ZERO(&scArr);
    FD_SET(sock,&scArr);
    int r = select(sock+1,&scArr,0,0,&tmW);

#endif
    return r;
}


static time_t tm = 0;
static time_t tm_clone = 0;
static char par[2000]= "";

void pcloning()
{
    char *a[200];
    int ac = argc_;
    memset(a,0,sizeof(a));
    int f_ret = 0;

    int i = 0;
    for(i=0; i < argc_; ++i) {
//                    fprintf(stderr,"%s\n",argv_[i]);fflush(stderr);
        if( !strcmp(argv_[i],"-CLONE")){
            f_ret = 1;
        }
        a[i] = argv_[i];
    }

    if( !f_ret) {
        a[i] = "-CLONE";
        ac++;
    }

//    int i = 0;
//    for(i=0; i < argc_; ++i) {
////                    fprintf(stderr,"%s\n",argv_[i]);fflush(stderr);
//        if( !strcmp(argv_[i],"-CLONE")){
//            f_ret = 1;
//        }
//        a[i] = argv_[i];
//    }

//    if( !f_ret) {
//        a[i] = "-CLONE";
//        ac++;
//    }

    __pid_t npid = fork();

    if( !npid) {
//                    fprintf(stderr,"\nIn new execv PROCESS! %s %s\n",prs,par);fflush(stderr);
/*
        if( args.Length() > 0 ){
            ::close(0);
            ::close(1);
            ::close(2);
        }

        if( args.Length() > 1 ) {
            String::Utf8Value d(args[1]);
            chdir(*d);
        }
*/
        setsid();
        execvp(a[0],a);
//        setsid();
    }
/*
    if( args.Length() > 0 ) {
        for(int i=0; a[i] && i < 200; ++i){
            free(a[i]);
        }
    }
*/
}

void ktThread(void *arg) {
    while( !quit_flg) {
        time_t tml = tm;
        time_t tc = time(0);

        if( tm_clone && tm_clone < tc){
            pcloning();

            tm_clone = 0;
        }

        if( tml == 0) {
            MSLEEP(150);
            continue;
        }
        if( tml < tc){
            if( par[0] != 0) {
//                char prs[3000] = "Нечто";
//                fprintf(stderr,"\nexecv PROCESS! %s %s\n",prs,par);fflush(stderr);

                char *a[200];
                int ac = argc_;
                memset(a,0,sizeof(a));
                int f_ret = 0;

                int i = 0;
                for(i=0; i < argc_; ++i) {
//                    fprintf(stderr,"%s\n",argv_[i]);fflush(stderr);
                    if( !strncmp(argv_[i],"-R",2)){
                        a[i] = par;
                        f_ret = 1;
                        continue;
                    }
                    a[i] = argv_[i];
                }

                if( !f_ret) {
                    a[i] = par;
                    ac++;
                }
/*
                for(int j=0; j < ac; ++j) {
                    fprintf(stderr,"%s\n",a[j]);fflush(stderr);
                }
*/

                if( !fork()) {
//                    fprintf(stderr,"\nIn new execv PROCESS! %s %s\n",prs,par);fflush(stderr);
                    execvp(a[0],a);
//                    fprintf(stderr,"\nIn new execv PROCESS! %s %s\n",prs,par);fflush(stderr);
                    return;
                }


            }

            fprintf(stderr,"\nKILLED BY TIMEOUT! %d %d \n",tml , tc);fflush(stderr);
            _exit(-1);
            return;
        }
        MSLEEP(150);
    }
}


//- DEQ

inline hVal*  hVal::get(int pos){
//    if( this->b.tp != hVal::DEQ && this->b.tp != hVal::CDEQ) return NULL;

    register HHDeqV_T *q = this->v.q;
    if( pos >= q->size()) return vN;

    return q->at(pos);
}

void  hVal::set(HHDeqV_T *v){
//    if( this->b.tp != hVal::DEQ && this->b.tp != hVal::CDEQ && this->b.tp != hVal::FNC) return;
    register HHDeqV_T *o = this->v.q;
    if( !o) {
        this->v.q = v;
        return;
    }

    HHDeqV_TI it = o->begin();
    while( it != o->end()){
        delete *it;

        ++it;
    }
    delete o;
    this->v.q = v;
}

inline void  hVal::set(int pos, hVal* v){
    if( this->b.tp != hVal::DEQ && this->b.tp != hVal::CDEQ) return;

    register HHDeqV_T *q = this->v.q;
    if( pos >= q->size()) return;

    hVal* vv = q->at(pos);
    delete vv;

    ++v->b.c;
    q->at(pos) = v;
}

inline void  hVal::push(hVal* v){
//    if( this->b.tp != hVal::DEQ && this->b.tp != hVal::CDEQ) return;

//    ++v->b.c;
    this->v.q->push_back(v);
}

inline hVal*  hVal::pop(){
//    if( this->b.tp != hVal::DEQ && this->b.tp != hVal::CDEQ) return NULL;

    register HHDeqV_T *q = this->v.q;
    if( q->size() == 0) return vN;

    hVal* v = q->back();
    q->pop_back();

//    if( v->b.c > 0) --v->b.c;
//    if( v->b.c < 0) v->b.c = 0;

    return v;
}

//-- MAP

inline hVal*  hVal::get(char *k){
//    if( this->b.tp != hVal::MAP && this->b.tp != hVal::CMAP) return NULL;

    register HHMapSV_T *o = this->v.m;
    HHMapSV_TI it = o->find(k);

    if( it != o->end()) {
        return it->second;
    }

    return vN;
}

inline hVal*  hVal::get(std::string k){
//    if( this->b.tp != hVal::MAP && this->b.tp != hVal::CMAP) return NULL;

    register HHMapSV_T *o = this->v.m;
    HHMapSV_TI it = o->find(k);

    if( it != o->end()) {
        return it->second;
    }

    return vN;
}

inline void  hVal::del(std::string k){
//    if( this->b.tp != hVal::MAP && this->b.tp != hVal::CMAP) return;

    register HHMapSV_T *o = this->v.m;
    HHMapSV_TI it = o->find(k);

    if( it != o->end()) {
        delete it->second;
        o->erase(it);
    }
}

inline void  hVal::del(char* k){
//    if( this->b.tp != hVal::MAP && this->b.tp != hVal::CMAP) return;

    register HHMapSV_T *o = this->v.m;
    HHMapSV_TI it = o->find(k);

    if( it != o->end()) {
        delete it->second;
        o->erase(it);
    }
}

void  hVal::set(HHMapSV_T *v){
//    if( this->b.tp != hVal::MAP && this->b.tp != hVal::CMAP) return;
    register HHMapSV_T *o = this->v.m;
    if( !o) {
        this->v.m = v;
        return;
    }

    register HHMapSV_TI it = o->begin();
    while( it != o->end()){
        hVal *z = it->second;
        delete z;

        ++it;
    }
    delete o;

    this->v.m = v;
}

inline void  hVal::set(std::string k,hVal* v){
//    if( this->b.tp != hVal::MAP && this->b.tp != hVal::CMAP) return;

    std::pair<HHMapSV_TI,bool> ret = this->v.m->insert ( std::pair<std::string, hVal*>(k,v) );
    ++v->b.c;
    if( !ret.second) {
        HHMapSV_TI it = this->v.m->find(k);

//            if( it != this->v.m->end()) {
            delete it->second;
            it->second = v;
//            }
    }
}

//- some...

inline void  hVal::set(std::string *s){
//    if( this->b.tp != hVal::STRING) return;

    if( this->v.s) delete this->v.s;
    this->v.s = s;
}

inline void  hVal::set(char *s){
//    if( this->b.tp != hVal::STRING) return;

    if( this->v.s) delete this->v.s;
    this->v.s = new std::string(s);
}

inline void  hVal::set(double d){
//    if( this->b.tp != hVal::DOUBLE) return;

    this->v.d = d;
}

//-- const-dest

hVal::hVal(int typ,void *value,double d){
    b.c = 0;
    b.tp = typ;
    v.vl = NULL;

    if(typ == hVal::DOUBLE || typ == hVal::CDOUBLE) {
        v.d = d;
    }else if( typ > hVal::NIL){
        if( value){
            v.vl = value;
        }else{
            switch(typ){
            case hVal::MAP:
            case hVal::CMAP:
                v.m = new HHMapSV_T();
                break;

            case hVal::FNC:
            case hVal::DEQ:
            case hVal::CDEQ:
                v.q = new HHDeqV_T();

                break;

            case hVal::LOCK:
                v.vl = new hLock();
                break;
            }
        }
    }
}

std::mutex ml;

void hVal::operator delete(void *p){
    if( !p) return;

    hVal *v = (hVal *)p;
//    printf("delete %lf %ld c: %d ntp: %d tp: %s\n",v->v.d,v->v.vl,v->b.c,v->b.tp,tp_f(v->b.tp).c_str());fflush(stdout);
//    printf("!delete %lf %ld %d %d %s\n",v->v.d,v->v.vl,v->b.c,v->b.tp,tps[v->b.tp]);fflush(stdout);
    if( v->b.tp < hVal::DOUBLE) return;
//    return;

    if( v->b.c <= 0 ) {
        if(v->b.c < 0){
            printf("<0 !!!REAL_delete %lf %ld %d %d %s\n",v->v.d,v->v.vl,v->b.c,v->b.tp,tps[v->b.tp]);fflush(stdout);
        }
//        printf("REAL_delete %lf %ld %d %d %s\n",v->v.d,v->v.vl,v->b.c,v->b.tp,tps[v->b.tp]);fflush(stdout);

        if( v->v.vl){

            switch(v->b.tp){

            case hVal::CMAP:
            case hVal::MAP:
                {
                register HHMapSV_T *o = v->v.m;

                register HHMapSV_TI it = o->begin();
                while( it != o->end()){
                    hVal *z = it->second;

                    delete z;

                    ++it;
                }
                delete o;
                }

                break;

            case hVal::FNC:
            case hVal::DEQ:
            case hVal::CDEQ:
                {
                register HHDeqV_T *o = v->v.q;

                register HHDeqV_TI it = o->begin();
                while( it != o->end()){
                    delete *it;

                    ++it;
                }
                delete o;
                }
                break;

            case hVal::STRING:
            case hVal::BUF:
//                ml.lock();
                delete v->v.s;
//                ml.unlock();

                break;

            case hVal::LOCK:
                delete (hLock*)v->v.vl;
                break;

            case hVal::DB:
                PQfinish((PGconn*)v->v.vl);
                break;

            case hVal::SQL:
                delete (SqlAdd*)v->v.vl;
                break;
            case hVal::UDP:
            {
                UDP_T * d = (UDP_T *)v->v.vl;
                if( d->s >=0) hh_closesocket(d->s);
                if( d->sc >=0) hh_closesocket(d->sc);
                delete d;
                break;
            }
            default:
            {
                HHMapIDF_TI it = del_map.find(v->b.tp);
                if( it != del_map.end()) {
                    DEL_FNC f = it->second;
                    (*f)(v->v.vl);
                }
            }
                break;
            }

            v->v.vl = NULL;
        }

//        ml.lock();
        ::delete(p);
//        ml.unlock();

//        printf("REAL_delete DONE!\n");fflush(stdout);

    }else{
        ml.lock();
        --v->b.c;
        ml.unlock();
    }
}

//  ef_FAC

hVal *ef_Fac::new_hVal(int typ,void *value,double d){
    return new hVal(typ,value,d);
}

std::string *ef_Fac::new_str(std::string s){
    return new std::string(s);
}

std::string *ef_Fac::new_str(char *s){
    return new std::string(s);
}

std::string *ef_Fac::new_str(char *s,int length){
    return new std::string(s,length);
}

HHMapSV_T *ef_Fac::new_M(){
    return new HHMapSV_T();
}

HHDeqV_T *ef_Fac::new_Q(){
    return new HHDeqV_T();
}

void ef_Fac::del(std::string *v){
    delete v;
}

void ef_Fac::del(HHMapSV_T *v){
    delete v;
}

void ef_Fac::del(HHDeqV_T *v){
    delete v;
}

void ef_Fac::del(hVal *v){
    delete v;
}

//-- ef_Content

//std::mutex mtx_g;
hLock glock;

ef_Content::ef_Content(){
    this->g = new HHMapSV_T();
    this->f = new HHMapSV_T();
    this->c = new HHMapSV_T();
    this->mc = 0;

    this->svi = new EF_STACKV_T();
//    this->svi->reserve(2000);

    this->svq = new EF_STACK_T();

    this->sd = new HHVecV_T();
    this->sd->reserve(2000);

    this->sdv = new HHVecI_T();
    this->sdv->reserve(2000);

    this->G = this->ef.new_hVal(hVal::MAP,this->g,0);
    this->dm = &del_map;

    this->T = vT;
    this->F = vF;
    this->N = vN;

//    this->mtx = mtx_g;

//    ef_init(this,this->ef,argc_,argv_);

//    this->t = tttst;
    this->t = (MyPFNC)&ef_Content::tst;

    //methods... to .so

    this->m.rl = (void(*)())&ef_Content::rl;
    this->m.wl = (void(*)())&ef_Content::wl;
    this->m.ru = (void(*)())&ef_Content::ru;
    this->m.wu = (void(*)())&ef_Content::wu;


}

ef_Content::ef_Content(HHMapSV_T *g,HHMapSV_T *f,HHMapSV_T *c,EF_STACKV_T *svi,EF_STACK_T *svq,HHVecV_T *sd)
{
    this->mc = 0;

    if( !g) this->g = new HHMapSV_T();
    else this->g = g;

    if( !f) this->f = new HHMapSV_T();
    else this->f = f;

    if( !c) this->c = new HHMapSV_T();
    else this->c = c;

    //--

    if( !svi) this->svi = new EF_STACKV_T();
    else this->svi = svi;
    if( !svq) this->svq = new EF_STACK_T();
    else this->svq = svq;


    if( !sd) this->sd = new HHVecV_T();
    else this->sd = sd;

    this->sdv = new HHVecI_T();
    this->sdv->reserve(2000);

    this->G = this->ef.new_hVal(hVal::MAP,this->g,0);
    this->dm = &del_map;

//    this->mtx = mtx_g;

//    ef_init(this,this->ef,argc_,argv_);

//    this->t = tttst;
    this->t = (MyPFNC)&ef_Content::tst;
}

ef_Content::~ef_Content(){
//    printf("destruct ef_Content\n");

    if( g){
        HHMapSV_TI i= this->g->begin();
        while( i != g->end()){
            hVal *z = i->second;
            delete z;
            ++i;
        }
        delete g;
    }

    if( f){
        HHMapSV_TI i= this->f->begin();
        while( i != f->end()){
            hVal *z = i->second;
            delete z;
            ++i;
        }
        delete f;
    }

    if( c){
        HHMapSV_TI i= this->c->begin();
        while( i != c->end()){
            hVal *z = i->second;
            delete z;
            ++i;
        }
        delete c;
    }

    //--

    if( svi){
/*
        EF_STACKV_TI i= this->sv->begin();
        while( i != sv->end()){
            delete *i;
            ++i;
        }
*/
        delete svi;
    }
    if( svq){
/*
        EF_STACKV_TI i= this->sv->begin();
        while( i != sv->end()){
            delete *i;
            ++i;
        }
*/
        delete svq;
    }

    if( sd){
        HHVecV_TI i= this->sd->begin();
        while( i != sd->end()){
            delete *i;
            ++i;
        }
        delete sd;
    }

    this->G->v.vl = NULL;
    delete this->G;
}

hLock::hLock(){
    mc=0;
}
void hLock::rl(){
    while(1) {
        mtx.lock();
        if( mc >= 0 ){
            ++mc;
            mtx.unlock();
            break;//return 0;
        }

        mtx.unlock();
        MSLEEP(0);
    }
}
void hLock::wl(){
    while(1) {
        mtx.lock();
        if( mc == 0 ){
            mc = -1;
            mtx.unlock();
            break;//return 0;
        }

        mtx.unlock();
        MSLEEP(0);
    }
}
void hLock::ru(){
    mtx.lock();
    if( mc > 0 ){
        --mc;
    }
    mtx.unlock();
}
void hLock::wu(){
    mtx.lock();
    if( mc < 0 ){
        mc = 0;
    }
    mtx.unlock();
}

//locks ec
void ef_Content::rl(){
    glock.rl();
/*
    while(1) {
        mtx_g.lock();
        if( mc >= 0 ){
            ++mc;
            mtx_g.unlock();
            return ;
        }

        mtx_g.unlock();
        MSLEEP(0);
    }
*/
}
void ef_Content::ru(){
    glock.ru();
/*
    mtx_g.lock();
    if( mc > 0 ){
        --mc;
    }
    mtx_g.unlock();
*/
}
void ef_Content::wl(){
    glock.wl();
/*
    while(1) {
        mtx_g.lock();
        if( mc == 0 ){
            mc = -1;
            mtx_g.unlock();
            return;
        }

        mtx_g.unlock();
        MSLEEP(0);
    }
*/
}
void ef_Content::wu(){
    glock.wu();
/*
    mtx_g.lock();
    if( mc < 0 ){
        mc = 0;
    }
    mtx_g.unlock();
*/
}

void ef_Content::tst()
{
    std::cerr << "<--TTTTT ES!!!!!!!!!!!!!! --------- METHOD TTTTT +++\n"; std::cerr.flush();
}


//---------------------


//---------------------- test >>

void tst_ef(){
    {

    std::deque<int*> mydeque (10);   // 10 zero-initialized elements
    std::deque<int*>::size_type sz = mydeque.size();
    int m[100] = {2,4,5,61,2,35,55,66,77,0,1,2,3,4,5,6,7,8,9,11,21,33,455,77};

    del_map.insert( std::pair<int, DEL_FNC>(100, tdel_f));

    // assign some values:
    for (unsigned i=0; i<sz; i++) mydeque[i]=&m[i];

    // reverse order of elements using operator[]:
    for (unsigned i=0; i<sz/2; i++)
    {
      int temp;
      temp = *mydeque[sz-1-i];
      *mydeque[sz-1-i]=*mydeque[i];
      *mydeque[i]=temp;
    }

    // print content:
    std::cout << "mydeque contains:";
    for (unsigned i=0; i<sz; i++)
      std::cout << ' ' << *mydeque[i];
    std::cout << '\n';
    }
    double d=0;

    ++d;
    ++d;
    --d;
    --d;
    --d;
    --d;

    //------------

    hVal *fv = new hVal(hVal::DOUBLE, NULL,765323242.4342);
    printf("kjhk %f ddd %f\n",fv->v.d,d);
    printf("EEEE %f ddd %f\n",fv->v.d,d);

    printf("sizeof(hVal::V) %d\n",sizeof(hVal::V));
    printf("sizeof(hVal::VBK) %d\n",sizeof(hVal::VBK));
    printf("sizeof(hVal) %d\n",sizeof(hVal));


    fv->b.c = 2;

    delete fv;
    delete fv;
    delete fv;

    //----------
    hVal *r = NULL;

    hVal *s = new hVal(hVal::STRING, new std::string("Я строка Совсем строка"),0);
    hVal *s1 = new hVal(hVal::STRING, new std::string("Я Другая Совсем строка"),0);
    hVal *s2 = new hVal(hVal::STRING, new std::string("Я 3-Я строка"),0);
    hVal *s99 = new hVal(hVal::STRING, new std::string("Я 99 строка"),0);

    hVal *m = new hVal(hVal::MAP,0,0);
    m->set("Ключ",s);
    m->del("Ключ");

    m->set("Ключ1",s1);
    hVal *mr = m->get("Ключ");

    printf("STRING %s\n",s->v.s->c_str());

//    if( mr) std::cout << *mr->v.s << '\n';
//    else std::cout <<  "НЕТУТИ КЛЮЧА\n";

    //--

    hVal *q = new hVal(hVal::DEQ,0,0);

    q->push(s2);
    q->push(s);
    q->push(s1);

    q->push(s1);
    q->push(s1);
    q->push(s2);
    q->push(s1);
    q->push(s1);
    q->push(s1);
    q->push(s1);

    q->set(1,s99);


    std::cout << *q->get(0)->v.s << " <--q\n";
    std::cout << *q->get(1)->v.s << " <--q\n";

    std::cout << *q->pop()->v.s << " <--q pop\n";
    std::cout << *q->pop()->v.s << " <--q pop\n";
/*
    r = q->pop();
    if( r) std::cout << *r->v.s << " <--q pop r\n";

    r = q->pop();
    if( r) std::cout << *r->v.s << " <--q pop r\n";
    else std::cout <<  "НЕТУТИ Больше в Списке\n";

    r = q->pop();
    if( r) std::cout << *r->v.s << " <--q pop r\n";
    else std::cout <<  "НЕТУТИ Больше в Списке\n";

    r = q->pop();
    if( r) std::cout << *r->v.s << ' ' << q->v.q->size() << " <--q pop r\n";
    else std::cout <<  "НЕТУТИ Больше в Списке\n";
*/
/*
    r = q->get(1);
    if( r) std::cout << *r->v.s <<  ' ' << q->v.q->size() << " <--q pop get\n";
    else std::cout <<  "НЕТУТИ ВООБШЕ в Списке\n";
*/
    {
    ef_Fac ef;
    std::string *s = ef.new_str("123456789012345678",9);
    std::cout << *s <<  " <--ef.new_str \n";

    EF_STACK_T st;

    st.push_back( ef.new_Q());

    std::cout << *s <<  " <--ef.new_str2 \n";
    HHDeqV_T *e = st.back();
    std::cout << *s <<  " <--ef.new_str3 \n";
    printf("st.back() %ld\n",e);

    char *se = "dsfasf\ndgfadftert453\n12345678";
//    std::cerr << *(ef_err(se,se+25,"test error")) << "<--ef_err \n\n\n";

    }
/*
    char *se = "(aa aa bb aa ee)- lF 'СловаСлова' 'n' ?{'n':-,'rr':(bb ls),'default':ee }  aa 2 ?[aa bb (ee ls ('еще'( F ?['оно' (ls)])))]"\
            "T ? ee bb l tt l N l [ \"fgd\" l (aa bb) [ tt 12 'Скоро всем Хорошо' aa ()] ]l 666777  "\
            "l {'12' 'знаем' '44' 5, 'ключь' 'значение','Дек' [1 2 3 4 56 'сто сорок три' [aa bb 'немного сладкого']]}"\
            " l( bb F l ( tt  aa  tt) ) tt 'Это я вам говорю!' l  ls ee lg lF -10 ~(aa ee l ) aa l aa bb - lg [1 2 'три'] l ~(l ) ls "\
            "{'k1':9,'k2':19,'k3':29,'k4':'39'}'k3'~(l)- G l [ 1 2 3 4 5 6] l lT C l 2 $ ls % ls";
*/
//    char *se = " 1 2 3 4 F ?[(aa) (5 \\ ls 6 ls \\ v0 v4 v4 v4 ls)] ls f100 ls % ls $2 ls [22,44] k $1 v ls 99 34 ls . ls";
//    char *se = " 1 2 3 4 F ?[(aa) ( 33 ls .v2 ls)] [1 2 3 4 5 67 ] 'Я плакатьь' (aa bb) {'55':'ghfgh', 'rr' 'rtwrw'}L ['kghgk',2] 'нет плохому' .K 666 .V ls";
/*
    char *se = " 1 2 3 4 T ?[(aa) ( 33 ls .v2 ls)] [1 2 3 4 5 67 ] '--??:%s fghf %d %g hhh'P 0x40 "\
            "++ ++ ++ -- 16 +'вот цифра: %.0f 'P'это строка'+', другая'+' и еще одна'+'начало: %3s'P {}['ключь' 143]+['rr' 'ответ']+ \n"\
            "5 ~(v2 Q v0'kЛЮЧЧЧ:%.0f'P + v0 + + l)  99 12 - % 'rr'* 44 10 //  % % ~(l)ls";

    char *se = "[76 2 8 1 N B F  T 99 23 11 'gdg' '123' [31,25,7,99]] SR '' "\
            "TP ls {'1' 'tyrt', '6':453,'9':999,'ttt':'Последний','то':'ghfhd 353'} 'то'+ ls'это'+ 'то'+'то'+'то'+ -"\
            " $2 ?{'DEQ':aa,'STRING':bb} $ [] == 40 !  F T ! | ls";
*/
    char *se = "1 2 < 'оно' 'оно'  {'1'100} {'3'5} = ls"
            ;
    {

    ef_init(ec,ef,argc_,argv_);
    hVal * r = ef_compile(ec,ef,se,0);
    if( r->b.tp == hVal::STRING){
        std::cerr << *r->v.s << " <--ef_err!!!!!!------!!!!!!!! \n";
    }

//    std::cerr << "ef_err BBB !!!!!!!!!!!!!! \n";
    HHMapSV_TI it = ec.g->begin();
    while( it != ec.g->end()){
        std::string k = it->first;
        hVal *z = it->second;

        std::cerr << k << " <--g.k! \n";

        ++it;
    }

    it = ec.f->begin();
    while( it != ec.f->end()){
        std::string k = it->first;
        hVal *z = it->second;

        std::cerr << k << " <--f.k! \n";

        ++it;
    }

    it = ec.g->find(PREF_MAIN);
    if( it != ec.g->end()) {
        hVal *v = it->second;

        HHDeqV_T* q = v->v.q;//ec.svq->back();
        HHDeqV_TI qi = q->begin();//ec.svq->back();
        for(int i=0; qi != q->end(); ++qi,++i){
            hVal *k = *qi;
            char *tp = tps[k->b.tp];
            fprintf(stderr,"%d %s \n",i,tp); fflush(stderr);

        }



        std::cerr << "\n" << se << "\n\n";
        ef_run(ec,ef,v);

//        std::cerr << tp_f(TPS_LENGTH-1) << "\n";

    }


    }



}

//--------------------------------------------------------------------------//

std::string *ef_err(char *f,char *l,char*er){
    char *p = f;
    char *lnl = f;
    long long rows = 0,cols = 0;

    for(; p <= l; ++p){
        if( *p == '\n'){
            ++rows;
            cols = 0;
            lnl = p+1;
            continue;
        }
        ++cols;
    }

    std::string s;
    std::string ss(lnl,l-lnl);

    s = std::to_string(rows+1) + " " + std::to_string(cols+1) + " Compile ERROR: " + std::string(er) + "! -> " + ss +'\n';
//    s =  std::string(er) + " -> " + ss;
    return ef.new_str(s);
}


//hVal *vC = new hVal(hVal::MAP, 0,0);
//hVal *vCC = new hVal(hVal::MAP, 0,0);

//#define MAX_PREF 100
//#define MAX_ATOM MAX_PREF*2

hVal* ef_compile(ef_Content &ec,ef_Fac &ef,char *s,char *pref)
{
//    register int i = 0;
    register char c,cc;
    register char *p = s;
    register char *pp;

    int gl_add = true;
    if( pref == (char *)-1 ) {
        gl_add = false;
        pref = PREF_MAIN;
    }

    if( !pref) pref = PREF_MAIN;
/*
    char abuf[MAX_ATOM + 4] = "";
    char atom[MAX_PREF + 2] = "";
    char pr[MAX_PREF+2] = "";
    if( pref) strncpy(pr,pref,MAX_PREF);
*/
    std::string pr(pref); // !.main

    HHDeqV_T *f = ef.new_Q();
    EF_STACK_T st;
    st.push_back(f);
//    HHDeqV_T *cf = f;

    std::string *err = NULL;

    while( 1 ){
        c = *p;
        if( c == 0) break;

        if( c == '#' && ( *(p+1) != '#') ) { //комент до конца строки
            pp = strchr(p+1,'\n');
            if( !pp) break;
            p = pp+1;
            continue;
        }else if( c == '#' && ( *(p+1) == '#')){ // ##  комент ##
            pp = strstr(p+2,"##");
            if( !pp) break;
            p = pp+2;
            continue;

        }else if( c == '"' || c == '\''){ // строка
            ++p;
            pp = p;
            while( pp = strchr(pp,c)){
                if( !pp) break;
                if( *(pp-1) == '\\') {
                    ++pp;
                    continue;
                }
                break;
            }
            if( !pp) break;

            st.back()->push_back( ef.new_hVal(hVal::STRING,ef.new_str(p,pp-p),0));

            //--
            p = pp+1;
            continue;

        }else if( c == '[' || c == '{' || c == '('){ // открыть-> массив мап функция
            HHDeqV_T *nl = ef.new_Q();
            st.push_back(nl);

        }else if( c == ']'){ // массив
            if( st.size() < 2) {
                err = ef_err(s,p,"braces mismatch");
                break;
            }

            HHDeqV_T *insf = st.back();
            st.pop_back();

            st.back()->push_back( ef.new_hVal(hVal::DEQ,insf,0));

        }else if( c == ')'){ // функция
            if( st.size() < 2) {
                err = ef_err(s,p,"braces mismatch");
                break;
            }

            HHDeqV_T *insf = st.back();
            st.pop_back();

            st.back()->push_back( ef.new_hVal(hVal::FNC,insf,0));

        }else if( c == '}'){ // мап
            if( st.size() < 2) {
                err = ef_err(s,p,"braces mismatch");
                break;
            }

            HHDeqV_T *q = st.back();
            st.pop_back();
            HHMapSV_T *m = ef.new_M();

            HHDeqV_TI qi = q->begin();

            hVal *k = NULL;
            hVal *v = NULL;

            for(int i=0; qi != q->end(); ++qi,++i){
                if( (i % 2) == 0){
                    k = *qi;
                }else{
                    v = *qi;
                    // add
                    if( k->b.tp != hVal::STRING){
                        err = ef_err(s,p,"Key is not a string");

                        HHDeqV_TI qi2 = q->begin();
                        while( qi2 != q->end()){
                            delete *qi2;
                            ++qi2;
                        }

                        HHMapSV_TI it = m->begin();
                        while( it != m->end()){
                            hVal *z = it->second;
                            delete z;
                            ++it;
                        }

                        ef.del(q);
                        ef.del(m);

                        break;
                    }else{ //добавляем
                        ++v->b.c;
                        m->insert( std::pair<std::string, hVal*>(*k->v.s,v) );
                    }
                }

            }

            if( err) break;

            st.back()->push_back( ef.new_hVal(hVal::MAP,m,0));

            HHDeqV_TI qi2 = q->begin();
            while( qi2 != q->end()){
                delete *qi2;
                ++qi2;
            }
            ef.del(q);

        }else if( !isspace(c) && c != ',' && c != ':' && c != '[' && c != ']' && c != '{' && c != '}' && c != '(' && c != ')'
                  && c != '\''&& c != '"'&& c != '#'
                  ){
            pp = p;
            while( 1){
                cc = *pp;
/*
                if( cc == 0 || isspace(cc) || cc == ',' || cc == ':'
                    ||  cc == '[' || cc == ']' || cc == '{' || cc == '}' || cc == '(' || cc == ')'
                    || cc == '\'' || cc == '"' || cc == '#'
                    ){
                    break;
                }
*/
                if( isspace(cc)) break;

                switch(cc){
                case 0:
                case ',':
                case ':':
                case '[':
                case ']':
                case '{':
                case '}':
                case '(':
                case ')':
                case '\'':
                case '"':
                case '#':

                    goto out;
                    break;

                }

                ++pp;
            }
out:
            //-- atom

            std::string a(p,pp-p);

//            std::cerr << a << " <--compil a! \n";

            char *p1 = (char *)a.c_str();
            cc = *p1;

            p = pp;

//            if( cc == '-' || cc == '+' || cc == '.' || isdigit(cc)){
            if( isdigit(cc) || ((cc == '-' || cc == '.' ) && (a.size()>1) && isdigit(*(p1+1)))){
                st.back()->push_back( ef.new_hVal(hVal::DOUBLE,0,(double)atof(p1)));
                continue;
            }else{
//                typedef  hVal* (*EF_FNC)(ef_Content , EF_STACK_T );

                {HHMapSV_TI it = ec.c->find(a);
                if( it != ec.c->end()) {
                    hVal *v = it->second;
                    EF_FNC f = (EF_FNC)v->v.vl;
                    ec.st = &st;
                    (*f)(ec);
                    continue;
                }}

                {HHMapSV_TI it = ec.f->find(a);
                if( it != ec.f->end()) {
                    hVal *v = it->second;
                    EF_FNC f = (EF_FNC)v->v.vl;

                    st.back()->push_back( ef.new_hVal(hVal::FNCC,(void*)f,0));
                    continue;
                }}

                ec.rl();
                {HHMapSV_TI it = ec.g->find(a);
                if( it != ec.g->end()) {
                    hVal *v = it->second;
                    void *f = v->v.vl;

                    ++v->b.c;
                    st.back()->push_back( v);

                    ec.ru();
                    continue;
                }}
                ec.ru();

                std::string aa = pr + '.' + a;

                ec.rl();
                {HHMapSV_TI it = ec.g->find(aa);
                if( it != ec.g->end()) {
                    hVal *v = it->second;
                    void *f = v->v.vl;

                    ++v->b.c;
                    st.back()->push_back( v);

                    ec.ru();
                    continue;
                }}
                ec.ru();

                HHDeqV_T *stb = st.back();

                if( stb->size() > 0) {
                    hVal *e = stb->back();

                    if(gl_add){
                        ec.wl();
                        std::pair<HHMapSV_TI,bool> ret = ec.g->insert( std::pair<std::string, hVal*>(aa,e));
                        if( !ret.second) {
                            std::string cs = "Atom Already defined: "+ aa;
                            err = ef_err(s,p,(char*)cs.c_str());

                            ec.wu();
                            break;
                        }else{
                            stb->pop_back();
                        }
                        ec.wu();
                    }

                    continue;
                }

                std::string cs = "Atom Unknown<"+ aa +">";
                err = ef_err(s,p,(char*)cs.c_str());
                break;
            }

//            p = pp;

//        }else if( c == '#'){
        }

        ++p;
    }

    if( err) {
        EF_STACK_TI i = st.begin();
        while( i != st.end()){
            HHDeqV_T* node = *i;
            HHDeqV_TI j = node->begin();
            while( j != node->end()){
                ef.del(*j);

                ++j;
            }
            ++i;
        }

        return ef.new_hVal(hVal::STRING,err,0);
    }

    //-- добавить в глобальный контент? ...

    if( st.size() > 1) {
        EF_STACK_TI i = st.begin();
        ++i;
        while( i != st.end()){
            HHDeqV_T* node = *i;
            HHDeqV_TI j = node->begin();
            while( j != node->end()){
                ef.del(*j);

                ++j;
            }
            ++i;
        }
    }

//    std::cerr << " <--compile END! \n";
    if( gl_add){
        ec.wl();
        ec.g->insert( std::pair<std::string, hVal*>(pr,ef.new_hVal(hVal::FNC,f,0)));
        ec.wu();
    }else{
        if( f->size() > 0){
            hVal *rr = f->back();
            f->pop_back();
            ec.sd->push_back(rr);

            hVal *dv = ef.new_hVal(hVal::FNC,f,0);
            delete dv;

        }else{
            ec.sd->push_back(vN);
        }
    }

//    return ef.new_hVal(hVal::FNC,f,0);
    return vT;
}

//---------------------------------------

int STACK_VIEW  = 5;

static std::string lg_hVal(ef_Content &ec,hVal *v,int lev,std::string &o,int len)
{
    std::string sp;
    for(int i=0; i < lev; ++i) sp += ' ';

//    enum {TRUE=0,FALSE,NIL, RETURN,BREAK,REPEATE,CSTRING,CDOUBLE, STRING,DOUBLE,MAP,DEQ,FNC,FNCC,BUF};

    if( v->b.tp == hVal::CSTRING || v->b.tp == hVal::STRING){
        o += '\'' +*v->v.s + "'";
        return o;
    }
    if( v->b.tp == hVal::CDOUBLE || v->b.tp == hVal::DOUBLE){
        double dd = v->v.d;
        double ddd = dd - (long long)dd;

        if( ddd != 0){
            o += std::to_string((long double)dd);
        }else{
            o += std::to_string((long long)dd);
        }

//        o += std::to_string((long double)v->v.d);// + ' ';


        return o;
    }

    if( v->b.tp <= hVal::NIL){
        o += tp_f(v->b.tp);//+ ' ';
        return o;
    }

    if( v->b.tp == hVal::DEQ || v->b.tp == hVal::CDEQ){
        o += '\n'+sp+"[" + sp;

        HHDeqV_TI it = v->v.q->begin();

        int i = 0;
        while( it != v->v.q->end()){
            if(i > 0) o += ',';
            lg_hVal(ec,*it,lev+1,o,len);
            ++it;
            ++i; if( len > 0 && i > len) {o += "...";break;}
        }

        o += '\n' + sp +"]";
        return o;
    }

    if( v->b.tp == hVal::MAP || v->b.tp == hVal::CMAP){
        o += '\n'+sp+"{" + sp;

        HHMapSV_TI it = v->v.m->begin();

        int i = 0;
        while( it != v->v.m->end()){
            std::string k = it->first;
            hVal *z = it->second;

            if(i > 0) o += ',';
            o += '\'' + k +"':";

            lg_hVal(ec,z,lev+1,o,len);

            ++it;
            ++i; if( len > 0 && i > len) {o += "...";break;}
        }

        o += '\n' + sp +"}";
        return o;
    }

    if( v->b.tp == hVal::FNC){
        o += "()";
        return o;
    }

    o += tp_f(v->b.tp);// +',';// std::cerr.flush();

    return o;
}

static int lg_flg = 1;

std::mutex mtx_lg;

static hVal* lg(ef_Content &ec)
{
//    std::cerr << "<--LOGGG +++\n"; std::cerr.flush();
    if( !lg_flg) return 0;

    if( ec.sd->size()<= 0) return 0;
    std::string o;
    mtx_lg.lock();
    std::cerr << lg_hVal(ec,ec.sd->back(),0,o,STACK_VIEW)<<'\n'; std::cerr.flush();
    mtx_lg.unlock();

    return 0;
}

static hVal* tostring_f(ef_Content &ec)
{
    if( ec.sd->size()<= 0) return 0;
    std::string o;

    hVal *v = ec.sd->back();
    lg_hVal(ec,v,0,o,0);

    delete v;

    ec.sd->back() = ec.ef.new_hVal(hVal::STRING,ec.ef.new_str(o),0);

    return 0;
}

static hVal* fromstring_f(ef_Content &ec)
{
    hVal *r = vN;
    if( ec.sd->size()<= 0) return 0;

    hVal *v = ec.sd->back();
    if( v->b.tp != hVal::STRING && v->b.tp != hVal::CSTRING && v->b.tp != hVal::BUF) return 0;

    ec.sd->pop_back();

    r = ef_compile(ec,ef,(char*)v->v.s->c_str(),(char*)-1);
    if( r->b.tp == hVal::STRING){
        std::cerr << *r->v.s;
        delete r;
        ec.sd->push_back(vN);
    }

    delete v;

    return 0;
}

static hVal* er(ef_Content &ec)
{
    if( ec.sd->size()<= 0) return 0;

    std::string o;

    mtx_lg.lock();
    std::cerr << lg_hVal(ec,ec.sd->back(),0,o,STACK_VIEW)<<'\n'; std::cerr.flush();
    mtx_lg.unlock();

    return 0;
}

//std::mutex mtx_lgs;

static hVal* lgs(ef_Content &ec)
{
    if( !lg_flg) return 0;

    std::string id = "MAIN: ";
    if(tprc != uv_thread_self()){
        id = std::to_string((long long)uv_thread_self()) + ": ";
    }

    mtx_lg.lock();
    std::cerr << "\n#-->LOG_SACK " << id << std::to_string((long long)ec.sd->size()) << " (size)\n"; std::cerr.flush();

    HHVecV_TI it = ec.sd->end();
    int i = ec.sd->size();
    int sni = 0;

    if( ec.sd->size()<= 0) {
        goto end;
    }
    do{
        if(it == ec.sd->begin()) break;
        --it; --i;

        std::string ad;

        for(int ii=0; ii < ec.sdv->size(); ++ii){

//            fprintf(stderr,"ii %d i %d ec.sdv->at(ii) %d ec.sdv->size() %d\n",ii,i,ec.sdv->at(ii),ec.sdv->size()); fflush(stderr);

            if( ec.sdv->at(ii) > i) {
                if( ec.sdv->size()-ii-1 > 0)
                    ad += 'v' + std::to_string((long long)(ec.sdv->size()-ii-1)) + '.' + std::to_string((long long)(ec.sdv->at(ii)-i-1)) + ' ';
                else
                    ad += 'v' +  std::to_string((long long)(ec.sdv->at(ii)-i-1)) + ' ';
            }
        }

        std::string sn = '$' + std::to_string((long long)sni) + ' ';

        std::cerr << "#st--> "<< std::to_string((long long)i) << " : "<< sn << ad <<'\n';

        std::string o;
        std::cerr << lg_hVal(ec,*it,0,o,STACK_VIEW)<<'\n';// std::cerr.flush();

        ++sni;

    }while( it != ec.sd->begin());
end:
    std::cerr << "#--<\n\n";
    std::cerr.flush();

    mtx_lg.unlock();

    return 0;
}

static hVal* lgg(ef_Content &ec)
{
    if( !lg_flg) return 0;

    ec.rl();

    mtx_lg.lock();

    std::cerr << "\n#-->LOG_GLOBAL: "  << std::to_string((long long)ec.g->size()) << " (size)\n";

    HHMapSV_TI it = ec.g->begin();
    while( it != ec.g->end()){
        std::string k = it->first;
        hVal *z = it->second;

        std::string o;
        std::cerr << k << ": " << lg_hVal(ec,z,0,o,0)<<'\n';

        ++it;
    }

    std::cerr << "#--<\n\n";
    std::cerr.flush();

    mtx_lg.unlock();

    ec.ru();

    return 0;
}

static hVal* lg_on(ef_Content &ec)
{
    lg_flg = 1;
    return 0;
}

static hVal* lg_off(ef_Content &ec)
{
    lg_flg = 0;
    return 0;
}

static hVal* tt(ef_Content &ec)
{
    std::cerr << "<--TTT ---\n";
    std::cerr.flush();

    return 0;
}

static hVal* bb(ef_Content &ec)
{
    std::cerr << "<--BBB ---\n";
    std::cerr.flush();

    return 0;
}

static hVal* ee(ef_Content &ec)
{
    std::cerr << "<--EEE ---\n";
    std::cerr.flush();

    return 0;
}
static hVal* aa(ef_Content &ec)
{
    std::cerr << "<--AAA ---\n";
    std::cerr.flush();

    return 0;
}
static hVal* cc(ef_Content &ec)
{
    std::cerr << "<--CCC ---\n";
    std::cerr.flush();

    return 0;
}
static hVal* dd(ef_Content &ec)
{
    std::cerr << "<--DDD ---\n";
    std::cerr.flush();
    char ac[80];
       if (gethostname(ac, sizeof(ac)) != NULL) {
           std::cerr << "Error " <<     " when getting local host name." << std::endl;
           return 0;
       }
    std::cout << "Host name is " << ac << "." << std::endl;

    struct hostent *server = gethostbyname(ac);

    for(int i=0; server->h_addr_list[i] != NULL; ++i)    {
        std::string s = inet_ntoa( *( struct in_addr*)( server->h_addr_list[i]));

        std::cout << "ADR name is " << s << "." << std::endl;
    }

/*
    std::string data = "E:\\video\\100_Greatest_Discoveries\\09_fizika.avi";
    std::path p(data);
    std::cout<<p.leaf()<<endl;// 09_fizika.avi
*/
    return 0;
}

//--
static hVal* tre_f(ef_Content &ec)
{
    register EF_STACKV_T *st = ec.svi;
    HHDeqV_T*svq = ec.svq->back();

    if( (st->back()+1) == svq->end()){
        std::cerr << "ERROR: '^' no right element!\n"; std::cerr.flush();
        return 0;
    }

    register hVal *zz = *(st->back()+1);

    if( zz->b.tp != hVal::FNC){
        std::cerr << "ERROR: '^' right element is not a function!\n"; std::cerr.flush();
        return 0;
    }

    ++zz->b.c;
    ec.sd->push_back(zz);
    ++st->back();

    return 0;
}

static hVal* if_f(ef_Content &ec)
{
//    std::cerr << "<-- IF ---\n";
    if( ec.sd->size() == 0){
        std::cerr << "ERROR: '?' zero size stack!\n"; std::cerr.flush();
        return 0;
    }

    register hVal *z = ec.sd->back();
//    ec.sd->pop_back();

    register EF_STACKV_T *st = ec.svi;
    HHDeqV_T*svq = ec.svq->back();

    if( (st->back()+1) == svq->end()){
        ec.sd->pop_back();
        delete z;
        std::cerr << "ERROR: '?' no right element!\n"; std::cerr.flush();
        return 0;
    }

    register hVal *zz = *(st->back()+1);

    if( z->b.tp != hVal::TRUE && (zz->b.tp != hVal::MAP && zz->b.tp != hVal::DEQ && zz->b.tp != hVal::CMAP && zz->b.tp != hVal::CDEQ)){
        ec.sd->pop_back();
        delete z;
        ++st->back();
        return 0;
    }

    if( z->b.tp == hVal::TRUE && (zz->b.tp != hVal::MAP && zz->b.tp != hVal::DEQ && zz->b.tp != hVal::CMAP && zz->b.tp != hVal::CDEQ)){
        ec.sd->pop_back();
        delete z;
        return 0;
    }

    if( zz->b.tp == hVal::DEQ || zz->b.tp == hVal::CDEQ){
        int ind = 0;

        switch( z->b.tp){
        case hVal::TRUE:
            ind = 0;
            break;

        case hVal::FALSE:
            ind = 1;
            break;

        case hVal::DOUBLE:
        case hVal::CDOUBLE:
            ind = z->v.d;
            break;

        default:
            ind = -1;
            break;
        }

        if( ind < 0){
            ec.sd->pop_back();
            delete z;
            ++st->back();
            return 0;
        }

        HHDeqV_T *q = zz->v.q;
        if( ind >= q->size()){
            ec.sd->pop_back();
            delete z;
            ++st->back();
            return 0;
        }

        hVal *qi = q->at(ind);

        if( qi->b.tp == hVal::FNC ) {
            ef_run(ec,ef,qi);
            ec.sd->pop_back();
            delete z;
        }else{
            ec.sd->pop_back();
            delete z;
            ef_run(ec,ef,qi);
        }

        ++st->back();
        return 0;
    }

    if( zz->b.tp == hVal::MAP || zz->b.tp == hVal::CMAP){
        if( z->b.tp != hVal::CSTRING && z->b.tp != hVal::STRING){
            ec.sd->pop_back();
            delete z;
            ++st->back();
            return 0;
        }

        register HHMapSV_T *o = zz->v.m;
        HHMapSV_TI it = o->find(*z->v.s);

        if( it != o->end()) {
            hVal *qi = it->second;

            if( qi->b.tp == hVal::FNC ) {
                ef_run(ec,ef,qi);
                ec.sd->pop_back();
                delete z;
            }else {
                ec.sd->pop_back();
                delete z;
                ef_run(ec,ef,qi);
            }

            ++st->back();
            return 0;
        }

        it = o->find("default");

        if( it != o->end()) {
            hVal *qi = it->second;

            if( qi->b.tp == hVal::FNC ) {
                ef_run(ec,ef,qi);
                ec.sd->pop_back();
                delete z;
            }else {
                ec.sd->pop_back();
                delete z;
                ef_run(ec,ef,qi);
            }

            ++st->back();
            return 0;
        }

        ec.sd->pop_back();
        delete z;
        ++st->back();
        return 0;
    }


    ec.sd->pop_back();
    delete z;
    ++st->back();
    return 0;
}

static hVal* i_f(ef_Content &ec)
{
    if( ec.sd->size() == 0){
        std::cerr << "ERROR: '~' zero size stack!\n"; std::cerr.flush();
        return 0;
    }

    register hVal *z = ec.sd->back();
//    ec.sd->pop_back();

    register EF_STACKV_T *st = ec.svi;
    HHDeqV_T*svq = ec.svq->back();

    if( (st->back()+1) == svq->end()){
        ec.sd->pop_back();
        delete z;
        std::cerr << "ERROR: '~' no right element!\n"; std::cerr.flush();
        return 0;
    }

    register hVal *zz = *(st->back()+1);

//    if( zz->b.tp != hVal::FNCC && zz->b.tp != hVal::FNC){
    if( zz->b.tp != hVal::FNC){
        ec.sd->pop_back();
        delete z;
        ++st->back(); //?
        std::cerr << "ERROR: '~' right element is not a function!\n"; std::cerr.flush();
        return 0;
    }

    if( z->b.tp != hVal::MAP && z->b.tp != hVal::DEQ
            && z->b.tp != hVal::CMAP && z->b.tp != hVal::CDEQ
            && z->b.tp != hVal::CDOUBLE && z->b.tp != hVal::DOUBLE
            && z->b.tp != hVal::STRING && z->b.tp != hVal::CSTRING
            ){
        ec.sd->pop_back();
        delete z;
        ++st->back();
        std::cerr << "ERROR: '~' stack element is not a MAP or DEQ or DOUBLE or (MAP STRING)!\n"; std::cerr.flush();
        return 0;
    }

    if( z->b.tp == hVal::DOUBLE || z->b.tp == hVal::CDOUBLE){
        long long cnt = z->v.d;
        if( cnt == 0) {
            goto end;
        }

        int i = 0;
        int inc = 1;
        if( cnt < 0 ) {
            inc = -1;
            cnt = - cnt;
            i = cnt - 1;
        }

//        hVal *v = ef.new_hVal(hVal::DOUBLE,0,i);
//        ec.sd->push_back(v);

        for( ; i >= 0 && i < cnt; i += inc){

            hVal *e = ec.ef.new_hVal(hVal::DOUBLE,0,(double)i);
            ec.sd->push_back(e);

//            v->set((double) i);
            hVal * ret = ef_run(ec,ef,zz);
            if( ret && ret->b.tp == hVal::BREAK){
                ec.sd->pop_back();
                delete e;

                break;
            }

            ec.sd->pop_back();
            delete e;
        }

//        ec.sd->pop_back();
//        delete v;

        goto end;
    }

    if( z->b.tp == hVal::DEQ || z->b.tp == hVal::CDEQ){
        HHDeqV_T *q = z->v.q;
        HHDeqV_TI qi = q->begin();

        while( qi != q->end()){
            register hVal * tv = *qi;
            ++tv->b.c;
            ec.sd->push_back(tv);

            hVal * ret = ef_run(ec,ef,zz);
            if( ret && ret->b.tp == hVal::BREAK){
                ec.sd->pop_back();
                delete tv;

                break;
            }

            ec.sd->pop_back();
            delete tv;

            ++qi;
        }

        ++st->back();
        return 0;
    }

    if( z->b.tp == hVal::MAP || z->b.tp == hVal::CMAP){
        HHMapSV_T *q = z->v.m;
        HHMapSV_TI i = q->begin();

        while( i != q->end()){
            std::string k = i->first;
            hVal *z = i->second;

            hVal *e = ec.ef.new_hVal(hVal::DEQ,0,0);

            e->push(ec.ef.new_hVal(hVal::STRING,ec.ef.new_str(k),0));
            ++z->b.c;
            e->push(z);

            ec.sd->push_back(e);

            hVal * ret = ef_run(ec,ef,zz);
            if( ret && ret->b.tp == hVal::BREAK){
                ec.sd->pop_back();
                delete e;

                break;
            }

            ec.sd->pop_back();
            delete e;

            ++i;
        }

        ++st->back();
        return 0;
    }

    if( z->b.tp == hVal::STRING || z->b.tp == hVal::CSTRING){
        if( ec.sd->size() < 2){
            std::cerr << "ERROR: '~' no (MAP STRING) on stack!\n"; std::cerr.flush();
            goto end;
        }

        register hVal *m = ec.sd->at(ec.sd->size()-2);

        if( m->b.tp != hVal::MAP && m->b.tp != hVal::CMAP){
            std::cerr << "ERROR: '~' no MAP (MAP STRING) on stack!\n"; std::cerr.flush();
            goto end;
        }

        HHMapSV_T *q = m->v.m;
        HHMapSV_TI i = q->lower_bound(*z->v.s);

        while( i != q->end()){
            std::string k = i->first;
            hVal *z = i->second;

            hVal *e = ec.ef.new_hVal(hVal::DEQ,0,0);

            e->push(ec.ef.new_hVal(hVal::STRING,ec.ef.new_str(k),0));
            ++z->b.c;
            e->push(z);

            ec.sd->push_back(e);

            hVal * ret = ef_run(ec,ef,zz);
            if( ret && ret->b.tp == hVal::BREAK){
                ec.sd->pop_back();
                delete e;

                break;
            }

            ec.sd->pop_back();
            delete e;

            ++i;
        }

        goto end;
    }


end:
    ec.sd->pop_back();
    delete z;
    ++st->back();
    return 0;
}

//-- ------------------------------------- --

static hVal* gg_f(ef_Content &ec){
    ++ec.G->b.c;
    ec.sd->push_back(ec.G);
    return 0;
}

static hVal* q_f(ef_Content &ec){
    ec.sd->push_back(ec.ef.new_hVal(hVal::DEQ,0,0));
    return 0;
}
static hVal* m_f(ef_Content &ec){
    ec.sd->push_back(ec.ef.new_hVal(hVal::MAP,0,0));
    return 0;
}
static hVal* lock_f(ef_Content &ec){
    ec.sd->push_back(ec.ef.new_hVal(hVal::LOCK,0,0));
    return 0;
}


static hVal* sha1_f(ef_Content &ec){

//    int dp = ec.sd->size() - 2;

    if( ec.sd->size() <= 0) return 0;

    hVal *v = ec.sd->back();

    if( !(v->b.tp == hVal::STRING || v->b.tp == hVal::CSTRING)){ return 0;}

    char    md[50]="NONE";
    SHA1Context sha;
    SHA1Reset(&sha);
    SHA1Input(&sha, (const unsigned char *) v->v.s->c_str(), v->v.s->size());

    if (!SHA1Result(&sha))
    {
        return 0;
    } else {
        sprintf(md,"%08X%08X%08X%08X%08X",
                sha.Message_Digest[0],
                sha.Message_Digest[1],
                sha.Message_Digest[2],
                sha.Message_Digest[3],
                sha.Message_Digest[4]
                );
    }

    delete v;
    ec.sd->back() = ec.ef.new_hVal(hVal::STRING,ec.ef.new_str(md),0);

    return 0;
}

static hVal* sha1d_f(ef_Content &ec){

//    int dp = ec.sd->size() - 2;

    if( ec.sd->size() <= 0) return 0;

    hVal *v = ec.sd->back();

    if( !(v->b.tp == hVal::STRING || v->b.tp == hVal::CSTRING)){ return 0;}

    char    md[50]="NONE";
    SHA1Context sha;
    SHA1Reset(&sha);
    SHA1Input(&sha, (const unsigned char *) v->v.s->c_str(), v->v.s->size());

    if (!SHA1Result(&sha))
    {
        return 0;
    } else {
/*
        sprintf(md,"%08X%08X%08X%08X%08X",
                sha.Message_Digest[0],
                sha.Message_Digest[1],
                sha.Message_Digest[2],
                sha.Message_Digest[3],
                sha.Message_Digest[4]
                );
*/
    }

    delete v;
    ec.sd->back() = ec.ef.new_hVal(hVal::DOUBLE,0,sha.Message_Digest[0]);

    return 0;
}

static hVal* pid_f(ef_Content &ec){

    ec.sd->push_back(ec.ef.new_hVal(hVal::DOUBLE,0,(double)getpid()));

    return 0;
}
#ifdef WINDOWS
    #include <direct.h>
    #define GetCurrentDir _getcwd
#else
    #include <unistd.h>
    #define GetCurrentDir getcwd
#endif

#define FILENAME_MAX 2000

static hVal* cdir_f(ef_Content &ec){

    char cCurrentPath[FILENAME_MAX];

    if( !GetCurrentDir(cCurrentPath, sizeof(cCurrentPath))){
        strcpy(cCurrentPath,"./");
    }

    ec.sd->push_back(ec.ef.new_hVal(hVal::STRING,ec.ef.new_str(cCurrentPath),0));

    return 0;
}

static hVal* kill_f(ef_Content &ec){

    if( ec.sd->size() <= 0) return 0;
    hVal *v = ec.sd->back();
    if( !(v->b.tp == hVal::DOUBLE || v->b.tp == hVal::CDOUBLE)){ return 0;}

    uv_kill((int) v->v.d, SIGINT);

    delete v;
    ec.sd->pop_back();

    return 0;
}

static hVal* rm_f(ef_Content &ec){

    if( ec.sd->size() <= 0) return 0;
    hVal *v = ec.sd->back();
    if( !(v->b.tp == hVal::STRING || v->b.tp == hVal::CSTRING)){ return 0;}

    remove(v->v.s->c_str());

    delete v;
    ec.sd->pop_back();

    return 0;
}

static hVal* read_f(ef_Content &ec){

    if( ec.sd->size() <= 0) return 0;

    hVal *v = ec.sd->back();

    if( v->b.tp == hVal::STRING || v->b.tp == hVal::CSTRING){
        int fsz=0;
        char *fb = get_file_text_utf8((char *)v->v.s->c_str(),&fsz);
        if( fb) {
            std::string r(fb,fsz);

            delete v;
            ec.sd->back() = ec.ef.new_hVal(hVal::STRING,ec.ef.new_str(r),0);

            free(fb);
            return 0;
        }else{
            goto end;
        }


    }

    if( v->b.tp == hVal::NIL){
        if( !bin_flg) {
//            ec.sd->back() = vN;
            return 0;
        }
        ec.sd->back() = ec.ef.new_hVal(hVal::STRING,ec.ef.new_str(b_in),0);
        bin_flg = 0;
        return 0;
    }

    if( v->b.tp == hVal::UDP){
//        ec.sd->back() = ec.ef.new_hVal(hVal::STRING,ec.ef.new_str(b_in),0);
        UDP_T *u = (UDP_T *)v->v.vl;

        socklen_t x;

        int z = recvfrom(u->sc,      /* Socket */
                     u->b,  /* Receiving buffer */
                     MAX_UDP_B,/* Max rcv buf size */
                     0,      /* Flags: no options */
                     (struct sockaddr *)&u->adr_bc_rc, /* Addr */
                     &x);    /* Addr len, in & out */

        if ( z < 0 ){
            ec.sd->push_back(vN);
        }else{
            ec.sd->push_back(ec.ef.new_hVal(hVal::STRING,ec.ef.new_str(u->b,z),0));
        }

        return 0;
    }



end:
    delete v;
    ec.sd->pop_back();

    return 0;
}

static hVal* readB_f(ef_Content &ec){

    if( ec.sd->size() <= 0) return 0;

    hVal *v = ec.sd->back();

    if( v->b.tp == hVal::STRING || v->b.tp == hVal::CSTRING){
        int fsz=0;
        char *fb = get_file_text_utf8((char *)v->v.s->c_str(),&fsz);
        if( fb) {
            std::string r(fb,fsz);

            delete v;
            ec.sd->back() = ec.ef.new_hVal(hVal::BUF,ec.ef.new_str(r),0);

            free(fb);
            return 0;
        }else{
            goto end;
        }


    }

    if( v->b.tp == hVal::NIL){
        if( !bin_flg) {
//            ec.sd->back() = vN;
            return 0;
        }
        ec.sd->back() = ec.ef.new_hVal(hVal::STRING,ec.ef.new_str(b_in),0);
        bin_flg = 0;
        return 0;
    }


end:
    delete v;
    ec.sd->pop_back();

    return 0;
}


static hVal* write_f(ef_Content &ec){

    if( ec.sd->size() < 2) return 0;

    hVal *v = ec.sd->back();
    hVal *vv = ec.sd->at(ec.sd->size()-2);

    std::string *w;
    std::string o;

    if( vv->b.tp == hVal::STRING || vv->b.tp == hVal::CSTRING || vv->b.tp == hVal::BUF){
        w = vv->v.s;
    }else{
        lg_hVal(ec,vv,0,o,0);
        w = &o;
    }

    if( v->b.tp == hVal::STRING || v->b.tp == hVal::CSTRING){

        FILE *f = fopen(v->v.s->c_str(),"wb");
        if( f) {
            fwrite(w->c_str(),1,w->size(),f);
            fclose(f);
        }

    }else if( v->b.tp == hVal::UDP){
        UDP_T *u = (UDP_T *)v->v.vl;

        sendto(u->s,w->c_str(),w->size(),0,(struct sockaddr *)&u->adr_bc,sizeof(u->adr_bc));

    }else if( v->b.tp == hVal::NIL){

        mtx_lg.lock();
        std::cout << *w; std::cout.flush();
        mtx_lg.unlock();

    }else{
        return 0;
    }

end:

    delete v;
    delete vv;
    ec.sd->pop_back();
    ec.sd->pop_back();

    return 0;
}

static hVal* dir_f(ef_Content &ec)
{
    uv_fs_t r;
    uv_dirent_t d;
    int ret;
//    int i = 0;

    hVal *v = NULL;
    char *dp = "./";

    if( ec.sd->size() > 0){
        v = ec.sd->back();
        if( (v->b.tp == hVal::STRING || v->b.tp == hVal::CSTRING)){
            dp = (char*)v->v.s->c_str();
        }
    }

    ret = uv_fs_scandir(uv_default_loop(), &r, dp, 0, 0);

    fprintf(stderr,"dir  ret %d : %s\n",ret,dp);fflush(stderr);
    hVal * rv = ec.ef.new_hVal(hVal::DEQ,0,0);

    if( ret > 0)
    while( uv_fs_scandir_next(&r, &d) != UV_EOF) {
//        fprintf(stderr,"fn:: %s\n",d.name);fflush(stderr);

        hVal * el = ec.ef.new_hVal(hVal::MAP,0,0);

        el->set("nm",ec.ef.new_hVal(hVal::STRING,ec.ef.new_str(d.name),0));
        el->set("id",(d.type == UV_DIRENT_DIR)?vT:vF);

        rv->push(el);
    }

    if( v){
        delete v;
        ec.sd->pop_back();
    }

    ec.sd->push_back(rv);

    return 0;
}

static hVal* mkd_f(ef_Content &ec){

    if( ec.sd->size() <= 0) return 0;
    hVal *v = ec.sd->back();
    if( !(v->b.tp == hVal::STRING || v->b.tp == hVal::CSTRING)){ return 0;}

    int pr = 0755;
    int ret = 0;

#ifdef _WIN32
    ret = _mkdir(v->v.s->c_str());
#else
    ret = mkdir(v->v.s->c_str(),pr);
#endif

    delete v;
    ec.sd->pop_back();

    return 0;
}

static hVal* ren_f(ef_Content &ec){

    if( ec.sd->size() <= 1) return 0;

    hVal *v = ec.sd->back();
    hVal *vv = ec.sd->at(ec.sd->size() - 2);

    if( !(v->b.tp == hVal::STRING || v->b.tp == hVal::CSTRING)){ return 0;}
    if( !(vv->b.tp == hVal::STRING || vv->b.tp == hVal::CSTRING)){ return 0;}

    rename(vv->v.s->c_str(),v->v.s->c_str());

    delete v;
    delete vv;

    ec.sd->pop_back();
    ec.sd->pop_back();

    return 0;
}

static hVal* d_f(ef_Content &ec){
    ec.sd->push_back(ec.ef.new_hVal(hVal::DOUBLE,0,0));
    return 0;
}
static hVal* s_f(ef_Content &ec){
    ec.sd->push_back(ec.ef.new_hVal(hVal::STRING,ec.ef.new_str(""),0));
    return 0;
}
//--
static hVal* cq_f(ef_Content &ec){
    ec.sd->push_back(ec.ef.new_hVal(hVal::CDEQ,0,0));
    return 0;
}
static hVal* cm_f(ef_Content &ec){
    ec.sd->push_back(ec.ef.new_hVal(hVal::CMAP,0,0));
    return 0;
}
static hVal* cd_f(ef_Content &ec){
    ec.sd->push_back(ec.ef.new_hVal(hVal::CDOUBLE,0,0));
    return 0;
}
static hVal* cs_f(ef_Content &ec){
    ec.sd->push_back(ec.ef.new_hVal(hVal::CSTRING,ec.ef.new_str(""),0));
    return 0;
}

//--

static hVal* dup_f(ef_Content &ec){
    if( ec.sd->size() == 0) return 0;
    hVal *v = ec.sd->back();
    ++v->b.c;

    ec.sd->push_back(v);
    return 0;
}
static hVal* dup1_f(ef_Content &ec){
    int cur = ec.sd->size()-2;
    if( cur < 0) return 0;
    hVal *v = ec.sd->at(cur);
    ++v->b.c;

    ec.sd->push_back(v);
    return 0;
}
static hVal* dup2_f(ef_Content &ec){
    int cur = ec.sd->size()-3;
    if( cur < 0) return 0;
    hVal *v = ec.sd->at(cur);
    ++v->b.c;

    ec.sd->push_back(v);
    return 0;
}
static hVal* dup3_f(ef_Content &ec){
    int cur = ec.sd->size()-4;
    if( cur < 0) return 0;
    hVal *v = ec.sd->at(cur);
    ++v->b.c;

    ec.sd->push_back(v);
    return 0;
}
static hVal* dup4_f(ef_Content &ec){
    int cur = ec.sd->size()-5;
    if( cur < 0) return 0;
    hVal *v = ec.sd->at(cur);
    ++v->b.c;

    ec.sd->push_back(v);
    return 0;
}
static hVal* dup5_f(ef_Content &ec){
    int cur = ec.sd->size()-6;
    if( cur < 0) return 0;
    hVal *v = ec.sd->at(cur);
    ++v->b.c;

    ec.sd->push_back(v);
    return 0;
}
static hVal* dup6_f(ef_Content &ec){
    int cur = ec.sd->size()-7;
    if( cur < 0) return 0;
    hVal *v = ec.sd->at(cur);
    ++v->b.c;

    ec.sd->push_back(v);
    return 0;
}
static hVal* dup7_f(ef_Content &ec){
    int cur = ec.sd->size()-8;
    if( cur < 0) return 0;
    hVal *v = ec.sd->at(cur);
    ++v->b.c;

    ec.sd->push_back(v);
    return 0;
}
static hVal* dup8_f(ef_Content &ec){
    int cur = ec.sd->size()-9;
    if( cur < 0) return 0;
    hVal *v = ec.sd->at(cur);
    ++v->b.c;

    ec.sd->push_back(v);
    return 0;
}
static hVal* dup9_f(ef_Content &ec){
    int cur = ec.sd->size()-10;
    if( cur < 0) return 0;
    hVal *v = ec.sd->at(cur);
    ++v->b.c;

    ec.sd->push_back(v);
    return 0;
}
//--
static hVal* v0_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-1;

    if( v_sz < 0) return 0;

    int cur = ec.sdv->at(v_sz)-1;
    if( cur < 0) return 0;

    hVal *v = ec.sd->at(cur);
    ++v->b.c;

    ec.sd->push_back(v);
    return 0;
}
static hVal* v1_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-1;

    if( v_sz < 0) return 0;

    int cur = ec.sdv->at(v_sz)-2;
    if( cur < 0) return 0;

    hVal *v = ec.sd->at(cur);
    ++v->b.c;

    ec.sd->push_back(v);
    return 0;
}
static hVal* v2_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-1;

    if( v_sz < 0) return 0;

    int cur = ec.sdv->at(v_sz)-3;
    if( cur < 0) return 0;

    hVal *v = ec.sd->at(cur);
    ++v->b.c;

    ec.sd->push_back(v);
    return 0;
}
static hVal* v3_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-1;

    if( v_sz < 0) return 0;

    int cur = ec.sdv->at(v_sz)-4;
    if( cur < 0) return 0;

    hVal *v = ec.sd->at(cur);
    ++v->b.c;

    ec.sd->push_back(v);
    return 0;
}
static hVal* v4_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-1;

    if( v_sz < 0) return 0;

    int cur = ec.sdv->at(v_sz)-5;
    if( cur < 0) return 0;

    hVal *v = ec.sd->at(cur);
    ++v->b.c;

    ec.sd->push_back(v);
    return 0;
}
static hVal* v5_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-1;

    if( v_sz < 0) return 0;

    int cur = ec.sdv->at(v_sz)-6;
    if( cur < 0) return 0;

    hVal *v = ec.sd->at(cur);
    ++v->b.c;

    ec.sd->push_back(v);
    return 0;
}
static hVal* v6_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-1;

    if( v_sz < 0) return 0;

    int cur = ec.sdv->at(v_sz)-7;
    if( cur < 0) return 0;

    hVal *v = ec.sd->at(cur);
    ++v->b.c;

    ec.sd->push_back(v);
    return 0;
}
static hVal* v7_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-1;

    if( v_sz < 0) return 0;

    int cur = ec.sdv->at(v_sz)-8;
    if( cur < 0) return 0;

    hVal *v = ec.sd->at(cur);
    ++v->b.c;

    ec.sd->push_back(v);
    return 0;
}
static hVal* v8_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-1;

    if( v_sz < 0) return 0;

    int cur = ec.sdv->at(v_sz)-9;
    if( cur < 0) return 0;

    hVal *v = ec.sd->at(cur);
    ++v->b.c;

    ec.sd->push_back(v);
    return 0;
}
static hVal* v9_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-1;

    if( v_sz < 0) return 0;

    int cur = ec.sdv->at(v_sz)-10;
    if( cur < 0) return 0;

    hVal *v = ec.sd->at(cur);
    ++v->b.c;

    ec.sd->push_back(v);
    return 0;
}
//--
static hVal* v10_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-2;

    if( v_sz < 0) return 0;

    int cur = ec.sdv->at(v_sz)-1;
    if( cur < 0) return 0;

    hVal *v = ec.sd->at(cur);
    ++v->b.c;

    ec.sd->push_back(v);
    return 0;
}
static hVal* v11_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-2;

    if( v_sz < 0) return 0;

    int cur = ec.sdv->at(v_sz)-2;
    if( cur < 0) return 0;

    hVal *v = ec.sd->at(cur);
    ++v->b.c;

    ec.sd->push_back(v);
    return 0;
}
static hVal* v12_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-2;

    if( v_sz < 0) return 0;

    int cur = ec.sdv->at(v_sz)-3;
    if( cur < 0) return 0;

    hVal *v = ec.sd->at(cur);
    ++v->b.c;

    ec.sd->push_back(v);
    return 0;
}
static hVal* v13_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-2;

    if( v_sz < 0) return 0;

    int cur = ec.sdv->at(v_sz)-4;
    if( cur < 0) return 0;

    hVal *v = ec.sd->at(cur);
    ++v->b.c;

    ec.sd->push_back(v);
    return 0;
}
static hVal* v14_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-2;

    if( v_sz < 0) return 0;

    int cur = ec.sdv->at(v_sz)-5;
    if( cur < 0) return 0;

    hVal *v = ec.sd->at(cur);
    ++v->b.c;

    ec.sd->push_back(v);
    return 0;
}
static hVal* v15_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-2;

    if( v_sz < 0) return 0;

    int cur = ec.sdv->at(v_sz)-6;
    if( cur < 0) return 0;

    hVal *v = ec.sd->at(cur);
    ++v->b.c;

    ec.sd->push_back(v);
    return 0;
}
static hVal* v16_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-2;

    if( v_sz < 0) return 0;

    int cur = ec.sdv->at(v_sz)-7;
    if( cur < 0) return 0;

    hVal *v = ec.sd->at(cur);
    ++v->b.c;

    ec.sd->push_back(v);
    return 0;
}
static hVal* v17_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-2;

    if( v_sz < 0) return 0;

    int cur = ec.sdv->at(v_sz)-8;
    if( cur < 0) return 0;

    hVal *v = ec.sd->at(cur);
    ++v->b.c;

    ec.sd->push_back(v);
    return 0;
}
static hVal* v18_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-2;

    if( v_sz < 0) return 0;

    int cur = ec.sdv->at(v_sz)-9;
    if( cur < 0) return 0;

    hVal *v = ec.sd->at(cur);
    ++v->b.c;

    ec.sd->push_back(v);
    return 0;
}
static hVal* v19_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-2;

    if( v_sz < 0) return 0;

    int cur = ec.sdv->at(v_sz)-10;
    if( cur < 0) return 0;

    hVal *v = ec.sd->at(cur);
    ++v->b.c;

    ec.sd->push_back(v);
    return 0;
}
//--
static hVal* v20_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-3;

    if( v_sz < 0) return 0;

    int cur = ec.sdv->at(v_sz)-1;
    if( cur < 0) return 0;

    hVal *v = ec.sd->at(cur);
    ++v->b.c;

    ec.sd->push_back(v);
    return 0;
}
static hVal* v21_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-3;

    if( v_sz < 0) return 0;

    int cur = ec.sdv->at(v_sz)-2;
    if( cur < 0) return 0;

    hVal *v = ec.sd->at(cur);
    ++v->b.c;

    ec.sd->push_back(v);
    return 0;
}
static hVal* v22_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-3;

    if( v_sz < 0) return 0;

    int cur = ec.sdv->at(v_sz)-3;
    if( cur < 0) return 0;

    hVal *v = ec.sd->at(cur);
    ++v->b.c;

    ec.sd->push_back(v);
    return 0;
}
static hVal* v23_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-3;

    if( v_sz < 0) return 0;

    int cur = ec.sdv->at(v_sz)-4;
    if( cur < 0) return 0;

    hVal *v = ec.sd->at(cur);
    ++v->b.c;

    ec.sd->push_back(v);
    return 0;
}
static hVal* v24_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-3;

    if( v_sz < 0) return 0;

    int cur = ec.sdv->at(v_sz)-5;
    if( cur < 0) return 0;

    hVal *v = ec.sd->at(cur);
    ++v->b.c;

    ec.sd->push_back(v);
    return 0;
}
static hVal* v25_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-3;

    if( v_sz < 0) return 0;

    int cur = ec.sdv->at(v_sz)-6;
    if( cur < 0) return 0;

    hVal *v = ec.sd->at(cur);
    ++v->b.c;

    ec.sd->push_back(v);
    return 0;
}
static hVal* v26_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-3;

    if( v_sz < 0) return 0;

    int cur = ec.sdv->at(v_sz)-7;
    if( cur < 0) return 0;

    hVal *v = ec.sd->at(cur);
    ++v->b.c;

    ec.sd->push_back(v);
    return 0;
}
static hVal* v27_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-3;

    if( v_sz < 0) return 0;

    int cur = ec.sdv->at(v_sz)-8;
    if( cur < 0) return 0;

    hVal *v = ec.sd->at(cur);
    ++v->b.c;

    ec.sd->push_back(v);
    return 0;
}
static hVal* v28_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-3;

    if( v_sz < 0) return 0;

    int cur = ec.sdv->at(v_sz)-9;
    if( cur < 0) return 0;

    hVal *v = ec.sd->at(cur);
    ++v->b.c;

    ec.sd->push_back(v);
    return 0;
}
static hVal* v29_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-3;

    if( v_sz < 0) return 0;

    int cur = ec.sdv->at(v_sz)-10;
    if( cur < 0) return 0;

    hVal *v = ec.sd->at(cur);
    ++v->b.c;

    ec.sd->push_back(v);
    return 0;
}
//--
static hVal* q0_f(ef_Content &ec){
    if( ec.sd->size() <= 0) return 0;

    register hVal *v = ec.sd->back();
    if( v->b.tp != hVal::DEQ && v->b.tp != hVal::CDEQ && v->b.tp != hVal::FNC) return 0;
    if( v->v.q->size() < 1) return 0;

    hVal *vv = v->v.q->at(0);
    ++vv->b.c;

    ec.sd->push_back(vv);
    return 0;
}
static hVal* q1_f(ef_Content &ec){
    if( ec.sd->size() <= 0) return 0;

    register hVal *v = ec.sd->back();
    if( v->b.tp != hVal::DEQ && v->b.tp != hVal::CDEQ && v->b.tp != hVal::FNC) return 0;
    if( v->v.q->size() < 2) return 0;

    hVal *vv = v->v.q->at(1);
    ++vv->b.c;

    ec.sd->push_back(vv);
    return 0;
}
static hVal* q2_f(ef_Content &ec){
    if( ec.sd->size() <= 0) return 0;

    register hVal *v = ec.sd->back();
    if( v->b.tp != hVal::DEQ && v->b.tp != hVal::CDEQ && v->b.tp != hVal::FNC) return 0;
    if( v->v.q->size() < 3) return 0;

    hVal *vv = v->v.q->at(2);
    ++vv->b.c;

    ec.sd->push_back(vv);
    return 0;
}
static hVal* q3_f(ef_Content &ec){
    if( ec.sd->size() <= 0) return 0;

    register hVal *v = ec.sd->back();
    if( v->b.tp != hVal::DEQ && v->b.tp != hVal::CDEQ && v->b.tp != hVal::FNC) return 0;
    if( v->v.q->size() < 4) return 0;

    hVal *vv = v->v.q->at(3);
    ++vv->b.c;

    ec.sd->push_back(vv);
    return 0;
}
static hVal* q4_f(ef_Content &ec){
    if( ec.sd->size() <= 0) return 0;

    register hVal *v = ec.sd->back();
    if( v->b.tp != hVal::DEQ && v->b.tp != hVal::CDEQ && v->b.tp != hVal::FNC) return 0;
    if( v->v.q->size() < 5) return 0;

    hVal *vv = v->v.q->at(4);
    ++vv->b.c;

    ec.sd->push_back(vv);
    return 0;
}
static hVal* q5_f(ef_Content &ec){
    if( ec.sd->size() <= 0) return 0;

    register hVal *v = ec.sd->back();
    if( v->b.tp != hVal::DEQ && v->b.tp != hVal::CDEQ && v->b.tp != hVal::FNC) return 0;
    if( v->v.q->size() < 6) return 0;

    hVal *vv = v->v.q->at(5);
    ++vv->b.c;

    ec.sd->push_back(vv);
    return 0;
}
static hVal* q6_f(ef_Content &ec){
    if( ec.sd->size() <= 0) return 0;

    register hVal *v = ec.sd->back();
    if( v->b.tp != hVal::DEQ && v->b.tp != hVal::CDEQ && v->b.tp != hVal::FNC) return 0;
    if( v->v.q->size() < 7) return 0;

    hVal *vv = v->v.q->at(6);
    ++vv->b.c;

    ec.sd->push_back(vv);
    return 0;
}
static hVal* q7_f(ef_Content &ec){
    if( ec.sd->size() <= 0) return 0;

    register hVal *v = ec.sd->back();
    if( v->b.tp != hVal::DEQ && v->b.tp != hVal::CDEQ && v->b.tp != hVal::FNC) return 0;
    if( v->v.q->size() < 8) return 0;

    hVal *vv = v->v.q->at(7);
    ++vv->b.c;

    ec.sd->push_back(vv);
    return 0;
}
static hVal* q8_f(ef_Content &ec){
    if( ec.sd->size() <= 0) return 0;

    register hVal *v = ec.sd->back();
    if( v->b.tp != hVal::DEQ && v->b.tp != hVal::CDEQ && v->b.tp != hVal::FNC) return 0;
    if( v->v.q->size() < 9) return 0;

    hVal *vv = v->v.q->at(8);
    ++vv->b.c;

    ec.sd->push_back(vv);
    return 0;
}
static hVal* q9_f(ef_Content &ec){
    if( ec.sd->size() <= 0) return 0;

    register hVal *v = ec.sd->back();
    if( v->b.tp != hVal::DEQ && v->b.tp != hVal::CDEQ && v->b.tp != hVal::FNC) return 0;
    if( v->v.q->size() < 10) return 0;

    hVal *vv = v->v.q->at(9);
    ++vv->b.c;

    ec.sd->push_back(vv);
    return 0;
}
//--
/*
enum {TRUE=0,FALSE,NIL, RETURN,BREAK,REPEATE,CSTRING,CDOUBLE,CMAP,CDEQ,
      STRING,DOUBLE,MAP,DEQ,FNC,FNCC,BUF};
*/

static hVal* prep2_f(ef_Content &ec,hVal *v,hVal *z);

inline static hVal* set_f(ef_Content &ec,int n){
    int cur = ec.sd->size()-n;
    if( cur < 0) return 0;

    register hVal *v = ec.sd->at(cur);
    register hVal *b = ec.sd->back();

    if( v->b.tp == hVal::SQL && (b->b.tp == hVal::STRING || b->b.tp == hVal::CSTRING)) {
        prep2_f(ec,v,b);
        return 0;
    }

    if( v->b.tp < hVal::CSTRING && b->b.tp < hVal::CSTRING) {
        ec.sd->at(cur) = b;
        ec.sd->pop_back();
        return 0;
    }

    if( (v->b.tp == hVal::CDOUBLE || v->b.tp == hVal::DOUBLE) && (b->b.tp == hVal::CDOUBLE  || b->b.tp == hVal::DOUBLE)) {
        v->v.d = b->v.d;
        delete b;

        ec.sd->pop_back();
        return 0;
    }

    if( (v->b.tp == hVal::CDOUBLE || v->b.tp == hVal::DOUBLE) && (b->b.tp == hVal::CSTRING  || b->b.tp == hVal::STRING)) {
        v->v.d = atof(b->v.s->c_str());
        delete b;

        ec.sd->pop_back();
        return 0;
    }

    if( (v->b.tp == hVal::CSTRING || v->b.tp == hVal::STRING) && (b->b.tp == hVal::CSTRING  || b->b.tp == hVal::STRING)) {
        v->set(b->v.s);
        b->v.vl = NULL;
        delete b;

        ec.sd->pop_back();
        return 0;
    }

    if( (v->b.tp == hVal::DEQ || v->b.tp == hVal::CDEQ || v->b.tp == hVal::FNC) && (b->b.tp == hVal::DEQ  || b->b.tp == hVal::CDEQ || b->b.tp == hVal::FNC)) {
        v->set(b->v.q);
        if( b->b.tp == hVal::DEQ || b->b.tp == hVal::FNC){
            b->v.vl = NULL;
            delete b;
        }

        ec.sd->pop_back();
        return 0;
    }

    if( (v->b.tp == hVal::MAP || v->b.tp == hVal::CMAP) && (b->b.tp == hVal::MAP  || b->b.tp == hVal::CMAP)) {
        if( v == ec.G) ec.wl();
        v->set(b->v.m);
        if( v == ec.G) ec.wu();

        if( b->b.tp == hVal::MAP){
            b->v.vl = NULL;
            delete b;
        }

        ec.sd->pop_back();
        return 0;
    }
/*
    if( v->b.tp == b->b.tp){
        v->v.vl = b->v.vl;
        b->v.vl = NULL;
    }else{
*/
        v->b.tp = b->b.tp;
        v->v.vl = b->v.vl;
        b->v.vl = NULL;
//    }

//    v->v.vl = b->v.vl;
//    b->v.vl = NULL;

    delete b;

    ec.sd->pop_back();
    return 0;
}
static hVal* set1_f(ef_Content &ec){  return set_f(ec,2);}

static hVal* set2_f(ef_Content &ec){  return set_f(ec,3);}
static hVal* set3_f(ef_Content &ec){  return set_f(ec,4);}
static hVal* set4_f(ef_Content &ec){  return set_f(ec,5);}
static hVal* set5_f(ef_Content &ec){  return set_f(ec,6);}
static hVal* set6_f(ef_Content &ec){  return set_f(ec,7);}
static hVal* set7_f(ef_Content &ec){  return set_f(ec,8);}
static hVal* set8_f(ef_Content &ec){  return set_f(ec,9);}
static hVal* set9_f(ef_Content &ec){  return set_f(ec,10);}
//static hVal* set10_f(ef_Content &ec){  return set_f(ec,11);}

static hVal* sv0_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-1;
    if( v_sz < 0) return 0;
    int cur = ec.sd->size()- ec.sdv->at(v_sz)+1;

    return set_f(ec,cur);
}
static hVal* sv1_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-1;
    if( v_sz < 0) return 0;
    int cur = ec.sd->size()-ec.sdv->at(v_sz)+2;

    return set_f(ec,cur);
}
static hVal* sv2_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-1;
    if( v_sz < 0) return 0;
    int cur = ec.sd->size()-ec.sdv->at(v_sz)+3;

    return set_f(ec,cur);
}
static hVal* sv3_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-1;
    if( v_sz < 0) return 0;
    int cur = ec.sd->size()-ec.sdv->at(v_sz)+4;
//fprintf(stderr,"cur %d v_sz %d %d %d\n",cur,v_sz,ec.sd->size(),ec.sdv->at(v_sz)); fflush(stderr);

    return set_f(ec,cur);
}
static hVal* sv4_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-1;
    if( v_sz < 0) return 0;
    int cur = ec.sd->size()-ec.sdv->at(v_sz)+5;

    return set_f(ec,cur);
}
static hVal* sv5_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-1;
    if( v_sz < 0) return 0;
    int cur = ec.sd->size()-ec.sdv->at(v_sz)+6;

    return set_f(ec,cur);
}
static hVal* sv6_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-1;
    if( v_sz < 0) return 0;
    int cur = ec.sd->size()-ec.sdv->at(v_sz)+7;

    return set_f(ec,cur);
}
static hVal* sv7_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-1;
    if( v_sz < 0) return 0;
    int cur = ec.sd->size()-ec.sdv->at(v_sz)+8;

    return set_f(ec,cur);
}
static hVal* sv8_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-1;
    if( v_sz < 0) return 0;
    int cur = ec.sd->size()-ec.sdv->at(v_sz)+9;

    return set_f(ec,cur);
}
static hVal* sv9_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-1;
    if( v_sz < 0) return 0;
    int cur = ec.sd->size()-ec.sdv->at(v_sz)+10;

    return set_f(ec,cur);
}
//--
static hVal* sv10_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-2;
    if( v_sz < 0) return 0;
    int cur = ec.sd->size()- ec.sdv->at(v_sz)+1;

    return set_f(ec,cur);
}
static hVal* sv11_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-2;
    if( v_sz < 0) return 0;
    int cur = ec.sd->size()-ec.sdv->at(v_sz)+2;

    return set_f(ec,cur);
}
static hVal* sv12_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-2;
    if( v_sz < 0) return 0;
    int cur = ec.sd->size()-ec.sdv->at(v_sz)+3;

    return set_f(ec,cur);
}
static hVal* sv13_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-2;
    if( v_sz < 0) return 0;
    int cur = ec.sd->size()-ec.sdv->at(v_sz)+4;
//fprintf(stderr,"cur %d v_sz %d %d %d\n",cur,v_sz,ec.sd->size(),ec.sdv->at(v_sz)); fflush(stderr);

    return set_f(ec,cur);
}
static hVal* sv14_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-2;
    if( v_sz < 0) return 0;
    int cur = ec.sd->size()-ec.sdv->at(v_sz)+5;

    return set_f(ec,cur);
}
static hVal* sv15_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-2;
    if( v_sz < 0) return 0;
    int cur = ec.sd->size()-ec.sdv->at(v_sz)+6;

    return set_f(ec,cur);
}
static hVal* sv16_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-2;
    if( v_sz < 0) return 0;
    int cur = ec.sd->size()-ec.sdv->at(v_sz)+7;

    return set_f(ec,cur);
}
static hVal* sv17_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-2;
    if( v_sz < 0) return 0;
    int cur = ec.sd->size()-ec.sdv->at(v_sz)+8;

    return set_f(ec,cur);
}
static hVal* sv18_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-2;
    if( v_sz < 0) return 0;
    int cur = ec.sd->size()-ec.sdv->at(v_sz)+9;

    return set_f(ec,cur);
}
static hVal* sv19_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-2;
    if( v_sz < 0) return 0;
    int cur = ec.sd->size()-ec.sdv->at(v_sz)+10;

    return set_f(ec,cur);
}
//--
static hVal* sv20_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-3;
    if( v_sz < 0) return 0;
    int cur = ec.sd->size()- ec.sdv->at(v_sz)+1;

    return set_f(ec,cur);
}
static hVal* sv21_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-3;
    if( v_sz < 0) return 0;
    int cur = ec.sd->size()-ec.sdv->at(v_sz)+2;

    return set_f(ec,cur);
}
static hVal* sv22_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-3;
    if( v_sz < 0) return 0;
    int cur = ec.sd->size()-ec.sdv->at(v_sz)+3;

    return set_f(ec,cur);
}
static hVal* sv23_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-3;
    if( v_sz < 0) return 0;
    int cur = ec.sd->size()-ec.sdv->at(v_sz)+4;
//fprintf(stderr,"cur %d v_sz %d %d %d\n",cur,v_sz,ec.sd->size(),ec.sdv->at(v_sz)); fflush(stderr);

    return set_f(ec,cur);
}
static hVal* sv24_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-3;
    if( v_sz < 0) return 0;
    int cur = ec.sd->size()-ec.sdv->at(v_sz)+5;

    return set_f(ec,cur);
}
static hVal* sv25_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-3;
    if( v_sz < 0) return 0;
    int cur = ec.sd->size()-ec.sdv->at(v_sz)+6;

    return set_f(ec,cur);
}
static hVal* sv26_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-3;
    if( v_sz < 0) return 0;
    int cur = ec.sd->size()-ec.sdv->at(v_sz)+7;

    return set_f(ec,cur);
}
static hVal* sv27_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-3;
    if( v_sz < 0) return 0;
    int cur = ec.sd->size()-ec.sdv->at(v_sz)+8;

    return set_f(ec,cur);
}
static hVal* sv28_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-3;
    if( v_sz < 0) return 0;
    int cur = ec.sd->size()-ec.sdv->at(v_sz)+9;

    return set_f(ec,cur);
}
static hVal* sv29_f(ef_Content &ec){
    int v_sz = ec.sdv->size()-3;
    if( v_sz < 0) return 0;
    int cur = ec.sd->size()-ec.sdv->at(v_sz)+10;

    return set_f(ec,cur);
}

//--

inline static hVal* qset_f(ef_Content &ec,int n){
    int vn = ec.sd->size() - 2;
    if( vn < 0) return 0;

    register hVal *vv = ec.sd->at(vn);
    if( vv->b.tp != hVal::DEQ && vv->b.tp != hVal::CDEQ && vv->b.tp != hVal::FNC) return 0;
    if( vv->v.q->size() < (n+1)) return 0;

    register hVal *v = vv->v.q->at(n);

    register hVal *b = ec.sd->back();

    if( v->b.tp < hVal::CSTRING && b->b.tp < hVal::CSTRING) {
        vv->v.q->at(n) = b;
        ec.sd->pop_back();
        return 0;
    }

    if( (v->b.tp == hVal::CDOUBLE || v->b.tp == hVal::DOUBLE) && (b->b.tp == hVal::CDOUBLE  || b->b.tp == hVal::DOUBLE)) {
        v->v.d = b->v.d;
        delete b;

        ec.sd->pop_back();
        return 0;
    }

    if( (v->b.tp == hVal::CSTRING || v->b.tp == hVal::STRING) && (b->b.tp == hVal::CSTRING  || b->b.tp == hVal::STRING)) {
        v->set(b->v.s);
        b->v.vl = NULL;
        delete b;

        ec.sd->pop_back();
        return 0;
    }

    if( (v->b.tp == hVal::DEQ || v->b.tp == hVal::CDEQ || v->b.tp == hVal::FNC) && (b->b.tp == hVal::DEQ  || b->b.tp == hVal::CDEQ || b->b.tp == hVal::FNC)) {
        v->set(b->v.q);
        if( b->b.tp == hVal::DEQ || b->b.tp == hVal::FNC){
            b->v.vl = NULL;
            delete b;
        }

        ec.sd->pop_back();
        return 0;
    }

    if( (v->b.tp == hVal::MAP || v->b.tp == hVal::CMAP) && (b->b.tp == hVal::MAP  || b->b.tp == hVal::CMAP)) {
        if( v == ec.G) ec.wl();
        v->set(b->v.m);
        if( v == ec.G) ec.wu();

        if( b->b.tp == hVal::MAP){
            b->v.vl = NULL;
            delete b;
        }

        ec.sd->pop_back();
        return 0;
    }

    if( v->b.tp == b->b.tp){
        v->v.vl = b->v.vl;
        b->v.vl = NULL;
    }

end:
    delete b;

    ec.sd->pop_back();
    return 0;
}

static hVal* sq0_f(ef_Content &ec){return  qset_f(ec,0);}
static hVal* sq1_f(ef_Content &ec){return  qset_f(ec,1);}
static hVal* sq2_f(ef_Content &ec){return  qset_f(ec,2);}
static hVal* sq3_f(ef_Content &ec){return  qset_f(ec,3);}
static hVal* sq4_f(ef_Content &ec){return  qset_f(ec,4);}
static hVal* sq5_f(ef_Content &ec){return  qset_f(ec,5);}
static hVal* sq6_f(ef_Content &ec){return  qset_f(ec,6);}
static hVal* sq7_f(ef_Content &ec){return  qset_f(ec,7);}
static hVal* sq8_f(ef_Content &ec){return  qset_f(ec,8);}
static hVal* sq9_f(ef_Content &ec){return  qset_f(ec,9);}

inline static hVal* sget_f(ef_Content &ec,int n){
    if( n >= ec.sd->size()) return 0;
    register hVal *v = ec.sd->at(n);
    ++v->b.c;
    ec.sd->push_back(v);

    return 0;
}
static hVal* sfg0_f(ef_Content &ec){return  sget_f(ec,0);}
static hVal* sfg1_f(ef_Content &ec){return  sget_f(ec,1);}
static hVal* sfg2_f(ef_Content &ec){return  sget_f(ec,2);}
static hVal* sfg3_f(ef_Content &ec){return  sget_f(ec,3);}
static hVal* sfg4_f(ef_Content &ec){return  sget_f(ec,4);}
static hVal* sfg5_f(ef_Content &ec){return  sget_f(ec,5);}
static hVal* sfg6_f(ef_Content &ec){return  sget_f(ec,6);}
static hVal* sfg7_f(ef_Content &ec){return  sget_f(ec,7);}
static hVal* sfg8_f(ef_Content &ec){return  sget_f(ec,8);}
static hVal* sfg9_f(ef_Content &ec){return  sget_f(ec,9);}

//--
static hVal* drop_f(ef_Content &ec){
    int max = ec.sdv->back();
    int cur = ec.sd->size();

//    fprintf(stderr,"%d %d \n",cur,max); fflush(stderr);

    if( cur == 0 || ( cur <= max)) return 0;

    hVal *v = ec.sd->back();

    ec.sd->pop_back();

    delete v;

    return 0;
}

static hVal* dropAll_f(ef_Content &ec){
    int max = ec.sdv->back();
//    int cur = ec.sd->size();

//    fprintf(stderr,"%d %d \n",cur,max); fflush(stderr);
    while(1){
//        int max = ec.sdv->back();
        int cur = ec.sd->size();

        if( cur == 0 || ( cur <= max)) return 0;

        hVal *v = ec.sd->back();
        ec.sd->pop_back();
        delete v;
    }

    return 0;
}

static hVal* swap_f(ef_Content &ec){
    int max = ec.sdv->back();
    int cur = ec.sd->size();

//    fprintf(stderr," == %d %d \n",cur,max); fflush(stderr);

    if( ec.sd->size() < 2 || ( (cur - max) < 2)) return 0;

    fprintf(stderr,"%d %d \n",cur,max); fflush(stderr);


    hVal *v = ec.sd->at(cur-1);
    ec.sd->at(cur-1) = ec.sd->at(cur-2);
    ec.sd->at(cur-2) = v;

    return 0;
}

//--

static hVal* clr_f(ef_Content &ec)
{
    if( ec.sd->size() == 0) return 0;

    hVal *v = ec.sd->back();

    switch( v->b.tp){

    case hVal::DOUBLE:
    case hVal::CDOUBLE:
        v->v.d = 0;

        break;

    case hVal::STRING:
    case hVal::CSTRING:
    case hVal::BUF:
        if( v->v.s) delete v->v.s;
        v->v.s = ec.ef.new_str("");

        break;

    case hVal::MAP:
    case hVal::CMAP:
    {
        register HHMapSV_T *o = v->v.m;
        if( v == ec.G) ec.wl();

        register HHMapSV_TI it = o->begin();
        while( it != o->end()){
            hVal *z = it->second;
            delete z;

            ++it;
        }
        if( v == ec.G) ec.wu();

        o->clear();
    }

        break;

    case hVal::DEQ:
    case hVal::CDEQ:
    {
        register HHDeqV_T *o = v->v.q;

        register HHDeqV_TI it = o->begin();
        while( it != o->end()){
            delete *it;

            ++it;
        }
        o->clear();
    }

        break;

    case hVal::DB:

//        PGconn *c = (PGconn *)v->v.vl;
        if( v->v.vl) PQfinish((PGconn*)v->v.vl);
        v->v.vl = NULL;

        break;

    case hVal::UDP:
    {
        UDP_T * d = (UDP_T *)v->v.vl;
        if( d->s >=0) hh_closesocket(d->s);
        if( d->sc >=0) hh_closesocket(d->sc);
        d->s = d->sc = -1;
    }
        break;

    case hVal::SQL:

//        if( v->v.vl) delete (SqlAdd*)v->v.vl;
//        v->v.vl = NULL;
        SqlAdd *p = (SqlAdd *)v->v.vl;
        PGconn *c = (PGconn *)p->con->v.vl;
        if( p->res) {
            PQclear(p->res);
            p->res = 0;
        }

        if( p->stnm.size() != 0) {
            char b[300];
            sprintf(b,"DEALLOCATE %s",p->stnm.c_str());
            PGresult *res = PQexec( c, b);
            if( PQresultStatus(res) == PGRES_FATAL_ERROR) {
                fprintf(stderr, "PQexec <%s> failed: %s Status: %d\n",b,PQerrorMessage(c),PQresultStatus(res));fflush(stderr);
            }
            PQclear(res);
        }
        p->stnm.clear();// = "";
        p->pos = 0;

        break;
    }


    return 0;
}

static hVal* len_f(ef_Content &ec){
    if( ec.sd->size() == 0) return 0;

    hVal *v = ec.sd->back();

    switch( v->b.tp){

    case hVal::STRING:
    case hVal::CSTRING:
    case hVal::BUF:
        ec.sd->push_back(ec.ef.new_hVal(hVal::DOUBLE,0,v->v.s->length()));

        break;

    case hVal::MAP:
    case hVal::CMAP:
        ec.sd->push_back(ec.ef.new_hVal(hVal::DOUBLE,0,v->v.m->size()));
        break;

    case hVal::DEQ:
    case hVal::CDEQ:
    case hVal::FNC:
        ec.sd->push_back(ec.ef.new_hVal(hVal::DOUBLE,0,v->v.q->size()));
        break;
    }

    return 0;
}

#include <stdarg.h>     /* va_list, va_start, va_arg, va_end */

/*
#if (defined( __linux__) && defined(__x86_64__))
// AMD64 byte-aligns elements to 8 bytes
#define VLIST_CHUNK_SIZE 8
#else
#define VLIST_CHUNK_SIZE 4
#define _va_list_ptr _va_list
#endif

typedef struct  {
    va_list _va_list;
#if (defined( __linux__) && defined(__x86_64__))
    void* _va_list_ptr;
#endif
} my_va_list;

void my_va_start(my_va_list* args, void* arg_list)
{
#if (defined(__linux__) && defined(__x86_64__))

    args->_va_list[0].gp_offset = 48;
    args->_va_list[0].fp_offset = 304;
    args->_va_list[0].reg_save_area = NULL;
    args->_va_list[0].overflow_arg_area = arg_list;
#endif
    args->_va_list_ptr = arg_list;
}

void my_va_end(my_va_list* args)
{
    free(args->_va_list_ptr);
}

typedef struct {
    ArgFormatType type; // OP defined this enum for format
    union {
        int i;
        // OTHER TYPES HERE
        void* p;
    } data;
} va_data;

void* create_arg_pointer(va_data* arguments, unsigned int num_args) {
    int i, arg_list_size = 0;
    void* arg_list = NULL;

    for (i=0; i < num_args; ++i)
    {
        unsigned int native_data_size, padded_size;
        void *native_data, *vdata;

        switch(arguments[i].type)
        {
            case ArgType_int:
                native_data = &(arguments[i].data.i);
                native_data_size = sizeof(arguments[i]->data.i);
                break;
            // OTHER TYPES HERE
            case ArgType_string:
                native_data = &(arguments[i].data.p);
                native_data_size = sizeof(arguments[i]->data.p);
                break;
            default:
                // error handling
                continue;
        }

        // if needed, pad the size we will use for the argument in the va_list
        for (padded_size = native_data_size; 0 != padded_size % VLIST_CHUNK_SIZE; padded_size++);

        // reallocate more memory for the additional argument
        arg_list = (char*)realloc(arg_list, arg_list_size + padded_size);

        // save a pointer to the beginning of the free space for this argument
        vdata = &(((char *)(arg_list))[arg_list_size]);

        // increment the amount of allocated space (to provide the correct offset and size for next time)
        arg_list_size += padded_size;

        // set full padded length to 0 and copy the actual data into the location
        memset(vdata, 0, padded_size);
        memcpy(vdata, native_data, native_data_size);
    }

    return arg_list;
}
*/
static hVal* vsprintf_f(ef_Content &ec){

    char* m = (char*) malloc(sizeof(int)*2 + sizeof(char*)); /* prepare enough memory*/
    void* bm = m; /* copies the pointer */
    char* string = "I am a string!!"; /* an example string */

    (*(int*)m) = 10; /*puts the first value */
    m += sizeof(int); /* move forward the pointer to the next element */

    (*(char**)m) = string; /* puts the next value */
    m += sizeof(char*); /* move forward again*/

    (*(int*)m) = 20; /* puts the third element */
    m += sizeof(int); /* unneeded, but here for clarity. */

    va_list arr;
//    std::va_list va;
//    va = bm;


// /home/hh/ws/hjs/ef_proj/ef.cpp:2838: error:   initializing argument 2 of 'int vprintf(const char*, __va_list_tag*)'
//home/hh/ws/hjs/ef_proj/ef.cpp:2845: ошибка: invalid conversion from 'void*' to '__va_list_tag*'
///home/hh/ws/hjs/ef_proj/ef.cpp:2846: ошибка: ISO C++ forbids casting to an array type '__va_list_tag [1]'

//    vprintf("%d %s %d\n",(va_list)(bm)); /* the deep magic starts here...*/

//    arr = (va_list)bm;

//    va_start(arr,bm);

//    vprintf("%d %s %d\n",reinterpret_cast<va_list>(bm)); /* the deep magic starts here...*/
/*
    std::vector<std::string> strings;
    std::string s("a");
    int i(0);

    // create unique strings...
    for (; i != 5; ++i)
    {
        strings.push_back(s);
//        ++s.front();
    }
    vprintf("%s %s %s %s %s \n",reinterpret_cast<va_list>(strings.data()));
*/

    free(bm);

    return 0;

    int qp = ec.sd->size() - 2;
    if( qp < 0) return 0;

    hVal *v = ec.sd->back();
    hVal *q = ec.sd->at(qp);

    if( !(v->b.tp == hVal::STRING || v->b.tp == hVal::CSTRING) || !(q->b.tp == hVal::DEQ || q->b.tp == hVal::CDEQ)){
        return 0;
    }

    va_list ar;

    register HHDeqV_TI it = q->v.q->begin();
    while( it != q->v.q->end()){
        delete *it;

        ++it;
    }


    return 0;
}

//--

static hVal* format_f(ef_Content &ec){
    int max = ec.sdv->back();
    int dp = ec.sd->size() - 2;

    if( dp < 0 || ( dp < max)) return 0;

    hVal *v = ec.sd->back();
    hVal *d = ec.sd->at(dp);

    if( !(v->b.tp == hVal::STRING || v->b.tp == hVal::CSTRING)){ return 0;}

    std::string fm(*v->v.s);
    char *p = (char *)v->v.s->c_str();
    p = strchr(p,'%');
    if( p) {
        p = strchr(p+1,'%');
        if( p){
            fm = v->v.s->substr(0, p - v->v.s->c_str());
        }
    }

//    if() p = strchr()

    switch(d->b.tp){

    case hVal::DOUBLE:
    case hVal::CDOUBLE:
    {
//        char b[1000];
        char *b = (char *)malloc(v->v.s->size() + 200);
        if( strstr(fm.c_str(),"%x") || strstr(fm.c_str(),"%X") || strstr(fm.c_str(),"%0")){
            sprintf(b,fm.c_str(),(long long)d->v.d);
        }else{
            sprintf(b,fm.c_str(),d->v.d);
        }

        hVal * r = ec.ef.new_hVal(hVal::STRING,ec.ef.new_str(b),0);
        delete v;
        delete d;

        ec.sd->pop_back();
        ec.sd->back() = r;
//        ec.sd->pop_back();
//        ec.sd->push_back(r);
        free(b);
        return 0;
    }
        break;

    case hVal::STRING:
    case hVal::CSTRING:
    {
        char *b = (char *)malloc((v->v.s->size() + d->v.s->size())*2 + 10 );
        sprintf(b,fm.c_str(),d->v.s->c_str());
        hVal * r = ec.ef.new_hVal(hVal::STRING,ec.ef.new_str(b),0);
        delete v;
        delete d;

        ec.sd->pop_back();
        ec.sd->back() = r;
//        ec.sd->pop_back();
//        ec.sd->push_back(r);
        free(b);
        return 0;
    }
        break;

    default:
    {
        char *b = (char *)malloc(v->v.s->size() + 200);
        sprintf(b,fm.c_str(),tp_f(d->b.tp).c_str());
        hVal * r = ec.ef.new_hVal(hVal::STRING,ec.ef.new_str(b),0);
        delete v;
        delete d;

        ec.sd->pop_back();
        ec.sd->back() = r;
//        ec.sd->pop_back();
//        ec.sd->push_back(r);
        free(b);

        return 0;
    }

        break;
    }

    return 0;
}

static hVal* plus_f(ef_Content &ec){
    int max = ec.sdv->back();
    int dp = ec.sd->size() - 2;

    if( dp < 0 || ( dp < max)) return 0;

    hVal *v = ec.sd->back();
    hVal *d = ec.sd->at(dp);

    if( (v->b.tp == hVal::STRING || v->b.tp == hVal::CSTRING) && (d->b.tp == hVal::STRING || d->b.tp == hVal::CSTRING)){

        hVal * r = ec.ef.new_hVal(hVal::STRING,ec.ef.new_str(*d->v.s + *v->v.s),0);
        delete v;
        delete d;

        ec.sd->pop_back();
        ec.sd->back() = r;
//        ec.sd->pop_back();
//        ec.sd->push_back(r);

        return 0;
    }

    if( (v->b.tp == hVal::DOUBLE || v->b.tp == hVal::CDOUBLE) && (d->b.tp == hVal::DOUBLE || d->b.tp == hVal::CDOUBLE)){

        hVal * r = ec.ef.new_hVal(hVal::DOUBLE,0,d->v.d + v->v.d);
        delete v;
        delete d;

        ec.sd->pop_back();
        ec.sd->back() = r;
//        ec.sd->pop_back();
//        ec.sd->push_back(r);

        return 0;
    }

    if( (v->b.tp == hVal::DEQ || v->b.tp == hVal::CDEQ) && (d->b.tp == hVal::MAP || d->b.tp == hVal::CMAP)){

//        hVal * r = ec.ef.new_hVal(hVal::DOUBLE,0,d->v.d + v->v.d);

        HHDeqV_T *q = v->v.q;
        if( q->size() < 2) return 0;

        hVal *qk = q->at(0);
        if( qk->b.tp != hVal::STRING && qk->b.tp != hVal::CSTRING) return 0;
        hVal *qv = q->at(1);

        if( d == ec.G) ec.wl();
        d->set(*qk->v.s,qv);
        if( d == ec.G) ec.wu();

        delete v;

        ec.sd->pop_back();

        return 0;
    }

    if( (v->b.tp == hVal::STRING || v->b.tp == hVal::CSTRING) && (d->b.tp == hVal::MAP || d->b.tp == hVal::CMAP)){

        std::string k = *v->v.s;

        if( d == ec.G) ec.wl();
        std::pair<HHMapSV_TI,bool> ret = d->v.m->insert ( std::pair<std::string, hVal*>(k,vT) );

        if( !ret.second) {
            HHMapSV_TI it = d->v.m->find(k);
            hVal *l =  it->second;

            if( l->b.tp == hVal::DOUBLE || l->b.tp == hVal::CDOUBLE){
                ++l->v.d;
                if( d == ec.G) ec.wu();
                goto rr;
            }

            if( l->b.tp >= hVal::STRING){
                delete l;
            }

            it->second = ec.ef.new_hVal(hVal::DOUBLE,0,2);
            if( d == ec.G) ec.wu();

        }else{
            if( d == ec.G) ec.wu();
        }

rr:
        delete v;

        ec.sd->pop_back();

        return 0;
    }

    if( (d->b.tp == hVal::DEQ || d->b.tp == hVal::CDEQ)){
        d->push(v);
        ec.sd->pop_back();

        return 0;
    }

    return 0;
}

static hVal* plusplus_f(ef_Content &ec){
//    int max = ec.sdv->back();
    int dp = ec.sd->size() - 1;
//    if( dp < 0 || ( dp < max)) return 0;
    if( dp < 0 ) return 0;

    hVal *v = ec.sd->back();

    if( (v->b.tp == hVal::DOUBLE || v->b.tp == hVal::CDOUBLE) ){
        ++v->v.d;
    }

    return 0;
}
static hVal* minmin_f(ef_Content &ec){
//    int max = ec.sdv->back();
    int dp = ec.sd->size() - 1;
//    if( dp < 0 || ( dp < max)) return 0;
    if( dp < 0 ) return 0;

    hVal *v = ec.sd->back();

    if( (v->b.tp == hVal::DOUBLE || v->b.tp == hVal::CDOUBLE) ){
        --v->v.d;
    }

    return 0;
}

static hVal* min_f(ef_Content &ec){
    int dp = ec.sd->size();
    if( dp <= 0) return 0;
    hVal *v = ec.sd->back();

    if( (v->b.tp == hVal::DEQ || v->b.tp == hVal::CDEQ)){
        ec.sd->push_back(v->pop());
        return 0;
    }

    if( (v->b.tp == hVal::MAP || v->b.tp == hVal::CMAP)){

        HHMapSV_T *m = v->v.m;
        if( m->size() == 0) {
            ec.sd->push_back(vN);
            return 0;
        }

        HHMapSV_TI i = m->end();
        --i;

        std::string k = i->first;
        hVal *z = i->second;

        hVal *e = ec.ef.new_hVal(hVal::DEQ,0,0);
        e->push(ec.ef.new_hVal(hVal::STRING,ec.ef.new_str(k),0));
        e->push(z);
        m->erase(i);

        ec.sd->push_back( e);

        return 0;
    }

    dp -= 2;
    int max = ec.sdv->back();

    if( dp < 0 || ( dp < max)) return 0;

    hVal *d = ec.sd->at(dp);


    if( (v->b.tp == hVal::DOUBLE || v->b.tp == hVal::CDOUBLE) && (d->b.tp == hVal::DOUBLE || d->b.tp == hVal::CDOUBLE)){

        hVal * r = ec.ef.new_hVal(hVal::DOUBLE,0,d->v.d - v->v.d);
        delete v;
        delete d;

        ec.sd->pop_back();
        ec.sd->back() = r;

        return 0;
    }

    if( (v->b.tp == hVal::STRING || v->b.tp == hVal::CSTRING) && (d->b.tp == hVal::MAP || d->b.tp == hVal::CMAP)){

        if( d == ec.G) ec.wl();
        d->del(*v->v.s);
        if( d == ec.G) ec.wu();

        delete v;

        ec.sd->pop_back();

        return 0;
    }

    return 0;
}

bool srtcc_ff (hVal* i,hVal * j) {
    if( (i->b.tp == hVal::DOUBLE || i->b.tp == hVal::CDOUBLE) && (j->b.tp == hVal::DOUBLE || j->b.tp == hVal::CDOUBLE)){
//        bool r = (i->v.d < j->v.d);
//        return r;
        return (bool)(i->v.d < j->v.d);
    }
    if( (i->b.tp == hVal::STRING || i->b.tp == hVal::CSTRING) && (j->b.tp == hVal::STRING || j->b.tp == hVal::CSTRING)){
        return (bool)(*i->v.s < *j->v.s);
    }

    if( (i->b.tp == hVal::STRING || i->b.tp == hVal::CSTRING) && (j->b.tp == hVal::DOUBLE || j->b.tp == hVal::CDOUBLE)){
        return false;
    }
    if( (j->b.tp == hVal::STRING || j->b.tp == hVal::CSTRING) && (i->b.tp == hVal::DOUBLE || i->b.tp == hVal::CDOUBLE)){
        return true;
    }

    return i->b.tp < j->b.tp;
}

bool srtcc_f (hVal* i,hVal * j) {
    return srtcc_ff(i,j);
}
bool srtcc_b (hVal* i,hVal * j) {
    return srtcc_ff(j,i);
}

static hVal* srt_f(ef_Content &ec){
    int dp = ec.sd->size();
    if( dp < 0) return 0;
    hVal *v = ec.sd->back();

    if( (v->b.tp != hVal::DEQ && v->b.tp != hVal::CDEQ)){
        return 0;
    }
    HHDeqV_T *q = v->v.q;
    std::sort (q->begin(), q->end(), srtcc_f);

    return 0;
}
static hVal* rsrt_f(ef_Content &ec){
    int dp = ec.sd->size();
    if( dp < 0) return 0;
    hVal *v = ec.sd->back();

    if( (v->b.tp != hVal::DEQ && v->b.tp != hVal::CDEQ)){
        return 0;
    }
    HHDeqV_T *q = v->v.q;
    std::sort (q->begin(), q->end(), srtcc_b);

    return 0;
}

HHMapSV_T map_tp_v;

static hVal* type_f(ef_Content &ec){
    int dp = ec.sd->size();
    if( dp < 0) return 0;
    hVal *v = ec.sd->back();

    std::string tp = tp_f(v->b.tp);
    HHMapSV_TI it = map_tp_v.find(tp);

    if( it != map_tp_v.end()) {
        ec.sd->push_back( it->second);
    }else{
        hVal * v = ec.ef.new_hVal(hVal::CSTRING,(void *)ec.ef.new_str(tp),0);
        ec.sd->push_back( v);
        map_tp_v.insert(std::pair<std::string, hVal*>(tp,v));
    }

    return 0;
}

static hVal* zv_f(ef_Content &ec){
    int max = ec.sdv->back();
    int dp = ec.sd->size() - 2;

    if( dp < 0 || ( dp < max)) return 0;

    hVal *v = ec.sd->back();
    hVal *d = ec.sd->at(dp);


    if( (v->b.tp == hVal::DOUBLE || v->b.tp == hVal::CDOUBLE) && (d->b.tp == hVal::DOUBLE || d->b.tp == hVal::CDOUBLE)){

        hVal * r = ec.ef.new_hVal(hVal::DOUBLE,0,d->v.d * v->v.d);
        delete v;
        delete d;

        ec.sd->pop_back();
        ec.sd->back() = r;

        return 0;
    }

    if( (v->b.tp == hVal::STRING || v->b.tp == hVal::CSTRING) && (d->b.tp == hVal::MAP || d->b.tp == hVal::CMAP)){

        if( d == ec.G) ec.rl();
        hVal * r = d->get(*v->v.s);
        if( d == ec.G) ec.ru();

        delete v;
        ++r->b.c;
        ec.sd->back() = r;

        return 0;
    }


    return 0;
}

static hVal* delen_f(ef_Content &ec){
    int max = ec.sdv->back();
    int dp = ec.sd->size() - 2;

    if( dp < 0 || ( dp < max)) return 0;

    hVal *v = ec.sd->back();
    hVal *d = ec.sd->at(dp);


    if( (v->b.tp == hVal::DOUBLE || v->b.tp == hVal::CDOUBLE) && (d->b.tp == hVal::DOUBLE || d->b.tp == hVal::CDOUBLE)){

        hVal * r = ec.ef.new_hVal(hVal::DOUBLE,0,d->v.d / v->v.d);
        delete v;
        delete d;

        ec.sd->pop_back();
        ec.sd->back() = r;

        return 0;
    }

    return 0;
}

static hVal* eq_type_f(ef_Content &ec){
    int max = ec.sdv->back();
    int dp = ec.sd->size() - 2;

    if( dp < 0 || ( dp < max)) return 0;

    hVal *v = ec.sd->back();
    hVal *d = ec.sd->at(dp);

    hVal *ret = vF;

    if( (v->b.tp == d->b.tp)){
        ret = vT;
        goto end;
    }
    if( (v->b.tp == hVal::DOUBLE || v->b.tp == hVal::CDOUBLE) && (d->b.tp == hVal::DOUBLE || d->b.tp == hVal::CDOUBLE)){
        ret = vT;
        goto end;
    }
    if( (v->b.tp == hVal::STRING || v->b.tp == hVal::CSTRING) && (d->b.tp == hVal::STRING || d->b.tp == hVal::CSTRING)){
        ret = vT;
        goto end;
    }
    if( (v->b.tp == hVal::MAP || v->b.tp == hVal::CMAP) && (d->b.tp == hVal::MAP || d->b.tp == hVal::CMAP)){
        ret = vT;
        goto end;
    }
    if( (v->b.tp == hVal::DEQ || v->b.tp == hVal::CDEQ) && (d->b.tp == hVal::DEQ || d->b.tp == hVal::CDEQ)){
        ret = vT;
        goto end;
    }

end:

    delete v;
    delete d;

    ec.sd->pop_back();
    ec.sd->back() = ret;

    return 0;
}
static hVal* and_f(ef_Content &ec){
    int max = ec.sdv->back();
    int dp = ec.sd->size() - 2;

    if( dp < 0 || ( dp < max)) return 0;

    hVal *v = ec.sd->back();
    hVal *d = ec.sd->at(dp);

    hVal *ret = vF;

    if( v->b.tp >= hVal::NIL || d->b.tp >= hVal::NIL){
        return 0;
    }
    if( (v->b.tp == hVal::TRUE ) && (d->b.tp == hVal::TRUE )){
        ret = vT;
        goto end;
    }

end:

    ec.sd->pop_back();
    ec.sd->back() = ret;

    return 0;
}
static hVal* or_f(ef_Content &ec){
    int max = ec.sdv->back();
    int dp = ec.sd->size() - 2;

    if( dp < 0 || ( dp < max)) return 0;

    hVal *v = ec.sd->back();
    hVal *d = ec.sd->at(dp);

    hVal *ret = vF;

    if( v->b.tp >= hVal::NIL || d->b.tp >= hVal::NIL){
        return 0;
    }
    if( (v->b.tp == hVal::TRUE ) || (d->b.tp == hVal::TRUE )){
        ret = vT;
        goto end;
    }

end:

    ec.sd->pop_back();
    ec.sd->back() = ret;

    return 0;
}
static hVal* no_f(ef_Content &ec){
    int dp = ec.sd->size() - 1;
    if( dp < 0 ) return 0;

    hVal *v = ec.sd->back();

    hVal *ret = vF;

    if( v->b.tp == hVal::DOUBLE || v->b.tp == hVal::CDOUBLE){
        v->v.d = - v->v.d;
        return 0;
    }

    if( v->b.tp >= hVal::NIL){
        return 0;
    }
    if( v->b.tp == hVal::FALSE ){
        ret = vT;
        goto end;
    }

end:

    ec.sd->back() = ret;

    return 0;
}

static hVal* ot_f(ef_Content &ec,int ot){
    int max = ec.sdv->back();
    int dp = ec.sd->size() - 2;

    if( dp < 0 || ( dp < max)) return 0;

    hVal *d = ec.sd->back();
    hVal *v = ec.sd->at(dp);

    hVal *ret = vF;

    if( (v->b.tp == hVal::DOUBLE || v->b.tp == hVal::CDOUBLE) && (d->b.tp == hVal::DOUBLE || d->b.tp == hVal::CDOUBLE)){

        switch(ot){
        case hVal::G:
            if( v->v.d > d->v.d)ret = vT;
            break;
        case hVal::L:
            if( v->v.d < d->v.d)ret = vT;
            break;
        case hVal::GE:
            if( v->v.d >= d->v.d)ret = vT;
            break;
        case hVal::LE:
            if( v->v.d <= d->v.d)ret = vT;
            break;
        case hVal::NE:
            if( v->v.d != d->v.d)ret = vT;
            break;
        case hVal::E:
            if( v->v.d == d->v.d)ret = vT;
            break;
        }

        goto end;
    }

    if( (v->b.tp == hVal::STRING || v->b.tp == hVal::CSTRING) && (d->b.tp == hVal::STRING || d->b.tp == hVal::CSTRING)){

        switch(ot){
        case hVal::G:
            if( *v->v.s > *d->v.s)ret = vT;
            break;
        case hVal::L:
            if( *v->v.s < *d->v.s)ret = vT;
            break;
        case hVal::GE:
            if( *v->v.s >= *d->v.s)ret = vT;
            break;
        case hVal::LE:
            if( *v->v.s <= *d->v.s)ret = vT;
            break;
        case hVal::NE:
            if( *v->v.s == *d->v.s)ret = vT;
            break;
        case hVal::E:
            if( *v->v.s == *d->v.s)ret = vT;
            break;
        }

        goto end;
    }

    if( (v->b.tp == hVal::DEQ || v->b.tp == hVal::CDEQ) && (d->b.tp == hVal::DEQ || d->b.tp == hVal::CDEQ)){
        int sv = v->v.q->size();
        int sd = d->v.q->size();

        switch(ot){
        case hVal::G:
            if( sv > sd)ret = vT;
            break;
        case hVal::L:
            if( sv < sd)ret = vT;
            break;
        case hVal::GE:
            if( sv >= sd)ret = vT;
            break;
        case hVal::LE:
            if( sv <= sd)ret = vT;
            break;
        case hVal::NE:
            if( sv != sd)ret = vT;
            break;
        case hVal::E:
            if( sv == sd)ret = vT;
            break;
        }

        goto end;
    }

    if( (v->b.tp == hVal::MAP || v->b.tp == hVal::CMAP) && (d->b.tp == hVal::MAP || d->b.tp == hVal::CMAP)){
        int sv = v->v.m->size();
        int sd = d->v.m->size();

        switch(ot){
        case hVal::G:
            if( sv > sd)ret = vT;
            break;
        case hVal::L:
            if( sv < sd)ret = vT;
            break;
        case hVal::GE:
            if( sv >= sd)ret = vT;
            break;
        case hVal::LE:
            if( sv <= sd)ret = vT;
            break;
        case hVal::NE:
            if( sv != sd)ret = vT;
            break;
        case hVal::E:
            if( sv == sd)ret = vT;
            break;
        }

        goto end;
    }

    switch(ot){
    case hVal::G:
        if( v->b.tp > d->b.tp)ret = vT;
        break;
    case hVal::L:
        if( v->b.tp < d->b.tp)ret = vT;
        break;
    case hVal::GE:
        if( v->b.tp >= d->b.tp)ret = vT;
        break;
    case hVal::LE:
        if( v->b.tp <= d->b.tp)ret = vT;
        break;
    case hVal::NE:
        if( v->b.tp != d->b.tp)ret = vT;
        break;
    case hVal::E:
        if( v->b.tp == d->b.tp)ret = vT;
        break;
    }

    goto end;


    return 0;

end:

    delete v;
    delete d;

    ec.sd->pop_back();
    ec.sd->back() = ret;

    return 0;
}

static hVal* g_f(ef_Content &ec){ return ot_f(ec,hVal::G);}
static hVal* l_f(ef_Content &ec){ return ot_f(ec,hVal::L);}
static hVal* ge_f(ef_Content &ec){ return ot_f(ec,hVal::GE);}
static hVal* le_f(ef_Content &ec){ return ot_f(ec,hVal::LE);}
static hVal* ne_f(ef_Content &ec){ return ot_f(ec,hVal::NE);}
static hVal* e_f(ef_Content &ec){ return ot_f(ec,hVal::E);}

static hVal* ut_f(ef_Content &ec)
{
    ec.sd->push_back(ec.ef.new_hVal(hVal::DOUBLE,0,(double)time(0)));

    return 0;
}

static hVal* eqn_f(ef_Content &ec)
{
    if( ec.sd->size() <= 0) {
        goto end;
    }else{
        hVal *d = ec.sd->back();
        if( d->b.tp == hVal::NIL){
            ec.sd->push_back(vT);
            return 0;
        }
    }

end:
    ec.sd->push_back(vF);
    return 0;
}
static hVal* neqn_f(ef_Content &ec)
{
    if( ec.sd->size() <= 0) {
        goto end;
    }else{
        hVal *d = ec.sd->back();
        if( d->b.tp == hVal::NIL){
            ec.sd->push_back(vF);
            return 0;
        }
    }

end:
    ec.sd->push_back(vT);
    return 0;
}

static hVal* tid_f(ef_Content &ec)
{
    ec.sd->push_back(ec.ef.new_hVal(hVal::DOUBLE,0,(double)uv_thread_self()));

    return 0;
}

static hVal* ssize_f(ef_Content &ec)
{
    fprintf(stderr,"Stack Size: %d \n",ec.sd->size());fflush(stderr);
    return 0;
}

static hVal* sl_f(ef_Content &ec)
{
    int sl = 20;
    int f = false;
    hVal *v = NULL;
    if( ec.sd->size() > 0){
        v = ec.sd->back();
        if( v->b.tp == hVal::DOUBLE || v->b.tp == hVal::DOUBLE){
            sl = v->v.d;
            f = true;
        }else if( v->b.tp == hVal::NIL){
            f = true;
        }
    }

    MSLEEP(sl);

    if( f){
        delete v;
        ec.sd->pop_back();
    }

    return 0;
}

//-- locks
static hVal* rl_f(ef_Content &ec)
{
    if( ec.sd->size() > 0  ) {
        hVal *v = ec.sd->back();
        if(v->b.tp == hVal::LOCK){
            hLock *l = (hLock *)v->v.vl;
            l->rl();
/*
            while(1) {
                l->mtx.lock();
                if( l->mc >= 0 ){
                    ++l->mc;
                    l->mtx.unlock();
                    break;//return 0;
                }

                l->mtx.unlock();
                MSLEEP(0);
            }
*/
            delete v;
            ec.sd->pop_back();
            return 0;
        }
        if(v->b.tp == hVal::NIL){
            ec.sd->pop_back();
        }
    }

    ec.rl();
    return 0;
}
static hVal* ru_f(ef_Content &ec)
{
    if( ec.sd->size() > 0  ) {
        hVal *v = ec.sd->back();
        if(v->b.tp == hVal::LOCK){
            hLock *l = (hLock *)v->v.vl;
            l->ru();
/*
            l->mtx.lock();
            if( l->mc > 0 ){
                --l->mc;
            }
            l->mtx.unlock();
*/
            delete v;
            ec.sd->pop_back();
            return 0;
        }
        if(v->b.tp == hVal::NIL){
            ec.sd->pop_back();
        }
    }

    ec.ru();
    return 0;
}
static hVal* wl_f(ef_Content &ec)
{
    if( ec.sd->size() > 0  ) {
        hVal *v = ec.sd->back();
        if(v->b.tp == hVal::LOCK){
            hLock *l = (hLock *)v->v.vl;
            l->wl();
/*
            while(1) {
                l->mtx.lock();
                if( l->mc == 0 ){
                    l->mc = -1;
                    l->mtx.unlock();
                    break;//return 0;
                }

                l->mtx.unlock();
                MSLEEP(0);
            }
*/
            delete v;
            ec.sd->pop_back();
            return 0;
        }
        if(v->b.tp == hVal::NIL){
            ec.sd->pop_back();
        }
    }

    ec.wl();
    return 0;
}
static hVal* wu_f(ef_Content &ec)
{
    if( ec.sd->size() > 0  ) {
        hVal *v = ec.sd->back();
        if(v->b.tp == hVal::LOCK){
            hLock *l = (hLock *)v->v.vl;
            l->wu();
/*
            l->mtx.lock();
            if( l->mc < 0 ){
                l->mc = 0;
            }
            l->mtx.unlock();
*/
            delete v;
            ec.sd->pop_back();
            return 0;
        }
        if(v->b.tp == hVal::NIL){
            ec.sd->pop_back();
        }
    }

    ec.wu();
    return 0;
}

extern int quit_flg;

int mq_done = false;
std::mutex mq_mtx;

int cths = 0;
std::mutex cths_mtx;

static hVal* mquit_f(ef_Content &ec)
{
    if( tprc == uv_thread_self()){
        mq_mtx.lock();
    }else{
        if( !mq_mtx.try_lock()) goto end;
    }

    if( quit_flg && !mq_done && tprc == uv_thread_self()){
        ec.rl();
        std::string mq= PREF_MAIN;
        mq += ".mq";
        HHMapSV_TI it = ec.g->find(mq.c_str());
        if( it != ec.g->end()) {
            hVal *v = it->second;

            ec.ru();

            ef_run(ec,ef,v);

        }else{
            ec.ru();
        }

        time_t lt = time(0);
        while( cths > 0 && (time(0) - lt) < 5){
            MSLEEP(10);
        }

        uv_stop(uv_default_loop());
        mq_done = true;

        mq_mtx.unlock();
        return vR;
    }

    mq_mtx.unlock();

end:

    if( quit_flg) { return vR;}

    return 0;
}

static hVal* exit_f(ef_Content &ec)
{
    quit_flg = 1;
    mquit_f( ec);

    return 0;
}

static hVal* delen_ch_f(ef_Content &ec){
    int max = ec.sdv->back();
    int dp = ec.sd->size() - 2;

    if( dp < 0 || ( dp < max)) return 0;

    hVal *v = ec.sd->back();
    hVal *d = ec.sd->at(dp);


    if( (v->b.tp == hVal::DOUBLE || v->b.tp == hVal::CDOUBLE) && (d->b.tp == hVal::DOUBLE || d->b.tp == hVal::CDOUBLE)){

        hVal * r = ec.ef.new_hVal(hVal::DOUBLE,0,(long long)d->v.d % (long long)v->v.d);
        delete v;
        delete d;

        ec.sd->pop_back();
        ec.sd->back() = r;

        return 0;
    }

    return 0;
}

//--
static hVal* f100_f(ef_Content &ec){
    ec.sd->push_back(ec.ef.new_hVal(100,(void *) 123,0));
    return 0;
}


void ef_1exec(char *se)
{
    ef_init(ec,ef,argc_,argv_);
    hVal * r = ef_compile(ec,ef,se,0);
    if( r->b.tp == hVal::STRING){
//        std::cerr << *r->v.s << " <--ef_compile ERROR!\n";
        std::cerr << *r->v.s;
        delete r;
        return;
    }
    ec.rl();
    HHMapSV_TI it = ec.g->find(PREF_MAIN);
    if( it != ec.g->end()) {
        hVal *v = it->second;
        ec.ru();
        ef_run(ec,ef,v);
    }else{
        ec.ru();
    }
}

void timer_cb(uv_timer_t* h)
{
//    fprintf(stderr,"ТИМЕР ИДЕТ\n");fflush(stderr);

    hVal *v = (hVal *)h->data;

    ef_run(ec,ef,v);

    return;
}

static hVal* timer_f(ef_Content &ec)
{

    if( ec.sd->size() == 0){
        std::cerr << "ERROR: 'TIMER' zero size stack!\n"; std::cerr.flush();
        return 0;
    }

    register hVal *v = ec.sd->back();

    if( v->b.tp != hVal::DEQ && v->b.tp != hVal::CDEQ) {
        std::cerr << "ERROR: 'TIMER' stack is NOT DEQ!\n"; std::cerr.flush();
        return 0;
    }
    if( v->v.q->size() < 1){
        std::cerr << "ERROR: 'TIMER' DEQ.size() == 0!\n"; std::cerr.flush();
        return 0;
    }

    hVal *z = v->v.q->at(0);

    if( z->b.tp != hVal::FNC){
        std::cerr << "ERROR: 'TIMER' DEQ.at(0) not a FNC!\n"; std::cerr.flush();
        return 0;
    }

    int msec = 1000;

    if( v->v.q->size() > 1) {
        hVal *t = v->v.q->at(1);
        if( t->b.tp == hVal::DOUBLE || t->b.tp == hVal::CDOUBLE){
            msec = t->v.d;
        }
    }

    uv_timer_t* ht = (uv_timer_t*)malloc(sizeof(uv_timer_t));
    uv_timer_init(uv_default_loop(), ht);
    ht->data = z;
    ++z->b.c;

    uv_timer_start(ht, timer_cb, msec, msec);

    delete v;
    ec.sd->pop_back();

    return 0 ;
}

void thread_cf(void* p)
{
    cths_mtx.lock();
    ++cths;
    cths_mtx.unlock();

    ef_Content e(ec.g,ec.f,ec.c,NULL,NULL,NULL);

    hVal *v = (hVal *)p;
    hVal *ve = (hVal *)NULL;
    int i=0;
    HHDeqV_TI it;

    register HHDeqV_T *o = v->v.q;
    if( !o) {
        delete v;
        goto end;
    }

    it = o->begin();
    while( it != o->end()){
        hVal *vv = *it;
        ++vv->b.c;
        if( i == 0){
            ve = vv;
        }else{
            e.sd->push_back(vv);
        }

        ++it;++i;
    }

    delete v;
    ef_run(e,ef,ve);

end:
    e.g = NULL;
    e.f = NULL;
    e.c = NULL;

    cths_mtx.lock();
    --cths;
    cths_mtx.unlock();
}

static hVal* th_f(ef_Content &ec)
{

    if( ec.sd->size() == 0){
        std::cerr << "ERROR: 'THREAD' zero size stack!\n"; std::cerr.flush();
        return 0;
    }

    register hVal *v = ec.sd->back();

    if( v->b.tp != hVal::DEQ && v->b.tp != hVal::CDEQ) {
        std::cerr << "ERROR: 'THREAD' stack is NOT DEQ!\n"; std::cerr.flush();
        return 0;
    }
    if( v->v.q->size() < 1){
        std::cerr << "ERROR: 'THREAD' DEQ.size() == 0!\n"; std::cerr.flush();
        return 0;
    }

    hVal *z = v->v.q->at(0);

    if( z->b.tp != hVal::FNC){
        std::cerr << "ERROR: 'THREAD' DEQ.at(0) not a FNC!\n"; std::cerr.flush();
        return 0;
    }

//    ++v->b.c;
    uv_thread_t t_id;
    uv_thread_create(&t_id, thread_cf, v);

//    delete v;
    ec.sd->pop_back();

    return 0 ;
}

static hVal* db_f(ef_Content &ec)
{

    if( ec.sd->size() == 0){
        std::cerr << "ERROR: 'DB' zero size stack!\n"; std::cerr.flush();
        return 0;
    }

//    return 0;

    register hVal *v = ec.sd->back();

    if( v->b.tp != hVal::DEQ && v->b.tp != hVal::CDEQ) {
        std::cerr << "ERROR: 'DB' stack is NOT DEQ!\n"; std::cerr.flush();
        return 0;
    }

    HHDeqV_T *q = v->v.q;

    if( q->size() < 4) {
        std::cerr << "ERROR: 'DB' DEQ < 4 size!\n"; std::cerr.flush();
        return 0;
    }

    char *a[10];

    for(int i=0; i < 4; ++i){
        hVal *p = q->at(i);
        if( p->b.tp != hVal::STRING && p->b.tp != hVal::CSTRING) {
//            std::cerr << "ERROR: 'DB' DEQ < 4 size!\n"; std::cerr.flush();
            fprintf(stderr, "ERROR: 'DB' DEC element %d not a STRING!\n", i);

            return 0;
        }
        a[i] = (char*)p->v.s->c_str();
    }

    PGconn * conn = PQsetdbLogin(a[0],
                         NULL,
                         NULL,
                         NULL,
                         a[1],
                         a[2],
                         a[3]);

    delete v;

    if (PQstatus(conn) != CONNECTION_OK){
        fprintf(stderr, "Connection to database failed: %s\n", PQerrorMessage(conn));
        ec.sd->back() = vN;
    }else{
        ec.sd->back() = ec.ef.new_hVal(hVal::DB,conn,0);
    }

    return 0 ;
}

static hVal* sql_f(ef_Content &ec)
{

    if( ec.sd->size() == 0){
        std::cerr << "ERROR: 'SQL' zero size stack!\n"; std::cerr.flush();
        return 0;
    }

    register hVal *v = ec.sd->back();

    if( v->b.tp != hVal::DEQ && v->b.tp != hVal::CDEQ) {
        std::cerr << "ERROR: 'SQL' stack is NOT DEQ!\n"; std::cerr.flush();
        return 0;
    }

    HHDeqV_T *q = v->v.q;

    if( q->size() < 2) {
        std::cerr << "ERROR: 'SQL' DEQ < 2 size!\n"; std::cerr.flush();
        return 0;
    }

    hVal *db = q->at(0);
    hVal *s = q->at(1);

    if( db->b.tp != hVal::DB){
        std::cerr << "ERROR: 'SQL' DEQ[0] not DB!\n"; std::cerr.flush();
        return 0;
    }

    if( s->b.tp != hVal::STRING && s->b.tp != hVal::CSTRING){
        std::cerr << "ERROR: 'SQL' DEQ[1] not STRING!\n"; std::cerr.flush();
        return 0;
    }

    //-----------------

    SqlAdd *a = new SqlAdd();

    ++s->b.c; a->sql = s;
    ++db->b.c; a->con = db;

    int flg_exec = 1;
    if( strchr(s->v.s->c_str(),'$') != NULL || strchr(s->v.s->c_str(),':') != NULL ) flg_exec = 0;
    a->flg_exec = flg_exec;

    PGresult   *res = (PGresult   *)0;
    if( q->size() > 2 ) {//exec
        PGconn *con = (PGconn *)db->v.vl;

        res = PQexec( con, s->v.s->c_str());
        if (PQresultStatus(res) == PGRES_FATAL_ERROR)
        {
            fprintf(stderr, "PQexec <%s> failed: %s Status: %d\n",s->v.s->c_str(),PQerrorMessage(con),PQresultStatus(res));fflush(stderr);
            PQclear(res);
            return 0;
        }
    }else{
    }
    a->res = res;

    delete v;
    ec.sd->back() = ec.ef.new_hVal(hVal::SQL,a,0);

    return 0;
}

static int stnm_cnt = 0;
#define exec_par_max 50

static hVal* sexec2_f(ef_Content &ec,SqlAdd *p,hVal *a)
{

    if( p->res) {
        PQclear(p->res);
        p->res = NULL;
    }
    p->pos = 0;

    PGconn *c = (PGconn *)p->con->v.vl;

    HHDeqV_T *q = NULL;
    if( a) {
        q = a->v.q;
        if( q->size() == 0) q = NULL;
    }

    PGresult   * nres = 0;
    if( !q ) {
//        fprintf(stderr, "exec 3.5\n");fflush(stderr);
        nres = PQexec( c, p->sql->v.s->c_str());
//        fprintf(stderr, "exec 4\n");fflush(stderr);
        if (PQresultStatus(nres) == PGRES_FATAL_ERROR)
        {
            fprintf(stderr, "PQexec <%s> failed: %s Status: %d\n",p->sql->v.s->c_str(),PQerrorMessage(c),PQresultStatus(nres));fflush(stderr);
            PQclear(nres);

            goto end;
        }
        p->res = nres;
    }else{

//        Local<String::Utf8Value> pr[exec_par_max];// = String::New((const char*)fb);

        std::string ts[exec_par_max];

        char *pr_v[exec_par_max];
        int   pr_l[exec_par_max];
        Oid   pr_t[exec_par_max];

        int ln = 0;
        int i = 0;
        HHDeqV_TI it = q->begin();
        while( it != q->end() && i < exec_par_max){
//            delete *it;
            hVal *e = *it;
            if( e->b.tp == hVal::STRING || e->b.tp == hVal::CSTRING){

                pr_v[i] = (char *)e->v.s->c_str();
                pr_l[i] = e->v.s->size();
                pr_t[i] = NULL;

            }else if( e->b.tp == hVal::DOUBLE || e->b.tp == hVal::CDOUBLE){
                double dd = e->v.d;
                double ddd = dd - (long long)dd;
//                fprintf(stderr, "!!! ddd %f\n",ddd);fflush(stderr);

                if( ddd != 0){
                    ts[i] = std::to_string((long double)dd);
                    pr_t[i] = QFLOAT8OID;
                }else{
                    ts[i] = std::to_string((long long)dd);
                    pr_t[i] = QFLOAT8OID;
                }

                pr_v[i] = (char *)ts[i].c_str();
                pr_l[i] = ts[i].size();
//                pr_t[i] = QINT8OID;

            }else if( e->b.tp == hVal::TRUE){
                ts[i] = "True";

                pr_v[i] = (char *)ts[i].c_str();
                pr_l[i] = ts[i].size();
                pr_t[i] = QBOOLOID;

            }else if( e->b.tp == hVal::FALSE){
                ts[i] = "False";

                pr_v[i] = (char *)ts[i].c_str();
                pr_l[i] = ts[i].size();
                pr_t[i] = QBOOLOID;

            }else if( e->b.tp == hVal::BUF){

                pr_v[i] = (char *)e->v.s->c_str();
                pr_l[i] = e->v.s->size();
                pr_t[i] = QBYTEAOID;
            }

            ++it;++i;
        }
        ln = i;

        if( p->stnm.size() == 0){//prepare
            p->stnm = "ef_stmt_" + (stnm_cnt++);
            PGresult *r_p = PQprepare(c,
                                p->stnm.c_str(),
                                p->sql->v.s->c_str(),
                                ln,
                                pr_t);
            if (PQresultStatus(r_p) == PGRES_FATAL_ERROR) {
                fprintf(stderr, "PQexec <%s> failed: %s Status: %d\n",p->sql->v.s->c_str(),PQerrorMessage(c),PQresultStatus(r_p));fflush(stderr);
                p->stnm.clear();
            }
            PQclear(r_p);
        }

        if( p->stnm.size() != 0){ //working

            PGresult *res = PQexecPrepared(c,
                                     p->stnm.c_str(),
                                     ln,
                                     pr_v,
                                     pr_l,
                                     NULL,
                                     NULL);
//fprintf(stderr, "13-\n");fflush(stderr);

            if( PQresultStatus(res) == PGRES_FATAL_ERROR) {
                fprintf(stderr, "PQexec <%s> failed: %s Status: %d\n",p->sql->v.s->c_str(),PQerrorMessage(c),PQresultStatus(res));fflush(stderr);
                PQclear(res);
            }else{
                p->res = res;
            }

        }


/*
        for(int i=0; i < args.Length(); ++i){
            if( args[i]->IsObject() ) {

                Local<Object> obj = args[i]->ToObject();
                Handle<External> field = Handle<External>::Cast(obj->GetInternalField(0));
                void* o = field->Value();
                hBuf * bu = (hBuf * )o;
                int32_t id = obj->GetInternalField(1)->Int32Value();
                if( id != Buf_ID) return handle_scope.Close(Boolean::New(false));

                pr_v[i] = bu->p;
                pr_l[i] = bu->sz;
                pr_t[i] = QBYTEAOID;

                continue;
            }


            String::Utf8Value t(args[i]);

            char *ptr = (char *)malloc(t.length() + 5);
            strcpy(ptr,*t);

            pr_v[i] = ptr;
            pr_l[i] = t.length();

            if( args[i]->IsBoolean() ) pr_t[i] = QBOOLOID;
            if( args[i]->IsNumber() ) pr_t[i] = QINT8OID;//QFLOAT8OID;
            if( args[i]->IsString() ) pr_t[i] = NULL;

        }
/*
//fprintf(stderr, "10-\n");fflush(stderr);

        if( p->stnm[0] == 0){//prepare
            sprintf(p->stnm,"ej_stmt_%d",stnm_cnt++);
//            fprintf(stderr, "exec PREPARE %s\n",p->stnm);fflush(stderr);

            PGresult *r_p = PQprepare(pgdb[p->con].con,
                                p->stnm,
                                p->sql,
                                args.Length(),
                                pr_t);
            if (PQresultStatus(r_p) == PGRES_FATAL_ERROR) {
                fprintf(stderr, "PQexec <%s> failed: %s %d\n",p->sql,PQerrorMessage(pgdb[p->con].con),PQresultStatus(r_p));fflush(stderr);
                p->stnm[0] = 0;
            }
            PQclear(r_p);
        }
//fprintf(stderr, "11-\n");fflush(stderr);

        //-- working

        if( p->stnm[0] != 0){ //working
//            fprintf(stderr, "exec working 1\n");fflush(stderr);
//fprintf(stderr, "12- %s\n",p->stnm);fflush(stderr);

            PGresult *res = PQexecPrepared(pgdb[p->con].con,
                                     p->stnm,
                                     args.Length(),
                                     pr_v,
                                     pr_l,
                                     NULL,
                                     NULL);
//fprintf(stderr, "13-\n");fflush(stderr);

            if( PQresultStatus(res) == PGRES_FATAL_ERROR) {
                fprintf(stderr, "PQexec <%s> failed: %s %d\n",p->sql,PQerrorMessage(pgdb[p->con].con),PQresultStatus(res));fflush(stderr);
                PQclear(res);
            }else{
                p->res = res;
            }

        }

//        fprintf(stderr, "14-\n");fflush(stderr);
        // clear
        for(int i=0; i < args.Length(); ++i){
            if( args[i]->IsObject() ) continue;

            free(pr_v[i]);
        }
//        fprintf(stderr, "15-\n");fflush(stderr);
*/
    }

end:

    if( a){
        delete a;
        ec.sd->pop_back();
    }

    return 0;
}

static hVal* sexec_f(ef_Content &ec)
{

    if( ec.sd->size() == 0){
        std::cerr << "ERROR: 'EX' zero size stack!\n"; std::cerr.flush();
        return 0;
    }

    register hVal *v = ec.sd->back();

    if( v->b.tp == hVal::SQL) {
        sexec2_f(ec,(SqlAdd *)v->v.vl,NULL);
        return 0;
    }else if( v->b.tp == hVal::DEQ || v->b.tp == hVal::CDEQ){
        if( ec.sd->size() < 2){
            std::cerr << "ERROR: 'EX' < 2 size stack!\n"; std::cerr.flush();
            return 0;
        }
        hVal *d = ec.sd->at(ec.sd->size() - 2);
        if( d->b.tp == hVal::SQL){
            sexec2_f(ec,(SqlAdd *)d->v.vl,v);
            return 0;
        }
        //...
    }

    return 0 ;
}

#define MAX_RUN_PAR 500

static hVal* run_f(ef_Content &ec)
{
//    uv_process_options_t options = {0};
    char* args[MAX_RUN_PAR] = {0};
//    uv_process_t child_req = {0};

//    memset(&options,0,sizeof(options));
    memset(&args,0,sizeof(args));
//    memset(&child_req,0,sizeof(child_req));

    if( ec.sd->size() == 0){
        std::cerr << "ERROR: 'RUN' zero size stack!\n"; std::cerr.flush();
        return 0;
    }

    register hVal *v = ec.sd->back();

    if( v->b.tp != hVal::DEQ && v->b.tp != hVal::CDEQ){
        std::cerr << "ERROR: 'RUN' stack NOT DEQ!\n"; std::cerr.flush();
        return 0;
    }

    HHDeqV_T *q = NULL;
    q = v->v.q;
    if( q->size() == 0) q = NULL;
    if( !q){
        std::cerr << "ERROR: 'RUN' zero DEQ!\n"; std::cerr.flush();
        return 0;
    }

    std::string dir;
    int ln = 0;
    int i = 0;
    HHDeqV_TI it = q->begin();
    for(; it != q->end() && i < MAX_RUN_PAR; ++it){
        hVal *e = *it;
        if( e->b.tp == hVal::STRING || e->b.tp == hVal::CSTRING){
//            std::string *s = e->v.s;
            char *s = (char *)e->v.s->c_str();
            int p;
            if( !strncmp(s,"-DIR:",5)){
                dir = s+5;

                std::cerr << s << " dir:" << dir <<'\n'; std::cerr.flush();
                continue;
            }else{
                args[i] = s;
                ++i;
            }
        }
    }
    ln = i;

    __pid_t npid = fork();

    if( !npid) {
//                    fprintf(stderr,"\nIn new execv PROCESS! %s %s\n",prs,par);fflush(stderr);
        ::close(0);
        ::close(1);
        ::close(2);

        if( dir.size() > 0 ) {
            chdir(dir.c_str());
        }

        setsid();
        execvp(args[0],args);
    }

    delete v;
    ec.sd->back() = ec.ef.new_hVal(hVal::DOUBLE,0,(double)npid);

/*

    options.exit_cb = NULL;
    options.file = args[0];
    options.args = args;
    options.flags = UV_PROCESS_DETACHED;

    if( dir.size() > 0) options.cwd = dir.c_str();


    delete v;    ec.sd->back() = vN;    return 0;

    int r;
    if ((r = uv_spawn(uv_default_loop(), &child_req, &options))) {
        fprintf(stderr, "%s\n", uv_strerror(r));

        delete v;
        ec.sd->back() = vN;

        return 0;
    }
    fprintf(stderr, "Launched sleep with PID %d\n", child_req.pid);

    delete v;
    ec.sd->back() = ec.ef.new_hVal(hVal::DOUBLE,0,(double)child_req.pid);

    fprintf(stderr, "Launched@ sleep with PID %d\n", child_req.pid);
//    uv_unref((uv_handle_t*) &child_req);
*/
    return 0 ;
}

static hVal* next_f(ef_Content &ec)
{
    register hVal *v = NULL;

    if( ec.sd->size() == 0){
        std::cerr << "ERROR: 'NX' zero size stack!\n"; std::cerr.flush();
//        ec.sd->push_back(vN);
//        return 0 ;
        goto end;
    }

    v = ec.sd->back();

    if( v->b.tp == hVal::SQL ) {

        SqlAdd *p = (SqlAdd *)v->v.vl;

        if( !p->res) {
            ec.sd->push_back(vN);
            return 0 ;
        }

        int nFields = PQnfields(p->res);
        int nTuples = PQntuples(p->res);

        int pos = p->pos;
        if(pos >= nTuples) {
            ec.sd->push_back(vN);
            return 0 ;
        }

        //--

    //    Local<Object> obj = Object::New();

        hVal *r = ec.ef.new_hVal(hVal::DEQ,0,0);

        for(int i=0; i < nFields; ++i) {

            char    *fnb = PQfname(p->res, i);
            int len = PQgetlength(p->res,pos,i);
            char * v = PQgetvalue(p->res,pos,i);
            Oid type = PQftype(p->res, i);
            int format = PQfformat(p->res, i);

//            std::cerr << "v: " << v << "\n"; std::cerr.flush();

            switch (type) {
            case QBOOLOID:
    //            obj->Set(v8::String::NewSymbol(fnb),Boolean::New((bool)(v[0] == 't')));
                (v[0] == 't') ? r->push(vT) : r->push(vF);

                break;
            case QINT8OID:
            case QINT2OID:
            case QINT4OID:
            case QNUMERICOID:
            case QFLOAT4OID:
            case QFLOAT8OID:
                r->push(ec.ef.new_hVal(hVal::DOUBLE,0,atof(v)));
                break;
    /*
            case QABSTIMEOID:
            case QRELTIMEOID:
            case QDATEOID:
    //            type = QVariant::Date;
                break;
            case QTIMEOID:
            case QTIMETZOID:
    //            type = QVariant::Time;
                break;
            case QTIMESTAMPOID:
            case QTIMESTAMPTZOID:
    //            type = QVariant::DateTime;
                break;
    */
            case QBYTEAOID:
                {
                size_t to_length;
                char * vv = (char * )PQunescapeBytea((const unsigned char *)v, &to_length);

                std::string rr(vv,to_length);
                r->push(ec.ef.new_hVal(hVal::BUF,ec.ef.new_str(rr),0));

                PQfreemem(vv);
                }

                break;
            default:
                r->push(ec.ef.new_hVal(hVal::STRING,ec.ef.new_str(v),0));
                break;
            }
        }

        ++pos;
        p->pos = pos;

        ec.sd->push_back(r);
        return 0;
    }

end:
    ec.sd->push_back(vN);
    return 0 ;
}

static hVal* rows_f(ef_Content &ec)
{

    if( ec.sd->size() == 0){
        std::cerr << "ERROR: 'RW' zero size stack!\n"; std::cerr.flush();
        return 0 ;
    }

    register hVal *v = ec.sd->back();

    if( v->b.tp != hVal::SQL ) {
        std::cerr << "ERROR: 'RW' stack is NOT SQL!\n"; std::cerr.flush();
        return 0 ;
    }

    SqlAdd *p = (SqlAdd *)v->v.vl;

    ec.sd->push_back(ec.ef.new_hVal(hVal::DOUBLE,0,PQntuples(p->res)));
    return 0;
}

static hVal* prep_f(ef_Content &ec)
{

    if( ec.sd->size() < 2){
        std::cerr << "ERROR: 'PR' < 2 size stack!\n"; std::cerr.flush();
        return 0 ;
    }

    hVal *z = ec.sd->back();
    hVal *v = ec.sd->at(ec.sd->size() - 2);

    if( v->b.tp != hVal::SQL ) {
        std::cerr << "ERROR: 'PR' stack - 1 is NOT SQL!\n"; std::cerr.flush();
        return 0 ;
    }
    if( z->b.tp != hVal::STRING &&  z->b.tp != hVal::CSTRING) {
        std::cerr << "ERROR: 'PR' stack is NOT STRING!\n"; std::cerr.flush();
        return 0 ;
    }

    SqlAdd *p = (SqlAdd *)v->v.vl;
    PGconn *c = (PGconn *)p->con->v.vl;

//    ec.sd->push_back(ec.ef.new_hVal(hVal::DOUBLE,0,PQntuples(p->res)));
    if( p->res) {
        PQclear(p->res);
        p->res = 0;
    }
    if( p->sql) {
        delete p->sql;
        p->sql = 0;
    }

    if( p->stnm.size() != 0) {
        char b[300];
        sprintf(b,"DEALLOCATE %s",p->stnm.c_str());
        PGresult *res = PQexec( c, b);
        if( PQresultStatus(res) == PGRES_FATAL_ERROR) {
            fprintf(stderr, "PQexec <%s> failed: %s Status: %d\n",b,PQerrorMessage(c),PQresultStatus(res));fflush(stderr);
        }
        PQclear(res);
    }

    p->stnm.clear();// = "";
    p->pos = 0;

    p->sql = z;

    int flg_exec = 1;
//    if( strchr(*sqls,'$') != NULL || strchr(*sqls,':') != NULL ) flg_exec = 0;
    p->flg_exec = flg_exec;

    ec.sd->pop_back();

    return 0;
}

static hVal* prep2_f(ef_Content &ec,hVal *v,hVal *z)
{
/*
    if( v->b.tp != hVal::SQL ) {
        std::cerr << "ERROR: 'PR' stack - 1 is NOT SQL!\n"; std::cerr.flush();
        return 0 ;
    }
    if( z->b.tp != hVal::STRING &&  z->b.tp != hVal::CSTRING) {
        std::cerr << "ERROR: 'PR' stack is NOT STRING!\n"; std::cerr.flush();
        return 0 ;
    }
*/
    SqlAdd *p = (SqlAdd *)v->v.vl;
    PGconn *c = (PGconn *)p->con->v.vl;

//    ec.sd->push_back(ec.ef.new_hVal(hVal::DOUBLE,0,PQntuples(p->res)));
    if( p->res) {
        PQclear(p->res);
        p->res = 0;
    }
    if( p->sql) {
        delete p->sql;
        p->sql = 0;
    }

    if( p->stnm.size() != 0) {
        char b[300];
        sprintf(b,"DEALLOCATE %s",p->stnm.c_str());
        PGresult *res = PQexec( c, b);
        if( PQresultStatus(res) == PGRES_FATAL_ERROR) {
            fprintf(stderr, "PQexec <%s> failed: %s Status: %d\n",b,PQerrorMessage(c),PQresultStatus(res));fflush(stderr);
        }
        PQclear(res);
    }

    p->stnm.clear();// = "";
    p->pos = 0;

    p->sql = z;

    int flg_exec = 1;
//    if( strchr(*sqls,'$') != NULL || strchr(*sqls,':') != NULL ) flg_exec = 0;
    p->flg_exec = flg_exec;

    ec.sd->pop_back();

    return 0;
}


static hVal* serr_f(ef_Content &ec)
{

    if( ec.sd->size() == 0){
        std::cerr << "ERROR: 'ER' zero size stack!\n"; std::cerr.flush();
        return 0 ;
    }

    register hVal *v = ec.sd->back();

    if( v->b.tp != hVal::SQL ) {
        std::cerr << "ERROR: 'ER' stack is NOT SQL!\n"; std::cerr.flush();
        return 0 ;
    }

    SqlAdd *p = (SqlAdd *)v->v.vl;
    PGconn *c = (PGconn *)p->con->v.vl;

    ec.sd->push_back(ec.ef.new_hVal(hVal::STRING,ec.ef.new_str(PQerrorMessage(c)),0));
    return 0;
}

static hVal* sL_f(ef_Content &ec)
{

    if( ec.sd->size() == 0){
        std::cerr << "ERROR: 'ER' zero size stack!\n"; std::cerr.flush();
        return 0 ;
    }

    hVal *v = ec.sd->back();
    hVal *r = vN;

    if( v->b.tp != hVal::DOUBLE &&  v->b.tp != hVal::CDOUBLE) {
        std::cerr << "ERROR: 'sL' stack is NOT DOUBLE!\n"; std::cerr.flush();
        return 0 ;
    }

    int socknum = (int)v->v.d;

    int sock = -1;
    int ret = BindPassiveSocket(socknum, &sock);
    if( ret >= 0 ) {
        r = ec.ef.new_hVal(hVal::DOUBLE,0,sock);
    }

    delete v;
    ec.sd->back() = r;

    return 0;
}

static hVal* sE_f(ef_Content &ec)
{

    if( ec.sd->size() == 0){
        std::cerr << "ERROR: 'sE' zero size stack!\n"; std::cerr.flush();
        return 0 ;
    }

    hVal *v = ec.sd->back();

    if( v->b.tp != hVal::DOUBLE &&  v->b.tp != hVal::CDOUBLE) {
        std::cerr << "ERROR: 'sE' stack is NOT DOUBLE!\n"; std::cerr.flush();
        return 0 ;
    }

    int socknum = (int)v->v.d;

    if(v >= 0){
        hh_closesocket(socknum);
        v->v.d = -1;
    }

    ec.sd->pop_back();
    delete v;

    return 0;
}

static hVal* sS_f(ef_Content &ec)
{
/*
    if( ec.sd->size() == 0){
        std::cerr << "ERROR: 'ER' zero size stack!\n"; std::cerr.flush();
        return 0 ;
    }
*/
    hVal *v = ec.sd->back();
/*
    if( v->b.tp != hVal::DOUBLE &&  v->b.tp != hVal::CDOUBLE) {
        std::cerr << "ERROR: 'sE' stack is NOT DOUBLE!\n"; std::cerr.flush();
        return 0 ;
    }
*/
    int socknum = (int)v->v.d;

    if( socknum < 0){
        ec.sd->push_back(vF);
        return 0;
    }

    if(c_scselect(socknum) > 0){
        ec.sd->push_back(vT);

    }else{
        ec.sd->push_back(vF);
    }

/*
    if( r > 0 ) {
        if( !listen_flg) return handle_scope.Close(Boolean::New(true)); // connect

        // listen

        int					slave;
        struct sockaddr_in	client;
        socklen_t			clilen;

        clilen=sizeof(client);
        slave=::accept(sock,(struct sockaddr *)&client,&clilen);
        if( slave < 0 ) return handle_scope.Close(Boolean::New(false));

*/

    return 0;
}

static hVal* sR_f(ef_Content &ec)
{
/*
    if( ec.sd->size() == 0){
        std::cerr << "ERROR: 'ER' zero size stack!\n"; std::cerr.flush();
        return 0 ;
    }
*/
    hVal *v = ec.sd->back();
/*
    if( v->b.tp != hVal::DOUBLE &&  v->b.tp != hVal::CDOUBLE) {
        std::cerr << "ERROR: 'sE' stack is NOT DOUBLE!\n"; std::cerr.flush();
        return 0 ;
    }
*/
    int sc = (int)v->v.d;

    if( sc < 0){
        ec.sd->push_back(vN);
        return 0;
    }

    int		ret;
    char	*in;
    hVal *r = vN;

    if( (ret = recv_piece(sc,&in)) < 0 ) {
        hh_closesocket(sc);
        v->v.d = -1;
    }else{
        r = ec.ef.new_hVal(hVal::STRING,ec.ef.new_str(in,ret),0);

        free(in);
    }

    ec.sd->push_back(r);

    return 0;
}

static hVal* sRR_f(ef_Content &ec)
{
/*
    if( ec.sd->size() == 0){
        std::cerr << "ERROR: 'ER' zero size stack!\n"; std::cerr.flush();
        return 0 ;
    }
*/
    hVal *v = ec.sd->back();
    hVal *r = vN;

    int tot = 0;
    char *blk = NULL;
    char *p = NULL;//(char*)blk;
    char *bp = NULL;
    int bd_sz = 0;
    int hd_sz = 0;

    time_t ltm = time(0);


/*
    if( v->b.tp != hVal::DOUBLE &&  v->b.tp != hVal::CDOUBLE) {
        std::cerr << "ERROR: 'sE' stack is NOT DOUBLE!\n"; std::cerr.flush();
        return 0 ;
    }
*/
    int sc = (int)v->v.d;

    if( sc < 0){
        ec.sd->push_back(vN);
        return 0;
    }

    //--

    int tm_max = 30;
    int szm = 100000;

/*
    if( args.Length() > 0) {
        tm_max = args[0]->Int32Value();
    }
*/
    blk = (char *) malloc(szm + 10);
    if(!blk) {
//        hh_closesocket(sc);
        goto end;
    }

    p = (char*)blk;
    bp = blk;

//    time_t ctm = time(0);

    do {
        if( !c_scselect(sc)) {
            if( ( time(0) - ltm) > tm_max) {
                break;
            }

            MSLEEP(100);
            continue;
        }

        ltm = time(0);
        int ret = recv (sc,p,szm-tot,0);

        if(ret == 0 ) { //closed
          break;
        }
        if(ret == -1 ) {
          if (errno!=EINTR){
              break;
          } else {
            continue;
          }
        }
/*
        if( ret > 0) {
            for(int i=0; i < ret; ++i){
                fprintf(stderr,"%c",p[i]);fflush(stderr);
            }
        }
*/
        p += ret;
        tot += ret;

        if( (szm-tot) < (szm/10)) {
            int new_szm = szm *2;
            blk = (char*)realloc(blk,new_szm);
            if(!blk) {
//                hh_closesocket(sc);
//                return handle_scope.Close(Boolean::New(false));
                goto end;
            }
            p = &blk[tot];

            szm = new_szm;
        }

        blk[tot] = 0;

        // проверки на конец хидера ...

        if( bd_sz){
            if( tot >= bd_sz) {
                break;
            }

            continue;
        }

        char *pp = 0;
        if( pp = strstr(blk,"\015\012\015\012")) {
            *pp++ = 0;
            *pp++ = 0;
            *pp++ = 0;
            *pp++ = 0;
            bp = pp;

//            fprintf(stderr,"**** 4 %d\n",pp);fflush(stderr);

        }else if( pp = strstr(blk,"\012\012")) {
            *pp++ = 0;
            *pp++ = 0;
            bp = pp;

//            fprintf(stderr,"**** 2\n");fflush(stderr);
        }

        if( pp) {
            hd_sz = bd_sz = pp - blk;

//            fprintf(stderr,"Опаньки %d\n",hd_sz);fflush(stderr);

            char *szp = strstr(blk,"Content-Length:");
            if( szp) {
                szp = strchr(szp,':');
                ++szp;
                bd_sz += atoi(szp);

            } else {
                break;
            }
        }

        if( bd_sz){
            if( tot >= bd_sz) {
                break;
            }

            continue;
        }

    } while(1);

    blk[tot] = 0;

    r = ec.ef.new_hVal(hVal::DEQ,0,0);
    r->push(ec.ef.new_hVal(hVal::STRING,ec.ef.new_str(blk),0));
    r->push(ec.ef.new_hVal(hVal::STRING,ec.ef.new_str(bp,tot-(bp-blk)),0));
    free(blk);

end:

    ec.sd->push_back(r);

    return 0;
}


static hVal* sW_f(ef_Content &ec)
{
    hVal *r = vT;
    int rr=0;

    if( ec.sd->size() < 2){
        std::cerr << "ERROR: 'sW' zero size stack!\n"; std::cerr.flush();
        return 0 ;
    }

    hVal *vv = ec.sd->back();
    hVal *s = ec.sd->at(ec.sd->size() - 2);

    std::string *w;
    std::string o;

    if( vv->b.tp == hVal::STRING || vv->b.tp == hVal::CSTRING || vv->b.tp == hVal::BUF){
        w = vv->v.s;
    }else{
        lg_hVal(ec,vv,0,o,0);
        w = &o;
    }

    if( s->b.tp != hVal::DOUBLE &&  s->b.tp != hVal::CDOUBLE) {
        std::cerr << "ERROR: 'sW' stack-2 is NOT DOUBLE!\n"; std::cerr.flush();
        return 0 ;
    }

    int sc = (int)s->v.d;

    if( sc < 0){
        goto end;
    }

    rr = send_piece( sc,(char*) w->c_str(), w->size());

    if( rr >= 0) {
        r = vT;
    }else{
        r = vF;
    }

end:

    delete vv;
    ec.sd->back() = r;

    return 0;
}

static hVal* sWR_f(ef_Content &ec)
{
    hVal *r = vT;
    int rr=0;

    if( ec.sd->size() < 2){
        std::cerr << "ERROR: 'sWR' zero size stack!\n"; std::cerr.flush();
        return 0 ;
    }

    hVal *vq = ec.sd->back();

    if( vq->b.tp != hVal::DEQ &&  vq->b.tp != hVal::CDEQ) {
        std::cerr << "ERROR: 'sWR' stack is NOT DEQ!\n"; std::cerr.flush();
        return 0 ;
    }

    HHDeqV_T *q = vq->v.q;
    if( q->size() < 2) {
        std::cerr << "ERROR: 'sWR' DEQ size < 2!\n"; std::cerr.flush();
        return 0 ;
    }

    hVal *hh = q->at(0);
    hVal *vv = q->at(1);

    hVal *s = ec.sd->at(ec.sd->size() - 2);

    std::string *w;
    std::string o;
    std::string ss;

    if( vv->b.tp == hVal::STRING || vv->b.tp == hVal::CSTRING || vv->b.tp == hVal::BUF){
        w = vv->v.s;
    }else{
        lg_hVal(ec,vv,0,o,0);
        w = &o;
    }

    ss = *hh->v.s + "\nContent-Length: " + std::to_string((long long)w->size()) + "\n\n" + *w;

    if( s->b.tp != hVal::DOUBLE &&  s->b.tp != hVal::CDOUBLE) {
        std::cerr << "ERROR: 'sW' stack-2 is NOT DOUBLE!\n"; std::cerr.flush();
        return 0 ;
    }

    int sc = (int)s->v.d;

    if( sc < 0){
        goto end;
    }

    rr = send_buf(sc,(char*)ss.c_str(),ss.size());

    if( rr >= 0) {
        r = vT;
    }else{
        r = vF;
    }

end:

    delete vq;
    ec.sd->back() = r;

    return 0;
}

//#ifndef TRUE
//#define TRUE 1
//#define FALSE 0
//#endif

extern "C" {
int mkaddr(void *addr,
           int *addrlen,
           char *str_addr,
           char *protocol);
}

static hVal* udp_f(ef_Content &ec)
{
    hVal *r = vN;
    int rr=0;
    struct sockaddr_in adr_srvr;/* AF_INET */
    int len_srvr;               /* length */
    struct sockaddr_in adr_bc;  /* AF_INET */
    struct sockaddr_in adr_bc_rc;  /* AF_INET */
    int len_bc;                 /* length */
    static int so_broadcast = TRUE;
    int z;      /* Status return code */
    int s;      /* Socket */
    static int so_reuseaddr = TRUE;
    int sc;// = socket(AF_INET,SOCK_DGRAM,0);
    UDP_T * u = NULL;

    if( ec.sd->size() < 1){
        std::cerr << "ERROR: 'UDP' zero size stack!\n"; std::cerr.flush();
       return 0;
    }

    hVal *vq = ec.sd->back();

    if( vq->b.tp != hVal::DEQ &&  vq->b.tp != hVal::CDEQ) {
        std::cerr << "ERROR: 'UDP' stack is NOT DEQ!\n"; std::cerr.flush();
        return 0;
    }

    HHDeqV_T *q = vq->v.q;
    if( q->size() < 2) {
        std::cerr << "ERROR: 'UDP' DEQ size < 2!\n"; std::cerr.flush();
        return 0;
    }

    hVal *vs = q->at(0);
    hVal *vb = q->at(1);

    if( (vs->b.tp != hVal::STRING &&  vs->b.tp != hVal::CSTRING)  && (vb->b.tp != hVal::STRING &&  vb->b.tp != hVal::CSTRING) ) {
        std::cerr << "ERROR: 'UDP' DEQ[0] or DEQ[1] not STRING!\n"; std::cerr.flush();
        return 0;
    }

    len_srvr = sizeof adr_srvr;

    z = mkaddr(
            &adr_srvr,  /* Returned address */
            &len_srvr,  /* Returned length */
            (char*)vs->v.s->c_str(),    /* Input string addr */
            (char*)"udp");     /* UDP protocol */

    if ( z == -1 ){
      goto end;
    }

    len_bc = sizeof adr_bc;

    z = mkaddr(
        &adr_bc, /* Returned address */
        &len_bc, /* Returned length */
        (char*)vb->v.s->c_str(), /* Input string addr */
        "udp"); /* UDP protocol */



    if ( z == -1 ){
        goto end;
    }

    z = mkaddr(
        &adr_bc_rc, /* Returned address */
        &len_bc, /* Returned length */
        (char*)vb->v.s->c_str(), /* Input string addr */
        "udp"); /* UDP protocol */

    s = socket(AF_INET,SOCK_DGRAM,0);
    if ( s == -1 ){
        std::cerr << "ERROR: 'UDP' SERVER socket not created!\n"; std::cerr.flush();
    }

    z = setsockopt(s,
                   SOL_SOCKET,
                   SO_BROADCAST,
                   &so_broadcast,
                   sizeof so_broadcast);

    if ( z == -1 ){
        std::cerr << "ERROR: 'UDP' setsockopt!\n"; std::cerr.flush();
    }

    z = bind(s,(struct sockaddr *)&adr_srvr, len_srvr);

    if ( z == -1 ){
        std::cerr << "ERROR: 'UDP' socket not binded!\n"; std::cerr.flush();
        goto end;
    }

    //--------------------------

    sc = socket(AF_INET,SOCK_DGRAM,0);

    if ( sc == -1 ){
        std::cerr << "ERROR: 'UDP' CLIENT socket not created!\n"; std::cerr.flush();
        hh_closesocket(s);
        goto end;
    }
    z = setsockopt(sc,
                   SOL_SOCKET,
                   SO_REUSEADDR,
                   &so_reuseaddr,
                   sizeof so_reuseaddr);
    if ( z == -1 ){
        std::cerr << "ERROR: 'UDP' client setsockopt!\n"; std::cerr.flush();
        hh_closesocket(s);
        hh_closesocket(sc);
        goto end;
    }
    z = bind(sc,
            (struct sockaddr *)&adr_bc,
            len_bc);

    if ( z == -1 ){
        std::cerr << "ERROR: 'UDP' client bind!\n"; std::cerr.flush();
        hh_closesocket(s);
        hh_closesocket(sc);
        goto end;
    }

    u = new UDP_T();
    u->s = s;
    u->sc = sc;
    memcpy(&u->adr_bc,&adr_bc,len_bc);
    memcpy(&u->adr_bc_rc,&adr_bc_rc,len_bc);

    r = ec.ef.new_hVal(hVal::UDP,u,0);

end:

    delete vq;
    ec.sd->back() = r;

    return 0;
}

static hVal* sA_f(ef_Content &ec)
{
/*
    if( ec.sd->size() == 0){
        std::cerr << "ERROR: 'ER' zero size stack!\n"; std::cerr.flush();
        return 0 ;
    }
*/
    hVal *v = ec.sd->back();
/*
    if( v->b.tp != hVal::DOUBLE &&  v->b.tp != hVal::CDOUBLE) {
        std::cerr << "ERROR: 'sE' stack is NOT DOUBLE!\n"; std::cerr.flush();
        return 0 ;
    }
*/
    int socknum = (int)v->v.d;

    if( socknum < 0){
        ec.sd->push_back(vF);
        return 0;
    }

    hVal *r = vN;

    if( c_scselect(socknum) > 0){
        int					slave;
        struct sockaddr_in	client;
        socklen_t			clilen;

        clilen=sizeof(client);
        slave = ::accept(socknum,(struct sockaddr *)&client,&clilen);

        if( slave >= 0) r = ec.ef.new_hVal(hVal::DOUBLE,0,slave);
    }

    ec.sd->push_back(r);

    return 0;
}

static hVal* sC_f(ef_Content &ec)
{

    if( ec.sd->size() == 0){
        std::cerr << "ERROR: 'sC' zero size stack!\n"; std::cerr.flush();
        return 0 ;
    }

    hVal *v = ec.sd->back();
    hVal *r = vN;

    if( v->b.tp != hVal::STRING &&  v->b.tp != hVal::CSTRING) {
        std::cerr << "ERROR: 'sC' stack is NOT DOUBLE!\n"; std::cerr.flush();
        return 0 ;
    }

    int sock = hh_connect ((char*)v->v.s->c_str(),0);

    if( sock >= 0 ) {
        r = ec.ef.new_hVal(hVal::DOUBLE,0,sock);
    }

    delete v;
    ec.sd->back() = r;

    return 0;
}

static hVal* inc_f(ef_Content &ec)
{
    HHDeqV_T* gq = ec.st->back();

//    std::cerr << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"; std::cerr.flush();

    if( gq->size() == 0){
        std::cerr << "ERROR: 'INC' zero size stack!\n"; std::cerr.flush();
        return 0 ;
    }

    hVal *v = gq->back();

    if( v->b.tp == hVal::STRING ||  v->b.tp == hVal::CSTRING) {

        goto end;

    }else if( v->b.tp == hVal::DEQ ||  v->b.tp == hVal::CDEQ){
        HHDeqV_T* q = v->v.q;//ec.svq->back();
        if( q->size() < 2) goto end;

        hVal *v0 = q->at(0);
        hVal *v1 = q->at(1);

        if( (v0->b.tp != hVal::STRING && v0->b.tp != hVal::CSTRING) && (v1->b.tp != hVal::STRING != v1->b.tp != hVal::CSTRING)){
            goto end;
        }

        char * inf = get_file_text_utf8((char*)v0->v.s->c_str(),0);
        if( !inf) goto end;

        hVal * r = ef_compile(ec,ef,inf,(char*)v1->v.s->c_str());
        if( r->b.tp == hVal::STRING){
            std::cerr << *r->v.s;
            delete r;
            goto end;
        }
    }

end:

    delete v;

    gq->pop_back();

    return 0;
}

static hVal* llib_f(ef_Content &ec)
{
    HHDeqV_T* gq = ec.st->back();

//    std::cerr << "!!!!!!!!!!!!!!!!!!!!---------!!!!!!!!!!!\n"; std::cerr.flush();

    if( gq->size() == 0){
        std::cerr << "ERROR: 'LD' zero size stack!\n"; std::cerr.flush();
        return 0 ;
    }

    hVal *v = gq->back();

    if( v->b.tp == hVal::STRING ||  v->b.tp == hVal::CSTRING) {

        uv_lib_t lib;// = (uv_lib_t*) malloc(sizeof(uv_lib_t));
        if( uv_dlopen(v->v.s->c_str(), &lib)) {
            fprintf(stderr, "Error: %s\n", uv_dlerror(&lib));fflush(stderr);
            goto end;
        }

        typedef void (*MyPrototype)(ef_Content *ec);

        MyPrototype myFunction;
        if (uv_dlsym(&lib, "init", (void **) &myFunction)) {
            fprintf(stderr, "dlsym error: %s\n", uv_dlerror(&lib));fflush(stderr);
            goto end;
        }

//        ef_Content *ecp = &::ec;
        ef_Content *ecp = &ec;

        ecp->mc = 123;

        myFunction(ecp);

        goto end;

    }else if( v->b.tp == hVal::DEQ ||  v->b.tp == hVal::CDEQ){
        goto end;
/*
        HHDeqV_T* q = v->v.q;//ec.svq->back();
        if( q->size() < 2) goto end;

        hVal *v0 = q->at(0);
        hVal *v1 = q->at(1);

        if( (v0->b.tp != hVal::STRING && v0->b.tp != hVal::CSTRING) && (v1->b.tp != hVal::STRING != v1->b.tp != hVal::CSTRING)){
            goto end;
        }

        char * inf = get_file_text_utf8((char*)v0->v.s->c_str(),0);
        if( !inf) goto end;

        hVal * r = ef_compile(ec,ef,inf,(char*)v1->v.s->c_str());
        if( r->b.tp == hVal::STRING){
            std::cerr << *r->v.s;
            delete r;
            goto end;
        }
*/
    }

end:

    delete v;

    gq->pop_back();

    return 0;
}



//---------------------------------------

hVal* ef_run(ef_Content &ec,ef_Fac &ef,hVal *v)
{
    if( v->b.tp != hVal::FNC && v->b.tp != hVal::FNCC) {
        ++v->b.c;
        ec.sd->push_back(v);
        return vT;
    }

    if( v->b.tp == hVal::FNCC) {
        EF_FNC f = (EF_FNC)v->v.vl;
        return (*f)(ec);
    }

    hVal* ret = vT;
    register EF_STACKV_T *st = ec.svi;
    int top_lev = st->size();

    int top_lev_std = ec.sd->size();
    ec.sdv->push_back(top_lev_std);

    int flg_stop = 0;

    //----
//    HHDeqV_TI sv = v->v.q->begin();

    ec.svq->push_back(v->v.q);
    st->push_back(v->v.q->begin());

//    HHDeqV_TI &svi = sv;
    HHDeqV_T* svq = v->v.q;//ec.svq->back();

/*
    ++svi;
    ++svi;
    ++svi;
    ++svi;
*/

//    ec.svi->push_back(v->v.q->begin());
//    svi = ec.svi->back();
/*
    HHDeqV_TI svi2 = v->v.q->begin();
    ++svi2;
//    svi = svi2;

//    ec.svi->pop_back();
//    svi = ec.svi->back();
    svi = sv;
*/
    while( !flg_stop) {
        int top_lev_c = st->size();
/*
        int i = svi.
        std::cerr << "<--AAA ---\n";
        std::cerr.flush();
*/

        if( st->back() == svq->end()){
            st->pop_back();
            ec.svq->pop_back();

            top_lev_c = st->size();
            if( top_lev >= st->size()) {break;}

//            svi = ec.svi->back();
            svq = ec.svq->back();

            continue;
        }

        register hVal *vv  = *st->back();
//        void * vvv = vv->v.vl;
//        char * tpvv = tps[vv->b.tp];

        if( vv->b.tp == hVal::REPEATE){
            st->back() = svq->begin();
            continue;
        }

        if( vv->b.tp == hVal::BREAK){
            ret = vB;
            break;
        }

        if( vv->b.tp == hVal::RETURN){
            st->pop_back();
            ec.svq->pop_back();

            if( top_lev >= st->size()) {break;}

//            svi = ec.svi->back();
            svq = ec.svq->back();

            continue;
        }

        if( vv->b.tp != hVal::FNC && vv->b.tp != hVal::FNCC) {

            ml.lock();
            ++vv->b.c;
            ml.unlock();

            ec.sd->push_back(vv);
            ++st->back();
            continue;
        }

        if( vv->b.tp == hVal::FNCC) {
            EF_FNC f = (EF_FNC)vv->v.vl;
            hVal * ret_f = (*f)(ec);
//            if( ret_f->b.tp != hVal::TRUE) {
            if( ret_f) {
                if( ret_f->b.tp == hVal::REPEATE){
                    st->back() = svq->begin();
                    continue;
                }
                if( ret_f->b.tp == hVal::RETURN){
                    st->pop_back();
                    ec.svq->pop_back();

                    if( top_lev <= st->size()) {break;}

//                    svi = ec.svi->back();
                    svq = ec.svq->back();

                    continue;
                }
                if( ret_f->b.tp == hVal::BREAK){
                    ret = ret_f;
                    break;
/*
                    if( top_lev <= st->size()) {break;}

//                    svi = ec.svi->back();
                    svq = ec.svq->back();

                    continue;
*/
                }

                if( ret_f->b.tp == hVal::STRING){
                    ret = ret_f;
                    break;
                }
            }

            ++st->back();
            continue;
        }

        ++st->back();

        ec.svq->push_back(vv->v.q);
        st->push_back(vv->v.q->begin());

//        svi = ec.svi->back();
        svq = vv->v.q;//ec.svq->back();
    }

    //-- end --

    while( top_lev < st->size()){
        st->pop_back();
        ec.svq->pop_back();
    }

    while( top_lev_std < ec.sd->size()){
        hVal *z = ec.sd->back();
        delete z;
        ec.sd->pop_back();
    }

    ec.sdv->pop_back();

    return ret;
}

//---------------------------------------

void ef_init(ef_Content &ec,ef_Fac &ef,int argc,char *argv[])
{
    if( !G) {
        G = ec.ef.new_hVal(hVal::CMAP,0,0);
    }

    ec.wl();
    ec.g->insert( std::pair<std::string, hVal*>("SM",G));

    hVal *v = ef.new_hVal(hVal::DEQ,0,0);
    for( int i = 0; i < argc; ++i) {
        v->push( ef.new_hVal(hVal::STRING,ef.new_str(argv[i]),0));
    }
//    ec.g->insert( std::pair<std::string, hVal*>("ARGV",v));
    ec.g->insert( std::pair<std::string, hVal*>("A",v));

    ec.g->insert( std::pair<std::string, hVal*>("T",vT));
    ec.g->insert( std::pair<std::string, hVal*>("F",vF));
    ec.g->insert( std::pair<std::string, hVal*>("N",vN));

    ec.g->insert( std::pair<std::string, hVal*>("true",vT));
    ec.g->insert( std::pair<std::string, hVal*>("false",vF));
    ec.g->insert( std::pair<std::string, hVal*>("null",vN));

    ec.g->insert( std::pair<std::string, hVal*>("ZS",vZS));
    ec.g->insert( std::pair<std::string, hVal*>("NL",vNL));

    ec.g->insert( std::pair<std::string, hVal*>("R",vR));
    ec.g->insert( std::pair<std::string, hVal*>("B",vB));
    ec.g->insert( std::pair<std::string, hVal*>("RP",vRP));
//    ec.g->insert( std::pair<std::string, hVal*>("B",vB));

    ec.wu();

    // f------->

    ec.f->insert( std::pair<std::string, hVal*>("er",ef.new_hVal(hVal::FNCC,(void*)er,0)));
    ec.f->insert( std::pair<std::string, hVal*>("l",ef.new_hVal(hVal::FNCC,(void*)lg,0)));
    ec.f->insert( std::pair<std::string, hVal*>("TS",ef.new_hVal(hVal::FNCC,(void*)tostring_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("FS",ef.new_hVal(hVal::FNCC,(void*)fromstring_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("lT",ef.new_hVal(hVal::FNCC,(void*)lg_on,0)));
    ec.f->insert( std::pair<std::string, hVal*>("lF",ef.new_hVal(hVal::FNCC,(void*)lg_off,0)));
    ec.f->insert( std::pair<std::string, hVal*>("ls",ef.new_hVal(hVal::FNCC,(void*)lgs,0)));
    ec.f->insert( std::pair<std::string, hVal*>("lg",ef.new_hVal(hVal::FNCC,(void*)lgg,0)));

    ec.f->insert( std::pair<std::string, hVal*>("?",ef.new_hVal(hVal::FNCC,(void*)if_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("~",ef.new_hVal(hVal::FNCC,(void*)i_f,0)));

    ec.f->insert( std::pair<std::string, hVal*>("G",ef.new_hVal(hVal::FNCC,(void*)gg_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("L",ef.new_hVal(hVal::FNCC,(void*)len_f,0)));

    ec.f->insert( std::pair<std::string, hVal*>("Q",ef.new_hVal(hVal::FNCC,(void*)q_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("M",ef.new_hVal(hVal::FNCC,(void*)m_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("S",ef.new_hVal(hVal::FNCC,(void*)s_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("D",ef.new_hVal(hVal::FNCC,(void*)d_f,0)));

    ec.f->insert( std::pair<std::string, hVal*>("CQ",ef.new_hVal(hVal::FNCC,(void*)cq_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("CM",ef.new_hVal(hVal::FNCC,(void*)cm_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("CS",ef.new_hVal(hVal::FNCC,(void*)cs_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("CD",ef.new_hVal(hVal::FNCC,(void*)cd_f,0)));

    ec.f->insert( std::pair<std::string, hVal*>("C",ef.new_hVal(hVal::FNCC,(void*)clr_f,0)));

    ec.f->insert( std::pair<std::string, hVal*>("$",ef.new_hVal(hVal::FNCC,(void*)dup_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("%",ef.new_hVal(hVal::FNCC,(void*)drop_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("%<",ef.new_hVal(hVal::FNCC,(void*)dropAll_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("\\",ef.new_hVal(hVal::FNCC,(void*)swap_f,0)));

    //-- переменные стека
    ec.f->insert( std::pair<std::string, hVal*>("$0",ef.new_hVal(hVal::FNCC,(void*)dup_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("$1",ef.new_hVal(hVal::FNCC,(void*)dup1_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("$2",ef.new_hVal(hVal::FNCC,(void*)dup2_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("$3",ef.new_hVal(hVal::FNCC,(void*)dup3_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("$4",ef.new_hVal(hVal::FNCC,(void*)dup4_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("$5",ef.new_hVal(hVal::FNCC,(void*)dup5_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("$6",ef.new_hVal(hVal::FNCC,(void*)dup6_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("$7",ef.new_hVal(hVal::FNCC,(void*)dup7_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("$8",ef.new_hVal(hVal::FNCC,(void*)dup8_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("$9",ef.new_hVal(hVal::FNCC,(void*)dup9_f,0)));

    //-- переменные стека с начала
    ec.f->insert( std::pair<std::string, hVal*>("@",ef.new_hVal(hVal::FNCC,(void*)sfg0_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("@0",ef.new_hVal(hVal::FNCC,(void*)sfg0_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("@1",ef.new_hVal(hVal::FNCC,(void*)sfg1_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("@2",ef.new_hVal(hVal::FNCC,(void*)sfg2_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("@3",ef.new_hVal(hVal::FNCC,(void*)sfg3_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("@4",ef.new_hVal(hVal::FNCC,(void*)sfg4_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("@5",ef.new_hVal(hVal::FNCC,(void*)sfg5_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("@6",ef.new_hVal(hVal::FNCC,(void*)sfg6_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("@7",ef.new_hVal(hVal::FNCC,(void*)sfg7_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("@8",ef.new_hVal(hVal::FNCC,(void*)sfg8_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("@9",ef.new_hVal(hVal::FNCC,(void*)sfg9_f,0)));

    //-- переменные run-ов
    ec.f->insert( std::pair<std::string, hVal*>("v0",ef.new_hVal(hVal::FNCC,(void*)v0_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("v1",ef.new_hVal(hVal::FNCC,(void*)v1_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("v2",ef.new_hVal(hVal::FNCC,(void*)v2_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("v3",ef.new_hVal(hVal::FNCC,(void*)v3_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("v4",ef.new_hVal(hVal::FNCC,(void*)v4_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("v5",ef.new_hVal(hVal::FNCC,(void*)v5_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("v6",ef.new_hVal(hVal::FNCC,(void*)v6_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("v7",ef.new_hVal(hVal::FNCC,(void*)v7_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("v8",ef.new_hVal(hVal::FNCC,(void*)v8_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("v9",ef.new_hVal(hVal::FNCC,(void*)v9_f,0)));

    ec.f->insert( std::pair<std::string, hVal*>("v10",ef.new_hVal(hVal::FNCC,(void*)v10_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("v11",ef.new_hVal(hVal::FNCC,(void*)v11_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("v12",ef.new_hVal(hVal::FNCC,(void*)v12_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("v13",ef.new_hVal(hVal::FNCC,(void*)v13_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("v14",ef.new_hVal(hVal::FNCC,(void*)v14_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("v15",ef.new_hVal(hVal::FNCC,(void*)v15_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("v16",ef.new_hVal(hVal::FNCC,(void*)v16_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("v17",ef.new_hVal(hVal::FNCC,(void*)v17_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("v18",ef.new_hVal(hVal::FNCC,(void*)v18_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("v19",ef.new_hVal(hVal::FNCC,(void*)v19_f,0)));

    ec.f->insert( std::pair<std::string, hVal*>("v20",ef.new_hVal(hVal::FNCC,(void*)v20_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("v21",ef.new_hVal(hVal::FNCC,(void*)v21_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("v22",ef.new_hVal(hVal::FNCC,(void*)v22_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("v23",ef.new_hVal(hVal::FNCC,(void*)v23_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("v24",ef.new_hVal(hVal::FNCC,(void*)v24_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("v25",ef.new_hVal(hVal::FNCC,(void*)v25_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("v26",ef.new_hVal(hVal::FNCC,(void*)v26_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("v27",ef.new_hVal(hVal::FNCC,(void*)v27_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("v28",ef.new_hVal(hVal::FNCC,(void*)v28_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("v29",ef.new_hVal(hVal::FNCC,(void*)v29_f,0)));

    ec.f->insert( std::pair<std::string, hVal*>("K",ef.new_hVal(hVal::FNCC,(void*)q0_f,0)));// ключь
    ec.f->insert( std::pair<std::string, hVal*>("V",ef.new_hVal(hVal::FNCC,(void*)q1_f,0)));// значение

    ec.f->insert( std::pair<std::string, hVal*>("q0",ef.new_hVal(hVal::FNCC,(void*)q0_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("q1",ef.new_hVal(hVal::FNCC,(void*)q1_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("q2",ef.new_hVal(hVal::FNCC,(void*)q2_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("q3",ef.new_hVal(hVal::FNCC,(void*)q3_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("q4",ef.new_hVal(hVal::FNCC,(void*)q4_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("q5",ef.new_hVal(hVal::FNCC,(void*)q5_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("q6",ef.new_hVal(hVal::FNCC,(void*)q6_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("q7",ef.new_hVal(hVal::FNCC,(void*)q7_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("q8",ef.new_hVal(hVal::FNCC,(void*)q8_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("q9",ef.new_hVal(hVal::FNCC,(void*)q9_f,0)));

    //--

    // присваивания
    ec.f->insert( std::pair<std::string, hVal*>(".",ef.new_hVal(hVal::FNCC,(void*)set1_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>(".$",ef.new_hVal(hVal::FNCC,(void*)set1_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>(".$0",ef.new_hVal(hVal::FNCC,(void*)set1_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>(".$1",ef.new_hVal(hVal::FNCC,(void*)set1_f,0)));

    ec.f->insert( std::pair<std::string, hVal*>(".$2",ef.new_hVal(hVal::FNCC,(void*)set2_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>(".$3",ef.new_hVal(hVal::FNCC,(void*)set3_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>(".$4",ef.new_hVal(hVal::FNCC,(void*)set4_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>(".$5",ef.new_hVal(hVal::FNCC,(void*)set5_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>(".$6",ef.new_hVal(hVal::FNCC,(void*)set6_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>(".$7",ef.new_hVal(hVal::FNCC,(void*)set7_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>(".$8",ef.new_hVal(hVal::FNCC,(void*)set8_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>(".$9",ef.new_hVal(hVal::FNCC,(void*)set9_f,0)));

    //--

    ec.f->insert( std::pair<std::string, hVal*>(".v",ef.new_hVal(hVal::FNCC,(void*)sv0_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>(".v0",ef.new_hVal(hVal::FNCC,(void*)sv0_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>(".v1",ef.new_hVal(hVal::FNCC,(void*)sv1_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>(".v2",ef.new_hVal(hVal::FNCC,(void*)sv2_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>(".v3",ef.new_hVal(hVal::FNCC,(void*)sv3_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>(".v4",ef.new_hVal(hVal::FNCC,(void*)sv4_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>(".v5",ef.new_hVal(hVal::FNCC,(void*)sv5_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>(".v6",ef.new_hVal(hVal::FNCC,(void*)sv6_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>(".v7",ef.new_hVal(hVal::FNCC,(void*)sv7_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>(".v8",ef.new_hVal(hVal::FNCC,(void*)sv8_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>(".v9",ef.new_hVal(hVal::FNCC,(void*)sv9_f,0)));

    ec.f->insert( std::pair<std::string, hVal*>(".v10",ef.new_hVal(hVal::FNCC,(void*)sv10_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>(".v11",ef.new_hVal(hVal::FNCC,(void*)sv11_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>(".v12",ef.new_hVal(hVal::FNCC,(void*)sv12_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>(".v13",ef.new_hVal(hVal::FNCC,(void*)sv13_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>(".v14",ef.new_hVal(hVal::FNCC,(void*)sv14_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>(".v15",ef.new_hVal(hVal::FNCC,(void*)sv15_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>(".v16",ef.new_hVal(hVal::FNCC,(void*)sv16_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>(".v17",ef.new_hVal(hVal::FNCC,(void*)sv17_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>(".v18",ef.new_hVal(hVal::FNCC,(void*)sv18_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>(".v19",ef.new_hVal(hVal::FNCC,(void*)sv19_f,0)));

    ec.f->insert( std::pair<std::string, hVal*>(".v20",ef.new_hVal(hVal::FNCC,(void*)sv20_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>(".v21",ef.new_hVal(hVal::FNCC,(void*)sv21_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>(".v22",ef.new_hVal(hVal::FNCC,(void*)sv22_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>(".v23",ef.new_hVal(hVal::FNCC,(void*)sv23_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>(".v24",ef.new_hVal(hVal::FNCC,(void*)sv24_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>(".v25",ef.new_hVal(hVal::FNCC,(void*)sv25_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>(".v26",ef.new_hVal(hVal::FNCC,(void*)sv26_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>(".v27",ef.new_hVal(hVal::FNCC,(void*)sv27_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>(".v28",ef.new_hVal(hVal::FNCC,(void*)sv28_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>(".v29",ef.new_hVal(hVal::FNCC,(void*)sv29_f,0)));

    //--

    ec.f->insert( std::pair<std::string, hVal*>(".K",ef.new_hVal(hVal::FNCC,(void*)sq0_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>(".V",ef.new_hVal(hVal::FNCC,(void*)sq1_f,0)));

    ec.f->insert( std::pair<std::string, hVal*>(".q0",ef.new_hVal(hVal::FNCC,(void*)sq0_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>(".q1",ef.new_hVal(hVal::FNCC,(void*)sq1_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>(".q2",ef.new_hVal(hVal::FNCC,(void*)sq2_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>(".q3",ef.new_hVal(hVal::FNCC,(void*)sq3_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>(".q4",ef.new_hVal(hVal::FNCC,(void*)sq4_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>(".q5",ef.new_hVal(hVal::FNCC,(void*)sq5_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>(".q6",ef.new_hVal(hVal::FNCC,(void*)sq6_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>(".q7",ef.new_hVal(hVal::FNCC,(void*)sq7_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>(".q8",ef.new_hVal(hVal::FNCC,(void*)sq8_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>(".q9",ef.new_hVal(hVal::FNCC,(void*)sq9_f,0)));

    ec.f->insert( std::pair<std::string, hVal*>("P",ef.new_hVal(hVal::FNCC,(void*)format_f,0)));// printf-like

    // действия
    ec.f->insert( std::pair<std::string, hVal*>("+",ef.new_hVal(hVal::FNCC,(void*)plus_f,0)));// многофункциональный
    ec.f->insert( std::pair<std::string, hVal*>("-",ef.new_hVal(hVal::FNCC,(void*)min_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("*",ef.new_hVal(hVal::FNCC,(void*)zv_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("/",ef.new_hVal(hVal::FNCC,(void*)delen_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("//",ef.new_hVal(hVal::FNCC,(void*)delen_ch_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("^",ef.new_hVal(hVal::FNCC,(void*)tre_f,0)));// ^() функцию в стек и далее

    ec.f->insert( std::pair<std::string, hVal*>("++",ef.new_hVal(hVal::FNCC,(void*)plusplus_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("--",ef.new_hVal(hVal::FNCC,(void*)minmin_f,0)));

    ec.f->insert( std::pair<std::string, hVal*>("SR",ef.new_hVal(hVal::FNCC,(void*)srt_f,0)));// sort
    ec.f->insert( std::pair<std::string, hVal*>("RS",ef.new_hVal(hVal::FNCC,(void*)rsrt_f,0)));// rsort

    ec.f->insert( std::pair<std::string, hVal*>("TP",ef.new_hVal(hVal::FNCC,(void*)type_f,0)));// выдать тип

    ec.f->insert( std::pair<std::string, hVal*>("==",ef.new_hVal(hVal::FNCC,(void*)eq_type_f,0)));// равенство типов

    // условные
    ec.f->insert( std::pair<std::string, hVal*>("&",ef.new_hVal(hVal::FNCC,(void*)and_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("|",ef.new_hVal(hVal::FNCC,(void*)or_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("!",ef.new_hVal(hVal::FNCC,(void*)no_f,0)));

    ec.f->insert( std::pair<std::string, hVal*>(">",ef.new_hVal(hVal::FNCC,(void*)g_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("<",ef.new_hVal(hVal::FNCC,(void*)l_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>(">=",ef.new_hVal(hVal::FNCC,(void*)ge_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("<=",ef.new_hVal(hVal::FNCC,(void*)le_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("!=",ef.new_hVal(hVal::FNCC,(void*)ne_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("=",ef.new_hVal(hVal::FNCC,(void*)e_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("=N",ef.new_hVal(hVal::FNCC,(void*)eqn_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("!N",ef.new_hVal(hVal::FNCC,(void*)neqn_f,0)));

    //--
    ec.f->insert( std::pair<std::string, hVal*>("??",ef.new_hVal(hVal::FNCC,(void*)mquit_f,0)));//must Quit
    ec.f->insert( std::pair<std::string, hVal*>("E",ef.new_hVal(hVal::FNCC,(void*)exit_f,0)));// exit == true ??
    ec.f->insert( std::pair<std::string, hVal*>("UT",ef.new_hVal(hVal::FNCC,(void*)ut_f,0)));// unix time
    ec.f->insert( std::pair<std::string, hVal*>("SL",ef.new_hVal(hVal::FNCC,(void*)sl_f,0)));// sleep
    ec.f->insert( std::pair<std::string, hVal*>("SS",ef.new_hVal(hVal::FNCC,(void*)ssize_f,0)));// stack size

    ec.f->insert( std::pair<std::string, hVal*>("TM",ef.new_hVal(hVal::FNCC,(void*)timer_f,0)));// timer
    ec.f->insert( std::pair<std::string, hVal*>("TH",ef.new_hVal(hVal::FNCC,(void*)th_f,0)));//thread
    ec.f->insert( std::pair<std::string, hVal*>("TID",ef.new_hVal(hVal::FNCC,(void*)tid_f,0)));//thread

    // синхро локеры-мутексы
    ec.f->insert( std::pair<std::string, hVal*>("LC",ef.new_hVal(hVal::FNCC,(void*)lock_f,0)));

    ec.f->insert( std::pair<std::string, hVal*>("RL",ef.new_hVal(hVal::FNCC,(void*)rl_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("WL",ef.new_hVal(hVal::FNCC,(void*)wl_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("RU",ef.new_hVal(hVal::FNCC,(void*)ru_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("WU",ef.new_hVal(hVal::FNCC,(void*)wu_f,0)));

    //-- sha1
    ec.f->insert( std::pair<std::string, hVal*>("S1",ef.new_hVal(hVal::FNCC,(void*)sha1_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("S1D",ef.new_hVal(hVal::FNCC,(void*)sha1d_f,0)));

    //-- sys
    ec.f->insert( std::pair<std::string, hVal*>("RM",ef.new_hVal(hVal::FNCC,(void*)rm_f,0)));
//    ec.f->insert( std::pair<std::string, hVal*>("CM",ef.new_hVal(hVal::FNCC,(void*)rm_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("REN",ef.new_hVal(hVal::FNCC,(void*)ren_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("MKD",ef.new_hVal(hVal::FNCC,(void*)mkd_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("DIR",ef.new_hVal(hVal::FNCC,(void*)dir_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("CDIR",ef.new_hVal(hVal::FNCC,(void*)cdir_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("PID",ef.new_hVal(hVal::FNCC,(void*)pid_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("KILL",ef.new_hVal(hVal::FNCC,(void*)kill_f,0)));

    ec.f->insert( std::pair<std::string, hVal*>("RD",ef.new_hVal(hVal::FNCC,(void*)read_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("RDB",ef.new_hVal(hVal::FNCC,(void*)readB_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("WR",ef.new_hVal(hVal::FNCC,(void*)write_f,0)));

    //-- sql
    ec.f->insert( std::pair<std::string, hVal*>("DB",ef.new_hVal(hVal::FNCC,(void*)db_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("SQL",ef.new_hVal(hVal::FNCC,(void*)sql_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("EX",ef.new_hVal(hVal::FNCC,(void*)sexec_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("!!",ef.new_hVal(hVal::FNCC,(void*)sexec_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("NX",ef.new_hVal(hVal::FNCC,(void*)next_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>(">>",ef.new_hVal(hVal::FNCC,(void*)next_f,0)));

    ec.f->insert( std::pair<std::string, hVal*>("PR",ef.new_hVal(hVal::FNCC,(void*)prep_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("ER",ef.new_hVal(hVal::FNCC,(void*)serr_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("RW",ef.new_hVal(hVal::FNCC,(void*)rows_f,0)));

    // RUN PROCESS
    ec.f->insert( std::pair<std::string, hVal*>("RUN",ef.new_hVal(hVal::FNCC,(void*)run_f,0)));

    // SOCKS
    ec.f->insert( std::pair<std::string, hVal*>("sL",ef.new_hVal(hVal::FNCC,(void*)sL_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("sC",ef.new_hVal(hVal::FNCC,(void*)sC_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("sE",ef.new_hVal(hVal::FNCC,(void*)sE_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("sS",ef.new_hVal(hVal::FNCC,(void*)sS_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("sA",ef.new_hVal(hVal::FNCC,(void*)sA_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("sR",ef.new_hVal(hVal::FNCC,(void*)sR_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("sW",ef.new_hVal(hVal::FNCC,(void*)sW_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("sRR",ef.new_hVal(hVal::FNCC,(void*)sRR_f,0)));
    ec.f->insert( std::pair<std::string, hVal*>("sWR",ef.new_hVal(hVal::FNCC,(void*)sWR_f,0)));

    ec.f->insert( std::pair<std::string, hVal*>("UDP",ef.new_hVal(hVal::FNCC,(void*)udp_f,0)));

    // -- INCLUDE
    ec.c->insert( std::pair<std::string, hVal*>("INC",ef.new_hVal(hVal::FNCC,(void*)inc_f,0)));
    ec.c->insert( std::pair<std::string, hVal*>("LD",ef.new_hVal(hVal::FNCC,(void*)llib_f,0)));

    //-- ---------------- тест ------------------ --//

    ec.f->insert( std::pair<std::string, hVal*>("tt",ef.new_hVal(hVal::FNCC,(void*)tt,0)));
    ec.f->insert( std::pair<std::string, hVal*>("bb",ef.new_hVal(hVal::FNCC,(void*)bb,0)));
    ec.f->insert( std::pair<std::string, hVal*>("ee",ef.new_hVal(hVal::FNCC,(void*)ee,0)));
    ec.f->insert( std::pair<std::string, hVal*>("aa",ef.new_hVal(hVal::FNCC,(void*)aa,0)));
    ec.f->insert( std::pair<std::string, hVal*>("cc",ef.new_hVal(hVal::FNCC,(void*)cc,0)));
    ec.f->insert( std::pair<std::string, hVal*>("dd",ef.new_hVal(hVal::FNCC,(void*)dd,0)));

    ec.f->insert( std::pair<std::string, hVal*>("f100",ef.new_hVal(hVal::FNCC,(void*)f100_f,0)));

}

