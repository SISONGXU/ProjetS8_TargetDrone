#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <std_msgs/Int32.h>
#include <geometry_msgs/Point.h>
#include <fstream>

/*
static const std::string OPENCV_raw = "raw image";
*/
using namespace cv;
using namespace std;

static const string OPENCV_HSV = "HSV image";
static const string OPENCV_thresholded = "Thresholded image";

int h = 0, s = 0, v = 0;
int iLowH = 0;
int iHighH = 30;
int iLowS = 120; 
int iHighS = 235;
int iLowV = 120;
int iHighV = 255;
Mat imgRGB;
Mat imgHSV;

class ImageConverter
{
  ros::NodeHandle nh_;
  image_transport::ImageTransport it_;
  image_transport::Subscriber image_sub_;
  image_transport::Publisher image_pub_;
  //ros::Publisher chatter_pub = nh_.advertise<std_msgs::Int32>("chatter", 1000);
   ros::Publisher publ;
public:
  ImageConverter()
    : it_(nh_)
  {
    // Subscrive to input video feed and publish output video feed
    image_sub_ = it_.subscribe("/bebop/image_raw", 1, 
      &ImageConverter::imageCb, this);
    image_pub_ = it_.advertise("/image_converter/output_video", 1);
    publ = nh_.advertise<geometry_msgs::Point>("/cible", 1000);

    namedWindow("raw image");
    namedWindow(OPENCV_HSV);
    namedWindow(OPENCV_thresholded);
  }

  ~ImageConverter()
  {
    destroyWindow("raw image");
    destroyWindow(OPENCV_HSV);
    destroyWindow(OPENCV_thresholded);
  }

  void imageCb(const sensor_msgs::ImageConstPtr& msg)
  {
    cv_bridge::CvImagePtr cv_ptr;
    try
    {
      cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
    }
    catch (cv_bridge::Exception& e)
    {
      ROS_ERROR("cv_bridge exception: %s", e.what());
      return;
    }


    	// Perform HSV tranformation

	imgRGB = cv_ptr->image;	
	/*
 	int iLowH = 0;
 	int iHighH = 30;
 	int iLowS = 120; 
 	int iHighS = 235;
 	int iLowV = 120;
 	int iHighV = 255;
*/
	//cv::Mat imgHSV;
	Mat imgThresholded= Mat(imgRGB.size(), CV_8UC1);
	cvtColor(imgRGB, imgHSV, cv::COLOR_BGR2HSV);

    	// Perform binarisation 

	//Mat imgThresholded;
	inRange(imgHSV, cv::Scalar(iLowH, iLowS, iLowV), cv::Scalar(iHighH, iHighS, iHighV), imgThresholded);

	//morphological opening (remove small objects from the foreground)
  	erode(imgThresholded, imgThresholded, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)) );
  	dilate( imgThresholded, imgThresholded, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)) ); 

  	//morphological closing (fill small holes in the foreground)
  	dilate( imgThresholded, imgThresholded, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)) ); 
  	erode(imgThresholded, imgThresholded, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)) );

	//Barycentre calculation
	
	Moments moment = cv::moments(imgThresholded, true);

	float surface = moment.m00;
	float x = moment.m10/surface;
	float y = moment.m01/surface;

	Point point(x, y);
	circle(imgRGB, point, 15, cv::Scalar(255, 255, 255), 2);
	circle(imgRGB, point, 1, cv::Scalar(0, 0, 255), 2);

	//Print barycentre position and surface

	printf("x : %f\n",x);
	printf("y : %f\n",y);
	printf("Surface : %f\n",surface);


    // Update GUI Window
    imshow("raw image", imgRGB);
    cv::imwrite("catkin_ws/src/deep_learning/src/stream.bmp", imgRGB);
    //imshow(OPENCV_HSV, imgHSV);
    imshow(OPENCV_thresholded, imgThresholded);
    waitKey(1);

    std::ofstream outfile ("src/deep_learning/src/position.txt");
    outfile << x << "\r" << y << "\r" << surface;
    outfile.close();

    // Output modified video stream
    image_pub_.publish(cv_ptr->toImageMsg());

	//Publish position and surface

	/*
	std_msgs::Int32 xPos;
	std_msgs::Int32 yPos;
	std_msgs::Int32 surfaceValue;
	xPos.data = x;
	yPos.data = y;
	surfaceValue.data = surface;
	ROS_INFO("x : %d", xPos.data);
	ROS_INFO("y : %d", yPos.data);
	ROS_INFO("surface : %d", surfaceValue.data);
	chatter_pub.publish(xPos);
	chatter_pub.publish(yPos);
	chatter_pub.publish(surfaceValue);*/
        geometry_msgs::Point myPos;
        myPos.x = x;
        myPos.y = y;
	myPos.z = surface;
        ROS_INFO("x : %f", myPos.x);
	ROS_INFO("y : %f", myPos.y);
	ROS_INFO("surface : %f", myPos.z );
	publ.publish(myPos); 

  }
};

void getObjectColor(int event, int x, int y, int flags, void *param = NULL) {
    	CvScalar pixel;
    	IplImage *hsv;
 	
        if(event == CV_EVENT_LBUTTONUP) {
  	IplImage* image;
   	image = cvCreateImage(cvSize(imgRGB.cols,imgRGB.rows),8,3);
        // Get the hsv image
        hsv = cvCloneImage(image);
        cvCvtColor(image, hsv, CV_BGR2HSV);
 
        // Get the selected pixel
        pixel = cvGet2D(hsv, y, x);
 
        // Change the value of the tracked color with the color of the selected pixel
        h = (int)pixel.val[0];
        s = (int)pixel.val[1];
        v = (int)pixel.val[2];
 
 	iLowH = (h-40);
 	iHighH = (h+40);	
	iLowS = (s-40);
	iHighS = (s+40);
	iLowV = (v-40);
 	iHighV = (v+40);
        // Release the memory of the hsv image
        cvReleaseImage(&hsv);
 
    }
  
}

int main(int argc, char** argv)
{
  ros::init(argc, argv, "image_converter");
  cvNamedWindow("raw image",CV_WINDOW_AUTOSIZE); 
  cvSetMouseCallback("raw image",getObjectColor);

  ImageConverter ic;
  ros::spin();
  return 0;
}
