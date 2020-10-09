// Copyright (c) 2020 Pilz GmbH & Co. KG
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef PSEN_SCAN_V2_ROS_SCANNER_NODE_H
#define PSEN_SCAN_V2_ROS_SCANNER_NODE_H

#include <stdexcept>
#include <string>
#include <atomic>
#include <gtest/gtest_prod.h>

#include <ros/ros.h>

#include "psen_scan_v2/scanner.h"
#include "psen_scan_v2/laserscan_ros_conversions.h"

namespace psen_scan_v2
{
/**
 * @brief ROS Node for fetching and publishing laserscan data from the scanner.
 *
 */
template <typename S = Scanner>
class ROSScannerNodeT
{
public:
  /**
   * @brief Constructor.
   *
   * @param nh Node handle for the ROS node on which the scanner topic is advertised.
   * @param topic Name of the ROS topic under which the scanner data are published.
   * @param frame_id Name of the frame id.
   * @param x_axis_rotation Rotation of 2D scan around the z-axis.
   * @param scanner_config Scanner configuration.
   */
  ROSScannerNodeT(ros::NodeHandle& nh,
                  const std::string& topic,
                  const std::string& frame_id,
                  const double& x_axis_rotation,
                  const ScannerConfiguration& scanner_config);

  //! @brief Continuously fetches data from the scanner and publishes the data as ROS scanner message.
  void run();
  //! @brief Terminates the fetching and publishing of scanner data.
  void terminate();

private:
  void laserScanCallback(const LaserScan& scan);

private:
  ros::NodeHandle nh_;
  ros::Publisher pub_;
  std::string frame_id_;
  double x_axis_rotation_;
  S scanner_;
  std::atomic_bool terminate_{ false };

  friend class RosScannerNodeTests;
  FRIEND_TEST(RosScannerNodeTests, testScannerInvocation);
  FRIEND_TEST(RosScannerNodeTests, testScanTopicReceived);
  FRIEND_TEST(RosScannerNodeTests, testScanBuildFailure);
};

typedef ROSScannerNodeT<> ROSScannerNode;

template <typename S>
ROSScannerNodeT<S>::ROSScannerNodeT(ros::NodeHandle& nh,
                                    const std::string& topic,
                                    const std::string& frame_id,
                                    const double& x_axis_rotation,
                                    const ScannerConfiguration& scanner_config)
  : nh_(nh)
  , frame_id_(frame_id)
  , x_axis_rotation_(x_axis_rotation)
  , scanner_(scanner_config, std::bind(&ROSScannerNodeT<S>::laserScanCallback, this, std::placeholders::_1))
{
  pub_ = nh_.advertise<sensor_msgs::LaserScan>(topic, 1);
}

template <typename S>
void ROSScannerNodeT<S>::laserScanCallback(const LaserScan& scan)
{
  pub_.publish(toLaserScanMsg(scan, frame_id_, x_axis_rotation_));
}

template <typename S>
void ROSScannerNodeT<S>::terminate()
{
  terminate_ = true;
}

template <typename S>
void ROSScannerNodeT<S>::run()
{
  ros::Rate r(10);
  scanner_.start();
  while (ros::ok() && !terminate_)
  {
    r.sleep();
  }
  scanner_.stop();
}

}  // namespace psen_scan_v2

#endif  // PSEN_SCAN_V2_ROS_SCANNER_NODE_H