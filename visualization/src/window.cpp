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

#include <pcl/visualization/window.h>
#include <vtkCallbackCommand.h>
#include <vtkObject.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <pcl/visualization/keyboard_event.h>
#include <pcl/visualization/mouse_event.h>
#include <pcl/common/time.h>
#include <pcl/visualization/interactor.h>


/////////////////////////////////////////////////////////////////////////////////////////////
pcl::visualization::Window::Window (const std::string& window_name)
  : mouse_command_ (vtkCallbackCommand::New ()), 
    keyboard_command_ (vtkCallbackCommand::New ()),
    style_ (vtkSmartPointer<pcl::visualization::PCLVisualizerInteractorStyle>::New ()),
    rens_ (vtkSmartPointer<vtkRendererCollection>::New ())
{
  mouse_command_->SetClientData (this);
  mouse_command_->SetCallback (Window::MouseCallback);
  
  keyboard_command_->SetClientData (this);
  keyboard_command_->SetCallback (Window::KeyboardCallback);

  // Create a RendererWindow
  win_ = vtkSmartPointer<vtkRenderWindow>::New ();
  win_->SetWindowName (window_name.c_str ());
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

  // Create the interactor style
  style_->Initialize ();
  style_->setRendererCollection (rens_);
  style_->UseTimersOn ();

 // Create the interactor
  //interactor_ = vtkSmartPointer<vtkRenderWindowInteractor>::New ();
  interactor_ = vtkSmartPointer<PCLVisualizerInteractor>::New ();

  interactor_->SetRenderWindow (win_);
  interactor_->SetInteractorStyle (style_);
  interactor_->SetDesiredUpdateRate (30.0);
  // Initialize and create timer
  interactor_->Initialize ();
  //interactor_->CreateRepeatingTimer (5000L);
  interactor_->timer_id_ = interactor_->CreateRepeatingTimer (5000L);
  //interactor_->timer_id_ = interactor_->CreateRepeatingTimer (30L);
  
  exit_main_loop_timer_callback_ = vtkSmartPointer<ExitMainLoopTimerCallback>::New ();
  exit_main_loop_timer_callback_->window = this;
  exit_main_loop_timer_callback_->right_timer_id = -1;
  interactor_->AddObserver (vtkCommand::TimerEvent, exit_main_loop_timer_callback_);

  exit_callback_ = vtkSmartPointer<ExitCallback>::New ();
  exit_callback_->window = this;
  interactor_->AddObserver (vtkCommand::ExitEvent, exit_callback_);

  resetStoppedFlag ();
}

/////////////////////////////////////////////////////////////////////////////////////////////
pcl::visualization::Window::~Window ()
{
  interactor_->DestroyTimer (interactor_->timer_id_);
}

/////////////////////////////////////////////////////////////////////////////////////////////
void 
pcl::visualization::Window::spin ()
{
  resetStoppedFlag ();
  // Render the window before we start the interactor
  win_->Render ();
  interactor_->Start ();
}

/////////////////////////////////////////////////////////////////////////////////////////////
void
pcl::visualization::Window::spinOnce (int time, bool force_redraw)
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
boost::signals2::connection 
pcl::visualization::Window::registerMouseCallback (boost::function<void (const pcl::visualization::MouseEvent&)> callback)
{
  // just add observer at first time when a callback is registered
  if (mouse_signal_.empty())
  {
    interactor_->AddObserver(vtkCommand::MouseMoveEvent, mouse_command_);
    interactor_->AddObserver(vtkCommand::MiddleButtonPressEvent, mouse_command_);
    interactor_->AddObserver(vtkCommand::MiddleButtonReleaseEvent, mouse_command_);
    interactor_->AddObserver(vtkCommand::MouseWheelBackwardEvent, mouse_command_);
    interactor_->AddObserver(vtkCommand::MouseWheelForwardEvent, mouse_command_);
    interactor_->AddObserver(vtkCommand::LeftButtonPressEvent, mouse_command_);
    interactor_->AddObserver(vtkCommand::LeftButtonReleaseEvent, mouse_command_);
    interactor_->AddObserver(vtkCommand::RightButtonPressEvent, mouse_command_);
    interactor_->AddObserver(vtkCommand::RightButtonReleaseEvent, mouse_command_);
  }
  return mouse_signal_.connect(callback);
}

/////////////////////////////////////////////////////////////////////////////////////////////
boost::signals2::connection 
pcl::visualization::Window::registerKeyboardCallback (boost::function<void (const pcl::visualization::KeyboardEvent&)> callback)
{
  // just add observer at first time when a callback is registered
  if (keyboard_signal_.empty())
  {
    interactor_->AddObserver(vtkCommand::KeyPressEvent, keyboard_command_);
    interactor_->AddObserver(vtkCommand::KeyReleaseEvent, keyboard_command_);
  }
  
  return keyboard_signal_.connect(callback);
}

/////////////////////////////////////////////////////////////////////////////////////////////
void 
pcl::visualization::Window::emitMouseEvent (unsigned long event_id)
{
  int x,y;
  interactor_->GetMousePosition (&x, &y);
  MouseEvent event (MouseEvent::MouseMove, MouseEvent::NoButton, x, y, interactor_->GetAltKey (), interactor_->GetControlKey (), interactor_->GetShiftKey ());
  bool repeat = false;
  switch (event_id)
  {
    case vtkCommand::MouseMoveEvent :
      event.setType(MouseEvent::MouseMove);
      break;
    
    case vtkCommand::LeftButtonPressEvent :
      event.setButton(MouseEvent::LeftButton);
      if (interactor_->GetRepeatCount () == 0)
        event.setType(MouseEvent::MouseButtonPress);
      else
        event.setType(MouseEvent::MouseDblClick);
      break;
      
    case vtkCommand::LeftButtonReleaseEvent :
      event.setButton(MouseEvent::LeftButton);
      event.setType(MouseEvent::MouseButtonRelease);
      break;
      
    case vtkCommand::RightButtonPressEvent :
      event.setButton(MouseEvent::RightButton);
      if (interactor_->GetRepeatCount () == 0)
        event.setType(MouseEvent::MouseButtonPress);
      else
        event.setType(MouseEvent::MouseDblClick);
      break;
      
    case vtkCommand::RightButtonReleaseEvent :
      event.setButton(MouseEvent::RightButton);
      event.setType(MouseEvent::MouseButtonRelease);
      break;
      
    case vtkCommand::MiddleButtonPressEvent :
      event.setButton(MouseEvent::MiddleButton);
      if (interactor_->GetRepeatCount () == 0)
        event.setType(MouseEvent::MouseButtonPress);
      else
        event.setType(MouseEvent::MouseDblClick);
      break;
      
    case vtkCommand::MiddleButtonReleaseEvent :
      event.setButton(MouseEvent::MiddleButton);
      event.setType(MouseEvent::MouseButtonRelease);
      break;
      
      case vtkCommand::MouseWheelBackwardEvent :
      event.setButton(MouseEvent::VScroll);
      event.setType(MouseEvent::MouseScrollDown);
      if (interactor_->GetRepeatCount () != 0)
        repeat = true;
      break;
      
    case vtkCommand::MouseWheelForwardEvent :
      event.setButton(MouseEvent::VScroll);
      event.setType(MouseEvent::MouseScrollUp);
      if (interactor_->GetRepeatCount () != 0)
        repeat = true;
      break;
    default:
      return;
  }
  
  mouse_signal_ (event);
  if (repeat)
    mouse_signal_ (event);
}

/////////////////////////////////////////////////////////////////////////////////////////////
void 
pcl::visualization::Window::emitKeyboardEvent (unsigned long event_id)
{
  KeyboardEvent event (bool(event_id == vtkCommand::KeyPressEvent), interactor_->GetKeySym (), interactor_->GetKeyCode (), interactor_->GetAltKey (), interactor_->GetControlKey (), interactor_->GetShiftKey ());
  keyboard_signal_ (event);
}

/////////////////////////////////////////////////////////////////////////////////////////////
void 
pcl::visualization::Window::MouseCallback(vtkObject*, unsigned long eid, void* clientdata, void* calldata)
{
  Window* window = reinterpret_cast<Window*> (clientdata);
  window->emitMouseEvent (eid);
}

/////////////////////////////////////////////////////////////////////////////////////////////
void 
pcl::visualization::Window::KeyboardCallback(vtkObject*, unsigned long eid, void* clientdata, void* calldata)
{
  Window* window = reinterpret_cast<Window*> (clientdata);
  window->emitKeyboardEvent (eid);
}
