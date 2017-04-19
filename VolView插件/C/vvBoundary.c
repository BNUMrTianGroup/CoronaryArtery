/*=========================================================================

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/VolViewCopyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/* perform a simple threshold filter */

#include "vtkVVPluginAPI.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define SetPixel(info, ptr, value)                               \
  switch (info->OutputVolumeScalarType)                          \
    {                                                            \
    case VTK_CHAR:                                               \
      ((char *)ptr)[0] = (char)value; break;                     \
    case VTK_UNSIGNED_CHAR:                                      \
      ((unsigned char *)ptr)[0] = (unsigned char)value; break;   \
    case VTK_SHORT:                                              \
      ((short *)ptr)[0] = (short)value; break;                   \
    case VTK_UNSIGNED_SHORT:                                     \
      ((unsigned short *)ptr)[0] = (unsigned short)value; break; \
    case VTK_LONG:                                               \
      ((long *)ptr)[0] = (long)value; break;                     \
    case VTK_UNSIGNED_LONG:                                      \
      ((unsigned long *)ptr)[0] = (unsigned long)value; break;   \
    case VTK_INT:                                                \
      ((int *)ptr)[0] = (int)value; break;                       \
    case VTK_UNSIGNED_INT:                                       \
      ((unsigned int *)ptr)[0] = (unsigned int)value; break;     \
    case VTK_FLOAT:                                              \
      ((float *)ptr)[0] = (float)value; break;                   \
    case VTK_DOUBLE:                                             \
      ((double *)ptr)[0] = value; break;                         \
    }

static int ProcessData(void *inf, vtkVVProcessDataStruct *pds)
{
  vtkVVPluginInfo *info = (vtkVVPluginInfo *)inf;
  double replacementValue = atof(info->GetGUIProperty(info, 0, VVP_GUI_VALUE));
  int *dim = info->InputVolumeDimensions;
  unsigned int rsize = info->InputVolumeScalarSize;
  int i, j, k;

  unsigned char *ptr = (unsigned char *)pds->outData;  
  for ( k = 0; k < dim[2]; k++ )
    {
    for ( j = 0; j < dim[1]; j++ )
      {
      if ( j == 0 || k == 0 || j == (dim[1]-1) || k == (dim[2]-1) )
        {
        for ( i = 0; i < dim[0]*info->InputVolumeNumberOfComponents; i++ )
          {
          SetPixel(info,ptr,replacementValue);
          ptr += rsize;
          }
        }
      else
        {
        for ( i = 0; i < info->InputVolumeNumberOfComponents; i++ )
          {
          SetPixel(info,ptr,replacementValue);
          SetPixel(info,
                   ptr+info->InputVolumeNumberOfComponents*dim[0]-1,
                   replacementValue);
          ptr += rsize;
          }
        ptr = ptr+ info->InputVolumeNumberOfComponents*rsize*
          (dim[0] - 1);
        }
      }
    }
  
  return 0;
}

static int UpdateGUI(void *inf)
{
  int i;
  vtkVVPluginInfo *info = (vtkVVPluginInfo *)inf;

  info->SetGUIProperty(info, 0, VVP_GUI_LABEL, "New Boundary Value");
  info->SetGUIProperty(info, 0, VVP_GUI_TYPE, VVP_GUI_SCALE);
  info->SetGUIProperty(info, 0, VVP_GUI_DEFAULT , "0");
  info->SetGUIProperty(info, 0, VVP_GUI_HELP,
               "What value to set the boundary voxels to");
    
  // what range should we show for possible output values
  vvPluginSetGUIScaleRange(0);

  info->OutputVolumeScalarType = info->InputVolumeScalarType;
  info->OutputVolumeNumberOfComponents = info->InputVolumeNumberOfComponents;
  for (i = 0; i < 3; i++)
    {
    info->OutputVolumeDimensions[i] = info->InputVolumeDimensions[i];
    info->OutputVolumeSpacing[i] = info->InputVolumeSpacing[i];
    info->OutputVolumeOrigin[i] = info->InputVolumeOrigin[i];
    }

  return 1;
}

void VV_PLUGIN_EXPORT vvBoundaryInit(vtkVVPluginInfo *info)
{
  /* always check the version */
  vvPluginVersionCheck();
  
  /* setup information that never changes */
  info->ProcessData = ProcessData;
  info->UpdateGUI = UpdateGUI;
  info->SetProperty(info, VVP_NAME, "Boundary");
  info->SetProperty(info, VVP_GROUP, "Utility");
  info->SetProperty(info, VVP_TERSE_DOCUMENTATION,
     "Replace all boundary (outside edge) voxels with the specified value");
  info->SetProperty(info, VVP_FULL_DOCUMENTATION,
    "This filter performs a voxel replacement, replacing every voxel that is a boundary voxel with the specified value. Boundary voxels are the voxels that have at least one face on the boundary of the volume. Put another way, boundary voxels are voxels that are not fully enclosed by other voxels.");
  
  info->SetProperty(info, VVP_SUPPORTS_IN_PLACE_PROCESSING, "1");
  info->SetProperty(info, VVP_SUPPORTS_PROCESSING_PIECES,   "1");
  info->SetProperty(info, VVP_REQUIRED_Z_OVERLAP,           "0");
  info->SetProperty(info, VVP_NUMBER_OF_GUI_ITEMS,          "1");
  info->SetProperty(info, VVP_REQUIRES_SERIES_INPUT,        "0");
  info->SetProperty(info, VVP_SUPPORTS_PROCESSING_SERIES_BY_VOLUMES, "0");
  info->SetProperty(info, VVP_PRODUCES_OUTPUT_SERIES, "0");
  info->SetProperty(info, VVP_PRODUCES_PLOTTING_OUTPUT, "0");
}




