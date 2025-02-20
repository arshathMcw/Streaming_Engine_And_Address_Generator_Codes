/*
My idea is to have 2 streaming engine , one is for image and another for 
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

    __SE_TEMPLATE_v1 seTemplate = __gen_SE_TEMPLATE_v1();
    seTemplate.ELETYPE   = se_eletype<float_vec>::value;
    seTemplate.VECLEN    = se_veclen<float_vec>::value;
    seTemplate.DIMFMT = __SE_DIMFMT_3D;
    seTemplate.ICNT0 = width;
    seTemplate.ICNT1 = height;  
    seTemplate.DIM1 = width;
    seTemplate.ICNT2 = channel;      
    seTemplate.DIM2 = (width * height);


    __SE_TEMPLATE_v1 seTemplate2 = __gen_SE_TEMPLATE_v1();
    seTemplate2.ELETYPE   = se_eletype<float_vec>::value;
    seTemplate2.VECLEN    = se_veclen<float_vec>::value;
    seTemplate2.DIMFMT = __SE_DIMFMT_1D;
    seTemplate2.ICNT0 = channel;
    seTemplate2.ELEDUP    = __SE_ELEDUP_16X;

    __SA_TEMPLATE_v1 saTemplate = __gen_SA_TEMPLATE_v1();
    saTemplate.VECLEN    = sa_veclen<int_vec>::value;
    saTemplate.DIMFMT = __SA_DIMFMT_2D;
    saTemplate.ICNT0 = width;
    saTemplate.ICNT1 = height; 
    saTemplate.DIM1 = width;  
    
    float colors[3] = {0.299f,0.587f,0.114f};
    int iteration = 0;
    int times = (height * width) / vec_len;
    __SE1_OPEN((void *)&colors[0], seTemplate2);
    __SE0_OPEN((void *)&input[0][0][0], seTemplate);
    for(int ch = 0;ch < 3;ch++){
        __SA0_OPEN(saTemplate);
        __SA1_OPEN(saTemplate);
        float_vec color = strm_eng<1, float_vec>::get_adv();
        for(int t = 0;t < times;t++){
            iteration++;
            float_vec curr = __vmpysp_vvv(color,strm_eng<0, float_vec>::get_adv());
            float_vec temp = * strm_agen<1, float_vec>::get_adv((void *)output);
            float_vec res = __vaddsp_vvv(curr,temp);
            __vpred pred = strm_agen<0, float_vec>::get_vpred();
            float_vec * addr = strm_agen<0, float_vec>::get_adv(&output[0][0]);
            __vstore_pred(pred, addr, res);
        }
        __SA1_CLOSE();
        __SA0_CLOSE();
    }
    __SE0_CLOSE();
    cout << "Output Dimensions: "<< height << " x " << width << endl;
    Mat gray_image(height, width, CV_32F, output);  
    gray_image.convertTo(gray_image, CV_8U);
    imwrite("output.jpg", gray_image);
    cout<<"Iteration : "<<iteration<<endl;
    cout<<(3*height*width)/iteration<<" times better"<<endl;
}

// http://www.codebind.com/cpp-tutorial/install-opencv-ubuntu-cpp/

