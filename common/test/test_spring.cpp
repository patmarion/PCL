/*
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2010, Willow Garage, Inc.
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
 * $Id: test_spring.cpp 3624 2011-12-22 01:18:57Z nizar $
 */

#include <gtest/gtest.h>
#include <pcl/pcl_config.h>
#include <pcl/pcl_tests.h>
#include <pcl/point_types.h>
#include <pcl/common/spring.h>

using namespace pcl;
using namespace pcl::test;

PointCloud<PointXYZ> cloud(5, 4);
const size_t size = 5 * 4;
const int amount = 2;

// TEST(PointCloudSpring, vertical)
// {
//   PointCloudSpring<PointXYZ> spring;
//   spring.setInputCloud (cloud_ptr);
//   spring.setAmount (2);
//   spring.setDirection (PointCloudSpring<PointXYZ>::VERTICAL);
//   spring.setExpandPolicy (PointCloudSpring<PointXYZ>::MIRROR);

//   PointXYZ xyz;
//   xyz.x = 0; xyz.y = 0; xyz.z = 0;
//   int old_height (cloud_ptr->height);
//   spring.expand (xyz);
//   EXPECT_EQ (cloud_ptr->height, old_height +2*2);
// }

// TEST(PointCloudSpring, horizontal)
// {
//   PointCloudSpring<PointXYZ> spring;
//   spring.setInputCloud (cloud_ptr);
//   spring.setAmount (2);
//   spring.setDirection (PointCloudSpring<PointXYZ>::HORIZONTAL);
//   spring.setExpandPolicy (PointCloudSpring<PointXYZ>::MIRROR);

//   PointXYZ xyz;
//   xyz.x = 0; xyz.y = 0; xyz.z = 0;
//   int old_width (cloud_ptr->width);
//   for (int i = 0; i < cloud_ptr->height; i++)    
//   {
//     std::cout << (*cloud_ptr) (0,i) << std::endl;
//   }
//   spring.expand (xyz);
//   for (int i = 0; i < cloud_ptr->height; i++)    
//   {
//     std::cout << (*cloud_ptr) (0,i) << std::endl;
//   }
//   EXPECT_EQ (cloud_ptr->width, old_width +2*2);
// }

TEST(PointCloudSpring, duplicateVertical)
{
  PointCloudSpring<PointXYZ> spring;
  PointCloud<PointXYZ>::Ptr cloud_ptr = cloud.makeShared ();
  spring.setInputCloud (cloud_ptr);
  spring.setAmount (amount);
  spring.setDirection (PointCloudSpring<PointXYZ>::VERTICAL);
  spring.setExpandPolicy (PointCloudSpring<PointXYZ>::DUPLICATE);
  spring.expand ();
  int w(cloud_ptr->width);
  EXPECT_EQ (cloud_ptr->height, cloud.height +2*amount);

  for (int i = 0; i < w; ++i)
  {
    EXPECT_EQ_VECTORS((*cloud_ptr)(i,0).getVector3fMap (), 
                      (*cloud_ptr)(i,1).getVector3fMap ());
    EXPECT_EQ_VECTORS((*cloud_ptr)(i,0).getVector3fMap (), 
                      (*cloud_ptr)(i,2).getVector3fMap ());
    EXPECT_EQ_VECTORS((*cloud_ptr)(i,cloud_ptr->height - 3).getVector3fMap (), 
                      (*cloud_ptr)(i,cloud_ptr->height - 1).getVector3fMap ());
    EXPECT_EQ_VECTORS((*cloud_ptr)(i,cloud_ptr->height - 3).getVector3fMap (), 
                      (*cloud_ptr)(i,cloud_ptr->height - 2).getVector3fMap ());
  }
}

TEST(PointCloudSpring, mirrorVertical)
{
  PointCloudSpring<PointXYZ> spring;
  PointCloud<PointXYZ>::Ptr cloud_ptr = cloud.makeShared ();
  spring.setInputCloud (cloud_ptr);
  spring.setAmount (amount);
  spring.setDirection (PointCloudSpring<PointXYZ>::VERTICAL);
  spring.setExpandPolicy (PointCloudSpring<PointXYZ>::MIRROR);
  spring.expand ();
  int w(cloud_ptr->width);
  int h(cloud_ptr->height);
  EXPECT_EQ (cloud_ptr->height, cloud.height +2*amount);

  for (int i = 0; i < w; ++i)
  {
    EXPECT_EQ_VECTORS((*cloud_ptr)(i,1).getVector3fMap (), 
                      (*cloud_ptr)(i,2).getVector3fMap ());
    EXPECT_EQ_VECTORS((*cloud_ptr)(i,0).getVector3fMap (), 
                      (*cloud_ptr)(i,3).getVector3fMap ());
    EXPECT_EQ_VECTORS((*cloud_ptr)(i,h - 3).getVector3fMap (), 
                      (*cloud_ptr)(i,h - 2).getVector3fMap ());
    EXPECT_EQ_VECTORS((*cloud_ptr)(i,h - 4).getVector3fMap (), 
                      (*cloud_ptr)(i,h - 1).getVector3fMap ());
  }
}

TEST(PointCloudSpring, mirrorHorizontal)
{
  PointCloudSpring<PointXYZ> spring;
  PointCloud<PointXYZ>::Ptr cloud_ptr = cloud.makeShared ();
  spring.setInputCloud (cloud_ptr);
  spring.setAmount (amount);
  spring.setDirection (PointCloudSpring<PointXYZ>::HORIZONTAL);
  spring.setExpandPolicy (PointCloudSpring<PointXYZ>::MIRROR);
  spring.expand ();
  int w(cloud_ptr->width);
  int h(cloud_ptr->height);
  EXPECT_EQ (cloud_ptr->width, cloud.width +2*amount);

  for (int j = 0; j < h; ++j)
  {
    EXPECT_EQ_VECTORS((*cloud_ptr)(0,j).getVector3fMap (), 
                      (*cloud_ptr)(3,j).getVector3fMap ());
    EXPECT_EQ_VECTORS((*cloud_ptr)(1,j).getVector3fMap (), 
                      (*cloud_ptr)(2,j).getVector3fMap ());
    EXPECT_EQ_VECTORS((*cloud_ptr)(w - 3,j).getVector3fMap (), 
                      (*cloud_ptr)(w - 2,j).getVector3fMap ());
    EXPECT_EQ_VECTORS((*cloud_ptr)(w - 4,j).getVector3fMap (), 
                      (*cloud_ptr)(w - 1,j).getVector3fMap ());
  }
}

TEST(PointCloudSpring, duplicateHorizontal)
{
  PointCloudSpring<PointXYZ> spring;
  PointCloud<PointXYZ>::Ptr cloud_ptr = cloud.makeShared ();
  spring.setInputCloud (cloud_ptr);
  spring.setAmount (amount);
  spring.setDirection (PointCloudSpring<PointXYZ>::HORIZONTAL);
  spring.setExpandPolicy (PointCloudSpring<PointXYZ>::DUPLICATE);
  spring.expand ();
  int h(cloud_ptr->height);
  int w(cloud_ptr->width);
  EXPECT_EQ (cloud_ptr->width, cloud.width +2*amount);

  for (int i = 0; i < h; ++i)
  {
    EXPECT_EQ_VECTORS((*cloud_ptr)(0,i).getVector3fMap (), 
                      (*cloud_ptr)(1,i).getVector3fMap ());
    EXPECT_EQ_VECTORS((*cloud_ptr)(0,i).getVector3fMap (), 
                      (*cloud_ptr)(2,i).getVector3fMap ());
    EXPECT_EQ_VECTORS((*cloud_ptr)(w - 3,i).getVector3fMap (), 
                      (*cloud_ptr)(w - 1,i).getVector3fMap ());
    EXPECT_EQ_VECTORS((*cloud_ptr)(w - 3,i).getVector3fMap (), 
                      (*cloud_ptr)(w - 2,i).getVector3fMap ());
  }
}

TEST(PointCloudSpring, deleteRows)
{
  PointCloudSpring<PointXYZ> spring;
  PointCloud<PointXYZ>::Ptr cloud_ptr = cloud.makeShared ();
  spring.setInputCloud (cloud_ptr);
  spring.setAmount (amount);
  spring.setDirection (PointCloudSpring<PointXYZ>::VERTICAL);
  spring.setExpandPolicy (PointCloudSpring<PointXYZ>::MIRROR);
  spring.expand ();
  EXPECT_EQ (cloud_ptr->height, cloud.height +2*amount);
  spring.shrink ();
  EXPECT_EQ (cloud_ptr->height, cloud.height);
  for(uint32_t i = 0; i < cloud.width; i++)
    for(uint32_t j = 0; j < cloud.height; j++)
      EXPECT_EQ_VECTORS ((*cloud_ptr)(i,j).getVector3fMap (),
                         cloud(i,j).getVector3fMap ());
}

TEST(PointCloudSpring, deleteCols)
{
  PointCloudSpring<PointXYZ> spring;
  PointCloud<PointXYZ>::Ptr cloud_ptr = cloud.makeShared ();
  spring.setInputCloud (cloud_ptr);
  spring.setAmount (amount);
  spring.setDirection (PointCloudSpring<PointXYZ>::HORIZONTAL);
  spring.setExpandPolicy (PointCloudSpring<PointXYZ>::MIRROR);
  spring.expand ();
  EXPECT_EQ (cloud_ptr->width, cloud.width +2*amount);
  spring.shrink ();
  EXPECT_EQ (cloud_ptr->width, cloud.width);
  for(uint32_t i = 0; i < cloud.width; i++)
    for(uint32_t j = 0; j < cloud.height; j++)
      EXPECT_EQ_VECTORS ((*cloud_ptr)(i,j).getVector3fMap (),
                         cloud(i,j).getVector3fMap ());
}

int
main (int argc, char** argv)
{
  for (uint32_t i = 0; i < size; ++i)
    cloud[i]  = PointXYZ (3*i+0,3*i+1,3*i+2);

  testing::InitGoogleTest (&argc, argv);
  return (RUN_ALL_TESTS ());
}
