/*
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2009, Willow Garage, Inc.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of Willow Garage, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id$
 *
 */
/** \author Stefan Holzer */

#include <gtest/gtest.h>

#include <pcl/point_types.h>
#include <pcl/io/pcd_io.h>
#include <pcl/features/integral_image_normal.h>

#include <iostream>

using namespace pcl;
using namespace pcl::io;
using namespace std;

PointCloud<PointXYZ> cloud;
IntegralImageNormalEstimation<PointXYZ, Normal> normalEstimator;


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TEST (PCL, IINormalEstimation)
{
//  Normal normal;
//  normalEstimator.computePointNormal (cloud.width/2, cloud.height/2, normal);
  
//  EXPECT_NEAR (fabs (normal.normal_x), 0.0662114, 1e-2);
//  EXPECT_NEAR (fabs (normal.normal_y), 0.599297,  1e-2);
//  EXPECT_NEAR (fabs (normal.normal_z), 0.797792,  1e-2);
}


/* ---[ */
int
  main (int argc, char** argv)
{
  if (argc < 2)
  {
    std::cerr << "No test file given. Please download `table_scene_mug_stereo_textured.pcd` and pass its path to the test." << std::endl;
    return (-1);
  }

  sensor_msgs::PointCloud2 cloud_blob;
  if (loadPCDFile (argv[1], cloud_blob) < 0)
  {
    std::cerr << "Failed to read test file. Please download `table_scene_mug_stereo_textured.pcd` and pass its path to the test." << std::endl;
    return (-1);
  }

  fromROSMsg (cloud_blob, cloud);
  
  /*normalEstimator.setInputData(
    reinterpret_cast<float*>(&(cloud.points[0])),
    cloud.width, cloud.height,
    3, sizeof(cloud.points[0])/sizeof(float), (sizeof(cloud.points[0])/sizeof(float))*cloud.width, 10.0f);
  */
  normalEstimator.setRectSize(2, 2);
  
  testing::InitGoogleTest (&argc, argv);
  return (RUN_ALL_TESTS ());
  
  return 1;
}
/* ]--- */

