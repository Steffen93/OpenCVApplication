/*
* Multitouch Finger Detection Framework
* using OpenCV 2.3.1
*
* June 2011
*
* Bjoern Froemmer
* Nils Roeder
*/


/*
*	TBB.DLL TBB_DEBUG.DLL
*	http://threadingbuildingblocks.org/ver.php?fid=177
*	http://threadingbuildingblocks.org/file.php?fid=77
*/


#include "opencv2/opencv.hpp"
#include <math.h>
#include <time.h>
#include "Blob.h"

using namespace cv;

double getDistance(Blob, Blob);

int main (void)
{
	Mat image;
	//VideoCapture cap(0); // open the default camera
	VideoCapture cap("../mt_camera_raw.avi"); // open a video
	if(!cap.isOpened()) // check if we succeeded
	{
		std::cout << "ERROR: Could not open camera/video stream.\n\n";
		return -1;
	}

	Mat frame, original, orig_restore; // matrices for image storing

	//CvFont font = fontQt("Times"); // requires Qt lib & Qt-enabled OpenCV (see OpenCV documentation!)

	int currentFrame = 0; // frame counter
	clock_t ms_start, ms_end, ms_time; // time

	char buffer [10]; // buffer for int to ascii conversion -> itoa(...)

	vector<Blob> lastCenters;
	vector<Blob> currCenters;
	vector<bool> isVisited;
	int idCounter = 0;
	for(;;) // endless loop, do frame grabbing and image processing here
	{
		ms_start = clock(); // time start
		cap >> frame; // get a new frame from camera and store it in matrix
		if (currentFrame == 0){
			image = frame.clone();
		}
		if ( frame.data == NULL ) // abort if there are no more frames
		{
			std::cout << "EXITING: Camerastream stopped or last video frame reached.\n\n";
			break;
		}

		original = frame.clone(); // copy frame to original
		absdiff(frame, image, frame);
		orig_restore = frame.clone();
		blur(frame, original, Size(20,20));
		absdiff(frame, original, original);
		blur(original, original, Size(10,10));
		threshold(original, original, 10, 255, THRESH_BINARY);

		vector<vector<Point>> contours;
		vector<Vec4i> hierarchy;
		cvtColor(original, original, CV_BGR2GRAY);
		cvtColor(orig_restore, orig_restore, CV_BGR2GRAY);
		findContours( original, contours, hierarchy, CV_RETR_CCOMP,
			CV_CHAIN_APPROX_SIMPLE );
		// iterate through all the top-level contours -> "hierarchy" may not be empty!)
		currCenters.clear();
		isVisited.clear();
		if( hierarchy.size() > 0 )
		{
			for( int idx = 0; idx >= 0; idx = hierarchy[idx][0] )
			{
				if (contourArea(Mat(contours.at(idx))) > 30) // check contour size
				{
					ellipse(original, fitEllipse(Mat(contours.at(idx))),
						Scalar(0,0,255), 1,
						8); // fit & draw ellipse to contour at index "idx"
					int x = fitEllipse(Mat(contours.at(idx))).center.x;
					int y = fitEllipse(Mat(contours.at(idx))).center.y;
					int size = 8;
					line(original, Point(x, y-size), Point(x, y+size), Scalar(255,255,255));
					line(original, Point(x-size, y), Point(x+size, y), Scalar(255,255,255));
					drawContours(original, contours, idx, Scalar(255,0,0), 1, 8,
						hierarchy); // draw contour at index "idx"
					currCenters.push_back(Blob(x,y));
					isVisited.push_back(false);
				}
			}
		}
		double min_dist = 100000.0;
		double dist = 0.0;
		int closestBlob = -1;
		for(int i = 0; i < lastCenters.size(); i++){
			min_dist = 1000000.0;
			for (int j = 0; j < currCenters.size(); j++){
				dist = getDistance (currCenters[j], lastCenters[i]);
				if(dist < min_dist){
					min_dist = dist;
					closestBlob = j;
				}
			}

			if(closestBlob != -1){
				currCenters[closestBlob].setID(lastCenters[i].getID());
				isVisited[closestBlob] = true;
			}
			putText(original, std::to_string(lastCenters[i].getID()), cvPoint(lastCenters[i].getX()+8,lastCenters[i].getY()-8), FONT_HERSHEY_PLAIN, 1, CV_RGB(255,255,255), 1, 8); // write framecounter to the image (useful for debugging)
			//j: nearest neighbour => curr i == last j
			//=> last j: id
		}
		//fancy magic
		lastCenters.clear();
		for (int i = 0; i < currCenters.size(); i++){
			lastCenters.push_back(currCenters.at(i));
		}
		
		// time end
		ms_end = clock();
		ms_time = ms_end - ms_start;
		
		add(original, orig_restore, original);

		putText(original, "frame #"+(string)_itoa(currentFrame, buffer, 10), cvPoint(0,15), FONT_HERSHEY_PLAIN, 1, CV_RGB(255,255,255), 1, 8); // write framecounter to the image (useful for debugging)
		putText(original, "time per frame: "+(string)_itoa(ms_time, buffer, 10)+"ms", cvPoint(0,30), FONT_HERSHEY_PLAIN, 1, CV_RGB(255,255,255), 1, 8); // write calculation time per frame to the image

		imshow("original & contours & ellipses", original); // render image to frame
		currentFrame++; // increment frame counter

		if( waitKey(50) == 27 ) // "esc" key pressed?
		{
			std::cout << "EXITING: User stopped the process.\n\n";
			break;
		}
	}

	// the camera will be deinitialized automatically in VideoCapture destructor
	return 0;
}


double getDistance (Blob p1, Blob p2){
	return std::sqrt((p2.getX()-p1.getX())*(p2.getX()-p1.getX())+(p2.getY()-p1.getY())*(p2.getY()-p1.getY()));
}