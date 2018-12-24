//#include "../ef.hpp"
#include "ef_curl.h"
/*
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include <unistd.h>
#include <stddef.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>


//#include <stdint.h>
//#include <netinet/in.h>

#ifndef _WIN32b
#include <stdint.h>
#include <unistd.h>
#include <netinet/in.h>
#define stricmp strcasecmp
#define strcmpi strcasecmp
#define strnicmp strncasecmp
#include <netdb.h>
#include <sys/socket.h>
#endif

#include <curl/curl.h>
#include <zlib.h>
*/
char * get_curl_url(char *ser_url,int *sz,char **hd,int *hsz);


#include "../../ua.h"

//extern CURL *curl_handle;

/*
typedef struct MemoryStruct {
  char *memory;
  size_t size;
} MemoryStructT;
*/

std::string LD_PREF = "curl.";
//#define LD_ID 113

std::string *newUUID();
void delStrEf(std::string *s);
void getTID(char *s);
std::string tp_f(int i);
std::string *toUL_f(char *s,int sl,int flg);

char *hh_strrstr(char *string, char *find)
{
    size_t stringlen, findlen;
    char *cp;

    findlen = strlen(find);
    stringlen = strlen(string) - findlen;
    if( stringlen < 0) stringlen = 0;

    if (findlen > stringlen)
        return NULL;

    for (cp = string + stringlen - findlen; cp >= string; cp--)
        if (strncmp(cp, find, findlen) == 0)
            return cp;

    return NULL;
}

static size_t WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)data;

  mem->memory = (char *)realloc(mem->memory, mem->size + realsize + 1);
  if (mem->memory == NULL) {
    /* out of memory! */
    printf("not enough memory (realloc returned NULL)\n");
    exit(EXIT_FAILURE);
  }

  memcpy(&(mem->memory[mem->size]), ptr, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}

int curl_set_timeout = 20;
int curl_set_connection_timeout = 10;
int curl_set_dns_timeout = 10;
int curl_set_low_speed_limit = 0;
int curl_set_low_speed_time = 0;
int curl_set_max_red = 6;
char cookiestring[5000] = "";
int curl_set_max_thread = 20;

char curl_set_proxystring[5000] = "";
char curl_set_proxytype[5000] = "";
static int ua_flg = 0;

CURL *curl_hd = 0;
static int curl_cnt = 0;
#define curl_max_urls  300

void hh_curl_set_opt(CURL *curl_handle)
{
    int cur_ua = 0;

    //---

    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0L) ;
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L) ;
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYSTATUS, 0L) ;
    curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 0L);


    /* enable TCP keep-alive for this transfer */
    curl_easy_setopt(curl_handle, CURLOPT_TCP_KEEPALIVE, 1L);

    /* keep-alive idle time to 120 seconds */
    curl_easy_setopt(curl_handle, CURLOPT_TCP_KEEPIDLE, 120L);

    /* interval time between keep-alive probes: 60 seconds */
    curl_easy_setopt(curl_handle, CURLOPT_TCP_KEEPINTVL, 60L);

    /* send all data to this function  */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, WriteMemoryCallback);

    if (strlen(cookiestring) > 0) {
        curl_easy_setopt(curl_handle, CURLOPT_COOKIE, cookiestring);
//        curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, cookiestring );
//        curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE, strlen(cookiestring));
    }

    if (strlen(curl_set_proxystring) > 0) {

        curl_easy_setopt(curl_handle,CURLOPT_PROXY,curl_set_proxystring);
        if (strcmp(curl_set_proxytype,"socks5h") == 0) {
            curl_easy_setopt(curl_handle,CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5_HOSTNAME);
        } else if (strcmp(curl_set_proxytype,"socks5") == 0) {
            curl_easy_setopt(curl_handle,CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5);
        } else if (strcmp(curl_set_proxytype,"socks4") == 0) {
            curl_easy_setopt(curl_handle,CURLOPT_PROXYTYPE, CURLPROXY_SOCKS4);
        }
    }

    curl_easy_setopt(curl_handle, CURLOPT_AUTOREFERER, 1);
    curl_easy_setopt(curl_handle, CURLOPT_ACCEPT_ENCODING, "");

    curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
    //curl_easy_setopt(curl_handle, CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTP | CURLPROTO_HTTPS) ;
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L) ;
    curl_easy_setopt(curl_handle, CURLOPT_MAXREDIRS, curl_set_max_red) ;

    /* some servers don't like requests that are made without a user-agent
       field, so we provide one */

    cur_ua = ((curl_cnt++)/curl_max_urls) % uaSize;
//    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.11 (KHTML, like Gecko) Chrome/23.0.1271.91 Safari/537.11");
    if( ua_flg) curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, ua[cur_ua]);


    curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, curl_set_timeout);

    if( curl_set_connection_timeout) curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT, curl_set_connection_timeout);
    if( curl_set_dns_timeout) curl_easy_setopt(curl_handle, CURLOPT_DNS_CACHE_TIMEOUT, curl_set_dns_timeout);

    if( curl_set_low_speed_limit) curl_easy_setopt(curl_handle, CURLOPT_LOW_SPEED_LIMIT, curl_set_low_speed_limit);
    if( curl_set_low_speed_time) curl_easy_setopt(curl_handle, CURLOPT_LOW_SPEED_TIME, curl_set_low_speed_time);

}

char * get_curl_url(char *ser_url,int *sz,char **hd,int *hsz)
{
    CURL *curl_handle;
    CURLcode res;
    int cur_ua = 0;

    struct MemoryStruct chunk;

    chunk.memory = (char *)malloc(1);  /* will be grown as needed by the realloc above */
    chunk.memory[0] = 0;
    chunk.size = 0;    /* no data at this point */

    struct MemoryStruct chunk_h;

    chunk_h.memory = (char *)malloc(1);  /* will be grown as needed by the realloc above */
    chunk_h.memory[0] = 0;
    chunk_h.size = 0;    /* no data at this point */

    /* init the curl session */
//    curl_global_init(CURL_GLOBAL_ALL);

    if( !curl_hd) {
        curl_hd = curl_handle = curl_easy_init();
    }else{
        curl_handle = curl_hd;
    }

    curl_easy_setopt( curl_handle, CURLOPT_URL, ser_url);

    hh_curl_set_opt( curl_handle);

    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEHEADER, (void *)&chunk_h);

    /* get it! */
    res = curl_easy_perform(curl_handle);

    /* check for errors */
     if(res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %d %s\n", res, curl_easy_strerror(res));

        chunk.memory[0] = 0;
        chunk_h.memory[0] = 0;
        chunk.size = 0;
        chunk_h.size = 0;
     }

    /* cleanup curl stuff */

//    curl_easy_cleanup(curl_handle);

//    curl_global_cleanup();

/*
    char *utf_p = strstr(chunk.memory,"charset=");
    if(utf_p) {
        if( !strnicmp(utf_p+8,"utf-8",5) ) {
            char *new_body = rd(QString::fromUtf8(chunk.memory));
            free(chunk.memory);
            chunk.size = strlen(new_body);
            chunk.memory = (char *)malloc(chunk.size + 1);
            strcpy(chunk.memory,new_body);
        }
    }
*/
    if( sz) {
        *sz = chunk.size;
    }

    if( hsz) {
        *hsz = chunk_h.size;
    }

    if( hd) {
        *hd = chunk_h.memory;
    }else {
        free(chunk_h.memory);
    }
/*
    if(chunk_h.size >0) {
        printf("HEADER:\n{%s}\n",chunk_h.memory);
    }
*/

    return (chunk.memory);
}

//--------------------

UrlReciveP get_curl_url_new(char *ser_url)
{
    CURL *curl_handle;
    CURLcode res;
    int cur_ua = 0;

    UrlRecive *r = new UrlRecive( ser_url,0);

    if( !curl_hd) {
        curl_hd = curl_handle = curl_easy_init();
    }else{
        curl_handle = curl_hd;
    }

    curl_easy_setopt( curl_handle, CURLOPT_URL, ser_url);
    hh_curl_set_opt( curl_handle);

    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&r->b);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEHEADER, (void *)&r->h);

    /* get it! */
    res = curl_easy_perform(curl_handle);

    long response_code;
    curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &response_code);
    r->response_code = response_code;
    r->result = res;

    char *urlp = NULL;
    curl_easy_getinfo(curl_handle, CURLINFO_EFFECTIVE_URL, &urlp);
    if( urlp) r->eurl = urlp;

    char *ctype = NULL;
    curl_easy_getinfo(curl_handle, CURLINFO_CONTENT_TYPE, &ctype);
    if( ctype) r->ctype = ctype;


    return ( r);
}

//--------------------

static void hhcurle_init(CURLM *cm, UrlRecive* d)
{
//  int cur_ua = 0;

  CURL *eh = curl_easy_init();

  curl_easy_setopt(eh, CURLOPT_URL, d->url.c_str());

  hh_curl_set_opt( eh);

  curl_easy_setopt(eh, CURLOPT_WRITEDATA, (void *)&d->b);
  curl_easy_setopt(eh, CURLOPT_WRITEHEADER, (void *)&d->h);
  curl_easy_setopt(eh, CURLOPT_PRIVATE, d);

  if( d->map){
      HHMapSV_T *m = d->map;

      HHMapSV_TI it = m->find("c");

      if( it != m->end()) {
          hVal *qi = it->second;
          if( qi->b.tp == hVal::STRING || qi->b.tp == hVal::CSTRING){
              curl_easy_setopt(eh, CURLOPT_COOKIE, qi->v.s->c_str());
          }
      }

      it = m->find("p");

      if( it != m->end()) {
          hVal *qi = it->second;
          if( qi->b.tp == hVal::STRING || qi->b.tp == hVal::CSTRING || qi->b.tp == hVal::BUF){
              curl_easy_setopt(eh, CURLOPT_POST, 1);
              curl_easy_setopt(eh, CURLOPT_POSTFIELDS, qi->v.s->c_str());
              curl_easy_setopt(eh, CURLOPT_POSTFIELDSIZE, qi->v.s->size());
          }
      }

      it = m->find("h");

      if( it != m->end()) {
          hVal *qi = it->second;
          if( qi->b.tp == hVal::DEQ || qi->b.tp == hVal::CDEQ){
              HHDeqV_T *q = qi->v.q;
              struct curl_slist *headers=NULL; // init to NULL is important

              HHDeqV_TI it = q->begin();
              for( ; it != q->end(); ++it){
                  hVal *tv = *it;

                  if( tv->b.tp == hVal::STRING || tv->b.tp == hVal::CSTRING){
                      headers = curl_slist_append(headers, tv->v.s->c_str());
                  }
              }

              if( headers) curl_easy_setopt(eh, CURLOPT_HTTPHEADER, headers);
          }
      }
  }

  curl_multi_add_handle(cm, eh);
}

//std::vector<UrlReciveP> *hh_curl_mul(std::vector<std::string> a, int mth){
std::vector<UrlReciveP> *hh_curl_mul(std::vector<UrlReciveP> *r, int mth){
/*
    std::vector<UrlReciveP> *r = new std::vector<UrlReciveP>();

    for( int n =0; n < a.size(); ++n ) {
//        std::cout << a[n] << '\n';

        r->push_back( new UrlRecive(a[n],n));
    }
*/
//    int ccc = 0;

    CURLM *cm;
    CURLMsg *msg;
    long L;
    unsigned int C=0;
    int M, Q, U = -1;
    fd_set R, W, E;
    struct timeval T;

    cm = curl_multi_init();

    /* we can optionally limit the total amount of connections this multi handle
       uses */
    curl_multi_setopt(cm, CURLMOPT_MAXCONNECTS, (long)mth);

    for(C = 0; (C < mth) && (C < r->size()); ++C) {
      hhcurle_init(cm, r->at(C));
    }

    while(U) {
      curl_multi_perform(cm, &U);

      if(U) {
        FD_ZERO(&R);
        FD_ZERO(&W);
        FD_ZERO(&E);

        if(curl_multi_fdset(cm, &R, &W, &E, &M)) {
          fprintf(stderr, "E: curl_multi_fdset\n");
          return r;
        }

        if(curl_multi_timeout(cm, &L)) {
          fprintf(stderr, "E: curl_multi_timeout\n");
          return r;
        }
        if(L == -1)
          L = 100;

        if(M == -1)  {
  #ifdef WIN32
          Sleep(L);
  #else
          sleep((unsigned int)L / 1000);
  #endif
        }
        else {
          T.tv_sec = L/1000;
          T.tv_usec = (L%1000)*1000;

          if(0 > select(M+1, &R, &W, &E, &T)) {
            fprintf(stderr, "E: select(%i,,,,%li): %i: %s\n",
                M+1, L, errno, strerror(errno));
            return r;
          }
        }
      }

      while((msg = curl_multi_info_read(cm, &Q))) {
        if(msg->msg == CURLMSG_DONE) {
//          int d;
          UrlRecive *d;
          CURL *e = msg->easy_handle;
          curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &d);

          long response_code;
          curl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE, &response_code);
          d->response_code = response_code;

//          r->at((int)d)->result = msg->data.result;
          d->result = msg->data.result;

          char *urlp = NULL;
          curl_easy_getinfo(msg->easy_handle, CURLINFO_EFFECTIVE_URL, &urlp);
          if( urlp) d->eurl = urlp;

          char *ctype = NULL;
          curl_easy_getinfo(msg->easy_handle, CURLINFO_CONTENT_TYPE, &ctype);
          if( ctype) d->ctype = ctype;


//          fprintf(stderr, "RRR: %d %d %d %d - %s <%s>\n",d->cnt,r->size(),ccc++,
//                  msg->data.result, curl_easy_strerror(msg->data.result),  d->url.c_str()); // d->url.c_str());
          curl_multi_remove_handle(cm, e);
          curl_easy_cleanup(e);
        }
        else {
          fprintf(stderr, "E: CURLMsg (%d)\n", msg->msg);
        }
        if(C < r->size()) {
          hhcurle_init(cm, r->at(C++));
          U++; /* just to prevent it from remaining at 0 if there are more
                  URLs to get */
        }
      }
    }

    curl_multi_cleanup(cm);

    return r;
}

//---------------->

hVal* curlSetOptions_f(ef_Content &ec){
    cookiestring[0] = 0;
    curl_set_proxystring[0] = 0;
    curl_set_proxytype[0] = 0;

    if( ec.sd->size() == 0) return 0;

    hVal *b = ec.sd->back();

    if( b->b.tp != hVal::MAP && b->b.tp != hVal::CMAP) goto end;

    {
        HHMapSV_T *m = b->v.m;

        HHMapSV_TI it = m->find("ua");

        if( it != m->end()) {
            hVal *v = it->second;
            if( v->b.tp == hVal::TRUE){
                ua_flg = 1;
            }else{
                ua_flg = 0;
            }
        }

        it = m->find("ps");

        if( it != m->end()) {
            hVal *v = it->second;
            if( v->b.tp == hVal::STRING || v->b.tp == hVal::CSTRING){
                strcpy(curl_set_proxystring,v->v.s->c_str());
            }
        }

        it = m->find("pt");

        if( it != m->end()) {
            hVal *v = it->second;
            if( v->b.tp == hVal::STRING || v->b.tp == hVal::CSTRING){
                strcpy(curl_set_proxytype,v->v.s->c_str());
            }
        }

        it = m->find("pt");

        if( it != m->end()) {
            hVal *v = it->second;
            if( v->b.tp == hVal::STRING || v->b.tp == hVal::CSTRING){
                strcpy(curl_set_proxytype,v->v.s->c_str());
            }
        }

        it = m->find("t");

        if( it != m->end()) {
            hVal *v = it->second;
            if( v->b.tp == hVal::DOUBLE || v->b.tp == hVal::CDOUBLE){
                curl_set_timeout = v->v.d;
            }
        }

        it = m->find("tc");

        if( it != m->end()) {
            hVal *v = it->second;
            if( v->b.tp == hVal::DOUBLE || v->b.tp == hVal::CDOUBLE){
                curl_set_connection_timeout = v->v.d;
            }
        }

        it = m->find("tdns");

        if( it != m->end()) {
            hVal *v = it->second;
            if( v->b.tp == hVal::DOUBLE || v->b.tp == hVal::CDOUBLE){
                curl_set_dns_timeout = v->v.d;
            }
        }

        it = m->find("lsl");

        if( it != m->end()) {
            hVal *v = it->second;
            if( v->b.tp == hVal::DOUBLE || v->b.tp == hVal::CDOUBLE){
                curl_set_low_speed_limit = v->v.d;
            }
        }

        it = m->find("lst");

        if( it != m->end()) {
            hVal *v = it->second;
            if( v->b.tp == hVal::DOUBLE || v->b.tp == hVal::CDOUBLE){
                curl_set_low_speed_time = v->v.d;
            }
        }

        it = m->find("mr");

        if( it != m->end()) {
            hVal *v = it->second;
            if( v->b.tp == hVal::DOUBLE || v->b.tp == hVal::CDOUBLE){
                curl_set_max_red = v->v.d;
            }
        }

        it = m->find("mt");

        if( it != m->end()) {
            hVal *v = it->second;
            if( v->b.tp == hVal::DOUBLE || v->b.tp == hVal::CDOUBLE){
                curl_set_max_thread = v->v.d;
            }
        }

    }

end:

    delete b;
    ec.sPB();

    return 0;
}

hVal* curlFind_f(ef_Content &ec){
    std::vector<UrlReciveP> *r = NULL;

    if( ec.sd->size() == 0) return 0;

    hVal *rv = ec.ef.new_hVal(hVal::DEQ,0,0);

//    fprintf(stderr,"curlFind_f!\n");fflush(stderr);

    hVal *v = ec.sd->back();

    if( v->b.tp != hVal::DEQ && v->b.tp != hVal::CDEQ && v->b.tp != hVal::STRING && v->b.tp != hVal::CSTRING) goto end;

    r = new std::vector<UrlReciveP>();

    if( v->b.tp == hVal::STRING || v->b.tp == hVal::CSTRING){
        r->push_back( new UrlRecive( *v->v.s,0));
    }else if( v->b.tp == hVal::DEQ || v->b.tp == hVal::CDEQ){

        HHDeqV_T *q = v->v.q;
        HHDeqV_TI it = q->begin();
        int i=0;
        for(i=0; it != q->end(); ++it){
            hVal *tv = *it;

            if( tv->b.tp == hVal::STRING || tv->b.tp == hVal::CSTRING){
                r->push_back( new UrlRecive( *tv->v.s,i));
                ++i;
                continue;
            }

            if( tv->b.tp == hVal::MAP || tv->b.tp == hVal::CMAP){
                HHMapSV_T *m = tv->v.m;

                HHMapSV_TI it = m->find("url");

                if( it != m->end()) {
                    hVal *qi = it->second;
                    if( qi->b.tp == hVal::STRING || qi->b.tp == hVal::CSTRING){
                        UrlRecive *ur = new UrlRecive( *qi->v.s,i);
                        ur->map = m;
                        r->push_back( ur);
                        ++i;
                        continue;
                    }
                }
            }
        }

    }

    {
        int length = r->size();
        int mth = (length > curl_set_max_thread) ? curl_set_max_thread : length;

        std::vector<UrlRecive*> *rr = hh_curl_mul(r, mth);

        int outc = 0;

        for( int n =0; n < rr->size(); ++n ) {

            UrlRecive *r = rr->at(n);

            if( r->result != CURLE_OK || !(r->response_code >= 200 && r->response_code < 300)) {
    //            std::cout << n <<" "<< r->url << " res: " << r->result << " response_code " << r->response_code <<" -OOUT- \n";
                delete r;
                continue;
            }

            hVal *em = ec.ef.new_hVal(hVal::MAP,0,0);

            em->setMV("b",ec.ef.new_hVal(hVal::STRING,ec.ef.new_str(r->b.memory,r->b.size),0));

            char *p = r->h.memory;
            char *pp = hh_strrstr( p,"\x0D\x0A\x0D\x0A");
            if( pp) {
                p = pp + 4;
            }else if( pp = hh_strrstr( p,"\x0A\x0A")){
                p = pp + 2;
            }

            em->setMV("h",ec.ef.new_hVal(hVal::STRING,ec.ef.new_str(p),0));
            em->setMV("hh",ec.ef.new_hVal(hVal::STRING,ec.ef.new_str(r->h.memory),0));

            em->setMV("url",ec.ef.new_hVal(hVal::STRING,ec.ef.new_str(r->url),0));
            em->setMV("eurl",ec.ef.new_hVal(hVal::STRING,ec.ef.new_str(r->eurl),0));
            em->setMV("ctype",ec.ef.new_hVal(hVal::STRING,ec.ef.new_str(r->ctype),0));
            em->setMV("n",ec.ef.new_hVal(hVal::DOUBLE,0,r->cnt));

            rv->_push(em);

            delete r;

            ++outc;
        }
        delete r;
    }

end:

    delete v;
    ec.sB(rv);

    return 0;
}

void init(ef_Content &ec)
{
//    std::cerr << "\n" << "inf INIT CURL" << "\n\n"; std::cerr.flush();

    ec.wl();

    ec.fI(LD_PREF + "F",ec.ef.new_hVal(hVal::FNCC,(void*)curlFind_f,0));
    ec.fI(LD_PREF + "SO",ec.ef.new_hVal(hVal::FNCC,(void*)curlSetOptions_f,0));

    ec.wu();
}
