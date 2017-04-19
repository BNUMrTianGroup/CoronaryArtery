#include "vtkVVPluginAPI.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
using namespace std;

template <class IT>
/* TODO 1: Rename vvSampleTemplate to vv<your_plugin>Template */
void vvReadTextTemplate(vtkVVPluginInfo *info,
                      vtkVVProcessDataStruct *pds, 
                      IT *)
{
  IT *inPtr = (IT *)pds->inData;
  IT *outPtr = (IT *)pds->outData;
  int *dim = info->InputVolumeDimensions;
  int inNumComp = info->InputVolumeNumberOfComponents;
  int abort;

  /* TODO 10: Get your GUI values here */
  
  int x, y, z, scalar;
  int *** vol = new int**[dim[2]];  
  for(int i = 0; i < dim[2]; i++){
    vol[i] = new int*[dim[1]];
    for(int j = 0; j < dim[1]; j++){
      vol[i][j] =  new int[dim[0]]();
    }
  }
  ifstream infile;
  infile.open("Z:\\papers\\FrangiVesslement\\total_delt=2-6(2).txt", ifstream::in);
  //ofstream outfile;
  //outfile.open(".\\REALLY.txt", ofstream::out);
  while (infile >> x >> y >> z >> scalar) {
    vol[z-1][y-1][x-1] = scalar;
	//outfile << x << " " << y << " " << z << " " << scalar << endl;
  }
  infile.close();
  //outfile.close();
	
  /* loop over the slices */
  for (int k = 0; k < dim[2]; k++ )                                      
    {                       
    /* update the progress status */
    info->UpdateProgress(info,(float)1.0*k/dim[2],"Processing..."); 
    /* see if we should abort */
    abort = atoi(info->GetProperty(info,VVP_ABORT_PROCESSING));
    /* loop over the rows and handle aborts */
    for (int j = 0; !abort && j < dim[1]; j++ )                          
      {          
      /* loop over the columns */
      for (int i = 0; i < dim[0]; i++ )
        {
        /* loop over the components */
          if(vol[k][j][i])
             *outPtr = vol[k][j][i];
          else
            *outPtr = -100;

		  *outPtr++;
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
    vtkTemplateMacro3(vvReadTextTemplate, info, pds, 
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
  void VV_PLUGIN_EXPORT vvReadTextInit(vtkVVPluginInfo *info)
  {
    /* always check the version */
    vvPluginVersionCheck();
    
    /* setup information that never changes */
    info->ProcessData = ProcessData;
    info->UpdateGUI = UpdateGUI;
    
    /* set the properties this plugin uses */
    /* TODO 4: Rename "Sample" to "<your_plugin>" */
    info->SetProperty(info, VVP_NAME, "LzfReadText");
    info->SetProperty(info, VVP_GROUP, "Utility");

    /* TODO 5: update the terse and full documentation for your filter */
    info->SetProperty(info, VVP_TERSE_DOCUMENTATION,
                      "Read a text of lines with format like 'x y z intensy'.");
    info->SetProperty(info, VVP_FULL_DOCUMENTATION,
                      "You should open any volume data first, read the text and replace it.");

    
    /* TODO 9: set these two values to "0" or "1" based on how your plugin
     * handles data all possible combinations of 0 and 1 are valid. */
    info->SetProperty(info, VVP_SUPPORTS_IN_PLACE_PROCESSING, "1");
    info->SetProperty(info, VVP_SUPPORTS_PROCESSING_PIECES,   "1");

    /* TODO 7: set the number of GUI items used by this plugin */
    info->SetProperty(info, VVP_NUMBER_OF_GUI_ITEMS,          "0");
  info->SetProperty(info, VVP_REQUIRES_SERIES_INPUT,        "0");
  info->SetProperty(info, VVP_SUPPORTS_PROCESSING_SERIES_BY_VOLUMES, "0");
  info->SetProperty(info, VVP_PRODUCES_OUTPUT_SERIES, "0");
  info->SetProperty(info, VVP_PRODUCES_PLOTTING_OUTPUT, "0");
  }
}