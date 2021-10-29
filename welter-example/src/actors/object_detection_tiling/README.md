## Getting Started

### Dependencies
* CMake
* OpenCV

### Build Instruction

Once all dependencies have been installed, the code can be compiled using the CMakeList.txt file included. For example:

```
mkdir build
cd build
cmake ..
make
```

This will produce the executable `yolo_object_detection`

### Example Usage

Be sure to have some example image and some yolo weight and configuration files before running the executable through the command line:

`./yolo_object_detection -model=../cfg/yolov3-tiny.weights -config=../cfg/yolov3-tiny.cfg -image=../images/000000574520.jpg`

