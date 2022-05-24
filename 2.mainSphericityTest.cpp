#include <windows.h>
#include "CircularityDetector.h"


#include <iostream>
#include <fstream>

#include "opencv2/core/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/calib3d.hpp"
#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>


void main(void) {

//0. Load image
cv::VideoCapture cap(0);
cap.set(cv::CAP_PROP_AUTOFOCUS, 0);
cap.set(cv::CAP_PROP_AUTO_EXPOSURE, 0);
cap.set(cv::CAP_PROP_FRAME_WIDTH, 1024);//we can use 1920 x 1080
cap.set(cv::CAP_PROP_FRAME_HEIGHT, 576);
cv::Mat frame, sourceImage; // creates just the header parts 
//1. Create our detector
CircularityDetector detector(100,60,2);
int discardFirstFrames = 20;

//2. Discard a few frames (when you wake the camera, the first few frames are darked)
while (discardFirstFrames--)
	cap>> frame;

//3. Run our detection cycle
while (true) {
	cap >> frame;
	cv::cvtColor(frame, sourceImage, cv::COLOR_BGR2GRAY);
	//cv::imshow("Webcam", sourceImage);
	detector.detect(sourceImage);
	cv::waitKey(1);
}

/*while (true) {
	cap >> frame;
	cv::cvtColor(frame, sourceImage, cv::COLOR_BGR2GRAY);
	//detectUsingSimpleBlobDetector(sourceImage); //NOTHING DETECTED (WTF!)
	detectRectangle(sourceImage);
	detectUsiongFindContours(sourceImage);
	cv::waitKey(1);
}*/
}

void detectRectangle(cv::Mat sourceImage) {
	bool rectDetected = false, homographyDone = false;
	std::vector<cv::Point2d> corners;
	//1. DETECTING RECTANGLE
	cv::RNG rng(12345);
	cv::Mat canny_output;
	std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::blur(sourceImage, sourceImage, cv::Size( 3, 3 ));
	cv::imshow( "Source", sourceImage);
	cv::Canny( sourceImage, canny_output, 0, 255, 3 );
	cv::imshow( "Canny", canny_output);
	cv::findContours( canny_output, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0) );
	cv::Mat drawing = cv::Mat::zeros( canny_output.size(), CV_8UC3 );
	for( size_t i = 0; !rectDetected && i< contours.size(); i++ )
		 {
		   cv::Scalar color = cv::Scalar( 0, 0, 255);
		   double area=cv::contourArea(contours[i]);
		   if (area > 100 * 100) {
			   /*cv::RotatedRect rect=cv::minAreaRect(contours[i]);
			   cv::Point2f corners[4];
			   rect.points(corners);
			   cv::drawContours(drawing, contours, (int)i, color, 2, 8, hierarchy, 0, cv::Point());
			   cv::circle(drawing, corners[0], 2, cv::Scalar(0, 0, 255), 2);
			   cv::circle(drawing, corners[1], 2, cv::Scalar(0, 255, 0), 2);
			   cv::circle(drawing, corners[2], 2, cv::Scalar(255, 0, 0), 2);
			   cv::circle(drawing, corners[3], 2, cv::Scalar(255, 0, 255), 2);		*/
			   cv::approxPolyDP(contours[i], corners, 4, true);
			   if (corners.size() < 4)
				   continue;
			   //cv::waitKey();
			   cv::drawContours(drawing, contours, (int)i, cv::Scalar(255, 0, 0), 2, 8, hierarchy, 0, cv::Point());
			   cv::circle(drawing, corners[0], 2, cv::Scalar(0, 0, 255), 2);
			   cv::circle(drawing, corners[1], 2, cv::Scalar(0, 255, 0), 2);
			   cv::circle(drawing, corners[2], 2, cv::Scalar(255, 0, 0), 2);
			   cv::circle(drawing, corners[3], 2, cv::Scalar(255, 0, 255), 2);
			   rectDetected = true;
		   }
		 }
	cv::namedWindow( "BGND", cv::WINDOW_AUTOSIZE );
	cv::imshow( "BGND", drawing );
	//cv::waitKey();
	//2. Apply homography
	if (rectDetected) {
		std::vector<cv::Point2d> correctedCorners;
		correctedCorners.push_back(cv::Point2d(1023, 0));
		correctedCorners.push_back(cv::Point2d(0, 0));
		correctedCorners.push_back(cv::Point2d(0, 575));
		correctedCorners.push_back(cv::Point2d(1023, 575));
		cv::Mat h = cv::findHomography(corners, correctedCorners);
		cv::Mat correctedImage; sourceImage.copyTo(correctedImage);
		warpPerspective(sourceImage, correctedImage, h, correctedImage.size());
		cv::namedWindow("CORRECT", cv::WINDOW_AUTOSIZE);
		cv::imshow("CORRECT", correctedImage);
		//cv::waitKey();
	}
	cv::waitKey(1);

}
void detectUsingSimpleBlobDetector(cv::Mat sourceImage) {
	//1. Create detector for circular blobs:
	//1.1.  Setup SimpleBlobDetector parameters.
	cv::SimpleBlobDetector::Params params;
	// Change thresholds
	params.minThreshold = 10;
	params.maxThreshold = 200;
	// Filter by Circularity
	params.filterByCircularity = true;
	params.minCircularity = 0.1;
	//1.2 Set up the detector with default parameters.
	cv::Ptr<cv::SimpleBlobDetector> detector=cv::SimpleBlobDetector::create(params);

	//2. Detect & Draw detected blobs as red circles.
	// DrawMatchesFlags::DRAW_RICH_KEYPOINTS flag ensures the size of the circle corresponds to the size of blob
	std::vector<cv::KeyPoint> keypoints;
	detector->detect( sourceImage, keypoints);
	cv::Mat im_with_keypoints;
	cv::drawKeypoints( sourceImage, keypoints, im_with_keypoints, cv::Scalar(0,0,255), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS );
	cv::imshow("example", im_with_keypoints);
}
void detectUsiongFindContours(cv::Mat sourceImage) {
  cv::RNG rng(12345);
  cv::Mat canny_output;
  std::vector<std::vector<cv::Point> > contours;
  std::vector<cv::Vec4i> hierarchy;
  cv::blur(sourceImage, sourceImage, cv::Size( 3, 3 ));
 cv::imshow( "Source", sourceImage);
  cv::Canny( sourceImage, canny_output, 0, 255, 3 );
  cv::imshow( "Canny", canny_output);
  cv::Mat binarySourceImage;
  cv::threshold(sourceImage, binarySourceImage, 200, 255, cv::THRESH_BINARY_INV);
  cv::imshow( "Threshold", binarySourceImage);

  //cv::findContours( binarySourceImage, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0) );
  cv::findContours( canny_output, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0) );
  cv::Mat drawing = cv::Mat::zeros( canny_output.size(), CV_8UC3 );
  for( size_t i = 0; i< contours.size(); i++ )
     {
       cv::Scalar color = cv::Scalar( 0, 0, 255);
	   double area=cv::contourArea(contours[i]);
	   double perimeter=cv::arcLength(contours[i],true);
	   double circularity = 4 * M_PI*area / (perimeter*perimeter);
	   if(circularity>0.92f)
			cv::drawContours( drawing, contours, (int)i, color, 2, 8, hierarchy, 0, cv::Point() );
     }
  cv::namedWindow( "Contours", cv::WINDOW_AUTOSIZE );
  cv::imshow( "Contours", drawing );
}
