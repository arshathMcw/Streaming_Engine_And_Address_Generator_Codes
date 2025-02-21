/*
We are going to use all the streaing engine and address generator
se 1,2 sa 1 for loading the value from input
sa 2,3,4 for storing it in output
*/

#include "opencv2/opencv.hpp"
#include <iostream>
#include <c7x.h>
#include <c7x_scalable.h>
using namespace std;
using namespace cv;
using namespace c7x;
int main() {
    float ri,gi,bi;
    cout<<"Enter Brightness for Red : ";
    cin>>ri;
    cout<<"Enter Brightness for Green : ";
    cin>>gi;
    cout<<"Enter Brightness for Blue : ";
    cin>>bi;
    Mat image = imread("image.png");
    if (image.empty()) {
        cout << "Could not open or find the image!" << endl;
        return -1;
    }
    int height = image.rows,width = image.cols,channel = 3,iteration = 0;
    const int vec_len = element_count_of<float_vec>::value;
    float input[channel][height][width],output[channel][height][width],output2[channel][height][width]; 
    cout << "Image Dimensions: " << channel << " x " << height << " x " << width << endl;
    for (int r = 0; r < height; r++) {
        for (int c = 0; c < width; c++) {
            Vec3b pixel = image.at<Vec3b>(r, c);
            input[0][r][c] = pixel[2] / 255.0f; 
            input[1][r][c] = pixel[1] / 255.0f; 
            input[2][r][c] = pixel[0] / 255.0f; 
        }
    }

    __SA_TEMPLATE_v1 saTemplate = __gen_SA_TEMPLATE_v1();
    saTemplate.VECLEN    = sa_veclen<float_vec>::value;
    saTemplate.DIMFMT = __SA_DIMFMT_2D;
    saTemplate.ICNT0 = width;
    saTemplate.ICNT1 = height; 
    saTemplate.DIM1 = width;  
    

    __SE_TEMPLATE_v1 seTemplate = __gen_SE_TEMPLATE_v1();
    seTemplate.ELETYPE   = se_eletype<float_vec>::value;
    seTemplate.VECLEN    = se_veclen<float_vec>::value;
    seTemplate.DIMFMT = __SE_DIMFMT_2D;
    seTemplate.ICNT0 = width;
    seTemplate.ICNT1 = height;  
    seTemplate.DIM1 = width;

    int times = (height * width) / vec_len;
    __SE0_OPEN((void *)&input[0][0][0], seTemplate);
    __SE1_OPEN((void *)&input[1][0][0], seTemplate);
    __SA0_OPEN(saTemplate);
    __SA1_OPEN(saTemplate);
    __SA2_OPEN(saTemplate);
    __SA3_OPEN(saTemplate);
    for(int t = 0;t < times;t++,iteration++){
        float_vec r = strm_eng<0, float_vec>::get_adv();
        float_vec g = strm_eng<1, float_vec>::get_adv();
        float_vec b = * strm_agen<0, float_vec>::get_adv(&input[2][0][0]);

        float_vec ra = __vaddsp_vvv(float_vec(ri),r);
        float_vec ga = __vaddsp_vvv(float_vec(gi),g);
        float_vec ba = __vaddsp_vvv(float_vec(bi),b);


        __vpred pred = strm_agen<1, float_vec>::get_vpred();
        float_vec * addr = strm_agen<1, float_vec>::get_adv(&output[0][0][0]);
        __vstore_pred(pred, addr, ra);

        __vpred pred1 = strm_agen<2, float_vec>::get_vpred();
        float_vec * addr1 = strm_agen<2, float_vec>::get_adv(&output[1][0][0]);
        __vstore_pred(pred1, addr1, ga);

        __vpred pred2 = strm_agen<3, float_vec>::get_vpred();
        float_vec * addr2 = strm_agen<3, float_vec>::get_adv(&output[2][0][0]);
        __vstore_pred(pred2, addr2, ba);
    }
    __SE0_CLOSE();
    __SE1_CLOSE();
    __SA0_CLOSE();
    __SA1_CLOSE();
    __SA2_CLOSE();
    __SA3_CLOSE();
    Mat image2(height, width, CV_32FC3);
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            image2.at<Vec3f>(i, j) = Vec3f(output[2][i][j], output[1][i][j], output[0][i][j]); // BGR Order
        }
    }
    Mat image8bit;
    image2.convertTo(image8bit, CV_8UC3, 255.0);
    imwrite("output.jpg", image8bit);
    cout<<"Iteration : "<<iteration<<endl;
    return 0;
}