/*
 * YoloObjectDetector.h
 *
 *  Created on: Dec 19, 2016
 *      Author: Marko Bjelonic
 *   Institute: ETH Zurich, Robotic Systems Lab
 */

#pragma once

// c++
#include <math.h>
#include <string>
#include <vector>
#include <iostream>
#include <pthread.h>

// ROS
#include <ros/ros.h>
#include <std_msgs/Int8.h>
#include <actionlib/server/simple_action_server.h>
#include <sensor_msgs/image_encodings.h>
#include <sensor_msgs/Image.h>
#include <sensor_msgs/PointCloud2.h>
#include <geometry_msgs/Point.h>
#include <image_transport/image_transport.h>

// OpenCv
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <cv_bridge/cv_bridge.h>

// darknet_ros_msgs
#include <darknet_ros_msgs/BoundingBoxes.h>
#include <darknet_ros_msgs/BoundingBox.h>
#include <darknet_ros_msgs/CheckForObjectsAction.h>

//detection_msgs
//#include <detection_msgs/PointCloudImage.h>
#include <detection_msgs/CompiledMessage.h>

namespace darknet_ros {

//! Bounding box of the detected object.
typedef struct {
  float x, y, w, h, prob;
  int num, Class;
} RosBox_;

/*!
 * Run YOLO and detect obstacles.
 * @param[out] bounding box.
 */
extern "C" RosBox_ *demo_yolo();

/*!
 * Initialize darknet network of yolo.
 * @param[in] cfgfile location of darknet's cfg file describing the layers of the network.
 * @param[in] weightfile location of darknet's weights file setting the weights of the network.
 * @param[in] datafile location of darknet's data file.
 * @param[in] thresh threshold of the object detection (0 < thresh < 1).
 */
extern "C" void load_network_demo(char *cfgfile, char *weightfile, char *datafile,
                                  float thresh,
                                  char **names, int classes,
                                  bool viewimage, int waitkeydelay,
                                  int frame_skip,
                                  float hier,
                                  int w, int h, int frames, int fullscreen,
                                  bool enableConsoleOutput);

/*!
 * This function is called in yolo and allows YOLO to receive the ROS image.
 * @param[out] current image of the camera.
 */
IplImage* get_ipl_image(void);

class YoloObjectDetector
{
 public:
  /*!
   * Constructor.
   */
  explicit YoloObjectDetector(ros::NodeHandle nh);

  /*!
   * Destructor.
   */
  ~YoloObjectDetector();

 private:
  /*!
   * Reads and verifies the ROS parameters.
   * @return true if successful.
   */
  bool readParameters();

  /*!
   * Initialize the ROS connections.
   */
  void init();

  /*!
   * Draws the bounding boxes of the detected objects.
   * @param[in] inputFrame image of current camera frame.
   * @param[in] rosBoxes vector of detected bounding boxes of specific class.
   * @param[in] numberOfObjects number of objects of specific class.
   * @param[in] rosBoxColor color of specific class.
   * @param[in] objectLabel name of detected class.
   */
  void drawBoxes(cv::Mat &inputFrame, std::vector<RosBox_> &rosBoxes, int &numberOfObjects,
      cv::Scalar &rosBoxColor, const std::string &objectLabel);

  /*!
   * Run YOLO and detect obstacles.
   * @param[in] fullFrame image of current camera frame.
   */
  void runYolo(cv::Mat &fullFrame, int id = 0);

  /*
   * Run YOLO and detect obstacles with pointcloud image.
   */
  void runPointCloudYolo(cv::Mat& fullFrame, const sensor_msgs::PointCloud2& cloud, int robotNumber, int id = 0);

  /*!
   * Callback of camera.
   * @param[in] msg image pointer.
   */
  void cameraCallback(const sensor_msgs::ImageConstPtr& msg);

  /*
   *  Callback for pointcloud message
   */
  void pointcloudCallback(const sensor_msgs::PointCloud2ConstPtr& msg, int robotNumber);

  void pointcloudOneCallback(const sensor_msgs::PointCloud2ConstPtr& msg);
  void pointcloudTwoCallback(const sensor_msgs::PointCloud2ConstPtr& msg);
  void pointcloudThreeCallback(const sensor_msgs::PointCloud2ConstPtr& msg);
  void pointcloudFourCallback(const sensor_msgs::PointCloud2ConstPtr& msg);

  /*!
   * Check for objects action goal callback.
   */
  void checkForObjectsActionGoalCB();

  /*!
   * Check for objects action preempt callback.
   */
  void checkForObjectsActionPreemptCB();

  /*!
   * Check if a preempt for the check for objects action has been requested.
   * @return false if preempt has been requested or inactive.
   */
  bool isCheckingForObjects() const;

  /*!
   * Publishes the detection image.
   * @return true if successful.
   */
  bool publishDetectionImage(const cv::Mat& detectionImage);

  /*
   * Publishes the detection image with pointcloud.
   */
  //bool publishDetectionImageWithPC(const cv::Mat& detectionImage, const sensor_msgs::PointCloud2& cloud);
  bool publishDetectionImageWithPC(const cv::Mat& detectionImage, detection_msgs::CompiledMessage cloudMessage);

  //! Typedefs.
  typedef actionlib::SimpleActionServer<darknet_ros_msgs::CheckForObjectsAction> CheckForObjectsActionServer;
  typedef std::shared_ptr<CheckForObjectsActionServer> CheckForObjectsActionServerPtr;

  //! ROS node handle.
  ros::NodeHandle nodeHandle_;

  //! Class labels.
  int numClasses_;
  std::vector<std::string> classLabels_;

  //! Check for objects action server.
  CheckForObjectsActionServerPtr checkForObjectsActionServer_;

  //! Advertise and subscribe to image topics.
  image_transport::ImageTransport imageTransport_;

  //! ROS subscriber and publisher.
  image_transport::Subscriber imageSubscriber_;
  // ros::Subscriber imageSubscriber_;
  ros::Subscriber pointcloudOneSubscriber_;
  ros::Subscriber pointcloudTwoSubscriber_;
  ros::Subscriber pointcloudThreeSubscriber_;
  ros::Subscriber pointcloudFourSubscriber_;
  
  ros::Publisher objectPublisher_;
  ros::Publisher boundingBoxesPublisher_;

  //! Detected objects. (these should seriously just be local )
  std::vector< std::vector<RosBox_> > rosBoxes_; // Bounding boxes, organized by class ID
  std::vector<int> rosBoxCounter_; // Number of bounding boxes, organized by class ID (e.g. counter[0] may correspond to # of Person bounding boxes)
  std::vector<cv::Scalar> rosBoxColors_; // Visualized bounding box colors, organized by class ID
  darknet_ros_msgs::BoundingBoxes boundingBoxesResults_; // BoundingBoxes message for publishing
  RosBox_* boxes_; //Unorganized array of bounding boxes (to be turned into rosBoxes_)

  //! Camera related parameters.
  int frameWidth_;
  int frameHeight_;

  //! Image view in opencv.
  const std::string opencvWindow_;
  bool viewImage_;
  int waitKeyDelay_;
  bool darknetImageViewer_;
  bool enableConsoleOutput_;

  //! Publisher of the bounding box image.
  ros::Publisher detectionImagePublisher_;
  //ros::Publisher detectionPointCloudPublisher_;

  //Generalized publisher
  ros::Publisher compiledMessagePublisher_;
};

} /* namespace darknet_ros*/
