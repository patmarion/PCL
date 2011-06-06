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

#ifndef PCL_IMPL_POINT_TYPES_HPP_
#define PCL_IMPL_POINT_TYPES_HPP_

// Define all PCL point types
#define PCL_POINT_TYPES       \
  (pcl::PointXYZ)             \
  (pcl::PointXYZI)            \
  (pcl::PointXYZRGBA)         \
  (pcl::PointXYZRGB)          \
  (pcl::PointXY)              \
  (pcl::InterestPoint)        \
  (pcl::Normal)               \
  (pcl::PointNormal)          \
  (pcl::PointXYZRGBNormal)    \
  (pcl::PointXYZINormal)      \
  (pcl::PointWithRange)       \
  (pcl::PointWithViewpoint)   \
  (pcl::MomentInvariants)     \
  (pcl::PrincipalRadiiRSD)    \
  (pcl::Boundary)             \
  (pcl::PrincipalCurvatures)  \
  (pcl::PFHSignature125)      \
  (pcl::FPFHSignature33)      \
  (pcl::VFHSignature308)      \
  (pcl::Narf36)               \
  (pcl::BorderDescription)    \
  (pcl::IntensityGradient)    \
  (pcl::Histogram<2>)         \
  (pcl::PointWithScale)

// Define all point types that include XYZ data
#define PCL_XYZ_POINT_TYPES   \
  (pcl::PointXYZ)             \
  (pcl::PointXYZI)            \
  (pcl::PointXYZRGBA)         \
  (pcl::PointXYZRGB)          \
  (pcl::InterestPoint)        \
  (pcl::PointNormal)          \
  (pcl::PointXYZRGBNormal)    \
  (pcl::PointXYZINormal)      \
  (pcl::PointWithRange)       \
  (pcl::PointWithViewpoint)   \
  (pcl::PointWithScale)

// Define all point types that include normal[3] data
#define PCL_NORMAL_POINT_TYPES  \
  (pcl::Normal)                 \
  (pcl::PointNormal)            \
  (pcl::PointXYZRGBNormal)      \
  (pcl::PointXYZINormal)

namespace pcl
{

#define PCL_ADD_POINT4D \
  union { \
    float data[4]; \
    struct { \
      float x; \
      float y; \
      float z; \
    }; \
  } EIGEN_ALIGN16; \
  inline Eigen::Map<Eigen::Vector3f> getVector3fMap () { return (Eigen::Vector3f::Map (data)); } \
  inline const Eigen::Map<const Eigen::Vector3f> getVector3fMap () const { return (Eigen::Vector3f::Map (data)); } \
  inline Eigen::Map<Eigen::Vector4f, Eigen::Aligned> getVector4fMap () { return (Eigen::Vector4f::MapAligned (data)); } \
  inline const Eigen::Map<const Eigen::Vector4f, Eigen::Aligned> getVector4fMap () const { return (Eigen::Vector4f::MapAligned (data)); } \
  inline Eigen::Map<Eigen::Array3f> getArray3fMap () { return (Eigen::Array3f::Map (data)); } \
  inline const Eigen::Map<const Eigen::Array3f> getArray3fMap () const { return (Eigen::Array3f::Map (data)); } \
  inline Eigen::Map<Eigen::Array4f, Eigen::Aligned> getArray4fMap () { return (Eigen::Array4f::MapAligned (data)); } \
  inline const Eigen::Map<const Eigen::Array4f, Eigen::Aligned> getArray4fMap () const { return (Eigen::Array4f::MapAligned (data)); }

#define PCL_ADD_NORMAL4D \
  union { \
    float data_n[4]; \
    float normal[3]; \
    struct { \
      float normal_x; \
      float normal_y; \
      float normal_z; \
    }; \
  } EIGEN_ALIGN16; \
  inline Eigen::Map<Eigen::Vector3f> getNormalVector3fMap () { return (Eigen::Vector3f::Map (data_n)); } \
  inline const Eigen::Map<const Eigen::Vector3f> getNormalVector3fMap () const { return (Eigen::Vector3f::Map (data_n)); } \
  inline Eigen::Map<Eigen::Vector4f, Eigen::Aligned> getNormalVector4fMap () { return (Eigen::Vector4f::MapAligned (data_n)); } \
  inline const Eigen::Map<const Eigen::Vector4f, Eigen::Aligned> getNormalVector4fMap () const { return (Eigen::Vector4f::MapAligned (data_n)); }

typedef Eigen::Map<Eigen::Array3f> Array3fMap;
typedef const Eigen::Map<const Eigen::Array3f> Array3fMapConst;
typedef Eigen::Map<Eigen::Array4f, Eigen::Aligned> Array4fMap;
typedef const Eigen::Map<const Eigen::Array4f, Eigen::Aligned> Array4fMapConst;
typedef Eigen::Map<Eigen::Vector3f> Vector3fMap;
typedef const Eigen::Map<const Eigen::Vector3f> Vector3fMapConst;
typedef Eigen::Map<Eigen::Vector4f, Eigen::Aligned> Vector4fMap;
typedef const Eigen::Map<const Eigen::Vector4f, Eigen::Aligned> Vector4fMapConst;


struct _PointXYZ
{
  PCL_ADD_POINT4D;  // This adds the members x,y,z which can also be accessed using the point (which is float[4])
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
};

/*struct PointXYZ
{
  ADD_4D_POINT_WITH_XYZ;  // This adds the members x,y,z which can also be accessed using the point (which is float[4])
  //inline PointXYZ() {}
  //inline PointXYZ(float x, float y, float z) : x(x), y(y), z(z) {}
};*/
/** \brief A point structure representing Euclidean xyz coordinates. (SSE friendly) 
  * \ingroup common
  */
struct PointXYZ : public _PointXYZ
{
  inline PointXYZ()
  {
    x = y = z = 0.0f;
    data[3] = 1.0f;
  }
  inline PointXYZ (float _x, float _y, float _z) { x = _x; y = _y; z = _z; data[3] = 1.0f; }
};

inline std::ostream& operator << (std::ostream& os, const PointXYZ& p)
{
  os << "(" << p.x << "," << p.y << "," << p.z << ")";
  return (os);
}

/** \brief A point structure representing Euclidean xyz coordinates, and the intensity value.
  * \ingroup common
  */ 
struct PointXYZI 
{ 
  PCL_ADD_POINT4D;  // This adds the members x,y,z which can also be accessed using the point (which is float[4])
  union
  {
    struct
    {
      float intensity; 
    };
    float data_c[4];
  };
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW 
} EIGEN_ALIGN16; 
inline std::ostream& operator << (std::ostream& os, const PointXYZI& p)
{
  os << "(" << p.x << "," << p.y << "," << p.z << " - " << p.intensity << ")";
  return (os);
}


/** \brief A point structure representing Euclidean xyz coordinates, and the RGBA color. 
  *
  * The RGBA information is available either as separate r, g, b, or as a
  * packed uint32_t rgba value. To pack it, use:
  *
  * \code
  * int rgb = ((int)r) << 16 | ((int)g) << 8 | ((int)b);
  * \endcode
  *
  * To unpack it use:
  *
  * \code
  * int rgb = ...;
  * uint8_t r = (rgb >> 16) & 0x0000ff;
  * uint8_t g = (rgb >> 8)  & 0x0000ff;
  * uint8_t b = (rgb)     & 0x0000ff;
  * \endcode
  *
  * \ingroup common
  */
struct PointXYZRGBA
{
  PCL_ADD_POINT4D;  // This adds the members x,y,z which can also be accessed using the point (which is float[4])
  union
  {
    struct 
    {
      uint8_t b;
      uint8_t g;
      uint8_t r;
      uint8_t _unused;
    };
    uint32_t rgba;
  };
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
} EIGEN_ALIGN16;
inline std::ostream& operator << (std::ostream& os, const PointXYZRGBA& p)
{
  unsigned char* rgba_ptr = (unsigned char*)&p.rgba;
  os << "(" << p.x << "," << p.y << "," << p.z << " - " << (int)(*rgba_ptr) << "," << (int)(*(rgba_ptr+1)) << "," << (int)(*(rgba_ptr+2)) << "," <<(int)(*(rgba_ptr+3)) << ")";
  return (os);
}


struct _PointXYZRGB 
{
  PCL_ADD_POINT4D;  // This adds the members x,y,z which can also be accessed using the point (which is float[4])
  union
  {
    struct 
    {
      uint8_t b;
      uint8_t g;
      uint8_t r;
      uint8_t _unused;
    };
    float rgb;
    uint32_t rgba;
  };
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
} EIGEN_ALIGN16;

/** \brief A point structure representing Euclidean xyz coordinates, and the RGB color. 
  * 
  * Due to historical reasons (PCL was first developed as a ROS package), the
  * RGB information is packed into an integer and casted to a float. This is
  * something we wish to remove in the near future, but in the meantime, the
  * following code snippet should help you pack and unpack RGB colors in your
  * PointXYZRGB structure:
  *
  * \code
  * // pack r/g/b into rgb
  * uint8_t r = 255, g = 0, b = 0;    // Example: Red color
  * uint32_t rgb = ((uint32_t)r << 16 | (uint32_t)g << 8 | (uint32_t)b);
  * p.rgb = *reinterpret_cast<float*>(&rgb);
  * \endcode
  *
  * To unpack the data into separate values, use:
  *
  * \code
  * PointXYZRGB p;
  * // unpack rgb into r/g/b
  * uint32_t rgb = *reinterpret_cast<int*>(&p.rgb);
  * uint8_t r = (rgb >> 16) & 0x0000ff;
  * uint8_t g = (rgb >> 8)  & 0x0000ff;
  * uint8_t b = (rgb)       & 0x0000ff;
  * \endcode
  *
  *
  * Alternatively, from 1.1.0 onwards, you can use p.r, p.g, and p.b directly.
  *
  * \ingroup common
  */
struct PointXYZRGB : public _PointXYZRGB
{
  inline PointXYZRGB ()
  {
    _unused = 0;
  }
  inline PointXYZRGB (uint8_t _r, uint8_t _g, uint8_t _b)
  {
    r = _r;
    g = _g;
    b = _b;
    _unused = 0;
  }
};
inline std::ostream& operator << (std::ostream& os, const PointXYZRGB& p)
{
  os << "(" << p.x << "," << p.y << "," << p.z << " - " << p.rgb << ")";
  return (os);
}


/** \brief A 2D point structure representing Euclidean xy coordinates. 
  * \ingroup common
  */
struct PointXY
{
  float x;
  float y;
};
inline std::ostream& operator << (std::ostream& os, const PointXY& p)
{
  os << "(" << p.x << "," << p.y << ")";
  return (os);
}


/** \brief A point structure representing an interest point with Euclidean xyz coordinates, and an interest value. 
  * \ingroup common
  */
struct InterestPoint
{
  PCL_ADD_POINT4D;  // This adds the members x,y,z which can also be accessed using the point (which is float[4])
  union
  {
    struct
    {
      float strength;
    };
    float data_c[4];
  };
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
} EIGEN_ALIGN16;
inline std::ostream& operator << (std::ostream& os, const InterestPoint& p)
{
  os << "(" << p.x << "," << p.y << "," << p.z << " - " << p.strength << ")";
  return (os);
}


/** \brief A point structure representing normal coordinates and the surface curvature estimate. (SSE friendly) 
  * \ingroup common
  */
struct Normal
{
  PCL_ADD_NORMAL4D;  // This adds the member normal[3] which can also be accessed using the point (which is float[4])
  union
  {
    struct
    {
      float curvature;
    };
    float data_c[4];
  };
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
} EIGEN_ALIGN16;
inline std::ostream& operator << (std::ostream& os, const Normal& p)
{
  os << "(" << p.normal[0] << "," << p.normal[1] << "," << p.normal[2] << " - " << p.curvature << ")";
  return (os);
}


/** \brief A point structure representing Euclidean xyz coordinates, together with normal coordinates and the surface curvature estimate. (SSE friendly) 
  * \ingroup common
  */
struct PointNormal
{
  PCL_ADD_POINT4D;    // This adds the members x,y,z which can also be accessed using the point (which is float[4])
  PCL_ADD_NORMAL4D;   // This adds the member normal[3] which can also be accessed using the point (which is float[4])
  union
  {
    struct
    {
      float curvature;
    };
    float data_c[4];
  };
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
} EIGEN_ALIGN16;
inline std::ostream& operator << (std::ostream& os, const PointNormal& p)
{
  os << "(" << p.x << "," << p.y << "," << p.z << " - " << p.normal[0] << "," << p.normal[1] << "," << p.normal[2] << " - " << p.curvature << ")";
  return (os);
}


/** \brief A point structure representing Euclidean xyz coordinates, and the RGB color, together with normal coordinates and the surface curvature estimate. 
  * Due to historical reasons (PCL was first developed as a ROS package), the
  * RGB information is packed into an integer and casted to a float. This is
  * something we wish to remove in the near future, but in the meantime, the
  * following code snippet should help you pack and unpack RGB colors in your
  * PointXYZRGB structure:
  *
  * \code
  * // pack r/g/b into rgb
  * uint8_t r = 255, g = 0, b = 0;    // Example: Red color
  * uint32_t rgb = ((uint32_t)r << 16 | (uint32_t)g << 8 | (uint32_t)b);
  * p.rgb = *reinterpret_cast<float*>(&rgb);
  * \endcode
  *
  * To unpack the data into separate values, use:
  *
  * \code
  * PointXYZRGB p;
  * // unpack rgb into r/g/b
  * uint32_t rgb = *reinterpret_cast<int*>(&p.rgb);
  * uint8_t r = (rgb >> 16) & 0x0000ff;
  * uint8_t g = (rgb >> 8)  & 0x0000ff;
  * uint8_t b = (rgb)       & 0x0000ff;
  * \endcode
  *
  *
  * Alternatively, from 1.1.0 onwards, you can use p.r, p.g, and p.b directly.
  * \ingroup common
  */
struct _PointXYZRGBNormal
{
  PCL_ADD_POINT4D;    // This adds the members x,y,z which can also be accessed using the point (which is float[4])
  PCL_ADD_NORMAL4D;   // This adds the member normal[3] which can also be accessed using the point (which is float[4])
  union
  {
    struct
    {
      // RGB union
      union
      {
        struct 
        {
          uint8_t b;
          uint8_t g;
          uint8_t r;
          uint8_t _unused;
        };
        float rgb;
        uint32_t rgba;
      };
      float curvature;
    };
    float data_c[4];
  };
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
} EIGEN_ALIGN16;
struct PointXYZRGBNormal : public _PointXYZRGBNormal
{
  inline PointXYZRGBNormal ()
  {
    _unused = 0;
    data[3] = 1.0f;
    data_n[3] = 0.0f;
  }
};
inline std::ostream& operator << (std::ostream& os, const PointXYZRGBNormal& p)
{
  os << "(" << p.x << "," << p.y << "," << p.z << " - " << p.rgb << " - " << p.normal[0] << "," << p.normal[1] << "," << p.normal[2] << " - " << p.r << ", " << p.g << ", " << p.b << " - " << p.curvature << ")";
  return (os);
}

/** \brief A point structure representing Euclidean xyz coordinates, intensity, together with normal coordinates and the surface curvature estimate. 
  * \ingroup common
  */
struct PointXYZINormal
{
  PCL_ADD_POINT4D;    // This adds the members x,y,z which can also be accessed using the point (which is float[4])
  PCL_ADD_NORMAL4D;   // This adds the member normal[3] which can also be accessed using the point (which is float[4])
  union
  {
    struct
    {
      float intensity;
      float curvature;
    };
    float data_c[4];
  };
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
} EIGEN_ALIGN16;
inline std::ostream& operator << (std::ostream& os, const PointXYZINormal& p)
{
  os << "(" << p.x << "," << p.y << "," << p.z << " - " << p.intensity << " - " << p.normal[0] << "," << p.normal[1] << "," << p.normal[2] << " - " << p.curvature << ")";
  return (os);
}

/** \brief A point structure representing Euclidean xyz coordinates, padded with an extra range float. 
  * \ingroup common
  */
struct PointWithRange 
{
  PCL_ADD_POINT4D;    // This adds the members x,y,z which can also be accessed using the point (which is float[4])
  union
  {
    struct
    {
      float range;
    };
    float data_c[4];
  };
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
} EIGEN_ALIGN16;
inline std::ostream& operator << (std::ostream& os, const PointWithRange& p)
{
  os << "(" << p.x << "," << p.y << "," << p.z << " - " << p.range << ")";
  return (os);
}

struct _PointWithViewpoint 
{
  PCL_ADD_POINT4D;    // This adds the members x,y,z which can also be accessed using the point (which is float[4])
  union
  {
    struct
    {
      float vp_x;
      float vp_y;
      float vp_z;
    };
    float data_c[4];
  };
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
} EIGEN_ALIGN16;

/** \brief A point structure representing Euclidean xyz coordinates together with the viewpoint from which it was seen. 
  * \ingroup common
  */
struct PointWithViewpoint : public _PointWithViewpoint
{
  PointWithViewpoint(float _x=0.0f, float _y=0.0f, float _z=0.0f, float _vp_x=0.0f, float _vp_y=0.0f, float _vp_z=0.0f)
  {
    x=_x; y=_y; z=_z; data[3] = 1.0f;
    vp_x=_vp_x; vp_y=_vp_y; vp_z=_vp_z;
  }
};
inline std::ostream& operator << (std::ostream& os, const PointWithViewpoint& p)
{
  os << "(" << p.x << "," << p.y << "," << p.z << " - " << p.vp_x << "," << p.vp_y << "," << p.vp_z << ")";
  return (os);
}

/** \brief A point structure representing the three moment invariants. 
  * \ingroup common
  */
struct MomentInvariants
{
  float j1, j2, j3;
};
inline std::ostream& operator << (std::ostream& os, const MomentInvariants& p)
{
  os << "(" << p.j1 << "," << p.j2 << "," << p.j3 << ")";
  return (os);
}

/** \brief A point structure representing the minimum and maximum surface radii (in meters) computed using RSD. 
  * \ingroup common
  */
struct PrincipalRadiiRSD
{
  float r_min, r_max;
};
inline std::ostream& operator << (std::ostream& os, const PrincipalRadiiRSD& p)
{
  os << "(" << p.r_min << "," << p.r_max << ")";
  return (os);
}

/** \brief A point structure representing a description of whether a point is lying on a surface boundary or not. 
  * \ingroup common
  */
struct Boundary
{
  uint8_t boundary_point;
};
inline std::ostream& operator << (std::ostream& os, const Boundary& p)
{
  os << p.boundary_point;
  return (os);
}

/** \brief A point structure representing the principal curvatures and their magnitudes. 
  * \ingroup common
  */
struct PrincipalCurvatures
{
  union
  {
    float principal_curvature[3];
    struct
    {
      float principal_curvature_x;
      float principal_curvature_y;
      float principal_curvature_z;
    };
  };
  float pc1;
  float pc2;
};
inline std::ostream& operator << (std::ostream& os, const PrincipalCurvatures& p)
{
  os << "(" << p.principal_curvature[0] << "," << p.principal_curvature[1] << "," << p.principal_curvature[2] << " - " << p.pc1 << "," << p.pc2 << ")";
  return (os);
}

/** \brief A point structure representing the Point Feature Histogram (PFH). 
  * \ingroup common
  */
struct PFHSignature125
{
  float histogram[125];
};
inline std::ostream& operator << (std::ostream& os, const PFHSignature125& p)
{
  for (int i = 0; i < 125; ++i) 
    os << (i == 0 ? "(" : "") << p.histogram[i] << (i < 124 ? ", " : ")");
  return (os);
}

/** \brief A point structure representing the Fast Point Feature Histogram (FPFH). 
  * \ingroup common
  */
struct FPFHSignature33
{
  float histogram[33];
};
inline std::ostream& operator << (std::ostream& os, const FPFHSignature33& p)
{
  for (int i = 0; i < 33; ++i) 
    os << (i == 0 ? "(" : "") << p.histogram[i] << (i < 32 ? ", " : ")");
  return (os);
}

/** \brief A point structure representing the Viewpoint Feature Histogram (VFH). 
  * \ingroup common
  */
struct VFHSignature308
{
  float histogram[308];
};
inline std::ostream& operator << (std::ostream& os, const VFHSignature308& p)
{
  for (int i = 0; i < 308; ++i) 
    os << (i == 0 ? "(" : "") << p.histogram[i] << (i < 307 ? ", " : ")");
  return (os);
}

/** \brief A point structure representing the Narf descriptor. 
  * \ingroup common
  */
struct Narf36
{
  float x, y, z, roll, pitch, yaw;
  float descriptor[36];
};
inline std::ostream& operator << (std::ostream& os, const Narf36& p)
{
  os << p.x<<","<<p.y<<","<<p.z<<" - "<<p.roll*360.0/M_PI<<"deg,"<<p.pitch*360.0/M_PI<<"deg,"<<p.yaw*360.0/M_PI<<"deg - ";
  for (int i = 0; i < 36; ++i) 
    os << (i == 0 ? "(" : "") << p.descriptor[i] << (i < 35 ? ", " : ")");
  return (os);
}

/** \brief A structure to store if a point in a range image lies on a border between an obstacle and the background. 
  * \ingroup common
  */
struct BorderDescription 
{
  int x, y;
  BorderTraits traits;
  //std::vector<const BorderDescription*> neighbors;
};

inline std::ostream& operator << (std::ostream& os, const BorderDescription& p)
{
  os << "(" << p.x << "," << p.y << ")";
  return (os);
}

/** \brief A point structure representing the intensity gradient of an XYZI point cloud. 
  * \ingroup common
  */
struct IntensityGradient
{
  union
  {
    float gradient[3];
    struct
    {
      float gradient_x;
      float gradient_y;
      float gradient_z;
    };
  };
};
inline std::ostream& operator << (std::ostream& os, const IntensityGradient& p)
{
  os << "(" << p.gradient[0] << "," << p.gradient[1] << "," << p.gradient[2] << ")";
  return (os);
}

/** \brief A point structure representing an N-D histogram. 
  * \ingroup common
  */
template <int N>
struct Histogram
{
  float histogram[N];
};
template <int N>
inline std::ostream& operator << (std::ostream& os, const Histogram<N>& p)
{
  for (int i = 0; i < N; ++i) 
    os << (i == 0 ? "(" : "") << p.histogram[i] << (i < N-1 ? ", " : ")");
  return (os);
}

/** \brief A point structure representing a 3-D position and scale. 
  * \ingroup common
  */
struct PointWithScale
{
  PCL_ADD_POINT4D;    // This adds the members x,y,z which can also be accessed using the point (which is float[4])
  float scale;
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
} EIGEN_ALIGN16;
inline std::ostream& operator << (std::ostream& os, const PointWithScale& p)
{
  os << "(" << p.x << "," << p.y << "," << p.z << " - " << p.scale << ")";
  return (os);
}

/** \brief A surfel, that is, a point structure representing Euclidean xyz coordinates, together with normal coordinates, a RGBA color, a radius, a confidence value and the surface curvature estimate. 
  * \ingroup common
  */
struct PointSurfel
{
  PCL_ADD_POINT4D;    // This adds the members x,y,z which can also be accessed using the point (which is float[4])
  PCL_ADD_NORMAL4D;   // This adds the member normal[3] which can also be accessed using the point (which is float[4])
  union
  {
    struct
    {
      uint32_t rgba;
      float radius;
      float confidence;
      float curvature;
    };
    float data_c[4];
  };
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
} EIGEN_ALIGN16;
inline std::ostream& operator << (std::ostream& os, const PointSurfel& p)
{
  unsigned char* rgba_ptr = (unsigned char*)&p.rgba;
  os << 
    "(" << p.x << "," << p.y << "," << p.z << " - " <<
    p.normal_x << "," << p.normal_y << "," << p.normal_z << " - " <<
    (int)(*rgba_ptr) << "," << (int)(*(rgba_ptr+1)) << "," << (int)(*(rgba_ptr+2)) << "," <<(int)(*(rgba_ptr+3)) << " - " <<
    p.radius << " - " << p.confidence << " - " << p.curvature << ")";
  return (os);
}


template <typename PointType1, typename PointType2> inline float 
squaredEuclideanDistance (const PointType1& p1, const PointType2& p2)
{
  float diff_x = p2.x - p1.x, diff_y = p2.y - p1.y, diff_z = p2.z - p1.z;
  return (diff_x*diff_x + diff_y*diff_y + diff_z*diff_z);
}

template <typename PointType1, typename PointType2> inline float 
euclideanDistance (const PointType1& p1, const PointType2& p2)
{
  return (sqrtf (squaredEuclideanDistance (p1, p2)));
}

template <typename PointType> inline bool 
hasValidXYZ (const PointType& p)
{
  return (pcl_isfinite (p.x) && pcl_isfinite (p.y) && pcl_isfinite (p.z));
}

}  // End namespace

#endif
