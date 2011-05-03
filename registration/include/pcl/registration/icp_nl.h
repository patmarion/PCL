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

#ifndef PCL_ICP_NL_H_
#define PCL_ICP_NL_H_

// PCL includes
#include "pcl/registration/icp.h"
#include "pcl/sample_consensus/ransac.h"
#include "pcl/sample_consensus/sac_model_registration.h"

#include <cminpack.h>

namespace pcl
{
  /** \brief @b IterativeClosestPointNonLinear is an ICP variant that uses Levenberg-Marquardt optimization backend. The
    * resultant transformation is optimized as a quaternion.
    * \author Radu Bogdan Rusu, Michael Dixon
    * \ingroup registration
    */
  template <typename PointSource, typename PointTarget>
  class IterativeClosestPointNonLinear : public IterativeClosestPoint<PointSource, PointTarget>
  {
    using IterativeClosestPoint<PointSource, PointTarget>::reg_name_;
    using IterativeClosestPoint<PointSource, PointTarget>::getClassName;
    using IterativeClosestPoint<PointSource, PointTarget>::indices_;
    using IterativeClosestPoint<PointSource, PointTarget>::target_;
    using IterativeClosestPoint<PointSource, PointTarget>::nr_iterations_;
    using IterativeClosestPoint<PointSource, PointTarget>::max_iterations_;
    using IterativeClosestPoint<PointSource, PointTarget>::previous_transformation_;
    using IterativeClosestPoint<PointSource, PointTarget>::final_transformation_;
    using IterativeClosestPoint<PointSource, PointTarget>::transformation_;
    using IterativeClosestPoint<PointSource, PointTarget>::transformation_epsilon_;
    using IterativeClosestPoint<PointSource, PointTarget>::converged_;
    using IterativeClosestPoint<PointSource, PointTarget>::corr_dist_threshold_;
    using IterativeClosestPoint<PointSource, PointTarget>::inlier_threshold_;
    using IterativeClosestPoint<PointSource, PointTarget>::min_number_correspondences_;

    using IterativeClosestPoint<PointSource, PointTarget>::rigid_transformation_estimation_;

    typedef pcl::PointCloud<PointSource> PointCloudSource;
    typedef typename PointCloudSource::Ptr PointCloudSourcePtr;
    typedef typename PointCloudSource::ConstPtr PointCloudSourceConstPtr;

    typedef pcl::PointCloud<PointTarget> PointCloudTarget;

    typedef PointIndices::Ptr PointIndicesPtr;
    typedef PointIndices::ConstPtr PointIndicesConstPtr;

    public:
      /** \brief Empty constructor. */
      IterativeClosestPointNonLinear ()
      {
        min_number_correspondences_ = 4;
        reg_name_ = "IterativeClosestPointNonLinear";

        rigid_transformation_estimation_ = 
          boost::bind (&IterativeClosestPointNonLinear<PointSource, PointTarget>::estimateRigidTransformationLM, 
              this, _1, _2, _3, _4, _5);
      }

      /** \brief Estimate a rigid rotation transformation between a source and a target point cloud using an iterative
        * non-linear Levenberg-Marquardt approach.
        * \param cloud_src the source point cloud dataset
        * \param cloud_tgt the target point cloud dataset
        * \param transformation_matrix the resultant transformation matrix
        */
      void 
      estimateRigidTransformationLM (const PointCloudSource &cloud_src, const PointCloudTarget &cloud_tgt, Eigen::Matrix4f &transformation_matrix);

      /** \brief Estimate a rigid rotation transformation between a source and a target point cloud using an iterative
        * non-linear Levenberg-Marquardt approach.
        * \param cloud_src the source point cloud dataset
        * \param indices_src the vector of indices describing the points of interest in \a cloud_src
        * \param cloud_tgt the target point cloud dataset
        * \param indices_tgt the vector of indices describing the correspondences of the interst points from \a indices_src
        * \param transformation_matrix the resultant transformation matrix
        */
      void 
      estimateRigidTransformationLM (const PointCloudSource &cloud_src, const std::vector<int> &indices_src, 
                                     const PointCloudTarget &cloud_tgt, const std::vector<int> &indices_tgt, 
                                     Eigen::Matrix4f &transformation_matrix);

    protected:
      using IterativeClosestPoint<PointSource, PointTarget>::computeTransformation;

    private:
      /** \brief Compute the median value from a set of doubles
        * \param fvec the set of doubles
        * \param m the number of doubles in the set
        */
      inline double 
      computeMedian (double *fvec, int m);

      /** \brief Cost function to be minimized
        * \param p a pointer to our data structure array
        * \param m the number of functions
        * \param n the number of variables
        * \param x a pointer to the variables array
        * \param fvec a pointer to the resultant functions evaluations
        * \param iflag set to -1 inside the function to terminate execution
        */
      static int 
      functionToOptimize (void *p, int m, int n, const double *x, double *fvec, int iflag);
      static int 
      functionToOptimizeIndices (void *p, int m, int n, const double *x, double *fvec, int iflag);

      /** \brief Use a Huber kernel to estimate the distance between two vectors
        * \param p_src the first eigen vector
        * \param p_tgt the second eigen vector
        * \param sigma the sigma value
        */
      inline double
      distHuber (const Eigen::Vector4f &p_src, const Eigen::Vector4f &p_tgt, double sigma)
      {
        Eigen::Array4f diff = (p_tgt.array () - p_src.array ()).abs ();
        double norm = 0.0;
        for (int i = 0; i < 3; ++i)
        {
          if (diff[i] < sigma)
            norm += diff[i] * diff[i];
          else
            norm += 2.0 * sigma * diff[i] - sigma * sigma;
        }
        return (norm);
      }

      /** \brief Use a Huber kernel to estimate the distance between two vectors
        * \param diff the norm difference between two vectors
        * \param sigma the sigma value
        */
      inline double
      distHuber (double diff, double sigma)
      {
        double norm = 0.0;
        if (diff < sigma)
          norm += diff * diff;
        else
          norm += 2.0 * sigma * diff - sigma * sigma;
        return (norm);
      }

      /** \brief Use a Gedikli kernel to estimate the distance between two vectors
        * (for more information, see 
        * \param val the norm difference between two vectors
        * \param clipping the clipping value
        */
      inline double
      distGedikli (double val, double clipping)
      {
        clipping *= 1.5;
        return (1.0 / (1.0 + pow (val / clipping, 4)));
      }

      /** \brief Compute the Manhattan distance between two eigen vectors.
        * \param p_src the first eigen vector
        * \param p_tgt the second eigen vector
        */
      inline double
      distL1 (const Eigen::Vector4f &p_src, const Eigen::Vector4f &p_tgt)
      {
        return ((p_src.array () - p_tgt.array ()).abs ().sum ());
      }

      /** \brief Compute the squared Euclidean distance between two eigen vectors.
        * \param p_src the first eigen vector
        * \param p_tgt the second eigen vector
        */
      inline double
      distL2Sqr (const Eigen::Vector4f &p_src, const Eigen::Vector4f &p_tgt)
      {
        return ((p_src - p_tgt).squaredNorm ());
      }

      /** \brief The vector of residual weights. Used internall in the LM loop. */
      std::vector<double> weights_;

      /** \brief Temporary boost mutex for \a tmp_src_ and \a tmp_tgt_*/
      boost::mutex tmp_mutex_;

      /** \brief Temporary pointer to the source dataset. */
      const PointCloudSource *tmp_src_;

      /** \brief Temporary pointer to the target dataset. */
      const PointCloudTarget  *tmp_tgt_;

      /** \brief Temporary pointer to the source dataset indices. */
      const std::vector<int> *tmp_idx_src_;

      /** \brief Temporary pointer to the target dataset indices. */
      const std::vector<int> *tmp_idx_tgt_;
  };
}

#include "pcl/registration/impl/icp_nl.hpp"

#endif  //#ifndef PCL_ICP_NL_H_
