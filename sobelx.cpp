#include "opencv2/opencv.hpp"
#include <iostream>
#include <c7x.h>
#include <c7x_scalable.h>

using namespace std;
using namespace cv;
using namespace c7x;

int main() {
    Mat image = imread("Bikesgray.jpg", IMREAD_GRAYSCALE); 
    if (image.empty()) {
        cout << "Could not open or find the image!" << endl;
        return -1;
    }
    cout<<image.rows<<" X "<<image.cols<<endl;
    const int vec_len = element_count_of<float_vec>::value;
    int height = image.rows;
    int width = image.cols;
    float input[height][width]; 
    for (int r = 0; r < height; r++) {
        for (int c = 0; c < width; c++) {
            input[r][c] = (float)image.at<uchar>(r, c); 
        }
    }
    float output[height][width];
    float kernel[3][3] = {{1,0,-1},{2,0,-2},{1,0,-1}};
    float *pixel=&output[0][0];
    
    for (int i = 0; i < height; i++) {
        float *outIdx = &output[i][0];
        for (int j = 0; j < width; j += vec_len) { 
            float_vec sum = float_vec(0);
            for (int k = 0; k <= 2; k++) {
                for (int l = 0; l <= 2; l++) {
                    pixel = &input[i + k][j + l];
                    float_vec pixels = *(float_vec *)pixel;
                    float_vec kernelVal = (float_vec)(kernel[k][l]); 
                    sum = __vaddsp_vvv(sum, __vmpysp_vvv(pixels, kernelVal));
                }
            }
            *(float_vec *) (outIdx) = sum;
            outIdx += vec_len;
        }
    }
    Mat sobel_image(height, width, CV_32F, output);  
    sobel_image.convertTo(sobel_image, CV_8U);
    imwrite("sobelx.jpg", sobel_image);
    
    cout<<sobel_image.rows<<" X "<<sobel_image.cols<<endl;
    return 0;
}

