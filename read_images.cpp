#include "popt_pp.h"
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <sstream>
#include <stdio.h>
#include <string>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std;
using namespace cv;

int main(int argc, char const *argv[]) {

    //图片尺寸
    int im_width = 1280;
    int im_height = 720;
    //图片存储位置
    const char *imgs_directory = "./calib_imgs/";
    //图片后缀
    const char *extension = "jpg";

    // string command;
    // command = "mkdir -p " + string(imgs_directory) + "/left";
    // system(command.c_str());
    // command = "mkdir -p " + string(imgs_directory) + "/right";
    // system(command.c_str());

    if (0 != access(imgs_directory, 00)) { // 00表示只判断是否存在
        mkdir(imgs_directory, 0777); // 创建文件夹
    }
    // // todo 删除目录,只能删除空目录??
    // // rmdir(imgs_directory); // warning
    // // ./calib_imgs/left或right文件夹必须存在
    // if (0 != access(imgs_directory, 00)) { // 00表示只判断是否存在
    //     mkdir(imgs_directory, 0777); // 创建文件夹
    //     mkdir((string(imgs_directory) + "/left").c_str(), 0777); // 创建文件夹
    //     mkdir((string(imgs_directory) + "/right").c_str(), 0777);
    // } else if (0 != access((string(imgs_directory) + "/left").c_str(), 00)) { // 00表示只判断是否存在
    //     mkdir((string(imgs_directory) + "/left").c_str(), 0777); // 创建文件夹
    //     mkdir((string(imgs_directory) + "/right").c_str(), 0777);
    // }

    // 解析命令行参数
    static struct poptOption options[] = {
        {"img_width", 'w', POPT_ARG_INT, &im_width, 0, "Image width", "NUM"},
        {"img_height", 'h', POPT_ARG_INT, &im_height, 0, "Image height", "NUM"},
        {"imgs_directory", 'd', POPT_ARG_STRING, &imgs_directory, 0, "Directory to save images in", "STR"},
        {"extension", 'e', POPT_ARG_STRING, &extension, 0, "Image extension", "STR"},
        POPT_AUTOHELP{NULL, 0, 0, NULL, 0, NULL, NULL}};
    POpt popt(NULL, argc, argv, options, 0);
    int c;
    while ((c = popt.getNextOpt()) >= 0) {
    }

    //单张图片尺寸
    cv::Size2i image_size = cv::Size2i(im_width, im_height); // HD720(1280, 720)

    // Open the ZED camera
    VideoCapture cap(2); // laptops 0
    if (!cap.isOpened())
        return -1;
    cap.grab();
    // Set the video resolution (2*Width * Height) // HD720(1280,720)
    cap.set(cv::CAP_PROP_FRAME_WIDTH, image_size.width * 2); // 1280*2
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, image_size.height); // 720
    cap.grab();

    //获取的图像
    cv::Mat frame_raw;
    cv::Mat left_raw, right_raw;

    int cnt_img = 1;
    char key = '\0';
    while (key != 'q') {
        // Get a new frame from camera
        cap >> frame_raw;
        // Extract left and right images from side-by-side
        left_raw = frame_raw(cv::Rect(0, 0, frame_raw.cols / 2, frame_raw.rows));
        right_raw = frame_raw(cv::Rect(frame_raw.cols / 2, 0, frame_raw.cols / 2, frame_raw.rows));
        // Display images
        imshow("frame_raw RAW", frame_raw);
        imshow("left RAW", left_raw);
        imshow("right RAW", right_raw);

        //空格保存
        if (key == ' ') {
            char filename1[200], filename2[200];
            sprintf(filename1, "%sleft%d.%s", imgs_directory, cnt_img, extension);
            sprintf(filename2, "%sright%d.%s", imgs_directory, cnt_img, extension);
            cout << "Saving img pair " << cnt_img << endl;
            imwrite(filename1, left_raw);
            imwrite(filename2, right_raw);
            waitKey(10);
            cnt_img++;
        }

        key = waitKey(10);
    }

    return 0;
}
