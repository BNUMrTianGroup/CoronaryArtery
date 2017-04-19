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


//-----------------------------------------------------------
template <class IT, class I2T>
void vvMergeVolumesTemplate2(
                             vtkVVPluginInfo *info,
                             vtkVVProcessDataStruct *pds, 
                             IT *, I2T *, bool)
{
  // Templated macro to calculate the max, ,min value in each
  // component and rescale them for purposes or merging
  // We will rescale every component to the same extent as the first one
  // 

  IT *inPtr = (IT *)pds->inData;
  I2T *inPtr2 = (I2T *)pds->inData2;
  IT *outPtr = (IT *)pds->outData;

  int *dim1 = info->InputVolumeDimensions;

  int abort = 0;
  int inNumCom = info->InputVolumeNumberOfComponents;
  int inNumCom2 = info->InputVolume2NumberOfComponents;
  
  static IT maxval[4];
  static IT minval[4];
  static IT diffval[4];
  static I2T maxval2[4];
  static I2T minval2[4];
  static I2T diffval2[4];
  
  int i, j, k, l;
  
  if (inNumCom + inNumCom2 > 4)
    {
    inNumCom = 4 - inNumCom2;
    }
  
  int inNumComE = info->InputVolumeNumberOfComponents - inNumCom;
  
  
  for( i=0; i < inNumCom; i++ )
    {
    if (i < inNumCom)
      {
      maxval[i] = *inPtr;
      minval[i] = *inPtr;
      }
    }

  for( i=0; i < inNumCom2; i++ )
    {
    if (i < inNumCom2)
      {
      maxval2[i] = *inPtr2;
      minval2[i] = *inPtr2;
      }
    }
      

  
  for ( k = 0; k < dim1[2]; k++ )                                      
    {                                                                 
    info->UpdateProgress(info,(float)0.5*k/dim1[2],"Processing..."); 
    abort = atoi(info->GetProperty(info,VVP_ABORT_PROCESSING));
    for ( j = 0; !abort && j < dim1[1]; j++ )                          
      {          
      // copy the result into the output
      for ( i = 0; i < dim1[0]; i++ )
        {
        for (l = 0; l < inNumCom; ++l)
          {
          if (*inPtr > maxval[l])
            {
            maxval[l] = *inPtr;
            }
          if (*inPtr < minval[l])
            {
            minval[l] = *inPtr;
            }
          inPtr++;
          }
        
        inPtr += inNumComE;
        for (l = 0; l < inNumCom2; ++l)
          {
          if (*inPtr2 > maxval2[l])
            {
            maxval2[l] = *inPtr2;
            }
          if (*inPtr2 < minval2[l])
            {
            minval2[l] = *inPtr2;
            }
          inPtr2++;
          }
        }
      }                                                      
    }          

  for( i=0; i < 4; i++ )
    {
    diffval[i] = maxval[i] - minval[i];
    diffval2[i] = maxval2[i] - minval2[i];
    }
  
  
  //TODO if diffval[0] = 0 throw an exception.
  
  inPtr = (IT *)pds->inData;
  inPtr2 = (I2T *)pds->inData2;
  outPtr = (IT *)pds->outData;

  for ( k = 0; k < dim1[2]; k++ )                                      
    {                                                                 
    info->UpdateProgress(info,(float)(0.5 + 0.5*k/dim1[2]),"Processing..."); 
    abort = atoi(info->GetProperty(info,VVP_ABORT_PROCESSING));
    for ( j = 0; !abort && j < dim1[1]; j++ )                          
      {          
      // copy the result into the output
      for ( i = 0; i < dim1[0]; i++ )
        {
        for (l = 0; l < inNumCom; l++)
          {
            *outPtr = static_cast< IT >(( (static_cast<double>(*inPtr - minval[l])) * 
              static_cast<double>(diffval[0]) / static_cast<double>(diffval[l])) + 
              static_cast<double>(minval[0]));
          ++outPtr;
          ++inPtr;
          }
        
        inPtr += inNumComE;
        for (l = 0; l < inNumCom2; l++)
          {
            *outPtr = static_cast< IT >(( (static_cast<double>(*inPtr2 - minval2[l])) * 
              static_cast<double>(diffval[0]) / static_cast<double>(diffval2[l])) + 
              static_cast<double>(minval[0]));
          ++outPtr;
          ++inPtr2;
          }
        }
      }                                                      
    }                                                               

  info->UpdateProgress(info,(float)1.0,"Processing Complete");
}

  
    




//-----------------------------------------------------------
template <class IT, class I2T>
void vvMergeVolumesTemplate2(vtkVVPluginInfo *info,
                             vtkVVProcessDataStruct *pds, 
                             IT *, I2T *)
{
  IT *inPtr = (IT *)pds->inData;
  I2T *inPtr2 = (I2T *)pds->inData2;
  IT *outPtr = (IT *)pds->outData;

  int *dim1 = info->InputVolumeDimensions;

  int abort = 0;
  int inNumCom = info->InputVolumeNumberOfComponents;
  int inNumCom2 = info->InputVolume2NumberOfComponents;
  
  if (inNumCom + inNumCom2 > 4)
    {
    inNumCom = 4 - inNumCom2;
    }
  
  int inNumComE = info->InputVolumeNumberOfComponents - inNumCom;
  
  int i, j, k, l;
  for ( k = 0; k < dim1[2]; k++ )                                      
    {                                                                 
    info->UpdateProgress(info,(float)1.0*k/dim1[2],"Processing..."); 
    abort = atoi(info->GetProperty(info,VVP_ABORT_PROCESSING));
    for ( j = 0; !abort && j < dim1[1]; j++ )                          
      {          
      // copy the result into the output
      for ( i = 0; i < dim1[0]; i++ )
        {
        for (l = 0; l < inNumCom; ++l)
          {
          *outPtr = *inPtr;
          outPtr++;
          inPtr++;
          }
        inPtr += inNumComE;
        for (l = 0; l < inNumCom2; ++l)
          {
          *outPtr = static_cast<IT>(*inPtr2);
          outPtr++;
          inPtr2++;
          }
        }
      }                                                      
    }                                                               
  info->UpdateProgress(info,(float)1.0,"Processing Complete");
}

//-----------------------------------------------------------
template <class IT>
void vvMergeVolumesTemplate(vtkVVPluginInfo *info,
                            vtkVVProcessDataStruct *pds, 
                            IT *)
{
  const int rescaleDynamicRangeForMerging = atoi( info->GetGUIProperty( 
    info, 0, VVP_GUI_VALUE));
  
  if (rescaleDynamicRangeForMerging)
    {
    switch (info->InputVolume2ScalarType)
      {
      // invoke the appropriate templated function
      vtkTemplateMacro5(vvMergeVolumesTemplate2, info, pds, 
                      static_cast<IT *>(0), static_cast<VTK_TT *>(0), true);
      }
    }
  else
    {
    switch (info->InputVolume2ScalarType)
      {
      // invoke the appropriate templated function
      vtkTemplateMacro4(vvMergeVolumesTemplate2, info, pds, 
                      static_cast<IT *>(0), static_cast<VTK_TT *>(0));
      }
    }

  return;
}

static int ProcessData(void *inf, vtkVVProcessDataStruct *pds)
{
  vtkVVPluginInfo *info = (vtkVVPluginInfo *)inf;

  int *dim1 = info->InputVolumeDimensions;
  int *dim2 = info->InputVolume2Dimensions;

  if (dim1[0] != dim2[0] || dim1[1] != dim2[1] || dim1[2] != dim2[2])
    {
    char buffer[256];
    sprintf(buffer, "The dimensions of the two volumes to be merged are not the same. This first volume has dimensions (%d, %d, %d) while the second volume has dimensions (%d, %d, %d)", 
            dim1[0], dim1[1], dim1[2], dim2[0], dim2[1], dim2[2]);
    info->SetProperty(info, VVP_REPORT_TEXT, buffer);
    return 1;
    }

  switch (info->InputVolumeScalarType)
    {
    // invoke the appropriate templated function
    vtkTemplateMacro3(vvMergeVolumesTemplate, info, pds, 
                      static_cast<VTK_TT *>(0));
    }
  return 0;
}


static int UpdateGUI(void *inf)
{
  vtkVVPluginInfo *info = (vtkVVPluginInfo *)inf;

  
  info->SetGUIProperty(info, 0, VVP_GUI_LABEL, "Rescale components");
  info->SetGUIProperty(info, 0, VVP_GUI_TYPE, VVP_GUI_CHECKBOX);
  info->SetGUIProperty(info, 0, VVP_GUI_DEFAULT, "1");
  info->SetGUIProperty(info, 0, VVP_GUI_HELP, "Enabling this option will rescale all components to the dynamic range of the first component. This provides the same window level settings for a blended output.");

  
  info->OutputVolumeScalarType = info->InputVolumeScalarType;
  int i;
  for (i = 0; i < 3; i++)
    {
    info->OutputVolumeDimensions[i] = info->InputVolumeDimensions[i];
    info->OutputVolumeSpacing[i] = info->InputVolumeSpacing[i];
    info->OutputVolumeOrigin[i] = info->InputVolumeOrigin[i];
    }

  info->OutputVolumeNumberOfComponents = 
    info->InputVolumeNumberOfComponents + 
    info->InputVolume2NumberOfComponents;

  if (info->OutputVolumeNumberOfComponents > 4)
    {
    info->OutputVolumeNumberOfComponents = 4;
    }
  
  return 1;
}

extern "C" 
{
  void VV_PLUGIN_EXPORT vvMergeVolumesInit(vtkVVPluginInfo *info)
{
  /* always check the version */
  vvPluginVersionCheck();

  /* setup information that never changes */
  info->ProcessData = ProcessData;
  info->UpdateGUI = UpdateGUI;

  /* set the properties this plugin uses */
  info->SetProperty(info, VVP_NAME, "Merge Volumes");
  info->SetProperty(info, VVP_GROUP, "Utility");
  info->SetProperty(info, VVP_TERSE_DOCUMENTATION,
                    "Merge the componets of two volumes");
  info->SetProperty(info, VVP_FULL_DOCUMENTATION,
                    "This filter computes a new volume by merging the components of two volumes together. So if both of your volumes had one component per voxel then after merging the result would be a two component per voxel volume. The two volumes must be of the same dimensions. The resulting volume will be the same scalar type as the original volume. If the combination of the two volume would result in more than four components then the second volume will replace some of the components of the original volume. For example if a four component volume merged with a one component volume the last component of the four component volume would be replaced by the firts component of the second volume.");

  info->SetProperty(info, VVP_SUPPORTS_IN_PLACE_PROCESSING, "0");
  info->SetProperty(info, VVP_SUPPORTS_PROCESSING_PIECES,   "1");
  info->SetProperty(info, VVP_NUMBER_OF_GUI_ITEMS,          "1");
  info->SetProperty(info, VVP_REQUIRES_SECOND_INPUT,        "1");
  info->SetProperty(info, VVP_REQUIRES_SERIES_INPUT,        "0");
  info->SetProperty(info, VVP_SUPPORTS_PROCESSING_SERIES_BY_VOLUMES, "0");
  info->SetProperty(info, VVP_PRODUCES_OUTPUT_SERIES, "0");
  info->SetProperty(info, VVP_PRODUCES_PLOTTING_OUTPUT, "0");
}
  
}





