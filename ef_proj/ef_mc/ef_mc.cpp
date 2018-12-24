#include "ef_mc.h"

extern "C" {
#include <libmemcached/memcached.h>
#include <libmemcached/util.h>
}

std::string LD_PREF = "mc.";
#define LD_ID 111

std::string *newUUID();
void delStrEf(std::string *s);
void getTID(char *s);
std::string tp_f(int i);

void delete_f(void *vv)
{
//    fprintf(stderr,"DEL_FNC %d %ld\n",LD_ID,vv); fflush(stderr);

    memcached_st *memc = (memcached_st *)vv;
    if( memc) memcached_free( memc);
}

hVal* mcNew_f(ef_Content &ec){

    //fprintf(stderr,"mcNew_f ------------- !!!\n"); fflush(stderr);

    hVal *b = ec.sd->back();

    std::string cs = "--SERVER=127.0.0.1";

    if( b->b.tp == hVal::STRING || b->b.tp == hVal::CSTRING){
        cs = *b->v.s;
    }else if(b->b.tp == hVal::DEQ || b->b.tp == hVal::CDEQ){

    }

    memcached_st *memc= memcached(cs.c_str(), cs.size());

    if( !memc) {
        return 0;
    }

    memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL, true);

    delete b;

    ec.sd->back() = ec.ef.new_hVal(LD_ID,memc,0);

    return 0;
}

enum{ CMD_GET,CMD_SET,CMD_DEL,CMD_APP};

char *cmd_s[] = {"CMD_GET","CMD_SET","CMD_DEL","CMD_APP"};

static hLock mcl;
hVal* mcCMD_f(ef_Content &ec,int cmd){
    if( ec.sd->size() < 2) return 0;

    memcached_return_t rc = MEMCACHED_SUCCESS;

    mcl.wl();

    unsigned long exp = 0;
    HHDeqV_T *q;
    hVal *q0;
    hVal *q1;
    hVal *q2;
    hVal *q3;

    hVal *k = 0;
    hVal *gk = 0;
    void *vl = (void *)"";
    long vll = 0;
    memcached_st *memc = 0;

    hVal *v = ec.sd->back();
    hVal *db = ec.sd->at(ec.sd->size()-2);

    if( db->b.tp != LD_ID) goto end;

    memc = (memcached_st *)db->v.vl;

    if( v->b.tp != hVal::DEQ && v->b.tp != hVal::CDEQ && v->b.tp != hVal::STRING && v->b.tp != hVal::CSTRING) goto end;

    if( v->b.tp == hVal::STRING || v->b.tp == hVal::CSTRING){
        k = v;
    }else{
        q = v->v.q;
//        fprintf(stderr,"q->size() %d\n",q->size());

        if( q->size() == 0){goto end;}

        q0 = q->at(0);
        if( q0->b.tp != hVal::STRING && q0->b.tp != hVal::CSTRING){goto end;}
        k = q0;

        if( q->size() > 1){
            q1 = q->at(1);
            if( q1->b.tp == hVal::STRING || q1->b.tp == hVal::CSTRING || q1->b.tp == hVal::BUF){
                vl = (void*)q1->v.s->c_str();
                vll = q1->v.s->size();
            }
        }

        if( q->size() > 2){
            q2 = q->at(2);
            if( q2->b.tp == hVal::DOUBLE || q2->b.tp == hVal::CDOUBLE){
                exp = time(0) + q2->v.d;
            }
        }

        if( q->size() > 3){
            q3 = q->at(3);
            if( q3->b.tp == hVal::STRING || q3->b.tp == hVal::CSTRING){
                gk = q3;
            }
        }

    }

//    fprintf(stderr,"%s -> gk: %lu exp %d\n",cmd_s[cmd],gk,exp);

    switch( cmd){
    case CMD_SET:

        if( !gk){
            rc = memcached_set(memc,k->v.s->c_str(), k->v.s->size(), (char*)vl, vll,exp, 0);
        }else{
//            fprintf(stderr,"CMD_SET gk: %lu %s %s\n",gk,gk->v.s->c_str(),vl);
            rc = memcached_set_by_key(memc, gk->v.s->c_str(), gk->v.s->size(),k->v.s->c_str(), k->v.s->size(),(char*) vl, vll,exp, 0);
        }
        if( rc) {
            fprintf(stderr,"memcached CMD_SET ERROR :: %d %s\n",rc,memcached_strerror(memc,rc));
        }
        goto end;

        break;

    case CMD_DEL:

        if( !gk){
            rc = memcached_delete(memc,k->v.s->c_str(), k->v.s->size(), exp);
        }else{
            rc = memcached_delete_by_key(memc, gk->v.s->c_str(), gk->v.s->size(),k->v.s->c_str(), k->v.s->size(),exp);
        }
        if( rc) {
            fprintf(stderr,"memcached CMD_DEL ERROR :: %d %s\n",rc,memcached_strerror(memc,rc));
        }
        goto end;

        break;

    case CMD_APP:

        if( !gk){
            rc = memcached_append(memc,k->v.s->c_str(), k->v.s->size(), (char*)vl, vll,exp, 0);
        }else{
            rc = memcached_append_by_key(memc, gk->v.s->c_str(), gk->v.s->size(),k->v.s->c_str(), k->v.s->size(),(char*) vl, vll,exp, 0);
        }
        if( rc) {
            fprintf(stderr,"memcached CMD_APP ERROR :: %d %s\n",rc,memcached_strerror(memc,rc));
        }
        goto end;

        break;

    case CMD_GET:

        hVal *rv = ec.gN();

        size_t value_length;
        char *zn = 0;

        if( !gk){
            zn =  memcached_get(memc,k->v.s->c_str(), k->v.s->size(), &value_length, 0,&rc);
        }else{
//            fprintf(stderr,"CMD_GET gk: %lu %s\n",gk,gk->v.s->c_str());
            zn =  memcached_get_by_key(memc, gk->v.s->c_str(), gk->v.s->size(),k->v.s->c_str(), k->v.s->size(), &value_length, 0,&rc);
        }

        if( rc) {
            if( rc != MEMCACHED_NOTFOUND)
                fprintf(stderr,"memcached CMD_GET ERROR :: %d %s\n",rc,memcached_strerror(memc,rc));
        }else{
            if( value_length > 0){
                rv = ec.ef.new_hVal(hVal::STRING,ec.ef.new_str(zn,value_length),0);
            }else{
                rv = ec.gZS();
            }
        }

        delete v;
        ec.sB(rv);

        if( zn) free(zn);

        mcl.wu();

        return 0;

        break;
    }

end:

    mcl.wu();

    delete v;
//    delete db;

    ec.sPB();
//    ec.sPB();

    return 0;
}

hVal* mcSet_f(ef_Content &ec,int cmd){
    mcCMD_f(ec,CMD_SET);
    return 0;
}

hVal* mcGet_f(ef_Content &ec){
    mcCMD_f(ec,CMD_GET);
    return 0;
}

hVal* mcDel_f(ef_Content &ec){
    mcCMD_f(ec,CMD_DEL);
    return 0;
}

hVal* mcApp_f(ef_Content &ec){
    mcCMD_f(ec,CMD_APP);
    return 0;
}


hVal* mcClear_f(ef_Content &ec){
    if( ec.sd->size() < 1) return 0;

    hVal *db = ec.sd->back();
    if( db->b.tp != LD_ID) return 0;

    memcached_st *memc = (memcached_st *)db->v.vl;
//    memcached_flush(memc, 0);

    return 0;
}

void init(ef_Content &ec)
{
//    std::cerr << "\n" << "inf INIT DLL" << "\n\n"; std::cerr.flush();

    ec.wl();

    ec.dI(LD_ID, delete_f);

    ec.fI(LD_PREF + "N",ec.ef.new_hVal(hVal::FNCC,(void*)mcNew_f,0));

    ec.fI(LD_PREF + "G",ec.ef.new_hVal(hVal::FNCC,(void*)mcGet_f,0));
    ec.fI(LD_PREF + "S",ec.ef.new_hVal(hVal::FNCC,(void*)mcSet_f,0));
    ec.fI(LD_PREF + "D",ec.ef.new_hVal(hVal::FNCC,(void*)mcDel_f,0));
    ec.fI(LD_PREF + "A",ec.ef.new_hVal(hVal::FNCC,(void*)mcApp_f,0));

    ec.fI(LD_PREF + "CLEAR!",ec.ef.new_hVal(hVal::FNCC,(void*)mcClear_f,0));


//    ec.dm->insert( std::pair<int, DEL_FNC>(LD_ID, lv_del_f));
//    ec.f->insert( std::pair<std::string, hVal*>(LD_PREF + "N",ec.ef.new_hVal(hVal::FNCC,(void*)lvNew_f,0)));

/*
    ec.dI(LD_ID, lv_del_f);

    ec.fI(LD_PREF + "N",ec.ef.new_hVal(hVal::FNCC,(void*)lvNew_f,0));

    ec.fI(LD_PREF + "S",ec.ef.new_hVal(hVal::FNCC,(void*)lvSet_f,0));
    ec.fI(LD_PREF + "S-",ec.ef.new_hVal(hVal::FNCC,(void*)lvSet_fm,0));

    ec.fI(LD_PREF + "D",ec.ef.new_hVal(hVal::FNCC,(void*)lvDel_f,0));
    ec.fI(LD_PREF + "D-",ec.ef.new_hVal(hVal::FNCC,(void*)lvDel_fm,0));

    ec.fI(LD_PREF + "SYNC",ec.ef.new_hVal(hVal::FNCC,(void*)lvSync_f,0));

    ec.fI(LD_PREF + "GL",ec.ef.new_hVal(hVal::FNCC,(void*)lvGetL_f,0));

    ec.fI(LD_PREF + "G",ec.ef.new_hVal(hVal::FNCC,(void*)lvGet_f,0));
    ec.fI(LD_PREF + "F",ec.ef.new_hVal(hVal::FNCC,(void*)lvFind_f,0));


    //-- TEST
    ec.fI(LD_PREF + "T",ec.ef.new_hVal(hVal::FNCC,(void*)lvTEST_f,0));
*/
    ec.wu();

//    std::cerr << "\n" << "inf INIT DLL DONE!!!" << "\n\n"; std::cerr.flush();
}

