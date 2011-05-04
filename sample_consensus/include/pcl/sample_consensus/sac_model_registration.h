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

#ifndef PCL_SAMPLE_CONSENSUS_MODEL_REGISTRATION_H_
#define PCL_SAMPLE_CONSENSUS_MODEL_REGISTRATION_H_

#include <boost/unordered_map.hpp>

#include "pcl/sample_consensus/sac_model.h"
#include "pcl/sample_consensus/model_types.h"
#include "pcl/common/centroid.h"
#include "pcl/common/eigen.h"

namespace pcl
{
  /** \brief @b SampleConsensusModelRegistration defines a model for Point-To-Point registration outlier rejection.
    * \author Radu Bogdan Rusu
    * \ingroup sample_consensus
    */
  template <typename PointT>
  class SampleConsensusModelRegistration : public SampleConsensusModel<PointT>
  {
    using SampleConsensusModel<PointT>::input_;
    using SampleConsensusModel<PointT>::indices_;

    public:
      typedef typename SampleConsensusModel<PointT>::PointCloud PointCloud;
      typedef typename SampleConsensusModel<PointT>::PointCloudPtr PointCloudPtr;
      typedef typename SampleConsensusModel<PointT>::PointCloudConstPtr PointCloudConstPtr;

      typedef boost::shared_ptr<SampleConsensusModelRegistration> Ptr;

      /** \brief Constructor for base SampleConsensusModelRegistration.
        * \param cloud the input point cloud dataset
        */
      SampleConsensusModelRegistration (const PointCloudConstPtr &cloud) : SampleConsensusModel<PointT> (cloud)
      {
        setInputCloud (cloud);
        computeOriginalIndexMapping();
      }

      /** \brief Constructor for base SampleConsensusModelRegistration.
        * \param cloud the input point cloud dataset
        * \param indices a vector of point indices to be used from \a cloud
        */
      SampleConsensusModelRegistration (const PointCloudConstPtr &cloud, 
                                        const std::vector<int> &indices) : 
        SampleConsensusModel<PointT> (cloud, indices)
      {
        computeOriginalIndexMapping();
        input_ = cloud;
        computeSampleDistanceThreshold (cloud);
      }

      /** \brief Provide a pointer to the input dataset
        * \param cloud the const boost shared pointer to a PointCloud message
        */
      inline virtual void
      setInputCloud (const PointCloudConstPtr &cloud)
      {
        SampleConsensusModel<PointT>::setInputCloud (cloud);
        computeOriginalIndexMapping();
        computeSampleDistanceThreshold (cloud);
      }

      /** \brief Computes an "optimal" sample distance threshold based on the
        * principal directions of the input cloud.
        * \param cloud the const boost shared pointer to a PointCloud message
        */
      inline void 
      computeSampleDistanceThreshold (const PointCloudConstPtr &cloud)
      {
        // Compute the principal directions via PCA
        Eigen::Vector4f xyz_centroid;
        compute3DCentroid (*cloud, xyz_centroid);
        EIGEN_ALIGN16 Eigen::Matrix3f covariance_matrix;
        computeCovarianceMatrixNormalized (*cloud, xyz_centroid, covariance_matrix);
        EIGEN_ALIGN16 Eigen::Vector3f eigen_values;
        EIGEN_ALIGN16 Eigen::Matrix3f eigen_vectors;
        pcl::eigen33 (covariance_matrix, eigen_vectors, eigen_values);

        // Compute the distance threshold for sample selection
        sample_dist_thresh_ = eigen_values.array ().sqrt ().sum () / 3.0;
        sample_dist_thresh_ *= sample_dist_thresh_;
        PCL_DEBUG ("[pcl::SampleConsensusModelRegistration::setInputCloud] Estimated a sample selection distance threshold of: %f\n", sample_dist_thresh_);
      }

      /** \brief Set the input point cloud target.
        * \param target the input point cloud target
        */
      inline void
      setInputTarget (const PointCloudConstPtr &target)
      {
        target_ = target;
        // Cache the size and fill the target indices
        unsigned int target_size = target->size();
        indices_tgt_->resize(target_size);
        for (unsigned int i = 0; i < target_size; ++i)
          indices_tgt_->push_back(i);
        computeOriginalIndexMapping();
      }

      /** \brief Set the input point cloud target.
        * \param target the input point cloud target
        * \param indices_tgt a vector of point indices to be used from \a target
        */
      inline void 
      setInputTarget (const PointCloudConstPtr &target, const std::vector<int> &indices_tgt)
      {
        target_ = target;
        indices_tgt_.reset (new std::vector<int> (indices_tgt));
        computeOriginalIndexMapping();
      }

      /** \brief Compute a 4x4 rigid transformation matrix from the samples given
        * \param samples the indices found as good candidates for creating a valid model
        * \param model_coefficients the resultant model coefficients
        */
      bool 
      computeModelCoefficients (const std::vector<int> &samples, 
                                Eigen::VectorXf &model_coefficients);

      /** \brief Compute all distances from the transformed points to their correspondences
        * \param model_coefficients the 4x4 transformation matrix
        * \param distances the resultant estimated distances
        */
      void 
      getDistancesToModel (const Eigen::VectorXf &model_coefficients, 
                           std::vector<double> &distances);

      /** \brief Select all the points which respect the given model coefficients as inliers.
        * \param model_coefficients the 4x4 transformation matrix
        * \param threshold a maximum admissible distance threshold for determining the inliers from the outliers
        * \param inliers the resultant model inliers
        */
      void 
      selectWithinDistance (const Eigen::VectorXf &model_coefficients, 
                            double threshold, std::vector<int> &inliers);

      /** \brief Recompute the 4x4 transformation using the given inlier set
        * \param inliers the data inliers found as supporting the model
        * \param model_coefficients the initial guess for the optimization
        * \param optimized_coefficients the resultant recomputed transformation
        */
      void 
      optimizeModelCoefficients (const std::vector<int> &inliers, 
                                 const Eigen::VectorXf &model_coefficients, 
                                 Eigen::VectorXf &optimized_coefficients);

      void 
      projectPoints (const std::vector<int> &inliers, 
                     const Eigen::VectorXf &model_coefficients, 
                     PointCloud &projected_points, bool copy_data_fields = true) 
      {};

      bool 
      doSamplesVerifyModel (const std::set<int> &indices, const Eigen::VectorXf &model_coefficients, double threshold)
      {
        PCL_ERROR ("[pcl::SampleConsensusModelRegistration::doSamplesVerifyModel] called!\n");
        return (false);
      }

      /** \brief Return an unique id for this model (SACMODEL_REGISTRATION). */
      inline pcl::SacModel getModelType() const { return (SACMODEL_REGISTRATION); }

    protected:
      /** \brief Check whether a model is valid given the user constraints.
        * \param model_coefficients the set of model coefficients
        */
      inline bool 
      isModelValid (const Eigen::VectorXf &model_coefficients)
      {
        // Needs a valid model coefficients
        if (model_coefficients.size () != 16)
          return (false);

        return (true);
      }

      /** \brief Check if a sample of indices results in a good sample of points
        * indices. Pure virtual.
        * \param samples the resultant index samples
        */
      bool
      isSampleGood(const std::vector<int> &samples) const;

    private:
      /** \brief compute mappings between original indices of the input_/target_ clouds */
      void
      computeOriginalIndexMapping() {
        if ((!indices_tgt_) || (!indices_) || (indices_->empty()) || (indices_->size()!=indices_tgt_->size()))
          return;
        for (unsigned int i = 0; i < indices_->size(); ++i)
          original_index_mapping_[indices_->operator[](i)] = indices_tgt_->operator[](i);
      }

      /** \brief A boost shared pointer to the target point cloud data array. */
      PointCloudConstPtr target_;

      /** \brief A pointer to the vector of target point indices to use. */
      IndicesPtr indices_tgt_;

      /** \brief Given the index in the original point cloud, give the matching original index in the target cloud */
      boost::unordered_map<int, int> original_index_mapping_;

      /** \brief Internal distance threshold used for the sample selection step. */
      double sample_dist_thresh_;
  };
}

#include "pcl/sample_consensus/impl/sac_model_registration.hpp"

#endif  //#ifndef PCL_SAMPLE_CONSENSUS_MODEL_REGISTRATION_H_
