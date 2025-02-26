#include <iostream>
#include <c7x.h>
#include <c7x_scalable.h>

using namespace std;

using namespace c7x;

using namespace std;
int main(){
    int height = 64,width = 64,channel = 3;
    int32_t cnt = 0;
    int32_t arr[height][width][channel],result[channel][height][width];
    for(int32_t h = 0;h < height;h++){
        for(int32_t w = 0;w < width;w++){
            for(int32_t ch = 0;ch < channel;ch++){
                arr[h][w][ch] = cnt++;
            }
        }
    }
    __SE_TEMPLATE_v1 seTemplate = __gen_SE_TEMPLATE_v1();
    seTemplate.ELETYPE   = se_eletype<int_vec>::value;
    seTemplate.VECLEN    = se_veclen<int_vec>::value;
    seTemplate.TRANSPOSE    = __SE_TRANSPOSE_32BIT;
    seTemplate.DIMFMT = __SE_DIMFMT_4D;
    const int vec_len = element_count_of<int_vec>::value;
    seTemplate.ICNT0 = 3; 
    seTemplate.ICNT1 = vec_len; 
    seTemplate.DIM1 = channel;
    seTemplate.ICNT2 = width;        
    seTemplate.DIM2 = (channel * vec_len);
    seTemplate.ICNT3 = height;               
    seTemplate.DIM3 = width * channel * vec_len;
     __SA_TEMPLATE_v1 saTemplate = __gen_SA_TEMPLATE_v1();
    saTemplate.VECLEN    = sa_veclen<int_vec>::value;
    saTemplate.DIMFMT = __SA_DIMFMT_2D;
    saTemplate.ICNT0 = width;
    saTemplate.ICNT1 = height; 
    saTemplate.DIM1 = width;  
    __SA0_OPEN(saTemplate);
    __SA1_OPEN(saTemplate);
    __SA2_OPEN(saTemplate);
    __SE0_OPEN((void *)&arr[0][0][0], seTemplate);
    int rem = width%vec_len;
    int32_t ctr = 0;

    for(int idx = 0;idx < height * (width / vec_len);idx++){
        __vpred pred = strm_agen<0, int_vec>::get_vpred();
        int_vec * addr = strm_agen<0, int_vec>::get_adv(&result[0][0][0]);
        __vstore_pred(pred, addr, strm_eng<0, int_vec>::get_adv());

        __vpred pred2 = strm_agen<1, int_vec>::get_vpred();
        int_vec * addr2 = strm_agen<1, int_vec>::get_adv(&result[1][0][0]);
        __vstore_pred(pred2, addr2, strm_eng<0, int_vec>::get_adv());

        __vpred pred3 = strm_agen<2, int_vec>::get_vpred();
        int_vec * addr3 = strm_agen<2, int_vec>::get_adv(&result[2][0][0]);
        __vstore_pred(pred3, addr3, strm_eng<0, int_vec>::get_adv());
    }
    __SE0_CLOSE();
    __SA0_CLOSE();
    __SA1_CLOSE();
    __SA2_CLOSE();
    for(int ch = 0;ch < channel;ch++){
        for(int h = 0;h < height;h++){
            for(int w = 0;w < width;w++){
                cout<<result[ch][h][w]<<" ";
            }
            cout<<endl;
        }
        cout<<endl;
    }
}
