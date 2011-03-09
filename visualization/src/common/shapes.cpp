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
 * $Id: shapes.cpp 36093 2011-02-19 21:54:55Z rusu $
 *
 */
#include <pcl/visualization/common/shapes.h>

////////////////////////////////////////////////////////////////////////////////////////////
/** \brief Create a cylinder shape from a set of model coefficients.
  * \param coefficients the model coefficients (point_on_axis, axis_direction, radius)
  * \param numsides (optional) the number of sides used for rendering the cylinder
  */
vtkSmartPointer<vtkDataSet> 
  pcl_visualization::createCylinder (const pcl::ModelCoefficients &coefficients, int numsides)
{
  vtkSmartPointer<vtkLineSource> line = vtkSmartPointer<vtkLineSource>::New ();
  line->SetPoint1 (coefficients.values[0], coefficients.values[1], coefficients.values[2]);
  line->SetPoint2 (coefficients.values[3]+coefficients.values[0], coefficients.values[4]+coefficients.values[1], coefficients.values[5]+coefficients.values[2]);

  vtkSmartPointer<vtkTubeFilter> tuber = vtkSmartPointer<vtkTubeFilter>::New ();
  tuber->SetInputConnection (line->GetOutputPort ());
  tuber->SetRadius (coefficients.values[6]);
  tuber->SetNumberOfSides (numsides);

  return (tuber->GetOutput ());
}

/** \brief Create a sphere shape from a set of model coefficients.
  * \param coefficients the model coefficients (sphere center, radius)
  * \param res (optional) the resolution used for rendering the model
  */
vtkSmartPointer<vtkDataSet> 
  pcl_visualization::createSphere (const pcl::ModelCoefficients &coefficients, int res)
{
  // Set the sphere origin
  vtkSmartPointer<vtkTransform> t = vtkSmartPointer<vtkTransform>::New ();
  t->Identity (); t->Translate (coefficients.values[0], coefficients.values[1], coefficients.values[2]);

  vtkSmartPointer<vtkSphereSource> s_sphere = vtkSmartPointer<vtkSphereSource>::New ();
  s_sphere->SetRadius (coefficients.values[3]);
  s_sphere->SetPhiResolution (res);
  s_sphere->SetThetaResolution (res);
  s_sphere->LatLongTessellationOff ();
  
  vtkSmartPointer<vtkTransformPolyDataFilter> tf = vtkSmartPointer<vtkTransformPolyDataFilter>::New ();
  tf->SetTransform (t);
  tf->SetInputConnection (s_sphere->GetOutputPort ());

  return (tf->GetOutput ());
}

/** \brief Create a line shape from a set of model coefficients.
  * \param coefficients the model coefficients (point_on_line, line_direction)
  */
vtkSmartPointer<vtkDataSet> 
  pcl_visualization::createLine (const pcl::ModelCoefficients &coefficients)
{
  vtkSmartPointer<vtkLineSource> line = vtkSmartPointer<vtkLineSource>::New ();
  line->SetPoint1 (coefficients.values[0], coefficients.values[1], coefficients.values[2]);
  line->SetPoint2 (coefficients.values[3] + coefficients.values[0], 
                   coefficients.values[4] + coefficients.values[1], 
                   coefficients.values[5] + coefficients.values[2]);
  line->Update ();

  return (line->GetOutput ());
}

/** \brief Create a planar shape from a set of model coefficients.
  * \param coefficients the model coefficients (a, b, c, d with ax+by+cz+d=0)
  */
vtkSmartPointer<vtkDataSet> 
  pcl_visualization::createPlane (const pcl::ModelCoefficients &coefficients)
{
  vtkSmartPointer<vtkPlaneSource> plane = vtkSmartPointer<vtkPlaneSource>::New ();
  plane->SetNormal (coefficients.values[0], coefficients.values[1], coefficients.values[2]);
  plane->Push (coefficients.values[3]);
  return (plane->GetOutput ());
}

/** \brief Create a 2d circle shape from a set of model coefficients.
  * \param coefficients the model coefficients (x, y, radius)
  * \param z (optional) specify a z value (default: 0)
  */
vtkSmartPointer<vtkDataSet> 
  pcl_visualization::create2DCircle (const pcl::ModelCoefficients &coefficients, double z)
{
  vtkSmartPointer<vtkDiskSource> disk = vtkSmartPointer<vtkDiskSource>::New ();
  // Maybe the resolution should be lower e.g. 50 or 25 
  disk->SetCircumferentialResolution (100);
  disk->SetInnerRadius (coefficients.values[2] - 0.001);
  disk->SetOuterRadius (coefficients.values[2] + 0.001);
  disk->SetCircumferentialResolution (20);

  // An alternative to <vtkDiskSource> could be <vtkRegularPolygonSource> with <vtkTubeFilter>
  /*
  vtkSmartPointer<vtkRegularPolygonSource> circle = vtkSmartPointer<vtkRegularPolygonSource>::New();
  circle->SetRadius (coefficients.values[2]);
  circle->SetNumberOfSides (100);
  
  vtkSmartPointer<vtkTubeFilter> tube = vtkSmartPointer<vtkTubeFilter>::New();
  tube->SetInput (circle->GetOutput());
  tube->SetNumberOfSides (25);
  tube->SetRadius (0.001);
  */ 

  // Set the circle origin
  vtkSmartPointer<vtkTransform> t = vtkSmartPointer<vtkTransform>::New ();
  t->Identity (); 
  t->Translate (coefficients.values[0], coefficients.values[1], z);

  vtkSmartPointer<vtkTransformPolyDataFilter> tf = vtkSmartPointer<vtkTransformPolyDataFilter>::New ();
  tf->SetTransform (t);
  tf->SetInputConnection (disk->GetOutputPort ());
  /*
  tf->SetInputConnection (tube->GetOutputPort ());
  */

  return (tf->GetOutput ());
}

/** \brief Create a cone shape from a set of model coefficients.
  * \param coefficients the cone coefficients (point_on_axis, axis_direction, radius))
  */
vtkSmartPointer<vtkDataSet> 
  pcl_visualization::createCone (const pcl::ModelCoefficients &coefficients)
{
  vtkSmartPointer<vtkConeSource> cone = vtkSmartPointer<vtkConeSource>::New ();
  cone->SetHeight (1.0);
  cone->SetCenter (coefficients.values[0] + coefficients.values[3] * 0.5, 
                   coefficients.values[1] + coefficients.values[1] * 0.5,
                   coefficients.values[2] + coefficients.values[2] * 0.5);
  cone->SetDirection (-coefficients.values[3], -coefficients.values[4], -coefficients.values[5]);
  cone->SetResolution (100);
  cone->SetAngle (coefficients.values[6]);

  return (cone->GetOutput ());
}

