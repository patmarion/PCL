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

#include <gtest/gtest.h>

#include <pcl/point_types.h>
#include <pcl/features/normal_3d.h>
#include <pcl/features/integral_image_normal.h>

#include <iostream>

using namespace pcl;
using namespace std;

typedef search::KdTree<PointXYZ>::Ptr KdTreePtr;
PointCloud<PointXYZ> cloud;
KdTreePtr tree;

NormalEstimation<PointXYZ, Normal> n;
IntegralImageNormalEstimation<PointXYZ, Normal> ne;

TEST(PCL, IntegralImage)
{
  const unsigned width = 640;
  const unsigned height = 480;
  const unsigned max_window_size = 11;
  IntegralImage2Dim<float,1> integral_image(true);
  
  // test for dense data with elementstride = 1
  float* data = new float[width * height];
  for(unsigned yIdx = 0; yIdx < height; ++yIdx)
  {
    for(unsigned xIdx = 0; xIdx < width; ++xIdx)
    {
      data[width * yIdx + xIdx] = 1;
    }
  }
  integral_image.setInput (data, width, height, 1, width);
  
  for (unsigned window_width = 2; window_width < max_window_size; ++window_width)
  {
    for (unsigned window_height = 2; window_height < max_window_size; ++window_height)
    {
      for(unsigned yIdx = 0; yIdx < height - window_height; ++yIdx)
      {
        for(unsigned xIdx = 0; xIdx < width - window_width; ++xIdx)
        {
          //std::cout << xIdx << " , " << yIdx << " - " << window_width << " x " << window_height << " : " << integral_image.getFirstOrderSum (xIdx, yIdx, window_width, window_height)[0] << std::endl;
          EXPECT_EQ (window_width * window_height, integral_image.getFirstOrderSum (xIdx, yIdx, window_width, window_height)[0]);
        }
      }
    } 
  }
  delete[] data;

  //now test with element-stride 2
  unsigned element_stride = 2;
  unsigned row_stride = width * element_stride;  
  data = new float[row_stride * height];
  for(unsigned yIdx = 0; yIdx < height; ++yIdx)
  {
    for(unsigned xIdx = 0; xIdx < row_stride; xIdx += element_stride)
    {
      data[row_stride * yIdx + xIdx] = 1;
      data[row_stride * yIdx + xIdx + 1] = -1;
    }
  }
  integral_image.setInput (data, width, height, element_stride, row_stride);
  for (unsigned window_width = 1; window_width < max_window_size; ++window_width)
  {
    for (unsigned window_height = 1; window_height < max_window_size; ++window_height)
    {
      for(unsigned yIdx = 0; yIdx < height - window_height; ++yIdx)
      {
        for(unsigned xIdx = 0; xIdx < width - window_width; ++xIdx)
        {
          //std::cout << xIdx << " , " << yIdx << " - " << window_width << " x " << window_height << " : " << integral_image.getFirstOrderSum (xIdx, yIdx, window_width, window_height)[0] << std::endl;
          EXPECT_EQ (window_width * window_height, integral_image.getFirstOrderSum (xIdx, yIdx, window_width, window_height)[0]);
        }
      }
    } 
  }
  delete[] data;

  //now test with odd element-stride 3 and modulo-uneven row_stride
  element_stride = 3;
  row_stride = width * element_stride;
  data = new float[row_stride * height];
  for(unsigned yIdx = 0; yIdx < height; ++yIdx)
  {
    for(unsigned xIdx = 0; xIdx < row_stride; xIdx += element_stride)
    {
      data[row_stride * yIdx + xIdx] = 1;
      data[row_stride * yIdx + xIdx + 1] = 2;
      data[row_stride * yIdx + xIdx + 2] = xIdx;
    }
  }
  integral_image.setInput (data, width, height, element_stride, row_stride);
  for (unsigned window_width = 1; window_width < max_window_size; ++window_width)
  {
    for (unsigned window_height = 1; window_height < max_window_size; ++window_height)
    {
      for(unsigned yIdx = 0; yIdx < height - window_height; ++yIdx)
      {
        for(unsigned xIdx = 0; xIdx < width - window_width; ++xIdx)
        {
          EXPECT_EQ (window_width * window_height, integral_image.getFirstOrderSum (xIdx, yIdx, window_width, window_height)[0]);
          EXPECT_EQ (window_width * window_height, integral_image.getSecondOrderSum (xIdx, yIdx, window_width, window_height)[0]);
//std::cout << xIdx << " , " << yIdx << " - " << window_width << " x " << window_height << " : " << integral_image.getSecondOrderSum (xIdx, yIdx, window_width, window_height)[0] << std::endl;
        }
      }
    } 
  }
  delete[] data;

  // now test multidimensional case with 3D but element_stride = 4 and row_stride non-dividable by element_stride
  IntegralImage2Dim<float, 3> integral_image3(true);
  element_stride = 4;
  row_stride = width * element_stride;
  data = new float[row_stride * height];
  for(unsigned yIdx = 0; yIdx < height; ++yIdx)
  {
    for(unsigned xIdx = 0; xIdx < width; ++xIdx)
    {
      data[row_stride * yIdx + xIdx * element_stride] = xIdx;
      data[row_stride * yIdx + xIdx * element_stride + 1] = yIdx;
      data[row_stride * yIdx + xIdx * element_stride + 2] = xIdx + yIdx;
    }
  }
  integral_image3.setInput (data, width, height, element_stride, row_stride);
  for (unsigned window_width = 1; window_width < max_window_size; ++window_width)
  {
    for (unsigned window_height = 1; window_height < max_window_size; ++window_height)
    {
      for(unsigned yIdx = 0; yIdx < height - window_height; ++yIdx)
      {
        for(unsigned xIdx = 0; xIdx < width - window_width; ++xIdx)
        {
          IntegralImage2Dim<float, 3>::ElementType sum = integral_image3.getFirstOrderSum (xIdx, yIdx, window_width, window_height);

          EXPECT_EQ (window_height * window_width * (window_width + 2 * xIdx - 1), sum[0] * 2);
          EXPECT_EQ (window_width * window_height * (window_height + 2 * yIdx - 1), sum[1] * 2);
          EXPECT_EQ (window_width * window_height * (window_height + 2 * yIdx - 1) + window_height * window_width * (window_width + 2 * xIdx - 1), sum[2] * 2);

          IntegralImage2Dim<float, 3>::SecondOrderType sumSqr = integral_image3.getSecondOrderSum (xIdx, yIdx, window_width, window_height);
  
          IntegralImage2Dim<float, 3>::SecondOrderType ground_truth;
          ground_truth.setZero ();
          for (unsigned wy = yIdx; wy < yIdx + window_height; ++wy)
          {
            for (unsigned wx = xIdx; wx < xIdx + window_width; ++wx)
            {
              float* val = data + (wy * row_stride + wx * element_stride);
              //ground_truth[0] += val[0] * val[0];
              ground_truth[1] += val[0] * val[1];
              ground_truth[2] += val[0] * val[2];
              //ground_truth[3] += val[1] * val[1];
              ground_truth[4] += val[1] * val[2];
              ground_truth[5] += val[2] * val[2];
            }
          }

          //EXPECT_EQ (ground_truth [0], sumSqr[0]);
          EXPECT_EQ (ground_truth [1], sumSqr[1]);
          EXPECT_EQ (ground_truth [2], sumSqr[2]);
          //EXPECT_EQ (ground_truth [3], sumSqr[3]);
          EXPECT_EQ (ground_truth [4], sumSqr[4]);
          EXPECT_EQ (ground_truth [5], sumSqr[5]);

          int w = window_width + xIdx - 1;
          long result = w * (w + 1) * (2*w + 1) - xIdx * (xIdx - 1) * (2*xIdx - 1);

          EXPECT_EQ (window_height * result, sumSqr[0] * 6);

          int h = window_height + yIdx - 1;
          result = h * (h + 1) * (2*h + 1) - yIdx * (yIdx - 1) * (2*yIdx - 1);
          EXPECT_EQ (window_width * result, sumSqr[3] * 6);
        }
      }
    } 
  }
  delete[] data;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TEST (PCL, NormalEstimation)
{
  tree.reset (new search::KdTree<PointXYZ> (false));
  n.setSearchMethod (tree);
  n.setKSearch (10);

  n.setInputCloud (cloud.makeShared ());

  PointCloud<Normal> output;
  n.compute (output);

  EXPECT_EQ (output.points.size (), cloud.points.size ());
  EXPECT_EQ (output.width, cloud.width);
  EXPECT_EQ (output.height, cloud.height);

  for (size_t i = 0; i < cloud.points.size (); ++i)
  {
    EXPECT_NEAR (fabs (output.points[i].normal_x),   0, 1e-2);
    EXPECT_NEAR (fabs (output.points[i].normal_y),   0, 1e-2);
    EXPECT_NEAR (fabs (output.points[i].normal_z), 1.0, 1e-2);
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TEST (PCL, IINormalEstimation)
{
  Normal normal;
  ne.setInputCloud (cloud.makeShared ());
  ne.setRectSize (2, 2);
  ne.computePointNormal (160, 120, normal);
  ne.setNormalEstimationMethod (ne.COVARIANCE_MATRIX);

  EXPECT_NEAR (fabs (normal.normal_x),   0, 1e-2);
  EXPECT_NEAR (fabs (normal.normal_y),   0, 1e-2);
  EXPECT_NEAR (fabs (normal.normal_z), 1.0, 1e-2);

  PointCloud<Normal> output;
  ne.compute (output);

  EXPECT_EQ (output.points.size (), cloud.points.size ());
  EXPECT_EQ (output.width, cloud.width);
  EXPECT_EQ (output.height, cloud.height);

  for (size_t v = 0; v < cloud.height; ++v)
  {
    for (size_t u = 0; u < cloud.width; ++u)
    {
      if (!pcl_isfinite(output (u, v).normal_x) && 
          !pcl_isfinite(output (u, v).normal_y) && 
          !pcl_isfinite(output (u, v).normal_z)) 
        continue;

      EXPECT_NEAR (fabs (output (u, v).normal_x),   0, 1e-2);
      EXPECT_NEAR (fabs (output (u, v).normal_y),   0, 1e-2);
      EXPECT_NEAR (fabs (output (u, v).normal_z), 1.0, 1e-2);
    }
  }
  EXPECT_NEAR (fabs (output (160, 120).normal_x),   0, 1e-2);
  EXPECT_NEAR (fabs (output (160, 120).normal_y),   0, 1e-2);
  EXPECT_NEAR (fabs (output (160, 120).normal_z), 1.0, 1e-2);


  ne.setNormalEstimationMethod (ne.AVERAGE_3D_GRADIENT);
  ne.compute (output);

  EXPECT_EQ (output.points.size (), cloud.points.size ());
  EXPECT_EQ (output.width, cloud.width);
  EXPECT_EQ (output.height, cloud.height);

  for (size_t v = 0; v < cloud.height; ++v)
  {
    for (size_t u = 0; u < cloud.width; ++u)
    {
      if (!pcl_isfinite(output (u, v).normal_x) && 
          !pcl_isfinite(output (u, v).normal_y) && 
          !pcl_isfinite(output (u, v).normal_z)) 
        continue;

      EXPECT_NEAR (fabs (output (u, v).normal_x),   0, 1e-2);
      EXPECT_NEAR (fabs (output (u, v).normal_y),   0, 1e-2);
      EXPECT_NEAR (fabs (output (u, v).normal_z), 1.0, 1e-2);
    }
  }
}


/* ---[ */
int
main (int argc, char** argv)
{
  cloud.points.resize (320 * 240);
  cloud.width = 320;
  cloud.height = 240;
  cloud.is_dense = true;
  for (size_t v = 0; v < cloud.height; ++v)
  {
    for (size_t u = 0; u < cloud.width; ++u)
    {
      cloud (u, v).x = u;
      cloud (u, v).y = v;
      cloud (u, v).z = 10;
    }
  }

  testing::InitGoogleTest (&argc, argv);
  return (RUN_ALL_TESTS ());
  
  return 1;
}
/* ]--- */

