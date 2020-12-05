#include "popt_pp.h"
#include <iostream>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <stdio.h>
#include <sys/stat.h>

using namespace std;
using namespace cv;

vector<vector<Point3f>> object_points;
vector<vector<Point2f>> image_points;
vector<Point2f> corners;
vector<vector<Point2f>> left_img_points;

Size im_size;

bool doesExist(const std::string &name) {
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}

void setup_calibration(int board_width, int board_height, int num_imgs, float square_size, const char *imgs_directory,
                       const char *imgs_filename, const char *extension) {
    Size board_size = Size(board_width, board_height);
    int board_n = board_width * board_height;

    Mat img, gray;
    for (int k = 1; k <= num_imgs; k++) {
        char img_file[100];
        //棋盘格图片
        sprintf(img_file, "%s%s%d.%s", imgs_directory, imgs_filename, k, extension);
        if (!doesExist(img_file))
            continue;
        img = imread(img_file, CV_LOAD_IMAGE_COLOR);
        cv::cvtColor(img, gray, CV_BGR2GRAY);
        // add blur
        cv::GaussianBlur(gray, gray, cv::Size(3, 3), 0);

        im_size = img.size(); //传出

        //角点检测
        bool found = false;
        found =
            cv::findChessboardCorners(img, board_size, corners, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FILTER_QUADS);
        if (found) {
            cornerSubPix(gray, corners, cv::Size(5, 5), cv::Size(-1, -1),
                         TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 30, 0.1));
            //显示角点
            Mat img_corners = img.clone();
            drawChessboardCorners(img_corners, board_size, corners, found);
            imshow("img_corners", img_corners);
            waitKey(100);
            //保存角点
            char img_corn_file[100];
            sprintf(img_corn_file, "%s_corners%d.%s", imgs_filename, k, extension);
            imwrite(img_corn_file, img_corners);
        }

        //角点的世界坐标系
        vector<Point3f> obj;
        for (int i = 0; i < board_height; i++)
            for (int j = 0; j < board_width; j++)
                obj.push_back(Point3f((float)j * square_size, (float)i * square_size, 0)); //单位m/mm对应双目T基线单位??

        if (found) {
            cout << k << ". Found corners!" << endl;
            image_points.push_back(corners);
            object_points.push_back(obj);
        }
    }
}

double computeReprojectionErrors(const vector<vector<Point3f>> &objectPoints,
                                 const vector<vector<Point2f>> &imagePoints, const vector<Mat> &rvecs,
                                 const vector<Mat> &tvecs, const Mat &cameraMatrix, const Mat &distCoeffs) {
    vector<Point2f> imagePoints2;
    int i, totalPoints = 0;
    double totalErr = 0, err;
    vector<float> perViewErrors;
    perViewErrors.resize(objectPoints.size());

    for (i = 0; i < (int)objectPoints.size(); ++i) {
        projectPoints(Mat(objectPoints[i]), rvecs[i], tvecs[i], cameraMatrix, distCoeffs, imagePoints2);
        err = norm(Mat(imagePoints[i]), Mat(imagePoints2), CV_L2);
        int n = (int)objectPoints[i].size();
        perViewErrors[i] = (float)std::sqrt(err * err / n);
        totalErr += err * err;
        totalPoints += n;
    }
    return std::sqrt(totalErr / totalPoints);
}

int main(int argc, char const **argv) {
    int board_width = 7, board_height = 5;
    float square_size = 0.05;
    int num_imgs = 27;
    const char *imgs_directory = "../calib_imgs/stereoImg/";
    const char *imgs_filename = "left"; //单独标左相机
    const char *out_file = "cam_left.yml";
    const char *extension = "jpg";

    // 解析命令行参数
    static struct poptOption options[] = {
        {"board_width", 'w', POPT_ARG_INT, &board_width, 0, "Checkerboard width", "NUM"},
        {"board_height", 'h', POPT_ARG_INT, &board_height, 0, "Checkerboard height", "NUM"},
        {"square_size", 's', POPT_ARG_FLOAT, &square_size, 0, "Size of checkerboard square", "NUM"},
        {"num_imgs", 'n', POPT_ARG_INT, &num_imgs, 0, "Number of checkerboard images", "NUM"},
        {"imgs_directory", 'd', POPT_ARG_STRING, &imgs_directory, 0, "Directory containing images", "STR"},
        {"imgs_filename", 'i', POPT_ARG_STRING, &imgs_filename, 0, "Image filename", "STR"},
        {"extension", 'e', POPT_ARG_STRING, &extension, 0, "Image extension", "STR"},
        {"out_file", 'o', POPT_ARG_STRING, &out_file, 0, "Output calibration filename (YML)", "STR"},
        POPT_AUTOHELP{NULL, 0, 0, NULL, 0, NULL, NULL}};
    POpt popt(NULL, argc, argv, options, 0);
    int c;
    while ((c = popt.getNextOpt()) >= 0) {
    }

    setup_calibration(board_width, board_height, num_imgs, square_size, imgs_directory, imgs_filename, extension);

    printf("--Starting Calibration\n");
    Mat K; //摄像机内参
    Mat D; //畸变系数
    vector<Mat> rvecs, tvecs; //存储每张图片的旋量向量和平移向量
    int flag = 0; // todo ??
    flag |= CV_CALIB_FIX_K3;
    // flag |= CV_CALIB_FIX_K4;
    // flag |= CV_CALIB_FIX_K5;
    //单目标定
    calibrateCamera(object_points, image_points, im_size, K, D, rvecs, tvecs, flag);

    //误差
    cout << "--Calibration error: " << computeReprojectionErrors(object_points, image_points, rvecs, tvecs, K, D)
         << endl;

    //保存标定结果
    FileStorage fs(out_file, FileStorage::WRITE);
    fs << "K" << K;
    fs << "D" << D;
    fs << "board_width" << board_width;
    fs << "board_height" << board_height;
    fs << "square_size" << square_size;
    printf("--Done Calibration\n");

    //去畸变验证
    char img1_file[100];
    sprintf(img1_file, "%s%s%d.%s", imgs_directory, imgs_filename, 1, extension);
    Mat img_0 = imread(img1_file, CV_LOAD_IMAGE_COLOR);
    Mat img_undist;
    undistort(img_0, img_undist, K, D);
    cout << "K: \n" << K << endl;
    cout << "D: \n" << D << endl;
    imshow("去畸变验证1", img_undist);
    waitKey(1000);

    return 0;
}
