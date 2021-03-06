
#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    ball_chaser::DriveToTarget srv;
    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;
    if (!client.call(srv))
        ROS_ERROR("Failed to call service command_robot");
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{
    int white_pixel = 3 * 255; // sum of all three color channels
    int horizontal_ball_center = 0;
    float lin_x = 0;
    float lin_x_max = 0.2;
    float ang_z = 0;
    float ang_z_max = 0.75;
    
    // Loop through each pixel in the image and check if its white and determine its position
    for (int i = 0; i < img.height * img.step - 3; i+=3) {
        if (img.data[i] + img.data[i+1] + img.data[i+2] >= white_pixel) {
	    // scan for horizontal ball center
	    for (int j = i; j < i + i % img.step - 3; j+=3) {
		if (img.data[j] + img.data[j+1] + img.data[j+2] < white_pixel || j == i + i % img.step - 3) {
	            	horizontal_ball_center = ((j + i) / 2) % img.step;
			break;
		}
	    }
	    /* step control
	    if (horizontal_ball_center < img.step / 3) {
		// turn left
                lin_x = lin_x_max;
                ang_z = ang_z_max;
            }
            else if (horizontal_ball_center > 2 * img.step / 3) {
		// turn right
                lin_x = lin_x_max;
                ang_z = -ang_z_max;
            }
            else {
		// drive straight
                lin_x = lin_x_max;
                ang_z = 0;
            }
	    */
	    // p-controller
	    lin_x = lin_x_max;
            ang_z = -(horizontal_ball_center - img.step / 2.0) / (img.step / 2.0) * ang_z_max;
ROS_INFO_STREAM("Moving robot lin_x: " + std::to_string(lin_x) + " ang_z: " + std::to_string(ang_z));
            break;
        }
    }

    // send drive command to robot
    drive_robot(lin_x, ang_z);
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
