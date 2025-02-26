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
    cout<<image.rows<<" X "<<image.cols<<endl;
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
    
    for (int i = 0; i < height; i++) {
        float *outIdx = &output3[i][0];
        for (int j = 0; j < width; j += vec_len) { 
            float_vec sum = float_vec(0);
            for (int k = 0; k <= 2; k++) {
                for (int l = 0; l <= 2; l++) {
                    pixel = &output[i + k][j + l];
                    float_vec pixels = *(float_vec *)pixel;
                    float_vec kernelVal = (float_vec)(kernel[k][l]); 
                    sum = __vaddsp_vvv(sum, __vmpysp_vvv(pixels, kernelVal));
                }
            }
            *(float_vec *) (outIdx) = sum;
            outIdx += vec_len;
        }
    }
    // for(int h = 0;h < oheight;h++){
    //     for(int w = 0;w < owidth;w++){
    //         cout<<output3[h][w]<<" ";
    //     }
    //     cout<<endl;
    // }
    Mat blur_image(height, width, CV_32F, output3);  
    blur_image.convertTo(blur_image, CV_8U);
    imwrite("blur.jpg", blur_image);
    for (int i = 0; i < blur_image.rows; i++) {
        for (int j = 0; j < blur_image.cols; j++) {
            cout << (int)blur_image.at<uchar>(i, j) << " ";  // Cast to int for proper display
        }
        cout << endl;
    }
    cout<<blur_image.rows<<" X "<<blur_image.cols<<endl;
    return 0;
}


// #include "opencv2/opencv.hpp"
// #include <iostream>
// #include <c7x.h>
// #include <c7x_scalable.h>

// using namespace std;
// using namespace cv;
// using namespace c7x;

// int main() {
//     Mat image = imread("gray_scale_image.jpg", IMREAD_GRAYSCALE); 
//     if (image.empty()) {
//         cout << "Could not open or find the image!" << endl;
//         return -1;
//     }
//     cout << "Original image size: " << image.rows << " X " << image.cols << endl;
    
//     const int vec_len = element_count_of<float_vec>::value;
//     int height = image.rows;
//     int width = image.cols;
    
//     float output[height][width]; 
//     for (int r = 0; r < height; r++) {
//         for (int c = 0; c < width; c++) {
//             output[r][c] = (float)image.at<uchar>(r, c); 
//         }
//     }
    
//     float output3[height][width];
//     float kernel[3][3] = {
//         {1/16.0, 2/16.0, 1/16.0},
//         {2/16.0, 4/16.0, 2/16.0},
//         {1/16.0, 2/16.0, 1/16.0}
//     };
    
//     // Apply convolution with vectorization
//     for (int i = 1; i < height - 1; i++) {  // Start from 1 and end at height-1 to handle boundaries
//         float *outIdx = &output3[i][0];
//         for (int j = 1; j < width - 1; j += vec_len) {  // Start from 1 and end at width-1
//             float_vec sum = float_vec(0);
//             for (int k = -1; k <= 1; k++) {
//                 for (int l = -1; l <= 1; l++) {
//                     float *pixel = &output[i + k][j + l];
//                     float_vec pixels = *(float_vec *)pixel;
//                     float_vec kernelVal = (float_vec)(kernel[k+1][l+1]); 
//                     sum = __vaddsp_vvv(sum, __vmpysp_vvv(pixels, kernelVal));
//                 }
//             }
//             *(float_vec *)(outIdx) = sum;
//             outIdx += vec_len;
//         }
//     }

//     Mat blur_image(height, width, CV_32F, output3);
//     blur_image.convertTo(blur_image, CV_8U);  // Convert to 8-bit unsigned
//     imwrite("blur.jpg", blur_image);
    
//     // Print the blurred image values correctly
//     cout << "\nBlurred image values:\n";
//     for (int i = 0; i < blur_image.rows; i++) {
//         for (int j = 0; j < blur_image.cols; j++) {
//             cout << (int)blur_image.at<uchar>(i, j) << " ";  // Cast to int for proper display
//         }
//         cout << endl;
//     }
    
//     cout << "Blurred image size: " << blur_image.rows << " X " << blur_image.cols << endl;
//     return 0;
// }


// #include "opencv2/opencv.hpp"
// #include <iostream>
// #include <c7x.h>
// #include <c7x_scalable.h>

// using namespace std;
// using namespace cv;
// using namespace c7x;

// int main() {
//     Mat image = imread("gray_image.jpg", IMREAD_GRAYSCALE); 
//     if (image.empty()) {
//         cout << "Could not open or find the image!" << endl;
//         return -1;
//     }
//     cout << "Original image size: " << image.rows << " X " << image.cols << endl;
    
//     const int vec_len = element_count_of<float_vec>::value;
//     int height = image.rows;
//     int width = image.cols;
    
//     float input[height + 2][width + 2];; 
//         for (int c = 0; c < width; c++) {
//             input[r + 1][c + 1] = (float)image.at<uchar>(r, c);
//         }
//     }
//     for (int r = 0; r < height + 2; r++) {
//         input[r][0] = input[r][1];                
//         input[r][width + 1] = input[r][width];    
//     }
//     for (int c = 0; c < width + 2; c++) {
//         input[0][c] = input[1][c];                
//         input[height + 1][c] = input[height][c];  
//     }
    
//     float output[height][width];
//     float kernel[3][3] = {
//         {1/16.0f, 2/16.0f, 1/16.0f},
//         {2/16.0f, 4/16.0f, 2/16.0f},
//         {1/16.0f, 2/16.0f, 1/16.0f}
//     };

    
//     int times = (height * width) / vec_len;
//     for (int t = 0; t < times; t++) {
//         int row = (t * vec_len) / width;
//         int col = (t * vec_len) % width;
//         float_vec sum = float_vec(0);
//         for (int kr = 0; kr < 3; kr++) {
//             for (int kc = 0; kc < 3; kc++) {
//                 float *pixel = &input[row + kr][col + kc];
//                 float_vec pixels = *(float_vec *)pixel;
//                 float_vec kernelVal = float_vec(kernel[kr][kc]);
//                 sum = __vaddsp_vvv(sum, __vmpysp_vvv(pixels, kernelVal));
//             }
//         }
//         float_vec *outAddr = (float_vec *)&output[row][col];
//         *outAddr = sum;
//     }
    
//     __SA1_CLOSE();
//     __SA0_CLOSE();
//     Mat blur_image(height, width, CV_32F, output);
//     blur_image.convertTo(blur_image, CV_8U);
//     imwrite("blur.jpg", blur_image);
    
//     // Print first few rows and columns to verify
//     cout << "\nFirst 5x5 pixels of blurred image:\n";
//     for (int i = 0; i < blur_image.rows; i++) {
//         for (int j = 0; j < blur_image.cols; j++) {
//             cout << (int)blur_image.at<uchar>(i, j) << "\t";
//         }
//         cout << endl;
//     }
    
//     cout << "Blurred image size: " << blur_image.rows << " X " << blur_image.cols << endl;
//     return 0;
// }