.. toctree::
  
The following links describe a set of basic PCL tutorials. Please note that
their source codes may already be provided as part of the PCL regular releases,
so check there before you start copy & pasting the code. The list of tutorials
below is automatically generated from reST files located in our SVN repository.

.. note::

  Before you start reading, please make sure that you go through the higher-level overview documentation at http://www.pointclouds.org/documentation/, under **Getting Started**. Thank you.

As always, we would be happy to hear your comments and receive your
contributions on any tutorial.

* Basic Usage

  * :ref:`using_pcl`

     ======  ======
     |mi_1|  Title: **Using PCL in own project**

             Author: *Nizar Sallem*

             Compatibility: > PCL 1.0
             
             In this tutorial, we will learn how to link your own project to PCL using cmake.
     ======  ======

     .. |mi_1| image:: images/pcl_logo.png
               :height: 75px

  * :ref:`building_pcl`

     ======  ======
     |mi_2|  Title: **Explaining PCL's cmake options**

             Author: *Nizar Sallem*

             Compatibility: > PCL 1.0
             
             In this tutorial, we will explain the basic PCL cmake options, and ways to tweak them to fit your project.
     ======  ======

     .. |mi_2| image:: images/pcl_ccmake.png
               :height: 100px

  * :ref:`compiling_pcl_windows`

     ======  ======
     |mi_3|  Title: **Compiling PCL and its dependencies from source in Windows**

             Author: *Alessio Placitelli*

             Compatibility: > PCL 1.0
             
             In this tutorial, we will explain how to compile PCL and all its 3rd party dependencies from source in Microsoft Windows.
     ======  ======

     .. |mi_3| image:: images/windows_logo.png
               :height: 100px

* Advanced Usage

  * :ref:`adding_custom_ptype`

     ======  ======
     |au_1|  Title: **Adding your own custom PointT point type**

             Author: *Radu B. Rusu*

             Compatibility: > PCL 0.9, < PCL 2.0
             
             This document explains what templated point types are in PCL, why do they exist, and how to create and use your own `PointT` point type.
     ======  ======

     .. |au_1| image:: images/pcl_logo.png
               :height: 75px
* I/O

  * :ref:`pcd_file_format`

     ======  ======
     |i_o0|  Title: **The PCD (Point Cloud Data) file format**

             Author: *Radu B. Rusu*

             Compatibility: > PCL 0.9

             This document describes the PCD file format, and the way it is used inside PCL.
     ======  ======
     
     .. |i_o0| image:: images/PCD_icon.png
               :height: 100px

  * :ref:`reading_pcd`

     ======  ======
     |i_o1|  Title: **Reading Point Cloud data from PCD files**

             Author: *Radu B. Rusu*

             Compatibility: > PCL 0.5

             In this tutorial, we will learn how to read a Point Cloud from a PCD file.
     ======  ======
     
     .. |i_o1| image:: images/read_pcd.jpg
               :height: 100px

  * :ref:`writing_pcd`

     ======  ======
     |i_o2|  Title: **Writing Point Cloud data to PCD files**

             Author: *Radu B. Rusu*

             Compatibility: > PCL 0.5

             In this tutorial, we will learn how to write a Point Cloud to a PCD file.
     ======  ======
     
     .. |i_o2| image:: images/write_pcd.jpg
               :height: 100px

  * :ref:`concatenate_fields`

     ======  ======
     |i_o3|  Title: **Concatenate the fields of two Point Clouds**

             Author: *Radu B. Rusu*

             Compatibility: > PCL 0.5

             In this tutorial, we will learn how to concatenate the fields of two Point Clouds, one containing only *XYZ* data, and one containing *Surface Normal* information.
     ======  ======

     .. |i_o3| image:: images/concatenate_fields.jpg
               :height: 100px

  * :ref:`concatenate_points`

     ======  ======
     |i_o4|  Title: **Concatenate the points of two Point Clouds**

             Author: *Radu B. Rusu*

             Compatibility: > PCL 0.5

             In this tutorial, we will learn how to concatenate the point data of two Point Clouds with the same fields.
     ======  ======

     .. |i_o4| image:: images/concatenate_data.jpg
               :height: 100px

  * :ref:`openni_grabber`

     ======  ======
     |i_o5|  Title: **Grabbing Point Clouds from an OpenNI camera**

             Author: *Nico Blodow*

             Compatibility: > PCL 1.0

             In this tutorial, we will learn how to acquire point cloud data from an OpenNI camera.
     ======  ======

     .. |i_o5| image:: images/openni_grabber.png
               :height: 100px

* Filtering

  * :ref:`passthrough`
    
     ======  ======
     |fi_1|  Title: **Filtering a PointCloud using a PassThrough filter**

             Author: *Radu B. Rusu*

             Compatibility: > PCL 0.5

             In this tutorial, we will learn how to remove points whose values fall inside/outside a user given interval along a specified dimension.
     ======  ======
     
     .. |fi_1| image:: images/passthrough.jpg
               :height: 100px

  * :ref:`voxelgrid`
    
     ======  ======
     |fi_2|  Title: **Downsampling a PointCloud using a VoxelGrid filter**

             Author: *Radu B. Rusu*

             Compatibility: > PCL 0.5

             In this tutorial, we will learn how to downsample (i.e., reduce the number of points) a Point Cloud.
     ======  ======
     
     .. |fi_2| image:: images/voxel_grid.jpg
               :height: 100px
    

  * :ref:`statistical_outlier_removal` 
    
     ======  ======
     |fi_3|  Title: **Removing outliers using a StatisticalOutlierRemoval filter**

             Author: *Radu B. Rusu*

             Compatibility: > PCL 0.5

             In this tutorial, we will learn how to remove sparse outliers from noisy data, using statistical analysis.
     ======  ======
     
     .. |fi_3| image:: images/statistical_removal.jpg
               :height: 100px

  * :ref:`project_inliers`

     ======  ======
     |fi_4|  Title: **Projecting points using a parametric model**

             Author: *Radu B. Rusu*

             Compatibility: > PCL 0.5

             In this tutorial, we will learn how to project points to a parametric model (i.e., plane).
     ======  ======
     
     .. |fi_4| image:: images/project_inliers.jpg
               :height: 100px

  * :ref:`extract_indices`

     ======  ======
     |fi_5|  Title: **Extracting indices from a PointCloud**

             Author: *Radu B. Rusu*

             Compatibility: > PCL 0.5

             In this tutorial, we will learn how to extract a set of indices given by a segmentation algorithm.
     ======  ======
     
     .. |fi_5| image:: images/extract_indices.jpg
               :height: 100px

* Features

  * :ref:`features_tutorial`
    
     ======  ======
     |fe_1|  Title: **How 3D features work**

             Author: *Radu B. Rusu*

             Compatibility: > PCL 1.0

             This document presents a basic introduction to the 3D feature estimation methodologies in PCL.
     ======  ======
     
     .. |fe_1| image:: images/good_features_small.png
               :height: 100px

  
* Segmentation

  * :ref:`planar_segmentation`
    
     ======  ======
     |se_1|  Title: **Planar model segmentation**

             Author: *Radu B. Rusu*

             Compatibility: > PCL 0.5

             In this tutorial, we will learn how to segment arbitrary planar models from a given point cloud dataset.
     ======  ======
     
     .. |se_1| image:: images/planar_segmentation.jpg
               :height: 100px

  * :ref:`cylinder_segmentation`

     ======  ======
     |se_2|  Title: **Cylinder model segmentation**

             Author: *Radu B. Rusu*

             Compatibility: > PCL 0.5

             In this tutorial, we will learn how to segment arbitrary cylindrical models from a given point cloud dataset.
     ======  ======
     
     .. |se_2| image:: images/cylinder_segmentation.jpg
               :height: 100px

* Surface

  * :ref:`moving_least_squares`

     ======  ======
     |su_1|  Title: **Smoothing and normal estimation based on polynomial reconstruction**

             Author: *Zoltan-Csaba Marton*

             Compatibility: > PCL 0.5

             In this tutorial, we will learn how to construct and run a Moving Least Squares (MLS) algorithm to obtain smoothed XYZ coordinates and normals.
     ======  ======
     
     .. |su_1| image:: images/resampling.jpg
               :height: 100px

  * :ref:`convex_hull_2d`

     ======  ======
     |su_2|  Title: **Construct a convex hull polygon for a planar model**

             Author: *Radu B. Rusu*

             Compatibility: > PCL 0.5

             In this tutorial we will learn how to calculate a simple 2D convex hull polygon for a set of points supported by a plane.
     ======  ======
     
     .. |su_2| image:: images/convex_hull_2d.jpg
               :height: 100px

  * :ref:`greedy_triangulation`

     ======  ======
     |su_3|  Title: **Fast triangulation of unordered point clouds**

             Author: *Zoltan-Csaba Marton*

             Compatibility: > PCL 0.5

             In this tutorial we will learn how to run a greedy triangulation algorithm on a PointCloud with normals to obtain a triangle mesh based on projections of the local neighborhood.
     ======  ======
     
     .. |su_3| image:: images/greedy_triangulation.png
               :height: 100px

* Octree

  * :ref:`octree_search`
  
     ======  ======
     |oc_1|  Title: **Octrees for spatial partitioning and neighbor search**

             Author: *Julius Kammerl*

             Compatibility: > PCL 0.5

             In this tutorial, we will learn how to use octrees for spatial partitioning and nearest neighbor search.
     ======  ======
     
     .. |oc_1| image:: images/octree_img.png
               :height: 100px
               
  * :ref:`octree_change_detection`
  
     ======  ======
     |oc_2|  Title: **Spatial change detection on unorganized point cloud data**

             Author: *Julius Kammerl*

             Compatibility: > PCL 0.5

             In this tutorial, we will learn how to use octrees for detecting spatial changes within point clouds.
     ======  ======
     
     .. |oc_2| image:: images/changedetectionThumb.png
               :height: 100px

* Visualization

  * :ref:`cloud_viewer`

     ======  ======
     |vi_1|  Title: **Visualizing Point Clouds**

             Author: *Ethan Rublee*

             Compatibility: > PCL 1.0

             This tutorial demonstrates how to use the pcl visualization tools.
     ======  ======

     .. |vi_1| image:: images/cloud_viewer.jpg
               :height: 100px

* Applications

  * :ref:`template_alignment`

     ======  ======
     |ap_1|  Title: **Aligning object templates to a point cloud**

             Author: *Michael Dixon*

             Compatibility: > PCL 1.0

             This tutorial gives an example of how some of the tools covered in the previous tutorials can be combined to solve a higher level problem --- aligning a previously captured model of an object to some newly captured data.
     ======  ======

     .. |ap_1| image:: images/template_alignment_1.png
               :height: 100px

 ..
  * :ref:`normal_estimation_integral_images`
    Surface normal estimation
  * Range Image
    * :ref:`range_image_visualization`
      How to visualize a range image
    * :ref:`range_image_creation`
      How to create a range image from a point cloud
    * :ref:`range_image_border_extraction`
      How to extract borders from range images
    * :ref:`narf_keypoint`
      How to extract NARF keypoints from a range image
    * :ref:`narf_descriptor`
      How to extract NARF descriptors from points in a range images
    * :ref:`narf_descriptor_visualization`
      Visualization of how the NARF descriptor is calculated and of the descriptor distances to a marked point.
    * :ref:`octree_search`
      Octrees for spatial partitioning and neighbor search.     

