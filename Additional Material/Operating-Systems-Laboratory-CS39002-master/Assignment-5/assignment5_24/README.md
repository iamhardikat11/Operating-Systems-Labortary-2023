# Creating a Memory Management System

The header file of the library is `memlab.h` and the source file is `memlab.cpp`. There are five demo files `demo1.cpp`, `demo2.cpp`, `demo3.cpp`, `demo4.cpp` and `demo5.cpp`. The folder `gc-results` contains memory footprint results with and without using the garbage collector. The design considerations and results are detailed in `report.pdf`.

## Execution Instructions
For running with logs (e.g. `demo1`):
```
make
./demo1
```

For running without logs (e.g. `demo1`):
```
make CFLAGS=""
./demo1
```