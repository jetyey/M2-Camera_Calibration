#include <sstream>
#include <iostream>
#include <fstream>
#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;

const float calibrationSquareDimension = 0.0280f; // meters
const Size chessboardDimensions = Size(9,6);

void createKnownBoardPosition(Size boardSize, float squareEdgeLength, vector<Point3f>& corners){
    for(int i =0; i< boardSize.height; i++){
        for(int j = 0; j<boardSize.width; j++){
            corners.push_back(Point3f(j*squareEdgeLength, i* squareEdgeLength, 0.0f));
        }
    }
}

void getChessboardCorners(vector<Mat> images, vector<vector<Point2f>>& allFoundCorners, bool showResults = false){
    
    for(vector<Mat>::iterator iter = images.begin(); iter != images.end(); iter++){
        vector<Point2f> pointBuf;
        bool found = findChessboardCorners(*iter, Size(9,6), pointBuf, CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_NORMALIZE_IMAGE);

        if(found){
            allFoundCorners.push_back(pointBuf);
        }

        if(showResults){
            drawChessboardCorners(*iter, Size(9,6), pointBuf, found);
            imshow("Looking for Corners", *iter);
            waitKey(0);
        }
    }
}

void cameraCalibration(vector<Mat> calibrationImages, Size boardSize, float squareEdgeLength, Mat& cameraMatrix, Mat& distortionCoefficients){
    
    vector<vector<Point2f>> checkerboardImageSpacePoints;
    getChessboardCorners(calibrationImages, checkerboardImageSpacePoints, false);

    vector<vector<Point3f>> worldSpaceCornerPoints(1);

    createKnownBoardPosition(boardSize, squareEdgeLength, worldSpaceCornerPoints[0]);
    worldSpaceCornerPoints.resize(checkerboardImageSpacePoints.size(), worldSpaceCornerPoints[0]);

    vector<Mat> rVectors, tVectors;
    distortionCoefficients = Mat::zeros(8, 1, CV_64F);

    calibrateCamera(worldSpaceCornerPoints, checkerboardImageSpacePoints, boardSize, cameraMatrix, distortionCoefficients, rVectors, tVectors);

}

bool savedCameraCalibration(string name, Mat cameraMatrix, Mat distortionCoefficients){
    ofstream outStream(name);
    if(outStream){
        uint16_t rows = cameraMatrix.rows;
        uint16_t columns = cameraMatrix.cols;

        outStream << "######## Camera Matrix ##########" << endl;
        for(int r=0; r<rows; r++){
            for(int c=0; c<columns; c++){
                double value = cameraMatrix.at<double>(r,c);
                outStream << value << "     ";
            }
            outStream << endl;
        }

        rows = distortionCoefficients.rows;
        columns = distortionCoefficients.cols;
        
        outStream << "######## Distortion Coefficients ##########" << endl;
        for(int r=0; r<rows; r++){
            for(int c=0; c<columns; c++){
                double value = distortionCoefficients.at<double>(r,c);
                outStream << value << "     ";
            }
            outStream << endl;
        }

        outStream.close();
        return true;
    }

    return false;
}

int main(){
    Mat frame;
    Mat drawtoFrame;
    int n = 1;
    vector<Mat> savedImages;

    Mat cameraMatrix = Mat::eye(3, 3, CV_64F);

    Mat distortionCoefficients;

    vector<vector<Point2f>> markedCorners, rejectedCandidates;

    VideoCapture vid(0);

    if(!vid.isOpened()){
        return 0;
    }

    int framePerSecond = 20;

    namedWindow("Webcam", WINDOW_AUTOSIZE);

    while (true){
        if(!vid.read(frame))
            break;

        vector<Vec2f> foundPoints;
        bool found =false;

        found = findChessboardCorners(frame, chessboardDimensions, foundPoints, CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_NORMALIZE_IMAGE);
        frame.copyTo(drawtoFrame);
        drawChessboardCorners(drawtoFrame, chessboardDimensions, foundPoints, found);
        if(found){
            imshow("Webcam", drawtoFrame);
        }else{
            imshow("Webcam", frame);
        }
        char character = waitKey(1000/framePerSecond);

        switch(character){

            case ' ':
                //saving image
                if(found){
                    Mat temp;
                    frame.copyTo(temp);
                    savedImages.push_back(temp);
                    cout << "saved " << n << " image" << endl;
                    n++;
                }
                break;
            case 's':
                //start callibration
                if(savedImages.size()>=15){
                    cameraCalibration(savedImages, chessboardDimensions, calibrationSquareDimension, cameraMatrix, distortionCoefficients);
                    savedCameraCalibration("CameraCalibration.txt", cameraMatrix, distortionCoefficients);
                    cout << "Start camera calib" << endl;
                    return 0;
                }
                break;
            case 27:
                //Exit Program
                return 0;
                break;
        }
        
    }

    return 0;
}