#include "image_tools.hpp"
#include <algorithm>
#include <cmath>

using namespace std;
using namespace cv;

void rotate90Chunk(const uchar* input, uchar* output,
                   int rows, int cols,
                   int start_row, int end_row, int rotated_cols) {
    for (int i = start_row; i < end_row; ++i) {
        for (int j = 0; j < rotated_cols; ++j) {
            int old_row = j;
            int old_col = rows - 1 - i;
            output[(i - start_row) * rotated_cols + j] = input[old_row * cols + old_col];
        }
    }
}

void scaleChunk(const uchar* input, uchar* output,
                int input_rows, int input_cols,
                int output_rows, int output_cols,
                int start_row, int end_row,
                float scaleFactor) {
    for (int i = start_row; i < end_row; ++i) {
        for (int j = 0; j < output_cols; ++j) {
            float src_row = i / scaleFactor;
            float src_col = j / scaleFactor;
            int src_row0 = static_cast<int>(floor(src_row));
            int src_col0 = static_cast<int>(floor(src_col));
            int src_row1 = min(src_row0 + 1, input_rows - 1);
            int src_col1 = min(src_col0 + 1, input_cols - 1);
            float dr = src_row - src_row0;
            float dc = src_col - src_col0;

            float top = (1.0f - dc) * input[src_row0 * input_cols + src_col0] +
                         dc * input[src_row0 * input_cols + src_col1];
            float bottom = (1.0f - dc) * input[src_row1 * input_cols + src_col0] +
                            dc * input[src_row1 * input_cols + src_col1];
            output[(i - start_row) * output_cols + j] = static_cast<uchar>((1.0f - dr) * top + dr * bottom);
        }
    }
}

// Полный поворот всего изображения (последовательный)
void rotate90Full(const Mat& input, Mat& output) {
    int rows = input.rows;
    int cols = input.cols;
    output = Mat(cols, rows, CV_8UC1);
    rotate90Chunk(input.data, output.data, rows, cols, 0, cols, rows);
}

// Полное масштабирование всего изображения (последовательный)
void scaleFull(const Mat& input, Mat& output, float scaleFactor) {
    int input_rows = input.rows;
    int input_cols = input.cols;
    int output_rows = static_cast<int>(input_rows * scaleFactor);
    int output_cols = static_cast<int>(input_cols * scaleFactor);
    output = Mat(output_rows, output_cols, CV_8UC1);
    scaleChunk(input.data, output.data,
               input_rows, input_cols,
               output_rows, output_cols,
               0, output_rows, scaleFactor);
}