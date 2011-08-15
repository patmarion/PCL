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
 * $Id: pcd_io.hpp 2079 2011-08-15 04:38:21Z rusu $
 *
 */

#ifndef PCL_IO_PCD_IO_IMPL_H_
#define PCL_IO_PCD_IO_IMPL_H_

#include <fstream>
#include <fcntl.h>
#include <string>
#include <stdlib.h>
#include <boost/algorithm/string.hpp>

#ifdef _WIN32
# include <io.h>
# include <windows.h>
# define pcl_open                    _open
# define pcl_close(fd)               _close(fd)
# define pcl_lseek(fd,offset,origin) _lseek(fd,offset,origin)
#else
# include <sys/mman.h>
# define pcl_open                    open
# define pcl_close(fd)               close(fd)
# define pcl_lseek(fd,offset,origin) lseek(fd,offset,origin)
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename PointT> std::string
pcl::PCDWriter::generateHeader (const pcl::PointCloud<PointT> &cloud, const int nr_points)
{
  std::ostringstream oss;

  oss << "# .PCD v0.7 - Point Cloud Data file format"
         "\nVERSION 0.7"
         "\nFIELDS";

  std::vector<sensor_msgs::PointField> fields;
  pcl::getFields (cloud, fields);
 
  std::stringstream field_names, field_types, field_sizes, field_counts;
  for (size_t i = 0; i < fields.size (); ++i)
  {
    if (fields[i].name == "_")
      continue;
    // Add the regular dimension
    field_names << " " << fields[i].name;
    field_sizes << " " << pcl::getFieldSize (fields[i].datatype);
    field_types << " " << pcl::getFieldType (fields[i].datatype);
    int count = abs ((int)fields[i].count);
    if (count == 0) count = 1;  // check for 0 counts (coming from older converter code)
    field_counts << " " << count;
  }
  oss << field_names.str ();
  oss << "\nSIZE" << field_sizes.str () 
      << "\nTYPE" << field_types.str () 
      << "\nCOUNT" << field_counts.str ();
  // If the user passes in a number of points value, use that instead
  if (nr_points != std::numeric_limits<int>::max ())
    oss << "\nWIDTH " << nr_points << "\nHEIGHT " << 1 << "\n";
  else
    oss << "\nWIDTH " << cloud.width << "\nHEIGHT " << cloud.height << "\n";

  oss << "VIEWPOINT " << cloud.sensor_origin_[0] << " " << cloud.sensor_origin_[1] << " " << cloud.sensor_origin_[2] << " " << 
                         cloud.sensor_orientation_.w () << " " << 
                         cloud.sensor_orientation_.x () << " " << 
                         cloud.sensor_orientation_.y () << " " << 
                         cloud.sensor_orientation_.z () << "\n";
  
  // If the user passes in a number of points value, use that instead
  if (nr_points != std::numeric_limits<int>::max ())
    oss << "POINTS " << nr_points << "\n";
  else
    oss << "POINTS " << cloud.points.size () << "\n";

  return (oss.str ());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename PointT> int
pcl::PCDWriter::writeBinary (const std::string &file_name, 
                             const pcl::PointCloud<PointT> &cloud)
{
  if (cloud.points.empty ())
  {
    throw pcl::IOException ("[pcl::PCDWriter::writeBinary] Input point cloud has no data!");
    return (-1);
  }
  int data_idx = 0;
  std::ostringstream oss;
  oss << generateHeader<PointT> (cloud) << "DATA binary\n";
  oss.flush ();
  data_idx = oss.tellp ();

#if _WIN32
  HANDLE h_native_file = CreateFile (file_name.c_str (), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if(h_native_file == INVALID_HANDLE_VALUE)
  {
    throw pcl::IOException ("[pcl::PCDWriter::writeBinary] Error during CreateFile!");
    return (-1);
  }
#else
  int fd = pcl_open (file_name.c_str (), O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
  if (fd < 0)
  {
    throw pcl::IOException ("[pcl::PCDWriter::writeBinary] Error during open!");
    return (-1);
  }
#endif

  std::vector<sensor_msgs::PointField> fields;
  std::vector<int> fields_sizes;
  size_t fsize = 0;
  size_t data_size = 0;
  size_t nri = 0;
  pcl::getFields (cloud, fields);
  // Compute the total size of the fields
  for (size_t i = 0; i < fields.size (); ++i)
  {
    if (fields[i].name == "_")
      continue;
    
    int fs = fields[i].count * getFieldSize (fields[i].datatype);
    fsize += fs;
    fields_sizes.push_back (fs);
    fields[nri++] = fields[i];
  }
  fields.resize (nri);
  
  data_size = cloud.points.size () * fsize;

  // Prepare the map
#if _WIN32
  HANDLE fm = CreateFileMapping (h_native_file, NULL, PAGE_READWRITE, 0, data_idx + data_size, NULL);
  char *map = static_cast<char*>(MapViewOfFile (fm, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, data_idx + data_size));
  CloseHandle (fm);

#else
  // Stretch the file size to the size of the data
  int result = pcl_lseek (fd, getpagesize () + data_size - 1, SEEK_SET);
  if (result < 0)
  {
    pcl_close (fd);
    throw pcl::IOException ("[pcl::PCDWriter::writeBinary] Error during lseek ()!");
    return (-1);
  }
  // Write a bogus entry so that the new file size comes in effect
  result = ::write (fd, "", 1);
  if (result != 1)
  {
    pcl_close (fd);
    throw pcl::IOException ("[pcl::PCDWriter::writeBinary] Error during write ()!");
    return (-1);
  }

  char *map = (char*)mmap (0, data_idx + data_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (map == MAP_FAILED)
  {
    pcl_close (fd);
    throw pcl::IOException ("[pcl::PCDWriter::writeBinary] Error during mmap ()!");
    return (-1);
  }
#endif

  // Copy the header
  memcpy (&map[0], oss.str ().c_str (), data_idx);

  // Copy the data
  char *out = &map[0] + data_idx;
  for (size_t i = 0; i < cloud.points.size (); ++i)
  {
    int nrj = 0;
    for (size_t j = 0; j < fields.size (); ++j)
    {
      memcpy (out, (const char*)&cloud.points[i] + fields[j].offset, fields_sizes[nrj]);
      out += fields_sizes[nrj++];
    }
  }

  // Unmap the pages of memory
#if _WIN32
    UnmapViewOfFile (map);
#else
  if (munmap (map, (data_idx + data_size)) == -1)
  {
    pcl_close (fd);
    throw pcl::IOException ("[pcl::PCDWriter::writeBinary] Error during munmap ()!");
    return (-1);
  }
#endif
  // Close file
#if _WIN32
  CloseHandle(h_native_file);
#else
  pcl_close (fd);
#endif
  return (0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename PointT> int
pcl::PCDWriter::writeASCII (const std::string &file_name, const pcl::PointCloud<PointT> &cloud, 
                            const int precision)
{
  if (cloud.points.empty ())
  {
    throw pcl::IOException ("[pcl::PCDWriter::writeASCII] Input point cloud has no data!");
    return (-1);
  }

  std::ofstream fs;
  fs.precision (precision);
  fs.open (file_name.c_str ());      // Open file
  if (!fs.is_open () || fs.fail ())
  {
    throw pcl::IOException ("[pcl::PCDWriter::writeASCII] Could not open file for writing!");
    return (-1);
  }

  if (cloud.width * cloud.height != cloud.points.size ())
  {
    throw pcl::IOException ("[pcl::PCDWriter::writeASCII] Number of points different than width * height!");
    return (-1);
  }
  std::vector<sensor_msgs::PointField> fields;
  pcl::getFields (cloud, fields);

  // Write the header information
  fs << generateHeader<PointT> (cloud) << "DATA ascii\n";

  std::ostringstream stream;
  stream.precision (precision);

  // Iterate through the points
  for (size_t i = 0; i < cloud.points.size (); ++i)
  {
    for (size_t d = 0; d < fields.size (); ++d)
    {
      // Ignore invalid padded dimensions that are inherited from binary data
      if (fields[d].name == "_")
        continue;

      int count = fields[d].count;
      if (count == 0) 
        count = 1;          // we simply cannot tolerate 0 counts (coming from older converter code)

      for (int c = 0; c < count; ++c)
      {
        switch (fields[d].datatype)
        {
          case sensor_msgs::PointField::INT8:
          {
            int8_t value;
            memcpy (&value, (const char*)&cloud.points[i] + fields[d].offset + c * sizeof (int8_t), sizeof (int8_t));
            if (pcl_isnan (value))
              stream << "nan";
            else
              stream << boost::numeric_cast<uint32_t>(value);
            break;
          }
          case sensor_msgs::PointField::UINT8:
          {
            uint8_t value;
            memcpy (&value, (const char*)&cloud.points[i] + fields[d].offset + c * sizeof (uint8_t), sizeof (uint8_t));
            if (pcl_isnan (value))
              stream << "nan";
            else
              stream << boost::numeric_cast<uint32_t>(value);
            break;
          }
          case sensor_msgs::PointField::INT16:
          {
            int16_t value;
            memcpy (&value, (const char*)&cloud.points[i] + fields[d].offset + c * sizeof (int16_t), sizeof (int16_t));
            if (pcl_isnan (value))
              stream << "nan";
            else
              stream << boost::numeric_cast<int16_t>(value);
            break;
          }
          case sensor_msgs::PointField::UINT16:
          {
            uint16_t value;
            memcpy (&value, (const char*)&cloud.points[i] + fields[d].offset + c * sizeof (uint16_t), sizeof (uint16_t));
            if (pcl_isnan (value))
              stream << "nan";
            else
              stream << boost::numeric_cast<uint16_t>(value);
            break;
          }
          case sensor_msgs::PointField::INT32:
          {
            int32_t value;
            memcpy (&value, (const char*)&cloud.points[i] + fields[d].offset + c * sizeof (int32_t), sizeof (int32_t));
            if (pcl_isnan (value))
              stream << "nan";
            else
              stream << boost::numeric_cast<int32_t>(value);
            break;
          }
          case sensor_msgs::PointField::UINT32:
          {
            uint32_t value;
            memcpy (&value, (const char*)&cloud.points[i] + fields[d].offset + c * sizeof (uint32_t), sizeof (uint32_t));
            if (pcl_isnan (value))
              stream << "nan";
            else
              stream << boost::numeric_cast<uint32_t>(value);
            break;
          }
          case sensor_msgs::PointField::FLOAT32:
          {
            float value;
            memcpy (&value, (const char*)&cloud.points[i] + fields[d].offset + c * sizeof (float), sizeof (float));
            if (pcl_isnan (value))
              stream << "nan";
            else
              stream << boost::numeric_cast<float>(value);
            break;
          }
          case sensor_msgs::PointField::FLOAT64:
          {
            double value;
            memcpy (&value, (const char*)&cloud.points[i] + fields[d].offset + c * sizeof (double), sizeof (double));
            if (pcl_isnan (value))
              stream << "nan";
            else
              stream << boost::numeric_cast<double>(value);
            break;
          }
          default:
            PCL_WARN ("[pcl::PCDWriter::writeASCII] Incorrect field data type specified (%d)!\n", fields[d].datatype);
            break;
        }

        if (d < fields.size () - 1 || c < (int)fields[d].count - 1)
          stream << " ";
      }
    }
    // Copy the stream, trim it, and write it to disk
    std::string result = stream.str ();
    boost::trim (result);
    stream.str ("");
    fs << result << "\n";
  }
  fs.close ();              // Close file
  return (0);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename PointT> int
pcl::PCDWriter::writeBinary (const std::string &file_name, 
                             const pcl::PointCloud<PointT> &cloud, 
                             const std::vector<int> &indices)
{
  if (cloud.points.empty () || indices.empty ())
  {
    throw pcl::IOException ("[pcl::PCDWriter::writeBinary] Input point cloud has no data or empty indices given!");
    return (-1);
  }
  int data_idx = 0;
  std::ostringstream oss;
  oss << generateHeader<PointT> (cloud, indices.size ()) << "DATA binary\n";
  oss.flush ();
  data_idx = oss.tellp ();

#if _WIN32
  HANDLE h_native_file = CreateFile (file_name.c_str (), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if(h_native_file == INVALID_HANDLE_VALUE)
  {
    throw pcl::IOException ("[pcl::PCDWriter::writeBinary] Error during CreateFile!");
    return (-1);
  }
#else
  int fd = pcl_open (file_name.c_str (), O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
  if (fd < 0)
  {
    throw pcl::IOException ("[pcl::PCDWriter::writeBinary] Error during open!");
    return (-1);
  }
#endif

  std::vector<sensor_msgs::PointField> fields;
  std::vector<int> fields_sizes;
  size_t fsize = 0;
  size_t data_size = 0;
  size_t nri = 0;
  pcl::getFields (cloud, fields);
  // Compute the total size of the fields
  for (size_t i = 0; i < fields.size (); ++i)
  {
    if (fields[i].name == "_")
      continue;
    
    int fs = fields[i].count * getFieldSize (fields[i].datatype);
    fsize += fs;
    fields_sizes.push_back (fs);
    fields[nri++] = fields[i];
  }
  fields.resize (nri);
  
  data_size = indices.size () * fsize;

  // Prepare the map
#if _WIN32
  HANDLE fm = CreateFileMapping (h_native_file, NULL, PAGE_READWRITE, 0, data_idx + data_size, NULL);
  char *map = static_cast<char*>(MapViewOfFile (fm, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, data_idx + data_size));
  CloseHandle (fm);

#else
  // Stretch the file size to the size of the data
  int result = pcl_lseek (fd, getpagesize () + data_size - 1, SEEK_SET);
  if (result < 0)
  {
    pcl_close (fd);
    throw pcl::IOException ("[pcl::PCDWriter::writeBinary] Error during lseek ()!");
    return (-1);
  }
  // Write a bogus entry so that the new file size comes in effect
  result = ::write (fd, "", 1);
  if (result != 1)
  {
    pcl_close (fd);
    throw pcl::IOException ("[pcl::PCDWriter::writeBinary] Error during write ()!");
    return (-1);
  }

  char *map = (char*)mmap (0, data_idx + data_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (map == MAP_FAILED)
  {
    pcl_close (fd);
    throw pcl::IOException ("[pcl::PCDWriter::writeBinary] Error during mmap ()!");
    return (-1);
  }
#endif

  // Copy the header
  memcpy (&map[0], oss.str ().c_str (), data_idx);

  char *out = &map[0] + data_idx;
  // Copy the data
  for (size_t i = 0; i < indices.size (); ++i)
  {
    int nrj = 0;
    for (size_t j = 0; j < fields.size (); ++j)
    {
      memcpy (out, (const char*)&cloud.points[indices[i]] + fields[j].offset, fields_sizes[nrj]);
      out += fields_sizes[nrj++];
    }
  }

  // Unmap the pages of memory
#if _WIN32
    UnmapViewOfFile (map);
#else
  if (munmap (map, (data_idx + data_size)) == -1)
  {
    pcl_close (fd);
    throw pcl::IOException ("[pcl::PCDWriter::writeBinary] Error during munmap ()!");
    return (-1);
  }
#endif
  // Close file
#if _WIN32
  CloseHandle(h_native_file);
#else
  pcl_close (fd);
#endif
  return (0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename PointT> int
pcl::PCDWriter::writeASCII (const std::string &file_name, 
                            const pcl::PointCloud<PointT> &cloud, 
                            const std::vector<int> &indices,
                            const int precision)
{
  if (cloud.points.empty () || indices.empty ())
  {
    throw pcl::IOException ("[pcl::PCDWriter::writeASCII] Input point cloud has no data or empty indices given!");
    return (-1);
  }

  std::ofstream fs;
  fs.precision (precision);
  fs.open (file_name.c_str ());      // Open file
  if (!fs.is_open () || fs.fail ())
  {
    throw pcl::IOException ("[pcl::PCDWriter::writeASCII] Could not open file for writing!");
    return (-1);
  }

  if (cloud.width * cloud.height != cloud.points.size ())
  {
    throw pcl::IOException ("[pcl::PCDWriter::writeASCII] Number of points different than width * height!");
    return (-1);
  }
  std::vector<sensor_msgs::PointField> fields;
  pcl::getFields (cloud, fields);

  // Write the header information
  fs << generateHeader<PointT> (cloud, indices.size ()) << "DATA ascii\n";

  std::ostringstream stream;
  stream.precision (precision);

  // Iterate through the points
  for (size_t i = 0; i < indices.size (); ++i)
  {
    for (size_t d = 0; d < fields.size (); ++d)
    {
      // Ignore invalid padded dimensions that are inherited from binary data
      if (fields[d].name == "_")
        continue;

      int count = fields[d].count;
      if (count == 0) 
        count = 1;          // we simply cannot tolerate 0 counts (coming from older converter code)

      for (int c = 0; c < count; ++c)
      {
        switch (fields[d].datatype)
        {
          case sensor_msgs::PointField::INT8:
          {
            int8_t value;
            memcpy (&value, (const char*)&cloud.points[indices[i]] + fields[d].offset + c * sizeof (int8_t), sizeof (int8_t));
            if (pcl_isnan (value))
              stream << "nan";
            else
              stream << boost::numeric_cast<uint32_t>(value);
            break;
          }
          case sensor_msgs::PointField::UINT8:
          {
            uint8_t value;
            memcpy (&value, (const char*)&cloud.points[indices[i]] + fields[d].offset + c * sizeof (uint8_t), sizeof (uint8_t));
            if (pcl_isnan (value))
              stream << "nan";
            else
              stream << boost::numeric_cast<uint32_t>(value);
            break;
          }
          case sensor_msgs::PointField::INT16:
          {
            int16_t value;
            memcpy (&value, (const char*)&cloud.points[indices[i]] + fields[d].offset + c * sizeof (int16_t), sizeof (int16_t));
            if (pcl_isnan (value))
              stream << "nan";
            else
              stream << boost::numeric_cast<int16_t>(value);
            break;
          }
          case sensor_msgs::PointField::UINT16:
          {
            uint16_t value;
            memcpy (&value, (const char*)&cloud.points[indices[i]] + fields[d].offset + c * sizeof (uint16_t), sizeof (uint16_t));
            if (pcl_isnan (value))
              stream << "nan";
            else
              stream << boost::numeric_cast<uint16_t>(value);
            break;
          }
          case sensor_msgs::PointField::INT32:
          {
            int32_t value;
            memcpy (&value, (const char*)&cloud.points[indices[i]] + fields[d].offset + c * sizeof (int32_t), sizeof (int32_t));
            if (pcl_isnan (value))
              stream << "nan";
            else
              stream << boost::numeric_cast<int32_t>(value);
            break;
          }
          case sensor_msgs::PointField::UINT32:
          {
            uint32_t value;
            memcpy (&value, (const char*)&cloud.points[indices[i]] + fields[d].offset + c * sizeof (uint32_t), sizeof (uint32_t));
            if (pcl_isnan (value))
              stream << "nan";
            else
              stream << boost::numeric_cast<uint32_t>(value);
            break;
          }
          case sensor_msgs::PointField::FLOAT32:
          {
            float value;
            memcpy (&value, (const char*)&cloud.points[indices[i]] + fields[d].offset + c * sizeof (float), sizeof (float));
            if (pcl_isnan (value))
              stream << "nan";
            else
              stream << boost::numeric_cast<float>(value);
            break;
          }
          case sensor_msgs::PointField::FLOAT64:
          {
            double value;
            memcpy (&value, (const char*)&cloud.points[indices[i]] + fields[d].offset + c * sizeof (double), sizeof (double));
            if (pcl_isnan (value))
              stream << "nan";
            else
              stream << boost::numeric_cast<double>(value);
            break;
          }
          default:
            PCL_WARN ("[pcl::PCDWriter::writeASCII] Incorrect field data type specified (%d)!\n", fields[d].datatype);
            break;
        }

        if (d < fields.size () - 1 || c < (int)fields[d].count - 1)
          stream << " ";
      }
    }
    // Copy the stream, trim it, and write it to disk
    std::string result = stream.str ();
    boost::trim (result);
    stream.str ("");
    fs << result << "\n";
  }
  fs.close ();              // Close file
  return (0);
}

#endif  //#ifndef PCL_IO_PCD_IO_H_

