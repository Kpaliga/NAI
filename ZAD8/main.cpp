#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

using namespace std;

bool compareContourAreas ( vector<cv::Point> contour1, vector<cv::Point> contour2 ) {
    double i = fabs( contourArea(cv::Mat(contour1)) );
    double j = fabs( contourArea(cv::Mat(contour2)) );
    return ( i < j );
}

int main( int argc, char** argv ) {
    bool capturing = true;
    cv::VideoCapture cap;

    int deviceID = 0;             // 0 = open default camera
    int apiID = cv::CAP_ANY;      // 0 = autodetect default API
    // open selected camera using selected API
    cap.open(deviceID, apiID);
    if ( !cap.isOpened() ) {
        cerr << "error opening frames source" << endl;
        return -1;
    }
    cout << "Video size: " << cap.get( cv::CAP_PROP_FRAME_WIDTH )
    << "x" << cap.get( cv::CAP_PROP_FRAME_HEIGHT ) << endl;
    namedWindow("detected", cv::WINDOW_AUTOSIZE);
    //gfg 0 ,141, 87 - 15, 255, 255
    vector<int> lower = {0,141,87};
    vector<int> upper = {15,255,255};
    cv::createTrackbar("lh", "detected", &lower[0], 255);
    cv::createTrackbar("ls", "detected", &lower[1], 255);
    cv::createTrackbar("lv", "detected", &lower[2], 255);
    cv::createTrackbar("hh", "detected", &upper[0], 255);
    cv::createTrackbar("hs", "detected", &upper[1], 255);
    cv::createTrackbar("hv", "detected", &upper[2], 255);
    do {
        cv::Mat frame;
        if ( cap.read( frame ) ) {
            cv::flip(frame,frame,1);

            cv::Mat gray_image;
            GaussianBlur(frame, frame, cv::Size(3, 3), 0);
            cvtColor( frame, gray_image, cv::COLOR_BGR2HSV );
            cv::Mat imgThresholded;

            inRange(gray_image, cv::Scalar(lower[0], lower[1], lower[2]), cv::Scalar(upper[0], upper[1], upper[2]), imgThresholded);
            auto kernel = getStructuringElement(cv::MORPH_ELLIPSE,cv::Size{5,5});

            cv::Mat eroded;
            dilate(imgThresholded, eroded, kernel);

            vector<vector<cv::Point> > contours;
            vector<cv::Vec4i> hierarchy;
            findContours(eroded, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0) );

            sort(contours.begin(), contours.end(), compareContourAreas);
            if (contours.size() >= 2) {
                vector<cv::Point> BiggestContour = contours[contours.size() - 1];
                vector<cv::Point> SecondBiggestContour = contours[contours.size() - 2];
                auto m = moments(BiggestContour, false);
                cv::Point p1 = {(int) (m.m10 / m.m00), (int) (m.m01 / m.m00)};
                int x = (int) (m.m10 / m.m00);
                int y = (int) (m.m01 / m.m00);


                auto m2 = moments(SecondBiggestContour, false);
                int y2 = (int) (m2.m01 / m2.m00);
                cv::Point p2 = {(int) (m2.m10 / m2.m00), (int) (m2.m01 / m2.m00)};


                if (abs(y2 - y) < 15) {
                    line(frame, p1, p2, cv::Scalar(255, 0, 0),
                         10, cv::LINE_8);
                }
            }



            cv::imshow("zakresy", eroded);
            cv::imshow("flipped",frame);



        } else {
            capturing = false;
        }
        if( (cv::waitKey( 1000.0/60.0 )&0x0ff) == 27 ) capturing = false;
    } while( capturing );
    return 0;
}