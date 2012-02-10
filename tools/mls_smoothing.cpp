/*
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2011, Willow Garage, Inc.
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
 * $Id: mls_smoothing.cpp 4314 2012-02-08 12:31:50Z aichim $
 *
 */

#include <sensor_msgs/PointCloud2.h>
#include <pcl/point_types.h>
#include <pcl/io/pcd_io.h>
#include <pcl/console/print.h>
#include <pcl/console/parse.h>
#include <pcl/console/time.h>
#include <pcl/surface/mls.h>
#include <pcl/filters/voxel_grid.h>

using namespace pcl;
using namespace pcl::io;
using namespace pcl::console;

int default_polynomial_order = 2;
bool default_use_polynomial_fit = false;
double default_search_radius = 0.0,
    default_sqr_gauss_param = 0.0;


void
printHelp (int argc, char **argv)
{
  print_error ("Syntax is: %s input.pcd output.pcd <options>\n", argv[0]);
  print_info ("  where options are:\n");
  print_info ("                     -search_radius X = sphere radius to be used for finding the k-nearest neighbors used for fitting (default: ");
  print_value ("%f", default_search_radius); print_info (")\n");
  print_info ("                     -sqr_gauss_param X = parameter used for the distance based weighting of neighbors (recommended = search_radius^2) (default: ");
  print_value ("%f", default_sqr_gauss_param); print_info (")\n");
  print_info ("                     -use_polynomial_fit X = decides whether the surface and normal are approximated using a polynomial or only via tangent estimation (default: ");
  print_value ("%d", default_use_polynomial_fit); print_info (")\n");
  print_info ("                     -polynomial_order X = order of the polynomial to be fit (implicitly, use_polynomial_fit = 1) (default: ");
  print_value ("%d", default_polynomial_order); print_info (")\n");
}

bool
loadCloud (const std::string &filename, sensor_msgs::PointCloud2 &cloud)
{
  TicToc tt;
  print_highlight ("Loading "); print_value ("%s ", filename.c_str ());

  tt.tic ();
  if (loadPCDFile (filename, cloud) < 0)
    return (false);
  print_info ("[done, "); print_value ("%g", tt.toc ()); print_info (" ms : "); print_value ("%d", cloud.width * cloud.height); print_info (" points]\n");
  print_info ("Available dimensions: "); print_value ("%s\n", pcl::getFieldsList (cloud).c_str ());

  return (true);
}

void
compute (const sensor_msgs::PointCloud2::ConstPtr &input, sensor_msgs::PointCloud2 &output,
         double search_radius, bool sqr_gauss_param_set, double sqr_gauss_param,
         bool use_polynomial_fit, int polynomial_order)
{

  PointCloud<PointXYZ>::Ptr xyz_cloud_pre (new pcl::PointCloud<PointXYZ> ()),
      xyz_cloud (new pcl::PointCloud<PointXYZ> ());
  fromROSMsg (*input, *xyz_cloud_pre);

  // Filter the NaNs from the cloud
  for (size_t i = 0; i < xyz_cloud_pre->size (); ++i)
    if (pcl_isfinite (xyz_cloud_pre->points[i].x))
      xyz_cloud->push_back (xyz_cloud_pre->points[i]);
  xyz_cloud->header = xyz_cloud_pre->header;
  xyz_cloud->height = 1;
  xyz_cloud->width = xyz_cloud->size ();
  xyz_cloud->is_dense = false;

//  io::savePCDFile ("test.pcd", *xyz_cloud);

  PointCloud<PointXYZ>::Ptr xyz_cloud_smoothed (new PointCloud<PointXYZ> ());

  MovingLeastSquares<PointXYZ, Normal> mls;
  mls.setInputCloud (xyz_cloud);
  mls.setSearchRadius (search_radius);
  if (sqr_gauss_param_set) mls.setSqrGaussParam (sqr_gauss_param);
  mls.setPolynomialFit (use_polynomial_fit);
  mls.setPolynomialOrder (polynomial_order);

//  mls.setUpsamplingMethod (MovingLeastSquares<PointXYZ, Normal>::SAMPLE_LOCAL_PLANE);
  mls.setUpsamplingMethod (MovingLeastSquares<PointXYZ, Normal>::UNIFORM_DENSITY);
//  mls.setUpsamplingMethod (MovingLeastSquares<PointXYZ, Normal>::FILL_HOLES);
//  mls.setUpsamplingMethod (MovingLeastSquares<PointXYZ, Normal>::NONE);
  mls.setFillingStepSize (0.02);
  mls.setPointDensity (50000*search_radius); // 300 points in a 5 cm radius
  mls.setUpsamplingRadius (0.025);
  mls.setUpsamplingStepSize (0.015);

  search::KdTree<PointXYZ>::Ptr tree (new search::KdTree<PointXYZ> ());
  mls.setSearchMethod (tree);
  PointCloud<Normal>::Ptr mls_normals (new PointCloud<Normal> ());
  mls.setOutputNormals (mls_normals);

  PCL_INFO ("Computing smoothed surface and normals with search_radius %f , sqr_gaussian_param %f, polynomial fitting %d, polynomial order %d\n",
            mls.getSearchRadius(), mls.getSqrGaussParam(), mls.getPolynomialFit(), mls.getPolynomialOrder());
  TicToc tt;
  tt.tic ();
  mls.reconstruct (*xyz_cloud_smoothed);
  print_info ("[done, "); print_value ("%g", tt.toc ()); print_info (" ms : "); print_value ("%d", xyz_cloud_smoothed->width * xyz_cloud_smoothed->height); print_info (" points]\n");

  sensor_msgs::PointCloud2 output_positions, output_normals;
//  printf ("sizes: %d %d   %d\n", xyz_cloud_smoothed->width, xyz_cloud_smoothed->height, xyz_cloud_smoothed->size ());
  toROSMsg (*xyz_cloud_smoothed, output_positions);
  toROSMsg (*mls_normals, output_normals);

  concatenateFields (output_positions, output_normals, output);


  PointCloud<PointXYZ> xyz_vg;
  VoxelGrid<PointXYZ> vg;
  vg.setInputCloud (xyz_cloud_smoothed);
  vg.setLeafSize (0.005, 0.005, 0.005);
  vg.filter (xyz_vg);
  sensor_msgs::PointCloud2 xyz_vg_2;
  toROSMsg (xyz_vg, xyz_vg_2);
  pcl::io::savePCDFile ("cloud_vg.pcd", xyz_vg_2,  Eigen::Vector4f::Zero (),
                        Eigen::Quaternionf::Identity (), true);
}

void
saveCloud (const std::string &filename, const sensor_msgs::PointCloud2 &output)
{
  TicToc tt;
  tt.tic ();

  print_highlight ("Saving "); print_value ("%s ", filename.c_str ());

  pcl::io::savePCDFile (filename, output,  Eigen::Vector4f::Zero (),
                        Eigen::Quaternionf::Identity (), true);

  print_info ("[done, "); print_value ("%g", tt.toc ()); print_info (" ms : "); print_value ("%d", output.width * output.height); print_info (" points]\n");
}

/* ---[ */
int
main (int argc, char** argv)
{
  print_info ("Moving Least Squares smoothing of a point cloud. For more information, use: %s -h\n", argv[0]);

  if (argc < 3)
  {
    printHelp (argc, argv);
    return (-1);
  }

  // Parse the command line arguments for .pcd files
  std::vector<int> p_file_indices;
  p_file_indices = parse_file_extension_argument (argc, argv, ".pcd");
  if (p_file_indices.size () != 2)
  {
    print_error ("Need one input PCD file and one output PCD file to continue.\n");
    return (-1);
  }

  // Command line parsing
  double search_radius = default_search_radius;
  double sqr_gauss_param = default_sqr_gauss_param;
  bool sqr_gauss_param_set = true;
  int polynomial_order = default_polynomial_order;
  bool use_polynomial_fit = default_use_polynomial_fit;

  parse_argument (argc, argv, "-search_radius", search_radius);
  if (parse_argument (argc, argv, "-sqr_gauss_param", sqr_gauss_param) == -1)
    sqr_gauss_param_set = false;
  if (parse_argument (argc, argv, "-polynomial_order", polynomial_order) != -1 )
    use_polynomial_fit = true;
  parse_argument (argc, argv, "-use_polynomial_fit", use_polynomial_fit);

  // Load the first file
  sensor_msgs::PointCloud2::Ptr cloud (new sensor_msgs::PointCloud2);
  if (!loadCloud (argv[p_file_indices[0]], *cloud))
    return (-1);

  // Do the smoothing
  sensor_msgs::PointCloud2 output;
  compute (cloud, output, search_radius, sqr_gauss_param_set, sqr_gauss_param,
           use_polynomial_fit, polynomial_order);

  // Save into the second file
  saveCloud (argv[p_file_indices[1]], output);
}
