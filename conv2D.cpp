#include <iostream>
#include <c7x.h>
#include <c7x_scalable.h>
#include <sys/time.h>
#include <bits/stdc++.h>
#define CPU_FREQ 2e9

using namespace std;
using namespace c7x;

int main(){
    struct timeval start, stop , start2, stop2;
    int in_channel=3 ,out_channel=3, input_height = 100 , input_width = 100 , kernel_height = 6  , kernel_width = 6;
    int stridex = 1,stridey = 1;
    int padding  = 0;
    int output_height = ((input_height + (2 * padding) - kernel_height) / stridex) + 1;
    int output_width = ((input_width + (2 * padding) - kernel_width) /   stridey) + 1;
    int inputwop[in_channel][input_height][input_width],kernel[out_channel][in_channel][kernel_height][kernel_width];
    int normal_op[out_channel][output_height][output_width],convoluted_op[out_channel][output_height][output_width];
    int cnt = 0;
    for(int ch = 0;ch < out_channel;ch++){
        for(int h = 0;h < output_height;h++){
            for(int w = 0;w < output_width;w++){
                normal_op[ch][h][w] = 0;
                convoluted_op[ch][h][w] = 0;
            }
        }
    }
    for(int ch = 0;ch < in_channel;ch++){
        for(int h = 0;h < input_height;h++){
            for(int w = 0;w < input_width;w++){
                inputwop[ch][h][w] = ch+h+w;
            }
        }
    }
    for(int och = 0;och < out_channel;och++){
        for(int ich = 0;ich < in_channel;ich++){
            for(int h = 0;h < kernel_height;h++){
                for(int w = 0;w < kernel_width;w++){
                    kernel[och][ich][h][w] = och+ich+h+w;
                }
            }
        }
    }
    int new_input_height = input_height+(2*padding);
    int new_input_width = input_width+(2*padding);
    int input[in_channel][new_input_height][new_input_width];
    for(int ch = 0;ch < in_channel;ch++){
        for(int h = 0;h < new_input_height;h++){
            for(int w = 0;w < new_input_width;w++){
                input[ch][h][w] = 0;
            }
        }
    }
    for(int ch = 0;ch < in_channel;ch++){
        for(int h = 0;h < input_height;h++){
            for(int w = 0;w < input_width;w++){
                input[ch][h+padding][w+padding] = inputwop[ch][h][w];
            }
        }
    }
    __SA_TEMPLATE_v1 saTemplate = __gen_SA_TEMPLATE_v1();
    saTemplate.VECLEN    = sa_veclen<int_vec>::value;
    saTemplate.DIMFMT = __SA_DIMFMT_3D;
    saTemplate.ICNT0 = output_width ;
    saTemplate.ICNT1 = output_height; 
    saTemplate.DIM1 = output_width;   
    saTemplate.ICNT2 =out_channel; 
    saTemplate.DIM2 = output_height * output_width;
    const int vec_len = element_count_of<int_vec>::value;
    int *pixel=&input[0][0][0];
    int iteration2 = 0;
    __SA0_OPEN(saTemplate);
    gettimeofday(&start, NULL);
    for(int ch = 0;ch < out_channel;ch++){
        for (int i = 0; i < output_height; i++) {
            for (int j = 0; j < output_width ; j += vec_len) { 
                int_vec sum = int_vec(0);
                for(int x = 0;x < in_channel;x++){
                    for (int k = 0; k < kernel_height; k++) {
                        for (int l = 0; l < kernel_width; l++) {
                            pixel = &input[x][(i * stridex) + k][(j*stridey) + l];
                            int_vec pixels = *(int_vec *)pixel;
                            int_vec kernelVal = (int_vec)kernel[ch][x][k][l]; 
                            sum = __vaddw_vvv(sum, __vmpyww_vvv(pixels, kernelVal));
                            iteration2++;
                        }
                    }
                }
                __vpred pred = strm_agen<0, int_vec>::get_vpred();
                int_vec * addr = strm_agen<0, int_vec>::get_adv(&convoluted_op[0][0][0]);
                __vstore_pred(pred, addr, sum);
            }
        }
    }
    gettimeofday(&stop, NULL);
    cout<<"For Intrinsics Convolution : "<<endl;
    double elapsed_time = (stop.tv_sec - start.tv_sec) + (stop.tv_usec - start.tv_usec) / 1e6;
    double estimated_cycles = elapsed_time * CPU_FREQ;
    printf("Estimated cycle count: %.0f cycles\n", estimated_cycles);
    cout << "Time taken by program is : " << fixed
    << elapsed_time << setprecision(6);
    cout << " sec" << endl;
    int iteration1 = 0;
    gettimeofday(&start2, NULL);
    for(int ch = 0;ch < out_channel;ch++){
        for(int r = 0; r < output_height; r++) {
            for(int c = 0; c < output_width; c++) {
                int res = 0;
                for(int cch = 0;cch < in_channel;cch++){
                    for(int rr = 0; rr < kernel_height; rr++) {
                        for(int cc = 0; cc < kernel_width; cc++) {
                            res += (input[cch][(r*stridex)+rr][(c*stridey)+cc] * kernel[ch][cch][rr][cc]);
                            iteration1++;
                        }
                    }
                }
                normal_op[ch][r][c] = res;
            }
            
        }
    }
    gettimeofday(&stop2, NULL);
    for(int ch = 0;ch < out_channel;ch++){
        for(int h = 0;h < output_height;h++){
            for(int w = 0;w < output_width;w++){
                if(convoluted_op[ch][h][w] != normal_op[ch][h][w]){
                    cout<<"They are not equal"<<endl;
                    return 0;
                }
            }
        }
    }
    
    cout<<"For Normal Convolution : "<<endl;
    double elapsed_time2 = (stop2.tv_sec - start2.tv_sec) + (stop2.tv_usec - start2.tv_usec) / 1e6;
    double estimated_cycles2 = elapsed_time2 * CPU_FREQ;
    printf("Estimated cycle count: %.0f cycles\n", estimated_cycles2);
    cout << "Time taken by program is : " << fixed
    << elapsed_time2 << setprecision(6);
    cout << " sec" << endl;    
    cout<<"They Are Equal"<<endl;
    cout<<iteration1<<" "<<iteration2<<" "<<iteration1/iteration2<<endl;
}


