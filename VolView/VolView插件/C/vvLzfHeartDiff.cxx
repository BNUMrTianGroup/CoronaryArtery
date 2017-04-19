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
#include <map>
#include <set>
#include <algorithm>
using namespace std;
struct Coordinate{
	int x, y, z;
	Coordinate(int zz, int yy, int xx){
		x = xx;
		y = yy;
		z = zz;
	}
	bool operator < (const Coordinate &co) const{
		return ((z < co.z) || (z == co.z && y < co.y) || (z == co.z && y == co.y && x <co.x));
	}
};

template <class IT>
void vvLzfHeartDiffTemplate(vtkVVPluginInfo *info,
                      vtkVVProcessDataStruct *pds, 
                      IT *)
{
	IT *inPtr = (IT *)pds->inData;
	IT *outPtr = (IT *)pds->outData;
	int *dim = info->InputVolumeDimensions;
	int inNumComp = info->InputVolumeNumberOfComponents;
	int abort;
	int Xd = (int)dim[0];
	int Yd = (int)dim[1]; 
	int Zd = (int)dim[2];

	IT *inPtr2 = (IT *)pds->inData2;
	int *dim2 = info->InputVolume2Dimensions;

	FILE *console;
	console = fopen("console.txt", "w");
  
	fprintf(console, "test second image!\npds->inData2 = %x, info->InputVolume2Dimensions = {%d, %d, %d}\n", inPtr2, dim2[0], dim2[1], dim2[2]);
	
	for (int i = 0; i < Zd; i++){
		info->UpdateProgress(info,(float)1.0*i/Zd,"Processing..."); 
		abort = atoi(info->GetProperty(info,VVP_ABORT_PROCESSING));
		for (int j = 0; !abort && j < Yd; j++){
			for (int k = 0; k < Xd; k++) {
				if(*inPtr2 == *inPtr && *inPtr > 0)
					*outPtr = 0;
				else
					*outPtr = max(*inPtr2, *inPtr);

				inPtr++;
				inPtr2++;
				outPtr++;
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
    vtkTemplateMacro3(vvLzfHeartDiffTemplate, info, pds, 
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
  void VV_PLUGIN_EXPORT vvLzfHeartDiffInit(vtkVVPluginInfo *info)
  {
    /* always check the version */
    vvPluginVersionCheck();
    
    /* setup information that never changes */
    info->ProcessData = ProcessData;
    info->UpdateGUI = UpdateGUI;
    
    /* set the properties this plugin uses */
    /* TODO 4: Rename "Sample" to "<your_plugin>" */
    info->SetProperty(info, VVP_NAME, "LzfHeartDiff");
    info->SetProperty(info, VVP_GROUP, "Utility");

    /* TODO 5: update the terse and full documentation for your filter */
    info->SetProperty(info, VVP_TERSE_DOCUMENTATION,
                      "The difference between two heart image...");
    info->SetProperty(info, VVP_FULL_DOCUMENTATION,
                      "This plugin is created on Jul 6th, 2015. It is used for getting the difference between two heart images of identical patient scanned at different phases. It is also a pratice of how to read a second image in.");

    
    /* TODO 9: set these two values to "0" or "1" based on how your plugin
     * handles data all possible combinations of 0 and 1 are valid. */
    info->SetProperty(info, VVP_SUPPORTS_IN_PLACE_PROCESSING, "1");
    info->SetProperty(info, VVP_SUPPORTS_PROCESSING_PIECES,   "1");

    /* TODO 7: set the number of GUI items used by this plugin */
    info->SetProperty(info, VVP_NUMBER_OF_GUI_ITEMS,          "0");
	info->SetProperty(info, VVP_REQUIRES_SECOND_INPUT,        "1");
	info->SetProperty(info, VVP_REQUIRES_SERIES_INPUT,        "0");
	info->SetProperty(info, VVP_SUPPORTS_PROCESSING_SERIES_BY_VOLUMES, "0");
	info->SetProperty(info, VVP_PRODUCES_OUTPUT_SERIES, "0");
	info->SetProperty(info, VVP_PRODUCES_PLOTTING_OUTPUT, "0");
  }
}