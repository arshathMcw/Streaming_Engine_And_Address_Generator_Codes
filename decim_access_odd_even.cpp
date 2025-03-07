#include <c7x.h>
#include <c7x_scalable.h>
#include <iostream>
#include <sys/time.h>
#include <bits/stdc++.h>
using namespace std;
using namespace c7x;
#define CPU_FREQ 2e9
int main(){
    cout<<"Access Odd Even using Decimation : "<<endl;
    struct timeval start, stop;
    int32_t height,width,idx,h,w,cnt = 0,vec_len = element_count_of<int_vec>::value;
    cout<<"Enter Image Height : ";
    cin>>height;
    cout<<"Enter Image Width : ";
    cin>>width;
    cout<<"Enter the Index [0 : Even , 1 : Odd] : ";
    cin>>idx;
    int32_t image[height][width],output[height][width];
    for(h = 0;h < height;h++){
        for(w = 0;w < width;w++){
            image[h][w] = cnt++;
        }
    }

    __SE_TEMPLATE_v1 seTemplate = __gen_SE_TEMPLATE_v1();
    seTemplate.ELETYPE = se_eletype<int_vec>::value;
    seTemplate.VECLEN  = se_veclen<long_vec>::value;   
    seTemplate.DIMFMT = __SE_DIMFMT_3D;
    seTemplate.ICNT0 = width+(width%2);
    seTemplate.ICNT1 = height;  
    seTemplate.DIM1 = width;
    seTemplate.ICNT2 = height / 16;  
    seTemplate.DIM2 = width * 16;
    seTemplate.DECIM   = __SE_DECIM_2;
    seTemplate.PROMOTE = __SE_PROMOTE_2X_SIGNEXT;
    __SA_TEMPLATE_v1 saTemplate = __gen_SA_TEMPLATE_v1();
    saTemplate.VECLEN    = sa_veclen<int_vec>::value;
    saTemplate.DIMFMT = __SA_DIMFMT_2D;
    saTemplate.ICNT0 = width ;
    saTemplate.ICNT1 = height; 
    saTemplate.DIM1 = width; 

    __SA0_OPEN(saTemplate);
    __SE0_OPEN((void *)&image[0][idx], seTemplate);
    gettimeofday(&start, NULL);
    for(int32_t ctr = 0; ctr < height * ceil(width / (float)vec_len); ctr++) {
        __vpred pred = strm_agen<0, int_vec>::get_vpred();
        int_vec * addr = strm_agen<0, int_vec>::get_adv(&output[0][idx]);
        __vstore_pred(pred, addr, strm_eng<0, int_vec>::get_adv());
    }
    gettimeofday(&stop, NULL);
    __SE0_CLOSE();

    double elapsed_time = (stop.tv_sec - start.tv_sec) + (stop.tv_usec - start.tv_usec) / 1e6;
    double estimated_cycles = elapsed_time * CPU_FREQ;
    printf("Estimated cycle count: %.0f cycles\n", estimated_cycles);
    double time_taken;
    time_taken = (stop.tv_sec - start.tv_sec) * 1e6;
    time_taken = (time_taken + (stop.tv_usec - 
                              start.tv_usec)) * 1e-6;
 
    cout << "Time taken by program is : " << fixed
         << time_taken << setprecision(6);
    cout << " sec" << endl;
}