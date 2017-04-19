/*=========================================================================

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/VolViewCopyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*
  This is a skeleton plugin that includes basic instructions on what you need
  to do to create a plugin. You will want to look through vtkVVPluginAPI.h as
  well to get a feel for how it works.
 
  1) You should save a copy of this file with the name of the plugin you want
      to create. Say you want to create a luminance plugin, then you should
      save this file as vvLuminance.cxx or vvJohnDoeLuminance.cxx in these
      two examples your plugin would be named Luminance or JohnDoeLuminance
      respectively
      
  2) Rename a few occurances of Sample in this file to the name of your
     plugin.  Specifically look for the "TODO: Rename" comments in the rest
     of this file and where you see <your_plugin> replace that with the name
     of your plugin (such as Luminance). This is TODO items 1 through 4
     
  3) Update the terse and full documentation strings for your plugin. This is
     TODO item 5. The terse documentation should be a quick one sentence
     description of your plugin. The full documentation should be a long
     description of what the plugin does, any limitations it has, and who to
     contact with questions.
  
  4) Inside the UpdateGUI function there is a block of code folloing TODO
     item 6: this code specifies the properties of the output of this
     plugin. The implementaiton currently provided indicates that the output
     volume has all the same properties as the input volume. Properties
     include the volume's dimensions, spacing, origin, number of components
     per voxel, and data type (short, int, float). If for example your plugin
     always creates floating point output then you would want to set
     info->OutputVolumeScalarType = VTK_FLOAT (note that while we use
     VTK_FLOAT to indicate floating point data your plugin does not need any
     relationship to VTK (The Visualization Toolkit) itself)
  
  5) Create your GUI elements. This involves two main steps. First you need
     to determine how many GUI items you will have. A GUI item can be an
     option menu, checkbox or slider. Once you have determined how many gui
     items you will have you need to set that value in TODO item 7. The you
     need to actually spcify the GUI items. This is adeed to the UpdateGUI
     function at TODO item 8. There are a number of options here and I
     recommend looking at the source code to the other plugins to get an idea
     for how to create the GUI items.
     
  6) Now you are ready to think about how your filter will process its
     data. There are a couple options here that you can adjust. Depending on
     the nature of your algorithm you might be able to process the volume
     in-place, in-pieces, or all-at-once. The most memory efficient option is
     to support processing the input volume in-place but fo rthis to work
     your algorithm must not change the parameters of the volume (see 4)
     because the input volume's block of memory will be used for the output
     volume. Typically only simple algorithms can be in-place. The next
     option is in-pieces. This means that you will be asked to process the
     input volume in pieces and ProcessData will be called one time for each
     piece. In this case the memory is not shared between the input and
     output so they can have different parameters. The third option
     all-at-once is the simplest but consumes the most memory (both the input
     volume and output volume must be in memory at the same time). With
     all-at-once there really are no limitations so it is probably the best
     option for your first attempt at writing a plugin. Once you have made
     your decision you need to specify it at TODO item 9.

  7) Now you can write your algorithm in the vv<your_plugin>Template
     function. If you are unfamilliar with C++ templated functions the only
     real thing you need to know is that IT represents the input volumes data
     type (e.g. float, short, etc). If you have GUI items then you should get
     the current values of them at TOTO item 10. The we loop over all the
     voxels and perform an operation. In this case a simple copy of the input
     volume to the output volume. At TODO item 11 you should place your own
     algorithm.

  8) Now you should have working source code that you need to compile into a
     DLL (or shared module on UNIX) 
  
The plugin interface support quite complex operations and I recommend looking
at some of the  other plugins source code to see how  they work and get ideas
for what can be done.

*/


#include "vtkVVPluginAPI.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

template <class IT>
/* TODO 1: Rename vvSampleTemplate to vv<your_plugin>Template */
void vvSampleTemplate(vtkVVPluginInfo *info,
                      vtkVVProcessDataStruct *pds, 
                      IT *)
{
  IT *inPtr = (IT *)pds->inData;
  IT *outPtr = (IT *)pds->outData;
  int *dim = info->InputVolumeDimensions;
  int inNumComp = info->InputVolumeNumberOfComponents;
  int i, j, k, l;
  int abort;

  /* TODO 10: Get your GUI values here */
  
  /* loop over the slices */
  for ( k = 0; k < dim[2]; k++ )                                      
    {                       
    /* update the progress status */
    info->UpdateProgress(info,(float)1.0*k/dim[2],"Processing..."); 
    /* see if we should abort */
    abort = atoi(info->GetProperty(info,VVP_ABORT_PROCESSING));
    /* loop over the rows and handle aborts */
    for ( j = 0; !abort && j < dim[1]; j++ )                          
      {          
      /* loop over the columns */
      for ( i = 0; i < dim[0]; i++ )
        {
        /* loop over the components */
        for (l = 0; l < inNumComp; ++l)
          {
          /* TODO 11: put your algorithm code here */
          *outPtr = *inPtr;
          outPtr++;
          inPtr++;
          }
        }
      }                                                      
    }                                                               
  info->UpdateProgress(info,(float)1.0,"Processing Complete");
}

static int ProcessData(void *inf, vtkVVProcessDataStruct *pds)
{
  vtkVVPluginInfo *info = (vtkVVPluginInfo *)inf;

  switch (info->InputVolumeScalarType)
    {
    /* TODO 2: Rename vvSampleTemplate to vv<your_plugin>Template */
    vtkTemplateMacro3(vvSampleTemplate, info, pds, 
                      static_cast<VTK_TT *>(0));
    }
  return 0;
}

/* this function updates the GUI elements to accomidate new data */
/* it will always get called prior to the plugin executing. */
static int UpdateGUI(void *inf)
{
  vtkVVPluginInfo *info = (vtkVVPluginInfo *)inf;

  /* TODO 8: create your required GUI elements here */


  /* TODO 6: modify the following code as required. By default the output
  *  image's properties match those of the input depending on what your
  *  filter does it may need to change some of these values
  */
  info->OutputVolumeScalarType = info->InputVolumeScalarType;
  info->OutputVolumeNumberOfComponents = 
    info->InputVolumeNumberOfComponents;
  int i;
  for (i = 0; i < 3; i++)
    {
    info->OutputVolumeDimensions[i] = info->InputVolumeDimensions[i];
    info->OutputVolumeSpacing[i] = info->InputVolumeSpacing[i];
    info->OutputVolumeOrigin[i] = info->InputVolumeOrigin[i];
    }

  return 1;
}

extern "C" 
{
  /* TODO 3: Rename vvSampleInit to vv<your_plugin>Init */
  void VV_PLUGIN_EXPORT vvSampleInit(vtkVVPluginInfo *info)
  {
    /* always check the version */
    vvPluginVersionCheck();
    
    /* setup information that never changes */
    info->ProcessData = ProcessData;
    info->UpdateGUI = UpdateGUI;
    
    /* set the properties this plugin uses */
    /* TODO 4: Rename "Sample" to "<your_plugin>" */
    info->SetProperty(info, VVP_NAME, "Sample");
    info->SetProperty(info, VVP_GROUP, "Utility");

    /* TODO 5: update the terse and full documentation for your filter */
    info->SetProperty(info, VVP_TERSE_DOCUMENTATION,
                      "Replace voxels above/at/below the threshold value");
    info->SetProperty(info, VVP_FULL_DOCUMENTATION,
                      "This filter performs a pixel replacement, replacing pixel in the original data by a specified replacement value. The pixels to be replaced are determine by comparing the original pixel value to a threshold value with a user specified comparison operation. This filter operates in place, and does not change the dimensions, data type, or spacing of the volume.");

    
    /* TODO 9: set these two values to "0" or "1" based on how your plugin
     * handles data all possible combinations of 0 and 1 are valid. */
    info->SetProperty(info, VVP_SUPPORTS_IN_PLACE_PROCESSING, "0");
    info->SetProperty(info, VVP_SUPPORTS_PROCESSING_PIECES,   "0");

    /* TODO 7: set the number of GUI items used by this plugin */
    info->SetProperty(info, VVP_NUMBER_OF_GUI_ITEMS,          "0");
  info->SetProperty(info, VVP_REQUIRES_SERIES_INPUT,        "0");
  info->SetProperty(info, VVP_SUPPORTS_PROCESSING_SERIES_BY_VOLUMES, "0");
  info->SetProperty(info, VVP_PRODUCES_OUTPUT_SERIES, "0");
  info->SetProperty(info, VVP_PRODUCES_PLOTTING_OUTPUT, "0");
  }
}



