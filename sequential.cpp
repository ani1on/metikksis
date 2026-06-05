// #include <iostream>
// #include <chrono>
// #include <opencv2/opencv.hpp>
// #include <iomanip>
// #include "image_tools.hpp"

// using namespace std;
// using namespace cv;

// int main(int argc, char* argv[]) {
//     string inputFile = "input.jpg";
//     float scaleFactor = 0.5f;

//     if (argc > 1) inputFile = argv[1];
//     if (argc > 2) scaleFactor = stof(argv[2]);

//     cout << "\n=== Sequential Image Processing (custom functions) ===\n";
//     cout << "Input file: " << inputFile << "\n";
//     cout << "Scale factor: " << scaleFactor << "\n\n";
//     cout << fixed << setprecision(3);

//     auto total_start = chrono::high_resolution_clock::now();

//     // ---------- Загрузка ----------
//     auto t1 = chrono::high_resolution_clock::now();
//     Mat original = imread(inputFile, IMREAD_GRAYSCALE);
//     if (original.empty()) {
//         cerr << "Error: cannot load image " << inputFile << endl;
//         return -1;
//     }
//     auto t2 = chrono::high_resolution_clock::now();
//     double load_ms = chrono::duration<double, milli>(t2 - t1).count();
//     cout << "Load: " << load_ms << " ms\n";
//     cout << "Original size: " << original.cols << "x" << original.rows << "\n";

//     // ---------- Поворот (самодельный) ----------
//     t1 = chrono::high_resolution_clock::now();
//     Mat rotated;
//     rotate90Full(original, rotated);
//     t2 = chrono::high_resolution_clock::now();
//     double rotate_ms = chrono::duration<double, milli>(t2 - t1).count();
//     cout << "Rotate 90°: " << rotate_ms << " ms\n";
//     cout << "Rotated size: " << rotated.cols << "x" << rotated.rows << "\n";

//     // ---------- Масштабирование исходного (самодельное) ----------
//     t1 = chrono::high_resolution_clock::now();
//     Mat scaled;
//     scaleFull(original, scaled, scaleFactor);
//     t2 = chrono::high_resolution_clock::now();
//     double scale_ms = chrono::duration<double, milli>(t2 - t1).count();
//     cout << "Scale original: " << scale_ms << " ms\n";
//     cout << "Scaled size: " << scaled.cols << "x" << scaled.rows << "\n";

//     // ---------- Масштабирование повёрнутого (цепочка) ----------
//     t1 = chrono::high_resolution_clock::now();
//     Mat rotated_scaled;
//     scaleFull(rotated, rotated_scaled, scaleFactor);
//     t2 = chrono::high_resolution_clock::now();
//     double both_ms = chrono::duration<double, milli>(t2 - t1).count();
//     cout << "Scale rotated (rotate+scale): " << both_ms << " ms\n";
//     cout << "Rotated+scaled size: " << rotated_scaled.cols << "x" << rotated_scaled.rows << "\n";

//     // ---------- Построение полотна 2x2 ----------
//     t1 = chrono::high_resolution_clock::now();
//     int maxW = max({original.cols, rotated.cols, scaled.cols, rotated_scaled.cols});
//     int maxH = max({original.rows, rotated.rows, scaled.rows, rotated_scaled.rows});
//     Mat canvas(maxH * 2, maxW * 2, CV_8UC1, Scalar(255));

//     original.copyTo(canvas(Rect(0, 0, original.cols, original.rows)));
//     putText(canvas, "Original", Point(10, 30), FONT_HERSHEY_SIMPLEX, 1, Scalar(0), 2);
//     putText(canvas, to_string(original.cols)+"x"+to_string(original.rows), Point(10, 60), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0), 1);

//     rotated.copyTo(canvas(Rect(maxW, 0, rotated.cols, rotated.rows)));
//     putText(canvas, "Rotated 90°", Point(maxW + 10, 30), FONT_HERSHEY_SIMPLEX, 1, Scalar(0), 2);
//     putText(canvas, to_string(rotated.cols)+"x"+to_string(rotated.rows), Point(maxW + 10, 60), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0), 1);

//     scaled.copyTo(canvas(Rect(0, maxH, scaled.cols, scaled.rows)));
//     putText(canvas, "Scaled", Point(10, maxH + 30), FONT_HERSHEY_SIMPLEX, 1, Scalar(0), 2);
//     putText(canvas, to_string(scaled.cols)+"x"+to_string(scaled.rows), Point(10, maxH + 60), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0), 1);

//     rotated_scaled.copyTo(canvas(Rect(maxW, maxH, rotated_scaled.cols, rotated_scaled.rows)));
//     putText(canvas, "Rotated+Scaled", Point(maxW + 10, maxH + 30), FONT_HERSHEY_SIMPLEX, 1, Scalar(0), 2);
//     putText(canvas, to_string(rotated_scaled.cols)+"x"+to_string(rotated_scaled.rows), Point(maxW + 10, maxH + 60), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0), 1);

//     line(canvas, Point(maxW, 0), Point(maxW, maxH*2), Scalar(0), 2);
//     line(canvas, Point(0, maxH), Point(maxW*2, maxH), Scalar(0), 2);
//     t2 = chrono::high_resolution_clock::now();
//     double canvas_ms = chrono::duration<double, milli>(t2 - t1).count();
//     cout << "Create canvas: " << canvas_ms << " ms\n";

//     auto total_end = chrono::high_resolution_clock::now();
//     double total_ms = chrono::duration<double, milli>(total_end - total_start).count();
//     cout << "\n=== TOTAL TIME: " << total_ms << " ms ===\n";

//     imwrite("original.jpg", original);
//     imwrite("rotated.jpg", rotated);
//     imwrite("scaled.jpg", scaled);
//     imwrite("rotated_scaled.jpg", rotated_scaled);
//     imwrite("result_canvas.jpg", canvas);

//     namedWindow("Results", WINDOW_NORMAL);
//     resizeWindow("Results", 1200, 900);
//     imshow("Results", canvas);
//     cout << "\nImages saved: original.jpg, rotated.jpg, scaled.jpg, rotated_scaled.jpg, result_canvas.jpg\n";
//     cout << "Press any key to exit...\n";
//     waitKey(0);
//     destroyAllWindows();

//     return 0;
// }


#include <iostream>
#include <chrono>
#include <opencv2/opencv.hpp>
#include <iomanip>
#include "image_tools.hpp"

using namespace std;
using namespace cv;

int main(int argc, char* argv[]) {
    string inputFile = "input.jpg";
    float scaleFactor = 0.5f;

    if (argc > 1) inputFile = argv[1];
    if (argc > 2) scaleFactor = stof(argv[2]);

    cout << "\n=== Sequential Image Processing (custom functions) ===\n";
    cout << "Input file: " << inputFile << "\n";
    cout << "Scale factor: " << scaleFactor << "\n\n";
    cout << fixed << setprecision(3);

    auto total_start = chrono::high_resolution_clock::now();

    // ---------- Загрузка ----------
    auto t1 = chrono::high_resolution_clock::now();
    Mat original = imread(inputFile, IMREAD_GRAYSCALE);
    if (original.empty()) {
        cerr << "Error: cannot load image " << inputFile << endl;
        return -1;
    }
    auto t2 = chrono::high_resolution_clock::now();
    double load_ms = chrono::duration<double, milli>(t2 - t1).count();
    cout << "Load: " << load_ms << " ms\n";
    cout << "Original size: " << original.cols << "x" << original.rows << "\n";

    // ---------- Поворот (самодельный) ----------
    t1 = chrono::high_resolution_clock::now();
    Mat rotated;
    rotate90Full(original, rotated);
    t2 = chrono::high_resolution_clock::now();
    double rotate_ms = chrono::duration<double, milli>(t2 - t1).count();
    cout << "Rotate 90°: " << rotate_ms << " ms\n";
    cout << "Rotated size: " << rotated.cols << "x" << rotated.rows << "\n";

    // ---------- Масштабирование исходного (самодельное) ----------
    t1 = chrono::high_resolution_clock::now();
    Mat scaled;
    scaleFull(original, scaled, scaleFactor);
    t2 = chrono::high_resolution_clock::now();
    double scale_ms = chrono::duration<double, milli>(t2 - t1).count();
    cout << "Scale original: " << scale_ms << " ms\n";
    cout << "Scaled size: " << scaled.cols << "x" << scaled.rows << "\n";

    // ---------- Масштабирование повёрнутого (цепочка) ----------
    t1 = chrono::high_resolution_clock::now();
    Mat rotated_scaled;
    scaleFull(rotated, rotated_scaled, scaleFactor);
    t2 = chrono::high_resolution_clock::now();
    double both_ms = chrono::duration<double, milli>(t2 - t1).count();
    cout << "Scale rotated (rotate+scale): " << both_ms << " ms\n";
    cout << "Rotated+scaled size: " << rotated_scaled.cols << "x" << rotated_scaled.rows << "\n";

    // ---------- Построение полотна 2x2 ----------
    t1 = chrono::high_resolution_clock::now();
    int maxW = max({original.cols, rotated.cols, scaled.cols, rotated_scaled.cols});
    int maxH = max({original.rows, rotated.rows, scaled.rows, rotated_scaled.rows});
    Mat canvas(maxH * 2, maxW * 2, CV_8UC1, Scalar(255));

    original.copyTo(canvas(Rect(0, 0, original.cols, original.rows)));
    putText(canvas, "Original", Point(10, 30), FONT_HERSHEY_SIMPLEX, 1, Scalar(0), 2);
    putText(canvas, to_string(original.cols)+"x"+to_string(original.rows), Point(10, 60), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0), 1);

    rotated.copyTo(canvas(Rect(maxW, 0, rotated.cols, rotated.rows)));
    putText(canvas, "Rotated 90°", Point(maxW + 10, 30), FONT_HERSHEY_SIMPLEX, 1, Scalar(0), 2);
    putText(canvas, to_string(rotated.cols)+"x"+to_string(rotated.rows), Point(maxW + 10, 60), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0), 1);

    scaled.copyTo(canvas(Rect(0, maxH, scaled.cols, scaled.rows)));
    putText(canvas, "Scaled", Point(10, maxH + 30), FONT_HERSHEY_SIMPLEX, 1, Scalar(0), 2);
    putText(canvas, to_string(scaled.cols)+"x"+to_string(scaled.rows), Point(10, maxH + 60), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0), 1);

    rotated_scaled.copyTo(canvas(Rect(maxW, maxH, rotated_scaled.cols, rotated_scaled.rows)));
    putText(canvas, "Rotated+Scaled", Point(maxW + 10, maxH + 30), FONT_HERSHEY_SIMPLEX, 1, Scalar(0), 2);
    putText(canvas, to_string(rotated_scaled.cols)+"x"+to_string(rotated_scaled.rows), Point(maxW + 10, maxH + 60), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0), 1);

    line(canvas, Point(maxW, 0), Point(maxW, maxH*2), Scalar(0), 2);
    line(canvas, Point(0, maxH), Point(maxW*2, maxH), Scalar(0), 2);
    t2 = chrono::high_resolution_clock::now();
    double canvas_ms = chrono::duration<double, milli>(t2 - t1).count();
    cout << "Create canvas: " << canvas_ms << " ms\n";

    // ---------- Вычисление общего времени обработки (сумма поворота и масштабирования) ----------
    double total_processing_time = rotate_ms + scale_ms;
    
    auto total_end = chrono::high_resolution_clock::now();
    double total_wall_time = chrono::duration<double, milli>(total_end - total_start).count();
    
    // Вывод результатов
    cout << "\n========================================\n";
    cout << "=== PROCESSING TIME BREAKDOWN ===\n";
    cout << "========================================\n";
    cout << "Rotation time           : " << rotate_ms << " ms\n";
    cout << "Scaling time            : " << scale_ms << " ms\n";
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    cout << "TOTAL processing time   : " << total_processing_time << " ms\n";
    cout << "(Sum of rotation + scaling)\n";
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    cout << "Additional operations:\n";
    cout << "  - Image loading       : " << load_ms << " ms\n";
    cout << "  - Canvas creation     : " << canvas_ms << " ms\n";
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    cout << "Total wall-clock time   : " << total_wall_time << " ms\n";
    cout << "========================================\n";

    // Сохранение изображений
    imwrite("original.jpg", original);
    imwrite("rotated.jpg", rotated);
    imwrite("scaled.jpg", scaled);
    imwrite("rotated_scaled.jpg", rotated_scaled);
    imwrite("result_canvas.jpg", canvas);

    namedWindow("Results", WINDOW_NORMAL);
    resizeWindow("Results", 1200, 900);
    imshow("Results", canvas);
    cout << "\nImages saved: original.jpg, rotated.jpg, scaled.jpg, rotated_scaled.jpg, result_canvas.jpg\n";
    cout << "Press any key to exit...\n";
    waitKey(0);
    destroyAllWindows();

    return 0;
}