#include "opencv2/opencv.hpp"
#include <iostream>
#include <c7x.h>
#include <c7x_scalable.h>
using namespace std;
using namespace cv;
using namespace c7x;
int main() {
    float intensity[3];
    cout<<"Enter Brightness for Red : ";
    cin>>intensity[0];
    cout<<"Enter Brightness for Green : ";
    cin>>intensity[1];
    cout<<"Enter Brightness for Blue : ";
    cin>>intensity[2];
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
    saTemplate.DIMFMT = __SA_DIMFMT_3D;
    saTemplate.ICNT0 = width;
    saTemplate.ICNT1 = height; 
    saTemplate.DIM1 = width; 
    saTemplate.ICNT2 = channel;      
    saTemplate.DIM2 = (width * height);
    int times = (height * width) / vec_len;
    __SE1_OPEN((void *)&intensity[0], seTemplate2);
    __SE0_OPEN((void *)&input[0][0][0], seTemplate);
    __SA0_OPEN(saTemplate);
    for(int ch = 0;ch < channel;ch++){
        float_vec color = strm_eng<1, float_vec>::get_adv();
        for(int t = 0;t < times;t++,iteration++){
            float_vec res1 = __vaddsp_vvv(color,strm_eng<0, float_vec>::get_adv());
            __vpred pred = strm_agen<0, float_vec>::get_vpred();
            float_vec * addr = strm_agen<0, float_vec>::get_adv(&output[0][0][0]);
            __vstore_pred(pred, addr, res1);
        }
    }
    __SE0_CLOSE();
    __SE1_CLOSE();
    __SA0_CLOSE();
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