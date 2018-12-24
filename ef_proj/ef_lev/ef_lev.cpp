#include "ef_lev.h"

//#define LD_PREF "lv."
std::string LD_PREF = "lv.";
#define LD_ID 110
//#define LD_PREF "lv."
std::string db_home_dir = "./leveldb/";//"./db/";

static char* async_s = "---AutomaticSync---";

std::string *newUUID();
void delStrEf(std::string *s);
void getTID(char *s);

lComparator *lcmp =  0;

void lv_del_f(void *vv)
{
    fprintf(stderr,"DEL_FNC lv_del_f %ld!!!\n",vv); fflush(stderr);

    efKV_C *v = (efKV_C *)vv;

    if( v) {
        if( v->db) {
            delete v->db;

            delete v->db_op.block_cache;
            delete v->db_op.filter_policy;
        }

        if( v->lg) {
            delete v->lg;

            delete v->lg_op.block_cache;
//            delete v->lg_op.filter_policy;
        }

        delete v;
    }
}

hVal* lvRepair_f(ef_Content &ec){

//    fprintf(stderr,"lvNew_f ------------- !!!\n"); fflush(stderr);

    hVal *b = ec.sd->back();

    int c_sz = 50;
    int b_sz = 4096*2;
    std::string nm = "ef";
    std::string hd = db_home_dir;

    if( b->b.tp == hVal::STRING || b->b.tp == hVal::CSTRING){
        nm = *b->v.s;
    }else if(b->b.tp == hVal::DEQ || b->b.tp == hVal::CDEQ){
        HHDeqV_T *q  = b->v.q;

        if( q->size() > 0){
            hVal * qe = q->at(0);
            if( qe->b.tp == hVal::STRING || qe->b.tp == hVal::CSTRING){
                nm = *qe->v.s;
            }
        }

        if( q->size() > 1){
            hVal * qe = q->at(1);
            if( qe->b.tp == hVal::STRING || qe->b.tp == hVal::CSTRING){
                hd = *qe->v.s;
            }
        }

        if( q->size() > 2){
            hVal * qe = q->at(2);
            if( qe->b.tp == hVal::DOUBLE || qe->b.tp == hVal::CDOUBLE){
                c_sz = qe->v.d;
            }
        }

        if( q->size() > 3){
            hVal * qe = q->at(3);
            if( qe->b.tp == hVal::DOUBLE || qe->b.tp == hVal::CDOUBLE){
                b_sz = 1024 * qe->v.d;
            }
        }


    }

    efKV_C *v = new efKV_C();

    std::string fnm = hd + nm;
    v->nm = nm;
    v->fnm = fnm;

//--

    int pr = 0755;
#ifdef _WIN32
    _mkdir(hd.c_str());
#else
    mkdir(hd.c_str(),pr);
#endif

    v->db_op.create_if_missing = true;
    v->db_op.block_cache = leveldb::NewLRUCache(c_sz * 1048576);
    v->db_op.filter_policy = leveldb::NewBloomFilterPolicy(10);
    v->db_op.block_size = b_sz;
    v->db_op.compression = leveldb::kSnappyCompression;

    leveldb::Status s = leveldb::RepairDB(fnm,v->db_op);

    delete v->db_op.block_cache;
    delete v->db_op.filter_policy;

    if( nm[0] == '_'){
        v->lg_op.create_if_missing = true;
        v->lg_op.block_cache = leveldb::NewLRUCache(2 * 1048576);
//        v->lg_op.filter_policy = leveldb::NewBloomFilterPolicy(10);
        v->lg_op.block_size = 4096;
        v->lg_op.compression = leveldb::kSnappyCompression;

    //    options.paranoid_checks = true;
        std::string lgn = fnm + "/lg";

        leveldb::Status s = leveldb::RepairDB(lgn,v->lg_op);

        delete v->lg_op.block_cache;
    }

    delete v;
    delete b;

    ec.sPB();
    return 0;
}

hVal* lvDestroy_f(ef_Content &ec){

//    fprintf(stderr,"lvNew_f ------------- !!!\n"); fflush(stderr);

    hVal *b = ec.sd->back();

    int c_sz = 50;
    int b_sz = 4096*2;
    std::string nm = "ef";
    std::string hd = db_home_dir;

    if( b->b.tp == hVal::STRING || b->b.tp == hVal::CSTRING){
        nm = *b->v.s;
    }else if(b->b.tp == hVal::DEQ || b->b.tp == hVal::CDEQ){
        HHDeqV_T *q  = b->v.q;

        if( q->size() > 0){
            hVal * qe = q->at(0);
            if( qe->b.tp == hVal::STRING || qe->b.tp == hVal::CSTRING){
                nm = *qe->v.s;
            }
        }

        if( q->size() > 1){
            hVal * qe = q->at(1);
            if( qe->b.tp == hVal::STRING || qe->b.tp == hVal::CSTRING){
                hd = *qe->v.s;
            }
        }

        if( q->size() > 2){
            hVal * qe = q->at(2);
            if( qe->b.tp == hVal::DOUBLE || qe->b.tp == hVal::CDOUBLE){
                c_sz = qe->v.d;
            }
        }

        if( q->size() > 3){
            hVal * qe = q->at(3);
            if( qe->b.tp == hVal::DOUBLE || qe->b.tp == hVal::CDOUBLE){
                b_sz = 1024 * qe->v.d;
            }
        }


    }

    efKV_C *v = new efKV_C();

    std::string fnm = hd + nm;
    v->nm = nm;
    v->fnm = fnm;

//--

    int pr = 0755;
#ifdef _WIN32
    _mkdir(hd.c_str());
#else
    mkdir(hd.c_str(),pr);
#endif

    v->db_op.create_if_missing = true;
    v->db_op.block_cache = leveldb::NewLRUCache(c_sz * 1048576);
    v->db_op.filter_policy = leveldb::NewBloomFilterPolicy(10);
    v->db_op.block_size = b_sz;
    v->db_op.compression = leveldb::kSnappyCompression;

    leveldb::Status s = leveldb::DestroyDB(fnm,v->db_op);

    delete v->db_op.block_cache;
    delete v->db_op.filter_policy;

    if( nm[0] == '_'){
        v->lg_op.create_if_missing = true;
        v->lg_op.block_cache = leveldb::NewLRUCache(2 * 1048576);
//        v->lg_op.filter_policy = leveldb::NewBloomFilterPolicy(10);
        v->lg_op.block_size = 4096;
        v->lg_op.compression = leveldb::kSnappyCompression;

    //    options.paranoid_checks = true;
        std::string lgn = fnm + "/lg";

        leveldb::Status s = leveldb::DestroyDB(lgn,v->lg_op);

        delete v->lg_op.block_cache;
    }

    delete v;
    delete b;

    ec.sPB();
    return 0;
}


hVal* lvNew_f(ef_Content &ec){

//    fprintf(stderr,"lvNew_f ------------- !!!\n"); fflush(stderr);

    hVal *b = ec.sd->back();

    int c_sz = 50;
    int b_sz = 4096*2;
    std::string nm = "ef";
    std::string hd = db_home_dir;

    if( b->b.tp == hVal::STRING || b->b.tp == hVal::CSTRING){
        nm = *b->v.s;
    }else if(b->b.tp == hVal::DEQ || b->b.tp == hVal::CDEQ){
        HHDeqV_T *q  = b->v.q;

        if( q->size() > 0){
            hVal * qe = q->at(0);
            if( qe->b.tp == hVal::STRING || qe->b.tp == hVal::CSTRING){
                nm = *qe->v.s;
            }
        }

        if( q->size() > 1){
            hVal * qe = q->at(1);
            if( qe->b.tp == hVal::STRING || qe->b.tp == hVal::CSTRING){
                hd = *qe->v.s;
            }
        }

        if( q->size() > 2){
            hVal * qe = q->at(2);
            if( qe->b.tp == hVal::DOUBLE || qe->b.tp == hVal::CDOUBLE){
                c_sz = qe->v.d;
            }
        }

        if( q->size() > 3){
            hVal * qe = q->at(3);
            if( qe->b.tp == hVal::DOUBLE || qe->b.tp == hVal::CDOUBLE){
                b_sz = 1024 * qe->v.d;
            }
        }

    }
    delete b;

    efKV_C *v = new efKV_C();


    std::string fnm = hd + nm;
    v->nm = nm;
    v->fnm = fnm;

//--

    int pr = 0755;
#ifdef _WIN32
    _mkdir(hd.c_str());
#else
    mkdir(hd.c_str(),pr);
#endif

    v->db_op.create_if_missing = true;
    v->db_op.block_cache = leveldb::NewLRUCache(c_sz * 1048576);
    v->db_op.filter_policy = leveldb::NewBloomFilterPolicy(10);
    v->db_op.block_size = b_sz;
    v->db_op.compression = leveldb::kSnappyCompression;

//    options.paranoid_checks = true;

    leveldb::Status status = leveldb::DB::Open(v->db_op, fnm, &v->db);
    if( !status.ok()) {
        fprintf(stderr, "Error OpenLevDb: %s\n", status.ToString().c_str());fflush(stderr);

        delete v->db_op.block_cache;
        delete v->db_op.filter_policy;

        ec.sd->back() = ec.N;

        delete v;

        return 0;
    }

    if( nm[0] == '_'){

        v->lg_op.create_if_missing = true;
        v->lg_op.comparator = lcmp;
        v->lg_op.block_cache = leveldb::NewLRUCache(2 * 1048576);
//        v->lg_op.filter_policy = leveldb::NewBloomFilterPolicy(10);
        v->lg_op.block_size = 4096;
        v->lg_op.compression = leveldb::kSnappyCompression;

    //    options.paranoid_checks = true;
        std::string lgn = fnm + "/lg";

        leveldb::Status status = leveldb::DB::Open(v->lg_op, lgn, &v->lg);
        if( !status.ok()) {
            fprintf(stderr, "Error OpenLevDb: %s\n", status.ToString().c_str());fflush(stderr);

            delete v->db_op.block_cache;
            delete v->db_op.filter_policy;

            delete v->lg_op.block_cache;
//            delete v->lg_op.filter_policy;

            delete  v->db;

            ec.sd->back() = ec.N;

            delete v;

            return 0;
        }

        leveldb::ReadOptions options;

        leveldb::Iterator* iter = v->lg->NewIterator(options);
        iter->SeekToLast();
        if( iter->Valid()){
            leveldb::Slice s = iter->key();
            v->lg_max = *((long*)s.data());
        }

        delete iter;
    }

    {
        leveldb::ReadOptions options;
        leveldb::Iterator* iter = v->db->NewIterator(options);
        iter->SeekToFirst();
        if( iter->Valid()){
            leveldb::Slice s = iter->key();
            v->kf = s.ToString();
        }
        delete iter;
    }

    {
        leveldb::ReadOptions options;
        leveldb::Iterator* iter = v->db->NewIterator(options);
        iter->SeekToLast();
        if( iter->Valid()){
            leveldb::Slice s = iter->key();
            v->kl = s.ToString();
        }
        delete iter;
    }

//    fprintf(stderr, "++++++++!!!!!!+++++++efKV_C kf: (%s), kl: (%s)\n", v->kf.c_str(),v->kl.c_str());fflush(stderr);

//--

    ec.sd->back() = ec.ef.new_hVal(LD_ID,v,0);

    return 0;
}



//--

hVal* lvSet_faf(ef_Content &ec,int af){
    if( ec.sd->size() < 2) return 0;

    leveldb::WriteOptions write_options;
    write_options.sync = false;

    hVal *v = ec.sd->back();
    hVal *db = ec.sd->at(ec.sd->size()-2);
    HHDeqV_T *q;
    efKV_C *kv;
    hVal *q0;
    hVal *q1;

    if( db->b.tp != LD_ID) goto end;
    kv = (efKV_C *)db->v.vl;

    if( v->b.tp != hVal::DEQ && v->b.tp != hVal::CDEQ && v->b.tp != hVal::STRING && v->b.tp != hVal::CSTRING) goto end;

    if( v->b.tp == hVal::STRING || v->b.tp == hVal::CSTRING){
        kv->l.wl();

        leveldb::Status s = kv->db->Put(write_options, *v->v.s, "");

        if( s.ok()) {
            if( kv->kf > *v->v.s) kv->kf = *v->v.s;
            if( kv->kl < *v->v.s) kv->kl = *v->v.s;
        }

        if( s.ok() && kv->lg && af) {
            ++kv->lg_max;

            leveldb::Slice kl((char*)&kv->lg_max,sizeof(long));
            leveldb::Status ss = kv->lg->Put(write_options, kl, *v->v.s);
        }

        kv->l.wu();

        goto end;
    }

    q = v->v.q;
    if( q->size() == 0){goto end;}

/*
    q0 = q->at(0);

    if( q0->b.tp == hVal::STRING || q0->b.tp == hVal::CSTRING){
        std::string k = *q0->v.s;
        std::string vl = "";
        if( q->size() > 1 ){
            q1 = q->at(1);

            if( q1->b.tp == hVal::STRING || q1->b.tp == hVal::CSTRING || q1->b.tp == hVal::BUF){
                vl = *q1->v.s;
            }else{
                ec.ts(q1,vl);
            }
        }

        kv->l.wl();

        leveldb::Status s = kv->db->Put(write_options, k, vl);

        if( s.ok() && kv->lg) {
            ++kv->lg_max;

            leveldb::Slice kl((char*)&kv->lg_max,sizeof(long));
            leveldb::Status ss = kv->lg->Put(write_options, kl, k);
        }

        kv->l.wu();

        goto end;
    }
*/
//    if( q0->b.tp == hVal::DEQ || q0->b.tp == hVal::CDEQ){
{
//        fprintf(stderr,"lvSet_f WriteBatch %d\n",q->size()); fflush(stderr);

        leveldb::WriteBatch b_db;
        leveldb::WriteBatch b_lg;

        std::string kf = kv->kf;
        std::string kl = kv->kl;

        std::vector<std::string> *db_k = new std::vector<std::string>(q->size());
        std::vector<std::string> *db_v = new std::vector<std::string>(q->size());
        std::vector<long> *lg_k = new std::vector<long>(q->size());

        HHDeqV_TI it = q->begin();
        int i=0;
        for(i=0; it != q->end(); ++it){
            hVal *tv = *it;

            if( tv->b.tp == hVal::STRING || tv->b.tp == hVal::CSTRING){
//                db_k->push_back(*tv->v.s);
//                db_v->push_back("");

                db_k->at(i) = (*tv->v.s);
                db_v->at(i) = "";


            }else if( tv->b.tp == hVal::DEQ || tv->b.tp == hVal::CDEQ){
                HHDeqV_T *tq = tv->v.q;
                if( tq->size() == 0) continue;
                hVal *tq0 = tq->at(0);

                if( tq0->b.tp == hVal::STRING || tq0->b.tp == hVal::CSTRING || tq0->b.tp == hVal::NIL){
                    std::string *k;
                    char b[100];
                    if( tq0->b.tp == hVal::NIL){
//                        k = newUUID();
                        getTID(b);
                        k = new std::string(b);
                    }else {
                        k = tq0->v.s;
                    }

                    std::string vl = "";
                    if( tq->size() > 1 ){
                        hVal *tq1 = tq->at(1);

                        if( tq1->b.tp == hVal::STRING || tq1->b.tp == hVal::CSTRING || tq1->b.tp == hVal::BUF){
                            vl = *tq1->v.s;
                        }else{
                            ec.ts(tq1,vl);
                        }
                    }

//                    db_k->push_back(k);
//                    db_v->push_back(vl);
                    db_k->at(i) = *k;
                    if( tq0->b.tp == hVal::NIL){
//                        delStrEf(k);
                        delete k;
                    }

                    db_v->at(i) = vl;

                }else{
                    continue;
                }
            }else{
                continue;
            }

            std::string sss = db_v->at(i);

//            fprintf(stderr,"lvSet_f WriteBatch i = %d (%s)\n",i,sss.c_str()); fflush(stderr);

            if( kv->lg && af) {
//                lg_k->push_back(kv->lg_max + i+ 1);
                lg_k->at(i) = ++kv->lg_max;
            }

            b_db.Put(db_k->at(i),db_v->at(i));

            if( kf > db_k->at(i)) kf = db_k->at(i);
            if( kl < db_k->at(i)) kl = db_k->at(i);

            if( kv->lg && af){
                leveldb::Slice kl((char*)&lg_k->at(i),sizeof(long));
                b_lg.Put(kl,db_k->at(i));
            }
            ++i;
        }

        kv->l.wl();

        leveldb::Status s = kv->db->Write(write_options, &b_db);

        if( s.ok()) {
            kv->kf = kf;
            kv->kl = kl;
        }

        if( kv->lg && af) {
            kv->lg->Write(write_options, &b_lg);
        }

        kv->l.wu();

        delete db_k;
        delete db_v;
        delete lg_k;

//        fprintf(stderr,"lvSet_f WriteBatch 99 \n"); fflush(stderr);

        goto end;
    }

end:

    delete v;
//    delete db;

    ec.sPB();
//    ec.sPB();

    return 0;
}

hVal* lvSet_f(ef_Content &ec){
    return lvSet_faf(ec,true);
}

hVal* lvSet_fm(ef_Content &ec){
    return lvSet_faf(ec,false);
}

//--

hVal* lvDel_faf(ef_Content &ec,int af){
    if( ec.sd->size() < 2) return 0;

    fprintf(stderr,"lvDel_faf 1\n"); fflush(stderr);


    leveldb::WriteOptions write_options;
    write_options.sync = false;

    hVal *v = ec.sd->back();
    hVal *db = ec.sd->at(ec.sd->size()-2);
    HHDeqV_T *q;
    efKV_C *kv;
    hVal *q0;
    hVal *q1;

    if( db->b.tp != LD_ID) goto end;
    kv = (efKV_C *)db->v.vl;

    if( v->b.tp != hVal::DEQ && v->b.tp != hVal::CDEQ && v->b.tp != hVal::STRING && v->b.tp != hVal::CSTRING) goto end;

    if( v->b.tp == hVal::STRING || v->b.tp == hVal::CSTRING){
        kv->l.wl();

        leveldb::Status s = kv->db->Delete(write_options, *v->v.s);

        if( s.ok() && kv->lg && af) {
            ++kv->lg_max;

            fprintf(stderr,"lvDel_faf 2 <%s>%d\n",v->v.s->c_str(),kv->lg_max); fflush(stderr);

            leveldb::Slice kl((char*)&kv->lg_max,sizeof(long));
            leveldb::Status ss = kv->lg->Put(write_options, kl, *v->v.s);
        }

        kv->l.wu();

        goto end;
    }

    q = v->v.q;
    if( q->size() == 0){goto end;}
/*
    q0 = q->at(0);

    if( q0->b.tp == hVal::STRING || q0->b.tp == hVal::CSTRING){
        std::string k = *q0->v.s;
        std::string vl = "";
        if( q->size() > 1 ){
            q1 = q->at(1);

            if( q1->b.tp == hVal::STRING || q1->b.tp == hVal::CSTRING || q1->b.tp == hVal::BUF){
                vl = *q1->v.s;
            }else{
                ec.ts(q1,vl);
            }
        }

        kv->l.wl();

        leveldb::Status s = kv->db->Put(write_options, k, vl);

        if( s.ok() && kv->lg) {
            ++kv->lg_max;

            leveldb::Slice kl((char*)&kv->lg_max,sizeof(long));
            leveldb::Status ss = kv->lg->Put(write_options, kl, k);
        }

        kv->l.wu();

        goto end;
    }
*/
//    if( q0->b.tp == hVal::DEQ || q0->b.tp == hVal::CDEQ){
    {

        fprintf(stderr,"lvDel_faf WriteBatch %d\n",q->size()); fflush(stderr);

        leveldb::WriteBatch b_db;
        leveldb::WriteBatch b_lg;

        std::vector<std::string> *db_k = new std::vector<std::string>(q->size());
//        std::vector<std::string> *db_v = new std::vector<std::string>(q->size());
        std::vector<long> *lg_k = new std::vector<long>(q->size());

        HHDeqV_TI it = q->begin();
        int i=0;
        for(i=0; it != q->end(); ++it){
            hVal *tv = *it;

            if( tv->b.tp == hVal::STRING || tv->b.tp == hVal::CSTRING){
//                db_k->push_back(*tv->v.s);
//                db_v->push_back("");

                db_k->at(i) = (*tv->v.s);
//                db_v->at(i) = "";

            }
/*
            else if( tv->b.tp == hVal::DEQ || tv->b.tp == hVal::CDEQ){
                HHDeqV_T *tq = tv->v.q;
                hVal *tq0 = tq->at(0);

                if( tq0->b.tp == hVal::STRING || tq0->b.tp == hVal::CSTRING){
                    std::string k = *tq0->v.s;
                    std::string vl = "";
                    if( tq->size() > 1 ){
                        hVal *tq1 = tq->at(1);

                        if( tq1->b.tp == hVal::STRING || tq1->b.tp == hVal::CSTRING || tq1->b.tp == hVal::BUF){
                            vl = *tq1->v.s;
                        }else{
                            ec.ts(tq1,vl);
                        }
                    }

//                    db_k->push_back(k);
//                    db_v->push_back(vl);
                    db_k->at(i) = k;
//                    db_v->at(i) = vl;

                }else{
                    continue;
                }
            }
*/            else{
                continue;
            }

//            std::string sss = db_v->at(i);
//            fprintf(stderr,"lvDel_faf WriteBatch i = %d (%s)\n",i,sss.c_str()); fflush(stderr);

            if( kv->lg && af) {
//                lg_k->push_back(kv->lg_max + i+ 1);
                lg_k->at(i) = ++kv->lg_max;
            }

            b_db.Delete(db_k->at(i));
            if( kv->lg && af){
                leveldb::Slice kl((char*)&lg_k->at(i),sizeof(long));
                b_lg.Put(kl,db_k->at(i));
            }
            ++i;
        }

        kv->l.wl();

        kv->db->Write(write_options, &b_db);
        if( kv->lg && af) {
            kv->lg->Write(write_options, &b_lg);
        }

        kv->l.wu();

        delete db_k;
//        delete db_v;
        delete lg_k;

        fprintf(stderr,"lvDel_faf WriteBatch 99 \n"); fflush(stderr);

        goto end;
    }

end:

    delete v;
//    delete db;

    ec.sPB();
//    ec.sPB();

    return 0;
}

hVal* lvDel_f(ef_Content &ec){
    return lvDel_faf(ec,true);
}

hVal* lvDel_fm(ef_Content &ec){
    return lvDel_faf(ec,false);
}

//--

hVal* lvGetL_f(ef_Content &ec){
    if( ec.sd->size() < 2) return 0;

//    fprintf(stderr,"lvGetL_f 1\n"); fflush(stderr);

    hVal *v = ec.sd->back();
    hVal *db = ec.sd->at(ec.sd->size()-2);
    efKV_C *kv;

    if( db->b.tp != LD_ID) goto end;
    kv = (efKV_C *)db->v.vl;

    if( (v->b.tp != hVal::DOUBLE && v->b.tp != hVal::CDOUBLE) || !kv->lg) goto end;
{
    leveldb::Iterator* it = kv->lg->NewIterator(leveldb::ReadOptions());

    long z = (long)v->v.d;

    leveldb::Slice kl((char*)&z,sizeof(long));

    long mx = 0;

    hVal *a = ec.ef.new_hVal(hVal::DEQ,0,0);

    kv->l.rl();

    for( it->Seek(kl); it->Valid() && mx < 100000; it->Next()) {
        leveldb::Slice sk = it->key();
        leveldb::Slice sv = it->value();

        long lk = *((long*)sk.data());
        if( lk == z) continue;


        std::string *v = ec.ef.new_str((char*)sv.data(),sv.size());


//        fprintf(stderr,"lvGetL_f %d <%s> \n",lk,v->c_str()); fflush(stderr);

        hVal *e = ec.ef.new_hVal(hVal::DEQ,0,0);

        e->_push(ec.ef.new_hVal(hVal::DOUBLE,0,(double)lk));
        e->_push(ec.ef.new_hVal(hVal::STRING,v,0));

        a->_push(e);

        ++mx;
    }

    delete it;

    kv->l.ru();

    delete v;

    ec.sB(a);
}

end:

    return 0;
}

//--

hVal* lvGet_f(ef_Content &ec){
    if( ec.sd->size() < 2) return 0;

    fprintf(stderr,"lvGet_f !!! 1\n"); fflush(stderr);

    hVal *v = ec.sd->back();
    hVal *db = ec.sd->at(ec.sd->size()-2);
    efKV_C *kv;

    if( db->b.tp != LD_ID) goto end;
    kv = (efKV_C *)db->v.vl;

    if( (v->b.tp != hVal::STRING && v->b.tp != hVal::CSTRING)) {goto end;}

{
        std::string value;

        kv->l.rl();

        leveldb::Status s = kv->db->Get(leveldb::ReadOptions(), *v->v.s, &value);

        fprintf(stderr,"lvGet_f !!! 2 s.ok()  %d\n",s.ok()); fflush(stderr);

        kv->l.ru();

//        delete v;

        if( !s.ok()) {
            ec.sB(ec.gN());
        }else{
            if( value.size() > 0){
//                std::string *v = ec.ef.new_str(value);
                hVal *e = ec.ef.new_hVal(hVal::STRING,ec.ef.new_str(value),0);
                ec.sB(e);
            }else{
                ec.sB(ec.gZS());
            }
        }
}

end:

    return 0;
}

hVal* lvFindKV_f(ef_Content &ec,int fkv){
    if( ec.sd->size() < 2) return 0;

//    fprintf(stderr,"lvFind_f 1\n"); fflush(stderr);

    hVal *v = ec.sd->back();
    hVal *db = ec.sd->at(ec.sd->size()-2);
    efKV_C *kv;

    if( db->b.tp != LD_ID) goto end;
    kv = (efKV_C *)db->v.vl;
    if( !kv->db) goto end;

//    fprintf(stderr,"lvFind_f 2\n"); fflush(stderr);


    if( (v->b.tp == hVal::STRING || v->b.tp == hVal::CSTRING)) {
//        fprintf(stderr,"lvFind_f 3\n"); fflush(stderr);

        char *p = (char *)v->v.s->c_str();
        if( *p != '(') goto end;
        char *pp = strchr(++p,')');
        if( !pp) goto end;
        std::string k(p,pp-p);

//        fprintf(stderr,"+++++++++++++++lvFind_f k<%s> !!!<%s><%s>\n",k.c_str(),kv->kf.c_str() , kv->kl.c_str()); fflush(stderr);

        if( k < kv->kf || k > kv->kl){

            delete v;
            ec.sB( ec.ef.new_hVal(hVal::DEQ,0,0));

//fprintf(stderr,"+++++++++++++++lvFind_f k<%s> out of BOUNDS!!!<%s><%s>\n",k.c_str(),kv->kf.c_str() , kv->kl.c_str()); fflush(stderr);

            return 0;
        }

//        fprintf(stderr,"lvFind_f k<%s>\n",k.c_str()); fflush(stderr);

        pcre2_code *re = 0;
        PCRE2_SPTR pattern = (PCRE2_SPTR)v->v.s->c_str();
//        PCRE2_SPTR subject = (PCRE2_SPTR)"12345Язнаю";
//        PCRE2_SPTR name_table;
        int errornumber;
        PCRE2_SIZE erroroffset;
        pcre2_match_data *match_data;
        int rc;
//        PCRE2_SIZE *ovector;

        re = pcre2_compile(
          pattern,               /* the pattern */
          PCRE2_ZERO_TERMINATED, /* indicates pattern is zero-terminated */
          PCRE2_UTF,// | PCRE2_CASELESS,                     /* default options */
          &errornumber,          /* for error number */
          &erroroffset,          /* for error offset */
          NULL);                 /* use default compile context */

        if (re == NULL)
          {
          PCRE2_UCHAR buffer[256];
          pcre2_get_error_message(errornumber, buffer, sizeof(buffer));
          printf("PCRE2 compilation failed at offset %d: %s\n", (int)erroroffset,buffer);
//            return 0;
          }

        if( re) {
            pcre2_jit_compile(re, 0);
            match_data = pcre2_match_data_create_from_pattern(re, NULL);
        }

        long mx = 0;

        leveldb::Iterator* it = kv->db->NewIterator(leveldb::ReadOptions());

        hVal *a = ec.ef.new_hVal(hVal::DEQ,0,0);

        kv->l.rl();

        for( it->Seek(k); it->Valid(); it->Next()) {
            leveldb::Slice sk = it->key();
            leveldb::Slice sv = it->value();

//            fprintf(stderr,"++ lvFind_f NONE BREAK KEY k<%s> sk<%s>\n",k.c_str(),sk.ToString().c_str()); fflush(stderr);

            if( !sk.starts_with(k)){
//               fprintf(stderr,"-- lvFind_f BREAK KEY k<%s> sk<%s>\n",k.c_str(),sk.ToString().c_str()); fflush(stderr);

                break;
            }

            if( re){
                rc = pcre2_match(
                  re,                   /* the compiled pattern */
                  (PCRE2_UCHAR*)sk.data(),              /* the subject string */
                  sk.size(),       /* the length of the subject */
                  0,                    /* start at offset 0 in the subject */
                  0,                    /* default options */
                  match_data,           /* block for storing the result */
                  NULL);                /* use default match context */

//                fprintf(stderr,"RE rc= %d\n",rc); fflush(stderr);

                if( rc < 0) {
                    continue;
/*
                  switch(rc)
                    {
                    case PCRE2_ERROR_NOMATCH: printf("No match\n"); break;
                    default: printf("Matching error %d\n", rc); break;
                    }
*/
                }else{
//                    ovector = pcre2_get_ovector_pointer(match_data);
//                    printf("Match succeeded at offset %d %d\n", (int)ovector[0],(int)ovector[1]);


                }
            }

//            fprintf(stderr,"lvFind_f k<%s> v<%s>\n",sk.data(),sv.data()); fflush(stderr);

            std::string *kk = ec.ef.new_str((char*)sk.data(),sk.size());
            std::string *vv = ec.ef.new_str((char*)sv.data(),sv.size());

//            std::cerr << *kk << "=" << *vv<< "\n"; std::cerr.flush();
            if( fkv){
                hVal *e = ec.ef.new_hVal(hVal::DEQ,0,0);

                e->_push(ec.ef.new_hVal(hVal::STRING,kk,0));
                e->_push(ec.ef.new_hVal(hVal::STRING,vv,0));

                a->_push(e);
            }else{
                a->_push(ec.ef.new_hVal(hVal::STRING,kk,0));
            }

            ++mx;
        }

        kv->l.ru();

        if( re){
            pcre2_match_data_free(match_data);   /* Release memory used for the match */
            pcre2_code_free(re);                 /* data and the compiled pattern. */
        }

        delete it;
        delete v;

        ec.sB(a);

        return 0;
    }

    if( v->b.tp == hVal::NIL) {
NIL:
        leveldb::Iterator* it = kv->db->NewIterator(leveldb::ReadOptions());

        hVal *a = ec.ef.new_hVal(hVal::DEQ,0,0);

        kv->l.rl();

        for( it->SeekToFirst(); it->Valid(); it->Next()) {
            leveldb::Slice sk = it->key();
            leveldb::Slice sv = it->value();

            std::string *kk = ec.ef.new_str((char*)sk.data(),sk.size());
            std::string *vv = ec.ef.new_str((char*)sv.data(),sv.size());

            if( fkv){
                hVal *e = ec.ef.new_hVal(hVal::DEQ,0,0);

                e->_push(ec.ef.new_hVal(hVal::STRING,kk,0));
                e->_push(ec.ef.new_hVal(hVal::STRING,vv,0));

                a->_push(e);
            }else{
                a->_push(ec.ef.new_hVal(hVal::STRING,kk,0));
            }
        }

        kv->l.ru();

        delete it;

        ec.sB(a);

        return 0;
    }

    if( (v->b.tp == hVal::DEQ || v->b.tp == hVal::CDEQ)) {
//        fprintf(stderr,"lvFind_f DEQ 0\n"); fflush(stderr);

        HHDeqV_T *q = v->v.q;
        if( q->size() == 0) {
            delete v;
            goto NIL;
        }
        int mx = 1000000000L;

        std::string k;
        std::string kl;

//        fprintf(stderr,"lvFind_f DEQ 0.01\n"); fflush(stderr);

        leveldb::Slice skl;

//        fprintf(stderr,"lvFind_f DEQ 0.1\n"); fflush(stderr);


        hVal *q0 = 0;
        if( q->size() > 0) {
            q0 = q->at(0);
            if( q0->b.tp == hVal::STRING || q0->b.tp == hVal::CSTRING){
                k = *q0->v.s;
            }
        }
//        fprintf(stderr,"lvFind_f DEQ 1\n"); fflush(stderr);

        hVal *q1 = 0;
        if( q->size() > 1) {
            q1 =q->at(1);
            if( q1->b.tp == hVal::STRING || q1->b.tp == hVal::CSTRING){
                kl = *q1->v.s;
                if( kl.size() > 0){
                    skl = kl;
                }
            }
        }

        hVal *q2 = 0;
        if( q->size() > 2) q2 =q->at(2);

//fprintf(stderr,"lvFind_f DEQ 2\n"); fflush(stderr);

        hVal *q3 = 0;
        if( q->size() > 3) {
            q3 =q->at(3);
            if(q3->b.tp == hVal::DOUBLE || q3->b.tp == hVal::CDOUBLE) {
                mx = (long)q3->v.d;
            }
        }

        if( !q0 && !q1 && !q2 && !q3) {
            delete v;
            goto NIL;
        }
//fprintf(stderr,"lvFind_f !!!\n"); fflush(stderr);

//        std::string k;
//        std::string kl;

        if( (k.size() > 0 && k > kv->kl) || (kl.size() > 0 && kl < kv->kf) ) {//////////////-------------------!!!!

            delete v;
            ec.sB( ec.ef.new_hVal(hVal::DEQ,0,0));

//fprintf(stderr,"[] !!!!! +++lvFind_f k<%s> kl<%s> out of BOUNDS!!!<%s><%s>\n",k.c_str(),kl.c_str(),kv->kf.c_str() , kv->kl.c_str()); fflush(stderr);

            return 0;
        }

        //--

        pcre2_code *re = 0;
//        PCRE2_SPTR pattern = (PCRE2_SPTR)v->v.s->c_str();
//        PCRE2_SPTR subject = (PCRE2_SPTR)"12345Язнаю";
//        PCRE2_SPTR name_table;
        int errornumber;
        PCRE2_SIZE erroroffset;
        pcre2_match_data *match_data;
        int rc;

        if( q2 && (q2->b.tp == hVal::STRING || q2->b.tp == hVal::CSTRING)){

            re = pcre2_compile(
              (PCRE2_SPTR)q2->v.s->c_str(),               /* the pattern */
              PCRE2_ZERO_TERMINATED, /* indicates pattern is zero-terminated */
              PCRE2_UTF,// | PCRE2_CASELESS,                     /* default options */
              &errornumber,          /* for error number */
              &erroroffset,          /* for error offset */
              NULL);                 /* use default compile context */

            if (re == NULL)
              {
              PCRE2_UCHAR buffer[256];
              pcre2_get_error_message(errornumber, buffer, sizeof(buffer));
              printf("PCRE2 compilation failed at offset %d: %s\n", (int)erroroffset,buffer);
    //            return 0;
              }

            if( re) {
                pcre2_jit_compile(re, 0);
                match_data = pcre2_match_data_create_from_pattern(re, NULL);
            }
        }

        leveldb::Iterator* it = kv->db->NewIterator(leveldb::ReadOptions());

        hVal *a = ec.ef.new_hVal(hVal::DEQ,0,0);

        kv->l.rl();

        if( k.size() > 0){
            it->Seek(k);
        }else{
            it->SeekToFirst();
        }

        for( ; it->Valid() && mx > 0; it->Next()) {
            leveldb::Slice sk = it->key();
            leveldb::Slice sv = it->value();
/*
            if( !sk.starts_with(k)){
                continue;
            }
*/
            if( skl.size() > 0){
                if( sk.compare(skl) >= 0){
                    break;
                }
            }

            if( re){
                rc = pcre2_match(
                  re,                   /* the compiled pattern */
                  (PCRE2_UCHAR*)sk.data(),              /* the subject string */
                  sk.size(),       /* the length of the subject */
                  0,                    /* start at offset 0 in the subject */
                  0,                    /* default options */
                  match_data,           /* block for storing the result */
                  NULL);                /* use default match context */

//                fprintf(stderr,"RE rc= %d\n",rc); fflush(stderr);

                if( rc < 0) {
                    continue;
/*
                  switch(rc)
                    {
                    case PCRE2_ERROR_NOMATCH: printf("No match\n"); break;
                    default: printf("Matching error %d\n", rc); break;
                    }
*/
                }else{
//                    ovector = pcre2_get_ovector_pointer(match_data);
//                    printf("Match succeeded at offset %d %d\n", (int)ovector[0],(int)ovector[1]);


                }
            }

//            fprintf(stderr,"lvFind_f k<%s> v<%s>\n",sk.data(),sv.data()); fflush(stderr);

            std::string *kk = ec.ef.new_str((char*)sk.data(),sk.size());
            std::string *vv = ec.ef.new_str((char*)sv.data(),sv.size());

//            std::cerr << *kk << "=" << *vv<< "\n"; std::cerr.flush();

            if( fkv){
                hVal *e = ec.ef.new_hVal(hVal::DEQ,0,0);

                e->_push(ec.ef.new_hVal(hVal::STRING,kk,0));
                e->_push(ec.ef.new_hVal(hVal::STRING,vv,0));

                a->_push(e);
            }else{
                a->_push(ec.ef.new_hVal(hVal::STRING,kk,0));
            }

            --mx;
        }

        kv->l.ru();

        if( re){
            pcre2_match_data_free(match_data);   /* Release memory used for the match */
            pcre2_code_free(re);                 /* data and the compiled pattern. */
        }

        delete it;
        delete v;

        ec.sB(a);

        return 0;
    }


/*
{
    leveldb::Iterator* it = kv->lg->NewIterator(leveldb::ReadOptions());

    long z = (long)v->v.d;

    leveldb::Slice kl((char*)&z,sizeof(long));

    long mx = 0;

    hVal *a = ec.ef.new_hVal(hVal::DEQ,0,0);

    kv->l.rl();

    for( it->Seek(kl); it->Valid() && mx < 1000; it->Next()) {
        leveldb::Slice sk = it->key();
        leveldb::Slice sv = it->value();

        long lk = *((long*)sk.data());
        if( lk == z) continue;


        std::string *v = ec.ef.new_str((char*)sv.data(),sv.size());


        fprintf(stderr,"lvGetL_f %d <%s> \n",lk,v->c_str()); fflush(stderr);

        hVal *e = ec.ef.new_hVal(hVal::DEQ,0,0);

        e->_push(ec.ef.new_hVal(hVal::DOUBLE,0,(double)lk));
        e->_push(ec.ef.new_hVal(hVal::STRING,v,0));

        a->_push(e);

        ++mx;
    }

    delete it;

    kv->l.ru();

    delete v;

    ec.sB(a);
}
*/
end:

    return 0;
}

hVal* lvFind_f(ef_Content &ec){
    return lvFindKV_f(ec,1);
}

hVal* lvFindK_f(ef_Content &ec){
    return lvFindKV_f(ec,0);
}

hVal* lvDelF_f(ef_Content &ec){
    lvFindK_f(ec);
    return lvDel_faf(ec,true);
}

hVal* lvDelF_fm(ef_Content &ec){
    lvFindK_f(ec);
    return lvDel_faf(ec,false);
}

//--

hVal* lvSync_f(ef_Content &ec){

    if( ec.sd->size() < 1) return 0;

    leveldb::WriteOptions write_options;
    write_options.sync = true;

    hVal *db = ec.sd->back();
    if( db->b.tp != LD_ID) return 0;

    efKV_C *kv;
    kv = (efKV_C *)db->v.vl;

    kv->l.wl();

    kv->db->Delete(write_options, async_s);

    if( kv->lg) {
        kv->lg->Delete(write_options, async_s);
    }

    kv->l.wu();

    return 0;
}

hVal* lvCompact_f(ef_Content &ec){

    if( ec.sd->size() < 1) return 0;

    leveldb::WriteOptions write_options;
    write_options.sync = true;

    hVal *db = ec.sd->back();
    if( db->b.tp != LD_ID) return 0;

    efKV_C *kv;
    kv = (efKV_C *)db->v.vl;

    kv->l.wl();

    kv->db->CompactRange(NULL, NULL);

    kv->l.wu();

    return 0;
}


hVal* lvTEST_f(ef_Content &ec){

    fprintf(stderr,"lvTEST_f\n"); fflush(stderr);

    pcre2_code *re;
    PCRE2_SPTR pattern = (PCRE2_SPTR)"(123)(\\d+)(Я.на.)";
    PCRE2_SPTR subject = (PCRE2_SPTR)"12345Язнаю";
    PCRE2_SPTR name_table;
    int errornumber;
    PCRE2_SIZE erroroffset;
    pcre2_match_data *match_data;
    int rc;
    PCRE2_SIZE *ovector;

    re = pcre2_compile(
      pattern,               /* the pattern */
      PCRE2_ZERO_TERMINATED, /* indicates pattern is zero-terminated */
      PCRE2_UTF,// | PCRE2_CASELESS,                     /* default options */
      &errornumber,          /* for error number */
      &erroroffset,          /* for error offset */
      NULL);                 /* use default compile context */

    int ret = pcre2_jit_compile(re, 0);

    /* Compilation failed: print the error message and exit. */

    if (re == NULL)
      {
      PCRE2_UCHAR buffer[256];
      pcre2_get_error_message(errornumber, buffer, sizeof(buffer));
      printf("PCRE2 compilation failed at offset %d: %s\n", (int)erroroffset,buffer);
        return 0;
      }

    match_data = pcre2_match_data_create_from_pattern(re, NULL);

    rc = pcre2_match(
      re,                   /* the compiled pattern */
      subject,              /* the subject string */
      PCRE2_ZERO_TERMINATED,       /* the length of the subject */
      0,                    /* start at offset 0 in the subject */
      0,                    /* default options */
      match_data,           /* block for storing the result */
      NULL);                /* use default match context */

    fprintf(stderr,"RE rc= %d ret= %d\n",rc,ret); fflush(stderr);

    if (rc < 0)
      {
      switch(rc)
        {
        case PCRE2_ERROR_NOMATCH: printf("No match\n"); break;
        /*
        Handle other special cases if you like
        */
        default: printf("Matching error %d\n", rc); break;
        }
//      pcre2_match_data_free(match_data);   /* Release memory used for the match */
//      pcre2_code_free(re);                 /* data and the compiled pattern. */
    }else{
        ovector = pcre2_get_ovector_pointer(match_data);
        printf("Match succeeded at offset %d %d\n", (int)ovector[0],(int)ovector[1]);


    }

    pcre2_match_data_free(match_data);   /* Release memory used for the match */
    pcre2_code_free(re);                 /* data and the compiled pattern. */


    return 0;
}

void init(ef_Content &ec)
{
    std::cerr << "\n" << "inf INIT DLL" << "\n\n"; std::cerr.flush();

    lcmp = new lComparator();
    std::cerr << "\n" << " 0.110 inf INIT DLL" << "\n\n"; std::cerr.flush();

    ec.wl();

//    std::cerr << "\n" << " 0.010!! inf INIT DLL" << "\n\n"; std::cerr.flush();
////    ec.dm->insert( std::pair<int, DEL_FNC>(LD_ID, lv_del_f));
////    ec.f->insert( std::pair<std::string, hVal*>(LD_PREF + "N",ec.ef.new_hVal(hVal::FNCC,(void*)lvNew_f,0)));


    ec.dI(LD_ID, lv_del_f);

    ec.fI(LD_PREF + "N",ec.ef.new_hVal(hVal::FNCC,(void*)lvNew_f,0));

    ec.fI(LD_PREF + "S",ec.ef.new_hVal(hVal::FNCC,(void*)lvSet_f,0));
    ec.fI(LD_PREF + "S-",ec.ef.new_hVal(hVal::FNCC,(void*)lvSet_fm,0));

    ec.fI(LD_PREF + "D",ec.ef.new_hVal(hVal::FNCC,(void*)lvDel_f,0));
    ec.fI(LD_PREF + "D-",ec.ef.new_hVal(hVal::FNCC,(void*)lvDel_fm,0));

    ec.fI(LD_PREF + "SYNC",ec.ef.new_hVal(hVal::FNCC,(void*)lvSync_f,0));

    ec.fI(LD_PREF + "GL",ec.ef.new_hVal(hVal::FNCC,(void*)lvGetL_f,0));


std::cerr << "\n" << " 10 inf INIT DLL" << "\n\n"; std::cerr.flush();

    ec.fI(LD_PREF + "G",ec.ef.new_hVal(hVal::FNCC,(void*)lvGet_f,0));
    ec.fI(LD_PREF + "F",ec.ef.new_hVal(hVal::FNCC,(void*)lvFind_f,0));
    ec.fI(LD_PREF + "FK",ec.ef.new_hVal(hVal::FNCC,(void*)lvFindK_f,0));

    ec.fI(LD_PREF + "DF",ec.ef.new_hVal(hVal::FNCC,(void*)lvDelF_f,0));
    ec.fI(LD_PREF + "DF-",ec.ef.new_hVal(hVal::FNCC,(void*)lvDelF_fm,0));

    ec.fI(LD_PREF + "Compact",ec.ef.new_hVal(hVal::FNCC,(void*)lvCompact_f,0));
    ec.fI(LD_PREF + "Repair",ec.ef.new_hVal(hVal::FNCC,(void*)lvRepair_f,0));
    ec.fI(LD_PREF + "Destroy",ec.ef.new_hVal(hVal::FNCC,(void*)lvDestroy_f,0));

    //-- TEST
    ec.fI(LD_PREF + "T",ec.ef.new_hVal(hVal::FNCC,(void*)lvTEST_f,0));

    ec.wu();

    std::cerr << "\n" << "inf INIT DLL DONE!!!" << "\n\n"; std::cerr.flush();
}
