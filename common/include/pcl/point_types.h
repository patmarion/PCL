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
 * $Id$
 *
 */
#ifndef PCL_DATA_TYPES_H_
#define PCL_DATA_TYPES_H_

#if defined __INTEL_COMPILER
  #pragma warning disable 2196 2536 279
#endif

#include <Eigen/Core>
#include <bitset>
#include <vector>
#include "pcl/ros/register_point_struct.h"
#include "pcl/win32_macros.h"
#include <math.h>

/**
  * \file pcl/point_types.h
  * Defines all the PCL implemented PointT point type structures
  * \ingroup common
  */

/** @{*/
namespace pcl
{
  struct PointXYZ;
  // Members: float x, y, z;

  struct PointXYZI;
  // Members: float x, y, z, intensity; 

  struct PointXYZRGBA;
  // Members: float x, y, z; uint32_t rgba;

  struct PointXYZRGB;
  // Members: float x, y, z, rgb;

  struct PointXY;
  // Members: float x, y;

  struct InterestPoint;
  // Members: float x, y, z, strength;

  struct Normal;
  // Members: float normal[3], curvature;

  struct PointNormal;
  // Members: float x, y, z; float normal[3], curvature;

  struct PointXYZRGBNormal;
  // Members: float x, y, z, rgb, normal[3], curvature;

  struct PointXYZINormal;
  // Members: float x, y, z, intensity, normal[3], curvature;

  struct PointWithRange;
  // Members: float x, y, z (union with float point[4]), range;

  struct PointWithViewpoint;
  // Members: float x, y, z, vp_x, vp_y, vp_z;

  struct MomentInvariants;
  // Members: float j1, j2, j3;

  // TODO add point type for Radius-based Surface Descriptor (RSD) histograms, and let pcl::RSDEstimation return it if needed

  struct PrincipalRadiiRSD;
  // Members: float r_min, r_max;

  struct Boundary;
  // Members: uint8_t boundary_point;

  struct PrincipalCurvatures;
  // Members: float principal_curvature[3], pc1, pc2;

  struct PFHSignature125;
  // Members: float pfh[125];

  struct FPFHSignature33;
  // Members: float fpfh[33];

  struct VFHSignature308;
  // Members: float vfh[308];
  
  struct Narf36;
  // Members: float x, y, z, roll, pitch, yaw; float descriptor[36];


  /** \brief Data type to store extended information about a transition from foreground to backgroundSpecification of the fields for BorderDescription::traits. 
    * \ingroup common
    */
  typedef std::bitset<32> BorderTraits;

  /** \brief Specification of the fields for BorderDescription::traits. 
    * \ingroup common
    */
  enum BorderTrait 
  {
    BORDER_TRAIT__OBSTACLE_BORDER,
    BORDER_TRAIT__SHADOW_BORDER,
    BORDER_TRAIT__VEIL_POINT,
    BORDER_TRAIT__SHADOW_BORDER_TOP,
    BORDER_TRAIT__SHADOW_BORDER_RIGHT,
    BORDER_TRAIT__SHADOW_BORDER_BOTTOM,
    BORDER_TRAIT__SHADOW_BORDER_LEFT,
    BORDER_TRAIT__OBSTACLE_BORDER_TOP,
    BORDER_TRAIT__OBSTACLE_BORDER_RIGHT,
    BORDER_TRAIT__OBSTACLE_BORDER_BOTTOM,
    BORDER_TRAIT__OBSTACLE_BORDER_LEFT,
    BORDER_TRAIT__VEIL_POINT_TOP,
    BORDER_TRAIT__VEIL_POINT_RIGHT,
    BORDER_TRAIT__VEIL_POINT_BOTTOM,
    BORDER_TRAIT__VEIL_POINT_LEFT,
  };
  
  struct BorderDescription;
  // Members: int x, y; BorderTraits traits;

  struct IntensityGradient;
  // Members: float gradient[3];

  template <int N>
  struct Histogram;
  // Members: float histogram[N];

  struct PointWithScale;
  // Members: float x, y, z, scale;
  
  struct PointSurfel;
  // Members: float x, y, z, normal[3], rgba, radius, confidence, curvature;

  // -----Functions on points-----
  //! Calculate the euclidean distance between the two given points.
  template <typename PointType1, typename PointType2> inline float 
  euclideanDistance (const PointType1& p1, const PointType2& p2);
  
  //! Calculate the squared euclidean distance between the two given points.
  template <typename PointType1, typename PointType2> inline float 
  squaredEuclideanDistance (const PointType1& p1, const PointType2& p2);

  //! Checks if x,y,z are finite numbers.
  template <typename PointType> inline bool 
  hasValidXYZ (const PointType& p);
}

/** @} */

#include "pcl/impl/point_types.hpp"  // Include struct definitions


// ==============================
// =====POINT_CLOUD_REGISTER=====
// ==============================

POINT_CLOUD_REGISTER_POINT_STRUCT (pcl::_PointXYZ,
                                   (float, x, x)
                                   (float, y, y)
                                   (float, z, z)
)

POINT_CLOUD_REGISTER_POINT_WRAPPER(pcl::PointXYZ, pcl::_PointXYZ)

POINT_CLOUD_REGISTER_POINT_STRUCT (pcl::PointXYZRGBA,
                                   (float, x, x)
                                   (float, y, y)
                                   (float, z, z)
                                   (uint32_t, rgba, rgba)
)
POINT_CLOUD_REGISTER_POINT_STRUCT (pcl::PointXYZRGB,
                                   (float, x, x)
                                   (float, y, y)
                                   (float, z, z)
                                   (float, rgb, rgb)
)
POINT_CLOUD_REGISTER_POINT_STRUCT (pcl::InterestPoint,
                                   (float, x, x)
                                   (float, y, y)
                                   (float, z, z)
                                   (float, strength, strength)
)

POINT_CLOUD_REGISTER_POINT_STRUCT (pcl::PointXYZI, 
                                   (float, x, x) 
                                   (float, y, y) 
                                   (float, z, z) 
                                   (float, intensity, intensity) 
) 

POINT_CLOUD_REGISTER_POINT_STRUCT (pcl::Normal,
                                   (float, normal_x, normal_x)
                                   (float, normal_y, normal_y)
                                   (float, normal_z, normal_z)
                                   (float, curvature, curvature)
)
POINT_CLOUD_REGISTER_POINT_STRUCT (pcl::PointNormal,
                                   (float, x, x)
                                   (float, y, y)
                                   (float, z, z)
                                   (float, normal_x, normal_x)
                                   (float, normal_y, normal_y)
                                   (float, normal_z, normal_z)
                                   (float, curvature, curvature)
)
POINT_CLOUD_REGISTER_POINT_STRUCT (pcl::PointXYZRGBNormal,
                                   (float, x, x)
                                   (float, y, y)
                                   (float, z, z)
                                   (float, rgb, rgb)
                                   (float, normal_x, normal_x)
                                   (float, normal_y, normal_y)
                                   (float, normal_z, normal_z)
                                   (float, curvature, curvature)
)
POINT_CLOUD_REGISTER_POINT_STRUCT (pcl::PointXYZINormal,
                                   (float, x, x)
                                   (float, y, y)
                                   (float, z, z)
                                   (float, intensity, intensity)
                                   (float, normal_x, normal_x)
                                   (float, normal_y, normal_y)
                                   (float, normal_z, normal_z)
                                   (float, curvature, curvature)
)
POINT_CLOUD_REGISTER_POINT_STRUCT (pcl::PointWithRange,
                                   (float, x, x)
                                   (float, y, y)
                                   (float, z, z)
                                   (float, range, range)
)

POINT_CLOUD_REGISTER_POINT_STRUCT (pcl::_PointWithViewpoint,
                                   (float, x, x)
                                   (float, y, y)
                                   (float, z, z)
                                   (float, vp_x, vp_x)
                                   (float, vp_y, vp_y)
                                   (float, vp_z, vp_z)
)

POINT_CLOUD_REGISTER_POINT_WRAPPER(pcl::PointWithViewpoint, pcl::_PointWithViewpoint)


POINT_CLOUD_REGISTER_POINT_STRUCT (pcl::MomentInvariants,
                                   (float, j1, j1)
                                   (float, j2, j2)
                                   (float, j3, j3)
)

POINT_CLOUD_REGISTER_POINT_STRUCT (pcl::PrincipalRadiiRSD,
                                   (float, r_min, r_min)
                                   (float, r_max, r_max)
)

POINT_CLOUD_REGISTER_POINT_STRUCT (pcl::Boundary,
                                   (uint8_t, boundary_point, boundary_point)
)

POINT_CLOUD_REGISTER_POINT_STRUCT (pcl::PrincipalCurvatures,
                                   (float, principal_curvature_x, principal_curvature_x)
                                   (float, principal_curvature_y, principal_curvature_y)
                                   (float, principal_curvature_z, principal_curvature_z)
                                   (float, pc1, pc1)
                                   (float, pc2, pc2)
)

POINT_CLOUD_REGISTER_POINT_STRUCT (pcl::PFHSignature125,
                                   (float[125], histogram, pfh)
)

POINT_CLOUD_REGISTER_POINT_STRUCT (pcl::FPFHSignature33,
                                   (float[33], histogram, fpfh)
)

POINT_CLOUD_REGISTER_POINT_STRUCT (pcl::VFHSignature308,
                                   (float[308], histogram, vfh)
)

POINT_CLOUD_REGISTER_POINT_STRUCT (pcl::Narf36,
                                   (float[36], descriptor, descriptor)
)

POINT_CLOUD_REGISTER_POINT_STRUCT (pcl::IntensityGradient,
                                   (float, gradient_x, gradient_x)
                                   (float, gradient_y, gradient_y)
                                   (float, gradient_z, gradient_z)
)

POINT_CLOUD_REGISTER_POINT_STRUCT (pcl::PointWithScale,
                                   (float, x, x)
                                   (float, y, y)
                                   (float, z, z)
                                   (float, scale, scale)
)

POINT_CLOUD_REGISTER_POINT_STRUCT(pcl::PointSurfel,
                                   (float, x, x)
                                   (float, y, y)
                                   (float, z, z)
                                   (float, normal_x, normal_x)
                                   (float, normal_y, normal_y)
                                   (float, normal_z, normal_z)
                                   (uint32_t, rgba, rgba)
                                   (float, radius, radius)
                                   (float, confidence, confidence)
                                   (float, curvature, curvature)
)

//POINT_CLOUD_REGISTER_POINT_STRUCT (pcl::BorderDescription,
                                   //(int, x, x)
                                   //(int, y, y)
                                   //(uint32_t, traits_int, traits)
//)


#endif  //#ifndef PCL_DATA_TYPES_H_
