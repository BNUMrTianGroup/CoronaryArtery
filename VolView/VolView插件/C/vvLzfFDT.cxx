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
#include <list>
#include <math.h>
#include <limits.h>
using namespace std;
struct Coordinate{
	short x, y, z;
	Coordinate(short zz, short yy, short xx){
		x = xx;
		y = yy;
		z = zz;
	}
	bool operator < (const Coordinate &co) const{
		return ((z < co.z) || (z == co.z && y < co.y) || (z == co.z && y == co.y && x <co.x));
	}
};

template <class IT, class I2T>
/* TODO 1: Rename vvSampleTemplate to vv<your_plugin>Template */
void FDT(vtkVVPluginInfo *info,
                      vtkVVProcessDataStruct *pds, 
                      IT *, I2T *)
{
	IT *inPtr = (IT *)pds->inData;
	I2T *inPtr2 = (I2T *)pds->inData2;
	IT *outPtr = (IT *)pds->outData;
	int *dim = info->InputVolumeDimensions;
	int inNumComp = info->InputVolumeNumberOfComponents;
	int abort = 0;
    IT *readVol = outPtr; 
	
 
	const int neighborDirection[26][3] = {
	  {1, 0, 0},{0, 1, 0},{0, 0, 1},{-1, 0, 0},{0, -1, 0},{0, 0, -1},
	  {1, 1, 0},{1, -1, 0},{-1, 1, 0},{-1, -1, 0},
	  {0, 1, 1},{0, 1, -1},{0, -1, 1},{0, -1, -1},
	  {1, 0, 1},{1, 0, -1},{-1, 0, 1},{-1, 0, -1},
	  {1, 1, 1},{1, 1, -1},{1, -1, 1},{1, -1, -1},{-1, 1, 1},{-1, 1, -1},{-1, -1, 1},{-1,-1,-1}
	};
  
	const double EuclidDistance[26] = {
		1, 1, 1, 1, 1, 1,
		1.41421356, 1.41421356, 1.41421356, 1.41421356, 
		1.41421356, 1.41421356, 1.41421356, 1.41421356,
		1.41421356, 1.41421356, 1.41421356, 1.41421356,
		1.73205081, 1.73205081, 1.73205081, 1.73205081, 1.73205081, 1.73205081, 1.73205081, 1.73205081
	};

	const double Eps = 1e-8;

	int Xd = (int)dim[0]; //*dim
	int Yd = (int)dim[1]; //*(dim+1)
	int Zd = (int)dim[2];
	
	/*FILE *out;
	if(out = fopen("coronary_data.txt", "w")){
		fprintf(out, "(1:%d, 1:%d, 1:%d) InputVolumeNumberOfComponents=%d\n", Xd, Yd, Zd, inNumComp);
	}*/

	FILE *console;
	console = fopen("fdt_info.txt", "w");
	
	int nonBackground = 0 ;
	double graySum = 0, meanVal, stdVal2;
	for (int i = 0; i < Zd; i++){
		info->UpdateProgress(info,(float)1.0*i/Zd,"Computing MEAN..."); 
		abort = atoi(info->GetProperty(info,VVP_ABORT_PROCESSING));
		for (int j = 0; !abort && j < Yd; j++){
			for (int k = 0; k < Xd; k++) {
				if(*readVol != -1024){ 
				  graySum += *readVol;
				  nonBackground++;
				}
				readVol++;
			}
		}
	}
	fprintf(console, "graySum = %f\n", graySum);
	meanVal = graySum / nonBackground;
	readVol = outPtr;
	graySum = 0;
	for (int i = 0; i < Zd; i++){
		info->UpdateProgress(info,(float)1.0*i/Zd,"Computing STD..."); 
		abort = atoi(info->GetProperty(info,VVP_ABORT_PROCESSING));
		for (int j = 0; !abort && j < Yd; j++){
			for (int k = 0; k < Xd; k++) {
				if(*readVol != -1024){ 
					graySum += (*readVol - meanVal) * (*readVol - meanVal);
				}
				readVol++;
			}
		}
	}
	//stdVal = sqrt(graySum / nonBackground);
	fprintf(console, "sum((x-u)^2) = %f\n", graySum);
	stdVal2 = graySum / nonBackground;
	fprintf(console, "mean = %f, std2 = %f\n", meanVal, stdVal2);

	
	readVol = outPtr;
	short*** GrayScalar = new short**[Zd];  //Never write "new (double**)[n]"
	for(int i = 0; i < Zd; i++){
		GrayScalar[i] = new short*[Yd];
		for(int j = 0; j < Yd; j++){
			GrayScalar[i][j] =  new short[Xd];
		}
	}
	for (int i = 0; i < Zd; i++){
		info->UpdateProgress(info,(float)1.0*i/Zd,"Reading into memory..."); 
		abort = atoi(info->GetProperty(info,VVP_ABORT_PROCESSING));
		for (int j = 0;  !abort && j < Yd; j++){
			for (int k = 0; k < Xd; k++) {
				GrayScalar[i][j][k] = (short)*readVol;
				readVol++;
			}
		}
	}

	
	list<Coordinate> Q;
	double*** mu = new double**[Zd]; 
	for(int i = 0; i < Zd; i++){
		mu[i] = new double*[Yd];
		for(int j = 0; j < Yd; j++){
			mu[i][j] =  new double[Xd];
			for(int k = 0; k < Xd; k++){
				//if(GrayScalar[i][j][k] - meanVal > numeric_limits<double>::epsilon()){
				if((double)GrayScalar[i][j][k] - meanVal > Eps){
					mu[i][j][k] = 1.0;
				}
				else{
					mu[i][j][k] = exp(-0.5 * pow((double)GrayScalar[i][j][k] - meanVal, 2) / stdVal2);
				}
			}
		}
	}

	double*** omega = new double**[Zd]; 
	for(int i = 0; i < Zd; i++){
		omega[i] = new double*[Yd];
		for(int j = 0; j < Yd; j++){
			omega[i][j] =  new double[Xd];
			for(int k = 0; k < Xd; k++){
				if(fabs((double)GrayScalar[i][j][k] + 1024.0) > Eps){ //GrayScalar[i][j][k] != -1024, object
					omega[i][j][k] = INT_MAX;
				}
				else{  //background
					omega[i][j][k] = 0;
					int qi, qj, qk;
					for(int h = 0; h < 26; h++){
						qi = i + neighborDirection[h][0];
						qj = j + neighborDirection[h][1];
						qk = k + neighborDirection[h][2];
						if(qi >= Zd || qi < 0 || qj >= Yd || qj < 0 || qk > Xd || qk < 0 ) continue;
						if(fabs((double)GrayScalar[qi][qj][qk] + 1024.0) > Eps){
							Coordinate co(i, j, k);
							Q.push_back(co);
							break;
						}
					}
				}
			}
		}
	}
	//fprintf(console, "size of Queue = %d\n", Q.size());
	fclose(console);
	
	double maxOmega = 0;
	while(!Q.empty()){
		
		//info->UpdateProgress(info,(float)1.0*Q.size()/1000000,"Queue..."); //这里的进度条写法太扯了
		Coordinate pco = Q.front();
		Q.pop_front();
		int pi = pco.z, pj = pco.y, pk = pco.x, qi, qj, qk;
		double link;
		for(int h = 0; h < 26; h++){
			qi = pi + neighborDirection[h][0];
			qj = pj + neighborDirection[h][1];
			qk = pk + neighborDirection[h][2];
			if(qi >= Zd || qi < 0 || qj >= Yd || qj < 0 || qk > Xd || qk < 0 ) continue;
			link = 0.5 * (mu[pi][pj][pk] + mu[qi][qj][qk]) * EuclidDistance[h];
			if(omega[qi][qj][qk] - (omega[pi][pj][pk] + link) > Eps){
				omega[qi][qj][qk] = omega[pi][pj][pk] + link;
				if(omega[qi][qj][qk] > maxOmega) maxOmega = omega[qi][qj][qk];
				Coordinate qco(qi, qj, qk);
				Q.push_back(qco);
			}
		}
	}

	FILE *ske;
	ske = fopen("skeleton_diameter.txt", "w");

	//visualize the fdt image
	for (int i = 0; i < Zd; i++){
		info->UpdateProgress(info,(float)1.0*i/Zd,"Writing Volume..."); 
		abort = atoi(info->GetProperty(info,VVP_ABORT_PROCESSING));
		for (int j = 0; !abort && j < Yd; j++){
			for (int k = 0; k < Xd; k++) {
				*outPtr =  (int)(100 * omega[i][j][k] / maxOmega);
				if(*inPtr2)
					fprintf(ske, "%d %d %d %d\n", k, j, i, *inPtr2);
				outPtr++;
				inPtr2++;
			}
		}
	}

	fclose(ske);

	for(int i = 0; i < Zd; i++){
		for(int j = 0; j < Yd; j++){
			delete[] GrayScalar[i][j];
		}
		delete[] GrayScalar[i];
	}
  
	delete[] GrayScalar;

	for(int i = 0; i < Zd; i++){
		for(int j = 0; j < Yd; j++){
			delete[] mu[i][j];
		}
		delete[] mu[i];
	}
	delete[] mu;

	for(int i = 0; i < Zd; i++){
		for(int j = 0; j < Yd; j++){
			delete[] omega[i][j];
		}
		delete[] omega[i];
	}
	delete[] omega;
                                                    
	info->UpdateProgress(info,(float)1.0,"Processing Complete");
}

template <class IT>
/* TODO 1: Rename vvSampleTemplate to vv<your_plugin>Template */
void vvLzfFDTTemplate(vtkVVPluginInfo *info,
                      vtkVVProcessDataStruct *pds, 
                      IT *)
{
	//以下方法解决了两幅图像位长不相同的问题。
	switch (info->InputVolume2ScalarType)
	{
		// invoke the appropriate templated function
		vtkTemplateMacro4(FDT, info, pds, static_cast<IT *>(0), static_cast<VTK_TT *>(0));
	}
                                                    
	info->UpdateProgress(info,(float)1.0,"Processing Complete");
}


static int ProcessData(void *inf, vtkVVProcessDataStruct *pds)
{
  vtkVVPluginInfo *info = (vtkVVPluginInfo *)inf;

  switch (info->InputVolumeScalarType)
    {
    /* TODO 2: Rename vvSampleTemplate to vv<your_plugin>Template */
    vtkTemplateMacro3(vvLzfFDTTemplate, info, pds, 
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
  void VV_PLUGIN_EXPORT vvLzfFDTInit(vtkVVPluginInfo *info)
  {
    /* always check the version */
    vvPluginVersionCheck();
    
    /* setup information that never changes */
    info->ProcessData = ProcessData;
    info->UpdateGUI = UpdateGUI;
    
    /* set the properties this plugin uses */
    /* TODO 4: Rename "Sample" to "<your_plugin>" */
    info->SetProperty(info, VVP_NAME, "LzfFDT");
    info->SetProperty(info, VVP_GROUP, "Utility");

    /* TODO 5: update the terse and full documentation for your filter */
    info->SetProperty(info, VVP_TERSE_DOCUMENTATION,
                      "Computing fuzzy distance transform at each voxel and write into a file values at skeleton voxels.");
    info->SetProperty(info, VVP_FULL_DOCUMENTATION,
                      "You need to open the skeleton image as the second input. This plugin is originally created on Jun, 2015, referred to Xu Yan's paper.");

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