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

#ifndef PCL_ICP_H_
#define PCL_ICP_H_

// PCL includes
#include "pcl/registration/registration.h"
#include "pcl/sample_consensus/ransac.h"
#include "pcl/sample_consensus/sac_model_registration.h"

namespace pcl
{
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /** \brief @b IterativeClosestPoint is an implementation of the Iterative Closest Point algorithm based on Singular
    * Value Decomposition (SVD).
    * \author Radu Bogdan Rusu, Michael Dixon
    * \ingroup registration
    */
  template <typename PointSource, typename PointTarget>
  class IterativeClosestPoint : public Registration<PointSource, PointTarget>
  {
    public:
      /** \brief Empty constructor. */
      IterativeClosestPoint () 
      {
        reg_name_ = "IterativeClosestPoint";
        void (*rigid_transformation)(const pcl::PointCloud<PointSource> &cloud_src, const std::vector<int> &indices_src,
                                   const pcl::PointCloud<PointTarget> &cloud_tgt, const std::vector<int> &indices_tgt,
                                   Eigen::Matrix4f &transformation_matrix) = &pcl::estimateRigidTransformationSVD;
        rigid_transformation_estimation_ = rigid_transformation;
      };

    protected:
      using Registration<PointSource, PointTarget>::reg_name_;
      using Registration<PointSource, PointTarget>::getClassName;
      using Registration<PointSource, PointTarget>::input_;
      using Registration<PointSource, PointTarget>::indices_;
      using Registration<PointSource, PointTarget>::target_;
      using Registration<PointSource, PointTarget>::nr_iterations_;
      using Registration<PointSource, PointTarget>::max_iterations_;
      using Registration<PointSource, PointTarget>::previous_transformation_;
      using Registration<PointSource, PointTarget>::final_transformation_;
      using Registration<PointSource, PointTarget>::transformation_;
      using Registration<PointSource, PointTarget>::transformation_epsilon_;
      using Registration<PointSource, PointTarget>::converged_;
      using Registration<PointSource, PointTarget>::corr_dist_threshold_;
      using Registration<PointSource, PointTarget>::inlier_threshold_;
      using Registration<PointSource, PointTarget>::min_number_correspondences_;

      typedef typename Registration<PointSource, PointTarget>::PointCloudSource PointCloudSource;
      typedef typename PointCloudSource::Ptr PointCloudSourcePtr;
      typedef typename PointCloudSource::ConstPtr PointCloudSourceConstPtr;

      typedef typename Registration<PointSource, PointTarget>::PointCloudTarget PointCloudTarget;

      typedef PointIndices::Ptr PointIndicesPtr;
      typedef PointIndices::ConstPtr PointIndicesConstPtr;

      /** \brief Rigid transformation computation method.
        * \param output the transformed input point cloud dataset using the rigid transformation found
        */
      virtual void 
      computeTransformation (PointCloudSource &output);

      /** \brief Rigid transformation computation method  with initial guess.
        * \param output the transformed input point cloud dataset using the rigid transformation found
        * \param guess the initial guess of the transformation to compute
        */
      virtual void 
      computeTransformation (PointCloudSource &output, const Eigen::Matrix4f &guess);

      /** \brief function to call to estimate the rigid transform */
      boost::function<void(const pcl::PointCloud<PointSource> &cloud_src, 
                           const std::vector<int> &indices_src,
                           const pcl::PointCloud<PointTarget> &cloud_tgt, 
                           const std::vector<int> &indices_tgt,
                           Eigen::Matrix4f &transformation_matrix)> rigid_transformation_estimation_;
  };
}

#include "pcl/registration/impl/icp.hpp"

#endif  //#ifndef PCL_ICP_H_
