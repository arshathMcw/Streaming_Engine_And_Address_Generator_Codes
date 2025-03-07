#include <iostream>
#include <c7x.h>
#include <c7x_scalable.h>
#include <sys/time.h>
 
#define CPU_FREQ 2e9

using namespace std;
using namespace c7x;

int main(){
    struct timeval start, stop;
    int in_channel=1 ,out_channel=1, input_height = 16 , input_width = 16 , kernel_height = 3  , kernel_width = 3;
    int stridex = 1,stridey = 1;
    int padding  = 1;
    int output_height = ((input_height + (2 * padding) - kernel_height) / stridex) + 1;
    int output_width = ((input_width + (2 * padding) - kernel_width) / stridey) + 1;
    int inputwop[in_channel][input_height][input_width],kernel[out_channel][in_channel][kernel_height][kernel_width];
    int output2[out_channel][output_height][output_width],output[out_channel][output_height][output_width];
    int cnt = 0;
    for(int ch = 0;ch < out_channel;ch++){
        for(int h = 0;h < output_height;h++){
            for(int w = 0;w < output_width;w++){
                output2[ch][h][w] = 0;
                output[ch][h][w] = 0;
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
    cnt = 0;
    for(int och = 0;och < out_channel;och++){
        for(int ich = 0;ich < in_channel;ich++){
            for(int h = 0;h < kernel_height;h++){
                for(int w = 0;w < kernel_width;w++){
                    kernel[och][ich][h][w] = och+ich+h+w;
                }
            }
        }
    }

    // For paddding
    int input[in_channel][input_height+2*padding][input_width+2*padding];
    for(int ch = 0;ch < in_channel;ch++){
        for(int h = 0;h < input_height;h++){
            for(int w = 0;w < input_width;w++){
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

    // for(int ch = 0;ch < in_channel;ch++){
    //     for(int h = 0;h < input_height;h++){
    //         for(int w = 0;w < input_width;w++){
    //             cout<<input[ch][h][w]<<" ";
    //         }
    //         cout<<endl;
    //     }
    //     cout<<endl;
    // }
    

    __SA_TEMPLATE_v1 saTemplate = __gen_SA_TEMPLATE_v1();
    saTemplate.VECLEN    = sa_veclen<int_vec>::value;
    saTemplate.DIMFMT = __SA_DIMFMT_3D;
    saTemplate.ICNT0 = output_width ;
    saTemplate.ICNT1 = output_height; 
    saTemplate.DIM1 = output_width;   
    saTemplate.ICNT2 =out_channel; 
    saTemplate.DIM2 = output_height * output_width;
    gettimeofday(&start, NULL);
    const int vec_len = element_count_of<int_vec>::value;
    int *pixel=&input[0][0][0];
    int iteration2 = 0;
    __SA0_OPEN(saTemplate);
    for(int ch = 0;ch < out_channel;ch++){
        for (int i = 0; i < output_height; i++) {
            // int *outIdx = &output[ch][i][0];
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
                // *(int_vec *) (outIdx) = sum;
                // outIdx += vec_len;
                __vpred pred = strm_agen<0, int_vec>::get_vpred();
                int_vec * addr = strm_agen<0, int_vec>::get_adv(&output[0][0][0]);
                __vstore_pred(pred, addr, sum);
            }
        }
    }

    int iteration1 = 0;
    for(int ch = 0;ch < out_channel;ch++){
        for(int r = 0; r < output_height; r++) {
            for(int c = 0; c < output_width; c++) {
                int res = 0;
                for(int cch = 0;cch < in_channel;cch++){
                    for(int rr = 0; rr < kernel_height; rr++) {
                        for(int cc = 0; cc < kernel_width; cc++) {
                            int input_r = r * stridex + rr;
                            int input_c = c * stridey + cc;
                            // res += (input[cch][(r*stridex)+rr][(c*stridey)+cc] * kernel[ch][cch][rr][cc]);
                            if (input_r < input_height && input_c < input_width) {
                                res += (input[cch][input_r][input_c] * kernel[ch][cch][rr][cc]);
                            }

                            iteration1++;
                        }
                    }
                }
                output2[ch][r][c] = res;
            }
            
        }
    }
    
    for(int ch = 0;ch < out_channel;ch++){
        for(int h = 0;h < output_height;h++){
            for(int w = 0;w < output_width;w++){
                cout<<output[ch][h][w]<<" ";
            }
            cout<<endl;
        }
        cout<<endl;
    }
    
    for(int ch = 0;ch < out_channel;ch++){
        for(int h = 0;h < output_height;h++){
            for(int w = 0;w < output_width;w++){
                cout<<output2[ch][h][w]<<" ";
            }
            cout<<endl;
        }
        cout<<endl;
    }

    for(int ch = 0;ch < out_channel;ch++){
        for(int h = 0;h < output_height;h++){
            for(int w = 0;w < output_width;w++){
                if(output[ch][h][w] != output2[ch][h][w]){
                    cout<<"They are not equal"<<endl;
                    return 0;
                }
            }
        }
    }
    // cout<<iteration1<<" "<<iteration2<<" "<<iteration1/iteration2<<endl;
}


