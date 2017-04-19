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
#include <string.h>
#include <iostream>

class Point3D{
public:
	Point3D(int x = 0, int y = 0, int z = 0): _x(x), _y(y), _z(z){}
	Point3D &operator -= (const Point3D &p2) {
		_x -= p2._x;
		_y -= p2._y;
		_z -= p2._z;
		return *this;
	}
	int distSquared(int x, int y, int z) const {
		Point3D p(x, y, z);
		p -= *this;
		return p.lengthSquared();
	}
	int lengthSquared() const {
		return _x * _x + _y * _y + _z * _z;
	}
private:
	int _x, _y, _z;
};

class Ball3D{
public:
	Ball3D(int  dim): _dim(dim), _data(new int[_dim * _dim * _dim]) {}
	virtual ~Ball3D() {
		delete[] _data;
	}
	int set(int x, int y, int z, int val) {
		return *point(x, y, z) = val;
	}
	int get(int x, int y, int z) {
		return *point(x, y, z);
	}
	int setSphere(const Point3D &center, int radius, int value) {
		int r2 = radius * radius;
		int voxelcount = 0;
		for (int i = 0; i < _dim; ++i) {
			for (int j = 0; j < _dim; ++j) {
				for (int k = 0; k < _dim; ++k) {
					if (center.distSquared(i, j, k) <= r2) {
						voxelcount++;
						set(i, j, k, value);
					} else {
						set(i, j, k, 0);
					}
				}
			}
		}
		return voxelcount;
	}
	// friend std::ostream& operator<<(std::ostream &out, const Ball3D &p) {
	// 	for (int i = 0; i < p._dim; ++i) {
	// 		out << "Dimension" << i << ":\n";
	// 		for (int j = 0; j < p._dim; ++j) {
	// 			for (int k = 0; k < p._dim; ++k) {
	// 				out << static_cast<int>(p.get(i, j, k)) << ',';
	// 			}
	// 			out << '\n';
	// 		}
	// 		out << '\n';
	// 	}
	// 	return out;
	// }
private:
	int *point(int x, int y, int z) const {
		return &_data[x + _dim * (y + _dim * z)];
	}	
	int _dim;
	int *_data;
};

int lengthSquared(int _x, int _y, int _z){
		return _x * _x + _y * _y + _z * _z;
}

int drawSphere(short*** volume, int DimX, int DimY, int DimZ, int center_x, int center_y, int center_z, int radius, int value) {
	int r2 = radius * radius;
	int changedvoxel = 0;
	for (int i = -radius; i <= radius; ++i) {
		for (int j = -radius; j <= radius; ++j) {
			for (int k = -radius; k <= radius; ++k) {
				if (0 < center_z + k && center_z + k < DimZ 
					&& 0 < center_y + j && center_y + j < DimY 
					&& 0 < center_x + i && center_x + i < DimX
					&& lengthSquared(i, j, k) <= r2 
					&& volume[center_z + k][center_y + j][center_x + i] != -1024) {
					volume[center_z + k][center_y + j][center_x + i] = value;
					changedvoxel++;
				}
			}
		}
	}
	return changedvoxel;
}

template <class IT>
/* TODO 1: Rename vvSampleTemplate to vv<your_plugin>Template */
void vvLzfStenosisDetectionTemplate(vtkVVPluginInfo *info,
					  vtkVVProcessDataStruct *pds, 
					  IT*){
	//计算中心线上的模糊距离
	IT *ptr = (IT *)pds->outData;
	IT* readVol = ptr;
	int* dim = info->InputVolumeDimensions;
	int abort = 0;
	int Xd = (int)dim[0];
	int Yd = (int)dim[1]; 
	int Zd = (int)dim[2];
	
	short*** volume = new short**[Zd];  //Never write "new (double**)[n]"
	for(int i = 0; i < Zd; i++){
		volume[i] = new short*[Yd];
		for(int j = 0; j < Yd; j++){
			volume[i][j] =  new short[Xd];
		}
	}
	for (int i = 0; i < Zd; i++){
		info->UpdateProgress(info,(float)1.0*i/Zd,"Processing..."); 
		abort = atoi(info->GetProperty(info,VVP_ABORT_PROCESSING));
		for (int j = 0;  !abort && j < Yd; j++){
			for (int k = 0; k < Xd; k++) {
				volume[i][j][k] = *readVol;
				readVol++;
			}
		}
	}


	FILE *in, *out;
	int x, y, z;
	float pct;
	char volInfo[50];
	out = fopen("console_stenosis_detection.txt", "w");
	if(in = fopen("/home/lzf/Documents/stenosis_detection/stenosis_percent.txt", "r")){
		//fscanf(in, "%s", volInfo); // 错误！一旦第一行有空格，就读到空格为止！
		//or scanf("%[^\n]", str);
		int ret;
		while(!feof(in)){
			memset(volInfo, 0, sizeof(volInfo));
			fgets(volInfo, 40, in);
			if(strcmp(volInfo, "\n") == 0) continue;  //skip empty
			sscanf(volInfo, "%d %d %d %f", &x, &y, &z, &pct);
			ret = drawSphere(volume, Xd, Yd, Zd, x, y, z, 5, (int)(pct * 12800 / 100));
			fprintf(out, "%d\n", ret);
		}
		fclose(in);
	}
	else{
		fprintf(out, "read file stenosis_percent.txt fail\n");
	}
	fclose(out);

	for (int i = 0; i < Zd; i++){
		info->UpdateProgress(info,(float)1.0*i/Zd,"Processing..."); 
		abort = atoi(info->GetProperty(info,VVP_ABORT_PROCESSING));
		for (int j = 0; !abort && j < Yd; j++){
			for (int k = 0; k < Xd; k++) {
				*ptr = volume[i][j][k];
				ptr++;
			}
		}
	}

}

static int ProcessData(void *inf, vtkVVProcessDataStruct *pds)
{
  vtkVVPluginInfo *info = (vtkVVPluginInfo *)inf;

  switch (info->InputVolumeScalarType)
    {
    /* TODO 2: Rename vvSampleTemplate to vv<your_plugin>Template */
    vtkTemplateMacro3(vvLzfStenosisDetectionTemplate, info, pds, 
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
  void VV_PLUGIN_EXPORT vvLzfStenosisDetectionInit(vtkVVPluginInfo *info)
  {
    /* always check the version */
    vvPluginVersionCheck();
    
    /* setup information that never changes */
    info->ProcessData = ProcessData;
    info->UpdateGUI = UpdateGUI;
    
    /* set the properties this plugin uses */
    /* TODO 4: Rename "Sample" to "<your_plugin>" */
    info->SetProperty(info, VVP_NAME, "LzfStenosisDetection");
    info->SetProperty(info, VVP_GROUP, "Utility");

    /* TODO 5: update the terse and full documentation for your filter */
    info->SetProperty(info, VVP_TERSE_DOCUMENTATION,
                      "Visualization for stenosis...");
    info->SetProperty(info, VVP_FULL_DOCUMENTATION,
                      "Visualization for stenosis...");

    
    /* TODO 9: set these two values to "0" or "1" based on how your plugin
     * handles data all possible combinations of 0 and 1 are valid. */
    info->SetProperty(info, VVP_SUPPORTS_IN_PLACE_PROCESSING, "1");
    info->SetProperty(info, VVP_SUPPORTS_PROCESSING_PIECES,   "1");

    /* TODO 7: set the number of GUI items used by this plugin */
    info->SetProperty(info, VVP_NUMBER_OF_GUI_ITEMS,          "0");
		info->SetProperty(info, VVP_REQUIRES_SECOND_INPUT,        "0");
  info->SetProperty(info, VVP_REQUIRES_SERIES_INPUT,        "0");
  info->SetProperty(info, VVP_SUPPORTS_PROCESSING_SERIES_BY_VOLUMES, "0");
  info->SetProperty(info, VVP_PRODUCES_OUTPUT_SERIES, "0");
  info->SetProperty(info, VVP_PRODUCES_PLOTTING_OUTPUT, "0");
  }
}