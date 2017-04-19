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
#include <map>
#include <set>
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
void generateStatisticsOrPly(vtkVVPluginInfo *info,  IT* ptr, int filetype){
	FILE *out;
	int abort;
	int* dim = info->InputVolumeDimensions;
	int nonzero = 0;
	int Xd = (int)dim[0];
	int Yd = (int)dim[1]; 
	int Zd = (int)dim[2];
	switch(filetype){
		case 0:
			if(out = fopen("coronary_skeleton.txt", "w")){
				fprintf(out, "%d x %d x %d voxels, InputVolumeNumberOfComponents = %d\n", Xd, Yd, Zd, info->InputVolumeNumberOfComponents);
			}
			for (int i = 0; i < Zd; i++){
				info->UpdateProgress(info,(float)1.0*i/Zd,"Processing..."); 
				abort = atoi(info->GetProperty(info,VVP_ABORT_PROCESSING));
				for (int j = 0;  !abort && j < Yd; j++){
					for (int k = 0; k < Xd; k++) {
						//grayScalar[i][j][k] = *readVol;
						if(*ptr){
							fprintf(out, "%d %d %d\n", i, j, k);
						}
						ptr++;
					}
				}
			}

			break;
		case 1:
			if(out = fopen("coronary_data.ply", "w")){
				fprintf(out, "ply\nformat ascii 1.0\ncomment VCGLIB generated\nelement vertex          \nproperty float x\nproperty float y\nproperty float z\nelement face 0\nproperty list uchar int vertex_indices\nend_header\n");
			}
			for (int i = 0; i < Zd; i++){
				info->UpdateProgress(info,(float)1.0*i/Zd,"Processing..."); 
				abort = atoi(info->GetProperty(info,VVP_ABORT_PROCESSING));
				for (int j = 0;  !abort && j < Yd; j++){
					for (int k = 0; k < Xd; k++) {
						//grayScalar[i][j][k] = *readVol;
						if(*ptr){
							//fprintf(out, "%f %f %f\n", 100.0*k/Xd, 100.0*j/Yd, 100.0*i/Zd);
							fprintf(out, "%d %d %d\n", i, j, k);
							nonzero++;
						}
						ptr++;
					}
				}
			}
			fseek(out, 64, SEEK_SET); //文件指针重新指向文件头
			fprintf(out, "%d", nonzero); //写点数（vertex）
			break;
			
		default:break;
	}

	/*
	short *** grayScalar = new short**[Zd];  //Never write "new (double**)[n]"
	for(int i = 0; i < Zd; i++){
		grayScalar[i] = new short*[Yd];
		for(int j = 0; j < Yd; j++){
			grayScalar[i][j] =  new short[Xd];
		}
	}*/
  
	//output the statistics of HU
	/*
	std::map<int, int> stat;
	fprintf(out, "-------statistics-------\n");
	fprintf(out, "Non-zero pixel:%d\n", nonzpix);
	for(std::map<int, int>::const_iterator it = stat.begin(); it != stat.end(); it++){
	  fprintf(out, "HU[%d]=%d\n", it->first, it->second);
	}*/
  

	/*
	for(int i = 0; i < Zd; i++){
		for(int j = 0; j < Yd; j++){
			delete[] grayScalar[i][j];
		}
		delete[] grayScalar[i];
	}
	delete[] grayScalar;*/
  
	fclose(out);
}

//添加于2016年1月25日。为王佳欣论文用。统计每一片的有效体素个数。
//修改于1月28日。更名：slicePoints->volumePoints。计算去噪结果（图像2）与ground truth（图像1）的相似性。
template <class IT, class I2T>
void volumePoints(vtkVVPluginInfo *info, vtkVVProcessDataStruct *pds, IT *, I2T *){
	
	IT *ptr1 = (IT *)pds->outData;
	I2T *ptr2 = (I2T *)pds->inData2;
	int *dim = info->InputVolumeDimensions;
	FILE *out;
	int abort;
	
	int Xd = (int)dim[0];
	int Yd = (int)dim[1]; 
	int Zd = (int)dim[2];
	
	int*** V1 = new int**[Zd];  

	int nonzero1 = 0, nonzero2 = 0, intersection = 0;
	for (int i = 0; i < Zd; i++){
		info->UpdateProgress(info,(float)1.0*i/Zd,"Reading ground truth..."); 
		abort = atoi(info->GetProperty(info,VVP_ABORT_PROCESSING));
		V1[i] = new int*[Yd];
		for (int j = 0;  !abort && j < Yd; j++){
			V1[i][j] =  new int[Xd]();
			for (int k = 0; k < Xd; k++) {
				if(*ptr1>0){
					nonzero1++;
					V1[i][j][k] = *ptr1;
				}
				ptr1++;
			}
		}
	}

	for (int i = 0; i < Zd; i++){
		info->UpdateProgress(info,(float)1.0*i/Zd,"Reading this volume..."); 
		abort = atoi(info->GetProperty(info,VVP_ABORT_PROCESSING));
		for (int j = 0;  !abort && j < Yd; j++){
			for (int k = 0; k < Xd; k++) {
				if(*ptr2>0){
					nonzero2++;
					if(V1[i][j][k] > 0) intersection++;
				}
				ptr2++;
			}
		}
	}
  
	out = fopen("volume_points.txt", "w");
	fprintf(out, "ground truth:%d, volume points:%d, similarity:%lf\n", nonzero1, nonzero2, 2.0 * intersection/(nonzero1 + nonzero2));
	fclose(out);

	for(int i = 0; i < Zd; i++){
		for(int j = 0; j < Yd; j++){
			delete[] V1[i][j];
		}
		delete[] V1[i];
	}
	delete[] V1;
}

template <class IT>
void reserveSurfacePoints(vtkVVPluginInfo *info,  IT* ptr){
	//保留二值“实心”图像的表面体素
	int* dim = info->InputVolumeDimensions;
	int Xd = (int)dim[0];
	int Yd = (int)dim[1]; 
	int Zd = (int)dim[2];
	int abort;
	IT* readVol = ptr;

	/*FILE* out;
	if(out = fopen("coronary_surface.txt", "w")){
		fprintf(out, "%d x %d x %d voxels, InputVolumeNumberOfComponents = %d\n", Xd, Yd, Zd, inc);
	}*/

	short *** grayScalar = new short**[Zd];  //Never write "new (double**)[n]"
	for(int i = 0; i < Zd; i++){
		grayScalar[i] = new short*[Yd];
		for(int j = 0; j < Yd; j++){
			grayScalar[i][j] =  new short[Xd];
		}
	}

	bool*** border = new bool**[Zd];   //标记是否为边界点
	for(int i = 0; i < Zd; i++){
		border[i] = new bool*[Yd];
		for(int j = 0; j < Yd; j++){
			border[i][j] =  new bool[Xd];
			for(int k = 0; k < Xd; k++)
				border[i][j][k] = false;
		}
	}
	
	for (int i = 0; i < Zd; i++){
		info->UpdateProgress(info,(float)1.0*i/Zd,"Processing..."); 
		abort = atoi(info->GetProperty(info,VVP_ABORT_PROCESSING));
		for (int j = 0;  !abort && j < Yd; j++){
			for (int k = 0; k < Xd; k++) {
				grayScalar[i][j][k] = (short)*readVol;
				readVol++;
			}
		}
	}

	const int neighborDirection[26][3] = {
		{1, 0, 0},{0, 1, 0},{0, 0, 1},{-1, 0, 0},{0, -1, 0},{0, 0, -1},
		{1, 1, 0},{1, -1, 0},{-1, 1, 0},{-1, -1, 0},
		{0, 1, 1},{0, 1, -1},{0, -1, 1},{0, -1, -1},
		{1, 0, 1},{1, 0, -1},{-1, 0, 1},{-1, 0, -1},
		{1, 1, 1},{1, 1, -1},{1, -1, 1},{1, -1, -1},{-1, 1, 1},{-1, 1, -1},{-1, -1, 1},{-1,-1,-1}
	};
	for (int i = 0; i < Zd; i++){
		info->UpdateProgress(info,(float)1.0*i/Zd,"Processing..."); 
		abort = atoi(info->GetProperty(info,VVP_ABORT_PROCESSING));
		for (int j = 0; !abort && j < Yd; j++){
			for (int k = 0; k < Xd; k++) {
				if(grayScalar[i][j][k]){
				  int countBackground = 0;
				  for(int h = 0; h < 26; h++){
					  if(!grayScalar[i + neighborDirection[h][0]][j + neighborDirection[h][1]][k + neighborDirection[h][2]])
						  countBackground++;
				  }
				  if(countBackground >= 9)  //26邻域里有多少是背景点算作边界，可调
					  border[i][j][k] = true;
				}
			}
		}
	}
  
	for (int i = 0; i < Zd; i++){
		info->UpdateProgress(info,(float)1.0*i/Zd,"Processing..."); 
		abort = atoi(info->GetProperty(info,VVP_ABORT_PROCESSING));
		for (int j = 0; !abort && j < Yd; j++){
			for (int k = 0; k < Xd; k++) {
				if(!border[i][j][k]){ 
					*ptr = 0;   //Min of gray scalar，可调
				}
				//输出功能放到generateStatisticsOrPly里去实现
				/*else{
					fprintf(out, "%.3f %.3f %.3f\n", 10.0 * i / Xd, 10.0 * j / Yd, 10.0 * k / Zd);
				}*/
				ptr++;
			}
		}
	}

	for(int i = 0; i < Zd; i++){
		for(int j = 0; j < Yd; j++){
			delete[] grayScalar[i][j];
		}
		delete[] grayScalar[i];
	}
	delete[] grayScalar;

	for(int i = 0; i < Zd; i++){
		for(int j = 0; j < Yd; j++){
			delete[] border[i][j];
		}
		delete[] border[i];
	}
	delete[] border;
	
	//fclose(out);
}

template <class IT>
void stenosisVisualization(vtkVVPluginInfo *info,  IT* ptr){
	//可视化狭窄情况
	int* dim = info->InputVolumeDimensions;
	int Xd = (int)dim[0];
	int Yd = (int)dim[1]; 
	int Zd = (int)dim[2];
	int abort;
	IT* readVol = ptr;


	short *** grayScalar = new short**[Zd];  //Never write "new (double**)[n]"
	for(int i = 0; i < Zd; i++){
		grayScalar[i] = new short*[Yd];
		for(int j = 0; j < Yd; j++){
			grayScalar[i][j] =  new short[Xd];
		}
	}
	
	for (int i = 0; i < Zd; i++){
		info->UpdateProgress(info,(float)1.0*i/Zd,"Reading into memory..."); 
		abort = atoi(info->GetProperty(info,VVP_ABORT_PROCESSING));
		for (int j = 0; !abort && j < Yd; j++){
			for (int k = 0; k < Xd; k++) {
				grayScalar[i][j][k] = (short)*readVol;
				readVol++;
			}
		}
	}

	FILE *in;
	int x, y, z;
	double stenosisPercent;  //stenosisPercent
	map<Coordinate, double> pointMap;
	char volInfo[102];
	if((in = fopen("stenosis_persent.txt","r")) == NULL) {  //判断文件是否存在及可读		
			printf("error!"); 
			return; 
	} 
	while (!feof(in)) { 
			fgets(volInfo,100,in);  //读取一行
			if(4 == fscanf(in, "%d %d %d %lf", &z, &y, &x, &stenosisPercent)){
				Coordinate co(z, y, x);
				pointMap[co] = stenosisPercent;
			}
	} 
	fclose(in);                     //关闭文件*/

	FILE *console;
	console = fopen("console.log", "w");
	fprintf(console, "size of map: %d\n", (int)pointMap.size());
	

	for (map<Coordinate, double>::iterator it = pointMap.begin(); it != pointMap.end(); it++){
		Coordinate pco = it->first;
		int z = pco.z, y = pco.y, x = pco.x;
		double pct = it->second;
		int r = 1 + (int)(6.0 * (1.0 - pct));
		fprintf(console, "r = %d, gray = %d\n", r, 25 + (int)(230.0 * pct));
		for(int k = -r; k <= r; k++){
			for(int j = -r; j <= r; j++){
				for(int i = -r; i <= r; i++){
					if (grayScalar[z+k][y+i][x+j])
						grayScalar[z+k][y+i][x+j] = 25 + (int)(230.0 * pct);
					}
				}
			}
		}
	fclose(console);
	for (int i = 0; i < Zd; i++){
		info->UpdateProgress(info,(float)1.0*i/Zd,"Writing volume..."); 
		abort = atoi(info->GetProperty(info,VVP_ABORT_PROCESSING));
		for (int j = 0; !abort && j < Yd; j++){
			for (int k = 0; k < Xd; k++) {
				*ptr = grayScalar[i][j][k];   
				ptr++;
			}
		}
	}

	for(int i = 0; i < Zd; i++){
		for(int j = 0; j < Yd; j++){
			delete[] grayScalar[i][j];
		}
		delete[] grayScalar[i];
	}
	delete[] grayScalar;

}


template <class IT>
void connectedComponentStatistics(vtkVVPluginInfo *info,  IT* ptr){
	//对脑血管的连通分量进行原始图像上的统计
	int* dim = info->InputVolumeDimensions;
	int Xd = (int)dim[0];
	int Yd = (int)dim[1]; 
	int Zd = (int)dim[2];
	int abort;
	IT* readVol = ptr;


	short *** grayScalar = new short**[Zd];  //Never write "new (double**)[n]"
	for(int i = 0; i < Zd; i++){
		grayScalar[i] = new short*[Yd];
		for(int j = 0; j < Yd; j++){
			grayScalar[i][j] =  new short[Xd];
		}
	}

	for (int i = 0; i < Zd; i++){
		info->UpdateProgress(info,(float)1.0*i/Zd,"Reading into memory..."); 
		abort = atoi(info->GetProperty(info,VVP_ABORT_PROCESSING));
		for (int j = 0; !abort && j < Yd; j++){
			for (int k = 0; k < Xd; k++) {
				grayScalar[i][j][k] = (short)*readVol;
				readVol++;
			}
		}
	}

	FILE *in;
	int x, y, z;
	char volInfo[102];
	if((in = fopen("log_connectivity.txt","r")) == NULL) {  //判断文件是否存在及可读		
		printf("error!"); 
		return; 
	} 

	map<int, double> averageGray;
	double graySum = 0.0;
	int vNum = 0, ccNO = 0;  //连通分量的体素数量，连通分量编号
	bool start = false, firstcc = true;
	while (!feof(in)) { 
		fgets(volInfo,100,in);  //读取一行
		
		if(!start) {
			if(volInfo[0] == '*') {
				start = true;
			}
			continue;
		}

		if(volInfo[0] == '#') {
			if(firstcc) {
				firstcc = false;
			}
			else{
				averageGray[ccNO] = graySum / vNum;
			}

			graySum = 0.0;
			vNum = 0;
			ccNO = 0;
			fscanf(in, "#%d $%d", &ccNO, &vNum);

			continue;
		}
		if(3 == fscanf(in, "%d %d %d", &z, &y, &x)){
			graySum += grayScalar[z][y][x];
			vNum++;
		}
	} 
	averageGray[ccNO] = graySum / vNum;
	fclose(in);                     //关闭文件*/


	FILE *console;
	console = fopen("log_original_gray", "w");
	fprintf(console, "size of components: %d\n", (int)averageGray.size());
	for(map<int, double>::iterator it = averageGray.begin(); it != averageGray.end(); it++) {
		fprintf(console, "#%d  %.4f\n", it->first, it->second);
	}
	
	fclose(console);

	for(int i = 0; i < Zd; i++){
		for(int j = 0; j < Yd; j++){
			delete[] grayScalar[i][j];
		}
		delete[] grayScalar[i];
	}
	delete[] grayScalar;

}

template <class IT, class I2T>
void coronaryBw2Gray(vtkVVPluginInfo *info,
								 vtkVVProcessDataStruct *pds, 
								 IT *, I2T *)
{
	//2015-11-27注：这个函数应该用不到了吧。VolView自带的Utility->Masking(ITK)就可以实现相同功能。
	IT *ptr = (IT *)pds->outData;
	I2T *ptr2 = (I2T *)pds->inData2;

	int *dim = info->InputVolumeDimensions;

	int abort = 0;
	int inNumCom = info->InputVolumeNumberOfComponents;
	int inNumCom2 = info->InputVolume2NumberOfComponents;
	
	//将分割好的二值图像匹配到原灰度图像

	int Xd = (int)dim[0];
	int Yd = (int)dim[1]; 
	int Zd = (int)dim[2];

	/*
	FILE *in;
	int x, y, z;
	set<Coordinate> pointSet;
	char volInfo[102];
	if(in = fopen("coronary_data.txt", "r")){
		//fscanf(in, "%s", volInfo); // 错误！一旦第一行有空格，就读到空格为止！
		fgets(volInfo, 100, in);  //or scanf("%[^\n]", str);
		while(3 == fscanf(in, "%d %d %d", &z, &y, &x)){
			Coordinate co(z, y, x);
			pointSet.insert(co);
		}
		fclose(in);
	}
	else{
		perror("coronary_data.txt");  //print "coronary_data.txt: No such file or directory"
	}
	*/
	
	/*FILE *console;
	console = fopen("console.log", "w");
	fprintf(console, "%s", volInfo);
	if(!pointSet.empty())
		fprintf(console, "size of set: %d\n", (int)pointSet.size());
	fclose(console);*/
	
	for (int i = 0; i < Zd; i++){
		info->UpdateProgress(info,(float)1.0*i/Zd,"Processing..."); 
		abort = atoi(info->GetProperty(info,VVP_ABORT_PROCESSING));
		for (int j = 0; !abort && j < Yd; j++){
			for (int k = 0; k < Xd; k++) {
				if(!*ptr2) 
					*ptr = -1024;
				ptr++;
				ptr2++;
			}
		}
	}
}


template <class IT>
/* TODO 1: Rename vvSampleTemplate to vv<your_plugin>Template */
void vvLzfStatisticsTemplate(vtkVVPluginInfo *info,
                      vtkVVProcessDataStruct *pds, 
                      IT *)
{
	IT *inPtr1 = (IT *)pds->inData;
	IT *outPtr1 = (IT *)pds->outData;
	int *dim1 = info->InputVolumeDimensions;
	int inNumComp1 = info->InputVolumeNumberOfComponents;

	//下面这样直接认定第二图像是某类型的写法不太好！但数据位长不统一，容易出错（都设为IT*（第一个图像的类型），指针自增的时候，位长短的图像提前访问到尾部）。先这样吧。。。
	unsigned char *inPtr2 = (unsigned char*)pds->inData2;  
	//对于coronaryBw2Gray，打开的是原始的灰度图像（2字节），图像2是分割好的血管二值图像（如果是心脏区域的提取也可以）（1字节）

	//对于coronaryStenosis，打开的是fdt后的图像（2字节），图像2是血管骨架线二值图像（1字节）
	int *dim2 = info->InputVolume2Dimensions;
	int inNumComp2 = info->InputVolume2NumberOfComponents;

	int Xd = (int)dim1[0];
	int Yd = (int)dim1[1]; 
	int Zd = (int)dim1[2];
  /*
  *The following code is writen by lzf on May 22, 2015
  *for save as a .ply file
  */
	
	//reserveSurfacePoints(info, outPtr1);
	
	//generateStatisticsOrPly(info, outPtr1, 1); //0:txt 1:ply

	//stenosisVisualization(info, outPtr1);

	connectedComponentStatistics(info, outPtr1);

	//以下方法解决了两幅图像位长不相同的问题【并不能！】。
	/*switch (info->InputVolume2ScalarType)
	{
		//invoke the appropriate templated function
		vtkTemplateMacro4(volumePoints, info, pds, 
			static_cast<IT *>(0), static_cast<VTK_TT *>(0));
	}*/
                                                    
	info->UpdateProgress(info,(float)1.0,"Processing Complete");
}

static int ProcessData(void *inf, vtkVVProcessDataStruct *pds)
{
  vtkVVPluginInfo *info = (vtkVVPluginInfo *)inf;

  switch (info->InputVolumeScalarType)
    {
    /* TODO 2: Rename vvSampleTemplate to vv<your_plugin>Template */
    vtkTemplateMacro3(vvLzfStatisticsTemplate, info, pds, 
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
  void VV_PLUGIN_EXPORT vvLzfStatisticsInit(vtkVVPluginInfo *info)
  {
    /* always check the version */
    vvPluginVersionCheck();
    
    /* setup information that never changes */
    info->ProcessData = ProcessData;
    info->UpdateGUI = UpdateGUI;
    
    /* set the properties this plugin uses */
    /* TODO 4: Rename "Sample" to "<your_plugin>" */
    info->SetProperty(info, VVP_NAME, "LzfStatistics");
    info->SetProperty(info, VVP_GROUP, "Utility");

    /* TODO 5: update the terse and full documentation for your filter */
    info->SetProperty(info, VVP_TERSE_DOCUMENTATION,
                      "This plugin includes several functions...");
    info->SetProperty(info, VVP_FULL_DOCUMENTATION,
                      "This plugin is created on May 22nd, 2015, and being expanded. You only need to import the second image when 'coronaryBw2Gray'.");

    
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
