#!/usr/bin/env python

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
# Ros Messages
from sensor_msgs.msg import *
# We do not use cv_bridge it does not support CompressedImage in python
from cv_bridge import CvBridge, CvBridgeError
from std_msgs.msg import String


class image_feature:
   
    def __init__(self):
        '''Initialize ros publisher, ros subscriber'''
        # topic where we publish
        self.image_pub = rospy.Publisher("/output/image_raw",Image,  queue_size = 1)
        # subscribed Topic
        self.subscriber = rospy.Subscriber("/bebop/image_raw",Image, self.callback,  queue_size = 1, buff_size=2**24)
        self.bridge = CvBridge()

    def callback(self, msg):
 
        print "Processing frame | Delay:%6.3f" % (rospy.Time.now() - msg.header.stamp).to_sec()
        orig_image = self.bridge.imgmsg_to_cv2(msg, "bgr8")
	#orig_image = cv2.cvtColor(orig_image, cv2.COLOR_BGR2GRAY)
        #orig_image = imutils.resize(orig_image, width=1100)

	# grab the frame dimensions and convert it to a blob
	(h, w) = orig_image.shape[:2]

	blob = cv2.dnn.blobFromImage(cv2.resize(orig_image, (400, 400)),
		0.0007843, (350, 350), 127.5)

	# pass the blob through the network and obtain the detections and
	# predictions
	net.setInput(blob)

	detections = net.forward()
	
	# loop over the detections
	for i in np.arange(0, detections.shape[2]):

		# extract the confidence (i.e., probability) associated with
		# the prediction
		confidence = detections[0, 0, i, 2]
		idx = int(detections[0, 0, i, 1])
		# filter out weak detections by ensuring the `confidence` is
		# greater than the minimum confidence
		if CLASSES[idx] == "person":
			if confidence > args["confidence"]:
				# extract the index of the class label from the
				# `detections`, then compute the (x, y)-coordinates of
				# the bounding box for the object
 				
				box = detections[0, 0, i, 3:7] * np.array([w, h, w, h])
				(startX, startY, endX, endY) = box.astype("int")

				# draw the prediction on the frame
				label = "{}: {:.2f}%".format(CLASSES[idx],
					confidence * 100)
				cv2.rectangle(orig_image, (startX, startY), (endX, endY),
					COLORS[idx], 2)
				y = startY - 15 if startY - 15 > 15 else startY + 15
				cv2.putText(orig_image, label, (startX, y),
					cv2.FONT_HERSHEY_SIMPLEX, 0.5, COLORS[idx], 2)

	# show the output frame
	cv2.imshow("Frame", orig_image)
	key = cv2.waitKey(1) 

	
  
def main(args):
    '''Initializes and cleanup ros node'''

    ic = image_feature()
    rospy.init_node('coordinate')

    try:
        rospy.spin()

    except KeyboardInterrupt:
        print "Shutting down ROS Image feature detector module"
    cv2.destroyAllWindows()

if __name__ == '__main__':
   
    # construct the argument parse and parse the arguments
    ap = argparse.ArgumentParser()
    ap.add_argument("-p","--prototxt", required=True,
   	    help="path to Caffe 'deploy' prototxt file")
    ap.add_argument("-m", "--model", required=True,
	    help="path to Caffe pre-trained model")
    ap.add_argument("-c", "--confidence", type=float, default=0.2,
	    help="minimum probability to filter weak detections")
    args = vars(ap.parse_args())
    # initialize the list of class labels MobileNet SSD was trained to
    # detect, then generate a set of bounding box colors for each class
    CLASSES = ["background", "aeroplane", "bicycle", "bird", "boat",
	"bottle", "bus", "car", "cat", "chair", "cow", "diningtable",
	"dog", "horse", "motorbike", "person", "pottedplant", "sheep",
	"sofa", "train", "tvmonitor"]
    COLORS = np.random.uniform(0, 255, size=(len(CLASSES), 3))

    # load our serialized model from disk
    print("[INFO] loading model...")
    net = cv2.dnn.readNetFromCaffe(args["prototxt"], args["model"])
    args = vars(ap.parse_args())

    main(sys.argv)

