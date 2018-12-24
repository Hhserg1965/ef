#include "ef_ocv.h"

std::string LD_PREF = "cv.";
#define LD_ID 115

#define MAT_ID LD_ID
#define GBT_ID (LD_ID+1)

//#define LD_PREF "lv."
std::string *newUUID();
void delStrEf(std::string *s);
void getTID(char *s);

void cvM_del_f(void *vv)
{
    Mat *o = (Mat *) vv;
    if( o) {
        o->release();
        delete o;
    }
}

void cvGBT_del_f(void *vv)
{
    CvGBTrees *o = (CvGBTrees *) vv;
    if( o) {
//        o->release();
fprintf(stderr,"DEL_FNC cvGBT_del_f %ld !!!\n",vv); fflush(stderr);
//        delete o;
fprintf(stderr,"DEL_FNC cvGBT_del_f DELETED %ld!!!\n",vv); fflush(stderr);
    }
}

hVal* cvPredict_f(ef_Content &ec){
    HHDeqV_T *q = 0;
    hVal *rv = ec.gN();
    double pz = 0;
    CvGBTrees * gbt = 0;

    if( ec.sd->size() < 2) return 0;

    hVal *b = ec.sd->back();
    hVal *g = ec.sd->at(ec.sd->size() - 2);

    if( g->b.tp != GBT_ID){
        fprintf(stderr,"ERROR: cvPredict_f THIS [] type NOT GBT_ID!\n"); fflush(stderr);
        return 0;
    }

    if( b->b.tp != hVal::DEQ && b->b.tp != hVal::CDEQ){
        fprintf(stderr,"ERROR: cvPredict_f GBT_ID THIS type NOT DEQ!\n"); fflush(stderr);
        return 0;
    }

    gbt = (CvGBTrees*) g->v.vl;

    q = b->v.q;
    Mat z(1,q->size(), CV_32F,Scalar(0.0));

    HHDeqV_TI it = q->begin();
    for(int i=0; it != q->end(); ++it,++i){
        hVal *tv = *it;
        if( tv->b.tp == hVal::DOUBLE || tv->b.tp == hVal::CDOUBLE){
            z.at<float>(i) = (float)tv->v.d;
        }
    }

    pz = (double) gbt->predict(z);
    rv = ec.ef.new_hVal(hVal::DOUBLE,0,pz);

end:

    delete b;
    ec.sB(rv);

    return 0;
}

hVal* cvSave_f(ef_Content &ec){
    if( ec.sd->size() < 2) return 0;

    hVal *b = ec.sd->back();
    hVal *g = ec.sd->at(ec.sd->size() - 2);

    if( b->b.tp != hVal::STRING && b->b.tp != hVal::CSTRING){

        return 0;
    }

    if( g->b.tp != GBT_ID){
        fprintf(stderr,"ERROR: cvSave_f THIS type NOT GBT_ID!\n"); fflush(stderr);
        return 0;
    }

    CvGBTrees *gbt = (CvGBTrees*)g->v.vl;

    gbt->save(b->v.s->c_str());

    delete b;
    ec.sPB();

    return 0;
}


hVal* cvNew_f(ef_Content &ec){

    CvGBTrees *gbt = 0;
    hVal *rv = ec.gN();
    hVal *v0 = 0;
    hVal *v1 = 0;
    hVal *v2 = 0;

    HHDeqV_T *q = 0;
    HHDeqV_T *qq = 0;

    HHDeqV_T *qr = 0;
    HHDeqV_T *qc = 0;

//    HHDeqV_T *qe = 0;

    if( ec.sd->size() < 1) return 0;

//    std::cerr << "\n" << " cvNew_f " << "\n\n"; std::cerr.flush();

    hVal *b = ec.sd->back();
    if( b->b.tp != hVal::STRING && b->b.tp != hVal::CSTRING && b->b.tp != hVal::DEQ && b->b.tp != hVal::CDEQ){
        return 0;
    }

    if( b->b.tp == hVal::STRING || b->b.tp == hVal::CSTRING){
        gbt = new CvGBTrees();
        fprintf(stderr,"CvGBTrees load %ld !!!\n",gbt); fflush(stderr);

        gbt->load(b->v.s->c_str());
        rv = ec.ef.new_hVal(GBT_ID,gbt,0);
        goto end;
    }else{
        qq = b->v.q;
        if( qq->size() < 2){ return 0;}

        v0 = qq->at(0);
        v1 = qq->at(1);

//        fprintf(stderr,"ERROR: cvNew_f [THIS...] type NOT DEQ!\n"); fflush(stderr);


        if( v0->b.tp != hVal::DEQ && v0->b.tp != hVal::CDEQ){
            fprintf(stderr,"ERROR: cvNew_f [THIS...] type NOT DEQ!\n"); fflush(stderr);
            return 0;
        }

        if( v1->b.tp != hVal::DEQ && v1->b.tp != hVal::CDEQ){
            fprintf(stderr,"ERROR: cvNew_f [0,THIS...] type NOT DEQ!\n"); fflush(stderr);
            return 0;
        }

        q = v0->v.q;

        int rows = q->size();

        int cols = 0;
        HHDeqV_TI it = q->begin();
        for(int i=0; it != q->end(); ++it,++i){
            hVal *tv = *it;
            if( tv->b.tp != hVal::DEQ && tv->b.tp != hVal::CDEQ){
                fprintf(stderr,"ERROR: cvNew_f [[]...] type NOT DEQ!\n"); fflush(stderr);
                return 0;
            }

//fprintf(stderr,"rows: %d i: %d tv->v.q->size(): %d \n",rows,i,tv->v.q->size()); fflush(stderr);

            if( i == 0) {
                cols = tv->v.q->size();
            }else if( cols != tv->v.q->size()){
                fprintf(stderr,"ERROR: cvNew_f mismatch LENGTH of cols!\n"); fflush(stderr);
                return 0;
            }
        }

        if( rows != v1->v.q->size()){
            fprintf(stderr,"ERROR: cvNew_f [[...] [].size ..] != cols!\n"); fflush(stderr);
            return 0;
        }

        //-- {} parametrs

        int weak_count = 450;
        int max_depth = 4;

        //--

        if( qq->size() > 2){
            v2 = qq->at(2);
            if( v2->b.tp == hVal::MAP || v2->b.tp == hVal::CMAP){

                HHMapSV_T *m = v2->v.m;

                HHMapSV_TI it;

                it = m->find("wc");

                if( it != m->end()) {
                    hVal *v = it->second;
                    if( v->b.tp == hVal::DOUBLE || v->b.tp == hVal::CDOUBLE){
                        weak_count = v->v.d;
                    }
                }

                it = m->find("md");

                if( it != m->end()) {
                    hVal *v = it->second;
                    if( v->b.tp == hVal::DOUBLE || v->b.tp == hVal::CDOUBLE){
                        max_depth = v->v.d;
                    }
                }

            }
        }

//        fprintf(stderr,"weak_count: %d max_depth: %d\n",weak_count,max_depth); fflush(stderr);

        CvGBTreesParams params;
        params.max_depth = max_depth;
        params.min_sample_count = 1;
        params.weak_count = weak_count;
        params.shrinkage = 0.2f;
        params.subsample_portion = 1.0f;

        params.loss_function_type = CvGBTrees::HUBER_LOSS;

/*
        if( l->type() == CV_32F) {
            params.loss_function_type = CvGBTrees::HUBER_LOSS;
//            params.loss_function_type = CvGBTrees::ABSOLUTE_LOSS;
//printf("CvGBTrees::HUBER_LOSS\n");fflush(stdout);
        }else {
            params.loss_function_type = CvGBTrees::DEVIANCE_LOSS;
        }
*/

        //-- OK working...

        Mat m(rows,cols, CV_32F,Scalar(0.0));
        Mat l(rows,1, CV_32F,Scalar(0.0));

//        fprintf(stderr,"-- 0.01 cols: %d rows: %d \n",cols,rows); fflush(stderr);
//        fprintf(stderr,"-- 0.1 l.cols: %d l.rows: %d elemSize: %d float: %d\n",l.cols,l.rows,l.elemSize(),sizeof(float)); fflush(stderr);

        it = v1->v.q->begin();

        for(int i=0; it != v1->v.q->end(); ++it,++i){

            hVal *tv = *it;

//            fprintf(stderr,"-- 0.2 %f i: %d\n",tv->v.d,i); fflush(stderr);

            if( tv->b.tp == hVal::DOUBLE || tv->b.tp == hVal::CDOUBLE){
                l.at<float>(i) = (float)tv->v.d;
            }else{
                l.at<float>(i) = (float)0;
            }
        }

//        fprintf(stderr,"-- 1\n"); fflush(stderr);

        it = v0->v.q->begin();

        for(int i=0; it != v0->v.q->end(); ++it,++i){
            hVal *tv = *it;
            if( tv->b.tp != hVal::DEQ && tv->b.tp != hVal::CDEQ) continue;

//            fprintf(stderr,"-- 1.1\n"); fflush(stderr);

            HHDeqV_TI it2 = tv->v.q->begin();

//fprintf(stderr,"-- 1.2\n"); fflush(stderr);

            for(int j=0; it2 != tv->v.q->end(); ++it2,++j){
                hVal *tv2 = *it2;
//fprintf(stderr,"-- 1.3 %ld\n",tv2); fflush(stderr);
                if( tv2->b.tp == hVal::DOUBLE || tv2->b.tp == hVal::CDOUBLE){
//fprintf(stderr,"++ %d %d %f\n",j,i,(float)tv2->v.d); fflush(stderr);

                    m.at<float>(i,j) = (float)tv2->v.d;
                }else{
//                    m.at<float>(j,i) = (float)0;
                }
            }
        }
//fprintf(stderr,"-- 2\n"); fflush(stderr);

        Mat t(1, cols + 1, CV_8U, Scalar(CV_VAR_ORDERED));
//        Mat t(1, cols, CV_8U, Scalar(CV_VAR_ORDERED));
        Mat varIdx(1, cols, CV_8U, Scalar(1));
/*
fprintf(stderr,"-- 466\n"); fflush(stderr);

        Mat trainSampleMask(1, cols, CV_32S);
        for (int i = 0; i < cols; ++i)
        {
            trainSampleMask.at<int>(i) = i;
        }

        for(int ii=0; ii < 1000; ++ii){
            Mat *ttt = new Mat(10000, cols*ii, CV_32S);
            delete ttt;
        }

fprintf(stderr,"-- 5\n"); fflush(stderr);
*/
        gbt = new CvGBTrees();
//        delete gbt;
//        fprintf(stderr,"-- 5!!!666\n"); fflush(stderr);
//        goto end;


//        fprintf(stderr,"CvGBTrees train %ld !!!\n",gbt); fflush(stderr);

        gbt->train(
                  m,           // trainData        CV_32F n,d
                  CV_ROW_SAMPLE,    // tflag
                  l,           // responses        CV_32S или CV_32F n,1 -> ответы_обучающие
                  varIdx,           // varIdx           (CV_8U 1,d 1->T) || (CV_32S num1,num2...) -> что используем поля вектора
                  Mat(),        // sampleIdx        CV_32S 1,n1 -> что используем элементы samples -
//                  trainSampleMask,

                  t,           // varType          CV_8U 1*(d+1) -> типы переменных входящего вектора CV_VAR_ORDERED-количественная CV_VAR_CATEGORICAL-категоральная

//Mat(1, m->cols + 1, CV_8U, Scalar(CV_VAR_ORDERED)),
                  Mat(),            // missingDataMask  CV_8U n,d - ???

                  params        //
                  );


        rv = ec.ef.new_hVal(GBT_ID,gbt,0);
        goto end;
    }

end:

    delete b;
    ec.sB(rv);

    return 0;
}

void init_ocv(ef_Content &ec)
{
//    std::cerr << "\n" << "OCV INIT DLL---" << "\n\n"; std::cerr.flush();

    ec.wl();

    ec.dI(MAT_ID, cvM_del_f);
    ec.dI(GBT_ID, cvGBT_del_f);

    ec.fI(LD_PREF + "N",ec.ef.new_hVal(hVal::FNCC,(void*)cvNew_f,0));
    ec.fI(LD_PREF + "P",ec.ef.new_hVal(hVal::FNCC,(void*)cvPredict_f,0));
    ec.fI(LD_PREF + "SAVE",ec.ef.new_hVal(hVal::FNCC,(void*)cvSave_f,0));

    ec.wu();

//    std::cerr << "\n" << "inf INIT DLL DONE!!!" << "\n\n"; std::cerr.flush();
}
/*
[ [[1 2] [3 4] [5 6] [7 8] [9 10]] [1 2 3 4 5] {'md':5,'wc':502} ]cv.N 'gbt.ss'cv.SAVE
#'gbt.ss'cv.N
[7 8] cv.P
*/
