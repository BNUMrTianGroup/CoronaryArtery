/*=========================================================================

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/VolViewCopyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/


#include "vtkVVPluginAPI.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define ApplyPixelMathFilterMacro( COMPARISON )                         \
    int i, j, k;                                                        \
    for ( k = 0; k < dim[2]; k++ )                                      \
      {                                                                 \
      info->UpdateProgress(info,(float)1.0*k/dim[2],"PixelMathing..."); \
      abort = atoi(info->GetProperty(info,VVP_ABORT_PROCESSING));       \
      for ( j = 0; !abort && j < dim[1]; j++ )                          \
        {                                                               \
        for ( i = 0; i < nc*dim[0]; i++ )                               \
          {                                                             \
          *ptr = ( *ptr COMPARISON compVal );                           \
          ptr++;                                                        \
          }                                                             \
        }                                                               \
      }                                                                 \
    info->UpdateProgress(info,(float)1.0,"PixelMathing Complete");


//-----------------------------------------------------------
template <class IT>
void vvPixelMathTemplate(vtkVVPluginInfo *info,
                         vtkVVProcessDataStruct *pds, 
                         IT *)
{
  IT *ptr = (IT *)pds->outData;
  int *dim = info->InputVolumeDimensions;
  double v1 = atof(info->GetGUIProperty(info, 1, VVP_GUI_VALUE));
  const char *label = info->GetGUIProperty(info, 0, VVP_GUI_VALUE);
  int abort = 0;
  IT compVal = (IT)v1;
  int nc = info->InputVolumeNumberOfComponents;
  
  if (!strcmp(label,"+"))
    {
    ApplyPixelMathFilterMacro( + );
    }
  if (!strcmp(label,"-"))
    {
    ApplyPixelMathFilterMacro( - );
    }
  if (!strcmp(label,"*"))
    {
    ApplyPixelMathFilterMacro( * );
    }
  if (!strcmp(label,"/"))
    {
    ApplyPixelMathFilterMacro( / );
    }
}

static int ProcessData(void *inf, vtkVVProcessDataStruct *pds)
{
  vtkVVPluginInfo *info = (vtkVVPluginInfo *)inf;

  switch (info->InputVolumeScalarType)
    {
    // invoke the appropriate templated function
    vtkTemplateMacro3(vvPixelMathTemplate, info, pds, 
                      static_cast<VTK_TT *>(0));
    }
  return 0;
}


static int UpdateGUI(void *inf)
{
  char tmp[1024];
  double stepSize = 1.0;
  vtkVVPluginInfo *info = (vtkVVPluginInfo *)inf;

  info->SetGUIProperty(info, 0, VVP_GUI_LABEL, "Operator");
  info->SetGUIProperty(info, 0, VVP_GUI_TYPE, VVP_GUI_CHOICE);
  info->SetGUIProperty(info, 0, VVP_GUI_DEFAULT , "+");
  info->SetGUIProperty(info, 0, VVP_GUI_HELP,
                       "The operator for a pixel");
  info->SetGUIProperty(info, 0, VVP_GUI_HINTS, "4\n+\n-\n*\n/");

  info->SetGUIProperty(info, 1, VVP_GUI_LABEL, "Constant operand");
  info->SetGUIProperty(info, 1, VVP_GUI_TYPE, VVP_GUI_SCALE);
  info->SetGUIProperty(info, 1, VVP_GUI_DEFAULT , "0");
  info->SetGUIProperty(info, 1, VVP_GUI_HELP,
                       "The value after the operator against");
    
  /* set the range of the sliders */
  if (info->InputVolumeScalarType == VTK_FLOAT || 
      info->InputVolumeScalarType == VTK_DOUBLE) 
    { 
    /* for float and double use a step size of 1/200 th the range */
    stepSize = info->InputVolumeScalarRange[1]*0.005 - 
      info->InputVolumeScalarRange[0]*0.005; 
    }
  sprintf(tmp,"%g %g %g", 
          (info->InputVolumeScalarRange[0]-2000),
          (info->InputVolumeScalarRange[1]+2000),
          stepSize);
  info->SetGUIProperty(info, 1, VVP_GUI_HINTS , tmp);

  /* auto range for scalar type */
  vvPluginSetGUIScaleRange(2);

  info->OutputVolumeScalarType = info->InputVolumeScalarType;
  info->OutputVolumeNumberOfComponents = info->InputVolumeNumberOfComponents;
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
  void VV_PLUGIN_EXPORT vvPixelMathInit(vtkVVPluginInfo *info)
{
  /* always check the version */
  vvPluginVersionCheck();

  /* setup information that never changes */
  info->ProcessData = ProcessData;
  info->UpdateGUI = UpdateGUI;

  /* set the properties this plugin uses */
  info->SetProperty(info, VVP_NAME, "PixelMath");
  info->SetProperty(info, VVP_GROUP, "Utility");
  info->SetProperty(info, VVP_TERSE_DOCUMENTATION,
                    "Pixel wise operation on an image. This can be used to add/subtract or multiply/divide a constant value to all pixels in the image.");
  info->SetProperty(info, VVP_FULL_DOCUMENTATION,
                    "This filter performs a pixel replacement, replacing pixel in the original data by the result of the specified operation. This filter operates in place, and does not change the dimensions, data type, or spacing of the volume.");

  info->SetProperty(info, VVP_SUPPORTS_IN_PLACE_PROCESSING, "1");
  info->SetProperty(info, VVP_SUPPORTS_PROCESSING_PIECES,   "1");
  info->SetProperty(info, VVP_NUMBER_OF_GUI_ITEMS,          "2");
  info->SetProperty(info, VVP_REQUIRES_SERIES_INPUT,        "0");
  info->SetProperty(info, VVP_SUPPORTS_PROCESSING_SERIES_BY_VOLUMES, "0");
  info->SetProperty(info, VVP_PRODUCES_OUTPUT_SERIES, "0");
  info->SetProperty(info, VVP_PRODUCES_PLOTTING_OUTPUT, "0");
}
  
}

