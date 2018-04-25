#include <drone_test/keyboard_controller.h>

int command_period = 25;

DroneController::DroneController()
{
   pubTakeoff = nh.advertise<std_msgs::Empty>("/bebop/takeoff", 1000);
   pubLand = nh.advertise<std_msgs::Empty>("/bebop/land", 1000);
  // pubAnimationFlip = nh.advertise<std_msgs::UInt8>("/bebop/flip",1000);
   pubCmd = nh.advertise<geometry_msgs::Twist>("/bebop/cmd_vel", 1000);
   timer = nh.createTimer(ros::Duration(command_period/1000), &DroneController::sendCommand, this);
   //sub = nh.subscribe("/trucbidule",1000,&DroneController::posCallback, this);
}





DroneController::~DroneController()
{
}

void DroneController::sendTakeoff()
{
   std_msgs::Empty myMsg;
   pubTakeoff.publish(myMsg);
}

void DroneController::sendLand()
{
   std_msgs::Empty myMsg;
   pubLand.publish(myMsg);
}

void DroneController::setCommand(float avancement, float translation, float hauteur, float rotation)
{
   myCmd.linear.x = avancement;
   myCmd.linear.y = translation;
   myCmd.linear.z = hauteur;
   myCmd.angular.z = rotation;
}

void DroneController::sendCommand(const ros::TimerEvent& event)
{
  pubCmd.publish(myCmd);
}


void DroneController::sendAnimationFlip()
{
   std_msgs::UInt8 myflip ;
   myflip.data = 0;
  
   pubAnimationFlip.publish(myflip);
}