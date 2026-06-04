#ПОДТЯГИВАНИЕ ЗАВИСИМОСТЕЙ:
    ОБЯЗАТЕЛЬНО wsl [https://learn.microsoft.com/ru-ru/windows/wsl/install]

    
    sudo apt install g++ make openmpi-bin openmpi-common libopenmpi-dev libopencv-dev


#ПОСЛЕДОВАТЕЛЬНАЯ ОБРАБОТКА:
    сборка проекта:
        g++ -std=c++11 sequential.cpp image_tools.cpp -o sequential `pkg-config --cflags --libs opencv4`
    
    Запуск:
        ./sequential image.jpg 0.5


#ПАРАЛЛЕЛЬНАЯ ОБРАБОТКА:


    сборка проекта:
        mpic++ -std=c++11 parallel.cpp image_tools.cpp -o parallel `pkg-config --cflags --libs opencv4`

    Запуск файла:
        mpirun -np 4 --oversubscribe ./parallel image.jpg 0.5





#=== Sequential Image Processing (custom functions) ===
Input file: image.jpg
Scale factor: 0.5

Load: 21.179 ms
Original size: 959x959
Rotate 90°: 4.696 ms
Rotated size: 959x959
Scale original: 6.503 ms
Scaled size: 479x479
Scale rotated (rotate+scale): 7.401 ms
Rotated+scaled size: 479x479
Create canvas: 3.352 ms

=== TOTAL TIME: 43.419 ms ===

Images saved: original.jpg, rotated.jpg, scaled.jpg, rotated_scaled.jpg, result_canvas.jpg
Press any key to exit...



#=== MPI Parallel Image Processing ===
Processes: 4
Image: 959x959
Scale factor: 0.5

=== Processing Times ===
Rotation        : 4.45731 ms
Scaling         : 2.87203 ms
Rotate+Scale    : 4.50907 ms
#Total           : 25.941 ms

Images saved: original.jpg, rotated.jpg, scaled.jpg, rotated_scaled.jpg
Press any key to exit...
