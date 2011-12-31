/*
 * Software License Agreement (BSD License)
 *
 *  Point Cloud Library (PCL) - www.pointclouds.org
 *  Copyright (c) 2011, Dirk Holz, University of Bonn.
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

#ifndef PCL_IO_VTK_LIB_IO_H_
#define PCL_IO_VTK_LIB_IO_H_

#include <vtkSmartPointer.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkCellArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkFloatArray.h>
#include <vtkPolyDataReader.h>
#include <vtkPolyDataWriter.h>
#include <vtkPLYReader.h>
#include <vtkPLYWriter.h>
#include <vtkOBJReader.h>
#include <vtkSTLReader.h>
#include <vtkSTLWriter.h>
#include <boost/filesystem.hpp>

#include "pcl/point_types.h"
#include "pcl/point_cloud.h"
#include "pcl/PolygonMesh.h"
#include "pcl/pcl_macros.h"
#include "pcl/ros/conversions.h"
#include "pcl/io/pcd_io.h"

namespace pcl
{
  namespace io
  {
    /** \brief Convert vtkPolyData object to a PCL PolygonMesh
      * \param[in] poly_data Pointer (vtkSmartPointer) to a vtkPolyData object
      * \param[out] mesh PCL Polygon Mesh to fill
      * \return Number of points in the point cloud of mesh.
      */
    PCL_EXPORTS int
    vtk2mesh (const vtkSmartPointer<vtkPolyData>& poly_data, pcl::PolygonMesh& mesh);

    /** \brief Convert a PCL PolygonMesh to a vtkPolyData object
      * \param[in] mesh Reference to PCL Polygon Mesh
      * \param[out] poly_data Pointer (vtkSmartPointer) to a vtkPolyData object
      * \return Number of points in the point cloud of mesh.
      */
    PCL_EXPORTS int
    mesh2vtk (const pcl::PolygonMesh& mesh, vtkSmartPointer<vtkPolyData>& poly_data);

    /** \brief Load a \ref PolygonMesh object given an input file name, based on the file extension
      * \param[in] file_name the name of the file containing the polygon data
      * \param[out] mesh the object that we want to load the data in 
      */ 
    PCL_EXPORTS int
    loadPolygonFile (const std::string &file_name, pcl::PolygonMesh& mesh);

    /** \brief Save a \ref PolygonMesh object given an input file name, based on the file extension
      * \param[in] file_name the name of the file to save the data to
      * \param[in] mesh the object that contains the data
      */
    PCL_EXPORTS int
    savePolygonFile (const std::string &file_name, const pcl::PolygonMesh& mesh);

    /** \brief Load a VTK file into a \ref PolygonMesh object
      * \param[in] file_name the name of the file that contains the data
      * \param[out] mesh the object that we want to load the data in 
      */
    PCL_EXPORTS int
    loadPolygonFileVTK (const std::string &file_name, pcl::PolygonMesh& mesh);

    /** \brief Load a PLY file into a \ref PolygonMesh object
      * \param[in] file_name the name of the file that contains the data
      * \param[out] mesh the object that we want to load the data in 
      */
    PCL_EXPORTS int
    loadPolygonFilePLY (const std::string &file_name, pcl::PolygonMesh& mesh);

    /** \brief Load an OBJ file into a \ref PolygonMesh object
      * \param[in] file_name the name of the file that contains the data
      * \param[out] mesh the object that we want to load the data in 
      */
    PCL_EXPORTS int
    loadPolygonFileOBJ (const std::string &file_name, pcl::PolygonMesh& mesh);

    /** \brief Load an STL file into a \ref PolygonMesh object
      * \param[in] file_name the name of the file that contains the data
      * \param[out] mesh the object that we want to load the data in 
      */
    PCL_EXPORTS int
    loadPolygonFileSTL (const std::string &file_name, pcl::PolygonMesh& mesh);

    /** \brief Save a \ref PolygonMesh object into a VTK file
      * \param[in] file_name the name of the file to save the data to
      * \param[in] mesh the object that contains the data
      */
    PCL_EXPORTS int
    savePolygonFileVTK (const std::string &file_name, const pcl::PolygonMesh& mesh);

    /** \brief Save a \ref PolygonMesh object into a PLY file
      * \param[in] file_name the name of the file to save the data to
      * \param[in] mesh the object that contains the data
      */
    PCL_EXPORTS int
    savePolygonFilePLY (const std::string &file_name, const pcl::PolygonMesh& mesh);

    /** \brief Save a \ref PolygonMesh object into an STL file
      * \param[in] file_name the name of the file to save the data to
      * \param[in] mesh the object that contains the data
      */
    PCL_EXPORTS int
    savePolygonFileSTL (const std::string &file_name, const pcl::PolygonMesh& mesh);

    /**
     * TO DO proper building the library (depending on whether or not vtk was found)
     * TO DO writing files
     * TO DO more importers/exporters
     * TO DO IMPORTANT colors and other scalars
     * TO DO DXF files
     *
     */
  }
}

#endif /* PLC_IO_VTK_LIB_IO_H_ */
