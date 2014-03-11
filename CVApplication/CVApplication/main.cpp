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

using namespace cv;

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

	Mat frame, original; // matrices for image storing

	//CvFont font = fontQt("Times"); // requires Qt lib & Qt-enabled OpenCV (see OpenCV documentation!)

	int currentFrame = 0; // frame counter
	clock_t ms_start, ms_end, ms_time; // time

	char buffer [10]; // buffer for int to ascii conversion -> itoa(...)

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
		blur(frame, original, Size(20,20));
		absdiff(frame, original, original);
		blur(original, original, Size(10,10));
		threshold(original, original, 10, 255, THRESH_BINARY);

		vector<vector<Point>> contours;
		vector<Vec4i> hierarchy;
		cvtColor(original, original, CV_BGR2GRAY);
		findContours( original, contours, hierarchy, CV_RETR_CCOMP,
			CV_CHAIN_APPROX_SIMPLE );
		// iterate through all the top-level contours -> "hierarchy" may not be empty!)
		if( hierarchy.size() > 0 )
		{
			for( int idx = 0; idx >= 0; idx = hierarchy[idx][0] )
			{
				if (contourArea(Mat(contours.at(idx))) > 30) // check contour size
				{
					ellipse(original, fitEllipse(Mat(contours.at(idx))),
						Scalar(0,0,255), 1,
						8); // fit & draw ellipse to contour at index "idx"
						drawContours(original, contours, idx, Scalar(255,0,0), 1, 8,
						hierarchy); // draw contour at index "idx"
				}
			}
		}

		//fancy stuff



		// time end
		ms_end = clock();
		ms_time = ms_end - ms_start;

		putText(original, "frame #"+(string)_itoa(currentFrame, buffer, 10), cvPoint(0,15), FONT_HERSHEY_PLAIN, 1, CV_RGB(255,255,255), 1, 8); // write framecounter to the image (useful for debugging)
		putText(original, "time per frame: "+(string)_itoa(ms_time, buffer, 10)+"ms", cvPoint(0,30), FONT_HERSHEY_PLAIN, 1, CV_RGB(255,255,255), 1, 8); // write calculation time per frame to the image

		imshow("original & contours & ellipses", original); // render image to frame
		currentFrame++; // increment frame counter

		if( waitKey(1) == 27 ) // "esc" key pressed?
		{
			std::cout << "EXITING: User stopped the process.\n\n";
			break;
		}
	}

	// the camera will be deinitialized automatically in VideoCapture destructor
	return 0;
}

