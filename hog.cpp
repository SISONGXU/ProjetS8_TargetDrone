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

using namespace cv;
using namespace std;
cv::HOGDescriptor hog;
//?? HOGDescriptor hog;
class ImageConverter
{
  ros::NodeHandle nh_;
  image_transport::ImageTransport it_;
  image_transport::Subscriber image_sub_;
  image_transport::Publisher image_pub_;
  ros::Publisher pub2;

  public:
  
  ImageConverter()
    : it_(nh_)
  {
	  hog.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector());
	  image_sub_ = it_.subscribe("/bebop/image_raw", 1,&ImageConverter::imageCb, this);
	  // Subscrive to input video feed and publish output video feed
	  //?? image_sub_ = it_.subscribe("/bebop/image_raw", 1,&ImageConverter::imageCb, this);
	  image_pub_ = it_.advertise("/image_converter/output_video", 1);
	  pub2 = nh_.advertise<geometry_msgs::Point>("/cible_humaine", 1000);
	  namedWindow("ciblehumaine", CV_WINDOW_AUTOSIZE);
	  moveWindow( "ciblehumaine",0,600);
  }

  
  ~ImageConverter()
  {
	destroyWindow("ciblehumaine");
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

	cv::Mat imgRGB = cv_ptr->image;	
	
 	int iLowH = 0;
 	int iHighH = 30;

 	int iLowS = 120; 
 	int iHighS = 235;

 	int iLowV = 120;
 	int iHighV = 255;

	cv::Mat imgHSV;
	
	
	cv::cvtColor(imgRGB, imgHSV, cv::COLOR_BGR2HSV);
	
	//cv::Mat frame = cv_ptr->image;
    //Mat img = cv_ptr->image;
	
	vector<Rect> found, found_filtered;


	//________LAG________________________________________
	hog.detectMultiScale(imgHSV, found, 0, Size(8,8),Size(0,0), 1.05, 2);
	//?? hog.detectMultiScale(img, found, 0, Size(0,0), Size(16,16), 1.05, 2);
	//________________________________________________
	size_t i, j;
	for (i=0; i<found.size(); i++) 
	{
		Rect r = found[i];
		for (j=0; j<found.size(); j++)
		{ 
			if (j!=i && (r & found[j]) == r)   //refound a same target
				break;
			if (j== found.size())
			found_filtered.push_back(r);
		}
	}

	for (i=0; i<found_filtered.size(); i++) 
	{
		Rect r = found_filtered[i];
		r.x += cvRound(r.width*0.1);
		r.width = cvRound(r.width*0.8);
		r.y += cvRound(r.height*0.07);
		r.height = cvRound(r.height*0.8);
		rectangle(imgRGB, r.tl(), r.br(), Scalar(0,255,0), 3);        
		geometry_msgs::Point pos_h;
		pos_h.x = (r.x+(r.width/2));
		pos_h.y = r.y;
		pos_h.z= r.height;
		pub2.publish(pos_h);
	}
	image_pub_.publish(cv_ptr->toImageMsg());
	imshow("ciblehumaine", imgRGB);
	waitKey(5);
  }
};

int main(int argc, char** argv)
{
  ros::init(argc, argv, "image_converter");
  ImageConverter ic;
  ros::spin();
  return 0;
}
