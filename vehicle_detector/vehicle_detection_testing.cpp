#include <iostream>
#include <cv.h>   // headers
#include <highgui.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio/videoio.hpp>
#include <stdio.h>
#include <math.h>

#define PI 3.14159265

using namespace cv;
using namespace std;

//filter and background 
//http://docs.opencv.org/trunk/d1/dc5/tutorial_background_subtraction.html

int main (int argc, char** argv){
	//declare variables for frame
	int frameCount;
	Mat frame;
	Mat grayFrame;
	Mat blurFrame;
	Mat Canny_output;


	//global variables
	int x,y;
	int xTop ;
	int yTop;				//the first point, on the top of processing area
	int xMu;
	int yMu;				//the mutual point for 2 vector
	int xBot;
	int yBot;				//the last point in the, bottom right corner


	//declare variables for Canny
	int const lowThres = 100;
	int ratio = 3;
	int highThres = ratio*lowThres;
	int kernel_size = 3;


	//open testing video, download from YOUTUBE
	//vehicel night: https://www.youtube.com/watch?v=I8mJLHOS2eU
	//daylight: https://www.youtube.com/watch?v=OA8w2tXNqMY
	char videoFileName[30] = ("testingVideo.mp4");
	VideoCapture cap(videoFileName);
	if(!cap.isOpened()){
		cout << "Error loading camera, please check your camera connection! "
				<<endl;											//check if the camera open successfully
		return -1;
	}


	//take frame from camera
	cap >> frame;





	//show cols and rows of the window(frame)
	int cols = frame.cols;
	int rows = frame.rows;
	cout <<"The number of window rows: " <<rows <<endl;
	cout <<"The number of window cols: " <<cols <<endl;


	//create windows
	namedWindow("Original", CV_WINDOW_AUTOSIZE);
//	namedWindow("Lane Line", CV_WINDOW_NORMAL);
//	namedWindow("Rectangle", CV_WINDOW_AUTOSIZE);
//	namedWindow("Car detection", CV_WINDOW_AUTOSIZE);
//	namedWindow("Process Window", CV_WINDOW_AUTOSIZE);
//	namedWindow("Result", CV_WINDOW_NORMAL);
	namedWindow("12345", CV_WINDOW_AUTOSIZE);


	for (frameCount = 0; ; frameCount++) {
			cap >> frame;

			imshow("Original", frame);									//imshow the original video

			cvtColor(frame,grayFrame,CV_RGB2GRAY);						//make gray scale



//change the color from gray Scale
			for( x = 0; x < cols; x++) {
				for (y = 500; y < rows; y++){

					if(grayFrame.at<uchar>(y,x) > 170 && grayFrame.at<uchar>(y,x) < 255) {		//highlight the lane line to green color

						frame.at<Vec3b>(y,x) [0] = 0;
						frame.at<Vec3b>(y,x) [1] = 255;
						frame.at<Vec3b>(y,x) [2] = 0;
					}
				}
			}

//filter the frame noise
			//Gaussian
			GaussianBlur(grayFrame,blurFrame,Size(3,3),7,7);	 		//filter the noise using GaussianBlur
			Canny(blurFrame,Canny_output,lowThres, highThres, kernel_size);	//Canny edges

			Mat CannyOriginal;
			Canny_output.copyTo(CannyOriginal);
			Mat detectCarArea;
			Canny_output.copyTo(detectCarArea);					//copy for car detection area

			Mat angleArea;
			Canny_output.copyTo(angleArea);



//crop the process area only. "Canny_output = crop Canny"

			//canny ecrased top of the edges
			for(y = 0; y < 450; y++){
				for(x = 0; x < frame.cols; x++){
					Canny_output.at<uchar>(y,x) = 0;
					}
				}


			if (frameCount < 480) {				//crop first 480 frame
				for(y = 450; y < 500; y++){
					for(x = 800; x < cols; x++){
						Canny_output.at<uchar>(y,x) = 0;
					}
				}
			}

			Mat cropCanny;
			Canny_output.copyTo(cropCanny);


//hough line

			Mat houghLine(frame);		//hough line on frame
#if 0
		vector<Vec2f> lines;
		HoughLines(cropCanny, lines, 100, CV_PI/50, 100, 0, 0);
		for(size_t i = 0; i < lines.size(); i++) {
			float rho = lines[i][0], theta = lines[i][0];
			Point pt1, pt2;
			double a = cos(theta), b = sin(theta);
			double x0 = a*rho, y0 = b*rho;
			pt1.x = Round(x0 + 1000*(-b));
			pt1.y = Round(y0 + 1000*(a));
			pt2.x = Round(x0 - 1000*(-b));
			pt2.y = Round(y0 - 1000*(a));
			line(houghLine, pt1, pt2, Scalar(0,0,255), 3, CV_AA);
		}
#else
		vector<Vec4i> lines;
		HoughLinesP(cropCanny, lines, 1, CV_PI/180, 100, 50, 50);
		for(size_t i = 0; i < lines.size() ; i++ ){
			Vec4i l = lines[i];
			line( houghLine, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, CV_AA);
		}
#endif
//		imshow("Lane line", houghLine);


//crop the area for detect Car only
		for(y = 0; y < 400; y++){
			for(x = 0; x < cols; x++){
				detectCarArea.at<uchar>(y,x) = 0;
			}
		}

		for(y = 0; y < rows; y++){					//erase left corner
			for(x = 0; x < 500; x++){
				detectCarArea.at<uchar>(y,x) = 0;
			}
		}

		for(y = 0; y < rows; y++){					//erase right corner
			for(x = 800; x < cols; x++){
				detectCarArea.at<uchar>(y,x) = 0;
			}
		}

		for(y = 500; y < rows; y++){					//erase bottom
			for(x = 0; x < cols; x++){
				detectCarArea.at<uchar>(y,x) = 0;
			}
		}

		rectangle(CannyOriginal,Rect(500,400,300,100),Scalar(255,0,0),1,8,0);



//find contour for vehicle and bound by rectangle
		//find contour
		//http://opencvexamples.blogspot.com/2013/09/find-contour.html
		vector<vector<Point> > contour;
		vector<Vec4i> hierarchy;
		findContours(detectCarArea, contour, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0,0));

		//Approximate contours to polygons and get bounding
		vector<vector<Point> > contour_poly (contour.size());
		vector<Rect>boundRect (contour.size());
		vector<Point2f>center (contour.size());
		vector<float>radius(contour.size());

		int largestIndex = 0;
		int largestContour = 0;
		int secondLargestIndex = 0;
		int secondLargestContour = 0;
		for( int i = 0; i< contour.size(); i++ ){
		    if(contour[i].size() > largestContour){
		        secondLargestContour = largestContour;
		        secondLargestIndex = largestIndex;
		        largestContour = contour[i].size();
		        largestIndex = i;
		    } else if(contour[i].size() > secondLargestContour){
		        secondLargestContour = contour[i].size();
		        secondLargestIndex = i;
		    }
		}



//		for (int i = 0; i < contour.size(); i++){
//			approxPolyDP (Mat (contour[i]), contour_poly[i], 5, true);
//			boundRect[i] = boundingRect(Mat(contour_poly[i]));
//			minEnclosingCircle((Mat)contour_poly[i],center[i],radius[i]);
//		}

		//draw polygonal contour + bonding rect
		Mat drawing = Mat::zeros(detectCarArea.size(),CV_8UC3);
//		for (int i = 0; i < contour.size(); i++){
//			drawContours(drawing, contour_poly, i, Scalar(0,128,255),1,8,vector<Vec4i>(),0,Point());





		Scalar color = Scalar(255,0,255);
		drawContours( drawing, contour, largestIndex, color, 5, 8);
		drawContours( drawing, contour, secondLargestIndex, color, 5, 8);

		//rectangle(drawing, boundRect[i].t./ol(), boundRect[i].br(), Scalar(255,255,255),1,8,0);
//		}



//display in the Process window

		for(x = 0; x < cols; x++){
			for(y = 0; y < rows; y++){
				if(drawing.at<Vec3b>(y,x)[0] == 255&&
					drawing.at<Vec3b>(y,x)[1] == 0&&
					drawing.at<Vec3b>(y,x)[2] == 255){

					houghLine.at<Vec3b>(y,x) [0] = 255;
					houghLine.at<Vec3b>(y,x) [1] = 0;
					houghLine.at<Vec3b>(y,x) [2] = 255;
				}
			}
		}

//calculate angle
		yTop = 500;
		for (int x = 400; x < 600; x++) {
			if(angleArea.at<uchar>(yTop,x) == 255) {
				xTop = x;
			}
		}

		//mutual point
		yMu = 700;
		for (int x = 200; x < 1000; x++) {
			if(angleArea.at<uchar>(yMu,x) == 255) {
				xMu = x;
			}
		}

		//bottom point
		xBot = 1100;
		yBot = 700;

		//draw line
		line(houghLine, Point(xTop,yTop), Point(xMu,yMu),Scalar(255,0,0),5,8,0);
		line(houghLine, Point(xBot,yBot), Point(xMu,yMu),Scalar(255,0,0),5,8,0);


//angle between 2 vector
		double angleCurve;

		angleCurve = acos((xTop*yTop+xMu*yMu+xBot*yBot)/
				(sqrt(pow(xTop,2) + pow(xMu,2) + pow(xBot,2))*sqrt(pow(yTop,2) + pow(yMu,2) + pow(yBot,2))))*180/PI;

		cout <<"The angle: " <<angleCurve <<endl;

//display in case of exchnage lane, turn left turn right

		if(angleCurve >= 20) {
			putText(frame, "Line 1",Point(50,50),FONT_HERSHEY_SIMPLEX, 1, Scalar(0,0,0),5,8);
		}

		else  {
			putText(frame, "Line 2",Point(50,100),FONT_HERSHEY_SIMPLEX, 1, Scalar(0,0,0),5,8);
		}

//display paramter
		Mat displayBox = frame(cv::Rect(10, 10, 300, 400));
		Mat color2(displayBox.size(), CV_8UC3, Scalar(255,0,0));
		double alpha = 0.3;
		addWeighted(color2, alpha, displayBox, 1.0 - alpha , 0.0, displayBox);
		rectangle(frame,Rect(10,10,300,400),Scalar(0,0,255),2,8,0);





//imshow the window
		imshow("Result",frame);
		imshow("12345",houghLine);



		if(waitKey(20) >= 0) break;


		}

	return 0;




}
