## OpenCV C++ Stereo Camera Calibration

This repository contains some sources to calibrate the intrinsics of individual cameras and also the extrinsics of a stereo pair.

### Dependencies

- OpenCV
- popt
```bash
sudo apt install libopencv-dev
sudo apt install libpopt-dev
```
### Compilation

Compile all the files using the following commands.

```bash
mkdir build && cd build
cmake ..
make
```

Make sure your are in the `build` folder to run the executables.

### Get images from webcams

This is a small helper tool to grab frames from two webcams operating as a stereo pair. Run the following command to use it.

```bash
./read -w [img_width] -h [img_height] -d [imgs_directory] -e [file_extension]
```

For example,

```bash
./read -w 1280 -h 720 -d ./calib_imgs -e jpg
```

Once it is running, hit any key to grab frames. Images are saved with prefixes `left` and `right` in the desired directory.

### Intrinsic calibration of a single camera

This is only for lenses which follow the pinhole model. If you have fisheye lenses with a very wide field of view then see [this](https://github.com/sourishg/fisheye_stereo_calibration) repository. The calibration saves the camera matrix and the distortion coefficients in a YAML file. The datatype for these matrices is `Mat`.

Once you have compiled the sources run the following command to calibrate the intrinsics.

```bash
./calibrate -w [board_width] -h [board_height] -s [square_size] -n [num_imgs] -d [imgs_directory] -i [imgs_filename] -o [file_extension] -e [output_filename]
```

For example, the command for the test images in `calib_imgs/stereoImg/` would be

left images
```bash
# ./calibrate -w 9 -h 6 -s 0.02423 -n 27 -d ../calib_imgs/stereoImg/ -i left -o cam_left.yml -e jpg
# 棋盘格大小: 7*5角点,50mm边长,(单位m/mm对应双目标定T基线单位??)
./calibrate -w 7 -h 5 -s 0.05 -n 27 -d ../calib_imgs/stereoImg/ -i left -o cam_left.yml -e jpg
```

right images
```bash
# ./calibrate -w 9 -h 6 -s 0.02423 -n 27 -d "../calib_imgs/stereoImg/" -i "right" -o "cam_right.yml" -e "jpg"
# 棋盘格大小: 7*5角点,50mm边长
./calibrate -w 7 -h 5 -s 0.05 -n 27 -d "../calib_imgs/stereoImg/" -i "right" -o "cam_right.yml" -e "jpg"
```

### Stereo calibration for extrinisics

Once you have the intrinsics calibrated for both the left and the right cameras, you can use their intrinsics to calibrate the extrinsics between them.

```bash
./calibrate_stereo -n [num_imgs] -u [left_cam_calib] -v [right_cam_calib] -L [left_img_dir] -R [right_img_dir] -l [left_img_prefix] -r [right_img_prefix] -o [output_calib_file] -e [file_extension]
```

For example, if you calibrated the left and the right cameras using the images in the `calib_imgs/stereoImg/` directory, the following command to compute the extrinsics.

```bash
./calibrate_stereo -n 27 -u cam_left.yml -v cam_right.yml -L ../calib_imgs/stereoImg/ -R ../calib_imgs/stereoImg/ -l left -r right -o cam_stereo.yml -e jpg
```

### Undistortion and Rectification

Once you have the stereo calibration data, you can remove the distortion and rectify any pair of images so that the resultant epipolar lines become scan lines.

```bash
./undistort_rectify -l [left_img_path] -r [right_img_path] -c [stereo_calib_file] -L [output_left_img] -R [output_right_img]
```

For example

```bash
./undistort_rectify -l ../calib_imgs/stereoImg/left1.jpg -r ../calib_imgs/stereoImg/right1.jpg -c cam_stereo.yml -L left_rectify.jpg -R right_rectify.jpg
```
