#!/usr/bin/env python

# USAGE
# python img_sub.py --prototxt MobileNetSSD_deploy.prototxt.txt --model MobileNetSSD_deploy.caffemodel

# Python libs

import sys, time

# numpy and scipy
import numpy as np
from scipy.ndimage import filters
import argparse
import imutils
# OpenCV
import cv2

# Ros libraries
import roslib
import rospy

import time

# Ros Messages
from sensor_msgs.msg import *
# We do not use cv_bridge it does not support CompressedImage in python
from cv_bridge import CvBridge, CvBridgeError

 

class image_feature:

    def __init__(self):
        '''Initialize ros publisher, ros subscriber'''
        # topic where we publish

        self.image_pub = rospy.Publisher("/output/image_raw",
            Image)	

        self.bridge = CvBridge()

        # subscribed Topic
        self.subscriber = rospy.Subscriber("/bebop/image_raw",
            Image, self.callback,  queue_size = 1)
        

    def callback(self, msg):
        #print "Processing frame | Delay:%6.3f" % (rospy.Time.now() - msg.header.stamp).to_sec()
        orig_image = self.bridge.imgmsg_to_cv2(msg, "bgr8")
        #orig_image = imutils.resize(orig_image, width=1000)


	
	# show the output frame
	#cv2.imshow("Frame", orig_image)
	cv2.imwrite("image/stream.bmp", orig_image)
	#cv2.resize(orig_image, (100,100))	
	
def main(args):
    '''Initializes and cleanup ros node'''
    ic = image_feature()
    rospy.init_node('image_feature', anonymous=True)
    try:
        rospy.spin()
    except KeyboardInterrupt:
        print ("Shutting down ROS Image feature detector module")
    cv2.destroyAllWindows()

if __name__ == '__main__':

    main(sys.argv)

