# Image saving using multiple ZED

This code is the modified version of the given sample from Sterolabs ZED. It shows three output images(LEFT, RIGHT, DEPTH) for the multiple ZED cameras and save them as an image file(.png .tiff .. etc).

Link: https://github.com/stereolabs/zed-multi-camera

## How to use

### Prerequisites

- Download the latest version of the ZED SDK on [stereolabs.com](https://www.stereolabs.com).
- Ubuntu LTS

### Build for Linux

Open a terminal in the sample directory and execute the following command:

    mkdir build
    cd build
    cmake ..
    make

### Run the program

Create data directory to save the image data from the multiple ZED.
Change 'string file_dir' in main.cpp to change the direcory for image saving.
Images are saved in /file_dir/CAMERA_NUM/IMAGE_TYPE

ex: zed-multi-camera/data/0/LEFT
ex: zed-multi-camera/data/1/DEPTH

    cd build
    ./ZED\ Multi\ Camera

