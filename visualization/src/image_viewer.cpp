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
 * Author: Suat Gedikli (gedikli@willowgarage.com)
 */

#include <pcl/visualization/image_viewer.h>
#include <vtkImageImport.h>
#include <vtkImageViewer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkImageReslice.h>
#include <vtkImageViewer.h>
#include <vtkTransform.h>
#include <vtkImageChangeInformation.h>

/////////////////////////////////////////////////////////////////////////////////////////////
pcl::visualization::ImageViewer::ImageViewer (const std::string& window_title)
  : image_viewer_ (vtkImageViewer::New ())
{
  image_viewer_->GetRenderWindow ()->SetWindowName (window_title.c_str ());

  vtkRenderWindowInteractor *iren = vtkRenderWindowInteractor::New ();
  image_viewer_->SetupInteractor (iren);

  iren->SetRenderWindow (image_viewer_->GetRenderWindow ());
  iren->Initialize ();
}

/////////////////////////////////////////////////////////////////////////////////////////////
void
pcl::visualization::ImageViewer::showRGBImage (const unsigned char* rgb_data, unsigned width, unsigned height)
{
  vtkImageImport* importer = vtkImageImport::New ();
  importer->SetNumberOfScalarComponents (3);
  importer->SetWholeExtent (0, width - 1, 0, height - 1, 0, 0);
  importer->SetDataScalarTypeToUnsignedChar ();
  importer->SetDataExtentToWholeExtent ();

  void* data = const_cast<void*> ((const void*)rgb_data);
  importer->SetImportVoidPointer (data, 1);
  importer->Update ();

  vtkSmartPointer<vtkMatrix4x4> transform = vtkSmartPointer<vtkMatrix4x4>::New ();
  transform->Identity ();
  transform->SetElement (1,1, -1.0);
  transform->SetElement (1,3, height);
  vtkSmartPointer<vtkTransform> imageTransform = vtkSmartPointer<vtkTransform>::New ();
  imageTransform->SetMatrix (transform);
  //imageTransform->Translate (width / 2.0, height / 2.0, 0.0);
  //imageTransform->RotateZ (180.0);
  // Now create filter and set previously created transformation
  vtkSmartPointer<vtkImageReslice> algo = vtkSmartPointer<vtkImageReslice>::New ();
  algo->SetInput (importer->GetOutput ());
  algo->SetInformationInput (importer->GetOutput ());
  algo->SetResliceTransform (imageTransform);
  algo->SetInterpolationModeToCubic ();
  algo->Update ();

  image_viewer_->SetInput (algo->GetOutput ());
  //image_viewer_->SetInput (importer->GetOutput());
  image_viewer_->SetColorLevel (127.5);
  image_viewer_->SetColorWindow (255);
  image_viewer_->SetSize (width, height);

  image_viewer_->Render ();
}

