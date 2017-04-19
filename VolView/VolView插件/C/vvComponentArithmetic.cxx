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

template <class IT>
void vvComponentArithmeticAverage(IT *inPtr, IT *outPtr, int xdim,
                                  int inNumCom)
{
  int i, j;
  double avg;
  
  for ( i = 0; i < xdim; i++ )                               
    { 
    avg = 0;
    for (j = 0; j < inNumCom; ++j)
      {
      avg += *inPtr;
      inPtr++;
      }
    *outPtr = static_cast<IT>(avg/inNumCom); 
    outPtr++;
    }
}

template <class IT>
void vvComponentArithmeticMinimum(IT *inPtr, IT *outPtr, int xdim,
                                  int inNumCom)
{
  int i, j;
  IT val;
  
  for ( i = 0; i < xdim; i++ )                               
    { 
    val = *inPtr;
    inPtr++;
    for (j = 1; j < inNumCom; ++j)
      {
      if (*inPtr < val)
        {
        val = *inPtr;
        }
      inPtr++;
      }
    *outPtr = val; 
    outPtr++;
    }
}

template <class IT>
void vvComponentArithmeticMaximum(IT *inPtr, IT *outPtr, int xdim,
                                  int inNumCom)
{
  int i, j;
  IT val;
  
  for ( i = 0; i < xdim; i++ )                               
    { 
    val = *inPtr;
    inPtr++;
    for (j = 1; j < inNumCom; ++j)
      {
      if (*inPtr > val)
        {
        val = *inPtr;
        }
      inPtr++;
      }
    *outPtr = val; 
    outPtr++;
    }
}

template <class IT>
void vvComponentArithmeticLuminance(IT *inPtr, IT *outPtr, int xdim,
                                    int inNumCom)
{
  int i, j;
  double res;
  double lum[4];
  lum[0] = 0.3;
  lum[1] = 0.59;
  lum[2] = 0.11;
  lum[3] = 0;
  
  for ( i = 0; i < xdim; i++ )                               
    { 
    res = 0;
    for (j = 0; j < inNumCom; ++j)
      {
      res += lum[j]* (*inPtr);
      inPtr++;
      }
    *outPtr = static_cast<IT>(res); 
    outPtr++;
    }
}

void vvCARGBToHSV(float r, float g, float b, float *h, float *s, float *v)
{
  float onethird = 1.0f / 3.0f;
  float onesixth = 1.0f / 6.0f;
  float twothird = 2.0f / 3.0f;

  float cmax, cmin;
  
  cmax = r;
  cmin = r;
  if (g > cmax)
    {
    cmax = g;
    }
  else if (g < cmin)
    {
    cmin = g;
    }
  if (b > cmax)
    {
    cmax = b;
    }
  else if (b < cmin)
    {
    cmin = b;
    }
  *v = cmax;

  if (*v > 0.0)
    {
    *s = (cmax - cmin) / cmax;
    }
  else 
    {
    *s = 0.0;
    }
  if (*s > 0)
    {
    if (r == cmax)
      {
      *h = onesixth * (g - b) / (cmax - cmin);
      }
    else if (g == cmax)
      {
      *h = onethird + onesixth * (b - r) / (cmax - cmin);
      }
    else
      {
      *h = twothird + onesixth * (r - g) / (cmax - cmin);
      }
    if (*h < 0.0)
      {
      *h += 1.0;
      }
    }
  else
    {
    *h = 0.0;
    }
}

template <class IT>
void vvComponentArithmeticHue(IT *inPtr, IT *outPtr, int xdim,
                              int inNumCom)
{
  int i, j;
  float rgb[4];
  float h,s,v;
  rgb[0] = 0;
  rgb[1] = 0;
  rgb[2] = 0;
  
  for ( i = 0; i < xdim; i++ )                               
    { 
    for (j = 0; j < inNumCom; ++j)
      {
      rgb[j] = static_cast<float>(*inPtr);
      inPtr++;
      }
    vvCARGBToHSV(rgb[0]/255.0f,rgb[1]/255.0f,rgb[2]/255.0f,&h, &s, &v);
    *outPtr = static_cast<IT>(h*255.0f); 
    outPtr++;
    }
}

template <class IT>
void vvComponentArithmeticSaturation(IT *inPtr, IT *outPtr, int xdim,
                                     int inNumCom)
{
  int i, j;
  float rgb[4];
  float h,s,v;
  rgb[0] = 0;
  rgb[1] = 0;
  rgb[2] = 0;
  
  for ( i = 0; i < xdim; i++ )                               
    { 
    for (j = 0; j < inNumCom; ++j)
      {
      rgb[j] = static_cast<float>(*inPtr);
      inPtr++;
      }
    vvCARGBToHSV(rgb[0]/255.0f,rgb[1]/255.0f,rgb[2]/255.0f,&h, &s, &v);
    *outPtr = static_cast<IT>(s*255.0f); 
    outPtr++;
    }
}

//-----------------------------------------------------------
template <class IT>
void vvComponentArithmeticTemplate(vtkVVPluginInfo *info,
                                   vtkVVProcessDataStruct *pds, 
                                   IT *)
{
  IT *inPtr = (IT *)pds->inData;
  IT *outPtr = (IT *)pds->outData;
  int *dim = info->InputVolumeDimensions;
  const char *operC = info->GetGUIProperty(info, 0, VVP_GUI_VALUE);
  const char *resultC = info->GetGUIProperty(info, 1, VVP_GUI_VALUE);
  const char *resultingUnits = 0;
  
  int result = 2;
  if (!strcmp(resultC,"Append Component"))
    {
    result = 0;
    }
  if (!strcmp(resultC,"Replace All Components"))
    {
    result = 1;
    }
  
  int abort = 0;
  int inNumCom = info->InputVolumeNumberOfComponents;

  if (inNumCom < 2)
    {
    info->SetProperty( info, VVP_ERROR, 
        "These operations require multicomponent data." ); 
    return;
    }

  int oper = 0;
  resultingUnits = "Average";
  if (!strcmp(operC,"Luminance"))
    {
    oper = 1;
    resultingUnits = "Luminance";
    }
  if (!strcmp(operC,"Hue"))
    {
    oper = 2;
    resultingUnits = "Hue";
    }
  if (!strcmp(operC,"Saturation"))
    {
    oper = 3;
    resultingUnits = "Saturation";
    }
  if (!strcmp(operC,"Maximum"))
    {
    oper = 4;
    resultingUnits = "Maximum";
    }
  if (!strcmp(operC,"Minimum"))
    {
    oper = 5;
    resultingUnits = "Minimum";
    }

  // allocate a result buffer 
  IT *outBuff = new IT [dim[0]];
  
  int i, j, k, l;
  for ( k = 0; k < dim[2]; k++ )                                      
    {                                                                 
    info->UpdateProgress(info,(float)1.0*k/dim[2],"Processing..."); 
    abort = atoi(info->GetProperty(info,VVP_ABORT_PROCESSING));
    for ( j = 0; !abort && j < dim[1]; j++ )                          
      {          
      switch (oper)
        {
        case 0:
          vvComponentArithmeticAverage(inPtr,outBuff,dim[0],inNumCom);
          break;
        case 1:
          vvComponentArithmeticLuminance(inPtr,outBuff,dim[0],inNumCom);
          break;
        case 2:
          vvComponentArithmeticHue(inPtr,outBuff,dim[0],inNumCom);
          break;
        case 3:
          vvComponentArithmeticSaturation(inPtr,outBuff,dim[0],inNumCom);
          break;
        case 4:
          vvComponentArithmeticMaximum(inPtr,outBuff,dim[0],inNumCom);
          break;
        case 5:
          vvComponentArithmeticMinimum(inPtr,outBuff,dim[0],inNumCom);
          break;
        }
      
      // copy the result into the output
      switch (result)
        {
        case 0: // append
          for ( i = 0; i < dim[0]; i++ )
            {
            for (l = 0; l < inNumCom; ++l)
              {
              *outPtr = *inPtr;
              outPtr++;
              inPtr++;
              }
            *outPtr = outBuff[i];
            outPtr++;
            }
          break;
        case 1: // replace all
          memcpy(outPtr,outBuff,dim[0]);
          outPtr += dim[0];
          inPtr += dim[0]*inNumCom;
          break;
        case 2: // replace last
          for ( i = 0; i < dim[0]; i++ )
            {
            for (l = 0; l < inNumCom-1; ++l)
              {
              *outPtr = *inPtr;
              outPtr++;
              inPtr++;
              }
            *outPtr = outBuff[i];
            outPtr++;
            inPtr++;
            }
          break;
        }
      }                                                      
    }               
  
  // set the resulting component's units
  int resultCompNum = 0;
  switch (result)
    {
    case 0:
      resultCompNum = inNumCom;
      break;
    case 1:
      resultCompNum = 0;
      break;
    case 2:
      resultCompNum = inNumCom - 1;
      break;      
    }
  switch (resultCompNum)
    {
    case 0:
      info->SetProperty(info, VVP_RESULTING_COMPONENT_1_UNITS,resultingUnits);
      break;
    case 1:
      info->SetProperty(info, VVP_RESULTING_COMPONENT_2_UNITS,resultingUnits);
      break;
    case 2:
      info->SetProperty(info, VVP_RESULTING_COMPONENT_3_UNITS,resultingUnits);
      break;
    case 3:
      info->SetProperty(info, VVP_RESULTING_COMPONENT_4_UNITS,resultingUnits);
      break;
    }
  
  info->UpdateProgress(info,(float)1.0,"Processing Complete");
  delete [] outBuff;
}

static int ProcessData(void *inf, vtkVVProcessDataStruct *pds)
{
  vtkVVPluginInfo *info = (vtkVVPluginInfo *)inf;

  switch (info->InputVolumeScalarType)
    {
    // invoke the appropriate templated function
    vtkTemplateMacro3(vvComponentArithmeticTemplate, info, pds, 
                      static_cast<VTK_TT *>(0));
    }
  return 0;
}


static int UpdateGUI(void *inf)
{
  vtkVVPluginInfo *info = (vtkVVPluginInfo *)inf;

  info->SetGUIProperty(info, 0, VVP_GUI_LABEL, "Operation");
  info->SetGUIProperty(info, 0, VVP_GUI_TYPE, VVP_GUI_CHOICE);
  info->SetGUIProperty(info, 0, VVP_GUI_DEFAULT , "Average");
  info->SetGUIProperty(info, 0, VVP_GUI_HELP,
                       "Operation to perform");
  info->SetGUIProperty(info, 0, VVP_GUI_HINTS, 
                       "6\nAverage\nLuminance\nHue\nSaturation\nMaximum\nMinimum");

  info->SetGUIProperty(info, 1, VVP_GUI_LABEL, "Result");
  info->SetGUIProperty(info, 1, VVP_GUI_TYPE, VVP_GUI_CHOICE);
  info->SetGUIProperty(info, 1, VVP_GUI_DEFAULT , "Append Component");
  info->SetGUIProperty(info, 1, VVP_GUI_HELP,
                       "Where to place the result");
  info->SetGUIProperty(info, 1, VVP_GUI_HINTS, 
                       "3\nAppend Component\nReplace All Components\nRelace Last Component");

  info->OutputVolumeScalarType = info->InputVolumeScalarType;
  int i;
  for (i = 0; i < 3; i++)
    {
    info->OutputVolumeDimensions[i] = info->InputVolumeDimensions[i];
    info->OutputVolumeSpacing[i] = info->InputVolumeSpacing[i];
    info->OutputVolumeOrigin[i] = info->InputVolumeOrigin[i];
    }

  info->OutputVolumeNumberOfComponents = 
    info->InputVolumeNumberOfComponents;

  // the components depend on the options selected
  const char *result = info->GetGUIProperty(info, 1, VVP_GUI_VALUE);
  if (result)
    {
    if (!strcmp(result,"Append Component"))
      {
      info->OutputVolumeNumberOfComponents = 
        info->InputVolumeNumberOfComponents + 1;
      // if the output num of components exceeds 4 then reduce it to 4 and
      // change the setting to replace last
      if (info->OutputVolumeNumberOfComponents > 4)
        {
        info->OutputVolumeNumberOfComponents = 4;
        info->SetGUIProperty(info, 1, VVP_GUI_VALUE, 
                             "Replace Last Component");
        }
      }
    if (!strcmp(result,"Replace All Components"))
      {
      info->OutputVolumeNumberOfComponents = 1;
      }
    }
  
  return 1;
}

extern "C" 
{
  void VV_PLUGIN_EXPORT vvComponentArithmeticInit(vtkVVPluginInfo *info)
{
  /* always check the version */
  vvPluginVersionCheck();

  /* setup information that never changes */
  info->ProcessData = ProcessData;
  info->UpdateGUI = UpdateGUI;

  /* set the properties this plugin uses */
  info->SetProperty(info, VVP_NAME, "Component Arithmetic");
  info->SetProperty(info, VVP_GROUP, "Utility");
  info->SetProperty(info, VVP_TERSE_DOCUMENTATION,
                    "Perform Arithmetic Operations on Components");
  info->SetProperty(info, VVP_FULL_DOCUMENTATION,
                    "This filter computes a new volume by performing an arithmatic operation on the components of the input volume. While all operations will work with any input volume, some make more sense than others. For example, computing Hue or Saturation really only makes sense with multicomponent color data and then typically with unsigned char data only. The results of the calculation can replace all the components of the original volume (e.g. create a new volume without only one component), replace just the last component of the input volume (typically good for color data), or append a component to the input volume (e.g. making a one component greyscale volume into a two component column). If the input volume already has four components then you cannot append another component onto it. Instead it will replace the last component.");

  info->SetProperty(info, VVP_SUPPORTS_IN_PLACE_PROCESSING, "0");
  info->SetProperty(info, VVP_SUPPORTS_PROCESSING_PIECES,   "1");
  info->SetProperty(info, VVP_NUMBER_OF_GUI_ITEMS,          "2");
  info->SetProperty(info, VVP_REQUIRES_SERIES_INPUT,        "0");
  info->SetProperty(info, VVP_SUPPORTS_PROCESSING_SERIES_BY_VOLUMES, "0");
  info->SetProperty(info, VVP_PRODUCES_OUTPUT_SERIES, "0");
  info->SetProperty(info, VVP_PRODUCES_PLOTTING_OUTPUT, "0");
}
  
}



