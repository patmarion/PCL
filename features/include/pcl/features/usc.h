/*
 * Software License Agreement (BSD License)
 *
 *  Point Cloud Library (PCL) - www.pointclouds.org
 *  Copyright (c) 2010-2011, Willow Garage, Inc.
 *
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
 *  $Id$
 *
 */

#ifndef PCL_FEATURES_USC_H_
#define PCL_FEATURES_USC_H_

#include <pcl/point_types.h>
#include <pcl/features/feature.h>

namespace pcl
{
  /** \brief UniqueShapeContext implements the Unique Shape Descriptor
    * described here:
    * 
    *   - F. Tombari, S. Salti, L. Di Stefano, 
    *     "Unique Shape Context for 3D data description", 
    *     International Workshop on 3D Object Retrieval (3DOR 10) - 
    *     in conjuction with ACM Multimedia 2010
    *
    * The USC computed feature has the following structure:
    *   - rf float[9] = x_axis | y_axis | normal and represents the local frame
    *     desc std::vector<float> which size is determined by the number of bins
    *     radius_bins_, elevation_bins_ and azimuth_bins_. 
    * 
    * \author Alessandro Franchi, Federico Tombari, Samuele Salti (original code)
    * \author Nizar Sallem (port to PCL)
    * \ingroup features
    */
  template <typename PointInT, typename PointOutT> 
  class UniqueShapeContext : public Feature<PointInT, PointOutT>
  {
    public:
       using Feature<PointInT, PointOutT>::feature_name_;
       using Feature<PointInT, PointOutT>::getClassName;
       using Feature<PointInT, PointOutT>::indices_;
       using Feature<PointInT, PointOutT>::search_parameter_;
       using Feature<PointInT, PointOutT>::search_radius_;
       using Feature<PointInT, PointOutT>::surface_;
       using Feature<PointInT, PointOutT>::input_;
       using Feature<PointInT, PointOutT>::searchForNeighbors;
       
       typedef typename Feature<PointInT, PointOutT>::PointCloudOut PointCloudOut;
       typedef typename Feature<PointInT, PointOutT>::PointCloudIn PointCloudIn;
       
       /** \brief Constructor. */
       UniqueShapeContext () :
         radii_interval_(0), theta_divisions_(0), phi_divisions_(0), volume_lut_(0),
         azimuth_bins_(12), elevation_bins_(11), radius_bins_(15), 
         min_radius_(0.1), point_density_radius_(0.2)
       {
         feature_name_ = "UniqueShapeContext";
         search_radius_ = 2.5;
         local_radius_ = 2.5;
       }

      virtual ~UniqueShapeContext() { }

      /** \brief Set the number of bins along the azimuth
        * \param[in] bins the number of bins along the azimuth
        */
      inline void 
      setAzimuthBins (size_t bins) { azimuth_bins_ = bins; }

      /** \return The number of bins along the azimuth. */
      inline size_t 
      getAzimuthBins (size_t bins) { return (azimuth_bins_); } 

      /** \brief Set the number of bins along the elevation
        * \param[in] bins the number of bins along the elevation
        */
      inline void 
      setElevationBins (size_t bins) { elevation_bins_ = bins; }

      /** \return The number of bins along the elevation */
      inline size_t 
      getElevationBins (size_t bins) { return (elevation_bins_); } 

      /** \brief Set the number of bins along the radii
        * \param[in] bins the number of bins along the radii
        */
      inline void 
      setRadiusBins (size_t bins) { radius_bins_ = bins; }

      /** \return The number of bins along the radii direction. */
      inline size_t 
      getRadiusBins (size_t bins) { return (radius_bins_); } 

      /** The minimal radius value for the search sphere (rmin) in the original paper 
        * \param[in] radius the desired minimal radius
        */
      inline void 
      setMinimalRadius (float radius) { min_radius_ = radius; }

      /** \return The minimal sphere radius. */
      inline float 
      getMinimalRadius () { return (min_radius_); }

      /** This radius is used to compute local point density 
        * density = number of points within this radius
        * \param[in] radius Value of the point density search radius
        */
      inline void 
      setPointDensityRadius (double radius) { point_density_radius_ = radius; }
      
      /** \return The point density search radius. */
      inline double 
      getPointDensityRadius () { return (point_density_radius_); }

      /** Set the local RF radius value
        * \param[in] radius the desired local RF radius
        */
      inline void 
      setLocalRadius (float radius) { local_radius_ = radius; }

      /** \return The local RF radius. */
      inline float 
      getLocalRadius () { return (local_radius_); }
      
    protected:
      /** Compute 3D shape context feature descriptor
        * \param[in] index point index in input_
        * \param[in] rf reference frame
        * \param[out] desc descriptor to compute
        */
      void
      computePointDescriptor (size_t index, float rf[9], std::vector<float> &desc);
      
      /** \brief Initialize computation by allocating all the intervals and the volume lookup table. */
      virtual bool 
      initCompute ();

      /** \brief The actual feature computation.
        * \param[out] output the resultant features
        */
      virtual void
      computeFeature (PointCloudOut &output);

      /** Compute 3D shape context feature local Reference Frame
        * \param[in] index point index in input_
        * \param[out] rf reference frame to compute
        * \return true if the computation of the local Reference Frame was succesful
        */
      bool
      computePointRF (size_t index, float rf[9]);

      /** \brief values of the radii interval. */
      std::vector<float> radii_interval_;

      /** \brief Theta divisions interval. */
      std::vector<float> theta_divisions_;

      /** \brief Phi divisions interval. */
      std::vector<float> phi_divisions_;

      /** \brief Volumes look up table. */
      std::vector<float> volume_lut_;
      
      /** \brief Bins along the azimuth dimension. */
      size_t azimuth_bins_;
      
      /** \brief Bins along the elevation dimension. */
      size_t elevation_bins_;

      /** \brief Bins along the radius dimension. */
      size_t radius_bins_;
      
      /** \brief Minimal radius value. */
      double min_radius_;
      
      /** \brief Point density radius. */
      double point_density_radius_;

      /** \brief Descriptor length. */
      size_t descriptor_length_;

      /** \brief Radius to compute local RF. */
      float local_radius_;
   private:
      /** \brief Make the computeFeature (&Eigen::MatrixXf); inaccessible from outside the class
        * \param[out] output the output point cloud 
        */
      void 
      computeFeature (pcl::PointCloud<Eigen::MatrixXf> &output) {}
  };

  /** \brief UniqueShapeContext implements the Unique Shape Descriptor
    * described here:
    * 
    *   - F. Tombari, S. Salti, L. Di Stefano, 
    *     "Unique Shape Context for 3D data description", 
    *     International Workshop on 3D Object Retrieval (3DOR 10) - 
    *     in conjuction with ACM Multimedia 2010
    *
    * The USC computed feature has the following structure:
    *   - rf float[9] = x_axis | y_axis | normal and represents the local frame 
    *     desc std::vector<float> which size is determined by the number of bins
    *     radius_bins_, elevation_bins_ and azimuth_bins_. 
    * 
    * \author Alessandro Franchi, Federico Tombari, Samuele Salti (original code)
    * \author Nizar Sallem (port to PCL)
    * \ingroup features
    */
  template <typename PointInT> 
  class UniqueShapeContext<PointInT, Eigen::MatrixXf> : public UniqueShapeContext<PointInT, pcl::SHOT>
  {
    public:
       using UniqueShapeContext<PointInT, pcl::SHOT>::indices_;
       using UniqueShapeContext<PointInT, pcl::SHOT>::descriptor_length_;
       using UniqueShapeContext<PointInT, pcl::SHOT>::compute;
       using UniqueShapeContext<PointInT, pcl::SHOT>::computePointRF;
       using UniqueShapeContext<PointInT, pcl::SHOT>::computePointDescriptor;

    private:
      /** \brief The actual feature computation.
        * \param[out] output the resultant features
        */
      virtual void
      computeFeature (pcl::PointCloud<Eigen::MatrixXf> &output);

      /** \brief Make the compute (&PointCloudOut); inaccessible from outside the class
        * \param[out] output the output point cloud 
        */
      void 
      compute (pcl::PointCloud<pcl::SHOT> &output) {}
  };
}

#endif  //#ifndef PCL_USC_H_
