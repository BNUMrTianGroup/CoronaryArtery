#include "vtkVVPluginAPI.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
using namespace std;

const int dir[26][3] = {
	{1, 0, 0},{0, 1, 0},{0, 0, 1},{-1, 0, 0},{0, -1, 0},{0, 0, -1},
	{1, 1, 0},{1, -1, 0},{-1, 1, 0},{-1, -1, 0},
	{0, 1, 1},{0, 1, -1},{0, -1, 1},{0, -1, -1},
	{1, 0, 1},{1, 0, -1},{-1, 0, 1},{-1, 0, -1},
	{1, 1, 1},{1, 1, -1},{1, -1, 1},{1, -1, -1},{-1, 1, 1},{-1, 1, -1},{-1, -1, 1},{-1,-1,-1}
};

bool*** tag;
int*** V1;
int*** V2;
int Xd, Yd, Zd;

void dfs(int s, int r, int c){	
	if(s < 0 || s >= Zd || r < 0 || r >= Yd || c < 0 || c >= Xd) return;
	if(tag[s][r][c] || V1[s][r][c] < 0) return; //V1<=-100
	tag[s][r][c] = true;
	for(int h = 0; h < 26; h++)
		dfs(s + dir[h][0], r + dir[h][1], c + dir[h][2]);
}

template <class IT, class I2T>
void reserveVessels(vtkVVPluginInfo *info,
								 vtkVVProcessDataStruct *pds, 
								 IT *, I2T *)
{
	IT *outPtr1 = (IT *)pds->outData;
	IT *inPtr1 = (IT *)pds->inData;
	I2T *inPtr2 = (I2T *)pds->inData2;

	int *dim = info->InputVolumeDimensions;

	int abort = 0;
	int inNumCom = info->InputVolumeNumberOfComponents;
	int inNumCom2 = info->InputVolume2NumberOfComponents;

	Xd = (int)dim[0];
	Yd = (int)dim[1]; 
	Zd = (int)dim[2];

	ofstream outfile;
	outfile.open(".\\log_remove_noise.txt", ofstream::out);
	outfile << Xd << "*" << Yd << "*"<< Zd << endl;

	/*FILE *console;
	console = fopen("console.log", "w");
	fprintf(console, "%s", volInfo);
	if(!pointSet.empty())
		fprintf(console, "size of set: %d\n", (int)pointSet.size());
	fclose(console);*/

	V2 = new int**[Zd];  
	for (int i = 0; i < Zd; i++){
		info->UpdateProgress(info,(float)1.0*i/Zd,"Reading Frangi result into memory..."); 
		abort = atoi(info->GetProperty(info,VVP_ABORT_PROCESSING));
		V2[i] = new int*[Yd];
		for (int j = 0; !abort && j < Yd; j++){
			V2[i][j] = new int[Xd];
			for (int k = 0; k < Xd; k++) {
				V2[i][j][k] = (int)*inPtr2;
				inPtr2++;
			}
		}
	}

	V1 = new int**[Zd];  
	for (int i = 0; i < Zd; i++){
		info->UpdateProgress(info,(float)1.0*i/Zd,"Reading Level Set segmentation result into memory..."); 
		abort = atoi(info->GetProperty(info,VVP_ABORT_PROCESSING));
		V1[i] = new int*[Yd];
		for (int j = 0; !abort && j < Yd; j++){
			V1[i][j] =  new int[Xd];
			for (int k = 0; k < Xd; k++) {
				V1[i][j][k] = (int)*inPtr1;
				inPtr1++;
			}
		}
	}

	tag = new bool**[Zd];  
	for(int i = 0; i < Zd; i++){
		tag[i] = new bool*[Yd];
		for(int j = 0; j < Yd; j++){
			tag[i][j] =  new bool[Xd];
			for(int k = 0; k < Xd; k++){
				tag[i][j][k] = false;
			}
		}
	}

	//int cz, cy, cx, nz, ny, nx; 

	for (int i = 0; i < Zd; i++ ){                       
		info->UpdateProgress(info,(float)1.0*i/Zd,"Computing connected components..."); 
		abort = atoi(info->GetProperty(info,VVP_ABORT_PROCESSING));
		for (int j = 0; !abort && j < Yd; j++ ){
			for (int k = 0; k < Xd; k++ ){
				if(V1[i][j][k] >= 0 && V2[i][j][k] > 0 && !tag[i][j][k]){  //在非背景上的要保留的种子点//V1>-100
					//outfile << "seed: " << k << " " << j << " "<< i << endl;
					dfs(i, j, k);
				}
			}
		}
	}


	
	for (int i = 0; i < Zd; i++){
		info->UpdateProgress(info,(float)1.0*i/Zd,"Modifying data..."); 
		abort = atoi(info->GetProperty(info,VVP_ABORT_PROCESSING));
		for (int j = 0; !abort && j < Yd; j++){
			for (int k = 0; k < Xd; k++) {
				if(!tag[i][j][k])  *outPtr1 = -100;  //-100
				else outfile << k << " " << j << " "<< i << endl;
				outPtr1++;
			}
		}
	}

	outfile.close();

	for(int i = 0; i < Zd; i++){
		for(int j = 0; j < Yd; j++){
			delete[] tag[i][j];
		}
		delete[] tag[i];
	}
	delete[] tag;

	for(int i = 0; i < Zd; i++){
		for(int j = 0; j < Yd; j++){
			delete[] V1[i][j];
		}
		delete[] V1[i];
	}
	delete[] V1;

	for(int i = 0; i < Zd; i++){
		for(int j = 0; j < Yd; j++){
			delete[] V2[i][j];
		}
		delete[] V2[i];
	}
	delete[] V2;
}


template <class IT>
/* TODO 1: Rename vvSampleTemplate to vv<your_plugin>Template */
void vvLzfRemoveNoiseTemplate(vtkVVPluginInfo *info,
                      vtkVVProcessDataStruct *pds, 
                      IT *)
{
	//以下方法解决了两幅图像位长不相同的问题。
	switch (info->InputVolume2ScalarType)
	{
		//invoke the appropriate templated function
		vtkTemplateMacro4(reserveVessels, info, pds, 
			static_cast<IT *>(0), static_cast<VTK_TT *>(0));
	}
                                                    
	info->UpdateProgress(info,(float)1.0,"Processing Complete");
}

static int ProcessData(void *inf, vtkVVProcessDataStruct *pds)
{
  vtkVVPluginInfo *info = (vtkVVPluginInfo *)inf;

  switch (info->InputVolumeScalarType)
    {
    /* TODO 2: Rename vvSampleTemplate to vv<your_plugin>Template */
    vtkTemplateMacro3(vvLzfRemoveNoiseTemplate, info, pds, 
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
  void VV_PLUGIN_EXPORT vvLzfRemoveNoiseInit(vtkVVPluginInfo *info)
  {
    /* always check the version */
    vvPluginVersionCheck();
    
    /* setup information that never changes */
    info->ProcessData = ProcessData;
    info->UpdateGUI = UpdateGUI;
    
    /* set the properties this plugin uses */
    /* TODO 4: Rename "Sample" to "<your_plugin>" */
    info->SetProperty(info, VVP_NAME, "LzfRemoveNoise");
    info->SetProperty(info, VVP_GROUP, "Utility");

    /* TODO 5: update the terse and full documentation for your filter */
    info->SetProperty(info, VVP_TERSE_DOCUMENTATION,
                      "Remove noise under the direction of another volume..");
    info->SetProperty(info, VVP_FULL_DOCUMENTATION,
		"This plugin is created on Jan 22nd, 2016.Wang J.X.'s paper on vessel segmentation using level set. The second volume: improved frangi method & threshold & connectivity.");

    
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