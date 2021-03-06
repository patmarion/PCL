/*
 * Software License Agreement  (BSD License)
 *
 *  Point Cloud Library  (PCL) - www.pointclouds.org
 *  Copyright  (c) 2012, Jeremie Papon.
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
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES  (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef COMMANDS_H_
#define COMMANDS_H_

#include <QUndoCommand>

#include <pcl/apps/cloud_composer/items/cloud_composer_item.h>

typedef QList<const pcl::cloud_composer::CloudComposerItem*> ConstItemList;
namespace pcl
{
  namespace cloud_composer
  {
    class AbstractTool;
    class ProjectModel;
    struct OutputPair
    {
      const CloudComposerItem* input_item_;
      QList <CloudComposerItem*> output_list_;
    };
    class CloudCommand : public QUndoCommand
    {
      public: 
        CloudCommand (ConstItemList input_data, QUndoCommand* parent = 0);
        
        virtual bool
        runCommand (AbstractTool* tool) = 0;

        virtual void 
        undo ()  = 0;
        
        virtual void
        redo () = 0;
        
        void 
        setProjectModel (ProjectModel* model);
      protected:
        ConstItemList original_data_;
        QList <OutputPair> output_data_;
        ProjectModel* project_model_;
    };
    
    class ModifyCloudCommand : public CloudCommand
    {
      public: 
        ModifyCloudCommand (ConstItemList input_data, QUndoCommand* parent = 0);
    
        virtual bool
        runCommand (AbstractTool* tool);
        
        virtual void
        undo ();
      
        virtual void
        redo ();
      
      
      
    };
    
    class NewItemCloudCommand : public CloudCommand
    {
      public: 
        NewItemCloudCommand (ConstItemList input_data, QUndoCommand* parent = 0);
      
        virtual bool
        runCommand (AbstractTool* tool);
        
        virtual void
        undo ();
      
        virtual void
        redo ();

    };
  }
} 

Q_DECLARE_METATYPE (ConstItemList);
#endif //COMMANDS_H_