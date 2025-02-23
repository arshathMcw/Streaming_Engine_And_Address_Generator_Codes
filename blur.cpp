#include "opencv2/opencv.hpp"
#include <iostream>
#include <c7x.h>
#include <c7x_scalable.h>

using namespace std;
using namespace cv;
using namespace c7x;

int main() {
    Mat image = imread("gray_image.jpg", IMREAD_GRAYSCALE); 
    if (image.empty()) {
        cout << "Could not open or find the image!" << endl;
        return -1;
    }
    const int vec_len = element_count_of<float_vec>::value;
    int height = image.rows;
    int width = image.cols;
    float output[height][width]; 
    for (int r = 0; r < height; r++) {
        for (int c = 0; c < width; c++) {
            output[r][c] = (float)image.at<uchar>(r, c); 
        }
    }
    float output3[height][width];
    float kernel[3][3] = {{1/16.0,2/16.0,1/16.0},{2/16.0,4/16.0,2/16.0},{1/16.0,2/16.0,1/16.0}};
    float *pixel=&output[0][0];

    for (int i = 1; i < height - 1; i++) {
        float *outIdx = &output3[i][0];
        for (int j = 1; j < width; j += vec_len) { 
            float_vec sum = float_vec(0);
            for (int k = -1; k <= 1; k++) {
                for (int l = -1; l <= 1; l++) {
                    pixel = &output[i + k][j + l];
                    float_vec pixels = *(float_vec *)pixel;
                    float_vec kernelVal = (float_vec)(kernel[k + 1][l + 1]); 
                    sum = __vaddsp_vvv(sum, __vmpysp_vvv(pixels, kernelVal));
                }
            }
            *(float_vec *) (outIdx) = sum;
            outIdx += vec_len;
        }
    }
    cout<<"Sana";
    for(int h = 0;h < height;h++){
        for(int w = 0;w < width;w++){
            cout<<output3[h][w]<<" ";
        }
        cout<<endl;
    }
    Mat gray_image(height, width, CV_32F, output3);  
    gray_image.convertTo(gray_image, CV_8U);
    imwrite("blur.jpg", gray_image);

    return 0;
}
