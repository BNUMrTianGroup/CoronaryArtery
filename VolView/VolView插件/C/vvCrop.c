/*=========================================================================

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/VolViewCopyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/* perform a simple crop filter */

#include "vtkVVPluginAPI.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static int ProcessData(void *inf, vtkVVProcessDataStruct *pds)
{
  vtkVVPluginInfo *info = (vtkVVPluginInfo *)inf;
  int *dim = info->InputVolumeDimensions;
  int *outDim = info->OutputVolumeDimensions;
  char results[1024];
    
  unsigned char *inPtr = (unsigned char *)pds->inData;
  unsigned char *inSlicePtr;
  unsigned char *ptr = (unsigned char *)pds->outData;
  int j, k;
  int outRowSize = outDim[0] * 
    info->OutputVolumeNumberOfComponents * info->InputVolumeScalarSize;
  int inRowInc = dim[0] * 
    info->InputVolumeNumberOfComponents * info->InputVolumeScalarSize;
  int inSliceInc = dim[0] * dim[1] *
    info->InputVolumeNumberOfComponents * info->InputVolumeScalarSize;
  
  /* convert cropping planes into indexes */
  int idx[6];
  for (j = 0; j < 6; ++j)
    {
    idx[j] = (int)((info->CroppingPlanes[j]-info->InputVolumeOrigin[j/2])/
      info->InputVolumeSpacing[j/2] + 0.5);
    if (idx[j] < 0) 
      {
      idx[j] = 0;
      }
    if (idx[j] >= info->InputVolumeDimensions[j/2])
      {
      idx[j] = info->InputVolumeDimensions[j/2] -1;
      }
    }

  /* compute the starting ptr loc */
  inPtr = inPtr + 
    info->InputVolumeNumberOfComponents * info->InputVolumeScalarSize * 
    (idx[0] + dim[0]*(idx[2] + dim[1]*idx[4]));
  inSlicePtr = inPtr;
  
  /* loop over the output volume, and copy from the input volume */
  for ( k = 0; k < outDim[2]; k++ )
    {
    inPtr = inSlicePtr;
    for ( j = 0; j < outDim[1]; j++ )
      {
      memcpy(ptr,inPtr,outRowSize);
      ptr += outRowSize;
      inPtr += inRowInc;
      }
    inSlicePtr += inSliceInc;
    }
  
  // report the results
  sprintf(results,"Cropped input volume with dimensions %i %i %i and origin of %g %g %g to output volume with dimensions %i %i %i and origin of %g %g %g", 
          dim[0], dim[1], dim[2], 
          info->InputVolumeOrigin[0],
          info->InputVolumeOrigin[1],
          info->InputVolumeOrigin[2],
          outDim[0], outDim[1], outDim[2],
          info->OutputVolumeOrigin[0],
          info->OutputVolumeOrigin[1],
          info->OutputVolumeOrigin[2]);
  info->SetProperty(info, VVP_REPORT_TEXT,results);

  return 0;
}

static int UpdateGUI(void *inf)
{
  vtkVVPluginInfo *info = (vtkVVPluginInfo *)inf;
  int i, j, idx[2];

  info->OutputVolumeScalarType = info->InputVolumeScalarType;
  info->OutputVolumeNumberOfComponents = info->InputVolumeNumberOfComponents;

  for (i = 0; i < 3; ++i)
    {
    for (j = 0; j < 2; ++j)
      {
      if (info->CroppingPlanes && info->InputVolumeSpacing[i] > 0)
        {
        idx[j] = (int)
          ((info->CroppingPlanes[i*2+j]- info->InputVolumeOrigin[i]) /
           info->InputVolumeSpacing[i] + 0.5);
        }
      else
        {
        idx[j] = 0;
        }
      if (idx[j] < 0) 
        {
        idx[j] = 0;
        }
      if (idx[j] >= info->InputVolumeDimensions[i])
        {
        idx[j] = info->InputVolumeDimensions[i] - 1;
        }
      }

    info->OutputVolumeDimensions[i] = idx[1] - idx[0] + 1;
    info->OutputVolumeOrigin[i] = 
      info->InputVolumeOrigin[i] + idx[0] * info->InputVolumeSpacing[i];
    info->OutputVolumeSpacing[i] = info->InputVolumeSpacing[i];
    }

  return 1;
}

void VV_PLUGIN_EXPORT vvCropInit(vtkVVPluginInfo *info)
{
  /* always check the version */
  vvPluginVersionCheck();

  /* setup information that never changes */
  info->ProcessData = ProcessData;
  info->UpdateGUI = UpdateGUI;
  info->SetProperty(info, VVP_NAME, "Crop");
  info->SetProperty(info, VVP_GROUP, "Utility");
  info->SetProperty(info, VVP_TERSE_DOCUMENTATION,
     "Crop the current volume to fit within the current cropping planes");
  info->SetProperty(info, VVP_FULL_DOCUMENTATION,
    "This filter crops a volume to fit within the current settings of the cropping planes. (regardless of if they are currently visible). This is useful for reducing a large volume to a smaller subvolume prior to applying a time consuming localized segmentation algorithm");
  
  info->SetProperty(info, VVP_SUPPORTS_IN_PLACE_PROCESSING, "0");
  info->SetProperty(info, VVP_SUPPORTS_PROCESSING_PIECES,   "0");
  info->SetProperty(info, VVP_REQUIRED_Z_OVERLAP,           "0");
  info->SetProperty(info, VVP_NUMBER_OF_GUI_ITEMS,          "0");
  info->SetProperty(info, VVP_REQUIRES_SERIES_INPUT,        "0");
  info->SetProperty(info, VVP_SUPPORTS_PROCESSING_SERIES_BY_VOLUMES, "0");
  info->SetProperty(info, VVP_PRODUCES_OUTPUT_SERIES, "0");
  info->SetProperty(info, VVP_PRODUCES_PLOTTING_OUTPUT, "0");
}




