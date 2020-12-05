#include "popt_pp.h"
#include <iostream>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <stdio.h>

using namespace std;
using namespace cv;

int main(int argc, char const *argv[]) {
    const char *leftimg_filename = "../calib_imgs/stereoImg/left1.jpg";
    const char *rightimg_filename = "../calib_imgs/stereoImg/right1.jpg";
    const char *calib_file = "cam_stereo.yml";
    const char *leftout_filename = "left_rectify.jpg";
    const char *rightout_filename = "right_rectify.jpg";

    // 解析命令行参数
    static struct poptOption options[] = {
        {"leftimg_filename", 'l', POPT_ARG_STRING, &leftimg_filename, 0, "Left imgage path", "STR"},
        {"rightimg_filename", 'r', POPT_ARG_STRING, &rightimg_filename, 0, "Right image path", "STR"},
        {"calib_file", 'c', POPT_ARG_STRING, &calib_file, 0, "Stereo calibration file", "STR"},
        {"leftout_filename", 'L', POPT_ARG_STRING, &leftout_filename, 0, "Left undistorted imgage path", "STR"},
        {"rightout_filename", 'R', POPT_ARG_STRING, &rightout_filename, 0, "Right undistorted image path", "STR"},
        POPT_AUTOHELP{NULL, 0, 0, NULL, 0, NULL, NULL}};

    POpt popt(NULL, argc, argv, options, 0);
    int c;
    while ((c = popt.getNextOpt()) >= 0) {
    }

    Mat R1, R2, P1, P2, Q;
    Mat K1, K2, R;
    Vec3d T;
    Mat D1, D2;
    Mat imgL = imread(leftimg_filename, CV_LOAD_IMAGE_COLOR);
    Mat imgR = imread(rightimg_filename, CV_LOAD_IMAGE_COLOR);

    //读取标定参数
    cv::FileStorage fs1(calib_file, cv::FileStorage::READ);
    fs1["K1"] >> K1;
    fs1["K2"] >> K2;
    fs1["D1"] >> D1;
    fs1["D2"] >> D2;
    fs1["R"] >> R;
    fs1["T"] >> T;

    fs1["R1"] >> R1;
    fs1["R2"] >> R2;
    fs1["P1"] >> P1;
    fs1["P2"] >> P2;
    fs1["Q"] >> Q;

    cv::Mat lmapx, lmapy, rmapx, rmapy;
    cv::Mat imgU_Left, imgU_Right;

    //摄像机校正映射
    cv::initUndistortRectifyMap(K1, D1, R1, P1, imgL.size(), CV_32F, lmapx, lmapy);
    cv::initUndistortRectifyMap(K2, D2, R2, P2, imgR.size(), CV_32F, rmapx, rmapy);

    imshow("before Rectify imgL", imgL);
    imshow("before Rectify imgR", imgR);

    //经过remap之后，左右相机的图像已经共面并且行对准了
    cv::remap(imgL, imgU_Left, lmapx, lmapy, cv::INTER_LINEAR);
    cv::remap(imgR, imgU_Right, rmapx, rmapy, cv::INTER_LINEAR);

    imshow("after Rectify imgU_Left", imgU_Left);
    imshow("after Rectify imgU_Right", imgU_Right);

    /*画上对应的线条*/
    for (int i = 0; i < imgU_Left.rows; i += 32) {
        line(imgU_Left, Point(0, i), Point(imgU_Left.cols, i), Scalar(0, 255, 0), 1, 8);
        line(imgU_Right, Point(0, i), Point(imgU_Right.cols, i), Scalar(0, 255, 0), 1, 8);
    }
    imshow("rectified imgU_Left", imgU_Left);
    imshow("rectified imgU_Right", imgU_Right);

    imwrite(leftout_filename, imgU_Left);
    imwrite(rightout_filename, imgU_Right);

    waitKey(1000);

    /*
    把校正结果显示出来
    把左右两幅图像显示到同一个画面上
    这里只显示了最后一副图像的校正结果。并没有把所有的图像都显示出来
    */
    // Size imageSize = Size(imgL.rows, imgL.cols);
    // Mat canvas;
    // double sf;
    // int w, h;
    // sf = 600. / MAX(imageSize.width, imageSize.height);
    // w = cvRound(imageSize.width * sf);
    // h = cvRound(imageSize.height * sf);
    // canvas.create(h, w * 2, CV_8UC3);

    // /*左图像画到画布上*/
    // Mat canvasPart = canvas(Rect(w * 0, 0, w, h)); //得到画布的一部分
    // resize(imgU_Left, canvasPart, canvasPart.size(), 0, 0, INTER_AREA); //把图像缩放到跟canvasPart一样大小
    // Rect vroiL(cvRound(validROIL.x * sf), cvRound(validROIL.y * sf), //获得被截取的区域
    //            cvRound(validROIL.width * sf), cvRound(validROIL.height * sf));
    // rectangle(canvasPart, vroiL, Scalar(0, 0, 255), 3, 8); //画上一个矩形
    // cout << "Painted ImageL" << endl;

    // /*右图像画到画布上*/
    // canvasPart = canvas(Rect(w, 0, w, h)); //获得画布的另一部分
    // resize(imgU_Right, canvasPart, canvasPart.size(), 0, 0, INTER_LINEAR);
    // Rect vroiR(cvRound(validROIR.x * sf), cvRound(validROIR.y * sf), cvRound(validROIR.width * sf),
    //            cvRound(validROIR.height * sf));
    // rectangle(canvasPart, vroiR, Scalar(0, 255, 0), 3, 8);
    // cout << "Painted ImageR" << endl;

    // /*画上对应的线条*/
    // for (int i = 0; i < canvas.rows; i += 16)
    //     line(canvas, Point(0, i), Point(canvas.cols, i), Scalar(0, 255, 0), 1, 8);
    // imshow("rectified", canvas);

    return 0;
}
