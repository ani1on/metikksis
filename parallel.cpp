// #include <iostream>
// #include <mpi.h>
// #include <opencv2/opencv.hpp>
// #include <vector>
// #include <algorithm>
// #include <cmath>
// #include <cstdlib>
// #include "image_tools.hpp"

// using namespace std;
// using namespace cv;

// int main(int argc, char* argv[]) {
//     MPI_Init(&argc, &argv);
//     int rank, size;
//     MPI_Comm_rank(MPI_COMM_WORLD, &rank);
//     MPI_Comm_size(MPI_COMM_WORLD, &size);

//     const int OPTIMAL_PROCS = 6;
//     if (size != OPTIMAL_PROCS && rank == 0) {
//         cerr << "Warning: Running with " << size << " processes (optimal: " << OPTIMAL_PROCS << ")\n";
//     }

//     double total_start = MPI_Wtime();

//     Mat original;
//     int rows = 0, cols = 0;
//     float scaleFactor = 0.5f;

//     if (rank == 0) {
//         string inputFile = (argc > 1) ? argv[1] : "input.jpg";
//         if (argc > 2) scaleFactor = stof(argv[2]);
//         original = imread(inputFile, IMREAD_GRAYSCALE);
//         if (original.empty()) {
//             cerr << "Error: cannot load image " << inputFile << endl;
//             MPI_Abort(MPI_COMM_WORLD, -1);
//         }
//         rows = original.rows;
//         cols = original.cols;
//         cout << "\n=== MPI Parallel Image Processing ===\n";
//         cout << "Processes: " << size << "\n";
//         cout << "Image: " << cols << "x" << rows << "\n";
//         cout << "Scale factor: " << scaleFactor << "\n";
//     }

//     MPI_Bcast(&rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
//     MPI_Bcast(&cols, 1, MPI_INT, 0, MPI_COMM_WORLD);
//     MPI_Bcast(&scaleFactor, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);

//     int scaled_rows = static_cast<int>(rows * scaleFactor);
//     int scaled_cols = static_cast<int>(cols * scaleFactor);
//     int rotated_rows = cols;
//     int rotated_cols = rows;
//     int scaled_rotated_rows = static_cast<int>(rotated_rows * scaleFactor);
//     int scaled_rotated_cols = static_cast<int>(rotated_cols * scaleFactor);

//     int num_procs = size;

//     // Переменные для сбора минимального времени
//     double min_rotate_time = 0.0;
//     double min_scale_time = 0.0;
//     double min_both_time = 0.0;
//     double min_total_time = 0.0;

//     // ------------------------------------------------------------------
//     // 1. ПОВОРОТ
//     // ------------------------------------------------------------------
//     double rotate_start = MPI_Wtime();

//     uchar* original_data = new uchar[rows * cols];
//     if (rank == 0) memcpy(original_data, original.data, rows * cols);
//     MPI_Bcast(original_data, rows * cols, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

//     int rotate_rows_per_proc = rotated_rows / num_procs;
//     int rotate_rem = rotated_rows % num_procs;
//     int my_rot_rows = rotate_rows_per_proc + (rank < rotate_rem ? 1 : 0);
//     int my_rot_start = 0;
//     for (int i = 0; i < rank; ++i)
//         my_rot_start += rotate_rows_per_proc + (i < rotate_rem ? 1 : 0);
//     int my_rot_end = my_rot_start + my_rot_rows;

//     uchar* rotated_local = new uchar[my_rot_rows * rotated_cols];
//     if (my_rot_rows > 0) {
//         rotate90Chunk(original_data, rotated_local, rows, cols,
//                       my_rot_start, my_rot_end, rotated_cols);
//     }

//     vector<int> rot_sendcounts(num_procs), rot_displs(num_procs);
//     Mat rotated;
//     if (rank == 0) {
//         rotated = Mat(rotated_rows, rotated_cols, CV_8UC1);
//         int curr = 0;
//         for (int i = 0; i < num_procs; ++i) {
//             int proc_rows = rotate_rows_per_proc + (i < rotate_rem ? 1 : 0);
//             rot_sendcounts[i] = proc_rows * rotated_cols;
//             rot_displs[i] = curr * rotated_cols;
//             curr += proc_rows;
//         }
//     }
//     MPI_Gatherv(rotated_local, my_rot_rows * rotated_cols, MPI_UNSIGNED_CHAR,
//                 rotated.data, rot_sendcounts.data(), rot_displs.data(),
//                 MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
    
//     double rotate_local_time = MPI_Wtime() - rotate_start;
//     // Находим минимальное время среди всех процессов
//     MPI_Reduce(&rotate_local_time, &min_rotate_time, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);

//     // ------------------------------------------------------------------
//     // 2. МАСШТАБИРОВАНИЕ исходного
//     // ------------------------------------------------------------------
//     double scale_start = MPI_Wtime();
//     int scale_rows_per_proc = scaled_rows / num_procs;
//     int scale_rem = scaled_rows % num_procs;
//     int my_scale_rows = scale_rows_per_proc + (rank < scale_rem ? 1 : 0);
//     int my_scale_start = 0;
//     for (int i = 0; i < rank; ++i)
//         my_scale_start += scale_rows_per_proc + (i < scale_rem ? 1 : 0);
//     int my_scale_end = my_scale_start + my_scale_rows;

//     uchar* scaled_local = new uchar[my_scale_rows * scaled_cols];
//     if (my_scale_rows > 0) {
//         scaleChunk(original_data, scaled_local, rows, cols,
//                    scaled_rows, scaled_cols, my_scale_start, my_scale_end, scaleFactor);
//     }

//     vector<int> scale_sendcounts(num_procs), scale_displs(num_procs);
//     Mat scaled;
//     if (rank == 0) {
//         scaled = Mat(scaled_rows, scaled_cols, CV_8UC1);
//         int curr = 0;
//         for (int i = 0; i < num_procs; ++i) {
//             int proc_rows = scale_rows_per_proc + (i < scale_rem ? 1 : 0);
//             scale_sendcounts[i] = proc_rows * scaled_cols;
//             scale_displs[i] = curr * scaled_cols;
//             curr += proc_rows;
//         }
//     }
//     MPI_Gatherv(scaled_local, my_scale_rows * scaled_cols, MPI_UNSIGNED_CHAR,
//                 scaled.data, scale_sendcounts.data(), scale_displs.data(),
//                 MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
    
//     double scale_local_time = MPI_Wtime() - scale_start;
//     // Находим минимальное время среди всех процессов
//     MPI_Reduce(&scale_local_time, &min_scale_time, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);

//     // ------------------------------------------------------------------
//     // 3. ПОВОРОТ + МАСШТАБИРОВАНИЕ
//     // ------------------------------------------------------------------
//     double both_start = MPI_Wtime();
//     uchar* rotated_data = new uchar[rotated_rows * rotated_cols];
//     if (rank == 0) memcpy(rotated_data, rotated.data, rotated_rows * rotated_cols);
//     MPI_Bcast(rotated_data, rotated_rows * rotated_cols, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

//     int both_rows_per_proc = scaled_rotated_rows / num_procs;
//     int both_rem = scaled_rotated_rows % num_procs;
//     int my_both_rows = both_rows_per_proc + (rank < both_rem ? 1 : 0);
//     int my_both_start = 0;
//     for (int i = 0; i < rank; ++i)
//         my_both_start += both_rows_per_proc + (i < both_rem ? 1 : 0);
//     int my_both_end = my_both_start + my_both_rows;

//     uchar* both_local = new uchar[my_both_rows * scaled_rotated_cols];
//     if (my_both_rows > 0) {
//         scaleChunk(rotated_data, both_local, rotated_rows, rotated_cols,
//                    scaled_rotated_rows, scaled_rotated_cols,
//                    my_both_start, my_both_end, scaleFactor);
//     }

//     vector<int> both_sendcounts(num_procs), both_displs(num_procs);
//     Mat scaled_rotated;
//     if (rank == 0) {
//         scaled_rotated = Mat(scaled_rotated_rows, scaled_rotated_cols, CV_8UC1);
//         int curr = 0;
//         for (int i = 0; i < num_procs; ++i) {
//             int proc_rows = both_rows_per_proc + (i < both_rem ? 1 : 0);
//             both_sendcounts[i] = proc_rows * scaled_rotated_cols;
//             both_displs[i] = curr * scaled_rotated_cols;
//             curr += proc_rows;
//         }
//     }
//     MPI_Gatherv(both_local, my_both_rows * scaled_rotated_cols, MPI_UNSIGNED_CHAR,
//                 scaled_rotated.data, both_sendcounts.data(), both_displs.data(),
//                 MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
    
//     double both_local_time = MPI_Wtime() - both_start;
//     // Находим минимальное время среди всех процессов
//     MPI_Reduce(&both_local_time, &min_both_time, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);

//     double total_local_time = MPI_Wtime() - total_start;
//     // Находим минимальное общее время среди всех процессов
//     MPI_Reduce(&total_local_time, &min_total_time, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);

//     if (rank == 0) {
//         cout << "\n=== Processing Times (Fastest Process) ===\n";
//         cout << "Rotation        : " << min_rotate_time * 1000 << " ms\n";
//         cout << "Scaling         : " << min_scale_time * 1000 << " ms\n";
//         cout << "Rotate+Scale    : " << min_both_time * 1000 << " ms\n";
//         cout << "Total           : " << min_total_time * 1000 << " ms\n";

//         int displayW = max({cols, rotated_cols, scaled_cols, scaled_rotated_cols});
//         int displayH = max({rows, rotated_rows, scaled_rows, scaled_rotated_rows});
//         Mat result(displayH * 2, displayW * 2, CV_8UC1, Scalar(255));

//         original.copyTo(result(Rect(0, 0, cols, rows)));
//         putText(result, "Original", Point(10, 30), FONT_HERSHEY_SIMPLEX, 1, Scalar(0), 2);

//         rotated.copyTo(result(Rect(displayW, 0, rotated_cols, rotated_rows)));
//         putText(result, "Rotated 90°", Point(displayW + 10, 30), FONT_HERSHEY_SIMPLEX, 1, Scalar(0), 2);

//         scaled.copyTo(result(Rect(0, displayH, scaled_cols, scaled_rows)));
//         putText(result, "Scaled", Point(10, displayH + 30), FONT_HERSHEY_SIMPLEX, 1, Scalar(0), 2);

//         scaled_rotated.copyTo(result(Rect(displayW, displayH, scaled_rotated_cols, scaled_rotated_rows)));
//         putText(result, "Rotated+Scaled", Point(displayW + 10, displayH + 30), FONT_HERSHEY_SIMPLEX, 1, Scalar(0), 2);

//         line(result, Point(displayW, 0), Point(displayW, 2 * displayH), Scalar(0), 2);
//         line(result, Point(0, displayH), Point(2 * displayW, displayH), Scalar(0), 2);

//         namedWindow("MPI Results", WINDOW_NORMAL);
//         resizeWindow("MPI Results", 1200, 900);
//         imshow("MPI Results", result);
//         imwrite("original.jpg", original);
//         imwrite("rotated.jpg", rotated);
//         imwrite("scaled.jpg", scaled);
//         imwrite("rotated_scaled.jpg", scaled_rotated);
//         cout << "\nImages saved: original.jpg, rotated.jpg, scaled.jpg, rotated_scaled.jpg\n";
//         cout << "Press any key to exit...\n";
//         waitKey(0);
//         destroyAllWindows();
//     }

//     delete[] original_data;
//     delete[] rotated_local;
//     delete[] scaled_local;
//     delete[] rotated_data;
//     delete[] both_local;

//     MPI_Finalize();
//     return 0;
// }








// #include <iostream>
// #include <mpi.h>
// #include <opencv2/opencv.hpp>
// #include <vector>
// #include <algorithm>
// #include <cmath>
// #include <cstdlib>

// using namespace std;
// using namespace cv;

// // Только объявления функций (без определений)
// void rotate90Chunk(const uchar* src, uchar* dst, int src_rows, int src_cols,
//                    int start_row, int end_row, int dst_cols);

// void scaleChunk(const uchar* src, uchar* dst, int src_rows, int src_cols,
//                 int dst_rows, int dst_cols, int start_row, int end_row, float scaleFactor);

// int main(int argc, char* argv[]) {
//     MPI_Init(&argc, &argv);
//     int rank, size;
//     MPI_Comm_rank(MPI_COMM_WORLD, &rank);
//     MPI_Comm_size(MPI_COMM_WORLD, &size);

//     double total_start = MPI_Wtime();

//     Mat original;
//     int rows = 0, cols = 0;
//     float scaleFactor = 0.5f;

//     if (rank == 0) {
//         string inputFile = (argc > 1) ? argv[1] : "input.jpg";
//         if (argc > 2) scaleFactor = stof(argv[2]);
//         original = imread(inputFile, IMREAD_GRAYSCALE);
//         if (original.empty()) {
//             cerr << "Error: cannot load image " << inputFile << endl;
//             MPI_Abort(MPI_COMM_WORLD, -1);
//         }
//         rows = original.rows;
//         cols = original.cols;
//         cout << "\n=== MPI Parallel Image Processing ===\n";
//         cout << "Processes: " << size << "\n";
//         cout << "Image: " << cols << "x" << rows << "\n";
//         cout << "Scale factor: " << scaleFactor << "\n";
//     }

//     MPI_Bcast(&rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
//     MPI_Bcast(&cols, 1, MPI_INT, 0, MPI_COMM_WORLD);
//     MPI_Bcast(&scaleFactor, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);

//     int scaled_rows = max(1, static_cast<int>(rows * scaleFactor));
//     int scaled_cols = max(1, static_cast<int>(cols * scaleFactor));
//     int rotated_rows = cols;
//     int rotated_cols = rows;

//     // Широковещательная рассылка исходного изображения
//     uchar* original_data = new uchar[rows * cols];
//     if (rank == 0) {
//         memcpy(original_data, original.data, rows * cols);
//     }
//     MPI_Bcast(original_data, rows * cols, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

//     // ------------------------------------------------------------------
//     // 1. ПОВОРОТ НА 90 ГРАДУСОВ
//     // ------------------------------------------------------------------
//     double rotate_start = MPI_Wtime();
    
//     int rotate_rows_per_proc = rotated_rows / size;
//     int rotate_rem = rotated_rows % size;
//     int my_rot_rows = rotate_rows_per_proc + (rank < rotate_rem ? 1 : 0);
    
//     int my_rot_start = 0;
//     for (int i = 0; i < rank; i++) {
//         my_rot_start += rotate_rows_per_proc + (i < rotate_rem ? 1 : 0);
//     }
//     int my_rot_end = my_rot_start + my_rot_rows;
    
//     uchar* rotated_local = new uchar[my_rot_rows * rotated_cols];
    
//     if (my_rot_rows > 0) {
//         rotate90Chunk(original_data, rotated_local, rows, cols,
//                      my_rot_start, my_rot_end, rotated_cols);
//     }
    
//     vector<int> recvCounts(size), displs(size);
//     Mat rotated;
    
//     if (rank == 0) {
//         rotated = Mat(rotated_rows, rotated_cols, CV_8UC1);
//         int offset = 0;
//         for (int i = 0; i < size; i++) {
//             int proc_rows = rotate_rows_per_proc + (i < rotate_rem ? 1 : 0);
//             recvCounts[i] = proc_rows * rotated_cols;
//             displs[i] = offset;
//             offset += recvCounts[i];
//         }
//     }
    
//     MPI_Gatherv(rotated_local, my_rot_rows * rotated_cols, MPI_UNSIGNED_CHAR,
//                 rank == 0 ? rotated.data : nullptr, recvCounts.data(), displs.data(),
//                 MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
    
//     double rotate_time = MPI_Wtime() - rotate_start;
//     delete[] rotated_local;
    
//     // ------------------------------------------------------------------
//     // 2. МАСШТАБИРОВАНИЕ
//     // ------------------------------------------------------------------
//     double scale_start = MPI_Wtime();
    
//     int scale_rows_per_proc = scaled_rows / size;
//     int scale_rem = scaled_rows % size;
//     int my_scale_rows = scale_rows_per_proc + (rank < scale_rem ? 1 : 0);
    
//     int my_scale_start = 0;
//     for (int i = 0; i < rank; i++) {
//         my_scale_start += scale_rows_per_proc + (i < scale_rem ? 1 : 0);
//     }
//     int my_scale_end = my_scale_start + my_scale_rows;
    
//     uchar* scaled_local = new uchar[my_scale_rows * scaled_cols];
    
//     if (my_scale_rows > 0) {
//         scaleChunk(original_data, scaled_local, rows, cols,
//                   scaled_rows, scaled_cols, my_scale_start, my_scale_end, scaleFactor);
//     }
    
//     Mat scaled;
//     if (rank == 0) {
//         scaled = Mat(scaled_rows, scaled_cols, CV_8UC1);
//         int offset = 0;
//         for (int i = 0; i < size; i++) {
//             int proc_rows = scale_rows_per_proc + (i < scale_rem ? 1 : 0);
//             recvCounts[i] = proc_rows * scaled_cols;
//             displs[i] = offset;
//             offset += recvCounts[i];
//         }
//     }
    
//     MPI_Gatherv(scaled_local, my_scale_rows * scaled_cols, MPI_UNSIGNED_CHAR,
//                 rank == 0 ? scaled.data : nullptr, recvCounts.data(), displs.data(),
//                 MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
    
//     double scale_time = MPI_Wtime() - scale_start;
    
//     delete[] scaled_local;
//     delete[] original_data;
    
//     double total_time = MPI_Wtime() - total_start;
    
//     if (rank == 0) {
//         double min_rotate, min_scale, min_total;
//         MPI_Reduce(&rotate_time, &min_rotate, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
//         MPI_Reduce(&scale_time, &min_scale, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
//         MPI_Reduce(&total_time, &min_total, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
        
//         cout << "\n=== Processing Times ===\n";
//         cout << "Rotation        : " << min_rotate * 1000 << " ms\n";
//         cout << "Scaling         : " << min_scale * 1000 << " ms\n";
//         cout << "Total           : " << min_total * 1000 << " ms\n";
        
//         // Сохраняем результаты
//         imwrite("rotated.jpg", rotated);
//         imwrite("scaled.jpg", scaled);
        
//         cout << "\nImages saved: rotated.jpg, scaled.jpg\n";
//         cout << "Rotated: " << rotated_cols << "x" << rotated_rows << "\n";
//         cout << "Scaled: " << scaled_cols << "x" << scaled_rows << "\n";
//     } else {
//         MPI_Reduce(&rotate_time, nullptr, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
//         MPI_Reduce(&scale_time, nullptr, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
//         MPI_Reduce(&total_time, nullptr, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
//     }
    
//     MPI_Finalize();
//     return 0;
// }


#include <iostream>
#include <mpi.h>
#include <opencv2/opencv.hpp>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdlib>

using namespace std;
using namespace cv;

// Только объявления функций (без определений)
void rotate90Chunk(const uchar* src, uchar* dst, int src_rows, int src_cols,
                   int start_row, int end_row, int dst_cols);

void scaleChunk(const uchar* src, uchar* dst, int src_rows, int src_cols,
                int dst_rows, int dst_cols, int start_row, int end_row, float scaleFactor);

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    Mat original;
    int rows = 0, cols = 0;
    float scaleFactor = 0.5f;

    if (rank == 0) {
        string inputFile = (argc > 1) ? argv[1] : "input.jpg";
        if (argc > 2) scaleFactor = stof(argv[2]);
        original = imread(inputFile, IMREAD_GRAYSCALE);
        if (original.empty()) {
            cerr << "Error: cannot load image " << inputFile << endl;
            MPI_Abort(MPI_COMM_WORLD, -1);
        }
        rows = original.rows;
        cols = original.cols;
        cout << "\n=== MPI Parallel Image Processing ===\n";
        cout << "Processes: " << size << "\n";
        cout << "Image: " << cols << "x" << rows << "\n";
        cout << "Scale factor: " << scaleFactor << "\n";
    }

    MPI_Bcast(&rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&cols, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&scaleFactor, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);

    int scaled_rows = max(1, static_cast<int>(rows * scaleFactor));
    int scaled_cols = max(1, static_cast<int>(cols * scaleFactor));
    int rotated_rows = cols;
    int rotated_cols = rows;

    // Широковещательная рассылка исходного изображения
    uchar* original_data = new uchar[rows * cols];
    if (rank == 0) {
        memcpy(original_data, original.data, rows * cols);
    }
    MPI_Bcast(original_data, rows * cols, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
    
    // Синхронизация всех процессов перед началом вычислений
    MPI_Barrier(MPI_COMM_WORLD);

    // ------------------------------------------------------------------
    // 1. ПОВОРОТ НА 90 ГРАДУСОВ
    // ------------------------------------------------------------------
    double rotate_start = MPI_Wtime();
    
    int rotate_rows_per_proc = rotated_rows / size;
    int rotate_rem = rotated_rows % size;
    int my_rot_rows = rotate_rows_per_proc + (rank < rotate_rem ? 1 : 0);
    
    int my_rot_start = 0;
    for (int i = 0; i < rank; i++) {
        my_rot_start += rotate_rows_per_proc + (i < rotate_rem ? 1 : 0);
    }
    int my_rot_end = my_rot_start + my_rot_rows;
    
    uchar* rotated_local = new uchar[my_rot_rows * rotated_cols];
    
    if (my_rot_rows > 0) {
        rotate90Chunk(original_data, rotated_local, rows, cols,
                     my_rot_start, my_rot_end, rotated_cols);
    }
    
    vector<int> recvCounts(size), displs(size);
    Mat rotated;
    
    if (rank == 0) {
        rotated = Mat(rotated_rows, rotated_cols, CV_8UC1);
        int offset = 0;
        for (int i = 0; i < size; i++) {
            int proc_rows = rotate_rows_per_proc + (i < rotate_rem ? 1 : 0);
            recvCounts[i] = proc_rows * rotated_cols;
            displs[i] = offset;
            offset += recvCounts[i];
        }
    }
    
    MPI_Gatherv(rotated_local, my_rot_rows * rotated_cols, MPI_UNSIGNED_CHAR,
                rank == 0 ? rotated.data : nullptr, recvCounts.data(), displs.data(),
                MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
    
    double rotate_time = MPI_Wtime() - rotate_start;
    delete[] rotated_local;
    
    // ------------------------------------------------------------------
    // 2. МАСШТАБИРОВАНИЕ
    // ------------------------------------------------------------------
    double scale_start = MPI_Wtime();
    
    int scale_rows_per_proc = scaled_rows / size;
    int scale_rem = scaled_rows % size;
    int my_scale_rows = scale_rows_per_proc + (rank < scale_rem ? 1 : 0);
    
    int my_scale_start = 0;
    for (int i = 0; i < rank; i++) {
        my_scale_start += scale_rows_per_proc + (i < scale_rem ? 1 : 0);
    }
    int my_scale_end = my_scale_start + my_scale_rows;
    
    uchar* scaled_local = new uchar[my_scale_rows * scaled_cols];
    
    if (my_scale_rows > 0) {
        scaleChunk(original_data, scaled_local, rows, cols,
                  scaled_rows, scaled_cols, my_scale_start, my_scale_end, scaleFactor);
    }
    
    Mat scaled;
    if (rank == 0) {
        scaled = Mat(scaled_rows, scaled_cols, CV_8UC1);
        int offset = 0;
        for (int i = 0; i < size; i++) {
            int proc_rows = scale_rows_per_proc + (i < scale_rem ? 1 : 0);
            recvCounts[i] = proc_rows * scaled_cols;
            displs[i] = offset;
            offset += recvCounts[i];
        }
    }
    
    MPI_Gatherv(scaled_local, my_scale_rows * scaled_cols, MPI_UNSIGNED_CHAR,
                rank == 0 ? scaled.data : nullptr, recvCounts.data(), displs.data(),
                MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
    
    double scale_time = MPI_Wtime() - scale_start;
    
    delete[] scaled_local;
    delete[] original_data;
    
    // Общее время обработки = сумма времени поворота и масштабирования
    double total_processing_time = rotate_time + scale_time;
    
    if (rank == 0) {
        double min_rotate, min_scale, min_total_processing;
        
        // Находим минимальное время среди всех процессов для каждой операции
        MPI_Reduce(&rotate_time, &min_rotate, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
        MPI_Reduce(&scale_time, &min_scale, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
        
        // Для общего времени также берем минимальное (или можно взять максимальное - зависит от задачи)
        // Так как процессы работают параллельно, общее время = max(rotate_time, scale_time)
        // Но по вашему требованию "сумма поворота и уменьшения"
        double sum_processing = min_rotate + min_scale;
        MPI_Reduce(&total_processing_time, &min_total_processing, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
        
        cout << "\n=== Processing Times (Pure Computation) ===\n";
        cout << "Rotation time           : " << min_rotate * 1000 << " ms\n";
        cout << "Scaling time            : " << min_scale * 1000 << " ms\n";
        cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
        cout << "TOTAL processing time   : " << min_total_processing * 1000 << " ms\n";
        cout << "(Sum of rotation + scaling)\n";
        
        // Дополнительная информация для анализа
        cout << "\n=== Additional Information ===\n";
        cout << "Parallel efficiency      : " << (min_rotate + min_scale) / (min_total_processing + 1e-9) << "x\n";
        
        // Сохраняем результаты
        double save_start = MPI_Wtime();
        imwrite("rotated.jpg", rotated);
        imwrite("scaled.jpg", scaled);
        double save_time = MPI_Wtime() - save_start;
        
        cout << "File I/O time           : " << save_time * 1000 << " ms\n";
        
        cout << "\nImages saved: rotated.jpg, scaled.jpg\n";
        cout << "Rotated: " << rotated_cols << "x" << rotated_rows << "\n";
        cout << "Scaled: " << scaled_cols << "x" << scaled_rows << "\n";
    } else {
        MPI_Reduce(&rotate_time, nullptr, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
        MPI_Reduce(&scale_time, nullptr, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
        MPI_Reduce(&total_processing_time, nullptr, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
    }
    
    MPI_Finalize();
    return 0;
}