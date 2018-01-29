#include <string>

#include "ros/ros.h"

#include "detection_msgs/AudioDetection.h"
#include "audio_detection_data.h"
#include "audio_logger.h"
#include "socketio_audio_logger.h"

using namespace sarwai;

void LogAudioDetection(const detection_msgs::AudioDetection::ConstPtr&);

AudioLogger audio_logger("./");
SocketIOAudioLogger socketio_logger("ec2-52-24-126-225.us-west-2.compute.amazonaws.com", 8000);

int main(int argc, char **argv) {
  ros::init(argc, argv, "audio_logger");
  ros::NodeHandle nh;
  std::string audio_detection_topic = "/sarwai_detection/detection_audio_transcript";
  ros::Subscriber audio_sub = nh.subscribe(audio_detection_topic.c_str(), 1000, LogAudioDetection);

  ros::spin();
}

void LogAudioDetection(const detection_msgs::AudioDetection::ConstPtr& msg) {
  struct AudioDetectionData data;
  data.timestamp = (int) msg->header.stamp.sec;
  data.confidence = msg->confidence;
  data.transcript = msg->transcript;
  data.audio_filename = msg->filename;
  // placeholder value 1.29.18
  data.robot_id = 123455678;

  audio_logger.Log(data);
  socketio_logger.Log(data);
}