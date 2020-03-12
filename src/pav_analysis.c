#include <math.h>
#include "pav_analysis.h"

float compute_power(const float *x, unsigned int N){
    float p_db=0, sumatory=0;
    int i;
    
    for(i=0;i<N;i++){
        sumatory+=x[i]*x[i];
    }
    p_db=10*log10(sumatory/N);
    return p_db;
}

float compute_am(const float *x, unsigned int N){
    float amplitude=0, sumatory=0;
    int i=0;
    for(i=0;i<N;i++){
       sumatory+=fabs(x[i]); 
    }  
    amplitude=sumatory/N;
    return amplitude;
}

float compute_zcr(const float *x, unsigned int N, float fm){
    int  i=1, sign1=0, sign2=0, sumatory=0;
    float zcr=0;
    for(i=1;i<N;i++){

        if(x[i-1]>0){
            sign1=1;
        }else{
            sign1=-1;
        }    
        if (x[i]>0){
            sign2=1;
        }else{
            sign2=-1;
        }  
        if(sign1!=sign2){
            sumatory++;
        }

        sign1=0;
        sign2=0;
    }
    zcr=(fm)/(2*N-2)*sumatory;
    return zcr;
}