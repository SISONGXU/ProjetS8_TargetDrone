#include <ros/ros.h>   
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.h> 
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <stdio.h>
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/face.hpp"

 
using namespace std;
using namespace cv;
 
const std::string face_cascade_name = "../opencv-3.2.0/data/haarcascades/haarcascade_frontalface_alt.xml";
const std::string eyes_cascade_name = "../opencv-3.2.0/data/haarcascades/haarcascade_eye_tree_eyeglasses.xml";
cv::CascadeClassifier face_cascade;
cv::CascadeClassifier eyes_cascade;
 
static const std::string OPENCV_WINDOW = "opencv";
 
class ImageConverter
{
  ros::NodeHandle nh_; 
  image_transport::ImageTransport it_;
  image_transport::Subscriber image_sub_;
  image_transport::Publisher image_pub_;
 
public:
  ImageConverter(): it_(nh_)
  {
    // Subscrive to input video feed and publish output video feed
    image_sub_ = it_.subscribe("/bebop/image_raw", 1, &ImageConverter::imageCb2, this); 
    image_pub_ = it_.advertise("/image_converter/outputvideo", 1);
    
    if( !face_cascade.load( face_cascade_name ) ){ printf("--(!)Error loading\n"); };
    if( !eyes_cascade.load( eyes_cascade_name ) ){ printf("--(!)Error loading\n"); };
    
    cv::namedWindow(OPENCV_WINDOW);
  }
 
  ~ImageConverter()
  {
    cv::destroyWindow(OPENCV_WINDOW);
  }
  
  void imageCb2(const sensor_msgs::ImageConstPtr& msg)//Callback2
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
    
    detectAndDisplay(cv_ptr->image);
    cv::waitKey(3);
    
    // Output modified video stream
    image_pub_.publish(cv_ptr->toImageMsg());
  }
  

  void detectAndDisplay(Mat frame)
  {
    std::vector<Rect> faces;
    cv::Mat frame_gray;
 
    cvtColor( frame, frame_gray, CV_BGR2GRAY );
    equalizeHist( frame_gray, frame_gray );

    face_cascade.detectMultiScale( frame_gray, faces, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE, Size(30, 30) );
 
  }

};
 

int main(int argc, char** argv)
{
  ros::init(argc, argv, "image_converter");
  ImageConverter ic;
  ros::spin();
  
  return 0;
}