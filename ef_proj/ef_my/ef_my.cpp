#include "ef_my.h"
#include <mysql/mysql.h>

std::string LD_PREF = "my.";
#define LD_ID 112

std::string *newUUID();
void delStrEf(std::string *s);
void getTID(char *s);
std::string tp_f(int i);
std::string *toUL_f(char *s,int sl,int flg);

void delete_f(void *vv)
{
//    fprintf(stderr,"DEL_FNC MY! %d %ld\n",LD_ID,vv); fflush(stderr);

    MYSQL *o = (MYSQL *) vv;

    if( o) {
//                fprintf(stderr,"MYSQL_ID delete %ld\n",o);fflush(stderr);
        mysql_close(o);
    }

}

hVal* myEnd_f(ef_Content &ec){

    mysql_library_end();

    return 0;
}

hVal* myRows_f(ef_Content &ec){

//    mysql_library_end();
    if( ec.sd->size() == 0) return 0;

    hVal *b = ec.sd->back();
    if( b->b.tp != LD_ID) return 0;

    MYSQL * conn = (MYSQL *)b->v.vl;
//    double cnt = (double)mysql_affected_rows(ro);

    ec.sP(ec.ef.new_hVal(hVal::DOUBLE,0,(double)mysql_affected_rows(conn)));

    return 0;
}

hVal* mySql_f(ef_Content &ec){

    MYSQL *conn = 0;
    std::string *ps=0;
    hVal* rv = ec.gN();

    if( ec.sd->size() < 2) return 0;

    hVal *v = ec.sd->back();
    hVal *db = ec.sd->at(ec.sd->size()-2);

    if( db->b.tp != LD_ID) goto end;

    if(v->b.tp != hVal::STRING && v->b.tp != hVal::CSTRING) goto end;

    conn = (MYSQL *)db->v.vl;

    ps = toUL_f((char*)v->v.s->c_str(),v->v.s->size(),0);

//    fprintf(stderr,"MYSQL SQL 1\n");fflush(stderr);


    if( mysql_query(conn,(char*)v->v.s->c_str()) != 0)  {
        fprintf(stderr,"MYSQL SQL ERROR: %s\n%s\n",(const char*)mysql_error(conn),(char*)v->v.s->c_str());fflush(stderr);
        goto end;
    }

//    fprintf(stderr,"MYSQL SQL 2\n");fflush(stderr);

    if( (ps->find("update") == 0) || (ps->find("insert") == 0)) {
        rv = ec.ef.new_hVal(hVal::DOUBLE,0,(double)mysql_affected_rows(conn));
        goto end;
    }

    if( ps->find("select") == 0) {
//        fprintf(stderr,"MYSQL SQL select 0\n");fflush(stderr);
        MYSQL_RES *result = mysql_store_result(conn);

        unsigned int num_fields;
        unsigned int i;
        MYSQL_FIELD *fields;

        num_fields = mysql_num_fields(result);
        fields = mysql_fetch_fields(result);

        for(i = 0; i < num_fields; i++)
        {
           printf("Field %u is %s\n", i, fields[i].name);
        }


//        Handle<Array> ar = Array::New();

        rv = ec.ef.new_hVal(hVal::DEQ,0,0);

        MYSQL_ROW row;
//        unsigned int j=0;

        while ((row = mysql_fetch_row(result)))
        {
           unsigned long *lengths;
           lengths = mysql_fetch_lengths(result);

           hVal *e = ec.ef.new_hVal(hVal::DEQ,0,0);

           for(i = 0; i < num_fields; i++)
           {
                if( !row[i]) {
                    e->_push(ec.gN());
                    continue;
                }

                switch(fields[i].type) {
/*
                case MYSQL_TYPE_DECIMAL:
                case MYSQL_TYPE_TINY:
                case MYSQL_TYPE_SHORT:
                case MYSQL_TYPE_LONG:

                case MYSQL_TYPE_LONGLONG:
                case MYSQL_TYPE_INT24:
                case MYSQL_TYPE_NEWDECIMAL:
                case MYSQL_TYPE_ENUM:
                case MYSQL_TYPE_SET:
//                    printf("int %s\n",fields[i].name);

//                    obj->Set(v8::String::NewSymbol(fields[i].name),Number::New(atoll(row[i])));
                    e->_push(ec.gN());

                    break;
*/
                case MYSQL_TYPE_FLOAT:
                case MYSQL_TYPE_DOUBLE:

                    e->_push(ec.ef.new_hVal(hVal::DOUBLE,0,atof(row[i])));

                    break;

                default:
                    if( !strcmp(fields[i].name,"id")){

                        char s[1000], *end;
                        unsigned long long z = strtouq(row[i], &end, 10);

                        sprintf(s,"%016LX",z);
//                        sprintf(s,"%lu",z);

                        e->_push(ec.ef.new_hVal(hVal::STRING,ec.ef.new_str(s),0));
                    }else{
                        e->_push(ec.ef.new_hVal(hVal::STRING,ec.ef.new_str(row[i]),0));
                    }

                    break;
                }
           }

           rv->_push(e);

//           ++j;
        }

        mysql_free_result(result);
    }

end:

    if( ps) {
        delStrEf(ps);
    }

    delete v;
    ec.sB(rv);

    return 0;
}

hVal* myNew_f(ef_Content &ec){

//    fprintf(stderr,"mcNew_f ------------- !!!\n"); fflush(stderr);

    hVal *host = 0;//host,user,passwd,db,port?
    hVal *user = 0;
    hVal *passwd = 0;
    hVal *db = 0;
    int port = 3306;
    HHDeqV_T *q  = 0;

    hVal *b = ec.sd->back();
    hVal *rv = ec.gN();

    if( b->b.tp != hVal::DEQ && b->b.tp != hVal::CDEQ){
        goto end;
    }

    q  = b->v.q;
    if( q->size() < 4) goto end;

    host = q->at(0);//host,user,passwd,db,port?
    user = q->at(1);
    passwd = q->at(2);
    db = q->at(3);

    if( host->b.tp != hVal::STRING && host->b.tp != hVal::CSTRING) goto end;
    if( user->b.tp != hVal::STRING && user->b.tp != hVal::CSTRING) goto end;
    if( passwd->b.tp != hVal::STRING && passwd->b.tp != hVal::CSTRING) goto end;
    if( db->b.tp != hVal::STRING && db->b.tp != hVal::CSTRING) goto end;

    if( q->size() > 4) {
        hVal *pt = q->at(4);
        if( pt->b.tp == hVal::DOUBLE || pt->b.tp == hVal::CDOUBLE) port = (int)pt->v.d;
    }

    MYSQL 	*conn, *rconn;

    conn = mysql_init(NULL);

    if(conn == NULL)
    {
        mysql_close(conn);
        goto end;
    }

    if( !( rconn = mysql_real_connect(conn,
                            host->v.s->c_str(),
                            user->v.s->c_str(),
                            passwd->v.s->c_str(),
                            db->v.s->c_str(),
                            port,
                            0,
                            0
                            )) )
    {
        fprintf(stderr,"!! mysql_real_connect:: %s>>\n",mysql_error(conn));fflush(stderr);
        mysql_close(conn);
        goto end;
    }

    rv = ec.ef.new_hVal(LD_ID,conn,0);

end:
    delete b;

//    ec.sd->back() = ec.ef.new_hVal(LD_ID,conn,0);
    ec.sd->back() = rv;
    return 0;
}

void init(ef_Content &ec)
{
//    std::cerr << "\n" << "inf INIT MY" << "\n\n"; std::cerr.flush();

    ec.wl();

    ec.dI(LD_ID, delete_f);

    ec.fI(LD_PREF + "N",ec.ef.new_hVal(hVal::FNCC,(void*)myNew_f,0));
    ec.fI(LD_PREF + "E",ec.ef.new_hVal(hVal::FNCC,(void*)myEnd_f,0));
    ec.fI(LD_PREF + "R",ec.ef.new_hVal(hVal::FNCC,(void*)myRows_f,0));
    ec.fI(LD_PREF + "S",ec.ef.new_hVal(hVal::FNCC,(void*)mySql_f,0));

    ec.wu();
}

