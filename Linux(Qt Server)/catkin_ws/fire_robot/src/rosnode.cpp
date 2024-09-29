/* kccistc embedded & iot by ksh */
#include "../include/fire_robot/rosnode.h"
#include <std_msgs/String.h>


RosNode::RosNode(int argc, char** argv)
{
    ros::init(argc, argv, "rosqt");

    if (!ros::master::check()) {
        return;
    }

    ros::NodeHandle nh;

    set_pub =
          nh.advertise<geometry_msgs::PoseWithCovarianceStamped>("initialpose", 1000);
    goal_pub =
          nh.advertise<geometry_msgs::PoseStamped>("move_base_simple/goal", 1000);
    data_pub =
          nh.advertise<std_msgs::String>("data_topic",1000);
    lds_sub =
         nh.subscribe("scan", 100, &RosNode::LDSMsgCallback,this);
    image_sub =
         nh.subscribe("cv_camera/image_raw", 1, &RosNode::imageCallback,this);
//    ROS_INFO("%s", "create ros node");

    ros::start();
}

RosNode::~RosNode(){
    if(ros::isStarted()) {
      ros::shutdown(); // explicitly needed since we use ros::start();
      ros::waitForShutdown();
    }
    wait();
}

void RosNode::run()
{
    ros::Rate loop_rate(20);    //20hz : 0.2Sec
    while(ros::ok())
    {
        ros::spinOnce();
        loop_rate.sleep();
    }
    emit rosShutdown();
}

void RosNode::LDSMsgCallback(const sensor_msgs::LaserScan::ConstPtr &msg)
{

  float scanData[4];
  scanData[0] = msg->ranges[0];
  scanData[1] = msg->ranges[90];
  scanData[2] = msg->ranges[180];
  scanData[3] = msg->ranges[270];
//  qDebug() << "scanData[0] : " << scanData[0];
  emit sigLdsReceive(scanData);
}


void RosNode::imageCallback(const sensor_msgs::ImageConstPtr& msg)
{

  cv::Mat frame,cframe;
  try
  {
    frame = cv_bridge::toCvShare(msg, "bgr8")->image;
//    imwrite("cap.jpg",frame);
  }
  catch (cv_bridge::Exception& e)
  {
    ROS_ERROR("Could not convert from '%s' to 'bgr8'.", msg->encoding.c_str());
  }
  cvtColor(frame, frame,  cv::COLOR_BGR2RGB);
  cv::line(frame, cv::Point((frame.cols >> 1) - 20, frame.rows >> 1), cv::Point((frame.cols >> 1) + 20, frame.rows >> 1), cv::Scalar(255, 0, 0), 3);
  cv::line(frame, cv::Point(frame.cols >> 1, (frame.rows >> 1) - 20), cv::Point(frame.cols >> 1, (frame.rows >> 1) + 20), cv::Scalar(255, 0, 0), 3);
  QImage * pImage = new QImage(frame.data, frame.cols, frame.rows, QImage::Format_RGB888);
  QImage repImage = pImage->scaled( pLcamView->height(),pLcamView->width(), Qt::KeepAspectRatio);
  // qDebug() << "image";
  copyImg = frame;

  pLcamView->setPixmap(QPixmap::fromImage(repImage));
}

void RosNode::sendDataToRos(const std::string &dataToSend)
{
    std_msgs::String msg;
    msg.data = dataToSend;

    data_pub.publish(msg);
}

/*
    imageCallBack 함수에서 이미지 1프레임 복사
*/
void RosNode::slotCopyRosImage(cv::Mat& mat)
{
    mat = copyImg.clone();
}
