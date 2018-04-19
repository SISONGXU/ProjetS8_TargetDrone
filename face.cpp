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
#include <geometry_msgs/Point.h>
 
using namespace std;
using namespace cv;

const string face_cascade_name = "opencv-3.2.0/data/haarcascades/haarcascade_frontalface_alt.xml";
const string eyes_cascade_name = "opencv-3.2.0/data/haarcascades/haarcascade_eye_tree_eyeglasses.xml";


cv::CascadeClassifier face_cascade;
cv::CascadeClassifier eyes_cascade;
 
static const std::string OPENCV_WINDOW = "opencv";
 
class ImageConverter
{
  ros::NodeHandle nh_; 
  image_transport::ImageTransport it_;
  image_transport::Subscriber image_sub_;
  image_transport::Publisher image_pub_;
  ros::Publisher pub;
 
public:
  ImageConverter(): it_(nh_)
  {
    // Subscrive to input video feed and publish output video feed
    image_sub_ = it_.subscribe("/bebop/image_raw", 1, &ImageConverter::imageCb2, this); 
    image_pub_ = it_.advertise("/image_converter/outputvideo", 1);
    pub = nh_.advertise<geometry_msgs::Point>("/cible_face", 1000);
    
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
    geometry_msgs::Point pos_f;
    cvtColor( frame, frame_gray, CV_BGR2GRAY );
    equalizeHist( frame_gray, frame_gray );
    float x,y;
    face_cascade.detectMultiScale( frame_gray, faces, 1.1, 2, 0|CV_HAAR_FIND_BIGGEST_OBJECT|CV_HAAR_SCALE_IMAGE, Size(30, 30) );
 
     for( int i = 0; i < faces.size(); i++ )
    {    x=faces[0].x + faces[0].width*0.5;
         y=faces[0].y + faces[0].height*0.5;
         Point center( faces[i].x + faces[i].width*0.5, faces[i].y + faces[i].height*0.5 );
         double aspect_ratio = (double)faces[i].width/faces[i].height;
        if( 0.9 < aspect_ratio && aspect_ratio < 1.1 )
        {
       
        ellipse( frame, center, Size( faces[i].width*0.5, faces[i].height*0.5), 0, 0, 360, Scalar  ( 255, 0, 255 ), 4, 8, 0 );
        if(x>300&&x<550&&y>150&&y<350){
        rectangle( frame, faces[i].tl(),faces[i].br(), Scalar(0,255,0), 3 );
        ROS_INFO("x : %d", center.x);
	ROS_INFO("y : %d", center.y);
	ROS_INFO("surface : %d", faces[i].width*faces[i].height);
        pos_f.x = center.x;
	pos_f.y = center.y;
	pos_f.z= faces[i].width*faces[i].height;
	pub.publish(pos_f);
}
        else{
        pos_f.x = 0;
	pos_f.y = 0;
	pos_f.z= 0;
	pub.publish(pos_f);
}
       /* Mat faceROI = frame_gray( faces[i] );
        
        std::vector<Rect> eyes;
        eyes_cascade.detectMultiScale( faceROI, eyes, 1.1, 2, 0 |CV_HAAR_SCALE_IMAGE, Size(30, 30) );
 
        for( int j = 0; j < eyes.size(); j++ )
        {
            Point center( faces[i].x + eyes[j].x + eyes[j].width*0.5, faces[i].y + eyes[j].y + eyes[j].height*0.5 );
            int radius = cvRound( (eyes[j].width + eyes[i].height)*0.25 );
            circle( frame, center, radius, Scalar( 255, 0, 0 ), 4, 8, 0 );
        }*/

}
}

    imshow( OPENCV_WINDOW, frame );
  }

};
 

int main(int argc, char** argv)
{
  ros::init(argc, argv, "image_converter");
  ImageConverter ic;
  ros::spin();
  
  return 0;
}
