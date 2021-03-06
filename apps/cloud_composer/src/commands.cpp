
#include <pcl/apps/cloud_composer/commands.h>
#include <pcl/apps/cloud_composer/tool_interface/abstract_tool.h>
#include <pcl/apps/cloud_composer/project_model.h>
#include <QDebug>

pcl::cloud_composer::CloudCommand::CloudCommand (QList <const CloudComposerItem*> input_data, QUndoCommand* parent)
  : QUndoCommand (parent)
  , original_data_ (input_data)
{
  
}

void
pcl::cloud_composer::CloudCommand::setProjectModel (ProjectModel* model)
{
  project_model_ = model;
}

//////////// MODIFY CLOUD COMMAND

pcl::cloud_composer::ModifyCloudCommand::ModifyCloudCommand (QList <const CloudComposerItem*> input_data, QUndoCommand* parent)
  : CloudCommand (input_data, parent)
{  
}

bool
pcl::cloud_composer::ModifyCloudCommand::runCommand (AbstractTool* tool)
{
  
  
}

void
pcl::cloud_composer::ModifyCloudCommand::undo ()
{
  
}

void
pcl::cloud_composer::ModifyCloudCommand::redo ()
{
  
}

//////////// Create CLOUD COMMAND

pcl::cloud_composer::NewItemCloudCommand::NewItemCloudCommand (QList <const CloudComposerItem*> input_data, QUndoCommand* parent)
  : CloudCommand (input_data, parent)
{
  
}

bool
pcl::cloud_composer::NewItemCloudCommand::runCommand (AbstractTool* tool)
{
  this->setText (tool->getToolName ());
  //For new item cloud command, each selected item should be processed separately
  //e.g. calculate normals for every selected cloud
  foreach (const CloudComposerItem *item, original_data_)
  {
    qDebug () <<"input pointer = "<<item;
    qDebug () << item->text();
    //Check to see if this is a cloud
    QList <const CloudComposerItem*> working_copy;
    working_copy.append (item);
    QList <CloudComposerItem*> output = tool->performAction (working_copy);
    if (output.size () == 0)
      qWarning () << "Warning: Tool " << tool->getToolName () << "returned no item in a NewItemCloudCommand";
    else if (output.size () == 1)
    {
      OutputPair output_pair = {item, output};
      output_data_.append (output_pair);
    }
    else
    {
      qWarning () << "Warning: Tool " << tool->getToolName () << "returned multiple items in a NewItemCloudCommand";
    }
    
  }
 // if (output_data_.size () > 0)
    return true;
  
  return false;
  
}

void
pcl::cloud_composer::NewItemCloudCommand::undo ()
{
  qDebug () << "Undo in NewItemCloudCommand";
  foreach (OutputPair output_pair, output_data_)
  {
    const CloudComposerItem* input_item = output_pair.input_item_;
    QList <CloudComposerItem*> output_items = output_pair.output_list_;
    //Find the input_item index in the project_model_
    QModelIndex input_index = project_model_->indexFromItem (input_item);
    if (!input_index.isValid ())
    {
      qCritical () << "Index of input cloud item is no longer valid upon command completion!";
      return;
    }
    QStandardItem* item_to_change = project_model_->itemFromIndex (input_index);
    foreach (CloudComposerItem* output_item, output_items)
    {
      QModelIndex output_index = project_model_->indexFromItem (output_item);
      //item_to_change->takeChild (output_index.row (),output_index.column ());
      item_to_change->takeRow (output_index.row ());
    }
    
  }
}

void
pcl::cloud_composer::NewItemCloudCommand::redo ()
{
  qDebug () << "Redo in NewItemCloudCommand - output data size="<<output_data_.size ();
  foreach (OutputPair output_pair, output_data_)
  {
    const CloudComposerItem* input_item = output_pair.input_item_;
    QList <CloudComposerItem*> output_items = output_pair.output_list_;
    //Find the input_item index in the project_model_
    QPersistentModelIndex input_index = QPersistentModelIndex(project_model_->indexFromItem (input_item));
    
    if (!input_index.isValid ())
    {
      qCritical () << "Index of input cloud item is no longer valid upon command completion!";
      return;
    }
    foreach (CloudComposerItem* output_item, output_items)
    {
      (project_model_->itemFromIndex (input_index))->appendRow (output_item);
      QPersistentModelIndex output_index = QPersistentModelIndex(project_model_->indexFromItem (output_item));
      if(!output_index.isValid())
        qDebug () << "OUTPUT INDEX NOT VALID";
      input_index = project_model_->indexFromItem (input_item);
    }
  }
}