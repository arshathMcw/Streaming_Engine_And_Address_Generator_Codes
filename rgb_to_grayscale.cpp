/*
My idea is to have 2 streaming engine , one is for image and another for colours
And 2 Address generator 
    One if for having the address at result matrix and another is for having the value at result matrix
This is just because i am doing operation for each channel seperately , So i need to take the result value which is in ouput when i do for 2nd channel (Blue)
*/




#include "opencv2/opencv.hpp"
#include <iostream>
#include <c7x.h>
#include <c7x_scalable.h>
using namespace std;
using namespace cv;
using namespace c7x;
int main() {
    cout<<"RGB To Gray Scale Conversion using Intrinsics"<<endl;
    Mat image = imread("image2.jpg");
    if (image.empty()) {
        cout << "Could not open or find the image!" << endl;
        return -1;
    }
    const int vec_len = element_count_of<float_vec>::value;
    int channel = 3;
    int height = image.rows;
    int width = image.cols;
    float input[channel][height][width];
    float output[height][width];
    cout << "Image Dimensions: " <<channel<<" x "<< height << " x " << width << endl;
    for (int r = 0; r < height; r++) {
        for (int c = 0; c < width; c++) {
            Vec3b pixel = image.at<Vec3b>(r, c);
            input[0][r][c] = (int)pixel[0];
            input[1][r][c] = (int)pixel[1];
            input[2][r][c] = (int)pixel[2];
        }
    }

    __SA_TEMPLATE_v1 saTemplate = __gen_SA_TEMPLATE_v1();
    saTemplate.VECLEN    = sa_veclen<int_vec>::value;
    saTemplate.DIMFMT = __SA_DIMFMT_2D;
    saTemplate.ICNT0 = width;
    saTemplate.ICNT1 = height; 
    saTemplate.DIM1 = width;  
    
    float_vec constR = float_vec(0.299f);
    float_vec constG = float_vec(0.299f);
    float_vec constB = float_vec(0.299f);
    int iteration = 0;
    int times = (height * width) / vec_len;
    __SA0_OPEN(saTemplate);
    __SA1_OPEN(saTemplate);
    __SA2_OPEN(saTemplate);
    __SA3_OPEN(saTemplate);
    for(int t = 0;t < times;t++){
        iteration++;
        float_vec red = __vmpysp_vvv(constR,* strm_agen<0, float_vec>::get_adv(&input[0][0][0]));
        float_vec green = __vmpysp_vvv(constG,* strm_agen<1, float_vec>::get_adv(&input[1][0][0]));
        float_vec blue = __vmpysp_vvv(constB,* strm_agen<2, float_vec>::get_adv(&input[2][0][0]));
        float_vec temp = __vaddsp_vvv(red,green);
        float_vec res2 = __vaddsp_vvv(temp,blue);
        __vpred pred = strm_agen<3, float_vec>::get_vpred();
        float_vec * addr = strm_agen<3, float_vec>::get_adv(&output[0][0]);
        __vstore_pred(pred, addr, res2);
    }
    __SA1_CLOSE();
    __SA0_CLOSE();
    __SE1_CLOSE();
    cout << "Output Dimensions: "<< height << " x " << width << endl;
    Mat gray_image(height, width, CV_32F, output);  
    gray_image.convertTo(gray_image, CV_8U);
    imwrite("output.jpg", gray_image);
    cout<<"Iteration : "<<iteration<<endl;
    cout<<(3*height*width)/iteration<<" times better"<<endl;
}

// http://www.codebind.com/cpp-tutorial/install-opencv-ubuntu-cpp/

