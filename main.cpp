#include <iostream>
#include <vector>
#include <immintrin.h> // For AVX intrinsics

using namespace std;

// Function to perform convolution with AVX optimization
vector<vector<float>> convolve(const vector<vector<float>> &input, 
                               const vector<vector<float>> &kernel, 
                               int stride) {
    int inSize = input.size();
    int kSize = kernel.size();
    int outSize = (inSize - kSize) / stride + 1;

    // Output matrix
    vector<vector<float>> output(outSize, vector<float>(outSize, 0));

    // Convert kernel to 1D for AVX processing
    vector<float> kernelFlat(kSize * kSize);
    int idx = 0;
    for (int i = 0; i < kSize; i++) {
        for (int j = 0; j < kSize; j++) {
            kernelFlat[idx++] = kernel[i][j];
        }
    }

    // Perform convolution
    for (int i = 0; i < outSize; i++) {
        for (int j = 0; j < outSize; j++) {
            float sum = 0.0;

            __m256 sumVec = _mm256_setzero_ps(); // AVX sum vector

            int kIdx = 0;
            for (int ki = 0; ki < kSize; ki++) {
                for (int kj = 0; kj < kSize; kj += 8) { // Process 8 elements at a time
                    if (kj + 8 > kSize) { // Handle remaining elements
                        for (int rem = 0; rem < kSize - kj; rem++) {
                            sum += input[i * stride + ki][j * stride + kj + rem] * kernelFlat[kIdx + rem];
                        }
                        break;
                    }

                    // Load input and kernel values into AVX registers
                    __m256 inputVec = _mm256_loadu_ps(&input[i * stride + ki][j * stride + kj]);
                    __m256 kernelVec = _mm256_loadu_ps(&kernelFlat[kIdx]);

                    // Multiply and accumulate
                    sumVec = _mm256_fmadd_ps(inputVec, kernelVec, sumVec);

                    kIdx += 8;
                }
            }

            // Sum up AVX register values
            float temp[8];
            _mm256_storeu_ps(temp, sumVec);
            for (int t = 0; t < 8; t++) sum += temp[t];

            output[i][j] = sum;
        }
    }

    return output;
}

// Function to take matrix input from user
vector<vector<float>> getMatrix(int size, const string &name) {
    vector<vector<float>> matrix(size, vector<float>(size));
    cout << "Enter elements of " << name << " matrix (" << size << "x" << size << "):\n";
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            matrix[i][j] = ;
        }
    }
    return matrix;
}

// Main function
int main() {
    int inSize, kSize, stride;

    // User input for matrix sizes
    cout << "Enter input matrix size (N x N): ";
    cin >> inSize;
    cout << "Enter kernel size (K x K): ";
    cin >> kSize;
    cout << "Enter stride value: ";
    cin >> stride;

    // Input validation
    if (inSize < kSize || stride <= 0) {
        cout << "Invalid input! Ensure N >= K and stride > 0.\n";
        return 1;
    }

    // Get matrices
    vector<vector<float>> input = getMatrix(inSize, "input");
    vector<vector<float>> kernel = getMatrix(kSize, "kernel");

    // Perform convolution
    vector<vector<float>> result = convolve(input, kernel, stride);

    // Print result
    cout << "\nConvolved Output:\n";
    for (const auto &row : result) {
        for (float val : row) {
            cout << val << " ";
        }
        cout << endl;
    }

    return 0;
}
