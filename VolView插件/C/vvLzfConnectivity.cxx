#include "vtkVVPluginAPI.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <vector>
#include <map>
#include <algorithm>
#include <stack>
using namespace std;

struct Voxel{
	short x, y, z;
	Voxel(short zz, short yy, short xx){
		x = xx;
		y = yy;
		z = zz;
	}
};

const int dir[26][3] = {
	{1, 0, 0},{0, 1, 0},{0, 0, 1},{-1, 0, 0},{0, -1, 0},{0, 0, -1},
	{1, 1, 0},{1, -1, 0},{-1, 1, 0},{-1, -1, 0},
	{0, 1, 1},{0, 1, -1},{0, -1, 1},{0, -1, -1},
	{1, 0, 1},{1, 0, -1},{-1, 0, 1},{-1, 0, -1},
	{1, 1, 1},{1, 1, -1},{1, -1, 1},{1, -1, -1},{-1, 1, 1},{-1, 1, -1},{-1, -1, 1},{-1,-1,-1}
};


int cmp1(const pair<int, int> &a, const pair<int, int> &b){
	return a.second > b.second;
}

int cmp2(const pair<int, double> &a, const pair<int, double> &b){
	return a.second - b.second > 1e-6;
}

/*void dfs(int s, int r, int c, int id){	
	if(s < 0 || s >= Zd || r < 0 || r >= Yd || c < 0 || c >= Xd) return;
		if(idx[s][r][c] > 0 || vol[s][r][c] <= 0) return;
		idx[s][r][c] = 1;
		component[id]++;
		for(int h = 0; h < 26; h++)
			dfs(s + dir[h][0], r + dir[h][1], c + dir[h][2], id);
}*/

template <class IT>
/* TODO 1: Rename vvSampleTemplate to vv<your_plugin>Template */
void vvLzfConnectivityTemplate(vtkVVPluginInfo *info,
                      vtkVVProcessDataStruct *pds, 
                      IT *)
{
	IT *inPtr = (IT *)pds->inData;
	IT *outPtr = (IT *)pds->outData;
	int *dim = info->InputVolumeDimensions;
	int inNumComp = info->InputVolumeNumberOfComponents;
	int i, j, k;
	int abort;

	int ***vol, ***idx;
	int Xd, Yd, Zd;
	map<int, int> compVoxelNum;
	map<int, double> compMeanInte;  //每个连通域的平均灰度


  	Xd = (int)dim[0]; //*dim
	Yd = (int)dim[1]; //*(dim+1)
	Zd = (int)dim[2];

	//存储每个点的连通分量编号, 初始0
	idx = new int**[Zd];  
	for(i = 0; i < Zd; i++){
		idx[i] = new int*[Yd];
		for(j = 0; j < Yd; j++){
			idx[i][j] =  new int[Xd]();
		}
	}

	//体数据
	vol = new int**[Zd];  
	for(i = 0; i < Zd; i++){
		vol[i] = new int*[Yd];
		for(j = 0; j < Yd; j++){
			vol[i][j] =  new int[Xd]();
		}
	}

	//连通分量数量
	int cnt = 0;
  
	for ( k = 0; k < Zd; k++ ){                       
		info->UpdateProgress(info,(float)1.0*k/Zd,"Reading into memory..."); 
		abort = atoi(info->GetProperty(info,VVP_ABORT_PROCESSING));
		for ( j = 0; !abort && j < Yd; j++ ){
			for ( i = 0; i < Xd; i++ ){
				vol[k][j][i] = *inPtr;
				inPtr++;
			}
		}
	}
	
	stack<Voxel*> vstack;
	int cz, cy, cx, nz, ny, nx; 

	ofstream outfile;
	outfile.open(".\\log_connectivity.txt", ofstream::out);

	for ( k = 0; k < Zd; k++ ){                       
		info->UpdateProgress(info,(float)1.0*k/Zd,"Computing connected components..."); 
		abort = atoi(info->GetProperty(info,VVP_ABORT_PROCESSING));
		for ( j = 0; !abort && j < Yd; j++ ){
			for ( i = 0; i < Xd; i++ ){
				if(vol[k][j][i] > 0 && idx[k][j][i] == 0){  //没访问过的非背景点
					//dfs(k, j, i, ++cnt);
					++cnt;
					idx[k][j][i] = cnt;
					compVoxelNum[cnt]++;
					compMeanInte[cnt] += vol[k][j][i];
					vstack.push(new Voxel(k, j, i));
					while(!vstack.empty()){
						Voxel* cv = vstack.top();
						vstack.pop();
						cx = cv->x;
						cy = cv->y;
						cz = cv->z;

						for(int h = 0; h < 26; h++){
							nz = cz + dir[h][0];
							ny = cy + dir[h][1];
							nx = cx + dir[h][2];
							
							if(nz < 0 || nz >= Zd || ny < 0 || ny >= Yd || nx < 0 || nx >= Xd){
								continue;
							}
							
							if(vol[nz][ny][nx] > 0 && idx[nz][ny][nx] == 0){
								vstack.push(new Voxel(nz, ny, nx));
								idx[nz][ny][nx] = cnt;
								compVoxelNum[cnt]++;
								compMeanInte[cnt] += vol[nz][ny][nx];
							}
						}

					}
				}
			}
		}
	}

	vector<pair<int, int> > sortComp;  //记录连通分量编号及其包含体素数量
	vector<pair<int, double> > sortCompMeanInte;  //记录连通分量的平均灰度
	//cnt == compVoxelNum.size() == compMeanInte.size();

	for(map<int, int>::iterator it = compVoxelNum.begin(); it != compVoxelNum.end(); it++){
		sortComp.push_back(make_pair(it->first, it->second));
	}

	for(map<int, double>::iterator it = compMeanInte.begin(); it != compMeanInte.end(); it++){
		it->second /= compVoxelNum[it->first];
		sortCompMeanInte.push_back(make_pair(it->first, it->second));
	}

	
	int replacementValue = atoi(info->GetGUIProperty(info, 0, VVP_GUI_VALUE));
	double numRatio = atof(info->GetGUIProperty(info, 1, VVP_GUI_VALUE)); 
	int componentsReserved = atoi(info->GetGUIProperty(info, 2, VVP_GUI_VALUE));


	//按照连通分量里体素数量和平均灰度均值降序排序。
	sort(sortComp.begin(), sortComp.end(), cmp1);
	sort(sortCompMeanInte.begin(), sortCompMeanInte.end(), cmp2);
	
	//为了综合考量体素数量和平均灰度，对两个指标进行归一化、加权平均处理。
	int maxComp = sortComp[0].second;
	double maxCompMeanInte = sortCompMeanInte[0].second;
	double w1 = numRatio, w2 = 1.0 - w1;  //体素数量和平均灰度的权重。
	vector<pair<int, double> > sortComprehensive;
	for(map<int, int>::iterator it = compVoxelNum.begin(); it != compVoxelNum.end(); it++){
		sortComprehensive.push_back(make_pair(it->first, w1 * it->second / maxComp + w2 * compMeanInte[it->first] / maxCompMeanInte));
	}

	sort(sortComprehensive.begin(), sortComprehensive.end(), cmp2);
	sortComp.clear();
	sortCompMeanInte.clear();
	
	//保留前n个
	//vector<int> reservedIdx;
	
	int n  = compVoxelNum.size() < 100 ? compVoxelNum.size() : 100;
	for(int i = 0; i < n; i++){
		//reservedIdx.push_back(sortComp[i].first);
		int T = sortComprehensive[i].first;
		outfile << "Component#" << T << ": " << sortComprehensive[i].second << ". Num: " << compVoxelNum[T] << ", Mean: " << compMeanInte[T] << endl;
	}

	map<int, vector<Voxel*> > componentMap;
	for ( k = 0; k < Zd; k++ ){                       
		info->UpdateProgress(info,(float)1.0*k/Zd,"Reserving the vessels..."); 
		abort = atoi(info->GetProperty(info,VVP_ABORT_PROCESSING));
		for ( j = 0; !abort && j < Yd; j++ ){
			for ( i = 0; i < Xd; i++ ){
				if(vol[k][j][i] > 0){
					int h;
					for(h = 0; h < componentsReserved; h++){
						if(idx[k][j][i] == sortComprehensive[h].first) break;
					}
					//outfile << k << " " << j << " " << i << " #" << idx[k][j][i] << " (" << h << ")" << endl;
					componentMap[idx[k][j][i]].push_back(new Voxel(k, j, i));
					if(h >= componentsReserved) *outPtr = replacementValue;  //不要保留的体素。值待定
				} 
				outPtr++;
			}
		}
	}

	outfile << "***************" << endl;
	for(map<int, vector<Voxel*> >::iterator it = componentMap.begin(); it != componentMap.end(); it++){
		outfile << "#" << it->first << " $" << compVoxelNum[it->first] << endl;
		for(vector<Voxel*>::iterator jt = it->second.begin(); jt != it->second.end(); jt++){
			outfile << (*jt)->z << " " << (*jt)->y << " " << (*jt)->x << endl;
		}
	}
	compVoxelNum.clear();
	compMeanInte.clear();
	sortComprehensive.clear();

	outfile.close();

	info->UpdateProgress(info,(float)1.0,"Processing Complete");

	for(int i = 0; i < Zd; i++){
		for(int j = 0; j < Yd; j++){
			delete[] vol[i][j];
		}
		delete[] vol[i];
	}
	delete[] vol;

	for(int i = 0; i < Zd; i++){
		for(int j = 0; j < Yd; j++){
			delete[] idx[i][j];
		}
		delete[] idx[i];
	}
	delete[] idx;
}

static int ProcessData(void *inf, vtkVVProcessDataStruct *pds)
{
  vtkVVPluginInfo *info = (vtkVVPluginInfo *)inf;

  switch (info->InputVolumeScalarType)
    {
    /* TODO 2: Rename vvSampleTemplate to vv<your_plugin>Template */
    vtkTemplateMacro3(vvLzfConnectivityTemplate, info, pds, 
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
  info->SetGUIProperty(info, 0, VVP_GUI_LABEL, "Replacement Value");
  info->SetGUIProperty(info, 0, VVP_GUI_TYPE, VVP_GUI_SCALE);
  info->SetGUIProperty(info, 0, VVP_GUI_DEFAULT , "0");
  info->SetGUIProperty(info, 0, VVP_GUI_HELP,
	  "What value to set the background voxels to");

  vvPluginSetGUIScaleRange(0); //what is this for????

  info->SetGUIProperty(info, 1, VVP_GUI_LABEL, "ratio of component voxel number(recommend 0.3)");
  info->SetGUIProperty(info, 1, VVP_GUI_TYPE, VVP_GUI_SCALE);
  info->SetGUIProperty(info, 1, VVP_GUI_DEFAULT , "1.0");
  info->SetGUIProperty(info, 1, VVP_GUI_HELP,
	  "the ratio of component voxel number in the connectivity metric");
  info->SetGUIProperty(info, 1, VVP_GUI_HINTS , "0.0 1.0 0.1");

  info->SetGUIProperty(info, 2, VVP_GUI_LABEL, "Number of connected components reserved");
  info->SetGUIProperty(info, 2, VVP_GUI_TYPE, VVP_GUI_SCALE);
  info->SetGUIProperty(info, 2, VVP_GUI_DEFAULT, "30");
  info->SetGUIProperty(info, 2, VVP_GUI_HELP, "How many connected components do you want to reserve");
  info->SetGUIProperty(info, 2, VVP_GUI_HINTS , "1 100 1");
  
  //vvPluginSetGUIScaleRange(2);
  
  /* TODO 6: modify the following code as required. By default the output
  *  image's properties match those of the input depending on what your
  *  filter does it may need to change some of these values
  */
  
  info->OutputVolumeScalarType = info->InputVolumeScalarType;
  info->OutputVolumeNumberOfComponents = info->InputVolumeNumberOfComponents;
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
  void VV_PLUGIN_EXPORT vvLzfConnectivityInit(vtkVVPluginInfo *info)
  {
    /* always check the version */
    vvPluginVersionCheck();
    
    /* setup information that never changes */
    info->ProcessData = ProcessData;
    info->UpdateGUI = UpdateGUI;
    
    /* set the properties this plugin uses */
    /* TODO 4: Rename "Sample" to "<your_plugin>" */
    info->SetProperty(info, VVP_NAME, "LzfConnectivity");
    info->SetProperty(info, VVP_GROUP, "Utility");

    /* TODO 5: update the terse and full documentation for your filter */
    info->SetProperty(info, VVP_TERSE_DOCUMENTATION,
                      "Find all connected components.");
    info->SetProperty(info, VVP_FULL_DOCUMENTATION,
                      "This filter is used to eliminate noise in a volume containing segmented vessels. Noise is those blob- and line-like particle. We only keep the components with larger numbers of voxels.");

    
    /* TODO 9: set these two values to "0" or "1" based on how your plugin
     * handles data all possible combinations of 0 and 1 are valid. */
    info->SetProperty(info, VVP_SUPPORTS_IN_PLACE_PROCESSING, "1");
    info->SetProperty(info, VVP_SUPPORTS_PROCESSING_PIECES,   "1");

    /* TODO 7: set the number of GUI items used by this plugin */
    info->SetProperty(info, VVP_NUMBER_OF_GUI_ITEMS,          "3");
  info->SetProperty(info, VVP_REQUIRES_SERIES_INPUT,        "0");
  info->SetProperty(info, VVP_SUPPORTS_PROCESSING_SERIES_BY_VOLUMES, "0");
  info->SetProperty(info, VVP_PRODUCES_OUTPUT_SERIES, "0");
  info->SetProperty(info, VVP_PRODUCES_PLOTTING_OUTPUT, "0");
  }
}