#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>


#include <sensor_msgs/image_encodings.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <sstream>
#include <geometry_msgs/Point.h>
#include "opencv/highgui.h"
#include "opencv/cv.h"
 
#include <iostream>
#include <stdlib.h>
#include <stdio.h>

using namespace cv;
using namespace std;

int h = 0, s = 0, v = 0;
int H_MIN = 120;
int H_MAX = 255;
int S_MIN = 55;
int S_MAX = 255;
int V_MIN = 34;
int V_MAX = 255;

int iLastX = -1;
int iLastY = -1;
int thresh = 100;
int max_thresh = 255;
RNG rng(12345);
       	vector<vector<Point> > contours;
        vector<Vec4i> hierarchy;
Mat threshold_output;
	Mat blur;
Mat frame;

class ImageConverter{
  ros::NodeHandle nt_;
  image_transport::ImageTransport im_;
  image_transport::Subscriber image_su_;
  ros::Publisher publ;

public:
  ImageConverter()
    : im_(nt_)
  {
    // Subscrive to input video feed and publish output video feed
    image_su_ = im_.subscribe("/bebop/image_raw", 1,
      &ImageConverter::imageCb, this);
    publ = nt_.advertise<geometry_msgs::Point>("/cible", 1000);
   

   	//cvNamedWindow("MyVideo", CV_WINDOW_AUTOSIZE); 
	//namedWindow("Trackbars", CV_WINDOW_AUTOSIZE);
	namedWindow("Thresholded Image", CV_WINDOW_AUTOSIZE);
	//namedWindow("Contours de la cible", CV_WINDOW_AUTOSIZE);
    
  }

  ~ImageConverter()
  {
    	//destroyWindow("MyVideo");
    	//destroyWindow("Trackbars");
	destroyWindow("Thresholded Image");
    	//destroyWindow("Contours de la cible");
  }

  void imageCb(const sensor_msgs::ImageConstPtr& msg1)
  {
    cv_bridge::CvImagePtr cv_ptr;
    try
    {
      cv_ptr = cv_bridge::toCvCopy(msg1, sensor_msgs::image_encodings::BGR8);
    }
    catch (cv_bridge::Exception& e)
    {
      ROS_ERROR("cv_bridge exception: %s", e.what());
      return;
    }

     frame = cv_ptr->image;
    Mat imgThresholded= Mat(frame.size(), CV_8UC1); //8bits non signé 1 canal
    Mat imgHSV = Mat(frame.size(), CV_8UC3);

   // ......................................................................
	
    /*string trackbarWindowName("trackbar");
    namedWindow(trackbarWindowName,1);

    createTrackbar( "H_MIN", trackbarWindowName, &H_MIN, H_MAX);
    createTrackbar( "H_MAX", trackbarWindowName, &H_MAX, H_MAX);
    createTrackbar( "S_MIN", trackbarWindowName, &S_MIN, S_MAX);
    createTrackbar( "S_MAX", trackbarWindowName, &S_MAX, S_MAX);
    createTrackbar( "V_MIN", trackbarWindowName, &V_MIN, V_MAX);
    createTrackbar( "V_MAX", trackbarWindowName, &V_MAX, V_MAX);*/

 
    cvtColor(frame, imgHSV,COLOR_BGR2HSV);
    inRange(imgHSV, cv::Scalar(H_MIN,S_MIN,V_MIN), cv::Scalar(H_MAX,S_MAX,V_MAX), imgThresholded);

    	/// ***************** TRAITEMENT DE L'IMAGE PAR MORPHOLOGIE MATHÉMATIQUE *********************
	// Opération d'ouverture 
	erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
	dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) ); 

	// Opération de fermeture
	dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(18, 18)) ); 
	erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(18, 18)) );




/// ********************* BINARISATION DE L'IMAGE THRESHOLDED *****************
        // Detect edges using Threshold
        threshold(imgThresholded, threshold_output, thresh, 255, THRESH_BINARY); // binarise l'image OK

	// Filtre gaussien pour obtenir des contours plus nets 
	//GaussianBlur(imgThresholded, blur, Size( 3, 3 ),0,0);

/// ********************* DÉTECTION DE LA FORME DE LA CIBLE **********************

        // Find contours
        findContours(threshold_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE,   
       	Point(0, 0));

        /// Find the convex hull object for each contour
        vector<vector<Point> >hull(contours.size());

/// *********************** DESSINER LES CONTOURS DE LA CIBLE *********************
	// Calculer les contours        
	for (int i = 0; i < contours.size(); i++)
        {
            convexHull(Mat(contours[i]), hull[i], false);
        }

        // Draw contours + hull results
        Mat drawing = Mat::zeros(threshold_output.size(), CV_8UC3); // matrice de zéros
        for (int i = 0; i< contours.size(); i++)
        {
            Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
            drawContours(drawing, contours, i, color, 1, 8, vector<Vec4i>(), 0, Point());
            drawContours(drawing, hull, i, color, 1, 8, vector<Vec4i>(), 0, Point());
        }

   	// Calculate the moments of the thresholded image
        Moments oMoments = moments(imgThresholded); // Calculates all of the moments up to the third order of a polygon or rasterized shape
						    // donc trouver les bruits ????
    	
        double dM01 = oMoments.m01;
        double dM10 = oMoments.m10;
        double dArea = oMoments.m00;
Mat imgLines;
imgLines = Mat::zeros(frame.size(), CV_8UC3);

        // if the area <= 10000, I consider that the there are no object in the image and it's because of the noise, the area is not zero 
        if (dArea > 10000)
        {

/// ********************** DÉTERMINER LA POSITION DE LA CIBLE DANS L'IMAGE ***********************
            //calculate the position of the object
            int posX = dM10 / dArea;
            int posY = dM01 / dArea;
            cout << "X = " << posX << endl;
            cout << "Y = " << posY << endl;

		geometry_msgs::Point myPos;
		myPos.x = posX;
		myPos.y = posY;
		//myPos.z = 600;
		publ.publish(myPos); 

            if (posX > 275 && posX < 325){
                cout << "Object Straight Ahead." << endl;
            }
            else if (posX < 275){
                cout << "Turn Left." << endl;
            }
            else if (posX > 325){
                cout << "Turn Right." << endl;
            }

            if (iLastX >= 0 && iLastY >= 0 && posX >= 0 && posY >= 0)
            {
                //Draw a red line from the previous point to the current point (for visual debugging)

                line(imgLines, Point(posX, posY), Point(iLastX, iLastY), Scalar(0, 0, 255), 2);
            }

            iLastX = posX;
            iLastY = posY;
        }




  /// AFFICHAGE DES FENÊTRES
	imshow("Choix de Couleur",frame);
	//imshow("MyVideo", imgHSV); // affiche l'image en HSV
        imshow("Thresholded Image", imgThresholded); // montre l'image binariser
	moveWindow( "Thresholded Image",850,0);
        //imshow("Contours de la cible", drawing);
	frame = frame + imgLines;
    waitKey(1);



	


   //................................................................................

 /* geometry_msgs::Point myMsg;
   myMsg.x = barycentre.x;
   myMsg.y = barycnetre.y;
   publ.publish(myMsg);
*/
  }






};
void getObjectColor(int event, int x, int y, int flags, void *param = NULL) {
 
    // Vars
    CvScalar pixel;
    IplImage *hsv;
 
    if(event == CV_EVENT_LBUTTONUP) {
 

/*
 cv_bridge::CvImagePtr cv_ptr;
    try
    {
      cv_ptr = cv_bridge::toCvCopy(msg1, sensor_msgs::image_encodings::BGR8);
    }
    catch (cv_bridge::Exception& e)
    {
      ROS_ERROR("cv_bridge exception: %s", e.what());
      return;
    }


    frame = cv_ptr->image;*/
 IplImage* image;
 image = cvCreateImage(cvSize(frame.cols,frame.rows),8,3);
 IplImage ipltemp=frame; cvCopy(&ipltemp,image);
 

        // Get the hsv image
        hsv = cvCloneImage(image);
        cvCvtColor(image, hsv, CV_BGR2HSV);
 
        // Get the selected pixel
        pixel = cvGet2D(hsv, y, x);
 
        // Change the value of the tracked color with the color of the selected pixel
        h = (int)pixel.val[0];
        s = (int)pixel.val[1];
        v = (int)pixel.val[2];
 
 H_MIN = (h-30);
 H_MAX = (h+30);
S_MIN = (s-30);
S_MAX = (s+30);
V_MIN = (v-30);
 V_MAX = (v+30);



        // Release the memory of the hsv image
            cvReleaseImage(&hsv);
 
    }
 
}
int main(int argc, char** argv)
{
  ros::init(argc, argv, "image_converter");

cvNamedWindow("Choix de Couleur", CV_WINDOW_AUTOSIZE); 
    // Mouse event to select the tracked color on the original image
 cvSetMouseCallback("Choix de Couleur", getObjectColor);
  ImageConverter ico;

 
  ros::spin();
//......................

//......................
  return 0;
}
