/*
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2011, Alexandru-Eugen Ichim
 *                      Willow Garage, Inc
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
 */

#ifndef STATISTICAL_MULTISCALE_INTEREST_REGION_EXTRACTION_H_
#define STATISTICAL_MULTISCALE_INTEREST_REGION_EXTRACTION_H_

#include <pcl/pcl_base.h>
#include <pcl/point_types.h>


namespace pcl
{
  template <typename PointT>
  class StatisticalMultiscaleInterestRegionExtraction : public PCLBase<PointT>
  {
    public:
      StatisticalMultiscaleInterestRegionExtraction (std::vector<float> &a_scale_values)
        : scale_values (a_scale_values)
      {};


      void
      generateCloudGraph ();

      void
      computeRegionsOfInterest (typename pcl::PointCloud<PointT>::Ptr &output);


    private:
      using PCLBase<PointT>::initCompute;
      using PCLBase<PointT>::input_;
      std::vector<float> scale_values;
      std::vector<std::vector<float> > geodesic_distances;
  };
}


#endif /* STATISTICAL_MULTISCALE_INTEREST_REGION_EXTRACTION_H_ */
