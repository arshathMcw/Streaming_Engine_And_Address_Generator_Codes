#include "opencv2/opencv.hpp"
#include <iostream>
#include <c7x.h>
#include <c7x_scalable.h>

using namespace std;
using namespace cv;
using namespace c7x;

int main(){
    int height = 16 , width = 16 , kheight = 3  , kwidth = 3;
    int oheight = height - kheight + 1;
    int owidth = width - kwidth + 1;
    float input[height][width],kernel[kheight][kwidth],output[oheight][owidth];
    int cnt = 0;
    for(int h = 0;h < height;h++){
        for(int w = 0;w < width;w++){
            input[h][w] = cnt++;
        }
    }
    cnt = 0;
    for(int h = 0;h < kheight;h++){
        for(int w = 0;w < kwidth;w++){
            kernel[h][w] = cnt++;
        }
    }
    // cout<<"Input : "<<endl;
    // for(int h = 0;h < height;h++){
    //     for(int w = 0;w < width;w++){
    //         cout<<input[h][w]<<" ";
    //     }
    //     cout<<endl;
    // }
    // cout<<"Kernel : "<<endl;
    // for(int h = 0;h < kheight;h++){
    //     for(int w = 0;w < kwidth;w++){
    //         cout<<kernel[h][w]<<" ";
    //     }
    //     cout<<endl;
    // }
    const int vec_len = element_count_of<float_vec>::value;
    float *pixel=&input[0][0];
    int iterations = 0;
    for (int i = 0; i < oheight; i++) {
        float *outIdx = &output[i][0];
        for (int j = 0; j < owidth ; j += vec_len) { 
            float_vec sum = float_vec(0);
            for (int k = 0; k <= 2; k++) {
                for (int l = 0; l <= 2; l++) {
                    pixel = &input[i + k][j + l];
                    float_vec pixels = *(float_vec *)pixel;
                    float_vec kernelVal = (float_vec)kernel[k][l]; 
                    cout<<"Input : ";
                    pixels.print();
                    cout<<"Kernel : ";
                    kernelVal.print();
                    sum = __vaddsp_vvv(sum, __vmpysp_vvv(pixels, kernelVal));
                    iterations++;
                    cout<<"Temporary Sum : ";
                    sum.print();
                }
            }
            cout<<"Sum : ";
            sum.print();
            cout<<"--------------------------------"<<endl;
            *(float_vec *) (outIdx) = sum;
            outIdx += vec_len;
        }
    }
    // cout<<"Iterations : "<<iterations<<endl;
    // cout<<"Output : "<<endl;
    // for(int h = 0;h < oheight;h++){
    //     for(int w = 0;w < owidth;w++){
    //         cout<<output[h][w]<<" ";
    //     }
    //     cout<<endl;
    // }
}