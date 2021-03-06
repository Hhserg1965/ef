#include <iostream>
#include "uv.h"
#include <stdio.h>
#include <string.h>
//#include<zlib.h>
#include<time.h>
#include<stdlib.h>

#ifndef _WIN32
#include <stdint.h>
#include <unistd.h>
#include <netinet/in.h>
#define stricmp strcasecmp
#define strcmpi strcasecmp
#define strnicmp strncasecmp
#include <netdb.h>
#include <sys/socket.h>
#endif

#ifdef _WIN32
#include "windows.h"
#include "winsock.h"

typedef int int32_t;
typedef int socklen_t;

#else
#include <signal.h>
#include <sys/wait.h>
#endif


//#include <v8.h>

//#include "js_main.h"
#include "../ef.hpp"

//using namespace std;
//using namespace v8;

//v8::Persistent<v8::Context> context;

int quit_flg = false;
int run_process = 0;

uv_thread_t tprc = 0;

int STACK_VIEW  = 5;

int argc_;
char **argv_;
void ktThread(void *arg);
void getsTrun(void*);

//uv_loop_t *loop;// = malloc(sizeof(uv_loop_t));

//#ifdef _WIN32
//#else
#include <signal.h>
//#include <sys/wait.h>

void js_init();
void mc_init();


uv_signal_t sig1, sig2,sig3,sig4,sig5,sig6;

void signal_handler(uv_signal_t *handle, int signum)
{
    printf("Signal received: %d\n", signum);
    uv_signal_stop(handle);
    quit_flg = true;
}

void signal_handlerP(uv_signal_t *handle, int signum)
{
    fprintf(stderr,"SIGNAL qhdl_PIPE %d !\n",signum);fflush(stderr);
//    FCGX_ShutdownPending();
//    quit_flg = true;
}
/*
void signal_handlerH(uv_signal_t *handle, int signum)
{
//    pid_t p = wait( &stat_loc);

    int stat_loc;

    while( 1) {
        pid_t p = waitpid( -1, &stat_loc,WNOHANG);
        if( p <= 0) {
            break;
        }
        fprintf(stderr,"EXIT PROCESS %d Status %d\n",p,stat_loc);fflush(stderr);
    }

    run_process = 0;
}
*/
void qhdl(int sig)
{
    fprintf(stderr,"SIGNAL qhdl %d !\n",sig);fflush(stderr);
//    FCGX_ShutdownPending();
    quit_flg = true;
}

void qhdl_PIPE(int sig)
{
    fprintf(stderr,"SIGNAL qhdl_PIPE %d !\n",sig);fflush(stderr);
//    FCGX_ShutdownPending();
//    quit_flg = true;
}
/*
void qhdl_SIGCHLD(int sig)
{
    fprintf(stderr,"SIGNAL %d !\n",sig);fflush(stderr);
    int stat_loc;

//    pid_t p = wait( &stat_loc);
    while( 1) {
        pid_t p = waitpid( -1, &stat_loc,WNOHANG);
        if( p <= 0) {
            break;
        }
        fprintf(stderr,"EXIT PROCESS %d Status %d\n",p,stat_loc);fflush(stderr);
    }

    run_process = 0;
}
*/
//#endif

void Usage(char *programName)
{
    /* Modify here to add your usage message when the program is
     * called without arguments */

    fprintf(stderr,"%s usage:\n",programName);
    fprintf(stderr,"%s [-<options>] \n",programName);
    fprintf(stderr,"options:\n");

//    fprintf(stderr,"\t-S<SPHINX server_name(localhost)>\n");

    fprintf(stderr,"\t-?\tHELP\n");
}

char f_in[1000] = "in.ef";
char silent_flg = 0;
char jstype_flg = 0;
//extern int STACK_VIEW;

char node_id[100] = "ef";
unsigned long long  node_cnt = 1;
int node_idn = 0;

void wrCTN()
{
    FILE *f = fopen("ef_cnt","wb");
    if( f){
        fprintf(f,"%lu",node_cnt);
        fclose(f);
    }
}

void getTID(char *s);
/*
void getTID(char *s)
{
    ++node_cnt;
    time_t t = time(0);

    sprintf(s,"%08X%s%08X",t,node_id,node_cnt % 4294967296);
}
*/
int HandleOptions(int argc,char *argv[])
{
    int i,firstnonoption=0;

    for (i=1; i< argc;i++) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
                /* An argument -? means help is requested */
            case '?':
                Usage(argv[0]);
                break;

            case '%':
                strcpy(node_id,&argv[i][2]);
                node_idn = atoi(node_id);
                break;

//            case 's':
//                if( !strcmpi("silent",&argv[i][1])) silent_flg = 1;
//                break;
            case 'J':
                if( !strcmpi("JSTYPE",&argv[i][1])) jstype_flg = 1;
                break;
            case 'S':
//                if( !strcmpi("silent",&argv[i][1])) silent_flg = 1;
                STACK_VIEW = atol(&argv[i][2]);
                break;

            default:
//                fprintf(stderr,"unknown option %s\n",argv[i]);


                break;
            }
        }
        else {
            firstnonoption = i;
            strcpy(f_in,argv[i]);
//			break;
        }
    }
    return firstnonoption;
}

char * get_file_text_utf8(char * fn,int *fsz)
{
    char *b = 0;
    FILE *f = fopen(fn,"rb");
    if( f) {
        fseek(f,0,SEEK_END);
        int sz = ftell(f);
        b = (char *)malloc(sz+10);
        fseek(f,0,SEEK_SET);
        fread(b,1,sz,f);
        b[sz] = 0;
        if( fsz) *fsz = sz;

        fclose(f);
    }

    return(b);
}


const int KB = 1024;
const int MB = KB * KB;
const int GB = KB * KB * KB;
const int kPointerSize   = sizeof(void*);     // NOLINT

int main(int argc, char *argv[])
{
    argc_ = argc;
    argv_ = argv;
/*
    printf("sizeof(hVal::V) %d\n",sizeof(hVal::V));
    printf("sizeof(hVal::VBK) %d\n",sizeof(hVal::VBK));
    printf("sizeof(hVal) %d\n",sizeof(hVal));
*/
//    printf("sizeof(hVal) %d\n",sizeof(hVal));

//    printf("node_idn %d\n",node_idn);
//--

//    tst_ef();

#ifdef _WIN32
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    wVersionRequested = MAKEWORD( 2, 2 );

    err = WSAStartup( wVersionRequested, &wsaData );
    if ( err != 0 ) {
    }

    /* Confirm that the WinSock DLL supports 2.2.*/
    /* Note that if the DLL supports versions later    */
    /* than 2.2 in addition to 2.2, it will still return */
    /* 2.2 in wVersion since that is the version we      */
    /* request ed.                                        */

    if ( LOBYTE( wsaData.wVersion ) != 2 ||
            HIBYTE( wsaData.wVersion ) != 2 ) {
        /* Tell the user that we could not find a usable */
        /* WinSock DLL.                                  */
        fprintf(stderr,"Tell the user that we could not find a usable WinSock DLL. \n");fflush(stderr);
//		WSACleanup( );
//		return;
    }
#endif

    HandleOptions(argc,argv);
/*
#ifdef _WIN32
#else
//    signal(SIGTERM, qhdl);
//    signal(SIGINT, qhdl);
//    signal(SIGTSTP, qhdl);
//    signal(SIGCHLD, qhdl_SIGCHLD);

//    signal(SIGPIPE, qhdl_PIPE);
#endif
*/
    //-- threads


    uv_signal_init(uv_default_loop(), &sig1);
    uv_signal_init(uv_default_loop(), &sig2);
    uv_signal_init(uv_default_loop(), &sig3);
    uv_signal_init(uv_default_loop(), &sig4);
    uv_signal_init(uv_default_loop(), &sig5);
    uv_signal_init(uv_default_loop(), &sig6);

    uv_signal_start(&sig1, signal_handler, SIGTERM);
    uv_signal_start(&sig2, signal_handler, SIGINT);
//    uv_signal_start(&sig3, signal_handler, SIGTSTP);
//    uv_signal_start(&sig4, signal_handlerH, SIGCHLD);
//    uv_signal_start(&sig5, signal_handlerP, SIGPIPE);

    uv_signal_start(&sig6, signal_handlerP, SIGKILL);

    tprc = uv_thread_self();

    int tracklen = 10;
    uv_thread_t ktThread_id;
    uv_thread_create(&ktThread_id, ktThread, &tracklen);

    uv_thread_t getsTrun_id;
    uv_thread_create(&getsTrun_id, getsTrun, 0);

//    uv_thread_join(&ktThread_id);

    //--
/*
    int lump_of_memory = (kPointerSize / 4) * MB;
    v8::ResourceConstraints constraints;
    constraints.set_max_young_space_size(16 * lump_of_memory);
    constraints.set_max_old_space_size(1000 * lump_of_memory);
//    constraints.set_max_executable_size(256 * lump_of_memory);

//    v8::SetResourceConstraints(&constraints);

//    fprintf(stderr,"max_old_space_size: %d\n",constraints.max_old_space_size());fflush(stderr);

    v8::HandleScope handle_scope;
    context = v8::Context::New();
    v8::Context::Scope context_scope(context);
    v8::TryCatch trycatch;

    Handle<Object> global = context->Global();
    Handle<Array> ar = Array::New();
    for( int i = 0; i < argc; ++i) {
        ar->Set(v8::Number::New(i),String::New(argv[i]));
    }
    global->Set(String::New("ARGV"), ar);

    js_init();

    mc_init();
*/

    char *cnt = get_file_text_utf8("ef_cnt",0);
    if( cnt){
        node_cnt = atoll(cnt);
        free( cnt);
    }

    char ids[100];

//    getTID(ids);
//    std::cerr << "ноде:\n" << node_id << ' ' << ids <<"\n\n";


    char *inf = get_file_text_utf8(f_in,0);

    void ef_1exec(char *se);

    if( inf) {
//        std::cerr << "\n" << inf << "\n\n";
        std::cerr.flush();

        ef_1exec( inf);
        /*
//        v8::Handle<v8::Value> result = v8::Script::Compile(v8::String::New(inf))->Run();
        Local<Value> result;
        Local<Script> script = Script::Compile(v8::String::New(inf)); // компилируем
        if (trycatch.HasCaught()) {
            String::Utf8Value err(trycatch.Message()->Get());
            fprintf(stderr,"COMPILE ERROR: %s\n",*err);fflush(stderr);
            String::Utf8Value exc(trycatch.Exception());
            fprintf(stderr,"COMPILE ERROR - EXEPTION: %s\n",*exc);fflush(stderr);
            String::Utf8Value st(trycatch.StackTrace());
            fprintf(stderr,"COMPILE ERROR - STACK: %s\n",*st);fflush(stderr);
            goto oop;
        }
        result = script->Run(); // выполняем
        if (trycatch.HasCaught()) {
            String::Utf8Value err(trycatch.Message()->Get());
            fprintf(stderr,"RUN ERROR: %s\n",*err);fflush(stderr);
            String::Utf8Value exc(trycatch.Exception());
            fprintf(stderr,"RUN ERROR - EXEPTION: %s\n",*exc);fflush(stderr);
            String::Utf8Value st(trycatch.StackTrace());
            fprintf(stderr,"RUN ERROR - STACK: %s\n",*st);fflush(stderr);
        }
        */
oop:
        free(inf);
    }

    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
    uv_loop_close(uv_default_loop());

//    context.Dispose();

#ifdef _WIN32
    WSACleanup();
#endif
    wrCTN();
    return 0;
}

