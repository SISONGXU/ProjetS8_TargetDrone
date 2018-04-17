## Use this command to run the Deep Learning script :

python real_time_object_detection.py --prototxt MobileNetSSD_deploy.prototxt.txt --model MobileNetSSD_deploy.caffemodel

## The program needs the following nodes to be run :

	roslaunch bebop_driver bebop_node.launch 
	rosrun drone_test drone_test_image_converter 

## The image_converter have been modified!!! The package modified can be found in the branch "hakamir-own-repositery". Just download it, source, make and run the appropriate node. In this version, image_converter creates two files : 

	position.txt
	stream.bmp

## Position.txt gives the position of the barycentre and the number of white pixels from the thresholded image. 
stream.bmp is the RGB_image coming from the drone. 
