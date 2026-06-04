#include <iostream>
#include <mpi.h>
#include <opencv2/opencv.hpp>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include "image_tools.hpp"

using namespace std;
using namespace cv;

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    const int OPTIMAL_PROCS = 6;
    if (size != OPTIMAL_PROCS && rank == 0) {
        cerr << "Warning: Running with " << size << " processes (optimal: " << OPTIMAL_PROCS << ")\n";
    }

    double total_start = MPI_Wtime();

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

    int scaled_rows = static_cast<int>(rows * scaleFactor);
    int scaled_cols = static_cast<int>(cols * scaleFactor);
    int rotated_rows = cols;
    int rotated_cols = rows;
    int scaled_rotated_rows = static_cast<int>(rotated_rows * scaleFactor);
    int scaled_rotated_cols = static_cast<int>(rotated_cols * scaleFactor);

    int num_procs = size;

    // ------------------------------------------------------------------
    // 1. ПОВОРОТ
    // ------------------------------------------------------------------
    double rotate_start = MPI_Wtime();

    uchar* original_data = new uchar[rows * cols];
    if (rank == 0) memcpy(original_data, original.data, rows * cols);
    MPI_Bcast(original_data, rows * cols, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

    int rotate_rows_per_proc = rotated_rows / num_procs;
    int rotate_rem = rotated_rows % num_procs;
    int my_rot_rows = rotate_rows_per_proc + (rank < rotate_rem ? 1 : 0);
    int my_rot_start = 0;
    for (int i = 0; i < rank; ++i)
        my_rot_start += rotate_rows_per_proc + (i < rotate_rem ? 1 : 0);
    int my_rot_end = my_rot_start + my_rot_rows;

    uchar* rotated_local = new uchar[my_rot_rows * rotated_cols];
    if (my_rot_rows > 0) {
        rotate90Chunk(original_data, rotated_local, rows, cols,
                      my_rot_start, my_rot_end, rotated_cols);
    }

    vector<int> rot_sendcounts(num_procs), rot_displs(num_procs);
    Mat rotated;
    if (rank == 0) {
        rotated = Mat(rotated_rows, rotated_cols, CV_8UC1);
        int curr = 0;
        for (int i = 0; i < num_procs; ++i) {
            int proc_rows = rotate_rows_per_proc + (i < rotate_rem ? 1 : 0);
            rot_sendcounts[i] = proc_rows * rotated_cols;
            rot_displs[i] = curr * rotated_cols;
            curr += proc_rows;
        }
    }
    MPI_Gatherv(rotated_local, my_rot_rows * rotated_cols, MPI_UNSIGNED_CHAR,
                rotated.data, rot_sendcounts.data(), rot_displs.data(),
                MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
    double rotate_end = MPI_Wtime();

    // ------------------------------------------------------------------
    // 2. МАСШТАБИРОВАНИЕ исходного
    // ------------------------------------------------------------------
    double scale_start = MPI_Wtime();
    int scale_rows_per_proc = scaled_rows / num_procs;
    int scale_rem = scaled_rows % num_procs;
    int my_scale_rows = scale_rows_per_proc + (rank < scale_rem ? 1 : 0);
    int my_scale_start = 0;
    for (int i = 0; i < rank; ++i)
        my_scale_start += scale_rows_per_proc + (i < scale_rem ? 1 : 0);
    int my_scale_end = my_scale_start + my_scale_rows;

    uchar* scaled_local = new uchar[my_scale_rows * scaled_cols];
    if (my_scale_rows > 0) {
        scaleChunk(original_data, scaled_local, rows, cols,
                   scaled_rows, scaled_cols, my_scale_start, my_scale_end, scaleFactor);
    }

    vector<int> scale_sendcounts(num_procs), scale_displs(num_procs);
    Mat scaled;
    if (rank == 0) {
        scaled = Mat(scaled_rows, scaled_cols, CV_8UC1);
        int curr = 0;
        for (int i = 0; i < num_procs; ++i) {
            int proc_rows = scale_rows_per_proc + (i < scale_rem ? 1 : 0);
            scale_sendcounts[i] = proc_rows * scaled_cols;
            scale_displs[i] = curr * scaled_cols;
            curr += proc_rows;
        }
    }
    MPI_Gatherv(scaled_local, my_scale_rows * scaled_cols, MPI_UNSIGNED_CHAR,
                scaled.data, scale_sendcounts.data(), scale_displs.data(),
                MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
    double scale_end = MPI_Wtime();

    // ------------------------------------------------------------------
    // 3. ПОВОРОТ + МАСШТАБИРОВАНИЕ
    // ------------------------------------------------------------------
    double both_start = MPI_Wtime();
    uchar* rotated_data = new uchar[rotated_rows * rotated_cols];
    if (rank == 0) memcpy(rotated_data, rotated.data, rotated_rows * rotated_cols);
    MPI_Bcast(rotated_data, rotated_rows * rotated_cols, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

    int both_rows_per_proc = scaled_rotated_rows / num_procs;
    int both_rem = scaled_rotated_rows % num_procs;
    int my_both_rows = both_rows_per_proc + (rank < both_rem ? 1 : 0);
    int my_both_start = 0;
    for (int i = 0; i < rank; ++i)
        my_both_start += both_rows_per_proc + (i < both_rem ? 1 : 0);
    int my_both_end = my_both_start + my_both_rows;

    uchar* both_local = new uchar[my_both_rows * scaled_rotated_cols];
    if (my_both_rows > 0) {
        scaleChunk(rotated_data, both_local, rotated_rows, rotated_cols,
                   scaled_rotated_rows, scaled_rotated_cols,
                   my_both_start, my_both_end, scaleFactor);
    }

    vector<int> both_sendcounts(num_procs), both_displs(num_procs);
    Mat scaled_rotated;
    if (rank == 0) {
        scaled_rotated = Mat(scaled_rotated_rows, scaled_rotated_cols, CV_8UC1);
        int curr = 0;
        for (int i = 0; i < num_procs; ++i) {
            int proc_rows = both_rows_per_proc + (i < both_rem ? 1 : 0);
            both_sendcounts[i] = proc_rows * scaled_rotated_cols;
            both_displs[i] = curr * scaled_rotated_cols;
            curr += proc_rows;
        }
    }
    MPI_Gatherv(both_local, my_both_rows * scaled_rotated_cols, MPI_UNSIGNED_CHAR,
                scaled_rotated.data, both_sendcounts.data(), both_displs.data(),
                MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
    double both_end = MPI_Wtime();

    double total_end = MPI_Wtime();

    if (rank == 0) {
        cout << "\n=== Processing Times ===\n";
        cout << "Rotation        : " << (rotate_end - rotate_start) * 1000 << " ms\n";
        cout << "Scaling         : " << (scale_end - scale_start) * 1000 << " ms\n";
        cout << "Rotate+Scale    : " << (both_end - both_start) * 1000 << " ms\n";
        cout << "Total           : " << (total_end - total_start) * 1000 << " ms\n";

        int displayW = max({cols, rotated_cols, scaled_cols, scaled_rotated_cols});
        int displayH = max({rows, rotated_rows, scaled_rows, scaled_rotated_rows});
        Mat result(displayH * 2, displayW * 2, CV_8UC1, Scalar(255));

        original.copyTo(result(Rect(0, 0, cols, rows)));
        putText(result, "Original", Point(10, 30), FONT_HERSHEY_SIMPLEX, 1, Scalar(0), 2);

        rotated.copyTo(result(Rect(displayW, 0, rotated_cols, rotated_rows)));
        putText(result, "Rotated 90°", Point(displayW + 10, 30), FONT_HERSHEY_SIMPLEX, 1, Scalar(0), 2);

        scaled.copyTo(result(Rect(0, displayH, scaled_cols, scaled_rows)));
        putText(result, "Scaled", Point(10, displayH + 30), FONT_HERSHEY_SIMPLEX, 1, Scalar(0), 2);

        scaled_rotated.copyTo(result(Rect(displayW, displayH, scaled_rotated_cols, scaled_rotated_rows)));
        putText(result, "Rotated+Scaled", Point(displayW + 10, displayH + 30), FONT_HERSHEY_SIMPLEX, 1, Scalar(0), 2);

        line(result, Point(displayW, 0), Point(displayW, 2 * displayH), Scalar(0), 2);
        line(result, Point(0, displayH), Point(2 * displayW, displayH), Scalar(0), 2);

        namedWindow("MPI Results", WINDOW_NORMAL);
        resizeWindow("MPI Results", 1200, 900);
        imshow("MPI Results", result);
        imwrite("original.jpg", original);
        imwrite("rotated.jpg", rotated);
        imwrite("scaled.jpg", scaled);
        imwrite("rotated_scaled.jpg", scaled_rotated);
        cout << "\nImages saved: original.jpg, rotated.jpg, scaled.jpg, rotated_scaled.jpg\n";
        cout << "Press any key to exit...\n";
        waitKey(0);
        destroyAllWindows();
    }

    delete[] original_data;
    delete[] rotated_local;
    delete[] scaled_local;
    delete[] rotated_data;
    delete[] both_local;

    MPI_Finalize();
    return 0;
}