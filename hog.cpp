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
#include <geometry_msgs/Quaternion.h>
#include <stdexcept>
#include <opencv2/objdetect.hpp>
#include <opencv2/imgcodecs.hpp>


using namespace cv;
using namespace std;
cv::HOGDescriptor hog;

//centre de zone qu'on a choisi a suivre
//video:856 x 480,donc on prends le centre de video par default
int center_x = 428;
int center_y = 240;
int side_x_l = 278;   //428-150
int side_x_r = 578;   //428+150


Mat imgRGB;
Mat img;

void getObjectCenter(int event, int x, int y, int flags, void *param = NULL) { 

  
  if(event == CV_EVENT_LBUTTONUP) {
      center_x=x;
      center_y=y;
      side_x_l=x-150;
      if (side_x_l<0)
      {
        side_x_l=0;
      }
      side_x_r=x+150;
     if (side_x_r>856)
      {
        side_x_r=856;
      }
      cout << "center_x = " << center_x << " " << endl;
      cout << "center_y = " << center_y << " " << endl;
      cout << "side_x_l = " << side_x_l << " " << endl;
      cout << "side_x_r = " << side_x_r << " " << endl;
    }
 
}

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
	  image_pub_ = it_.advertise("/image_converter/output_video", 1);
	  pub2 = nh_.advertise<geometry_msgs::Quaternion>("/cible_humaine", 1000);       //return 4 values
	  namedWindow("choose_ciblehumaine", CV_WINDOW_AUTOSIZE);
	
  }

  
  ~ImageConverter()
  {
	destroyWindow("choose_ciblehumaine");
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
	

     imgRGB = cv_ptr->image;
    // img = imgRGB;
	
     vector<Rect> found, found_filtered;
    double t = (double) getTickCount();
    // Run the detector with default parameters. to get a higher hit-rate
    // (and more false alarms, respectively), decrease the hitThreshold and
    // groupThreshold (set groupThreshold to 0 to turn off the grouping completely).
    hog.detectMultiScale(imgRGB, found, 0, Size(8,8), Size(0,0), 1.05, 2);
    
    /*
    t = (double) getTickCount() - t;
    cout << "detection time = " << (t*1000./cv::getTickFrequency()) << " ms" << endl;
    */


    for(size_t i = 0; i < found.size(); i++ )
    {
        Rect r = found[i];

        size_t j;
        // Do not add small detections inside a bigger detection.
        for ( j = 0; j < found.size(); j++ )
            if ( j != i && (r & found[j]) == r )
                break;
          
      
        if((r.x < side_x_l) || ((r.x+r.width) > side_x_r)){
           cout << "humain a l'exterieur de la zone = " << endl;
            break;
        }
        if ( j == found.size() )
            found_filtered.push_back(r);
    }

	 for (size_t i = 0; i < found_filtered.size(); i++)
    {
        Rect r = found_filtered[i];
		// The HOG detector returns slightly larger rectangles than the real objects,
        // so we slightly shrink the rectangles to get a nicer output.
        r.x += cvRound(r.width*0.1);
        r.width = cvRound(r.width*0.8);
        r.y += cvRound(r.height*0.08);
        r.height = cvRound(r.height*0.8);
		rectangle(imgRGB, r.tl(), r.br(), Scalar(0,255,0), 3);        
		Point tp = r.tl();
		Point br = r.br();
		geometry_msgs::Quaternion pos_h;
		pos_h.x = tp.x;
		pos_h.y= tp.y;
		pos_h.z= br.x;
		pos_h.w= br.y;
		pub2.publish(pos_h);
		cout << "x = " << tp.x << " " << endl;
    cout << "y = " << tp.y << " " << endl;
		cout << "x2 = " << br.x << " " << endl;
		cout << "y2 = " << br.y << " " << endl;
	}
	image_pub_.publish(cv_ptr->toImageMsg());
          
        //imshow("Choix center",img);  
	imshow("choose_ciblehumaine", imgRGB);
  cvSetMouseCallback("choose_ciblehumaine", getObjectCenter);
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
