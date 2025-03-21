#include <iostream>
#include <c7x.h>
#include <c7x_scalable.h>
#include <sys/time.h>
#include <bits/stdc++.h>

using namespace c7x;
using namespace std;

#define CPU_FREQ 2e9

int main() {
    struct timeval start, stop;
    int input_h = 100, input_w = 100;
    int in_channel = 3, out_channel = 3;
    int kernel_h = 64, kernel_w = 64;
    int stride_x = 1, stride_y = 1;
    int padding = 0;
    int padded_h = input_h + 2 * padding;
    int padded_w = input_w + 2 * padding;
    int output_h = (padded_h - kernel_h) / stride_x + 1;
    int output_w = (padded_w - kernel_w) / stride_y + 1;
    int col_size = output_h * output_w; 
    int vec_len = element_count_of<int_vec>::value;
    int input[in_channel][input_h][input_w];
    int iteration = 0;
    int cnt = 0;
    for (int c = 0; c < in_channel; c++) {
        for (int i = 0; i < input_h; i++) {
            for (int j = 0; j < input_w; j++) {
                input[c][i][j] = i+j;
            }
        }
    }
    int kernel[out_channel][in_channel][kernel_h][kernel_w];
    cnt = 0;
    for (int oc = 0; oc < out_channel; oc++) {
        for (int ic = 0; ic < in_channel; ic++) {
            for (int i = 0; i < kernel_h; i++) {
                for (int j = 0; j < kernel_w; j++) {
                    kernel[oc][ic][i][j] = i+j;
                }
            }
        }
    }
    int padded_input[in_channel][padded_h][padded_w] = {0};
    for (int c = 0; c < in_channel; c++) {
        for (int i = 0; i < input_h; i++) {
            for (int j = 0; j < input_w; j++) {
                padded_input[c][i + padding][j + padding] = input[c][i][j];
            }
        }
    }
    int patch_size = in_channel * kernel_h * kernel_w ;
    int** col = new int*[patch_size];
    for (int i = 0; i < patch_size; i++) {
        col[i] = new int[col_size];
    }

    for (int i = 0, col_index = 0; i + kernel_h <= padded_h; i += stride_x) {
        for (int j = 0; j + kernel_w <= padded_w; j += stride_y) {
            int patch_index = 0;
            for (int ic = 0; ic < in_channel; ic++) {
                for (int ki = 0; ki < kernel_h; ki++) {
                    for (int kj = 0; kj < kernel_w; kj++) {
                        col[patch_index][col_index] = padded_input[ic][i + ki][j + kj];
                        patch_index++;
                    }
                }
            }
            col_index++;
        }
    }
    int kernel_row[out_channel][patch_size];
    for (int oc = 0; oc < out_channel; oc++) {
        int idx = 0;
        for (int ic = 0; ic < in_channel; ic++) {
            for (int i = 0; i < kernel_h; i++) {
                for (int j = 0; j < kernel_w; j++) {
                    kernel_row[oc][idx++] = kernel[oc][ic][i][j];
                }
            }
        }
    }
    __SE_TEMPLATE_v1 seTemplate2 = __gen_SE_TEMPLATE_v1();
    seTemplate2.ELETYPE   = se_eletype<int_vec>::value;
    seTemplate2.VECLEN    = se_veclen<int_vec>::value;
    seTemplate2.DIMFMT = __SE_DIMFMT_3D;
    seTemplate2.ICNT0 = patch_size * out_channel;
    seTemplate2.ICNT1 = ceil(col_size / (float) vec_len); 
    seTemplate2.DIM1 = 0;
    seTemplate2.ICNT2 = out_channel; 
    seTemplate2.DIM2 = patch_size;
    seTemplate2.ELEDUP    = __SE_ELEDUP_16X;
    
    __SE_TEMPLATE_v1 seTemplate = __gen_SE_TEMPLATE_v1();
    seTemplate.ELETYPE   = se_eletype<int_vec>::value;
    seTemplate.VECLEN    = se_veclen<int_vec>::value;
    seTemplate.DIMFMT = __SE_DIMFMT_4D;
    seTemplate.ICNT0 = vec_len;
    seTemplate.ICNT1 = patch_size; 
    seTemplate.DIM1 = col_size;
    seTemplate.ICNT2 = ceil(col_size / (float) vec_len); ; 
    seTemplate.DIM2 = 16;
    seTemplate.ICNT3 = out_channel;  
    seTemplate.DIM3 = 0;
    
    __SA_TEMPLATE_v1 saTemplate = __gen_SA_TEMPLATE_v1();
    saTemplate.VECLEN    = sa_veclen<int_vec>::value;
    saTemplate.DIMFMT = __SA_DIMFMT_2D;
    saTemplate.ICNT0 = col_size ;
    saTemplate.ICNT1 = out_channel; 
    saTemplate.DIM1 = col_size;  
    __SA0_OPEN(saTemplate);
    __SE0_OPEN((void *)&kernel_row[0][0], seTemplate2);
    __SE1_OPEN((void *)&col[0][0], seTemplate);
    
    gettimeofday(&start, NULL);
    int gemm_result[out_channel][col_size] = {0};
    for(int times = 0;times < out_channel*ceil(col_size/(float)vec_len);times++){
        int_vec vOutC = (int_vec)(0);
        for(int cc = 0;cc < patch_size;cc+=1){
            int_vec resw = __vmpyww_vvv(strm_eng<0, int_vec>::get_adv(),strm_eng<1, int_vec>::get_adv());
            vOutC = __vaddw_vvv(vOutC,resw);
        }
        __vpred pred = strm_agen<0, int_vec>::get_vpred();
        int_vec * addr = strm_agen<0, int_vec>::get_adv(&gemm_result[0][0]);
        __vstore_pred(pred, addr, vOutC);
    }
    gettimeofday(&stop, NULL);
    int32_t output[out_channel][output_h][output_w];
    for (int oc = 0; oc < out_channel; oc++) {
        for (int i = 0; i < output_h; i++) {
            for (int j = 0; j < output_w; j++) {
                output[oc][i][j] = gemm_result[oc][i * output_w + j];
            }
        }
    }
    cout<<"For Img Convolution : "<<endl;
    double elapsed_time = (stop.tv_sec - start.tv_sec) + (stop.tv_usec - start.tv_usec) / 1e6;
    double estimated_cycles = elapsed_time * CPU_FREQ;
    printf("Estimated cycle count: %.0f cycles\n", estimated_cycles);
    cout << "Time taken by program is : " << fixed
         << elapsed_time << setprecision(6);
    cout << " sec" << endl;
    return 0;
}
