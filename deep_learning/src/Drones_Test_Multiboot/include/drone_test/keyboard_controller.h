#include <ros/ros.h>
#include <geometry_msgs/Twist.h>
#include <std_msgs/Empty.h>

#include <geometry_msgs/Point.h>

#include <std_msgs/UInt8.h>

class DroneController
{
	public:
      
		DroneController();
		~DroneController();
		void sendTakeoff();
		void sendLand();
		void sendAnimationFlip();
		void setCommand(float avancement, float translation,float hauteur,float rotation);
		void sendCommand(const ros::TimerEvent& event);
		//void posCallback(const geometry_msgs::Point myPos);

	private:
		ros::NodeHandle nh;
		ros::Subscriber sub;
		ros::Publisher pubCmd;
		ros::Publisher pubTakeoff;
		ros::Publisher pubLand;
		ros::Publisher pubAnimationFlip;
		ros::Timer timer;
		geometry_msgs::Twist myCmd;
};
