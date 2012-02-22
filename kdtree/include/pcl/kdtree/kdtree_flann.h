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
 * $Id$
 *
 */

#ifndef PCL_KDTREE_KDTREE_FLANN_H_
#define PCL_KDTREE_KDTREE_FLANN_H_

#include <cstdio>
#include <pcl/point_representation.h>
#include <flann/flann.hpp>
#include <pcl/kdtree/kdtree.h>

namespace pcl
{
  /** \brief KdTreeFLANN is a generic type of 3D spatial locator using kD-tree structures. The class is making use of
    * the FLANN (Fast Library for Approximate Nearest Neighbor) project by Marius Muja and David Lowe.
    *
    * \author Radu B. Rusu, Marius Muja
    * \ingroup kdtree 
    */
  template <typename PointT, typename Dist = flann::L2_Simple<float> >
  class KdTreeFLANN : public pcl::KdTree<PointT>
  {
    public:
      using KdTree<PointT>::input_;
      using KdTree<PointT>::indices_;
      using KdTree<PointT>::epsilon_;
      using KdTree<PointT>::sorted_;
      using KdTree<PointT>::point_representation_;
      using KdTree<PointT>::nearestKSearch;
      using KdTree<PointT>::radiusSearch;

      typedef typename KdTree<PointT>::PointCloud PointCloud;
      typedef typename KdTree<PointT>::PointCloudConstPtr PointCloudConstPtr;

      typedef boost::shared_ptr<std::vector<int> > IndicesPtr;
      typedef boost::shared_ptr<const std::vector<int> > IndicesConstPtr;

      typedef flann::Index<Dist> FLANNIndex;

      // Boost shared pointers
      typedef boost::shared_ptr<KdTreeFLANN<PointT> > Ptr;
      typedef boost::shared_ptr<const KdTreeFLANN<PointT> > ConstPtr;

      /** \brief Default Constructor for KdTreeFLANN.
        * \param[in] sorted set to true if the application that the tree will be used for requires sorted nearest neighbor indices (default). False otherwise. 
        *
        * By setting sorted to false, the \ref radiusSearch operations will be faster.
        */
      KdTreeFLANN (bool sorted = true) : 
        pcl::KdTree<PointT> (sorted), 
        flann_index_ (NULL), cloud_ (NULL), 
        dim_ (0), total_nr_points_ (0),
        param_k_ (flann::SearchParams (-1 , (float) epsilon_)),
        param_radius_ (flann::SearchParams (-1, (float) epsilon_, sorted))
      {
      }

      /** \brief Set the search epsilon precision (error bound) for nearest neighbors searches.
        * \param[in] eps precision (error bound) for nearest neighbors searches
        */
      inline void
      setEpsilon (double eps)
      {
        epsilon_ = eps;
        param_k_ = flann::SearchParams (-1 , (float) epsilon_);
        param_radius_ = flann::SearchParams (-1 , (float) epsilon_, sorted_);
      }

      inline void setSortedResults (bool sorted)
      {
        sorted_ = sorted;
        param_k_ = flann::SearchParams (-1 ,epsilon_);
        param_radius_ = flann::SearchParams (-1 ,epsilon_, sorted_);
      }
      
      inline Ptr makeShared () { return Ptr (new KdTreeFLANN<PointT> (*this)); } 

      /** \brief Destructor for KdTreeFLANN. 
        * Deletes all allocated data arrays and destroys the kd-tree structures. 
        */
      virtual ~KdTreeFLANN ()
      {
        cleanup ();
      }

      /** \brief Provide a pointer to the input dataset.
        * \param[in] cloud the const boost shared pointer to a PointCloud message
        * \param[in] indices the point indices subset that is to be used from \a cloud - if NULL the whole cloud is used
        */
      void 
      setInputCloud (const PointCloudConstPtr &cloud, const IndicesConstPtr &indices = IndicesConstPtr ());

      /** \brief Search for k-nearest neighbors for the given query point.
        * 
        * \attention This method does not do any bounds checking for the input index
        * (i.e., index >= cloud.points.size () || index < 0), and assumes valid (i.e., finite) data.
        * 
        * \param[in] point a given \a valid (i.e., finite) query point
        * \param[in] k the number of neighbors to search for
        * \param[out] k_indices the resultant indices of the neighboring points (must be resized to \a k a priori!)
        * \param[out] k_sqr_distances the resultant squared distances to the neighboring points (must be resized to \a k 
        * a priori!)
        * \return number of neighbors found
        * 
        * \exception asserts in debug mode if the index is not between 0 and the maximum number of points
        */
      int 
      nearestKSearch (const PointT &point, int k, 
                      std::vector<int> &k_indices, std::vector<float> &k_sqr_distances) const;

      /** \brief Search for all the nearest neighbors of the query point in a given radius.
        * 
        * \attention This method does not do any bounds checking for the input index
        * (i.e., index >= cloud.points.size () || index < 0), and assumes valid (i.e., finite) data.
        * 
        * \param[in] point a given \a valid (i.e., finite) query point
        * \param[in] radius the radius of the sphere bounding all of p_q's neighbors
        * \param[out] k_indices the resultant indices of the neighboring points
        * \param[out] k_sqr_distances the resultant squared distances to the neighboring points
        * \param[in] max_nn if given, bounds the maximum returned neighbors to this value. If \a max_nn is set to
        * 0 or to a number higher than the number of points in the input cloud, all neighbors in \a radius will be
        * returned.
        * \return number of neighbors found in radius
        *
        * \exception asserts in debug mode if the index is not between 0 and the maximum number of points
        */
      int 
      radiusSearch (const PointT &point, double radius, std::vector<int> &k_indices,
                    std::vector<float> &k_sqr_distances, unsigned int max_nn = 0) const;

    private:
      /** \brief Internal cleanup method. */
      void 
      cleanup ();

      /** \brief Converts a PointCloud to the internal FLANN point array representation. Returns the number
        * of points.
        * \param cloud the PointCloud 
        */
      void 
      convertCloudToArray (const PointCloud &cloud);

      /** \brief Converts a PointCloud with a given set of indices to the internal FLANN point array
        * representation. Returns the number of points.
        * \param[in] cloud the PointCloud data
        * \param[in] indices the point cloud indices
       */
      void 
      convertCloudToArray (const PointCloud &cloud, const std::vector<int> &indices);

    private:
      /** \brief Class getName method. */
      virtual std::string 
      getName () const { return ("KdTreeFLANN"); }

      /** \brief A FLANN index object. */
      FLANNIndex* flann_index_;

      /** \brief Internal pointer to data. */
      float* cloud_;
      
      /** \brief mapping between internal and external indices. */
      std::vector<int> index_mapping_;
      
      /** \brief whether the mapping bwwteen internal and external indices is identity */
      bool identity_mapping_;

      /** \brief Tree dimensionality (i.e. the number of dimensions per point). */
      int dim_;

      /** \brief The total size of the data (either equal to the number of points in the input cloud or to the number of indices - if passed). */
      int total_nr_points_;

      /** \brief The KdTree search parameters for K-nearest neighbors. */
      flann::SearchParams param_k_;

      /** \brief The KdTree search parameters for radius search. */
      flann::SearchParams param_radius_;
  };

  /** \brief KdTreeFLANN is a generic type of 3D spatial locator using kD-tree structures. The class is making use of
    * the FLANN (Fast Library for Approximate Nearest Neighbor) project by Marius Muja and David Lowe.
    *
    * \author Radu B. Rusu, Marius Muja
    * \ingroup kdtree 
    */
  template <>
  class KdTreeFLANN <Eigen::MatrixXf>
  {
    public:
      typedef pcl::PointCloud<Eigen::MatrixXf> PointCloud;
      typedef PointCloud::ConstPtr PointCloudConstPtr;

      typedef boost::shared_ptr<std::vector<int> > IndicesPtr;
      typedef boost::shared_ptr<const std::vector<int> > IndicesConstPtr;

      typedef flann::Index<flann::L2_Simple<float> > FLANNIndex;

      typedef pcl::PointRepresentation<Eigen::MatrixXf> PointRepresentation;
      typedef boost::shared_ptr<const PointRepresentation> PointRepresentationConstPtr;

      // Boost shared pointers
      typedef boost::shared_ptr<KdTreeFLANN<Eigen::MatrixXf> > Ptr;
      typedef boost::shared_ptr<const KdTreeFLANN<Eigen::MatrixXf> > ConstPtr;

      /** \brief Default Constructor for KdTreeFLANN.
        * \param[in] sorted set to true if the application that the tree will be used for requires sorted nearest neighbor indices (default). False otherwise. 
        *
        * By setting sorted to false, the \ref radiusSearch operations will be faster.
        */
      KdTreeFLANN (bool sorted = true) : 
        input_(), indices_(), epsilon_(0.0), sorted_(sorted), flann_index_(NULL), cloud_(NULL)
      {
        param_k_ = flann::SearchParams (-1, (float) epsilon_);
        param_radius_ = flann::SearchParams (-1, (float) epsilon_, sorted);
        cleanup ();
      }

      /** \brief Set the search epsilon precision (error bound) for nearest neighbors searches.
        * \param[in] eps precision (error bound) for nearest neighbors searches
        */
      inline void
      setEpsilon (double eps)
      {
        epsilon_ = eps;
        param_k_ = flann::SearchParams (-1 , (float) epsilon_);
        param_radius_ = flann::SearchParams (-1, (float) epsilon_, sorted_);
      }

      inline Ptr 
      makeShared () { return Ptr (new KdTreeFLANN<Eigen::MatrixXf> (*this)); } 

      /** \brief Destructor for KdTreeFLANN. 
        * Deletes all allocated data arrays and destroys the kd-tree structures. 
        */
      virtual ~KdTreeFLANN ()
      {
        cleanup ();
      }

      /** \brief Provide a pointer to the input dataset.
        * \param[in] cloud the const boost shared pointer to a PointCloud message
        * \param[in] indices the point indices subset that is to be used from \a cloud - if NULL the whole cloud is used
        */
      void 
      setInputCloud (const PointCloudConstPtr &cloud, const IndicesConstPtr &indices = IndicesConstPtr ())
      {
        cleanup ();   // Perform an automatic cleanup of structures

        if (cloud == NULL)
          return;

        epsilon_ = 0.0;   // default error bound value
        input_   = cloud;
        indices_ = indices;
        dim_ = (int) cloud->points.cols (); // Number of dimensions = number of columns in the eigen matrix
        
        // Allocate enough data
        if (indices != NULL)
        {
          total_nr_points_ = (int) indices_->size ();
          convertCloudToArray (*input_, *indices_);
        }
        else
        {
          // get the number of points as the number of rows
          total_nr_points_ = (int) cloud->points.rows ();
          convertCloudToArray (*input_);
        }

        flann_index_ = new FLANNIndex (flann::Matrix<float> (cloud_, index_mapping_.size (), dim_),
                                       flann::KDTreeSingleIndexParams (15)); // max 15 points/leaf
        flann_index_->buildIndex ();
      }

      /** \brief Get a pointer to the vector of indices used. */
      inline IndicesConstPtr const
      getIndices ()
      {
        return (indices_);
      }

      /** \brief Get a pointer to the input point cloud dataset. */
      inline PointCloudConstPtr
      getInputCloud ()
      {
        return (input_);
      }

      /** \brief Search for k-nearest neighbors for the given query point.
        * \param[in] point the given query point
        * \param[in] k the number of neighbors to search for
        * \param[out] k_indices the resultant indices of the neighboring points (must be resized to \a k a priori!)
        * \param[out] k_sqr_distances the resultant squared distances to the neighboring points (must be resized to \a k 
        * a priori!)
        * \return number of neighbors found
        */
      template <typename T> int 
      nearestKSearch (const T &point, int k, 
                      std::vector<int> &k_indices, std::vector<float> &k_sqr_distances) const
      {
        assert (isRowValid (point) && "Invalid (NaN, Inf) point coordinates given to nearestKSearch!");

        if (k > total_nr_points_)
        {
          PCL_ERROR ("[pcl::KdTreeFLANN::nearestKSearch] An invalid number of nearest neighbors was requested! (k = %d out of %d total points).\n", k, total_nr_points_);
          k = total_nr_points_;
        }

        k_indices.resize (k);
        k_sqr_distances.resize (k);

        size_t dim = point.size ();
        std::vector<float> query (dim);
        for (size_t i = 0; i < dim; ++i)
          query[i] = point[i];

        flann::Matrix<int> k_indices_mat (&k_indices[0], 1, k);
        flann::Matrix<float> k_distances_mat (&k_sqr_distances[0], 1, k);
        // Wrap the k_indices and k_distances vectors (no data copy)
        flann_index_->knnSearch (flann::Matrix<float> (&query[0], 1, dim), 
                                 k_indices_mat, k_distances_mat,
                                 k, param_k_);

        // Do mapping to original point cloud
        if (!identity_mapping_) 
        {
          for (size_t i = 0; i < (size_t)k; ++i)
          {
            int& neighbor_index = k_indices[i];
            neighbor_index = index_mapping_[neighbor_index];
          }
        }

        return (k);
      }

      /** \brief Search for k-nearest neighbors for the given query point.
        * \param[in] cloud the point cloud data
        * \param[in] index the index in \a cloud representing the query point
        * \param[in] k the number of neighbors to search for
        * \param[out] k_indices the resultant indices of the neighboring points (must be resized to \a k a priori!)
        * \param[out] k_sqr_distances the resultant squared distances to the neighboring points (must be resized to \a k 
        * a priori!)
        * \return number of neighbors found
        */
      inline int 
      nearestKSearch (const PointCloud &cloud, int index, int k, 
                      std::vector<int> &k_indices, std::vector<float> &k_sqr_distances) const
      {
        assert (index >= 0 && index < (int)cloud.points.size () && "Out-of-bounds error in nearestKSearch!");
        return (nearestKSearch (cloud.points.row (index), k, k_indices, k_sqr_distances));
      }

      /** \brief Search for k-nearest neighbors for the given query point (zero-copy).
        * \param[in] index the index representing the query point in the dataset given by \a setInputCloud
        *        if indices were given in setInputCloud, index will be the position in the indices vector
        * \param[in] k the number of neighbors to search for
        * \param[out] k_indices the resultant indices of the neighboring points (must be resized to \a k a priori!)
        * \param[out] k_sqr_distances the resultant squared distances to the neighboring points (must be resized to \a k 
        * a priori!)
        * \return number of neighbors found
        */
      inline int 
      nearestKSearch (int index, int k, 
                      std::vector<int> &k_indices, std::vector<float> &k_sqr_distances) const
      {
        if (indices_ == NULL)
        {
          assert (index >= 0 && index < (int)input_->points.size () && "Out-of-bounds error in nearestKSearch!");
          return (nearestKSearch (input_->points.row (index), k, k_indices, k_sqr_distances));
        }
        else
        {
          assert (index >= 0 && index < (int)indices_->size () && "Out-of-bounds error in nearestKSearch!");
          return (nearestKSearch (input_->points.row ((*indices_)[index]), k, k_indices, k_sqr_distances));
        }
      }

      /** \brief Search for all the nearest neighbors of the query point in a given radius.
        * \param[in] point the given query point
        * \param[in] radius the radius of the sphere bounding all of p_q's neighbors
        * \param[out] k_indices the resultant indices of the neighboring points
        * \param[out] k_sqr_dists the resultant squared distances to the neighboring points
        * \param[in] max_nn if given, bounds the maximum returned neighbors to this value. If \a max_nn is set to
        * 0 or to a number higher than the number of points in the input cloud, all neighbors in \a radius will be
        * returned.
        * \return number of neighbors found in radius
        */
      template <typename T> int 
      radiusSearch (const T &point, double radius, std::vector<int> &k_indices,
                    std::vector<float> &k_sqr_dists, unsigned int max_nn = 0) const
      {
        assert (isRowValid (point) && "Invalid (NaN, Inf) point coordinates given to radiusSearch!");

        size_t dim = point.size ();
        std::vector<float> query (dim);
        for (size_t i = 0; i < dim; ++i)
          query[i] = point[i];

        int neighbors_in_radius = 0;

        // If the user pre-allocated the arrays, we are only going to 
        if (k_indices.size () == (size_t)total_nr_points_ && k_sqr_dists.size () == (size_t)total_nr_points_)
        {
          flann::Matrix<int> k_indices_mat (&k_indices[0], 1, k_indices.size ());
          flann::Matrix<float> k_distances_mat (&k_sqr_dists[0], 1, k_sqr_dists.size ());
          neighbors_in_radius = flann_index_->radiusSearch (flann::Matrix<float> (&query[0], 1, dim),
                                                            k_indices_mat,
                                                            k_distances_mat,
                                                            (float) (radius * radius), 
                                                            param_radius_);
        }
        else
        {
          // Has max_nn been set properly?
          if (max_nn == 0 || max_nn > (unsigned int)total_nr_points_)
            max_nn = total_nr_points_;

          static flann::Matrix<int> indices_empty;
          static flann::Matrix<float> dists_empty;
          neighbors_in_radius = flann_index_->radiusSearch (flann::Matrix<float> (&query[0], 1, dim),
                                                            indices_empty,
                                                            dists_empty,
                                                            (float) (radius * radius), 
                                                            param_radius_);
          neighbors_in_radius = std::min ((unsigned int)neighbors_in_radius, max_nn);

          k_indices.resize (neighbors_in_radius);
          k_sqr_dists.resize (neighbors_in_radius);
          if(neighbors_in_radius != 0)
          {
            flann::Matrix<int> k_indices_mat (&k_indices[0], 1, k_indices.size ());
            flann::Matrix<float> k_distances_mat (&k_sqr_dists[0], 1, k_sqr_dists.size ());
            flann_index_->radiusSearch (flann::Matrix<float> (&query[0], 1, dim),
                                        k_indices_mat,
                                        k_distances_mat,
                                        (float) (radius * radius), 
                                        param_radius_);
          }
        }

        // Do mapping to original point cloud
        if (!identity_mapping_) 
        {
          for (int i = 0; i < neighbors_in_radius; ++i)
          {
            int& neighbor_index = k_indices[i];
            neighbor_index = index_mapping_[neighbor_index];
          }
        }

        return (neighbors_in_radius);
      }

      /** \brief Search for all the nearest neighbors of the query point in a given radius.
        * \param[in] cloud the point cloud data
        * \param[in] index the index in \a cloud representing the query point
        * \param[in] radius the radius of the sphere bounding all of p_q's neighbors
        * \param[out] k_indices the resultant indices of the neighboring points
        * \param[out] k_sqr_distances the resultant squared distances to the neighboring points
        * \param[in] max_nn if given, bounds the maximum returned neighbors to this value. If \a max_nn is set to
        * 0 or to a number higher than the number of points in the input cloud, all neighbors in \a radius will be
        * returned.
        * \return number of neighbors found in radius
        */
      inline int 
      radiusSearch (const PointCloud &cloud, int index, double radius, 
                    std::vector<int> &k_indices, std::vector<float> &k_sqr_distances, 
                    unsigned int max_nn = 0) const
      {
        assert (index >= 0 && index < (int)cloud.points.size () && "Out-of-bounds error in radiusSearch!");
        return (radiusSearch (cloud.points.row (index), radius, k_indices, k_sqr_distances, max_nn));
      }

      /** \brief Search for all the nearest neighbors of the query point in a given radius (zero-copy).
        * \param[in] index the index representing the query point in the dataset given by \a setInputCloud
        *        if indices were given in setInputCloud, index will be the position in the indices vector
        * \param[in] radius the radius of the sphere bounding all of p_q's neighbors
        * \param[out] k_indices the resultant indices of the neighboring points
        * \param[out] k_sqr_distances the resultant squared distances to the neighboring points
        * \param[in] max_nn if given, bounds the maximum returned neighbors to this value. If \a max_nn is set to
        * 0 or to a number higher than the number of points in the input cloud, all neighbors in \a radius will be
        * returned.
        * \return number of neighbors found in radius
        */
      inline int 
      radiusSearch (int index, double radius, std::vector<int> &k_indices,
                    std::vector<float> &k_sqr_distances, unsigned int max_nn = 0) const
      {
        if (indices_ == NULL)
        {
          assert (index >= 0 && index < (int)input_->points.size () && "Out-of-bounds error in radiusSearch!");
          return (radiusSearch (input_->points.row (index), radius, k_indices, k_sqr_distances, max_nn));
        }
        else
        {
          assert (index >= 0 && index < (int)indices_->size () && "Out-of-bounds error in radiusSearch!");
          return (radiusSearch (input_->points.row ((*indices_)[index]), radius, k_indices, k_sqr_distances, max_nn));
        }
      }

      /** \brief Get the search epsilon precision (error bound) for nearest neighbors searches. */
      inline double
      getEpsilon ()
      {
        return (epsilon_);
      }

    private:
      /** \brief Internal cleanup method. */
      void 
      cleanup ()
      {
        if (flann_index_)
          delete flann_index_;

        // Data array cleanup
        if (cloud_)
        {
          free (cloud_);
          cloud_ = NULL;
        }
        index_mapping_.clear ();

        if (indices_)
          indices_.reset ();
      }

      /** \brief Check if a given expression is valid (i.e., contains only finite values)
        * \param[in] pt the expression to evaluate
        */
      template <typename Expr> bool
      isRowValid (const Expr &pt) const
      {
        for (size_t i = 0; i < (size_t)pt.size (); ++i)
          if (!pcl_isfinite (pt[i]))
           return (false);

        return (true);
      }

      /** \brief Converts a PointCloud to the internal FLANN point array representation. Returns the number
        * of points.
        * \param cloud the PointCloud 
        */
      void 
      convertCloudToArray (const PointCloud &cloud)
      {
        // No point in doing anything if the array is empty
        if (cloud.empty ())
        {
          cloud_ = NULL;
          return;
        }

        int original_no_of_points = cloud.points.rows ();

        cloud_ = (float*)malloc (original_no_of_points * dim_ * sizeof (float));
        float* cloud_ptr = cloud_;
        index_mapping_.reserve (original_no_of_points);
        identity_mapping_ = true;

        for (int cloud_index = 0; cloud_index < original_no_of_points; ++cloud_index)
        {
          // Check if the point is invalid
          if (!isRowValid (cloud.points.row (cloud_index)))
          {
            identity_mapping_ = false;
            continue;
          }

          index_mapping_.push_back (cloud_index);

          for (size_t i = 0; i < (size_t)dim_; ++i)
          {
            *cloud_ptr = cloud.points.coeffRef (cloud_index, i);
            cloud_ptr++;
          }
        }
      }

      /** \brief Converts a PointCloud with a given set of indices to the internal FLANN point array
        * representation. Returns the number of points.
        * \param[in] cloud the PointCloud data
        * \param[in] indices the point cloud indices
       */
      void 
      convertCloudToArray (const PointCloud &cloud, const std::vector<int> &indices)
      {
        // No point in doing anything if the array is empty
        if (cloud.empty ())
        {
          cloud_ = NULL;
          return;
        }

        int original_no_of_points = (int) indices.size ();

        cloud_ = (float*)malloc (original_no_of_points * dim_ * sizeof (float));
        float* cloud_ptr = cloud_;
        index_mapping_.reserve (original_no_of_points);
        identity_mapping_ = true;

        for (int indices_index = 0; indices_index < original_no_of_points; ++indices_index)
        {
          int cloud_index = indices[indices_index];
          // Check if the point is invalid
          if (!isRowValid (cloud.points.row (cloud_index)))
          {
            identity_mapping_ = false;
            continue;
          }

          index_mapping_.push_back (indices_index);  // If the returned index should be for the indices vector

          for (size_t i = 0; i < (size_t)dim_; ++i)
          {
            *cloud_ptr = cloud.points.coeffRef (cloud_index, i);
            cloud_ptr++;
          }
        }
      }

    protected:
      /** \brief The input point cloud dataset containing the points we need to use. */
      PointCloudConstPtr input_;

      /** \brief A pointer to the vector of point indices to use. */
      IndicesConstPtr indices_;

      /** \brief Epsilon precision (error bound) for nearest neighbors searches. */
      double epsilon_;

      /** \brief Return the radius search neighbours sorted **/
      bool sorted_;

    private:
      /** \brief Class getName method. */
      std::string 
      getName () const { return ("KdTreeFLANN"); }

      /** \brief A FLANN index object. */
      FLANNIndex* flann_index_;

      /** \brief Internal pointer to data. */
      float* cloud_;
      
      /** \brief mapping between internal and external indices. */
      std::vector<int> index_mapping_;
      
      /** \brief whether the mapping bwwteen internal and external indices is identity */
      bool identity_mapping_;

      /** \brief Tree dimensionality (i.e. the number of dimensions per point). */
      int dim_;

      /** \brief The KdTree search parameters for K-nearest neighbors. */
      flann::SearchParams param_k_;

      /** \brief The KdTree search parameters for radius search. */
      flann::SearchParams param_radius_;

      /** \brief The total size of the data (either equal to the number of points in the input cloud or to the number of indices - if passed). */
      int total_nr_points_;
    };
}

#endif
