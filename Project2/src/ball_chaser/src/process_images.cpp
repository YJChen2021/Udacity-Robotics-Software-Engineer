#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>
#include <cmath>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    //@: Request a service and pass the velocities to it to drive the robot

    // Create a service object
    ball_chaser::DriveToTarget srv;

    // Set requested linear and angular velocities
    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;

    // Call the command_robot service and pass the service object
    if (!client.call(srv))
    {
        ROS_ERROR("Failed to call service command_robot");
    }
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img){
    // @: Loop through each pixel in the image and check if there's a bright white one
    // Then, identify if this pixel falls in the left, mid, or right side of the image
    // Depending on the white ball position, call the drive_bot function and pass velocities to it
    // Request a stop when there's no white ball seen by the camera

    int white_pixel = 255;
    double pos_x = 0.0;  // Initialize to zero
    int temp = 0;
    float lin_x = 0.0;
    float lin_z = 0.0;
		// Loop through each pixel in the image and check if there's a bright white one
    for (int i = 0; i < img.step * img.height; i += 3) {
      int position_index = i % (img.width * 3) / 3;
      if (img.data[i] == white_pixel && img.data[i + 1] == white_pixel && img.data[i + 2] == white_pixel) {
				pos_x += position_index;
				temp += 1;
			}
    }
		pos_x = pos_x / temp;
		ROS_INFO_STREAM("ball_position:"<<pos_x);	
		if(pos_x < img.width * 0.33){
			lin_z = 0.1;
		}
   	else if(pos_x > img.width * 0.66){
			lin_z = -0.1;
		}
		else if(pos_x < img.width * 0.66 && pos_x > img.width * 0.33){
			lin_x = 0.2;
		}
    drive_robot(lin_x, lin_z);
}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}
