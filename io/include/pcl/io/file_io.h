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
 * $Id: file_io.h 827 2011-05-04 02:00:04Z nizar $
 *
 */

#ifndef PCL_IO_FILE_IO_H_
#define PCL_IO_FILE_IO_H_

#include "pcl/io/io.h"
#include <boost/numeric/conversion/cast.hpp>
#include <boost/lexical_cast.hpp>

namespace pcl
{
  /** \brief Point Cloud Data (FILE) file format reader interface.
    * Any (FILE) format file reader should implement its virtual methodes.
    * \author Nizar Sallem
    * \ingroup io
    */
  class PCL_EXPORTS FileReader
  {
    public:
      /** \brief empty constructor */ 
      FileReader() {}
      /** \brief empty destructor */ 
      virtual ~FileReader() {}
      /** \brief Read a point cloud data header from a FILE file. 
        *
        * Load only the meta information (number of points, their types, etc),
        * and not the points themselves, from a given FILE file. Useful for fast
        * evaluation of the underlying data structure.
        *
        * Returns:
        *  * < 0 (-1) on error
        *  * > 0 on success
        * \param file_name the name of the file to load
        * \param cloud the resultant point cloud dataset (only the header will be filled)
        * \param origin the sensor acquisition origin (only for > FILE_V7 - null if not present)
        * \param orientation the sensor acquisition orientation (only for > FILE_V7 - identity if not present)
        * \param file_version the FILE version of the file (either FILE_V6 or FILE_V7)
        * \param binary_data is true if the FILE file contains binary data, false if ascii 
        * \param data_idx the offset of cloud data within the file
        */
      virtual int 
      readHeader (const std::string &file_name, sensor_msgs::PointCloud2 &cloud, 
                  Eigen::Vector4f &origin, Eigen::Quaternionf &orientation, int &file_version,
                  bool &binary_data, int &data_idx) = 0;

      /** \brief Read a point cloud data from a FILE file and store it into a sensor_msgs/PointCloud2.
        * \param file_name the name of the file containing the actual PointCloud data
        * \param cloud the resultant PointCloud message read from disk
        * \param origin the sensor acquisition origin (only for > FILE_V7 - null if not present)
        * \param orientation the sensor acquisition orientation (only for > FILE_V7 - identity if not present)
        * \param file_version the FILE version of the file (either FILE_V6 or FILE_V7)
        */
      virtual int 
      read (const std::string &file_name, sensor_msgs::PointCloud2 &cloud, 
            Eigen::Vector4f &origin, Eigen::Quaternionf &orientation, int &file_version) = 0;

      /** \brief Read a point cloud data from a FILE file (FILE_V6 only!) and store it into a sensor_msgs/PointCloud2.
        * 
        * \note This function is provided for backwards compatibility only and
        * it can only read FILE_V6 files correctly, as sensor_msgs::PointCloud2
        * does not contain a sensor origin/orientation. Reading any file 
        * > FILE_V6 will generate a warning. 
        *
        * \param file_name the name of the file containing the actual PointCloud data
        * \param cloud the resultant PointCloud message read from disk
        */
      int 
      read (const std::string &file_name, sensor_msgs::PointCloud2 &cloud)
      {
        Eigen::Vector4f origin;
        Eigen::Quaternionf orientation;
        int file_version;
        return read(file_name, cloud, origin, orientation, file_version);
      }

      /** \brief Read a point cloud data from any FILE file, and convert it to the given template format.
        * \param file_name the name of the file containing the actual PointCloud data
        * \param cloud the resultant PointCloud message read from disk
        */
      template<typename PointT> inline int
      read (const std::string &file_name, pcl::PointCloud<PointT> &cloud)
      {
        sensor_msgs::PointCloud2 blob;
        int file_version;
        int res = read (file_name, blob, cloud.sensor_origin_, cloud.sensor_orientation_, 
                        file_version);

        // Exit in case of error
        if (res < 0)
          return res;
        pcl::fromROSMsg (blob, cloud);
        return 0;
      }

      /** \brief Copy one single value of type T (uchar, char, uint, int, float, double, ...) from a string
        * \param st the string containing the value to convert and copy
        * \param cloud the cloud to copy it to
        * \param point_index the index of the point
        * \param field_idx the index of the dimension/field
        * \param fields_count the current fields count
        */
      template <typename Type> inline void
      copyValue (const std::string &st, sensor_msgs::PointCloud2 &cloud, 
                 unsigned int point_index, unsigned int field_idx, unsigned int fields_count)
      {
        //char value = (char)atoi (st.at (d + c).c_str ());
        Type value;
        try
        {
          value = boost::numeric_cast<Type>(boost::lexical_cast<double>(st));
        }
        catch (...)
        {
          value = std::numeric_limits<Type>::quiet_NaN ();
          cloud.is_dense = false;
        }
        if (!pcl_isfinite (value))
          cloud.is_dense = false;

        memcpy (&cloud.data[point_index * cloud.point_step + 
                            cloud.fields[field_idx].offset + 
                            fields_count * sizeof (Type)], &value, sizeof (Type));
      }

  };

  /** \brief Point Cloud Data (FILE) file format writer.
    * Any (FILE) format file reader should implement its virtual methodes
    * \author Nizar Sallem
    * \ingroup io
    */
  class PCL_EXPORTS FileWriter
  {
    public:
      /** \brief empty constructor */ 
      FileWriter() {}
      /** \brief empty destructor */ 
      virtual ~FileWriter() {}
      /** \brief Save point cloud data to a FILE file containing n-D points
        * \param file_name the output file name
        * \param cloud the point cloud data message
        * \param origin the sensor acquisition origin
        * \param orientation the sensor acquisition orientation
        * \param binary set to true if the file is to be written in a binary
        * FILE format, false (default) for ASCII
        */
      virtual int
      write (const std::string &file_name, const sensor_msgs::PointCloud2 &cloud, 
             const Eigen::Vector4f &origin = Eigen::Vector4f::Zero (), 
             const Eigen::Quaternionf &orientation = Eigen::Quaternionf::Identity (),
             bool binary = false) = 0;

      /** \brief Save point cloud data to a FILE file containing n-D points
        * \param file_name the output file name
        * \param cloud the point cloud data message (boost shared pointer)
        * \param binary set to true if the file is to be written in a binary
        * FILE format, false (default) for ASCII
        * \param origin the sensor acquisition origin
        * \param orientation the sensor acquisition orientation
        */
      inline int
      write (const std::string &file_name, const sensor_msgs::PointCloud2::ConstPtr &cloud, 
             const Eigen::Vector4f &origin = Eigen::Vector4f::Zero (), 
             const Eigen::Quaternionf &orientation = Eigen::Quaternionf::Identity (),
             bool binary = false)
      {
        return (write (file_name, *cloud, origin, orientation, binary));
      }

      /** \brief Save point cloud data to a FILE file containing n-D points
        * \param file_name the output file name
        * \param cloud the pcl::PointCloud data
        * \param binary set to true if the file is to be written in a binary
        * FILE format, false (default) for ASCII
        */
      template<typename PointT> inline int
      write (const std::string &file_name, 
             const pcl::PointCloud<PointT> &cloud, 
             bool binary = false)
      {
        Eigen::Vector4f origin = cloud.sensor_origin_;
        Eigen::Quaternionf orientation = cloud.sensor_orientation_;

        sensor_msgs::PointCloud2 blob;
        pcl::toROSMsg (cloud, blob);

        // Save the data
        return (write (file_name, blob, origin, orientation, binary));
      }
  };
}

#endif  //#ifndef PCL_IO_FILE_IO_H_
