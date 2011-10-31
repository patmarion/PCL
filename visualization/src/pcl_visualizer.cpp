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

#include <pcl/common/common_headers.h>
#include <pcl/ros/conversions.h>
#include <pcl/visualization/common/common.h>
#include <pcl/visualization/pcl_visualizer.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkCellData.h>
#include <vtkWorldPointPicker.h>
#include <vtkPropPicker.h>
#include <vtkPlatonicSolidSource.h>
#include <vtkLoopSubdivisionFilter.h>
#include <vtkTriangle.h>
#include <vtkTransform.h>
#include <vtkVisibleCellSelector.h>
#include <vtkSelection.h>
// Only available in older versions of VTK
//#include <vtkHardwareSelector.h>
//#include <vtkSelectionNode.h>
#include <vtkPointPicker.h>

/////////////////////////////////////////////////////////////////////////////////////////////
pcl::visualization::PCLVisualizer::PCLVisualizer (const std::string &name, const bool create_interactor) :
    rens_ (vtkSmartPointer<vtkRendererCollection>::New ()),
    style_ (vtkSmartPointer<pcl::visualization::PCLVisualizerInteractorStyle>::New ()),
    cloud_actor_map_ (new CloudActorMap),
    shape_actor_map_ (new ShapeActorMap)
{
  // FPS callback
  vtkSmartPointer<vtkTextActor> txt = vtkSmartPointer<vtkTextActor>::New ();
  vtkSmartPointer<FPSCallback> update_fps = vtkSmartPointer<FPSCallback>::New ();
  update_fps->setTextActor (txt);

  // Create a Renderer
  vtkSmartPointer<vtkRenderer> ren = vtkSmartPointer<vtkRenderer>::New ();
  ren->AddObserver (vtkCommand::EndEvent, update_fps);
  ren->AddActor (txt);
  // Add it to the list of renderers
  rens_->AddItem (ren);

  // Create a RendererWindow
  win_ = vtkSmartPointer<vtkRenderWindow>::New ();
  win_->SetWindowName (name.c_str ());
  win_->AlphaBitPlanesOff ();
  win_->PointSmoothingOff ();
  win_->LineSmoothingOff ();
  win_->PolygonSmoothingOff ();
  win_->SwapBuffersOn ();
  win_->SetStereoTypeToAnaglyph ();

  // Get screen size
  int *scr_size = win_->GetScreenSize ();
  // Set the window size as 1/2 of the screen size
  win_->SetSize (scr_size[0] / 2, scr_size[1] / 2);

  // Add all renderers to the window
  rens_->InitTraversal ();
  vtkRenderer* renderer = NULL;
  while ((renderer = rens_->GetNextItem ()) != NULL)
    win_->AddRenderer (renderer);

  // Create the interactor style
  style_->Initialize ();
  style_->setRendererCollection (rens_);
  style_->setCloudActorMap (cloud_actor_map_);
  style_->UseTimersOn ();

  if (create_interactor) 
    createInteractor (); 

  win_->SetWindowName (name.c_str ());
}

/////////////////////////////////////////////////////////////////////////////////////////////
pcl::visualization::PCLVisualizer::PCLVisualizer (int &argc, char **argv, const std::string &name, PCLVisualizerInteractorStyle* style, const bool create_interactor) : 
    rens_ (vtkSmartPointer<vtkRendererCollection>::New ()), 
    cloud_actor_map_ (new CloudActorMap),
    shape_actor_map_ (new ShapeActorMap)
{
  style_ = style;

  // FPS callback
  vtkSmartPointer<vtkTextActor> txt = vtkSmartPointer<vtkTextActor>::New ();
  vtkSmartPointer<FPSCallback> update_fps = vtkSmartPointer<FPSCallback>::New ();
  update_fps->setTextActor (txt);

  // Create a Renderer
  vtkSmartPointer<vtkRenderer> ren = vtkSmartPointer<vtkRenderer>::New ();
  ren->AddObserver (vtkCommand::EndEvent, update_fps);
  ren->AddActor (txt);
  // Add it to the list of renderers
  rens_->AddItem (ren);
  
  // Create a RendererWindow
  win_ = vtkSmartPointer<vtkRenderWindow>::New ();
  win_->SetWindowName (name.c_str ());
  win_->AlphaBitPlanesOff ();
  win_->PointSmoothingOff ();
  win_->LineSmoothingOff ();
  win_->PolygonSmoothingOff ();
  win_->SwapBuffersOn ();
  win_->SetStereoTypeToAnaglyph ();

  // Get screen size
  int *scr_size = win_->GetScreenSize ();
  camera_.window_size[0] = scr_size[0]; camera_.window_size[1] = scr_size[1] / 2;
  camera_.window_pos[0] = camera_.window_pos[1] = 0;
 
  // Set default camera parameters
  initCameraParameters ();
 
  // Parse the camera settings and update the internal camera
  camera_set_ = getCameraParameters (argc, argv);
  updateCamera ();
  // Set the window size as 1/2 of the screen size or the user given parameter
  win_->SetSize (camera_.window_size[0], camera_.window_size[1]);
  win_->SetPosition (camera_.window_pos[0], camera_.window_pos[1]);
 
  // Add all renderers to the window
  rens_->InitTraversal ();
  vtkRenderer* renderer = NULL;
  while ((renderer = rens_->GetNextItem ()) != NULL)
    win_->AddRenderer (renderer);

  // Create the interactor style
  style_->Initialize ();
  style_->setRendererCollection (rens_);
  style_->setCloudActorMap (cloud_actor_map_);
  style_->UseTimersOn ();

  if (create_interactor) 
    createInteractor (); 
  
  win_->SetWindowName (name.c_str ());
}

/////////////////////////////////////////////////////////////////////////////////////////////
void 
pcl::visualization::PCLVisualizer::createInteractor ()
{
#if ((VTK_MAJOR_VERSION == 5) && (VTK_MINOR_VERSION <= 4))
   interactor_ = vtkSmartPointer<PCLVisualizerInteractor>::New ();
#else
  interactor_ = vtkSmartPointer<vtkRenderWindowInteractor>::New ();
#endif

  interactor_->SetRenderWindow (win_);
  interactor_->SetInteractorStyle (style_);
  //interactor_->SetStillUpdateRate (30.0);
  interactor_->SetDesiredUpdateRate (30.0);

  // Initialize and create timer, also create window
  interactor_->Initialize ();
#if ((VTK_MAJOR_VERSION == 5) && (VTK_MINOR_VERSION <= 4))
  interactor_->timer_id_ = interactor_->CreateRepeatingTimer (5000L);
#else
  timer_id_ = interactor_->CreateRepeatingTimer (5000L);
#endif
  // Set a simple PointPicker
  vtkSmartPointer<vtkPointPicker> pp = vtkSmartPointer<vtkPointPicker>::New ();
  pp->SetTolerance (pp->GetTolerance () * 2);
  interactor_->SetPicker (pp);

  exit_main_loop_timer_callback_ = vtkSmartPointer<ExitMainLoopTimerCallback>::New ();
  exit_main_loop_timer_callback_->pcl_visualizer = this;
  exit_main_loop_timer_callback_->right_timer_id = -1;
  interactor_->AddObserver (vtkCommand::TimerEvent, exit_main_loop_timer_callback_);

  exit_callback_ = vtkSmartPointer<ExitCallback>::New ();
  exit_callback_->pcl_visualizer = this;
  interactor_->AddObserver (vtkCommand::ExitEvent, exit_callback_);
  
  resetStoppedFlag ();
}

/////////////////////////////////////////////////////////////////////////////////////////////
pcl::visualization::PCLVisualizer::~PCLVisualizer ()
{
  if (interactor_ != NULL) 
#if ((VTK_MAJOR_VERSION == 5) && (VTK_MINOR_VERSION <= 4))
    interactor_->DestroyTimer (interactor_->timer_id_);
#else
    interactor_->DestroyTimer (timer_id_);
#endif
  // Clear the collections
  rens_->RemoveAllItems ();
}

/////////////////////////////////////////////////////////////////////////////////////////////
void
pcl::visualization::PCLVisualizer::saveScreenshot (const std::string &file)
{
  style_->saveScreenshot (file);
}

/////////////////////////////////////////////////////////////////////////////////////////////
boost::signals2::connection
pcl::visualization::PCLVisualizer::registerKeyboardCallback (boost::function<void (const pcl::visualization::KeyboardEvent&)> callback)
{
  return (style_->registerKeyboardCallback (callback));
}

/////////////////////////////////////////////////////////////////////////////////////////////
boost::signals2::connection
pcl::visualization::PCLVisualizer::registerMouseCallback (boost::function<void (const pcl::visualization::MouseEvent&)> callback)
{
  return (style_->registerMouseCallback (callback));
}

/////////////////////////////////////////////////////////////////////////////////////////////
boost::signals2::connection
pcl::visualization::PCLVisualizer::registerPointPickingCallback (boost::function<void (const pcl::visualization::PointPickingEvent&)> callback)
{
  return (style_->registerPointPickingCallback (callback));
}

/////////////////////////////////////////////////////////////////////////////////////////////
void
pcl::visualization::PCLVisualizer::spin ()
{
  resetStoppedFlag ();
  // Render the window before we start the interactor
  win_->Render ();
  interactor_->Start ();
}

/////////////////////////////////////////////////////////////////////////////////////////////
void
pcl::visualization::PCLVisualizer::spinOnce (int time, bool force_redraw)
{
  resetStoppedFlag ();

  if (time <= 0)
    time = 1;
  
  if (force_redraw)
  {
    interactor_->Render ();
    exit_main_loop_timer_callback_->right_timer_id = interactor_->CreateRepeatingTimer (time);
    interactor_->Start ();
    interactor_->DestroyTimer (exit_main_loop_timer_callback_->right_timer_id);
    return;
  }
  
  DO_EVERY(1.0/interactor_->GetDesiredUpdateRate (),
    interactor_->Render ();
    exit_main_loop_timer_callback_->right_timer_id = interactor_->CreateRepeatingTimer (time);
    interactor_->Start ();
    interactor_->DestroyTimer (exit_main_loop_timer_callback_->right_timer_id);
  );
}

/////////////////////////////////////////////////////////////////////////////////////////////
void
pcl::visualization::PCLVisualizer::addCoordinateSystem (double scale, int viewport)
{
  vtkSmartPointer<vtkAxes> axes = vtkSmartPointer<vtkAxes>::New ();
  axes->SetOrigin (0, 0, 0);
  axes->SetScaleFactor (scale);

  vtkSmartPointer<vtkFloatArray> axes_colors = vtkSmartPointer<vtkFloatArray>::New ();
  axes_colors->Allocate (6);
  axes_colors->InsertNextValue (0.0);
  axes_colors->InsertNextValue (0.0);
  axes_colors->InsertNextValue (0.5);
  axes_colors->InsertNextValue (0.5);
  axes_colors->InsertNextValue (1.0);
  axes_colors->InsertNextValue (1.0);

  vtkSmartPointer<vtkPolyData> axes_data = axes->GetOutput ();
  axes_data->Update ();
  axes_data->GetPointData ()->SetScalars (axes_colors);

  vtkSmartPointer<vtkTubeFilter> axes_tubes = vtkSmartPointer<vtkTubeFilter>::New ();
  axes_tubes->SetInput (axes_data);
  axes_tubes->SetRadius (axes->GetScaleFactor () / 50.0);
  axes_tubes->SetNumberOfSides (6);

  vtkSmartPointer<vtkPolyDataMapper> axes_mapper = vtkSmartPointer<vtkPolyDataMapper>::New ();
  axes_mapper->SetScalarModeToUsePointData ();
  axes_mapper->SetInput (axes_tubes->GetOutput ());

  vtkSmartPointer<vtkLODActor> axes_actor = vtkSmartPointer<vtkLODActor>::New ();
  axes_actor->SetMapper (axes_mapper);

  // Save the ID and actor pair to the global actor map
  coordinate_actor_map_[viewport] = axes_actor;

  addActorToRenderer (axes_actor, viewport);
}

/////////////////////////////////////////////////////////////////////////////////////////////
void
pcl::visualization::PCLVisualizer::addCoordinateSystem (double scale, float x, float y, float z, int viewport)
{
  vtkSmartPointer<vtkAxes> axes = vtkSmartPointer<vtkAxes>::New ();
  axes->SetOrigin (0, 0, 0);
  axes->SetScaleFactor (scale);

  vtkSmartPointer<vtkFloatArray> axes_colors = vtkSmartPointer<vtkFloatArray>::New ();
  axes_colors->Allocate (6);
  axes_colors->InsertNextValue (0.0);
  axes_colors->InsertNextValue (0.0);
  axes_colors->InsertNextValue (0.5);
  axes_colors->InsertNextValue (0.5);
  axes_colors->InsertNextValue (1.0);
  axes_colors->InsertNextValue (1.0);

  vtkSmartPointer<vtkPolyData> axes_data = axes->GetOutput ();
  axes_data->Update ();
  axes_data->GetPointData ()->SetScalars (axes_colors);

  vtkSmartPointer<vtkTubeFilter> axes_tubes = vtkSmartPointer<vtkTubeFilter>::New ();
  axes_tubes->SetInput (axes_data);
  axes_tubes->SetRadius (axes->GetScaleFactor () / 50.0);
  axes_tubes->SetNumberOfSides (6);

  vtkSmartPointer<vtkPolyDataMapper> axes_mapper = vtkSmartPointer<vtkPolyDataMapper>::New ();
  axes_mapper->SetScalarModeToUsePointData ();
  axes_mapper->SetInput (axes_tubes->GetOutput ());

  vtkSmartPointer<vtkLODActor> axes_actor = vtkSmartPointer<vtkLODActor>::New ();
  axes_actor->SetMapper (axes_mapper);
  axes_actor->SetPosition (x, y, z);

  // Save the ID and actor pair to the global actor map
  coordinate_actor_map_[viewport] = axes_actor;

  addActorToRenderer (axes_actor, viewport);
}

/////////////////////////////////////////////////////////////////////////////////////////////
void
pcl::visualization::PCLVisualizer::addCoordinateSystem (double scale, const Eigen::Matrix4f& t, int viewport)
{
  vtkSmartPointer<vtkAxes> axes = vtkSmartPointer<vtkAxes>::New ();
  axes->SetOrigin (0, 0, 0);
  axes->SetScaleFactor (scale);

  vtkSmartPointer<vtkFloatArray> axes_colors = vtkSmartPointer<vtkFloatArray>::New ();
  axes_colors->Allocate (6);
  axes_colors->InsertNextValue (0.0);
  axes_colors->InsertNextValue (0.0);
  axes_colors->InsertNextValue (0.5);
  axes_colors->InsertNextValue (0.5);
  axes_colors->InsertNextValue (1.0);
  axes_colors->InsertNextValue (1.0);

  vtkSmartPointer<vtkPolyData> axes_data = axes->GetOutput ();
  axes_data->Update ();
  axes_data->GetPointData ()->SetScalars (axes_colors);

  vtkSmartPointer<vtkTubeFilter> axes_tubes = vtkSmartPointer<vtkTubeFilter>::New ();
  axes_tubes->SetInput (axes_data);
  axes_tubes->SetRadius (axes->GetScaleFactor () / 50.0);
  axes_tubes->SetNumberOfSides (6);

  vtkSmartPointer<vtkPolyDataMapper> axes_mapper = vtkSmartPointer<vtkPolyDataMapper>::New ();
  axes_mapper->SetScalarModeToUsePointData ();
  axes_mapper->SetInput (axes_tubes->GetOutput ());

  vtkSmartPointer<vtkLODActor> axes_actor = vtkSmartPointer<vtkLODActor>::New ();
  axes_actor->SetMapper (axes_mapper);

  // All Input values are radian.
  // To avoid singularity when inversion from Rotation matrix to angles.
  double pitch,yaw,roll;
  // pitch = 90 degree
  if (t(1,0) > 0.998)
  { // singularity at north pole
    pitch = atan2 (t (0, 2), t (2, 2));
    yaw = M_PI/2.0;
    roll = 0;
  }
  else
  {
    pitch = atan2(-t(2,0),t(0,0));
    roll = atan2(-t(1,2),t(1,1));
    yaw = asin(t(1,0));
  }

  // pitch = -90 degree
  if (t (1, 0) < -0.998)
  { // singularity at south pole
    pitch = atan2(t(0,2),t(2,2));
    yaw = -M_PI/2.0;
    roll = 0;
  }
  else
  {
    pitch = atan2(-t(2,0),t(0,0));
    roll = atan2(-t(1,2),t(1,1));
    yaw = asin(t(1,0));
  }

  // Convert from radian to degree.
  pitch = pitch * 180.0 / (double)M_PI;
  roll  = roll  * 180.0 / (double)M_PI;
  yaw   = yaw   * 180.0 / (double)M_PI;

  axes_actor->SetPosition (t (0, 3), t(1, 3), t(2, 3));
  axes_actor->SetOrientation (roll, pitch, yaw);

  // Save the ID and actor pair to the global actor map
  coordinate_actor_map_[viewport] = axes_actor;
  addActorToRenderer (axes_actor, viewport);
}

/////////////////////////////////////////////////////////////////////////////////////////////
bool
pcl::visualization::PCLVisualizer::removeCoordinateSystem (int viewport)
{
  // Check to see if the given ID entry exists
  CoordinateActorMap::iterator am_it = coordinate_actor_map_.find (viewport);

  if (am_it == coordinate_actor_map_.end ())
    return (false);

  // Remove it from all renderers
  removeActorFromRenderer (am_it->second, viewport);
  // Remove the ID pair to the global actor map
  coordinate_actor_map_.erase (am_it);
  return (true);
}

/////////////////////////////////////////////////////////////////////////////////////////////
bool
pcl::visualization::PCLVisualizer::removePointCloud (const std::string &id, int viewport)
{
  // Check to see if the given ID entry exists
  CloudActorMap::iterator am_it = cloud_actor_map_->find (id);

  if (am_it == cloud_actor_map_->end ())
    return (false);

  // Remove it from all renderers
  removeActorFromRenderer (am_it->second.actor, viewport);

  // Remove the pointer/ID pair to the global actor map
  cloud_actor_map_->erase (am_it);
  return (true);
}

/////////////////////////////////////////////////////////////////////////////////////////////
bool
pcl::visualization::PCLVisualizer::removeShape (const std::string &id, int viewport)
{
  // Check to see if the given ID entry exists
  ShapeActorMap::iterator am_it = shape_actor_map_->find (id);

  if (am_it == shape_actor_map_->end ())
  {
    //pcl::console::print_warn ("[removeSape] Could not find any shape with id <%s>!\n", id.c_str ());
    return (false);
  }

  // Remove it from all renderers
  removeActorFromRenderer (am_it->second, viewport);

  // Remove the pointer/ID pair to the global actor map
  shape_actor_map_->erase (am_it);
  return (true);
}

/////////////////////////////////////////////////////////////////////////////////////////////
bool
pcl::visualization::PCLVisualizer::removeText3D (const std::string &id, int viewport)
{
  // Check to see if the given ID entry exists
  ShapeActorMap::iterator am_it = shape_actor_map_->find (id);

  if (am_it == shape_actor_map_->end ())
  {
    //pcl::console::print_warn ("[removeSape] Could not find any shape with id <%s>!\n", id.c_str ());
    return (false);
  }

  // Remove it from all renderers
  removeActorFromRenderer (am_it->second, viewport);

  // Remove the pointer/ID pair to the global actor map
  shape_actor_map_->erase (am_it);
  return (true);
}

/////////////////////////////////////////////////////////////////////////////////////////////
bool 
pcl::visualization::PCLVisualizer::removeAllPointClouds (int viewport) 
{ 
  // Check to see if the given ID entry exists 
  CloudActorMap::iterator am_it = cloud_actor_map_->begin ();
  while (am_it != cloud_actor_map_->end () )
  {  
    removePointCloud (am_it->first, viewport);
    am_it = cloud_actor_map_->begin ();
  }
  return (true); 
}

/////////////////////////////////////////////////////////////////////////////////////////////
bool 
pcl::visualization::PCLVisualizer::removeAllShapes (int viewport) 
{ 
  // Check to see if the given ID entry exists 
  ShapeActorMap::iterator am_it = shape_actor_map_->begin ();  
  while (am_it != shape_actor_map_->end ())
  { 
    removeShape (am_it->first, viewport); 
    am_it = shape_actor_map_->begin ();
  } 
  return (true); 
}

/////////////////////////////////////////////////////////////////////////////////////////////
bool
pcl::visualization::PCLVisualizer::addPointCloudPrincipalCurvatures (const pcl::PointCloud<pcl::PointXYZ>::ConstPtr &cloud, 
                                                                    const pcl::PointCloud<pcl::Normal>::ConstPtr &normals,
                                                                    const pcl::PointCloud<pcl::PrincipalCurvatures>::ConstPtr &pcs,
                                                                    int level, double scale,
                                                                    const std::string &id, int viewport)
{
  if (pcs->points.size () != cloud->points.size () || normals->points.size () != cloud->points.size ())
  {
    pcl::console::print_error ("[addPointCloudPrincipalCurvatures] The number of points differs from the number of principal curvatures/normals!\n");
    return (false);
  }
  // Check to see if this ID entry already exists (has it been already added to the visualizer?)
  CloudActorMap::iterator am_it = cloud_actor_map_->find (id);

  if (am_it != cloud_actor_map_->end ())
  {
    pcl::console::print_warn ("[addPointCloudPrincipalCurvatures] A PointCloud with id <%s> already exists! Please choose a different id and retry.\n", id.c_str ());
    return (false);
  }

  vtkSmartPointer<vtkAppendPolyData> polydata_1 = vtkSmartPointer<vtkAppendPolyData>::New ();
  vtkSmartPointer<vtkAppendPolyData> polydata_2 = vtkSmartPointer<vtkAppendPolyData>::New ();
 
  // Setup two colors - one for each line
  unsigned char green[3] = {0, 255, 0};
  unsigned char blue[3] = {0, 0, 255};
 
  // Setup the colors array
  vtkSmartPointer<vtkUnsignedCharArray> line_1_colors =vtkSmartPointer<vtkUnsignedCharArray>::New ();
  line_1_colors->SetNumberOfComponents (3);
  line_1_colors->SetName ("Colors"); 
  vtkSmartPointer<vtkUnsignedCharArray> line_2_colors =vtkSmartPointer<vtkUnsignedCharArray>::New ();
  line_2_colors->SetNumberOfComponents (3);
  line_2_colors->SetName ("Colors"); 

  // Create the first sets of lines
  for (size_t i = 0; i < cloud->points.size (); i+=level)
  {
    pcl::PointXYZ p = cloud->points[i];
    p.x += (pcs->points[i].pc1 * pcs->points[i].principal_curvature[0]) * scale;
    p.y += (pcs->points[i].pc1 * pcs->points[i].principal_curvature[1]) * scale;
    p.z += (pcs->points[i].pc1 * pcs->points[i].principal_curvature[2]) * scale;

    vtkSmartPointer<vtkLineSource> line_1 = vtkSmartPointer<vtkLineSource>::New ();
    line_1->SetPoint1 (cloud->points[i].x, cloud->points[i].y, cloud->points[i].z);
    line_1->SetPoint2 (p.x, p.y, p.z);
    line_1->Update ();
    polydata_1->AddInput (line_1->GetOutput ());
    line_1_colors->InsertNextTupleValue (green);
  }
  polydata_1->Update (); 
  vtkSmartPointer<vtkPolyData> line_1_data = polydata_1->GetOutput ();
  line_1_data->GetCellData ()->SetScalars (line_1_colors);
  
  // Create the second sets of lines
  for (size_t i = 0; i < cloud->points.size (); i += level)
  {
    Eigen::Vector3f pc (pcs->points[i].principal_curvature[0], 
                        pcs->points[i].principal_curvature[1], 
                        pcs->points[i].principal_curvature[2]);
    Eigen::Vector3f normal (normals->points[i].normal[0], 
                            normals->points[i].normal[1], 
                            normals->points[i].normal[2]); 
    Eigen::Vector3f pc_c = pc.cross (normal);

    pcl::PointXYZ p = cloud->points[i];
    p.x += (pcs->points[i].pc2 * pc_c[0]) * scale;
    p.y += (pcs->points[i].pc2 * pc_c[1]) * scale;
    p.z += (pcs->points[i].pc2 * pc_c[2]) * scale;

    vtkSmartPointer<vtkLineSource> line_2 = vtkSmartPointer<vtkLineSource>::New ();
    line_2->SetPoint1 (cloud->points[i].x, cloud->points[i].y, cloud->points[i].z);
    line_2->SetPoint2 (p.x, p.y, p.z);
    line_2->Update ();
    polydata_2->AddInput (line_2->GetOutput ());
    line_2_colors->InsertNextTupleValue (blue);
  }
  polydata_2->Update (); 
  vtkSmartPointer<vtkPolyData> line_2_data = polydata_2->GetOutput ();
  line_2_data->GetCellData ()->SetScalars (line_2_colors);

  // Assemble the two sets of lines
  vtkSmartPointer<vtkAppendPolyData> alldata = vtkSmartPointer<vtkAppendPolyData>::New ();
  alldata->AddInput (line_1_data);
  alldata->AddInput (line_2_data);

  // Create an Actor
  vtkSmartPointer<vtkLODActor> actor;
  createActorFromVTKDataSet (alldata->GetOutput (), actor);
  actor->GetMapper ()->SetScalarModeToUseCellData ();

  // Add it to all renderers
  addActorToRenderer (actor, viewport);

  // Save the pointer/ID pair to the global actor map
  CloudActor act;
  act.actor = actor;
  (*cloud_actor_map_)[id] = act;
  return (true);
}

/////////////////////////////////////////////////////////////////////////////////////////////
void
pcl::visualization::PCLVisualizer::removeActorFromRenderer (const vtkSmartPointer<vtkLODActor> &actor, int viewport)
{
  // Add it to all renderers
  rens_->InitTraversal ();
  vtkRenderer* renderer = NULL;
  int i = 1;
  while ((renderer = rens_->GetNextItem ()) != NULL)
  {
    // Should we add the actor to all renderers?
    if (viewport == 0)
    {
      renderer->RemoveActor (actor);
      renderer->Render ();
    }
    else if (viewport == i)               // add the actor only to the specified viewport
    {
      renderer->RemoveActor (actor);
      renderer->Render ();
    }
    ++i;
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////
void
pcl::visualization::PCLVisualizer::addActorToRenderer (const vtkSmartPointer<vtkProp> &actor, int viewport)
{
  // Add it to all renderers
  rens_->InitTraversal ();
  vtkRenderer* renderer = NULL;
  int i = 1;
  while ((renderer = rens_->GetNextItem ()) != NULL)
  {
    // Should we add the actor to all renderers?
    if (viewport == 0)
    {
      renderer->AddActor (actor);
      renderer->Render ();
    }
    else if (viewport == i)               // add the actor only to the specified viewport
    {
      renderer->AddActor (actor);
      renderer->Render ();
    }
    ++i;
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////
void
pcl::visualization::PCLVisualizer::removeActorFromRenderer (const vtkSmartPointer<vtkProp> &actor, int viewport)
{
  // Add it to all renderers
  rens_->InitTraversal ();
  vtkRenderer* renderer = NULL;
  int i = 1;
  while ((renderer = rens_->GetNextItem ()) != NULL)
  {
    // Should we add the actor to all renderers?
    if (viewport == 0)
    {
      renderer->RemoveActor (actor);
      renderer->Render ();
    }
    else if (viewport == i)               // add the actor only to the specified viewport
    {
      renderer->RemoveActor (actor);
      renderer->Render ();
    }
    ++i;
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////
void
pcl::visualization::PCLVisualizer::createActorFromVTKDataSet (const vtkSmartPointer<vtkDataSet> &data, 
                                                              vtkSmartPointer<vtkLODActor> &actor)
{
  // If actor is not initialized, initialize it here
  if (!actor)
    actor = vtkSmartPointer<vtkLODActor>::New ();

  vtkSmartPointer<vtkDataArray> scalars = data->GetPointData ()->GetScalars ();
  double minmax[2];
  if (scalars)
    scalars->GetRange (minmax);

  vtkSmartPointer<vtkDataSetMapper> mapper = vtkSmartPointer<vtkDataSetMapper>::New ();      
  mapper->SetInput (data);
  if (scalars)
    mapper->SetScalarRange (minmax);
  mapper->SetScalarModeToUsePointData ();
  mapper->InterpolateScalarsBeforeMappingOn ();
  mapper->ScalarVisibilityOn ();
  mapper->ImmediateModeRenderingOff ();

  actor->SetNumberOfCloudPoints (data->GetNumberOfPoints () / 10);
  actor->GetProperty ()->SetInterpolationToFlat ();

  actor->SetMapper (mapper);
}

/////////////////////////////////////////////////////////////////////////////////////////////
void 
pcl::visualization::PCLVisualizer::convertPointCloudToVTKPolyData (
    const GeometryHandlerConstPtr &geometry_handler, 
    vtkSmartPointer<vtkPolyData> &polydata,
    vtkSmartPointer<vtkIdTypeArray> &initcells)
{
  vtkSmartPointer<vtkCellArray> vertices;

  if (!polydata)
  {
    allocVtkPolyData (polydata);
    vertices = vtkSmartPointer<vtkCellArray>::New ();
    polydata->SetVerts (vertices);
  }

  // Use the handler to obtain the geometry
  vtkSmartPointer<vtkPoints> points;
  geometry_handler->getGeometry (points);
  polydata->SetPoints (points);

  vtkIdType nr_points = points->GetNumberOfPoints ();

  // Create the supporting structures
  vertices = polydata->GetVerts ();
  if (!vertices)
    vertices = vtkSmartPointer<vtkCellArray>::New ();

  vtkSmartPointer<vtkIdTypeArray> cells = vertices->GetData ();
  updateCells (cells, initcells, nr_points);
  // Set the cells and the vertices
  vertices->SetCells (nr_points, cells);
}

//////////////////////////////////////////////////////////////////////////////////////////////
void 
pcl::visualization::PCLVisualizer::setBackgroundColor (
    const double &r, const double &g, const double &b, int viewport)
{
  rens_->InitTraversal ();
  vtkRenderer* renderer = NULL;
  int i = 1;
  while ((renderer = rens_->GetNextItem ()) != NULL)
  {
    // Should we add the actor to all renderers?
    if (viewport == 0)
    {
      renderer->SetBackground (r, g, b);
      renderer->Render ();
    }
    else if (viewport == i)               // add the actor only to the specified viewport
    {
      renderer->SetBackground (r, g, b);
      renderer->Render ();
    }
    ++i;
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////
bool 
pcl::visualization::PCLVisualizer::setPointCloudRenderingProperties (
    int property, double val1, double val2, double val3, const std::string &id, int viewport)
{
  // Check to see if this ID entry already exists (has it been already added to the visualizer?)
  CloudActorMap::iterator am_it = cloud_actor_map_->find (id);

  if (am_it == cloud_actor_map_->end ())
  {
    pcl::console::print_error ("[setPointCloudRenderingProperties] Could not find any PointCloud datasets with id <%s>!\n", id.c_str ());
    return (false);
  }
  // Get the actor pointer
  vtkLODActor* actor = vtkLODActor::SafeDownCast (am_it->second.actor);

  switch (property)
  {
    case PCL_VISUALIZER_COLOR:
    {
      actor->GetProperty ()->SetColor (val1, val2, val3);
      actor->Modified ();
      break;
    }
    default:
    {
      pcl::console::print_error ("[setPointCloudRenderingProperties] Unknown property (%d) specified!\n", property);
      return (false);
    }
  }
  return (true);
}

/////////////////////////////////////////////////////////////////////////////////////////////
bool 
pcl::visualization::PCLVisualizer::getPointCloudRenderingProperties (int property, double &value, const std::string &id)
{
  // Check to see if this ID entry already exists (has it been already added to the visualizer?)
  CloudActorMap::iterator am_it = cloud_actor_map_->find (id);

  if (am_it == cloud_actor_map_->end ())
    return (false);
  // Get the actor pointer
  vtkLODActor* actor = vtkLODActor::SafeDownCast (am_it->second.actor);

  switch (property)
  {
    case PCL_VISUALIZER_POINT_SIZE:
    {
      value = actor->GetProperty ()->GetPointSize ();
      actor->Modified ();
      break;
    }
    case PCL_VISUALIZER_OPACITY:
    {
      value = actor->GetProperty ()->GetOpacity ();
      actor->Modified ();
      break;
    }
    case PCL_VISUALIZER_LINE_WIDTH:
    {
      value = actor->GetProperty ()->GetLineWidth ();
      actor->Modified ();
      break;
    }
    default:
    {
      pcl::console::print_error ("[getPointCloudRenderingProperties] Unknown property (%d) specified!\n", property);
      return (false);
    }
  }
  return (true);
}

/////////////////////////////////////////////////////////////////////////////////////////////
bool 
pcl::visualization::PCLVisualizer::setPointCloudRenderingProperties (
    int property, double value, const std::string &id, int viewport)
{
  // Check to see if this ID entry already exists (has it been already added to the visualizer?)
  CloudActorMap::iterator am_it = cloud_actor_map_->find (id);

  if (am_it == cloud_actor_map_->end ())
  {
    pcl::console::print_error ("[setPointCloudRenderingProperties] Could not find any PointCloud datasets with id <%s>!\n", id.c_str ());
    return (false);
  }
  // Get the actor pointer
  vtkLODActor* actor = vtkLODActor::SafeDownCast (am_it->second.actor);

  switch (property)
  {
    case PCL_VISUALIZER_POINT_SIZE:
    {
      actor->GetProperty ()->SetPointSize (value);
      actor->Modified ();
      break;
    }
    case PCL_VISUALIZER_OPACITY:
    {
      actor->GetProperty ()->SetOpacity (value);
      actor->Modified ();
      break;
    }
    case PCL_VISUALIZER_LINE_WIDTH:
    {
      actor->GetProperty ()->SetLineWidth (value);
      actor->Modified ();
      break;
    }
    default:
    {
      pcl::console::print_error ("[setPointCloudRenderingProperties] Unknown property (%d) specified!\n", property);
      return (false);
    }
  }
  return (true);
}

/////////////////////////////////////////////////////////////////////////////////////////////
bool 
pcl::visualization::PCLVisualizer::setShapeRenderingProperties (
    int property, double val1, double val2, double val3, const std::string &id, int viewport)
{
  // Check to see if this ID entry already exists (has it been already added to the visualizer?)
  ShapeActorMap::iterator am_it = shape_actor_map_->find (id);

  if (am_it == shape_actor_map_->end ())
  {
    pcl::console::print_error ("[setShapeRenderingProperties] Could not find any shape with id <%s>!\n", id.c_str ());
    return (false);
  }
  // Get the actor pointer
  vtkLODActor* actor = vtkLODActor::SafeDownCast (am_it->second);

  switch (property)
  {
    case PCL_VISUALIZER_COLOR:
    {
      actor->GetProperty ()->SetColor (val1, val2, val3);
      actor->GetProperty ()->SetAmbientColor  (val1, val2, val3);
      actor->GetProperty ()->SetSpecularColor (val1, val2, val3);
      actor->GetProperty ()->SetAmbient (0.8);
      actor->Modified ();
      break;
    }
    default:
    {
      pcl::console::print_error ("[setShapeRenderingProperties] Unknown property (%d) specified!\n", property);
      return (false);
    }
  }
  return (true);
}

/////////////////////////////////////////////////////////////////////////////////////////////
bool
pcl::visualization::PCLVisualizer::setShapeRenderingProperties (
    int property, double value, const std::string &id, int viewport)
{
  // Check to see if this ID entry already exists (has it been already added to the visualizer?)
  ShapeActorMap::iterator am_it = shape_actor_map_->find (id);

  if (am_it == shape_actor_map_->end ())
  {
    pcl::console::print_error ("[setShapeRenderingProperties] Could not find any shape with id <%s>!\n", id.c_str ());
    return (false);
  }
  // Get the actor pointer
  vtkLODActor* actor = vtkLODActor::SafeDownCast (am_it->second);

  switch (property)
  {
    case PCL_VISUALIZER_POINT_SIZE:
    {
      actor->GetProperty ()->SetPointSize (value);
      actor->Modified ();
      break;
    }
    case PCL_VISUALIZER_OPACITY:
    {
      actor->GetProperty ()->SetOpacity (value);
      actor->Modified ();
      break;
    }
    case PCL_VISUALIZER_LINE_WIDTH:
    {
      actor->GetProperty ()->SetLineWidth (value);
      actor->Modified ();
      break;
    }
    case PCL_VISUALIZER_FONT_SIZE:
    {
      vtkTextActor* text_actor = vtkTextActor::SafeDownCast (am_it->second);
      vtkSmartPointer<vtkTextProperty> tprop = text_actor->GetTextProperty ();
      tprop->SetFontSize (value);
      text_actor->Modified ();
      break;
    }
    case PCL_VISUALIZER_REPRESENTATION:
    {
      switch ((int)value)
      {
        case PCL_VISUALIZER_REPRESENTATION_POINTS:
        {
          actor->GetProperty ()->SetRepresentationToPoints ();
          break;
        }
        case PCL_VISUALIZER_REPRESENTATION_WIREFRAME:
        {
          actor->GetProperty ()->SetRepresentationToWireframe ();
          break;
        }
        case PCL_VISUALIZER_REPRESENTATION_SURFACE:
        {
          actor->GetProperty ()->SetRepresentationToSurface ();
          break;
        }
      }
      actor->Modified ();
      break;
    }
    default:
    {
      pcl::console::print_error ("[setShapeRenderingProperties] Unknown property (%d) specified!\n", property);
      return (false);
    }
  }
  return (true);
}

/////////////////////////////////////////////////////////////////////////////////////////////
void
pcl::visualization::PCLVisualizer::initCameraParameters ()
{
  // Set default camera parameters
  camera_.clip[0] = 0.01; camera_.clip[1] = 1000.01;
  camera_.focal[0] = camera_.focal[1] = camera_.focal[2] = 0;
  camera_.pos[0] = camera_.pos[1] = 0; camera_.pos[2] = 1;
  camera_.view[0] = camera_.view[2] = 0; camera_.view[1] = 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////
bool
pcl::visualization::PCLVisualizer::cameraParamsSet () const
{
  return (camera_set_);
}

/////////////////////////////////////////////////////////////////////////////////////////////
void
pcl::visualization::PCLVisualizer::updateCamera ()
{
   // Update the camera parameters
   rens_->InitTraversal ();
   vtkRenderer* renderer = NULL;
   while ((renderer = rens_->GetNextItem ()) != NULL)
   {
     renderer->GetActiveCamera ()->SetPosition (camera_.pos);
     renderer->GetActiveCamera ()->SetFocalPoint (camera_.focal);
     renderer->GetActiveCamera ()->SetViewUp (camera_.view);
     renderer->GetActiveCamera ()->SetClippingRange (camera_.clip);
   }
}

/////////////////////////////////////////////////////////////////////////////////////////////
void
pcl::visualization::PCLVisualizer::getCameras (std::vector<pcl::visualization::Camera>& cameras)
{
  cameras.clear();
  rens_->InitTraversal ();
  vtkRenderer* renderer = NULL;
  while ((renderer = rens_->GetNextItem ()) != NULL)
  {
    cameras.push_back(Camera());
    cameras.back ().pos[0] = renderer->GetActiveCamera ()->GetPosition ()[0];
    cameras.back ().pos[1] = renderer->GetActiveCamera ()->GetPosition ()[1];
    cameras.back ().pos[2] = renderer->GetActiveCamera ()->GetPosition ()[2];
    cameras.back ().focal[0] = renderer->GetActiveCamera ()->GetFocalPoint ()[0];
    cameras.back ().focal[1] = renderer->GetActiveCamera ()->GetFocalPoint ()[1];
    cameras.back ().focal[2] = renderer->GetActiveCamera ()->GetFocalPoint ()[2];
    cameras.back ().clip[0] = renderer->GetActiveCamera ()->GetClippingRange ()[0];
    cameras.back ().clip[1] = renderer->GetActiveCamera ()->GetClippingRange ()[1];
    cameras.back ().view[0] = renderer->GetActiveCamera ()->GetViewUp ()[0];
    cameras.back ().view[1] = renderer->GetActiveCamera ()->GetViewUp ()[1];
    cameras.back ().view[2] = renderer->GetActiveCamera ()->GetViewUp ()[2];
    cameras.back().fovy = renderer->GetActiveCamera ()->GetViewAngle () / 180.0 * M_PI;
    cameras.back ().window_size[0] = renderer->GetRenderWindow ()->GetSize()[0];
    cameras.back ().window_size[1] = renderer->GetRenderWindow ()->GetSize()[1];
    cameras.back ().window_pos[0] = 0;
    cameras.back ().window_pos[1] = 0;
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////
Eigen::Affine3f
pcl::visualization::PCLVisualizer::getViewerPose ()
{
  Eigen::Affine3f ret(Eigen::Affine3f::Identity());
  if (rens_->GetNumberOfItems () < 1)
    return ret;
  vtkCamera& camera = *rens_->GetFirstRenderer ()->GetActiveCamera ();
  Eigen::Vector3d pos, x_axis, y_axis, z_axis;
  camera.GetPosition (pos[0], pos[1], pos[2]);
  camera.GetViewPlaneNormal (x_axis[0], x_axis[1], x_axis[2]);
  x_axis = -x_axis;
  camera.GetViewUp (z_axis[0], z_axis[1], z_axis[2]);
  y_axis = z_axis.cross(x_axis).normalized();
  ret(0,0)=x_axis[0], ret(0,1)=y_axis[0], ret(0,2)=z_axis[0], ret(0,3)=pos[0],
  ret(1,0)=x_axis[1], ret(1,1)=y_axis[1], ret(1,2)=z_axis[1], ret(1,3)=pos[1],
  ret(2,0)=x_axis[2], ret(2,1)=y_axis[2], ret(2,2)=z_axis[2], ret(2,3)=pos[2];
  return ret;
}

/////////////////////////////////////////////////////////////////////////////////////////////
void
pcl::visualization::PCLVisualizer::resetCamera ()
{
  // Update the camera parameters
  rens_->InitTraversal ();
  vtkRenderer* renderer = NULL;
  while ((renderer = rens_->GetNextItem ()) != NULL)
  {
    renderer->ResetCamera ();
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////
void
pcl::visualization::PCLVisualizer::setCameraPosition (double posX,double posY, double posZ,
                   double viewX, double viewY, double viewZ)
{
    // Set position
    camera_.pos[0] = posX;
    camera_.pos[1] = posY;
    camera_.pos[2] = posZ;

    // Set "rotation"
    camera_.view[0] = viewX;
    camera_.view[1] = viewY;
    camera_.view[2] = viewZ;

    updateCamera();
    //Render one frame from the new camera position
    spinOnce();
}

/////////////////////////////////////////////////////////////////////////////////////////////
void
pcl::visualization::PCLVisualizer::resetCameraViewpoint (const std::string &id)
{
  // Check to see if the given ID entry exists
  CloudActorMap::iterator am_it = cloud_actor_map_->find (id);

  if (am_it == cloud_actor_map_->end ())
    return;

  // Get all the data
  double bounds[6];
  vtkPolyDataMapper *mapper = reinterpret_cast<vtkPolyDataMapper*>(am_it->second.actor->GetMapper ());
  if (!mapper)
    return;
  vtkPolyData *data = mapper->GetInput ();
  if (!data)
    return;
  data->GetBounds (bounds);

  double focal[3];
  focal[0] = (bounds[0] + bounds[1]) / 2.0;
  focal[1] = (bounds[2] + bounds[3]) / 2.0;
  focal[2] = (bounds[4] + bounds[5]) / 2.0;

  // Update the camera parameters
  rens_->InitTraversal ();
  vtkRenderer* renderer = NULL;
  while ((renderer = rens_->GetNextItem ()) != NULL)
  {
    vtkSmartPointer<vtkCamera> cam = renderer->GetActiveCamera ();
    double view[3];
    cam->SetFocalPoint (focal);
    cam->SetPosition (0 - .25 * focal[0], 0 - .25 * focal[1], 0 - .25 * focal[2]);
    cam->GetViewUp (view);

    // Dataset negative on Z?
    if (focal[2] > 0)
      for (int i = 0; i < 3; i++) view[i] *= -1;
    cam->SetViewUp (view[0], view[1], view[2]);
    renderer->SetActiveCamera (cam);
    renderer->ResetCameraClippingRange (bounds);
    renderer->Render ();
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////
bool
pcl::visualization::PCLVisualizer::getCameraParameters (int argc, char **argv)
{
  for (int i = 1; i < argc; i++)
  {
    if ((strcmp (argv[i], "-cam") == 0) && (++i < argc))
    {
      std::ifstream fs;
      std::string camfile = std::string (argv[i]);
      std::string line;
      
      std::vector<std::string> camera;
      if (camfile.find (".cam") == std::string::npos)
      {
        // Assume we have clip/focal/pos/view
        boost::split (camera, argv[i], boost::is_any_of ("/"), boost::token_compress_on);
      }
      else
      {
        // Assume that if we don't have clip/focal/pos/view, a filename.cam was given as a parameter
        fs.open (camfile.c_str ());
        while (!fs.eof ())
        {
          getline (fs, line);
          if (line == "")
            continue;

          boost::split (camera, line, boost::is_any_of ("/"), boost::token_compress_on);
          break;
        }
        fs.close ();
      }

      // look for '/' as a separator
      if (camera.size () != 7)
      {
        pcl::console::print_error ("[PCLVisualizer::getCameraParameters] Camera parameters given, but with an invalid number of options (%lu vs 7)!\n", (unsigned long)camera.size ());
        return (false);
      }

      std::string clip_str  = camera.at (0);
      std::string focal_str = camera.at (1);
      std::string pos_str   = camera.at (2);
      std::string view_str  = camera.at (3);
      std::string fovy_str  = camera.at (4);
      std::string win_size_str = camera.at (5);
      std::string win_pos_str  = camera.at (6);

      // Get each camera setting separately and parse for ','
      std::vector<std::string> clip_st;
      boost::split (clip_st, clip_str, boost::is_any_of (","), boost::token_compress_on);
      if (clip_st.size () != 2)
      {
        pcl::console::print_error ("[PCLVisualizer::getCameraParameters] Invalid parameters given for camera clipping angle!\n");
        return (false);
      }
      camera_.clip[0] = atof (clip_st.at (0).c_str ());
      camera_.clip[1] = atof (clip_st.at (1).c_str ());

      std::vector<std::string> focal_st;
      boost::split (focal_st, focal_str, boost::is_any_of (","), boost::token_compress_on);
      if (focal_st.size () != 3)
      {
        pcl::console::print_error ("[PCLVisualizer::getCameraParameters] Invalid parameters given for camera focal point!\n");
        return (false);
      }
      camera_.focal[0] = atof (focal_st.at (0).c_str ());
      camera_.focal[1] = atof (focal_st.at (1).c_str ());
      camera_.focal[2] = atof (focal_st.at (2).c_str ());

      std::vector<std::string> pos_st;
      boost::split (pos_st, pos_str, boost::is_any_of (","), boost::token_compress_on);
      if (pos_st.size () != 3)
      {
        pcl::console::print_error ("[PCLVisualizer::getCameraParameters] Invalid parameters given for camera position!\n");
        return (false);
      }
      camera_.pos[0] = atof (pos_st.at (0).c_str ());
      camera_.pos[1] = atof (pos_st.at (1).c_str ());
      camera_.pos[2] = atof (pos_st.at (2).c_str ());

      std::vector<std::string> view_st;
      boost::split (view_st, view_str, boost::is_any_of (","), boost::token_compress_on);
      if (view_st.size () != 3)
      {
        pcl::console::print_error ("[PCLVisualizer::getCameraParameters] Invalid parameters given for camera viewup!\n");
        return (false);
      }
      camera_.view[0] = atof (view_st.at (0).c_str ());
      camera_.view[1] = atof (view_st.at (1).c_str ());
      camera_.view[2] = atof (view_st.at (2).c_str ());

      std::vector<std::string> fovy_size_st;
      boost::split (fovy_size_st, fovy_str, boost::is_any_of (","), boost::token_compress_on);
      if (fovy_size_st.size () != 1)
      {
        pcl::console::print_error ("[PCLVisualizer::getCameraParameters] Invalid parameters given for field of view angle!\n");
        return (false);
      }
      camera_.fovy = atof (fovy_size_st.at (0).c_str ());

      std::vector<std::string> win_size_st;
      boost::split (win_size_st, win_size_str, boost::is_any_of (","), boost::token_compress_on);
      if (win_size_st.size () != 2)
      {
        pcl::console::print_error ("[PCLVisualizer::getCameraParameters] Invalid parameters given for window size!\n");
        return (false);
      }
      camera_.window_size[0] = atof (win_size_st.at (0).c_str ());
      camera_.window_size[1] = atof (win_size_st.at (1).c_str ());

      std::vector<std::string> win_pos_st;
      boost::split (win_pos_st, win_pos_str, boost::is_any_of (","), boost::token_compress_on);
      if (win_pos_st.size () != 2)
      {
        pcl::console::print_error ("[PCLVisualizer::getCameraParameters] Invalid parameters given for window position!\n");
        return (false);
      }
      camera_.window_pos[0] = atof (win_pos_st.at (0).c_str ());
      camera_.window_pos[1] = atof (win_pos_st.at (1).c_str ());

      return (true);
    }
  }
  return (false);
}

////////////////////////////////////////////////////////////////////////////////////////////
bool
pcl::visualization::PCLVisualizer::addCylinder (const pcl::ModelCoefficients &coefficients, 
                                               const std::string &id, int viewport)
{
  // Check to see if this ID entry already exists (has it been already added to the visualizer?)
  ShapeActorMap::iterator am_it = shape_actor_map_->find (id);
  if (am_it != shape_actor_map_->end ())
  {
    pcl::console::print_warn ("[addCylinder] A shape with id <%s> already exists! Please choose a different id and retry.\n", id.c_str ());
    return (false);
  }

  vtkSmartPointer<vtkDataSet> data = createCylinder (coefficients);

  // Create an Actor
  vtkSmartPointer<vtkLODActor> actor;
  createActorFromVTKDataSet (data, actor);
  actor->GetProperty ()->SetRepresentationToWireframe ();
  addActorToRenderer (actor, viewport);

  // Save the pointer/ID pair to the global actor map
  (*shape_actor_map_)[id] = actor;
  return (true);
}

////////////////////////////////////////////////////////////////////////////////////////////
bool
pcl::visualization::PCLVisualizer::addCube (const pcl::ModelCoefficients &coefficients, 
                                            const std::string &id, int viewport)
{
  // Check to see if this ID entry already exists (has it been already added to the visualizer?)
  ShapeActorMap::iterator am_it = shape_actor_map_->find (id);
  if (am_it != shape_actor_map_->end ())
  {
    pcl::console::print_warn ("[addCube] A shape with id <%s> already exists! Please choose a different id and retry.\n", id.c_str ());
    return (false);
  }

  vtkSmartPointer<vtkDataSet> data = createCube (coefficients);

  // Create an Actor
  vtkSmartPointer<vtkLODActor> actor;
  createActorFromVTKDataSet (data, actor);
  actor->GetProperty ()->SetRepresentationToWireframe ();
  addActorToRenderer (actor, viewport);

  // Save the pointer/ID pair to the global actor map
  (*shape_actor_map_)[id] = actor;
  return (true);
}

////////////////////////////////////////////////////////////////////////////////////////////
bool
pcl::visualization::PCLVisualizer::addSphere (const pcl::ModelCoefficients &coefficients, 
                                             const std::string &id, int viewport)
{
  // Check to see if this ID entry already exists (has it been already added to the visualizer?)
  ShapeActorMap::iterator am_it = shape_actor_map_->find (id);
  if (am_it != shape_actor_map_->end ())
  {
    pcl::console::print_warn ("[addSphere] A shape with id <%s> already exists! Please choose a different id and retry.\n", id.c_str ());
    return (false);
  }

  vtkSmartPointer<vtkDataSet> data = createSphere (coefficients);

  // Create an Actor
  vtkSmartPointer<vtkLODActor> actor;
  createActorFromVTKDataSet (data, actor);
  actor->GetProperty ()->SetRepresentationToWireframe ();
  addActorToRenderer (actor, viewport);

  // Save the pointer/ID pair to the global actor map
  (*shape_actor_map_)[id] = actor;
  return (true);
}

////////////////////////////////////////////////////////////////////////////////////////////
bool
pcl::visualization::PCLVisualizer::addModelFromPolyData (
    vtkSmartPointer<vtkPolyData> polydata, const std::string & id, int viewport) 
{
  ShapeActorMap::iterator am_it = shape_actor_map_->find (id);
  if (am_it != shape_actor_map_->end ())
  {
    pcl::console::print_warn (
                                "[addModelFromPolyData] A shape with id <%s> already exists! Please choose a different id and retry.\n",
                                id.c_str ());
    return (false);
  }

  vtkSmartPointer<vtkLODActor> actor;
  createActorFromVTKDataSet (polydata, actor);
  actor->GetProperty ()->SetRepresentationToWireframe ();
  addActorToRenderer (actor, viewport);

  // Save the pointer/ID pair to the global actor map
  (*shape_actor_map_)[id] = actor;
  return (true);
}

////////////////////////////////////////////////////////////////////////////////////////////
bool
pcl::visualization::PCLVisualizer::addModelFromPolyData (
    vtkSmartPointer<vtkPolyData> polydata, vtkSmartPointer<vtkTransform> transform, const std::string & id, int viewport) 
{
  ShapeActorMap::iterator am_it = shape_actor_map_->find (id);
  if (am_it != shape_actor_map_->end ())
  {
    pcl::console::print_warn (
                                "[addModelFromPolyData] A shape with id <%s> already exists! Please choose a different id and retry.\n",
                                id.c_str ());
    return (false);
  }

  vtkSmartPointer < vtkTransformFilter > trans_filter = vtkSmartPointer<vtkTransformFilter>::New ();
  trans_filter->SetTransform (transform);
  trans_filter->SetInput ( polydata );
  trans_filter->Update();

  // Create an Actor
  vtkSmartPointer < vtkLODActor > actor;
  createActorFromVTKDataSet (trans_filter->GetOutput (), actor);
  actor->GetProperty ()->SetRepresentationToWireframe ();
  addActorToRenderer (actor, viewport);

  // Save the pointer/ID pair to the global actor map
  (*shape_actor_map_)[id] = actor;
  return (true);
}


////////////////////////////////////////////////////////////////////////////////////////////
bool
pcl::visualization::PCLVisualizer::addModelFromPLYFile (const std::string &filename, 
                                                       const std::string &id, int viewport)
{
  ShapeActorMap::iterator am_it = shape_actor_map_->find (id);
  if (am_it != shape_actor_map_->end ())
  {
    pcl::console::print_warn (
                                "[addModelFromPLYFile] A shape with id <%s> already exists! Please choose a different id and retry.\n",
                                id.c_str ());
    return (false);
  }

  vtkSmartPointer<vtkPLYReader> reader = vtkSmartPointer<vtkPLYReader>::New ();
  reader->SetFileName (filename.c_str ());

  // Create an Actor
  vtkSmartPointer<vtkLODActor> actor;
  createActorFromVTKDataSet (reader->GetOutput (), actor);
  actor->GetProperty ()->SetRepresentationToWireframe ();
  addActorToRenderer (actor, viewport);

  // Save the pointer/ID pair to the global actor map
  (*shape_actor_map_)[id] = actor;
  return (true);
}

////////////////////////////////////////////////////////////////////////////////////////////
bool
pcl::visualization::PCLVisualizer::addModelFromPLYFile (const std::string &filename,
                                                       vtkSmartPointer<vtkTransform> transform, const std::string &id,
                                                       int viewport)
{
  ShapeActorMap::iterator am_it = shape_actor_map_->find (id);
  if (am_it != shape_actor_map_->end ())
  {
    pcl::console::print_warn (
                                "[addModelFromPLYFile] A shape with id <%s> already exists! Please choose a different id and retry.\n",
                                id.c_str ());
    return (false);
  }

  vtkSmartPointer < vtkPLYReader > reader = vtkSmartPointer<vtkPLYReader>::New ();
  reader->SetFileName (filename.c_str ());

  //create transformation filter
  vtkSmartPointer < vtkTransformFilter > trans_filter = vtkSmartPointer<vtkTransformFilter>::New ();
  trans_filter->SetTransform (transform);
  trans_filter->SetInputConnection (reader->GetOutputPort ());

  // Create an Actor
  vtkSmartPointer < vtkLODActor > actor;
  createActorFromVTKDataSet (trans_filter->GetOutput (), actor);
  actor->GetProperty ()->SetRepresentationToWireframe ();
  addActorToRenderer (actor, viewport);

  // Save the pointer/ID pair to the global actor map
  (*shape_actor_map_)[id] = actor;
  return (true);
}

/////////////////////////////////////////////////////////////////////////////////////////////
bool
pcl::visualization::PCLVisualizer::addLine (const pcl::ModelCoefficients &coefficients, const std::string &id, int viewport)
{
  // Check to see if this ID entry already exists (has it been already added to the visualizer?)
  ShapeActorMap::iterator am_it = shape_actor_map_->find (id);
  if (am_it != shape_actor_map_->end ())
  {
    pcl::console::print_warn ("[addLine] A shape with id <%s> already exists! Please choose a different id and retry.\n", id.c_str ());
    return (false);
  }

  vtkSmartPointer<vtkDataSet> data = createLine (coefficients);

  // Create an Actor
  vtkSmartPointer<vtkLODActor> actor;
  createActorFromVTKDataSet (data, actor);
  actor->GetProperty ()->SetRepresentationToWireframe ();
  addActorToRenderer (actor, viewport);

  // Save the pointer/ID pair to the global actor map
  (*shape_actor_map_)[id] = actor;
  return (true);
}

////////////////////////////////////////////////////////////////////////////////////////////
/** \brief Add a plane from a set of given model coefficients 
  * \param coefficients the model coefficients (a, b, c, d with ax+by+cz+d=0)
  * \param id the plane id/name (default: "plane")
  * \param viewport (optional) the id of the new viewport (default: 0)
  */
bool
  pcl::visualization::PCLVisualizer::addPlane (const pcl::ModelCoefficients &coefficients, const std::string &id, int viewport)
{
  // Check to see if this ID entry already exists (has it been already added to the visualizer?)
  ShapeActorMap::iterator am_it = shape_actor_map_->find (id);
  if (am_it != shape_actor_map_->end ())
  {
    pcl::console::print_warn ("[addPlane] A shape with id <%s> already exists! Please choose a different id and retry.\n", id.c_str ());
    return (false);
  }

  vtkSmartPointer<vtkDataSet> data = createPlane (coefficients);

  // Create an Actor
  vtkSmartPointer<vtkLODActor> actor;
  createActorFromVTKDataSet (data, actor);
  actor->GetProperty ()->SetRepresentationToWireframe ();
  addActorToRenderer (actor, viewport);

  // Save the pointer/ID pair to the global actor map
  (*shape_actor_map_)[id] = actor;
  return (true);
}

/////////////////////////////////////////////////////////////////////////////////////////////
bool
pcl::visualization::PCLVisualizer::addCircle (const pcl::ModelCoefficients &coefficients, const std::string &id, int viewport)
{
  // Check to see if this ID entry already exists (has it been already added to the visualizer?)
  ShapeActorMap::iterator am_it = shape_actor_map_->find (id);
  if (am_it != shape_actor_map_->end ())
  {
    pcl::console::print_warn ("[addCircle] A shape with id <%s> already exists! Please choose a different id and retry.\n", id.c_str ());
    return (false);
  }

  vtkSmartPointer<vtkDataSet> data = create2DCircle (coefficients);

  // Create an Actor
  vtkSmartPointer<vtkLODActor> actor;
  createActorFromVTKDataSet (data, actor);
  actor->GetProperty ()->SetRepresentationToWireframe ();
  addActorToRenderer (actor, viewport);

  // Save the pointer/ID pair to the global actor map
  (*shape_actor_map_)[id] = actor;
  return (true);
}

/////////////////////////////////////////////////////////////////////////////////////////////
bool
pcl::visualization::PCLVisualizer::addCone (const pcl::ModelCoefficients &coefficients, const std::string &id, int viewport)
{
  // Check to see if this ID entry already exists (has it been already added to the visualizer?)
  ShapeActorMap::iterator am_it = shape_actor_map_->find (id);
  if (am_it != shape_actor_map_->end ())
  {
    pcl::console::print_warn ("[addCone] A shape with id <%s> already exists! Please choose a different id and retry.\n", id.c_str ());
    return (false);
  }

  vtkSmartPointer<vtkDataSet> data = createCone (coefficients);

  // Create an Actor
  vtkSmartPointer<vtkLODActor> actor;
  createActorFromVTKDataSet (data, actor);
  actor->GetProperty ()->SetRepresentationToWireframe ();
  addActorToRenderer (actor, viewport);

  // Save the pointer/ID pair to the global actor map
  (*shape_actor_map_)[id] = actor;
  return (true);
}

/////////////////////////////////////////////////////////////////////////////////////////////
void 
pcl::visualization::PCLVisualizer::createViewPort (double xmin, double ymin, double xmax, double ymax, int &viewport)
{
  // Create a new renderer
  vtkSmartPointer<vtkRenderer> ren = vtkSmartPointer<vtkRenderer>::New ();
  ren->SetViewport (xmin, ymin, xmax, ymax);

  if (rens_->GetNumberOfItems () > 0)
    ren->SetActiveCamera (rens_->GetFirstRenderer ()->GetActiveCamera ());
  ren->ResetCamera ();

  // Add it to the list of renderers
  rens_->AddItem (ren);

  if (rens_->GetNumberOfItems () <= 1)          // If only one renderer
    viewport = 0;                               // set viewport to 'all'
  else
    viewport = rens_->GetNumberOfItems ();

  win_->AddRenderer (ren);
  win_->Modified ();
}

/////////////////////////////////////////////////////////////////////////////////////////////
bool 
pcl::visualization::PCLVisualizer::addText (const std::string &text, int xpos, int ypos, const std::string &id, int viewport)
{
  std::string tid;
  if (id.empty ())
    tid = text;
  else
    tid = id;

  // Check to see if this ID entry already exists (has it been already added to the visualizer?)
  ShapeActorMap::iterator am_it = shape_actor_map_->find (tid);
  if (am_it != shape_actor_map_->end ())
  {
    pcl::console::print_warn ("[addText] A text with id <%s> already exists! Please choose a different id and retry.\n", tid.c_str ());
    return (false);
  }

  // Create an Actor
  vtkSmartPointer<vtkTextActor> actor = vtkSmartPointer<vtkTextActor>::New ();
  actor->SetPosition (xpos, ypos);
  actor->SetInput (text.c_str ());
  
  vtkSmartPointer<vtkTextProperty> tprop = actor->GetTextProperty ();
  tprop->SetFontSize (10);
  tprop->SetFontFamilyToArial ();
  tprop->SetJustificationToLeft ();
  tprop->BoldOn ();
  tprop->SetColor (1, 1, 1);
  addActorToRenderer (actor, viewport);

  // Save the pointer/ID pair to the global actor map
  (*shape_actor_map_)[tid] = actor;
  return (true);
}

/////////////////////////////////////////////////////////////////////////////////////////////
bool 
pcl::visualization::PCLVisualizer::addText (const std::string &text, int xpos, int ypos, double r, double g, double b, const std::string &id, int viewport)
{
  std::string tid;
  if (id.empty ())
    tid = text;
  else
    tid = id;

  // Check to see if this ID entry already exists (has it been already added to the visualizer?)
  ShapeActorMap::iterator am_it = shape_actor_map_->find (tid);
  if (am_it != shape_actor_map_->end ())
  {
    pcl::console::print_warn ("[addText] A text with id <%s> already exists! Please choose a different id and retry.\n", tid.c_str ());
    return (false);
  }

  // Create an Actor
  vtkSmartPointer<vtkTextActor> actor = vtkSmartPointer<vtkTextActor>::New ();
  actor->SetPosition (xpos, ypos);
  actor->SetInput (text.c_str ());
  
  vtkSmartPointer<vtkTextProperty> tprop = actor->GetTextProperty ();
  tprop->SetFontSize (10);
  tprop->SetFontFamilyToArial ();
  tprop->SetJustificationToLeft ();
  tprop->BoldOn ();
  tprop->SetColor (r, g, b);
  addActorToRenderer (actor, viewport);

  // Save the pointer/ID pair to the global actor map
  (*shape_actor_map_)[tid] = actor;
  return (true);
}

/////////////////////////////////////////////////////////////////////////////////////////////
bool 
pcl::visualization::PCLVisualizer::addText (const std::string &text, int xpos, int ypos, int fontsize, double r, double g, double b, const std::string &id, int viewport)
{
  std::string tid;
  if (id.empty ())
    tid = text;
  else
    tid = id;

  // Check to see if this ID entry already exists (has it been already added to the visualizer?)
  ShapeActorMap::iterator am_it = shape_actor_map_->find (tid);
  if (am_it != shape_actor_map_->end ())
  {
    pcl::console::print_warn ("[addText] A text with id <%s> already exists! Please choose a different id and retry.\n", tid.c_str ());
    return (false);
  }

  // Create an Actor
  vtkSmartPointer<vtkTextActor> actor = vtkSmartPointer<vtkTextActor>::New ();
  actor->SetPosition (xpos, ypos);
  actor->SetInput (text.c_str ());
  
  vtkSmartPointer<vtkTextProperty> tprop = actor->GetTextProperty ();
  tprop->SetFontSize (fontsize);
  tprop->SetFontFamilyToArial ();
  tprop->SetJustificationToLeft ();
  tprop->BoldOn ();
  tprop->SetColor (r, g, b);
  addActorToRenderer (actor, viewport);

  // Save the pointer/ID pair to the global actor map
  (*shape_actor_map_)[tid] = actor;
  return (true);
}


/////////////////////////////////////////////////////////////////////////////////////////////
bool
pcl::visualization::PCLVisualizer::updateColorHandlerIndex (const std::string &id, int index)
{
  CloudActorMap::iterator am_it = cloud_actor_map_->find (id);
  if (am_it == cloud_actor_map_->end ())
  {
    pcl::console::print_warn ("[updateColorHandlerIndex] PointCloud with id <%s> doesn't exist!\n", id.c_str ());
    return (false);
  }

  if (index >= (int)am_it->second.color_handlers.size ())
  {
    pcl::console::print_warn ("[updateColorHandlerIndex] Invalid index <%d> given! Maximum range is: 0-%lu.\n", index, (unsigned long)am_it->second.color_handlers.size ());
    return (false);
  }
  // Get the handler
  PointCloudColorHandler<sensor_msgs::PointCloud2>::ConstPtr color_handler = am_it->second.color_handlers[index];

  vtkSmartPointer<vtkDataArray> scalars;
  color_handler->getColor (scalars);
  double minmax[2];
  scalars->GetRange (minmax);
  // Update the data
  vtkPolyData *data = static_cast<vtkPolyData*>(am_it->second.actor->GetMapper ()->GetInput ());
  data->GetPointData ()->SetScalars (scalars);
  data->Update ();
  // Modify the mapper
  vtkPolyDataMapper* mapper = static_cast<vtkPolyDataMapper*>(am_it->second.actor->GetMapper ());
  mapper->SetScalarRange (minmax);
  mapper->SetScalarModeToUsePointData ();
  mapper->SetInput (data);
  // Modify the actor
  am_it->second.actor->SetMapper (mapper);
  am_it->second.actor->Modified ();
  am_it->second.color_handler_index_ = index;
  
  //style_->setCloudActorMap (cloud_actor_map_);

  return (true);
}



/////////////////////////////////////////////////////////////////////////////////////////////
bool
pcl::visualization::PCLVisualizer::addPolygonMesh (const pcl::PolygonMesh &poly_mesh, 
                                                  const std::string &id,
                                                  int viewport)
{
  ShapeActorMap::iterator am_it = shape_actor_map_->find (id);
  if (am_it != shape_actor_map_->end ())
  {
    pcl::console::print_warn (
                                "[addPolygonMesh] A shape with id <%s> already exists! Please choose a different id and retry.\n",
                                id.c_str ());
    return (false);
  }

  // Create points from polyMesh.cloud
  vtkSmartPointer<vtkPoints> poly_points = vtkSmartPointer<vtkPoints>::New ();
  pcl::PointCloud<pcl::PointXYZ>::Ptr point_cloud (new pcl::PointCloud<pcl::PointXYZ> ());
  pcl::fromROSMsg (poly_mesh.cloud, *point_cloud);
  poly_points->SetNumberOfPoints (point_cloud->points.size ());

  size_t i;
  for (i = 0; i < point_cloud->points.size (); ++i)
    poly_points->InsertPoint (i, point_cloud->points[i].x, point_cloud->points[i].y, point_cloud->points[i].z);

  bool has_color = false;
  vtkSmartPointer<vtkUnsignedCharArray> colors = vtkSmartPointer<vtkUnsignedCharArray>::New ();
  if (pcl::getFieldIndex(poly_mesh.cloud, "rgb") != -1)
  {
    has_color = true;
    colors->SetNumberOfComponents (3);
    colors->SetName ("Colors");
    pcl::PointCloud<pcl::PointXYZRGB> cloud;
    pcl::fromROSMsg(poly_mesh.cloud, cloud);
    for (i = 0; i < cloud.points.size (); ++i)
    {
      const unsigned char color[3] = {cloud.points[i].r, cloud.points[i].g, cloud.points[i].b};
      colors->InsertNextTupleValue(color);
    }
  }
  if (pcl::getFieldIndex(poly_mesh.cloud, "rgba") != -1)
  {
    has_color = true;
    colors->SetNumberOfComponents (3);
    colors->SetName ("Colors");
    pcl::PointCloud<pcl::PointXYZRGBA> cloud;
    pcl::fromROSMsg(poly_mesh.cloud, cloud);
    for (i = 0; i < cloud.points.size (); ++i)
    {
      const unsigned char color[3] = {cloud.points[i].r, cloud.points[i].g, cloud.points[i].b};
      colors->InsertNextTupleValue(color);
    }
  }

  vtkSmartPointer<vtkLODActor> actor;
  if (poly_mesh.polygons.size() > 1) 
  {
    //create polys from polyMesh.polygons
    vtkSmartPointer<vtkCellArray> cell_array = vtkSmartPointer<vtkCellArray>::New ();

    for (i = 0; i < poly_mesh.polygons.size (); i++) 
    {
      size_t n_points = poly_mesh.polygons[i].vertices.size ();
      cell_array->InsertNextCell (n_points);
      for (size_t j = 0; j < n_points; j++) 
        cell_array->InsertCellPoint (poly_mesh.polygons[i].vertices[j]);
    }

    vtkSmartPointer<vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();
//    polydata->SetStrips (cell_array);
    polydata->SetPolys (cell_array);
    polydata->SetPoints (poly_points);

    if (has_color)
      polydata->GetPointData()->SetScalars(colors);

    createActorFromVTKDataSet (polydata, actor);
  } 
  else if (poly_mesh.polygons.size() == 1)
  {
    vtkSmartPointer<vtkPolygon> polygon = vtkSmartPointer<vtkPolygon>::New ();
    size_t n_points = poly_mesh.polygons[0].vertices.size ();
    polygon->GetPointIds ()->SetNumberOfIds (n_points - 1);

    for (size_t j=0; j < (n_points - 1); j++) 
      polygon->GetPointIds ()->SetId (j, poly_mesh.polygons[0].vertices[j]);

    vtkSmartPointer<vtkUnstructuredGrid> poly_grid = vtkSmartPointer<vtkUnstructuredGrid>::New ();
    poly_grid->Allocate (1, 1);
    poly_grid->InsertNextCell (polygon->GetCellType (), polygon->GetPointIds ());
    poly_grid->SetPoints (poly_points);
    poly_grid->Update ();

    createActorFromVTKDataSet (poly_grid, actor);
    actor->GetProperty ()->SetRepresentationToWireframe ();
  }
  else
  {
    PCL_ERROR("PCLVisualizer::addPolygonMesh: No polygons\n");
    return false;
  }

  actor->GetProperty ()->SetRepresentationToSurface ();
  addActorToRenderer (actor, viewport);

  // Save the pointer/ID pair to the global actor map
  (*shape_actor_map_)[id] = actor;
  return (true);
}


///////////////////////////////////////////////////////////////////////////////////
bool
pcl::visualization::PCLVisualizer::addPolylineFromPolygonMesh (
    const pcl::PolygonMesh &polymesh, const std::string &id, int viewport) 
{
  ShapeActorMap::iterator am_it = shape_actor_map_->find (id);
  if (am_it != shape_actor_map_->end ())
  {
    pcl::console::print_warn (
                                "[addPolylineFromPolygonMesh] A shape with id <%s> already exists! Please choose a different id and retry.\n",
                                id.c_str ());
    return (false);
  }

  // Create points from polyMesh.cloud
  vtkSmartPointer<vtkPoints> poly_points = vtkSmartPointer<vtkPoints>::New ();
  pcl::PointCloud<pcl::PointXYZ> point_cloud;
  pcl::fromROSMsg (polymesh.cloud, point_cloud);
  poly_points->SetNumberOfPoints (point_cloud.points.size ());

  size_t i;
  for (i = 0; i < point_cloud.points.size (); ++i)
    poly_points->InsertPoint (i, point_cloud.points[i].x, point_cloud.points[i].y, point_cloud.points[i].z);

  // Create a cell array to store the lines in and add the lines to it
  vtkSmartPointer <vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New ();
  vtkSmartPointer <vtkPolyData> polyData;
  allocVtkPolyData (polyData);

  for (i = 0; i < polymesh.polygons.size (); i++)
  {
    vtkSmartPointer<vtkPolyLine> polyLine = vtkSmartPointer<vtkPolyLine>::New();
    polyLine->GetPointIds()->SetNumberOfIds(polymesh.polygons[i].vertices.size());
    for(unsigned int k = 0; k < polymesh.polygons[i].vertices.size(); k++)
    {
      polyLine->GetPointIds()->SetId(k,polymesh.polygons[i].vertices[k]);
    }

    cells->InsertNextCell (polyLine);
  }

  // Add the points to the dataset
  polyData->SetPoints (poly_points);

  // Add the lines to the dataset
  polyData->SetLines (cells);

  // Setup actor and mapper
  vtkSmartPointer < vtkPolyDataMapper > mapper = vtkSmartPointer<vtkPolyDataMapper>::New ();
  mapper->SetInput (polyData);

  vtkSmartPointer < vtkActor > actor = vtkSmartPointer<vtkActor>::New ();
  actor->SetMapper (mapper);


  addActorToRenderer (actor, viewport);

  // Save the pointer/ID pair to the global actor map
  (*shape_actor_map_)[id] = actor;

  return (true);
}

///////////////////////////////////////////////////////////////////////////////////
void
pcl::visualization::PCLVisualizer::setRepresentationToSurfaceForAllActors ()
{
  ShapeActorMap::iterator am_it;
  rens_->InitTraversal ();
  vtkRenderer* renderer = NULL;
  while ((renderer = rens_->GetNextItem ()) != NULL)
  {
    vtkActorCollection * actors = renderer->GetActors ();
    actors->InitTraversal ();
    vtkActor * actor;
    while ((actor = actors->GetNextActor ()) != NULL)
    {
      actor->GetProperty ()->SetRepresentationToSurface ();
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////
void
pcl::visualization::PCLVisualizer::renderViewTesselatedSphere (
                                                               int xres,
                                                               int yres,
                                                               std::vector<pcl::PointCloud<pcl::PointXYZ>,
                                                                   Eigen::aligned_allocator<pcl::PointCloud<
                                                                       pcl::PointXYZ> > > & clouds,
                                                               std::vector<Eigen::Matrix4f, Eigen::aligned_allocator<
                                                                   Eigen::Matrix4f> > & poses,
                                                               std::vector<float> & enthropies, int tesselation_level,
                                                               float view_angle, float radius_sphere, bool use_vertices)
{
  if (rens_->GetNumberOfItems () > 1)
  {
    PCL_WARN ("[renderViewTesselatedSphere] Method works only with one renderer.\n");
    return;
  }

  rens_->InitTraversal ();
  vtkRenderer* renderer_pcl_vis = rens_->GetNextItem ();
  vtkActorCollection * actors = renderer_pcl_vis->GetActors ();

  if (actors->GetNumberOfItems () > 1)
    PCL_INFO ("[renderViewTesselatedSphere] Method only consider the first actor on the scene, more than one found.\n");

  //get vtk object from the visualizer
  actors->InitTraversal ();
  vtkActor * actor = actors->GetNextActor ();
  vtkSmartPointer<vtkPolyData> polydata;
  allocVtkPolyData (polydata);
  polydata->CopyStructure (actor->GetMapper ()->GetInput ());

  //center object
  double CoM[3];
  vtkIdType npts_com = 0, *ptIds_com = NULL;
  vtkSmartPointer<vtkCellArray> cells_com = polydata->GetPolys ();

  double center[3], p1_com[3], p2_com[3], p3_com[3], area_com, totalArea_com = 0;
  double comx = 0, comy = 0, comz = 0;
  for (cells_com->InitTraversal (); cells_com->GetNextCell (npts_com, ptIds_com);)
  {
    polydata->GetPoint (ptIds_com[0], p1_com);
    polydata->GetPoint (ptIds_com[1], p2_com);
    polydata->GetPoint (ptIds_com[2], p3_com);
    vtkTriangle::TriangleCenter (p1_com, p2_com, p3_com, center);
    area_com = vtkTriangle::TriangleArea (p1_com, p2_com, p3_com);
    comx += center[0] * area_com;
    comy += center[1] * area_com;
    comz += center[2] * area_com;
    totalArea_com += area_com;
  }

  CoM[0] = comx / totalArea_com;
  CoM[1] = comy / totalArea_com;
  CoM[2] = comz / totalArea_com;

  vtkSmartPointer<vtkTransform> trans_center = vtkSmartPointer<vtkTransform>::New ();
  trans_center->Translate (-CoM[0], -CoM[1], -CoM[2]);
  vtkSmartPointer<vtkMatrix4x4> matrixCenter = trans_center->GetMatrix ();

  vtkSmartPointer<vtkTransformFilter> trans_filter_center = vtkSmartPointer<vtkTransformFilter>::New ();
  trans_filter_center->SetTransform (trans_center);
  trans_filter_center->SetInput (polydata);
  trans_filter_center->Update ();

  vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New ();
  mapper->SetInputConnection (trans_filter_center->GetOutputPort ());
  mapper->Update ();

  //scale so it fits in the unit sphere!
  double bb[6];
  mapper->GetBounds (bb);
  double ms = (std::max) ((std::fabs) (bb[0] - bb[1]),
                          (std::max) ((std::fabs) (bb[2] - bb[3]), (std::fabs) (bb[4] - bb[5])));
  double max_side = radius_sphere / 2.0;
  double scale_factor = max_side / ms;

  vtkSmartPointer<vtkTransform> trans_scale = vtkSmartPointer<vtkTransform>::New ();
  trans_scale->Scale (scale_factor, scale_factor, scale_factor);
  vtkSmartPointer<vtkMatrix4x4> matrixScale = trans_scale->GetMatrix ();

  vtkSmartPointer<vtkTransformFilter> trans_filter_scale = vtkSmartPointer<vtkTransformFilter>::New ();
  trans_filter_scale->SetTransform (trans_scale);
  trans_filter_scale->SetInputConnection (trans_filter_center->GetOutputPort ());
  trans_filter_scale->Update ();

  mapper->SetInputConnection (trans_filter_scale->GetOutputPort ());
  mapper->Update ();

  //////////////////////////////
  // * Compute area of the mesh
  //////////////////////////////
  vtkSmartPointer<vtkCellArray> cells = mapper->GetInput ()->GetPolys ();
  vtkIdType npts = 0, *ptIds = NULL;

  double p1[3], p2[3], p3[3], area, totalArea = 0;
  for (cells->InitTraversal (); cells->GetNextCell (npts, ptIds);)
  {
    polydata->GetPoint (ptIds[0], p1);
    polydata->GetPoint (ptIds[1], p2);
    polydata->GetPoint (ptIds[2], p3);
    area = vtkTriangle::TriangleArea (p1, p2, p3);
    totalArea += area;
  }

  //create icosahedron
  vtkSmartPointer<vtkPlatonicSolidSource> ico = vtkSmartPointer<vtkPlatonicSolidSource>::New ();
  ico->SetSolidTypeToIcosahedron ();
  ico->Update ();

  //tesselate cells from icosahedron
  vtkSmartPointer<vtkLoopSubdivisionFilter> subdivide = vtkSmartPointer<vtkLoopSubdivisionFilter>::New ();
  subdivide->SetNumberOfSubdivisions (tesselation_level);
  subdivide->SetInputConnection (ico->GetOutputPort ());

  // Get camera positions
  vtkPolyData *sphere = subdivide->GetOutput ();
  sphere->Update ();

  std::vector<Eigen::Vector3f> cam_positions;
  if (!use_vertices)
  {
    vtkSmartPointer<vtkCellArray> cells_sphere = sphere->GetPolys ();
    cam_positions.resize (sphere->GetNumberOfPolys ());

    size_t i=0;
    for (cells_sphere->InitTraversal (); cells_sphere->GetNextCell (npts_com, ptIds_com);)
    {
      sphere->GetPoint (ptIds_com[0], p1_com);
      sphere->GetPoint (ptIds_com[1], p2_com);
      sphere->GetPoint (ptIds_com[2], p3_com);
      vtkTriangle::TriangleCenter (p1_com, p2_com, p3_com, center);
      cam_positions[i] = Eigen::Vector3f (center[0], center[1], center[2]);
      i++;
    }

  }
  else
  {
    cam_positions.resize (sphere->GetNumberOfPoints ());
    for (int i = 0; i < sphere->GetNumberOfPoints (); i++)
    {
      double cam_pos[3];
      sphere->GetPoint (i, cam_pos);
      cam_positions[i] = Eigen::Vector3f (cam_pos[0], cam_pos[1], cam_pos[2]);
    }
  }

  double camera_radius = radius_sphere;
  double cam_pos[3];
  double first_cam_pos[3];

  first_cam_pos[0] = cam_positions[0][0] * radius_sphere;
  first_cam_pos[1] = cam_positions[0][1] * radius_sphere;
  first_cam_pos[2] = cam_positions[0][2] * radius_sphere;

  //create renderer and window
  vtkSmartPointer<vtkRenderWindow> render_win = vtkSmartPointer<vtkRenderWindow>::New ();
  vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New ();
  render_win->AddRenderer (renderer);
  render_win->SetSize (xres, yres);
  renderer->SetBackground (1.0, 0, 0);

  //create picker
  vtkSmartPointer<vtkWorldPointPicker> worldPicker = vtkSmartPointer<vtkWorldPointPicker>::New ();

  vtkSmartPointer<vtkCamera> cam = vtkSmartPointer<vtkCamera>::New ();
  cam->SetFocalPoint (0, 0, 0);

  Eigen::Vector3f cam_pos_3f = cam_positions[0];
  Eigen::Vector3f perp = cam_pos_3f.cross (Eigen::Vector3f::UnitY ());
  cam->SetViewUp (perp[0], perp[1], perp[2]);

  cam->SetPosition (first_cam_pos);
  cam->SetViewAngle (view_angle);
  cam->Modified ();

  //For each camera position, traposesnsform the object and render view
  for (size_t i = 0; i < cam_positions.size (); i++)
  {
    cam_pos[0] = cam_positions[i][0];
    cam_pos[1] = cam_positions[i][1];
    cam_pos[2] = cam_positions[i][2];

    //create temporal virtual camera
    vtkSmartPointer<vtkCamera> cam_tmp = vtkSmartPointer<vtkCamera>::New ();
    cam_tmp->SetViewAngle (view_angle);

    Eigen::Vector3f cam_pos_3f (cam_pos[0], cam_pos[1], cam_pos[2]);
    cam_pos_3f = cam_pos_3f.normalized ();
    Eigen::Vector3f test = Eigen::Vector3f::UnitY ();

    //If the view up is parallel to ray cam_pos - focalPoint then the transformation
    //is singular and no points are rendered...
    //make sure it is perpendicular
    if (fabs (cam_pos_3f.dot (test)) == 1)
    {
      //parallel, create
      test = cam_pos_3f.cross (Eigen::Vector3f::UnitX ());
    }

    cam_tmp->SetViewUp (test[0], test[1], test[2]);

    for (int k = 0; k < 3; k++)
    {
      cam_pos[k] = cam_pos[k] * camera_radius;
    }

    cam_tmp->SetPosition (cam_pos);
    cam_tmp->SetFocalPoint (0, 0, 0);
    cam_tmp->Modified ();

    //rotate model so it looks the same as if we would look from the new position
    vtkSmartPointer<vtkMatrix4x4> view_trans_inverted = vtkSmartPointer<vtkMatrix4x4>::New ();
    vtkMatrix4x4::Invert (cam->GetViewTransformMatrix (), view_trans_inverted);
    vtkSmartPointer<vtkTransform> trans_rot_pose = vtkSmartPointer<vtkTransform>::New ();
    trans_rot_pose->Identity ();
    trans_rot_pose->Concatenate (view_trans_inverted);
    trans_rot_pose->Concatenate (cam_tmp->GetViewTransformMatrix ());
    vtkSmartPointer<vtkTransformFilter> trans_rot_pose_filter = vtkSmartPointer<vtkTransformFilter>::New ();
    trans_rot_pose_filter->SetTransform (trans_rot_pose);
    trans_rot_pose_filter->SetInputConnection (trans_filter_scale->GetOutputPort ());

    //translate model so we can place camera at (0,0,0)
    vtkSmartPointer<vtkTransform> translation = vtkSmartPointer<vtkTransform>::New ();
    translation->Translate (first_cam_pos[0] * -1, first_cam_pos[1] * -1, first_cam_pos[2] * -1);
    vtkSmartPointer<vtkTransformFilter> translation_filter = vtkSmartPointer<vtkTransformFilter>::New ();
    translation_filter->SetTransform (translation);
    translation_filter->SetInputConnection (trans_rot_pose_filter->GetOutputPort ());

    //modify camera
    cam_tmp->SetPosition (0, 0, 0);
    cam_tmp->SetFocalPoint (first_cam_pos[0] * -1, first_cam_pos[1] * -1, first_cam_pos[2] * -1);
    cam_tmp->Modified ();

    //notice transformations for final pose
    vtkSmartPointer<vtkMatrix4x4> matrixRotModel = trans_rot_pose->GetMatrix ();
    vtkSmartPointer<vtkMatrix4x4> matrixTranslation = translation->GetMatrix ();

    mapper->SetInputConnection (translation_filter->GetOutputPort ());
    mapper->Update ();

    //render view
    vtkSmartPointer<vtkActor> actor_view = vtkSmartPointer<vtkActor>::New ();
    actor_view->SetMapper (mapper);
    renderer->SetActiveCamera (cam_tmp);
    renderer->AddActor (actor_view);
    renderer->Modified ();
    //renderer->ResetCameraClippingRange ();
    render_win->Render ();

    //back to real scale transform
    vtkSmartPointer<vtkTransform> backToRealScale = vtkSmartPointer<vtkTransform>::New ();
    backToRealScale->PostMultiply ();
    backToRealScale->Identity ();
    backToRealScale->Concatenate (matrixScale);
    backToRealScale->Concatenate (matrixTranslation);
    backToRealScale->Inverse ();
    backToRealScale->Modified ();
    backToRealScale->Concatenate (matrixTranslation);
    backToRealScale->Modified ();

    Eigen::Matrix4f backToRealScale_eigen;
    backToRealScale_eigen.setIdentity ();

    for (size_t x = 0; x < 4; x++)
      for (size_t y = 0; y < 4; y++)
        backToRealScale_eigen (x, y) = backToRealScale->GetMatrix ()->GetElement (x, y);

    pcl::PointCloud<pcl::PointXYZ>::Ptr cloud (new pcl::PointCloud<pcl::PointXYZ>);

    cloud->points.resize (xres * yres);
    cloud->width = xres * yres;
    cloud->height = 1;

    double coords[3];
    float * depth = new float[xres * yres];
    render_win->GetZbufferData (0, 0, xres - 1, yres - 1, &(depth[0]));

    int count_valid_depth_pixels = 0;
    for (size_t x = 0; x < (size_t)xres; x++)
    {
      for (size_t y = 0; y < (size_t)yres; y++)
      {
        float value = depth[y * xres + x];
        if (value == 1.0)
          continue;

        worldPicker->Pick (x, y, value, renderer);
        worldPicker->GetPickPosition (coords);
        cloud->points[count_valid_depth_pixels].x = coords[0];
        cloud->points[count_valid_depth_pixels].y = coords[1];
        cloud->points[count_valid_depth_pixels].z = coords[2];
        cloud->points[count_valid_depth_pixels].getVector4fMap () = backToRealScale_eigen
            * cloud->points[count_valid_depth_pixels].getVector4fMap ();
        count_valid_depth_pixels++;
      }
    }

    delete[] depth;

    //////////////////////////////
    // * Compute area of the mesh
    //////////////////////////////

    vtkSmartPointer<vtkPolyData> polydata = mapper->GetInput ();
    polydata->BuildCells ();

    vtkSmartPointer<vtkCellArray> cells = polydata->GetPolys ();
    vtkIdType npts = 0, *ptIds = NULL;

    double p1[3], p2[3], p3[3], area, totalArea = 0;
    for (cells->InitTraversal (); cells->GetNextCell (npts, ptIds);)
    {
      polydata->GetPoint (ptIds[0], p1);
      polydata->GetPoint (ptIds[1], p2);
      polydata->GetPoint (ptIds[2], p3);
      area = vtkTriangle::TriangleArea (p1, p2, p3);
      totalArea += area;
    }

    /////////////////////////////////////
    // * Select visible cells (triangles)
    /////////////////////////////////////
    vtkSmartPointer<vtkVisibleCellSelector> selector = vtkSmartPointer<vtkVisibleCellSelector>::New ();
    vtkSmartPointer<vtkIdTypeArray> selection = vtkSmartPointer<vtkIdTypeArray>::New ();

    selector->SetRenderer (renderer);
    selector->SetArea (0, 0, xres - 1, yres - 1);
    selector->Select ();
    selector->GetSelectedIds (selection);

    double visible_area = 0;
    for (int sel_id = 3; sel_id < (selection->GetNumberOfTuples () * selection->GetNumberOfComponents ()); sel_id
        += selection->GetNumberOfComponents ())
    {
      int id_mesh = selection->GetValue (sel_id);

      if (id_mesh >= polydata->GetNumberOfCells ())
        continue;

      vtkCell * cell = polydata->GetCell (id_mesh);
      vtkTriangle* triangle = dynamic_cast<vtkTriangle*> (cell);
      double p0[3];
      double p1[3];
      double p2[3];
      triangle->GetPoints ()->GetPoint (0, p0);
      triangle->GetPoints ()->GetPoint (1, p1);
      triangle->GetPoints ()->GetPoint (2, p2);
      visible_area += vtkTriangle::TriangleArea (p0, p1, p2);
    }

    //THIS CAN BE USED WHEN VTK >= 5.4 IS REQUIRED... vtkVisibleCellSelector is deprecated from VTK5.4
    /*vtkSmartPointer<vtkHardwareSelector> hardware_selector = vtkSmartPointer<vtkHardwareSelector>::New ();
     hardware_selector->ClearBuffers();
     vtkSmartPointer<vtkSelection> hdw_selection = vtkSmartPointer<vtkSelection>::New ();
     hardware_selector->SetRenderer (renderer);
     hardware_selector->SetArea (0, 0, xres - 1, yres - 1);
     hardware_selector->SetFieldAssociation(vtkDataObject::FIELD_ASSOCIATION_CELLS);
     hdw_selection = hardware_selector->Select ();
     vtkSmartPointer<vtkIdTypeArray> ids = vtkSmartPointer<vtkIdTypeArray>::New ();
     ids = vtkIdTypeArray::SafeDownCast(hdw_selection->GetNode(0)->GetSelectionList());
     double visible_area = 0;
     for (int sel_id = 0; sel_id < (ids->GetNumberOfTuples ()); sel_id++)
     {
     int id_mesh = selection->GetValue (sel_id);
     vtkCell * cell = polydata->GetCell (id_mesh);
     vtkTriangle* triangle = dynamic_cast<vtkTriangle*> (cell);
     double p0[3];
     double p1[3];
     double p2[3];
     triangle->GetPoints ()->GetPoint (0, p0);
     triangle->GetPoints ()->GetPoint (1, p1);
     triangle->GetPoints ()->GetPoint (2, p2);
     area = vtkTriangle::TriangleArea (p0, p1, p2);
     visible_area += area;
     }*/

    enthropies.push_back (visible_area / totalArea);

    cloud->points.resize (count_valid_depth_pixels);
    cloud->width = count_valid_depth_pixels;

    //transform cloud to give camera coordinates instead of world coordinates!
    vtkSmartPointer<vtkMatrix4x4> view_transform = cam_tmp->GetViewTransformMatrix ();
    Eigen::Matrix4f trans_view;
    trans_view.setIdentity ();

    for (size_t x = 0; x < 4; x++)
      for (size_t y = 0; y < 4; y++)
        trans_view (x, y) = view_transform->GetElement (x, y);

    //NOTE: vtk view coordinate system is different than the standard camera coordinates (z forward, y down, x right)
    //thus, the fliping in y and z
    for (size_t i = 0; i < cloud->points.size (); i++)
    {
      cloud->points[i].getVector4fMap () = trans_view * cloud->points[i].getVector4fMap ();
      cloud->points[i].y *= -1.0;
      cloud->points[i].z *= -1.0;
    }

    renderer->RemoveActor (actor_view);

    clouds.push_back (*cloud);

    //create pose, from OBJECT coordinates to CAMERA coordinates!
    vtkSmartPointer<vtkTransform> transOCtoCC = vtkSmartPointer<vtkTransform>::New ();
    transOCtoCC->PostMultiply ();
    transOCtoCC->Identity ();
    transOCtoCC->Concatenate (matrixCenter);
    transOCtoCC->Concatenate (matrixRotModel);
    transOCtoCC->Concatenate (matrixTranslation);
    transOCtoCC->Concatenate (cam_tmp->GetViewTransformMatrix ());

    //NOTE: vtk view coordinate system is different than the standard camera coordinates (z forward, y down, x right)
    //thus, the fliping in y and z
    vtkSmartPointer<vtkMatrix4x4> cameraSTD = vtkSmartPointer<vtkMatrix4x4>::New ();
    cameraSTD->Identity ();
    cameraSTD->SetElement (0, 0, 1);
    cameraSTD->SetElement (1, 1, -1);
    cameraSTD->SetElement (2, 2, -1);

    transOCtoCC->Concatenate (cameraSTD);
    transOCtoCC->Modified ();

    Eigen::Matrix4f pose_view;
    pose_view.setIdentity ();

    for (size_t x = 0; x < 4; x++)
      for (size_t y = 0; y < 4; y++)
        pose_view (x, y) = transOCtoCC->GetMatrix ()->GetElement (x, y);

    poses.push_back (pose_view);

  }
}

///////////////////////////////////////////////////////////////////////////////////
void
pcl::visualization::PCLVisualizer::renderView (int xres, int yres, pcl::PointCloud<pcl::PointXYZ>::Ptr &cloud)
{
  if (rens_->GetNumberOfItems () > 1)
  {
    PCL_WARN("[renderView] Method will render only the first viewport\n");
    return;
  }

  //create render window with first element of rens_ (viewport 0)
  vtkSmartPointer<vtkRenderWindow> render_win = vtkSmartPointer<vtkRenderWindow>::New ();
  rens_->InitTraversal ();
  vtkSmartPointer<vtkRenderer> renderer = rens_->GetNextItem ();
  render_win->AddRenderer (renderer);
  render_win->SetSize (xres, yres);
  renderer->SetBackground (1.0, 0.5, 0.5);

  vtkSmartPointer<vtkWorldPointPicker> worldPicker = vtkSmartPointer<vtkWorldPointPicker>::New ();

  render_win->Render ();

  cloud->points.resize (xres * yres);
  cloud->width = xres * yres;
  cloud->height = 1;

  double coords[3];
  float * depth = new float[xres * yres];
  render_win->GetZbufferData (0, 0, xres - 1, yres - 1, &(depth[0]));

  int count_valid_depth_pixels = 0;
  for (size_t x = 0; x < (size_t)xres; x++)
  {
    for (size_t y = 0; y < (size_t)yres; y++)
    {
      float value = depth[y * xres + x];

      if (value == 1.0)
        continue;

      worldPicker->Pick (x, y, value, renderer);
      worldPicker->GetPickPosition (coords);
      cloud->points[count_valid_depth_pixels].x = coords[0];
      cloud->points[count_valid_depth_pixels].y = coords[1];
      cloud->points[count_valid_depth_pixels].z = coords[2];
      count_valid_depth_pixels++;
    }
  }

  delete[] depth;

  cloud->points.resize (count_valid_depth_pixels);
  cloud->width = count_valid_depth_pixels;

  //transform cloud to give camera coordinates instead of world coordinates!
  vtkSmartPointer<vtkCamera> active_camera = renderer->GetActiveCamera ();
  vtkSmartPointer<vtkMatrix4x4> view_transform = active_camera->GetViewTransformMatrix ();
  Eigen::Matrix4f trans_view;
  trans_view.setIdentity ();

  for (size_t x = 0; x < 4; x++)
    for (size_t y = 0; y < 4; y++)
      trans_view (x, y) = view_transform->GetElement (x, y);

  //NOTE: vtk view coordinate system is different than the standard camera coordinates (z forward, y down, x right)
  //thus, the fliping in y and z
  for (size_t i = 0; i < cloud->points.size (); i++)
  {
    cloud->points[i].getVector4fMap () = trans_view * cloud->points[i].getVector4fMap ();
    cloud->points[i].y *= -1.0;
    cloud->points[i].z *= -1.0;
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////
bool
pcl::visualization::PCLVisualizer::fromHandlersToScreen (
    const GeometryHandlerConstPtr &geometry_handler,
    const ColorHandlerConstPtr &color_handler, 
    const std::string &id,
    int viewport)
{
  if (!geometry_handler->isCapable ())
  {
    PCL_WARN ("[fromHandlersToScreen] PointCloud <%s> requested with an invalid geometry handler (%s)!\n", id.c_str (), geometry_handler->getName ().c_str ());
    return (false);
  }

  if (!color_handler->isCapable ())
  {
    PCL_WARN ("[fromHandlersToScreen] PointCloud <%s> requested with an invalid color handler (%s)!\n", id.c_str (), color_handler->getName ().c_str ());
    return (false);
  }

  vtkSmartPointer<vtkPolyData> polydata;
  vtkSmartPointer<vtkIdTypeArray> initcells;
  // Convert the PointCloud to VTK PolyData
  convertPointCloudToVTKPolyData (geometry_handler, polydata, initcells);
  // use the given geometry handler
  polydata->Update ();

  // Get the colors from the handler
  vtkSmartPointer<vtkDataArray> scalars;
  color_handler->getColor (scalars);
  polydata->GetPointData ()->SetScalars (scalars);

  // Create an Actor
  vtkSmartPointer<vtkLODActor> actor;
  createActorFromVTKDataSet (polydata, actor);

  // Add it to all renderers
  addActorToRenderer (actor, viewport);

  // Save the pointer/ID pair to the global actor map
  (*cloud_actor_map_)[id].actor = actor;
  (*cloud_actor_map_)[id].cells = reinterpret_cast<vtkPolyDataMapper*>(actor->GetMapper ())->GetInput ()->GetVerts ()->GetData ();
  (*cloud_actor_map_)[id].geometry_handlers.push_back (geometry_handler);
  (*cloud_actor_map_)[id].color_handlers.push_back (color_handler);
  return (true);
}

//////////////////////////////////////////////////////////////////////////////////////////////
void
pcl::visualization::PCLVisualizer::updateCells (vtkSmartPointer<vtkIdTypeArray> &cells,
                                                vtkSmartPointer<vtkIdTypeArray> &initcells,
                                                vtkIdType nr_points)
{
  // If no init cells and cells has not been initialized...
  if (!cells)
    cells = vtkSmartPointer<vtkIdTypeArray>::New ();

  // If we have less values then we need to recreate the array
  if (cells->GetNumberOfTuples () < nr_points)
  {
    cells = vtkSmartPointer<vtkIdTypeArray>::New ();

    // If init cells is given, and there's enough data in it, use it
    if (initcells && initcells->GetNumberOfTuples () >= nr_points)
    {
      cells->DeepCopy (initcells);
      cells->SetNumberOfComponents (2);
      cells->SetNumberOfTuples (nr_points);
    }
    else
    {
      // If the number of tuples is still too small, we need to recreate the array
      cells->SetNumberOfComponents (2);
      cells->SetNumberOfTuples (nr_points);
      vtkIdType *cell = cells->GetPointer (0);
      // Fill it with 1s
      std::fill_n (cell, nr_points * 2, 1);
      cell++;
      for (vtkIdType i = 0; i < nr_points; ++i, cell += 2)
        *cell = i;
      // Save the results in initcells
      initcells = vtkSmartPointer<vtkIdTypeArray>::New ();
      initcells->DeepCopy (cells);
    }
  }
  else
  {
    // The assumption here is that the current set of cells has more data than needed
    cells->SetNumberOfComponents (2);
    cells->SetNumberOfTuples (nr_points);
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////
void
pcl::visualization::PCLVisualizer::allocVtkPolyData (vtkSmartPointer<vtkAppendPolyData> &polydata)
{
  polydata = vtkSmartPointer<vtkAppendPolyData>::New ();
}
//////////////////////////////////////////////////////////////////////////////////////////////
void
pcl::visualization::PCLVisualizer::allocVtkPolyData (vtkSmartPointer<vtkPolyData> &polydata)
{
  polydata = vtkSmartPointer<vtkPolyData>::New ();
}
//////////////////////////////////////////////////////////////////////////////////////////////
void
pcl::visualization::PCLVisualizer::allocVtkUnstructuredGrid (vtkSmartPointer<vtkUnstructuredGrid> &polydata)
{
  polydata = vtkSmartPointer<vtkUnstructuredGrid>::New ();
}

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
void
pcl::visualization::FPSCallback::Execute (vtkObject *caller, unsigned long, void*)
{
  vtkRenderer *ren = reinterpret_cast<vtkRenderer *> (caller);
  float fps = 1.0 / ren->GetLastRenderTimeInSeconds ();
  char buf[128];
  sprintf (buf, "%.1f FPS", fps);
  actor_->SetInput (buf);
}
