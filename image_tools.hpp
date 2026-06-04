#ifndef IMAGE_TOOLS_HPP
#define IMAGE_TOOLS_HPP

#include <opencv2/opencv.hpp>

// Поворот ЧАСТИ изображения на 90° против часовой стрелки
// input  : исходное изображение (rows × cols)
// output : выходной буфер для rotated (rotated_rows = cols, rotated_cols = rows)
// Обрабатываются строки повёрнутого изображения от start_row до end_row
void rotate90Chunk(const uchar* input, uchar* output,
                   int rows, int cols,
                   int start_row, int end_row, int rotated_cols);

// Масштабирование ЧАСТИ изображения с билинейной интерполяцией
void scaleChunk(const uchar* input, uchar* output,
                int input_rows, int input_cols,
                int output_rows, int output_cols,
                int start_row, int end_row,
                float scaleFactor);

// Вспомогательные функции для обработки всего изображения (последовательный режим)
void rotate90Full(const cv::Mat& input, cv::Mat& output);
void scaleFull(const cv::Mat& input, cv::Mat& output, float scaleFactor);

#endif