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
#include <math.h>


#define ApplyImageMathFilterMacro( COMPARISON )                         \
    int i, j, k;                                                        \
    for ( k = 0; k < dim[2]; k++ )                                      \
      {                                                                 \
      info->UpdateProgress(info,(float)1.0*k/dim[2],"ImageMathing..."); \
      abort = atoi(info->GetProperty(info,VVP_ABORT_PROCESSING));       \
      for ( j = 0; !abort && j < dim[1]; j++ )                          \
        {                                                               \
        for ( i = 0; i < nc*dim[0]; i++ )                               \
          {                                                             \
          *ptr = ( COMPARISON );                                        \
          ++ptr;                                                        \
          ++ptr2;                                                       \
          }                                                             \
        }                                                               \
      }                                                                 \
    info->UpdateProgress(info,(float)1.0,"ImageMathing Complete");


//-----------------------------------------------------------
template <class IT, class I2T>
void vvImageMathematicsTemplate2(vtkVVPluginInfo *info,
                             vtkVVProcessDataStruct *pds, 
                             IT *, I2T *)
{
  IT *ptr = (IT *)pds->outData;
  I2T *ptr2 = (I2T *)pds->inData2;

  int *dim = info->InputVolumeDimensions;

  int abort = 0;
  int inNumCom = info->InputVolumeNumberOfComponents;
  int inNumCom2 = info->InputVolume2NumberOfComponents;
  int nc = info->InputVolumeNumberOfComponents;
  const char *label = info->GetGUIProperty(info, 0, VVP_GUI_VALUE);
  
  if (!strcmp(label,"+"))
    {
    ApplyImageMathFilterMacro( (*ptr)+(*ptr2) );
    }
  if (!strcmp(label,"-"))
    {
    ApplyImageMathFilterMacro( (*ptr)-(*ptr2) );
    }
  if (!strcmp(label,"*"))
    {
    ApplyImageMathFilterMacro( (*ptr)*(*ptr2) );
    }
  if (!strcmp(label,"/"))
    {
    ApplyImageMathFilterMacro( (*ptr)/(*ptr2) );
    }
  if (!strcmp(label,"|-|"))
    {
    ApplyImageMathFilterMacro( fabs((double)((*ptr)-(*ptr2))) );
    }
}

//-----------------------------------------------------------
template <class IT>
void vvImageMathematicsTemplate(vtkVVPluginInfo *info,
                            vtkVVProcessDataStruct *pds, 
                            IT *)
{
  const int rescaleDynamicRangeForMerging = atoi( info->GetGUIProperty( 
    info, 0, VVP_GUI_VALUE));
  
  switch (info->InputVolume2ScalarType)
    {
    // invoke the appropriate templated function
    vtkTemplateMacro4(vvImageMathematicsTemplate2, info, pds, 
                      static_cast<IT *>(0), static_cast<VTK_TT *>(0));
    }

  return;
}

static int ProcessData(void *inf, vtkVVProcessDataStruct *pds)
{
  vtkVVPluginInfo *info = (vtkVVPluginInfo *)inf;

  int *dim1 = info->InputVolumeDimensions;
  int *dim2 = info->InputVolume2Dimensions;
  int inNumCom = info->InputVolumeNumberOfComponents;
  int inNumCom2 = info->InputVolume2NumberOfComponents;  
  
  if (dim1[0] != dim2[0] || dim1[1] != dim2[1] || dim1[2] != dim2[2])
    {
    char buffer[256];
    sprintf(buffer, "The dimensions of the two volumes to be merged are not the same. This first volume has dimensions (%d, %d, %d) while the second volume has dimensions (%d, %d, %d)", 
            dim1[0], dim1[1], dim1[2], dim2[0], dim2[1], dim2[2]);
    info->SetProperty(info, VVP_REPORT_TEXT, buffer);
    return 1;
    }

  if (inNumCom2 != inNumCom)
    {
    char buffer[256];
    sprintf(buffer, "The two volumes must have the same number of components. This first volume has %d components. The second has %d components", 
            inNumCom, inNumCom2);
    info->SetProperty(info, VVP_REPORT_TEXT, buffer);
    return 1;
    }
  
  switch (info->InputVolumeScalarType)
    {
    // invoke the appropriate templated function
    vtkTemplateMacro3(vvImageMathematicsTemplate, info, pds, 
                      static_cast<VTK_TT *>(0));
    }
  return 0;
}


static int UpdateGUI(void *inf)
{
  vtkVVPluginInfo *info = (vtkVVPluginInfo *)inf;

  
  info->SetGUIProperty(info, 0, VVP_GUI_LABEL, "Operator");
  info->SetGUIProperty(info, 0, VVP_GUI_TYPE, VVP_GUI_CHOICE);
  info->SetGUIProperty(info, 0, VVP_GUI_DEFAULT , "-");
  info->SetGUIProperty(info, 0, VVP_GUI_HELP,
                       "The operator for a pixel");
  info->SetGUIProperty(info, 0, VVP_GUI_HINTS, "5\n+\n-\n*\n|-|\n/");
  
  info->OutputVolumeScalarType = info->InputVolumeScalarType;
  int i;
  for (i = 0; i < 3; i++)
    {
    info->OutputVolumeDimensions[i] = info->InputVolumeDimensions[i];
    info->OutputVolumeSpacing[i] = info->InputVolumeSpacing[i];
    info->OutputVolumeOrigin[i] = info->InputVolumeOrigin[i];
    }

  info->OutputVolumeNumberOfComponents = 
    info->InputVolumeNumberOfComponents;
  
  return 1;
}

extern "C" 
{
  void VV_PLUGIN_EXPORT vvImageMathematicsInit(vtkVVPluginInfo *info)
{
  /* always check the version */
  vvPluginVersionCheck();

  /* setup information that never changes */
  info->ProcessData = ProcessData;
  info->UpdateGUI = UpdateGUI;

  /* set the properties this plugin uses */
  info->SetProperty(info, VVP_NAME, "Dual-image math");
  info->SetProperty(info, VVP_GROUP, "Utility");
  info->SetProperty(info, VVP_TERSE_DOCUMENTATION,
                    "Pixel wise Mathematics on two inputs");
  info->SetProperty(info, VVP_FULL_DOCUMENTATION,
                    "This filter performs a pixel wise operations based on two images. The images need to have the same dimensions, scalar type and metadata.");

  info->SetProperty(info, VVP_SUPPORTS_IN_PLACE_PROCESSING, "1");
  info->SetProperty(info, VVP_SUPPORTS_PROCESSING_PIECES,   "1");
  info->SetProperty(info, VVP_NUMBER_OF_GUI_ITEMS,          "1");
  info->SetProperty(info, VVP_REQUIRES_SECOND_INPUT,        "1");
  info->SetProperty(info, VVP_REQUIRES_SERIES_INPUT,        "0");
  info->SetProperty(info, VVP_SUPPORTS_PROCESSING_SERIES_BY_VOLUMES, "0");
  info->SetProperty(info, VVP_PRODUCES_OUTPUT_SERIES, "0");
  info->SetProperty(info, VVP_PRODUCES_PLOTTING_OUTPUT, "0");
}
  
}

