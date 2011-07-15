/* \author Bastian Steder */

/* ---[ */
#include <iostream>
using namespace std;
#include <pcl/io/openni_grabber.h>
#include "pcl/range_image/range_image_planar.h"
#include "pcl/common/common_headers.h"
#include "pcl/visualization/range_image_visualizer.h"
#include "pcl/visualization/pcl_visualizer.h"
#include "pcl/features/range_image_border_extractor.h"
#include "pcl/keypoints/narf_keypoint.h"
#include "pcl/console/parse.h"

using namespace pcl;
using namespace pcl::visualization;

std::string device_id = "#1";

float angular_resolution = 0.5f;
float support_size = 0.2f;
bool set_unseen_to_max_range = true;

boost::mutex depth_image_mutex,
             ir_image_mutex,
             image_mutex;
pcl::PointCloud<pcl::PointXYZ>::ConstPtr point_cloud_ptr;
boost::shared_ptr<openni_wrapper::DepthImage> depth_image_ptr;
boost::shared_ptr<openni_wrapper::IRImage> ir_image_ptr;
boost::shared_ptr<openni_wrapper::Image> image_ptr;

bool received_new_depth_data = false,
     received_new_ir_image   = false,
     received_new_image   = false;
struct EventHelper
{
  void
  depth_image_cb (const boost::shared_ptr<openni_wrapper::DepthImage>& depth_image)
  {
    if (depth_image_mutex.try_lock ())
    {
      depth_image_ptr = depth_image;
      depth_image_mutex.unlock ();
      received_new_depth_data = true;
    }
  }
};


void
printUsage (const char* progName)
{
  cout << "\n\nUsage: "<<progName<<" [options] [scene.pcd] <model.pcl> [model_2.pcl] ... [model_n.pcl]\n\n"
       << "Options:\n"
       << "-------------------------------------------\n"
       << "-d <device_id>  set the device id (default \""<<device_id<<"\")\n"
       << "-r <float>      angular resolution in degrees (default "<<rad2deg (angular_resolution)<<")\n"
       << "-s <float>      support size for the interest points (diameter of the used sphere in meters) (default "<<support_size<<")\n"
       << "-h              this help\n"
       << "\n\n";
}

int main (int argc, char** argv)
{
  // --------------------------------------
  // -----Parse Command Line Arguments-----
  // --------------------------------------
  if (pcl::console::find_argument (argc, argv, "-h") >= 0)
  {
    printUsage (argv[0]);
    return 0;
  }
  if (pcl::console::parse (argc, argv, "-d", device_id) >= 0)
    cout << "Using device id \""<<device_id<<"\".\n";
  if (pcl::console::parse (argc, argv, "-r", angular_resolution) >= 0)
    cout << "Setting angular resolution to "<<angular_resolution<<"deg.\n";
  if (pcl::console::parse (argc, argv, "-s", support_size) >= 0)
    cout << "Setting support size to "<<angular_resolution<<"m.\n";
  angular_resolution = deg2rad (angular_resolution);
  
  RangeImageVisualizer range_image_widget ("Range Image");
  
  PCLVisualizer viewer ("3D Viewer");
  viewer.addCoordinateSystem (1.0f);
  viewer.setBackgroundColor (1, 1, 1);
  
  viewer.initCameraParameters ();
  viewer.camera_.pos[0] = 0.0;
  viewer.camera_.pos[1] = -0.3;
  viewer.camera_.pos[2] = -2.0;
  viewer.camera_.focal[0] = 0.0;
  viewer.camera_.focal[1] = 0.0+viewer.camera_.pos[1];
  viewer.camera_.focal[2] = 1.0;
  viewer.camera_.view[0] = 0.0;
  viewer.camera_.view[1] = -1.0;
  viewer.camera_.view[2] = 0.0;
  viewer.updateCamera();
  
  openni_wrapper::OpenNIDriver& driver = openni_wrapper::OpenNIDriver::getInstance ();
  if (driver.getNumberDevices () > 0)
  {
    for (unsigned deviceIdx = 0; deviceIdx < driver.getNumberDevices (); ++deviceIdx)
    {
      cout << "Device: " << deviceIdx + 1 << ", vendor: " << driver.getVendorName (deviceIdx)
           << ", product: " << driver.getProductName (deviceIdx) << ", connected: "
           << (int) driver.getBus (deviceIdx) << " @ " << (int) driver.getAddress (deviceIdx)
           << ", serial number: \'" << driver.getSerialNumber (deviceIdx) << "\'\n";
    }
  }
  else
  {
    cout << "\nNo devices connected.\n\n";
    return 1;
  }
  
  pcl::Grabber* interface = new pcl::OpenNIGrabber (device_id);
  EventHelper event_helper;
  
  boost::function<void (const boost::shared_ptr<openni_wrapper::DepthImage>&) > f_depth_image =
    boost::bind (&EventHelper::depth_image_cb, &event_helper, _1);
  boost::signals2::connection c_depth_image = interface->registerCallback (f_depth_image);
  
  cout << "Starting grabber\n";
  interface->start ();
  cout << "Done\n";
  
  boost::shared_ptr<RangeImagePlanar> range_image_planar_ptr (new RangeImagePlanar);
  RangeImagePlanar& range_image_planar = *range_image_planar_ptr;
  
  RangeImageBorderExtractor range_image_border_extractor;
  NarfKeypoint narf_keypoint_detector;
  narf_keypoint_detector.setRangeImageBorderExtractor(&range_image_border_extractor);
  narf_keypoint_detector.getParameters().support_size = support_size;
  //narf_keypoint_detector.getParameters().add_points_on_straight_edges = true;
  //narf_keypoint_detector.getParameters().distance_for_additional_points = 0.5;
  
  PointCloud<PointXYZ>::Ptr keypoints_cloud_ptr(new PointCloud<PointXYZ>);
  PointCloud<PointXYZ>& keypoints_cloud = *keypoints_cloud_ptr;
  
  while (true)//!viewer.wasStopped () && range_image_widget.isShown ())
  {
    viewer.spinOnce (10);
    ImageWidgetWX::spinOnce ();  // process GUI events
    usleep (10000);
    
    bool got_new_range_image = false;
    if (received_new_depth_data && depth_image_mutex.try_lock ())
    {
      received_new_depth_data = false;
      
      //unsigned long time_stamp = depth_image_ptr->getTimeStamp ();
      //int frame_id = depth_image_ptr->getFrameID ();
      const unsigned short* depth_map = depth_image_ptr->getDepthMetaData().Data();
      int width = depth_image_ptr->getWidth (), height = depth_image_ptr->getHeight ();
      float center_x = width/2, center_y = height/2;
      float focal_length_x = depth_image_ptr->getFocalLength(), focal_length_y = focal_length_x;
      float original_angular_resolution = asinf (0.5f*float(width)/float(focal_length_x)) / (0.5f*float(width));
      float desired_angular_resolution = angular_resolution;
      range_image_planar.setDepthImage (depth_map, width, height, center_x, center_y,
                                        focal_length_x, focal_length_y, desired_angular_resolution);
      depth_image_mutex.unlock ();
      got_new_range_image = !range_image_planar.points.empty();
    }
    
    if (!got_new_range_image)
      continue;
    
    // --------------------------------
    // -----Extract NARF keypoints-----
    // --------------------------------
    if (set_unseen_to_max_range)
      range_image_planar.setUnseenToMaxRange();
    narf_keypoint_detector.setRangeImage(&range_image_planar);
    PointCloud<int> keypoint_indices;
    narf_keypoint_detector.compute(keypoint_indices);
    std::cout << "Found "<<keypoint_indices.points.size()<<" key points.\n";
    
    // ----------------------------------------------
    // -----Show keypoints in range image widget-----
    // ----------------------------------------------
    range_image_widget.setRangeImage (range_image_planar, 0.0f, 10.0f);
    for (size_t i=0; i<keypoint_indices.points.size(); ++i)
      range_image_widget.markPoint(keypoint_indices.points[i]%range_image_planar.width,
                                   keypoint_indices.points[i]/range_image_planar.width);
    
    // -------------------------------------
    // -----Show keypoints in 3D viewer-----
    // -------------------------------------
    PointCloudColorHandlerCustom<pcl::PointWithRange> color_handler_cloud (range_image_planar_ptr, 0, 0, 0);
    if (!viewer.updatePointCloud<pcl::PointWithRange> (range_image_planar_ptr, color_handler_cloud, "range image"))
      viewer.addPointCloud<pcl::PointWithRange> (range_image_planar_ptr, color_handler_cloud, "range image");
    
    keypoints_cloud.points.resize(keypoint_indices.points.size());
    for (size_t i=0; i<keypoint_indices.points.size(); ++i)
      keypoints_cloud.points[i].getVector3fMap() = range_image_planar.points[keypoint_indices.points[i]].getVector3fMap();
    PointCloudColorHandlerCustom<pcl::PointXYZ> color_handler_keypoints (keypoints_cloud_ptr, 0, 255, 0);
    if (!viewer.updatePointCloud(keypoints_cloud_ptr, color_handler_keypoints, "keypoints"))
      viewer.addPointCloud(keypoints_cloud_ptr, color_handler_keypoints, "keypoints");
    viewer.setPointCloudRenderingProperties(PCL_VISUALIZER_POINT_SIZE, 7, "keypoints");
  }

  interface->stop ();
}
