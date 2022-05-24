#pragma once
#include "opencv2/core/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/calib3d.hpp"
#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>

class CircularityDetector {
	enum estates{BACKGROUND_DETECT=0, RUNNING } curEstate;
	//0. Background related parameters (bgnd substraction and homography correction)
	float backgroundWidth, backgroundHeight;
	float imageWidth, imageHeight;
	std::vector<cv::Point2d> backgroundCorners, correctedCorners;
	cv::Mat homography;
	cv::Mat backgroundImage;
	//1. Detection features (particle size and circularity)
	float  circularity, particleRadius;
	
public: 
	CircularityDetector(float backWidthInMM, float backHeightInMM, float particleRadiusInMM, float circularity = 0.9f)
		: backgroundWidth(backWidthInMM)
		, backgroundHeight(backHeightInMM)
		, circularity(circularity)
		, particleRadius(particleRadiusInMM)
		, curEstate(BACKGROUND_DETECT)
	{
	
	}

	/**
		Is there a correct bead in sight? YES or NO
	*/
	bool detect(cv::Mat sourceImage) {
		if (curEstate == BACKGROUND_DETECT)
			initializeDetector(sourceImage);
		//NOTE: Estate can change inside initializeDetector... so the line bellow is not the same than an else
		if (curEstate == RUNNING)
			return doTests(sourceImage);

		return false;
	}

	void initializeDetector(cv::Mat sourceImage) {
		cv::RNG rng(12345);
		cv::Mat canny_output;
		std::vector<std::vector<cv::Point> > contours;
		std::vector<cv::Vec4i> hierarchy;

		//0. Take a copy for the background substraction.
		sourceImage.copyTo(backgroundImage);
		cv::blur(sourceImage, sourceImage, cv::Size( 3, 3 ));
		cv::Canny( sourceImage, canny_output, 0, 255, 3 );
		//cv::imshow( "Canny", canny_output);

		//1. Detect edges
		cv::findContours( canny_output, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0) );
		cv::Mat drawing = cv::Mat::zeros( canny_output.size(), CV_8UC3 );
		for( size_t i = 0; curEstate == BACKGROUND_DETECT && i< contours.size(); i++ ) {
			   cv::Scalar color = cv::Scalar( 0, 0, 255);
			   double area=cv::contourArea(contours[i]);
			   //We assume the largest blob will be our black background square
			   if (area > 100 * 100) {
				   cv::approxPolyDP(contours[i], backgroundCorners, 4, true);
				   if (backgroundCorners.size() < 4)
					   continue;
				   //3. Found! Let's save relevant information: 
				   imageWidth = sourceImage.cols; imageHeight = sourceImage.rows;//Frame size
				   correctedCorners.push_back(cv::Point2d(imageWidth-1, 0));
				   correctedCorners.push_back(cv::Point2d(0, 0));
				   correctedCorners.push_back(cv::Point2d(0, imageHeight-1));
				   correctedCorners.push_back(cv::Point2d(imageWidth-1, imageHeight-1));
				   homography=cv::findHomography(backgroundCorners, correctedCorners);
				   /* //DEBUG Code (Visualize background detected (square)
				   cv::drawContours(drawing, contours, (int)i, cv::Scalar(255, 0, 0), 2, 8, hierarchy, 0, cv::Point());
				   cv::circle(drawing, corners[0], 2, cv::Scalar(0, 0, 255), 2);
				   cv::circle(drawing, corners[1], 2, cv::Scalar(0, 255, 0), 2);
				   cv::circle(drawing, corners[2], 2, cv::Scalar(255, 0, 0), 2);
				   cv::circle(drawing, corners[3], 2, cv::Scalar(255, 0, 255), 2);
				   cv::imshow( "BGND", drawing );			   
				   cv::waitKey();*/
				   curEstate = RUNNING;
			   }
		}		
	}

	cv::Mat correctImage(cv::Mat sourceImage) {
		//1. Remove the background.
		cv::subtract(sourceImage, backgroundImage, sourceImage);
		//threshold (100);//transform to binary Black/White
		//erode/dilate //Remove noisy bits
		//combine image (Can I use min to transform the filtered background back to colour?)

		//2. Apply homography
		cv::Mat correctedImage; sourceImage.copyTo(correctedImage);
		warpPerspective(sourceImage, correctedImage,homography, correctedImage.size());
		cv::imshow("CORRECT", correctedImage);

		return correctedImage;
	}

	bool doTests(cv::Mat sourceImage) {
		cv::destroyWindow("FOUND");
		//0. Correct the image:
		cv::Mat correctedImage = correctImage(sourceImage), canny_output;
		
		//1. Process corrected frame looking for blobs:
		std::vector<std::vector<cv::Point> > contours;
		std::vector<cv::Vec4i> hierarchy;
		cv::Canny( correctedImage, canny_output, 0, 255, 3 );//Reinforce edges
		//cv::imshow( "Canny", canny_output);
		cv::findContours( canny_output, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0) );
		 
		 //2. Check if any blob is our bead
		 for( size_t i = 0; i< contours.size(); i++ )
		 {
			//2.1. Check some of its relevant parameters
		   cv::Scalar color = cv::Scalar( 255, 255, 255);
		   double area=cv::contourArea(contours[i]);
		   double perimeter=cv::arcLength(contours[i],true);
		   cv::Point2f circleCentre; 
		   float circleRadiusInPix, circleRadiusInMM;
		   cv::minEnclosingCircle(contours[i], circleCentre, circleRadiusInPix);
		   circleRadiusInMM = circleRadiusInPix * backgroundWidth / imageWidth;
		   //2.2. Circularity test
		   double circularity = 4 * M_PI*area / (perimeter*perimeter);
		   if (circleRadiusInMM>0.5f && circularity > this->circularity 
			   /*&& (circleRadiusInMM / this->particleRadius)>0.9f 
			   && (circleRadiusInMM / this->particleRadius)<1.1f*/ ) {
			   cv::drawContours(correctedImage, contours, (int)i, color, 2, 8, hierarchy, 0, cv::Point());
			   cv::imshow("FOUND", correctedImage);
			   printf("FOUND BEAD: circularity %f; \t radius %f mm\n", circularity, circleRadiusInMM);

			   cv::waitKey();
			   return true;
		   
		   }
		 }
		 return false;
	}
};


