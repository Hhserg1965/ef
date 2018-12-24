#include <../ef.hpp>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "../ef.hpp"
#include "uv.h"
//#include "unistd.h"


//#include <cassert>

extern hVal *vT;
extern hVal *vF;
extern hVal *vN;

static std::string LD_PREF = "sp.";
static int LD_ID = 60;

class efSPAWN
{
public:
    int af;
    int er;
    uv_process_t pc;

    efSPAWN(){
        er=0;
        af = 1;
        //pc={0};
        memset(&pc,0,sizeof(pc));
        pc.u.reserved[3] = this;
    }
};

static void del_f(void *vv)
{
    efSPAWN *o = (efSPAWN *) vv;
    if( o) {
//        fprintf(stderr,"DEL_FNC efSPAWN %ld !!!\n",vv); fflush(stderr);
        uv_unref((uv_handle_t*) &o->pc);
        delete o;
    }
}

void sp_exit_cb(uv_process_t* h, int64_t exit_status, int term_signal)
{
//    fprintf(stderr, "!!!!!!!!!!--------- sp_exit_cb >> %d term_signal %d \n", exit_status,term_signal);fflush(stderr);
    efSPAWN *pc = (efSPAWN *)h->u.reserved[3];

//    fprintf(stderr,"sp_exit_cb %ld !!!\n",pc); fflush(stderr);

    pc->af = 0;
}

static hVal* spA_f(ef_Content &ec)
{
    hVal *r = vF;

    register hVal *v = ec.sd->back();
    if( v->b.tp == LD_ID){
//        fprintf(stderr, "!spA_f++\n");fflush(stderr);
        efSPAWN *pc = (efSPAWN *) v->v.vl;
//        fprintf(stderr,"spA_f %ld !!!\n",pc); fflush(stderr);

        if( pc->af) r = vT;
    }

end:
    ec.sP(r);

    return 0;
}

static hVal* spK_f(ef_Content &ec)
{
    register hVal *v = ec.sd->back();
    if( v->b.tp == LD_ID){
//        fprintf(stderr, "!spA_f++\n");fflush(stderr);
        efSPAWN *pc = (efSPAWN *) v->v.vl;
        uv_process_kill(&pc->pc, SIGINT);
    }

    return 0;
}

static hVal* spNew_f(ef_Content &ec)
{
#define MAX_RUN_PAR 500

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
//                std::cerr << s << " dir(" << dir <<")\n"; std::cerr.flush();
                continue;
            }else{
                args[i] = s;
                ++i;
            }
        }
    }
    ln = i;
/*
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
*/
    uv_process_options_t options = {0};

    options.file = args[0];
    options.args = args;
    options.flags = UV_PROCESS_DETACHED | UV_PROCESS_WINDOWS_HIDE;
    options.exit_cb = sp_exit_cb;

    if( dir.size() > 0) options.cwd = dir.c_str();

    efSPAWN *pc = new efSPAWN();

//    fprintf(stderr,"DEL_FNC new %ld !!!\n",pc); fflush(stderr);

    int r;
    if ((r = uv_spawn(uv_default_loop(), &pc->pc, &options))) {
        fprintf(stderr, "efSPAWN >> %s\n", uv_strerror(r));
//        uv_unref((uv_handle_t*) &pc->pc);
//        delete pc;
        pc->er = 1;
        pc->af = 0;

//        delete v;
//        ec.sd->back() = ec.gN();

//        return 0;
    }
    fprintf(stderr, "Launched efSPAWN with PID %d af %d\n", pc->pc.pid,pc->af);

    delete v;
    ec.sd->back() = ec.ef.new_hVal(LD_ID,pc,0);
//    ec.sd->back() = ec.ef.new_hVal(hVal::DOUBLE,0,(double)child_req.pid);

//    fprintf(stderr, "Launched@ sleep with PID %d\n", child_req.pid);
//    uv_unref((uv_handle_t*) &child_req);

    return 0 ;
}

void ef_sp_init(ef_Content &ec)
{
//    std::cerr << "\n" << "OCV INIT DLL---" << "\n\n"; std::cerr.flush();

    ec.wl();

    ec.dI(LD_ID, del_f);

    ec.fI(LD_PREF + "N",ec.ef.new_hVal(hVal::FNCC,(void*)spNew_f,0));
    ec.fI(LD_PREF + "A",ec.ef.new_hVal(hVal::FNCC,(void*)spA_f,0));
    ec.fI(LD_PREF + "K",ec.ef.new_hVal(hVal::FNCC,(void*)spK_f,0));
/*    ec.fI(LD_PREF + "SAVE",ec.ef.new_hVal(hVal::FNCC,(void*)cvSave_f,0));
*/
    ec.wu();
}

