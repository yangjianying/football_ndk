#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vulkan/vulkan.h>

#include "FootballConfig.h"
#include "utils/football_debugger.h"

#include "utils/ANativeWindowUtils.h"

#include "ndk_extend/NativeHooApi_Loader.h"

#include "computedemo1_1.h"
#include "computedemo1_1_HistogramCompute.h"

// Android log function wrappers
static const char* kTAG = "computedemo1_1";
#include "utils/android_logcat_.h"

#undef __CLASS__
#define __CLASS__ "HistogramCompute"


namespace computedemo1_1 {

static const char *shader_1 = " \n"
"#version 450 \n"
"	layout (local_size_x = 16, local_size_y = 16) in; \n"
"	layout (binding = 0, rgba8) uniform readonly image2D inputImage; \n"
"	layout (binding = 1, rgba8) uniform image2D resultImage; \n"
"	float conv(in float[9] kernel, in float[9] data, in float denom, in float offset)  \n"
"	{ \n"
"	   float res = 0.0; \n"
"	   for (int i=0; i<9; ++i)  \n"
"	   { \n"
"		  res += kernel[i] * data[i]; \n"
"	   } \n"
"	   return clamp(res/denom + offset, 0.0, 1.0); \n"
"	} \n"
"	struct ImageData  \n"
"	{ \n"
"		float avg[9]; \n"
"	} imageData;	 \n"
"	void main() \n"
"	{	 \n"
"		// Fetch neighbouring texels \n"
"		int n = -1; \n"
"		for (int i=-1; i<2; ++i)  \n"
"		{	 \n"
"			for(int j=-1; j<2; ++j)  \n"
"			{	  \n"
"				n++;	 \n"
"				vec3 rgb = imageLoad(inputImage, ivec2(gl_GlobalInvocationID.x + i, gl_GlobalInvocationID.y + j)).rgb; \n"
"				imageData.avg[n] = (rgb.r + rgb.g + rgb.b) / 3.0; \n"
"			} \n"
"		} \n"
"		float[9] kernel; \n"
"		kernel[0] = -1.0; kernel[1] =  0.0; kernel[2] =  0.0; \n"
"		kernel[3] = 0.0; kernel[4] = -1.0; kernel[5] =	0.0; \n"
"		kernel[6] = 0.0; kernel[7] =  0.0; kernel[8] = 2.0; \n"
"		vec4 res = vec4(vec3(conv(kernel, imageData.avg, 1.0, 0.50)), 1.0); \n"
"		imageStore(resultImage, ivec2(gl_GlobalInvocationID.xy), res); \n"
"	} \n"
"	";

static const char *shader_cal_histogram_mean = " \n"
	"\n"
	"#version 450 \n"
	"precision highp float; \n"		//"precision mediump float; \n"
	"precision highp int;\n"
	"layout (local_size_x = 16, local_size_y = 16) in; \n"
	"layout (binding = 0, rgba8) uniform readonly image2D scaledImage; \n"
	"layout (binding = 1, rgba8) uniform image2D resultImage; \n"
	"// Binding 3: Indirect draw stats \n"
	"layout (binding = 3) buffer UBOOut \n"
	"{ \n"
	"	uint drawCount; \n"
	"	uint lodCount[" __stringify(IndirectStats__c_HIST_BIN_SIZE) "]; \n"
	"	uint luminance_sumed; \n"
	"} uboOut; \n"
	"							\n"
	"const vec3 W1 = vec3(0.2125, 0.7154, 0.0721); \n"
	"const vec3 W2 = vec3(0.299, 0.587, 0.114); \n"
	"							\n"
	"vec3 getHSV_(vec3 rgb) { \n"
	"	int bR = int(rgb.r * 255.0);		\n"
	"	int bG = int(rgb.g * 255.0); 		\n"
	"	int bB = int(rgb.b * 255.0);		\n"
	"	float fR =rgb.r; 			\n"
	"	float fG = rgb.g; 			\n"
	"	float fB = rgb.b; 			\n"
	"	float fH, fS, fV; 			\n"
	"	float fDelta; 				\n"
	"	float fMin, fMax;			\n"
	"	int iMax;  					\n"
	"	if (bB < bG) { \n"
	"		if (bB < bR) { \n"
	"			fMin = fB; \n"
	"			if (bR > bG) { iMax = bR; fMax = fR;} \n"
	"			else {iMax = bG;fMax = fG;} \n"
	"		} \n"
	"		else { fMin = fR;fMax = fG;iMax = bG; } \n"
	"	} \n"
	"	else { \n"
	"		if (bG < bR) { \n"
	"			fMin = fG; \n"
	"			if (bB > bR) { fMax = fB; iMax = bB; } \n"
	"			else { fMax = fR; iMax = bR; } \n"
	"		} \n"
	"		else { fMin = fR; fMax = fB; iMax = bB; } \n"
	"	} \n"
	"	fDelta = fMax - fMin; \n"
	"	fV = fMax;    // Value (Brightness). \n"
	"	if (iMax != 0) {   // Make sure its not pure black. \n"
	"		fS = fDelta / fMax;  // Saturation. \n"
	"		float ANGLE_TO_UNIT = 1.0f / (6.0f * fDelta); // Make the Hues between 0.0 to 1.0 instead of 6.0 \n"
	"		if (iMax == bR) {  // between yellow and magenta. \n"
	"			fH = (fG - fB) * ANGLE_TO_UNIT; \n"
	"		} \n"
	"		else if (iMax == bG) {  // between cyan and yellow. \n"
	"			fH = (2.0f / 6.0f) + (fB - fR) * ANGLE_TO_UNIT; \n"
	"		} \n"
	"		else {    // between magenta and cyan. \n"
	"			fH = (4.0f / 6.0f) + (fR - fG) * ANGLE_TO_UNIT; \n"
	"		} \n"
	"		// Wrap outlier Hues around the circle. \n"
	"		if (fH < 0.0f) \n"
	"			fH += 1.0f; \n"
	"		if (fH >= 1.0f) \n"
	"			fH -= 1.0f; \n"
	"	} \n"
	"	else { \n"
	"		// color is pure Black. \n"
	"		fS = 0; \n"
	"		fH = 0; // undefined hue \n"
	"	} \n"
	"	fH = clamp(fH, 0.0, 1.0);			\n"
	"	fS = clamp(fS, 0.0, 1.0);			\n"
	"	fV = clamp(fV, 0.0, 1.0);			\n"
	"	return vec3(fH, fS, fV); \n"
	"} \n"
	"							\n"
	"void main() \n"
	"{							\n"
	"	uint idx = gl_GlobalInvocationID.x + gl_GlobalInvocationID.y * gl_NumWorkGroups.x * gl_WorkGroupSize.x; \n"
// how to clear on first invocation !!!
	"	atomicAdd(uboOut.drawCount, 1);\n"

	"	vec3 rgb = imageLoad(scaledImage, ivec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y)).rgb; \n"

	//"	float luminance = dot(rgb.rgb, W1); \n"
	//"	float luminance = dot(rgb.rgb, W2); \n"
	"	float luminance = getHSV_(rgb.rgb).z; \n"

	"	rgb = vec3(luminance, luminance, luminance);  \n"

	"	float luminance_range = luminance * (" __stringify(IndirectStats__c_HIST_BIN_SIZE) ".0 - 1.0); \n"

	//"	int gray_index = int(luminance_range); \n"
	"	int gray_index = int(luminance_range + 0.5); \n"

	"	if (gray_index < 0) { gray_index = 0; } \n"
	"	else if(gray_index >= " __stringify(IndirectStats__c_HIST_BIN_SIZE) ") {\n"
	"		gray_index = (" __stringify(IndirectStats__c_HIST_BIN_SIZE) " - 1); \n"
	"	} \n"

	"	atomicAdd(uboOut.luminance_sumed, gray_index);\n"
	"	atomicAdd(uboOut.lodCount[gray_index], 1);\n"

#if 0
	"	vec4 res = vec4(rgb, 1.0); \n"
	"	imageStore(resultImage, ivec2(gl_GlobalInvocationID.xy), res); \n"
#endif
	"}\n"
	;
static const char *shader_gen_acc_hist = " \n"
	"\n"
	"#version 450 \n"
	"precision highp float; \n"
	"precision highp int;\n"
	"layout (local_size_x = 1, local_size_y = 1) in; \n"
	"// Binding 3: Indirect draw stats \n"
	"layout (binding = 3) buffer UBOOut \n"
	"{ \n"
	"	uint drawCount; \n"
	"	uint lodCount[" __stringify(IndirectStats__c_HIST_BIN_SIZE) "]; \n"
	"	uint luminance_sumed; \n"
	"								\n"
	"	uint accHist[" __stringify(IndirectStats__c_HIST_BIN_SIZE) "]; \n"
	"	float accHistNormal[" __stringify(IndirectStats__c_HIST_BIN_SIZE) "]; \n"
	"	uint accCount; \n"
	"	uint pointIndexFlags[8];\n"
	"	uint pointIndex[8]; \n"
	"								\n"
	"	uint bhe_lut[" __stringify(IndirectStats__c_HIST_BIN_SIZE) "]; \n"
	"	float bhe_lut0[" __stringify(IndirectStats__c_HIST_BIN_SIZE) "]; \n"
	"	float bhe_lut1[" __stringify(IndirectStats__c_HIST_BIN_SIZE) "]; \n"
	"								\n"
	"	uint lumi_counter; \n"
	"	uint luminance_mean;\n"
	"	uint luminance_std;\n"
	"	float lumi_mean; \n"
	"	float lumi_std; \n"
	"	uint type; \n" // 0, 1, 2, 3
	"	float y1; \n"
	"	float y2; \n"
	"	uint agc_lut[" __stringify(IndirectStats__c_HIST_BIN_SIZE) "]; \n"
	"	uint agc_counter; \n"
	"								\n"
	"	uint merged_lut[" __stringify(IndirectStats__c_HIST_BIN_SIZE) "]; \n"
	"	float bhe_factor0; \n"
	"	float bhe_factor1; \n"
	"	float bhe_tuning0; \n"
	"	float bhe_tuning1; \n"

	"} uboOut; \n"
	"\n"
	"void main() \n"
	"{\n"
	"	uint idx = gl_GlobalInvocationID.x + gl_GlobalInvocationID.y * gl_NumWorkGroups.x * gl_WorkGroupSize.x; \n"
	"	uint acc = uboOut.lodCount[0]; \n"
	"	uint level = 0; \n"
	"	for (level=1; level < " __stringify(IndirectStats__c_HIST_BIN_SIZE) " && level <= gl_GlobalInvocationID.x; level ++) { \n"
	"		acc += uboOut.lodCount[level]; \n"
	"	} \n"
	"	uboOut.accHist[gl_GlobalInvocationID.x] = acc; \n"
	"	float acc_f = float(acc); \n"
	"	float normal_acc = acc_f/float(uboOut.drawCount); \n"
	"	uboOut.accHistNormal[gl_GlobalInvocationID.x] = normal_acc; \n"
	"	\n"
#if 1 // if this works ...
	"	atomicAdd(uboOut.accCount, 1); \n"
	"	if (uboOut.accCount == " __stringify(IndirectStats__c_HIST_BIN_SIZE) ") { \n"
	"		for(level=0;level<" __stringify(IndirectStats__c_HIST_BIN_SIZE) ";level++) { \n"
	"			if (uboOut.pointIndexFlags[0] == 0) { \n"
	"				if (uboOut.accHistNormal[level] >= uboOut.bhe_factor0) { atomicAdd(uboOut.pointIndexFlags[0], 1); uboOut.pointIndex[0] = level; }\n"
	"			} \n"
	"			if (uboOut.pointIndexFlags[1] == 0) { \n"
	"				if (uboOut.accHistNormal[level] >= 0.5) { atomicAdd(uboOut.pointIndexFlags[1], 1); uboOut.pointIndex[1] = level; }\n"
	"			} \n"
	"			if (uboOut.pointIndexFlags[2] == 0) { \n"
	"				if (uboOut.accHistNormal[level] >= uboOut.bhe_factor1) { atomicAdd(uboOut.pointIndexFlags[2], 1); uboOut.pointIndex[2] = level; }\n"
	"			} \n"
	"		} \n"
	"\n"
#if 1
	"		uint idx0_new = uint(float(uboOut.pointIndex[0]) * uboOut.bhe_tuning0); \n"
	"		uint idx2_new = uint(float(uboOut.pointIndex[2]) * uboOut.bhe_tuning1); \n"
	"		if (idx0_new > uboOut.pointIndex[1]) { idx0_new = uboOut.pointIndex[1]; } \n"
	"		if (idx2_new < uboOut.pointIndex[1]) { idx2_new = uboOut.pointIndex[1]; } \n"
	"		if (idx2_new >= " __stringify(IndirectStats__c_HIST_BIN_SIZE) ") { idx2_new = " __stringify(IndirectStats__c_HIST_BIN_SIZE) " - 1; } \n"
	"		uboOut.pointIndex[0] = idx0_new; \n"
	"		uboOut.pointIndex[2] = idx2_new; \n"
	"		\n"
#endif
#if 1
	"		for(level=0;level<" __stringify(IndirectStats__c_HIST_BIN_SIZE) ";level++) { \n"
	"			if (level >= uboOut.pointIndex[0] && level <= uboOut.pointIndex[1]) { \n"
	"				if (uboOut.pointIndex[1] != uboOut.pointIndex[0]) {"
	"					uboOut.bhe_lut0[level] = \\\n"
	"						float(uboOut.accHist[level] - uboOut.accHist[uboOut.pointIndex[0]]) \n"
	"							/ float(uboOut.accHist[uboOut.pointIndex[1]] - uboOut.accHist[uboOut.pointIndex[0]]); \n"
	"				} else {\n"
	"					uboOut.bhe_lut0[level] = 0.001; "
	"				} \n"
	"			} \n"
	"								\n"
	"			else if (level >= uboOut.pointIndex[1] && level <= uboOut.pointIndex[2]) { \n"
	"				if (uboOut.pointIndex[2] != uboOut.pointIndex[1]) {"
	"					uboOut.bhe_lut1[level] = \\\n"
	"						float(uboOut.accHist[level] - uboOut.accHist[uboOut.pointIndex[1]]) \n"
	"							/ float(uboOut.accHist[uboOut.pointIndex[2]] - uboOut.accHist[uboOut.pointIndex[1]]); \n"
	"				} else {\n"
	"					uboOut.bhe_lut1[level] = 0.001; "
	"				} \n"
	"			} \n"
	"		} \n"
#endif
	"\n"
	"	} \n"
#endif

	"}\n"
	;

static const char *shader_gen_bhe_lut = " \n"
	"\n"
	"#version 450 \n"
	"precision highp float; \n"
	"precision highp int;\n"
	"layout (local_size_x = 1, local_size_y = 1) in; \n"
	"// Binding 3: Indirect draw stats \n"
	"layout (binding = 3) buffer UBOOut \n"
	"{ \n"
	"	uint drawCount; \n"
	"	uint lodCount[" __stringify(IndirectStats__c_HIST_BIN_SIZE) "]; \n"
	"	uint luminance_sumed; \n"
	"								\n"
	"	uint accHist[" __stringify(IndirectStats__c_HIST_BIN_SIZE) "]; \n"
	"	float accHistNormal[" __stringify(IndirectStats__c_HIST_BIN_SIZE) "]; \n"
	"	uint accCount; \n"
	"	uint pointIndexFlags[8];\n"
	"	uint pointIndex[8]; \n"
	"								\n"
	"	uint bhe_lut[" __stringify(IndirectStats__c_HIST_BIN_SIZE) "]; \n"
	"	float bhe_lut0[" __stringify(IndirectStats__c_HIST_BIN_SIZE) "]; \n"
	"	float bhe_lut1[" __stringify(IndirectStats__c_HIST_BIN_SIZE) "]; \n"
	"								\n"
	"	uint lumi_counter; \n"
	"	uint luminance_mean;\n"
	"	uint luminance_std;\n"
	"	float lumi_mean; \n"
	"	float lumi_std; \n"
	"	uint type; \n" // 0, 1, 2, 3
	"	float y1; \n"
	"	float y2; \n"
	"	uint agc_lut[" __stringify(IndirectStats__c_HIST_BIN_SIZE) "]; \n"
	"	uint agc_counter; \n"
	"								\n"
	"	uint merged_lut[" __stringify(IndirectStats__c_HIST_BIN_SIZE) "]; \n"
	"	float bhe_factor0; \n"
	"	float bhe_factor1; \n"
	"} uboOut; \n"
	"\n"
	"void main() \n"
	"{\n"
	"									\n"
	"									\n"
#if 1
	"	if (gl_GlobalInvocationID.x <= uboOut.pointIndex[0]) { atomicAdd(uboOut.bhe_lut[gl_GlobalInvocationID.x], gl_GlobalInvocationID.x); } \n"
	"									\n"
	"	else if (gl_GlobalInvocationID.x >= uboOut.pointIndex[0] && gl_GlobalInvocationID.x <= uboOut.pointIndex[1]) { \n"
	"		uboOut.bhe_lut[gl_GlobalInvocationID.x] = uint(uboOut.bhe_lut0[gl_GlobalInvocationID.x] * float(gl_GlobalInvocationID.x - uboOut.pointIndex[0])) "
	"					+ uboOut.pointIndex[0]; \n"
	"	} \n"
	"									\n"
	"	else if (gl_GlobalInvocationID.x >= uboOut.pointIndex[1] && gl_GlobalInvocationID.x <= uboOut.pointIndex[2]) { \n"
	"		uboOut.bhe_lut[gl_GlobalInvocationID.x] = uint(uboOut.bhe_lut1[gl_GlobalInvocationID.x] * float(gl_GlobalInvocationID.x - uboOut.pointIndex[1]))"
	"					+ uboOut.pointIndex[1]; \n"
	"	} \n"
	"									\n"
	"	else if (gl_GlobalInvocationID.x >= uboOut.pointIndex[2]) { \n"
	"		uboOut.bhe_lut[gl_GlobalInvocationID.x] = gl_GlobalInvocationID.x; \n"
	"	} \n"
	"	else {} \n"
#endif
	"}\n"
	;


static const char *shader_cal_std = " \n"
	"\n"
	"#version 450 \n"
	"precision highp float; \n"
	"precision highp int;\n"
	"layout (local_size_x = 16, local_size_y = 16) in; \n"
	"layout (binding = 0, rgba8) uniform readonly image2D scaledImage; \n"
	"layout (binding = 1, rgba8) uniform image2D resultImage; \n"
	"layout (binding = 5, rgba8) uniform readonly image2D inputAlignedImage; \n"
	"									\n"
	"// Binding 3: Indirect draw stats \n"
	"layout (binding = 3) buffer UBOOut \n"
	"{ \n"
	"	uint drawCount; \n"
	"	uint lodCount[" __stringify(IndirectStats__c_HIST_BIN_SIZE) "]; \n"
	"	uint luminance_sumed; \n"
	"								\n"
	"	uint accHist[" __stringify(IndirectStats__c_HIST_BIN_SIZE) "]; \n"
	"	float accHistNormal[" __stringify(IndirectStats__c_HIST_BIN_SIZE) "]; \n"
	"	uint accCount; \n"
	"	uint pointIndexFlags[8];\n"
	"	uint pointIndex[8]; \n"
	"								\n"
	"	uint bhe_lut[" __stringify(IndirectStats__c_HIST_BIN_SIZE) "]; \n"
	"	float bhe_lut0[" __stringify(IndirectStats__c_HIST_BIN_SIZE) "]; \n"
	"	float bhe_lut1[" __stringify(IndirectStats__c_HIST_BIN_SIZE) "]; \n"
	"								\n"
	"	uint lumi_counter; \n"
	"	uint luminance_mean; \n"
	"	uint luminance_std; \n"
	"	float lumi_mean; \n"
	"	float lumi_std; \n"
	"	uint type; \n" // 0, 1, 2, 3
	"	float y1; \n"
	"	float y2; \n"
	"	uint agc_lut[" __stringify(IndirectStats__c_HIST_BIN_SIZE) "]; \n"
	"	uint agc_counter; \n"
	"								\n"
	"	uint merged_lut[" __stringify(IndirectStats__c_HIST_BIN_SIZE) "]; \n"
	"	float bhe_factor0; \n"
	"	float bhe_factor1; \n"
	"} uboOut; \n"
	"\n"
#if 0
	"// Binding 4:  \n"					// not used !!!
	"layout (binding = 4) buffer STDTmp \n"
	"{ \n"
	"	float pixel_[161024]; \n"		// this size should be the same size as dst size !!! 272 x 592
	"} stdTmp; \n"
	"\n"
#endif
	"const vec3 W1 = vec3(0.2125, 0.7154, 0.0721); \n"
	"								\n"
	"const vec3 W2 = vec3(0.299, 0.587, 0.114); \n"
	"								\n"
	"const float scale_factor_ = 250.0; 	        \n"
	"									\n"
	"vec3 getHSV_(vec3 rgb) { \n"
	"	int bR = int(rgb.r * 255.0);		\n"
	"	int bG = int(rgb.g * 255.0); 		\n"
	"	int bB = int(rgb.b * 255.0);		\n"
	"	float fR =rgb.r; 			\n"
	"	float fG = rgb.g; 			\n"
	"	float fB = rgb.b; 			\n"
	"	float fH, fS, fV; 			\n"
	"	float fDelta; 				\n"
	"	float fMin, fMax;			\n"
	"	int iMax;  					\n"
	"	if (bB < bG) { \n"
	"		if (bB < bR) { \n"
	"			fMin = fB; \n"
	"			if (bR > bG) { iMax = bR; fMax = fR;} \n"
	"			else {iMax = bG;fMax = fG;} \n"
	"		} \n"
	"		else { fMin = fR;fMax = fG;iMax = bG; } \n"
	"	} \n"
	"	else { \n"
	"		if (bG < bR) { \n"
	"			fMin = fG; \n"
	"			if (bB > bR) { fMax = fB; iMax = bB; } \n"
	"			else { fMax = fR; iMax = bR; } \n"
	"		} \n"
	"		else { fMin = fR; fMax = fB; iMax = bB; } \n"
	"	} \n"
	"	fDelta = fMax - fMin; \n"
	"	fV = fMax;    // Value (Brightness). \n"
	"	if (iMax != 0) {   // Make sure its not pure black. \n"
	"		fS = fDelta / fMax;  // Saturation. \n"
	"		float ANGLE_TO_UNIT = 1.0f / (6.0f * fDelta); // Make the Hues between 0.0 to 1.0 instead of 6.0 \n"
	"		if (iMax == bR) {  // between yellow and magenta. \n"
	"			fH = (fG - fB) * ANGLE_TO_UNIT; \n"
	"		} \n"
	"		else if (iMax == bG) {  // between cyan and yellow. \n"
	"			fH = (2.0f / 6.0f) + (fB - fR) * ANGLE_TO_UNIT; \n"
	"		} \n"
	"		else {    // between magenta and cyan. \n"
	"			fH = (4.0f / 6.0f) + (fR - fG) * ANGLE_TO_UNIT; \n"
	"		} \n"
	"		// Wrap outlier Hues around the circle. \n"
	"		if (fH < 0.0f) \n"
	"			fH += 1.0f; \n"
	"		if (fH >= 1.0f) \n"
	"			fH -= 1.0f; \n"
	"	} \n"
	"	else { \n"
	"		// color is pure Black. \n"
	"		fS = 0; \n"
	"		fH = 0; // undefined hue \n"
	"	} \n"
	"	fH = clamp(fH, 0.0, 1.0);			\n"
	"	fS = clamp(fS, 0.0, 1.0);			\n"
	"	fV = clamp(fV, 0.0, 1.0);			\n"
	"	return vec3(fH, fS, fV); \n"
	"} \n"

	"void main() \n"
	"{\n"
	"	uint idx = gl_GlobalInvocationID.x + gl_GlobalInvocationID.y * gl_NumWorkGroups.x * gl_WorkGroupSize.x; \n"
	"	vec3 rgb = imageLoad(scaledImage, ivec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y)).rgb; \n"

	//"	float luminance = dot(rgb.rgb, W1); \n"
	//"	float luminance = dot(rgb.rgb, W2); \n"
	"	float luminance = getHSV_(rgb.rgb).z; \n"

	//"	rgb = vec3(luminance, luminance, luminance);  \n"

	"	float luminance_range = luminance * (" __stringify(IndirectStats__c_HIST_BIN_SIZE) ".0 - 1.0); \n"
	//" int gray_index = int(luminance_range); \n"
	"	int gray_index = int(luminance_range + 0.5); \n"
	"									\n"
	"	if (gray_index < 0) { gray_index = 0; } \n"
	"	else if(gray_index >= " __stringify(IndirectStats__c_HIST_BIN_SIZE) ") {\n"
	"		gray_index = (" __stringify(IndirectStats__c_HIST_BIN_SIZE) " - 1); \n"
	"	} \n"
	"	uint mean_ = uboOut.luminance_sumed/uboOut.drawCount; \n"
	"	int delta_ =  int(mean_) - gray_index; \n"
	"	float delta_f = float(abs(delta_) * abs(delta_)); \n"
	"	delta_f = float(delta_f * scale_factor_) /  float(uboOut.drawCount - 1); \n"
	"	atomicAdd(uboOut.luminance_std, uint(delta_f));\n"
	"	\n"
	"	\n"
	"	atomicAdd(uboOut.lumi_counter, 1); \n"
	"	if (uboOut.lumi_counter == (gl_NumWorkGroups.x *gl_NumWorkGroups.y) * (gl_WorkGroupSize.x * gl_WorkGroupSize.y)) { \n"
	"		uboOut.luminance_mean = uboOut.luminance_sumed/uboOut.drawCount; \n"
	"		uboOut.luminance_std = uint(sqrt(uboOut.luminance_std/scale_factor_)); \n"
	"		uboOut.lumi_mean = float(uboOut.luminance_mean)/float(" __stringify(IndirectStats__c_HIST_BIN_SIZE) "); \n"
	"		uboOut.lumi_std = float(uboOut.luminance_std)/float(" __stringify(IndirectStats__c_HIST_BIN_SIZE) "); \n"
	"		uboOut.y1 = -log2(uboOut.lumi_std); \n"
	"		uboOut.y2 = exp(1.0 - (uboOut.lumi_std + uboOut.lumi_mean)/2.0); \n"
	"		if ((4.0 * uboOut.lumi_std) < (1.0/3.0)) { \n"
	"			if (uboOut.lumi_mean < 0.5) { uboOut.type = 0; } \n"
	"			else {uboOut.type = 1;} \n"
	"		} else { \n "
	"			if (uboOut.lumi_mean < 0.5) { uboOut.type = 2; } \n"
	"			else {uboOut.type = 3;} \n"
	"		} \n"
	"	} \n"
	"}\n"
;

static const char *shader_gen_agc_lut = " \n"
	"\n"
	"#version 450 \n"
	"precision highp float; \n"
	"precision highp int;\n"
	"layout (local_size_x = 1, local_size_y = 1) in; \n"
	"// Binding 3: Indirect draw stats \n"
	"layout (binding = 3) buffer UBOOut \n"
	"{ \n"
	"	uint drawCount; \n"
	"	uint lodCount[" __stringify(IndirectStats__c_HIST_BIN_SIZE) "]; \n"
	"	uint luminance_sumed; \n"
	"								\n"
	"	uint accHist[" __stringify(IndirectStats__c_HIST_BIN_SIZE) "]; \n"
	"	float accHistNormal[" __stringify(IndirectStats__c_HIST_BIN_SIZE) "]; \n"
	"	uint accCount; \n"
	"	uint pointIndexFlags[8];\n"
	"	uint pointIndex[8]; \n"
	"								\n"
	"	uint bhe_lut[" __stringify(IndirectStats__c_HIST_BIN_SIZE) "]; \n"
	"	float bhe_lut0[" __stringify(IndirectStats__c_HIST_BIN_SIZE) "]; \n"
	"	float bhe_lut1[" __stringify(IndirectStats__c_HIST_BIN_SIZE) "]; \n"
	"								\n"
	"	uint lumi_counter; \n"
	"	uint luminance_mean;\n"
	"	uint luminance_std;\n"
	"	float lumi_mean; \n"
	"	float lumi_std; \n"
	"	uint type; \n" // 0, 1, 2, 3
	"	float y1; \n"
	"	float y2; \n"
	"	uint agc_lut[" __stringify(IndirectStats__c_HIST_BIN_SIZE) "]; \n"
	"	uint agc_counter; \n"
	"								\n"
	"	uint merged_lut[" __stringify(IndirectStats__c_HIST_BIN_SIZE) "]; \n"
	"	float bhe_factor0; \n"
	"	float bhe_factor1; \n"
	"} uboOut; \n"
	"													\n"
	"const float DIFF = 0.001 * " __stringify(IndirectStats__c_HIST_BIN_SIZE) ".0; \n"
	"													\n"
	"uint getLinearDiff(uint level) { \n"
	"	if (abs(int(uboOut.bhe_lut[level]) - int(level)) < abs(int(uboOut.agc_lut[level]) - int(level))) { return 0; } \n"
	"	else { return 1; } \n"
	"} \n"
	"uint isCurveCross(uint level) { \n"
	"	if (float(abs(int(uboOut.bhe_lut[level]) - int(uboOut.agc_lut[level]))) <= DIFF) { return 1; } \n"
	"	else { return 0; } \n"
	"} \n"
	"void setMergedLut(uint curve_sel, uint level) {\n"
	"	if (curve_sel == 0) { uboOut.merged_lut[level] = uboOut.bhe_lut[level]; } \n"
	"	else { uboOut.merged_lut[level] = uboOut.agc_lut[level]; } \n"
	"} \n"
	"\n"
	"void main() \n"
	"{\n"
	"	uint level = 0; \n"
	"	float x2 = float(gl_GlobalInvocationID.x); \n"
	"	x2 = x2 / float(gl_NumWorkGroups.x);"
	"\n"
	"	float y = 0.0;"
	"	if (uboOut.type == 0) { \n"
	"		float pow_x2 = pow(x2, uboOut.y1); \n"
	"		y = pow_x2 / (pow_x2 + pow(uboOut.lumi_mean, uboOut.y1) * (1.0 - pow_x2)); \n"
	"	} else if(uboOut.type == 1) { \n"
	"		float pow_x2 = pow(x2, uboOut.y1); \n"
	"		y = pow_x2; \n"
	"	} else if(uboOut.type == 2) { \n"
	"		float pow_x2 = pow(x2, uboOut.y2); \n"
	"		y = pow_x2 / (pow_x2 + pow(uboOut.lumi_mean, uboOut.y2) * (1.0 - pow_x2)); \n"
	"	} else if(uboOut.type == 3) { \n"
	"		float pow_x2 = pow(x2, uboOut.y2); \n"
	"		y = pow_x2; \n"
	"	} \n"
	"	uboOut.agc_lut[gl_GlobalInvocationID.x] = uint(y * " __stringify(IndirectStats__c_HIST_BIN_SIZE) ".0); \n"
#if 1  // here merge lut
	"											\n"
	"	atomicAdd(uboOut.agc_counter, 1); \n"
	"	if (uboOut.agc_counter == (gl_NumWorkGroups.x *gl_NumWorkGroups.y) * (gl_WorkGroupSize.x * gl_WorkGroupSize.y)) { \n"
	"		uint curve_sel = 0; \n"
	"		uboOut.merged_lut[0] = uboOut.bhe_lut[0]; \n"
	"		uboOut.merged_lut[1] = uboOut.bhe_lut[1]; \n"
	"		for(level=2;level<" __stringify(IndirectStats__c_HIST_BIN_SIZE) ";level++) { \n"
	"			uint current_curve = getLinearDiff(level); \n"
	"			if (current_curve == curve_sel) { setMergedLut(curve_sel, level); } \n"
	"			else { \n"
	"				if (isCurveCross(level) > 0) { \n"
	"					curve_sel = current_curve; \n"
	"					setMergedLut(curve_sel, level); \n"
	"				} else {\n"
	"					setMergedLut(curve_sel, level);  \n"
	"				} \n"
	"			} \n"
	"		} \n"
	"	} \n"
	"											\n"
#endif
	"}\n"
	;
static const char *shader_apply_lut = " \n"
	"\n"
	"#version 450 \n"
	"precision highp float; \n" //"precision mediump float; \n"
	"precision highp int;\n"
	"layout (local_size_x = 16, local_size_y = 16) in; \n"
	"layout (binding = 0, rgba8) uniform readonly image2D scaledImage; \n"
	"layout (binding = 1, rgba8) uniform image2D resultImage; \n"
	"layout (binding = 5, rgba8) uniform readonly image2D inputAlignedImage; \n"
	"// Binding 3: Indirect draw stats \n"
	"layout (binding = 3) buffer UBOOut \n"
	"{ \n"
	"	uint drawCount; \n"
	"	uint lodCount[" __stringify(IndirectStats__c_HIST_BIN_SIZE) "]; \n"
	"	uint luminance_sumed; \n"
	"								\n"
	"	uint accHist[" __stringify(IndirectStats__c_HIST_BIN_SIZE) "]; \n"
	"	float accHistNormal[" __stringify(IndirectStats__c_HIST_BIN_SIZE) "]; \n"
	"	uint accCount; \n"
	"	uint pointIndexFlags[8];\n"
	"	uint pointIndex[8]; \n"
	"								\n"
	"	uint bhe_lut[" __stringify(IndirectStats__c_HIST_BIN_SIZE) "]; \n"
	"	float bhe_lut0[" __stringify(IndirectStats__c_HIST_BIN_SIZE) "]; \n"
	"	float bhe_lut1[" __stringify(IndirectStats__c_HIST_BIN_SIZE) "]; \n"
	"								\n"
	"	uint lumi_counter; \n"
	"	uint luminance_mean;\n"
	"	uint luminance_std;\n"
	"	float lumi_mean; \n"
	"	float lumi_std; \n"
	"	uint type; \n" // 0, 1, 2, 3
	"	float y1; \n"
	"	float y2; \n"
	"	uint agc_lut[" __stringify(IndirectStats__c_HIST_BIN_SIZE) "]; \n"
	"	uint agc_counter; \n"
	"								\n"
	"	uint merged_lut[" __stringify(IndirectStats__c_HIST_BIN_SIZE) "]; \n"
	"	float bhe_factor0; \n"
	"	float bhe_factor1; \n"
	"} uboOut; \n"
	"\n"
	"layout (constant_id = 100) const uint lut_selection = 0; \n"
	"							\n"
	"const vec3 W2 = vec3(0.299, 0.587, 0.114); \n"
	"const vec3 Pb_r = vec3(-0.169, -0.331, 0.5); \n"
	"const vec3 Pr_r = vec3(0.5, -0.439, -0.081); \n"
	"							\n"
	"const vec3 R_r = vec3(1.0, 0.0, 1.402);  \n"
	"const vec3 G_r = vec3(1.0, -0.344, -0.792); \n"
	"const vec3 B_r = vec3(1.0, 1.772, 0.0); \n"
	"							\n"
	"vec3 getHSV_(vec3 rgb) { \n"
	"	int bR = int(rgb.r * 255.0);		\n"
	"	int bG = int(rgb.g * 255.0); 		\n"
	"	int bB = int(rgb.b * 255.0);		\n"
	"	float fR =rgb.r; 			\n"
	"	float fG = rgb.g; 			\n"
	"	float fB = rgb.b; 			\n"
	"	float fH = 0.0;  \n"
	"	float fS = 0.0;  \n"
	"	float fV = 0.0; 			\n"
	"	float fDelta; 				\n"
	"	float fMin, fMax;			\n"
	"	int iMax;  					\n"
	"	if (bB < bG) { \n"
	"		if (bB < bR) { \n"
	"			fMin = fB; \n"
	"			if (bR > bG) { iMax = bR; fMax = fR;} \n"
	"			else {iMax = bG;fMax = fG;} \n"
	"		} \n"
	"		else { fMin = fR;fMax = fG;iMax = bG; } \n"
	"	} \n"
	"	else { \n"
	"		if (bG < bR) { \n"
	"			fMin = fG; \n"
	"			if (bB > bR) { fMax = fB; iMax = bB; } \n"
	"			else { fMax = fR; iMax = bR; } \n"
	"		} \n"
	"		else { fMin = fR; fMax = fB; iMax = bB; } \n"
	"	} \n"
	"	fDelta = fMax - fMin; \n"
	"\n"
	"	fV = fMax;    // Value (Brightness). \n"
	"\n"
#if 0
	"	if (iMax != 0) {   // Make sure its not pure black. \n"
	"		fS = fDelta / fMax;  // Saturation. \n"
	"		float ANGLE_TO_UNIT = 1.0f / (6.0f * fDelta); // Make the Hues between 0.0 to 1.0 instead of 6.0 \n"
	"		if (iMax == bR) {  // between yellow and magenta. \n"
	"			fH = (fG - fB) * ANGLE_TO_UNIT; \n"
	"		} \n"
	"		else if (iMax == bG) {	// between cyan and yellow. \n"
	"			fH = (2.0f / 6.0f) + (fB - fR) * ANGLE_TO_UNIT; \n"
	"		} \n"
	"		else {	  // between magenta and cyan. \n"
	"			fH = (4.0f / 6.0f) + (fR - fG) * ANGLE_TO_UNIT; \n"
	"		} \n"
	"		// Wrap outlier Hues around the circle. \n"
	"		if (fH < 0.0f) \n"
	"			fH += 1.0f; \n"
	"		if (fH >= 1.0f) \n"
	"			fH -= 1.0f; \n"
	"	} \n"
	"	else { \n"
	"		// color is pure Black. \n"
	"		fS = 0; \n"
	"		fH = 0; // undefined hue \n"
	"	} \n"
#else
	"	if (fDelta == 0) {					\n"
	"		fH = 0.0;						\n"
	"	} else { \n"
	"		float ANGLE_TO_UNIT = 1.0f / (6.0f * fDelta); // Make the Hues between 0.0 to 1.0 instead of 6.0 \n"
	"		if (iMax == bR) {  // between yellow and magenta. \n"
	"			fH = (fG - fB) * ANGLE_TO_UNIT; \n"
	"		} \n"
	"		else if (iMax == bG) {  // between cyan and yellow. \n"
	"			fH = (2.0f / 6.0f) + (fB - fR) * ANGLE_TO_UNIT; \n"
	"		} \n"
	"		else {    // between magenta and cyan. \n"
	"			fH = (4.0f / 6.0f) + (fR - fG) * ANGLE_TO_UNIT; \n"
	"		} \n"
	"		// Wrap outlier Hues around the circle. \n"
	"		if (fH < 0.0f) { \n"
	"			fH += 1.0f; \n"
	"		} \n"
	"		if (fH >= 1.0f) { \n"
	"			fH -= 1.0f; \n"
	"		} \n"
	"	} \n"
	"									\n"
	"	if (iMax != 0) {   // Make sure its not pure black. \n"
	"		fS = fDelta / fMax;  // Saturation. \n"
	"	} else {\n"
	"		// color is pure Black. \n"
	"		fS = 0.0;"
	"	}\n"
#endif
#if 0
	"	if(fH > 1.0) { fH = 1.0; } else if(fH < 0.0) { fH = 0.0; } \n"
	"	if(fS > 1.0) { fS = 1.0; } else if(fS < 0.0) { fS = 0.0; } \n"
	"	if(fV > 1.0) { fV = 1.0; } else if(fV < 0.0) { fV= 0.0; } \n"
#else
	"	fH = clamp(fH, 0.0, 1.0);			\n"
	"	fS = clamp(fS, 0.0, 1.0);			\n"
	"	fV = clamp(fV, 0.0, 1.0);			\n"
#endif
	"	return vec3(fH, fS, fV); \n"
	"} \n"
	"								\n"
	"vec3 getRGB_(vec3 hsv) { \n"
	"	float h = hsv.x; \n"
	"	float s = hsv.y; \n"
	"	float v = hsv.z; \n"
	"	float r = 0.0; \n"
	"	float g = 0.0; \n"
	"	float b = 0.0; \n"
	"	float hi = h * 6.0;			\n"
	"	uint uhi = uint(floor(hi)); 				\n"
	"	uhi = uint(mod(uhi, 6));  \n"
	"	float f = (hi - float(uhi)); 				\n"
	"	float p = v * (1 - s); \n"
	"	float q = v * (1 - s * f); \n"
	"	float t = v * (1 - s * (1 - f)); \n"
	"	if (s == 0) { \n"
	"		r = g = b = v; \n"
	"	} else { \n"
	"		if (uhi == 0) { r = v; g = t; b = p; } \n"
	"		else if (uhi == 1) { r = q; g = v; b = p;} \n"
	"		else if (uhi == 2) { r = p; g = v; b = t; } \n"
	"		else if (uhi == 3) { r = p; g = q; b = v; } \n"
	"		else if (uhi == 4) { r = t; g = p; b = v; } \n"
	"		else if (uhi == 5) { r = v; g = p; b = q; } \n"
	"	} \n"
	"	if(r > 1.0) { r = 1.0; } else if(r < 0.0) { r= 0.0; } \n"
	"	if(g > 1.0) { g = 1.0; } else if(g < 0.0) { g = 0.0; } \n"
	"	if(b > 1.0) { b= 1.0; } else if(b < 0.0) { b= 0.0; } \n"
	"	return vec3(r, g, b); \n"

	"} \n"
	"void main() \n"
	"{\n"
	"	vec3 rgb = imageLoad(inputAlignedImage, ivec2(gl_GlobalInvocationID.xy)).rgb; \n"
#if 0
	"	float luminance = dot(rgb.rgb, W2); \n"
	"	float pb = dot(rgb.rgb, Pb_r); \n"
	"	float pr = dot(rgb.rgb, Pr_r); \n"
	"	vec3 ypbpr = vec3(luminance, pb, pr);  \n"
#else
	"	vec3 ypbpr = getHSV_(rgb.rgb); \n"
	"	float luminance = ypbpr.z; \n"
#endif
	"	\n"
#if 1
	"	uint index = uint(luminance * " __stringify(IndirectStats__c_HIST_BIN_SIZE) ".0);"
	"	if (index >= " __stringify(IndirectStats__c_HIST_BIN_SIZE) ") { \n"
	"		index = " __stringify(IndirectStats__c_HIST_BIN_SIZE) " - 1; \n"
	"	} \n"
	"				\n"
	"	if (lut_selection == 0) {"
	"		luminance = float(uboOut.agc_lut[index]); \n"
	"	} else if(lut_selection == 1) { \n"
	"		luminance = float(uboOut.bhe_lut[index]); \n"
	"	} else { \n"
	"		luminance = float(uboOut.merged_lut[index]); \n"
	"	} \n"
	"				\n"
	"	luminance = luminance / " __stringify(IndirectStats__c_HIST_BIN_SIZE) ".0;  \n"
	"	clamp(luminance, 0.0, 1.0);  \n"
#endif
	"	ypbpr.z = luminance; \n"
	"	\n"

#if 0
	"	float out_r = dot(ypbpr.rgb, R_r); \n"
	"	float out_g = dot(ypbpr.rgb, G_r); \n"
	"	float out_b = dot(ypbpr.rgb, B_r); \n"
	"	vec4 res = vec4(out_r, out_g, out_b, 1.0); \n"
#else
	"	vec3 out_rgb = getRGB_(ypbpr); \n"
	"	vec4 res = vec4(out_rgb, 1.0); \n"
#endif
	//"	res = vec4(1.0, 0.0, 0.0, 1.0); \n"
	"	imageStore(resultImage, ivec2(gl_GlobalInvocationID.xy), res); \n"
	"}\n"
	;


HistogramCompute::HistogramCompute(VulkanExample *vulkanExample)
	: VulkanExample_Parasite(vulkanExample) {

	if (mVulkanExample->mInitFlag == VulkanExample::INIT_COMPUTE_AGC) {
		mLutSection = 0;
	} else if(mVulkanExample->mInitFlag == VulkanExample::INIT_COMPUTE_BHE) {
		mLutSection = 1;
	} else if(mVulkanExample->mInitFlag == VulkanExample::INIT_COMPUTE_AGC_BHE) {
		mLutSection = 2;
	}
{
	Shader_ shader_1_ = {COMP_CONV1, "", shader_1};
	mShaders.insert(COMP_CONV1, shader_1_);
}
{
	Shader_ shader_cal_histogram_mean_ = {COMP_CAL_HIST_MEAN, "shader_cal_histogram_mean", shader_cal_histogram_mean};
	mShaders.insert(COMP_CAL_HIST_MEAN, shader_cal_histogram_mean_);
}
{
	Shader_ shader_gen_acc_hist_ = {COMP_GEN_ACC_HIST, "shader_gen_acc_hist", shader_gen_acc_hist};
	mShaders.insert(COMP_GEN_ACC_HIST, shader_gen_acc_hist_);
}
{
	Shader_ shader_gen_bhe_lut_ = {COMP_GEN_BHE_LUT, "shader_gen_bhe_lut", shader_gen_bhe_lut};
	mShaders.insert(COMP_GEN_BHE_LUT, shader_gen_bhe_lut_);
}
{
	Shader_ shader_cal_std_ = {COMP_CAL_STD, "shader_cal_std", shader_cal_std};
	mShaders.insert(COMP_CAL_STD, shader_cal_std_);
}
{
	Shader_ shader_gen_agc_lut_ = {COMP_GEN_AGC_LUT, "shader_gen_agc_lut", shader_gen_agc_lut};
	mShaders.insert(COMP_GEN_AGC_LUT, shader_gen_agc_lut_);
}
{
	Shader_ shader_apply_lut_ = {COMP_APPLY_LUT, "shader_apply_lut", shader_apply_lut};
	mShaders.insert(COMP_APPLY_LUT, shader_apply_lut_);
}
}
HistogramCompute::~HistogramCompute() {
	if (mHistogramGrapher_ != nullptr) {
		delete mHistogramGrapher_; mHistogramGrapher_ = nullptr;
	}

	// release all resources allocated !!!

	// Compute
	int stage_ = 0;
	VkPipeline pipeline_ = VK_NULL_HANDLE;
	mPipelines.next_begin();
	while(mPipelines.next(&stage_, &pipeline_) == 0) {
		vkDestroyPipeline(mDevice, pipeline_, nullptr);
	}

	//
	vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(mDevice, mDescriptorSetLayout, nullptr);

	//

	//
	vkDestroyFence(mDevice, mFence, nullptr);
	vkDestroyCommandPool(mDevice, mCommandPool, nullptr);

	mSourceAlignScaled.destroy();
	mSourceAligned.destroy();
	mTextureTarget.destroy();
	indirectDrawCountBuffer.destroy();
	stdTempBuffer.destroy();
	if (mStdTempBuffer != nullptr) { delete mStdTempBuffer; mStdTempBuffer = nullptr; }

	if (mDescriptorPool != VK_NULL_HANDLE) {
		vkDestroyDescriptorPool(mDevice, mDescriptorPool, nullptr);
	}

}

void HistogramCompute::getComputeQueue() {
	uint32_t queueFamilyCount;
	vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queueFamilyCount, NULL);
	assert(queueFamilyCount >= 1);

	std::vector<VkQueueFamilyProperties> queueFamilyProperties;
	queueFamilyProperties.resize(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queueFamilyCount, queueFamilyProperties.data());

	// Some devices have dedicated compute queues, so we first try to find a queue that supports compute and not graphics
	bool computeQueueFound = false;
	for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++)
	{
		if ((queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) 
			&& ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
		{
			mQueueFamilyIndex = i;
			computeQueueFound = true;
			break;
		}
	}
	// If there is no dedicated compute queue, just find the first queue family that supports compute
	if (!computeQueueFound)
	{
		for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++)
		{
			if (queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
			{
				mQueueFamilyIndex = i;
				computeQueueFound = true;
				break;
			}
		}
	}

	// Compute is mandatory in Vulkan, so there must be at least one queue family that supports compute
	assert(computeQueueFound);
	// Get a compute queue from the device
	vkGetDeviceQueue(mDevice, mQueueFamilyIndex, 0, &mQueue);

	DLOGD( "* HistogramCompute::%s, compute.queueFamilyIndex = %d, queue=%p \r\n", __func__, mQueueFamilyIndex, mQueue);
}

void HistogramCompute::createTextureTarget(vks::Texture *tex, uint32_t w_, uint32_t h_, VkFormat format, VkImageUsageFlags extra_flags) {
	uint32_t width = w_;
	uint32_t height = h_;

	vks::VulkanDevice *vulkanDevice = mVulkanExample->vulkanDevice;

	VkFormatProperties formatProperties;

	// Get device properties for the requested texture format
	vkGetPhysicalDeviceFormatProperties(mPhysicalDevice, format, &formatProperties);
	// Check if requested image format supports image storage operations
	assert(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT);

{
	DLOGD( "HistogramCompute::%s format 0x%08x(%d) \r\n" , __func__, format, format);
	DLOGD( "	 linearTilingFeatures:\r\n");
	::vks::android::print_VkFormatFeatureFlags(formatProperties.linearTilingFeatures);
	DLOGD( "	 optimalTilingFeatures:\r\n");
	::vks::android::print_VkFormatFeatureFlags(formatProperties.optimalTilingFeatures);
	DLOGD( "	 bufferFeatures:\r\n");
	::vks::android::print_VkFormatFeatureFlags(formatProperties.bufferFeatures);
}
	// Prepare blit target texture
	tex->width = width;
	tex->height = height;
	DLOGD( "HistogramCompute::%s, size = %4d x %4d \r\n", __func__, width, height);

	VkImageCreateInfo imageCreateInfo = vks::initializers::imageCreateInfo();
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = format;
	imageCreateInfo.extent = { width, height, 1 };
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	// Image will be sampled in the fragment shader and used as storage target in the compute shader
	imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT
		| VK_IMAGE_USAGE_STORAGE_BIT
		| extra_flags;
	imageCreateInfo.flags = 0;
	// Sharing mode exclusive means that ownership of the image does not need to be explicitly transferred between the compute and graphics queue
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkMemoryAllocateInfo memAllocInfo = vks::initializers::memoryAllocateInfo();
	VkMemoryRequirements memReqs;

	VK_CHECK_RESULT(vkCreateImage(mDevice, &imageCreateInfo, nullptr, &tex->image));

	vkGetImageMemoryRequirements(mDevice, tex->image, &memReqs);
	memAllocInfo.allocationSize = memReqs.size;
	memAllocInfo.memoryTypeIndex = vulkanDevice->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VK_CHECK_RESULT(vkAllocateMemory(mDevice, &memAllocInfo, nullptr, &tex->deviceMemory));
	VK_CHECK_RESULT(vkBindImageMemory(mDevice, tex->image, tex->deviceMemory, 0));

	VkCommandBuffer layoutCmd = mVulkanExample->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
	tex->imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	vks::tools::setImageLayout(
		layoutCmd, tex->image, 
		VK_IMAGE_ASPECT_COLOR_BIT, 
		VK_IMAGE_LAYOUT_UNDEFINED, 
		tex->imageLayout);
	mVulkanExample->flushCommandBuffer(layoutCmd, mQueue, true);

	// Create sampler
	VkSamplerCreateInfo sampler = vks::initializers::samplerCreateInfo();
	sampler.magFilter = VK_FILTER_LINEAR;
	sampler.minFilter = VK_FILTER_LINEAR;
	sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	sampler.addressModeV = sampler.addressModeU;
	sampler.addressModeW = sampler.addressModeU;
	sampler.mipLodBias = 0.0f;
	sampler.maxAnisotropy = 1.0f;
	sampler.compareOp = VK_COMPARE_OP_NEVER;
	sampler.minLod = 0.0f;
	sampler.maxLod = tex->mipLevels;
	sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	VK_CHECK_RESULT(vkCreateSampler(mDevice, &sampler, nullptr, &tex->sampler));

	// Create image view
	VkImageViewCreateInfo view = vks::initializers::imageViewCreateInfo();
	view.image = VK_NULL_HANDLE;
	view.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view.format = format;
	view.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	view.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	view.image = tex->image;
	VK_CHECK_RESULT(vkCreateImageView(mDevice, &view, nullptr, &tex->view));

	// Initialize a descriptor for later use
	tex->descriptor.imageLayout = tex->imageLayout;
	tex->descriptor.imageView = tex->view;
	tex->descriptor.sampler = tex->sampler;
	tex->device = vulkanDevice;
}

void HistogramCompute::createDescriptorPool() {
	std::vector<VkDescriptorPoolSize> poolSizes = {
		// Compute pipelines uses a storage image for image reads and writes
		vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 3),
		vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1),
		vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1)
	};
	VkDescriptorPoolCreateInfo descriptorPoolInfo = vks::initializers::descriptorPoolCreateInfo(
			static_cast<uint32_t>(poolSizes.size()), poolSizes.data(), 3);

	VK_CHECK_RESULT(vkCreateDescriptorPool(mDevice, &descriptorPoolInfo, nullptr, &mDescriptorPool));
}

// here build our seperate command buffer, then should submit seperately !
void HistogramCompute::buildCommandBuffers() {
	// build our command buffer

	// Flush the queue if we're rebuilding the command buffer after a pipeline change to ensure it's not currently in use
	vkQueueWaitIdle(mQueue);

	VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

	VK_CHECK_RESULT(vkBeginCommandBuffer(mCommandBuffer, &cmdBufInfo));

	// using (source) to calculate histogram, mean into "buffer"
	vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, mPipelines.getAssert(COMP_CAL_HIST_MEAN));
	vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, mPipelineLayout, 0, 1, &mDescriptorSet, 0, 0);
	vkCmdDispatch(mCommandBuffer, mScaledWidth / 16, mScaledHeight / 16, 1);

#if 1
	//(using buffer) to generate acc histogram (including normalization acc histogram), and 0.3, 0.5, 0.8 point index !!!
	vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, mPipelines.getAssert(COMP_GEN_ACC_HIST));
	vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, mPipelineLayout, 0, 1, &mDescriptorSet, 0, 0);
	vkCmdDispatch(mCommandBuffer, IndirectStats__c_HIST_BIN_SIZE, 1, 1);
#endif

#if 1
	//using acc histogram (including normalization acc histogram), and 0.3, 0.5, 0.8 point index to generate LUT
	vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, mPipelines.getAssert(COMP_GEN_BHE_LUT));
	vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, mPipelineLayout, 0, 1, &mDescriptorSet, 0, 0);
	vkCmdDispatch(mCommandBuffer, IndirectStats__c_HIST_BIN_SIZE, 1, 1);
#endif

#if 1
	// using (buffer, source) to calculate the "std"
	// here should ensure the buffer is already written by previous compute stage !!!
	vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, mPipelines.getAssert(COMP_CAL_STD));
	vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, mPipelineLayout, 0, 1, &mDescriptorSet, 0, 0);
	vkCmdDispatch(mCommandBuffer, mScaledWidth / 16, mScaledHeight / 16, 1);
#endif

#if 1
	// using (mean, std) to produce the LUT of AGC (size if IndirectStats__c_HIST_BIN_SIZE x 1)
	vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, mPipelines.getAssert(COMP_GEN_AGC_LUT));
	vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, mPipelineLayout, 0, 1, &mDescriptorSet, 0, 0);
	vkCmdDispatch(mCommandBuffer, IndirectStats__c_HIST_BIN_SIZE, 1, 1);
#endif
#if 1
	vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, mPipelines.getAssert(COMP_APPLY_LUT));
	vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, mPipelineLayout, 0, 1, &mDescriptorSet, 0, 0);
	vkCmdDispatch(mCommandBuffer, mSourceAlignedWidth / 16, mSourceAlignedHeight / 16, 1);
#endif




	vkEndCommandBuffer(mCommandBuffer);

}

void HistogramCompute::prepare() {
	vks::VulkanDevice *vulkanDevice = mVulkanExample->vulkanDevice;

#if 0
	if (mHistogramGrapher_ == nullptr) {
		mHistogramGrapher_ = HistogramGrapher::create();
	}
#endif

	uint32_t w_ = 1080;
	uint32_t h_ = 2340;

	mSourceTextureWidth = w_;
	mSourceTextureHeight = h_;
	mSourceAlignedWidth = ALIGN_SIZE(w_, 16);
	mSourceAlignedHeight = ALIGN_SIZE(h_, 16);

	mScaledWidth = ALIGN_SIZE(w_, 16);
	mScaledHeight = ALIGN_SIZE(h_, 16);
#if 1
	mScaledWidth = ALIGN_SIZE(w_/4, 16);
	mScaledHeight = ALIGN_SIZE(h_/4, 16);
#endif

	if (mStdTempBuffer != nullptr) { delete mStdTempBuffer; mStdTempBuffer = nullptr; }
	if (mStdTempBuffer == nullptr) { mStdTempBuffer = new stdTempBuffer_c(mScaledWidth, mScaledHeight); }

	VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

	DLOGD( "HistogramCompute::%s source size = %4d x %4d -> scaled size:%4d x %4d \r\n", 
		__func__, w_, h_, mScaledWidth, mScaledHeight);

/* should prepare: mPhysicalDevice, mDevice, mPipelineCache
*/
	getComputeQueue();  // mQueueFamilyIndex, mQueue

	createTextureTarget(&mSourceAlignScaled, mScaledWidth, mScaledHeight, format, VK_IMAGE_USAGE_TRANSFER_DST_BIT);
	createTextureTarget(&mTextureTarget, mSourceAlignedWidth, mSourceAlignedHeight, format, 0);
	createTextureTarget(&mSourceAligned, mSourceAlignedWidth, mSourceAlignedHeight, format, 0);

	// buffer
	VK_CHECK_RESULT(vulkanDevice->createBuffer(
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT 
			| VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&indirectDrawCountBuffer, sizeof(indirectStats)));
	// Map for host access
	VK_CHECK_RESULT(indirectDrawCountBuffer.map());

	//
	VK_CHECK_RESULT(vulkanDevice->createBuffer(
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
			| VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&stdTempBuffer, mStdTempBuffer->getLength()));
	// Map for host access
	VK_CHECK_RESULT(stdTempBuffer.map());


	createDescriptorPool();

	// Create compute pipeline
	// Compute pipelines are created separate from graphics pipelines even if they use the same queue

	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
		// Binding 0: Input image (read-only)
		vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 0),
		// Binding 1: Output image (write)
		vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 1),
		// Binding 3: Indirect draw stats (output)
		vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,VK_SHADER_STAGE_COMPUTE_BIT,3),
		#if 0// Binding 4: Indirect draw stats (output)
		vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,VK_SHADER_STAGE_COMPUTE_BIT,4),
		#endif
		// Binding 5: Output image (write)
		vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 5),
	};
	VkDescriptorSetLayoutCreateInfo descriptorLayout = vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings);
	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(mDevice, &descriptorLayout, nullptr, &mDescriptorSetLayout));

{
	VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
		vks::initializers::pipelineLayoutCreateInfo(&mDescriptorSetLayout, 1);
	VK_CHECK_RESULT(vkCreatePipelineLayout(mDevice, &pPipelineLayoutCreateInfo, nullptr, &mPipelineLayout));
}

	VkDescriptorSetAllocateInfo allocInfo =
		vks::initializers::descriptorSetAllocateInfo(mDescriptorPool, &mDescriptorSetLayout, 1);
	VK_CHECK_RESULT(vkAllocateDescriptorSets(mDevice, &allocInfo, &mDescriptorSet));
	std::vector<VkWriteDescriptorSet> computeWriteDescriptorSets = {
		//
		vks::initializers::writeDescriptorSet(mDescriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0, &mSourceAlignScaled.descriptor),
		//
		vks::initializers::writeDescriptorSet(mDescriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, &mTextureTarget.descriptor),
		// Binding 3: Atomic counter (written in shader)
		vks::initializers::writeDescriptorSet(mDescriptorSet,VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,3, &indirectDrawCountBuffer.descriptor),
		#if 0// Binding 4:
		vks::initializers::writeDescriptorSet(mDescriptorSet,VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,4, &stdTempBuffer.descriptor),
		#endif
		// Binding 5: for aligned source texture !!!
		vks::initializers::writeDescriptorSet(mDescriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 5, &mSourceAligned.descriptor),
	};
	vkUpdateDescriptorSets(mDevice, computeWriteDescriptorSets.size(), computeWriteDescriptorSets.data(), 0, NULL);



{
	// Create compute shader pipeline
	VkComputePipelineCreateInfo computePipelineCreateInfo =
		vks::initializers::computePipelineCreateInfo(mPipelineLayout, 0);

	Shader_ shader_ = mShaders.getAssert(COMP_CAL_HIST_MEAN);
	DLOGD( "compiling : %s \r\n", shader_.name.c_str());
	VkPipelineShaderStageCreateInfo shaderStageCi = 
		mVulkanExample->loadShader_from_strings_c(shader_.source, VK_SHADER_STAGE_COMPUTE_BIT);
	computePipelineCreateInfo.stage = shaderStageCi;
	VkPipeline pipeline;
	VK_CHECK_RESULT(vkCreateComputePipelines(mDevice, mPipelineCache, 1, &computePipelineCreateInfo, nullptr, &pipeline));
	mPipelines.insert(COMP_CAL_HIST_MEAN, pipeline);
}
{
	// Create compute shader pipeline
	VkComputePipelineCreateInfo computePipelineCreateInfo =
		vks::initializers::computePipelineCreateInfo(mPipelineLayout, 0);

	Shader_ shader_ = mShaders.getAssert(COMP_GEN_ACC_HIST);
	DLOGD( "compiling : %s \r\n", shader_.name.c_str());
	VkPipelineShaderStageCreateInfo shaderStageCi = 
		mVulkanExample->loadShader_from_strings_c(shader_.source, VK_SHADER_STAGE_COMPUTE_BIT);
	computePipelineCreateInfo.stage = shaderStageCi;
	VkPipeline pipeline;
	VK_CHECK_RESULT(vkCreateComputePipelines(mDevice, mPipelineCache, 1, &computePipelineCreateInfo, nullptr, &pipeline));
	mPipelines.insert(COMP_GEN_ACC_HIST, pipeline);
}
#if 1
{
	// Create compute shader pipeline
	VkComputePipelineCreateInfo computePipelineCreateInfo =
		vks::initializers::computePipelineCreateInfo(mPipelineLayout, 0);

	Shader_ shader_ = mShaders.getAssert(COMP_GEN_BHE_LUT);
	DLOGD( "compiling : %s \r\n", shader_.name.c_str());
	VkPipelineShaderStageCreateInfo shaderStageCi = 
		mVulkanExample->loadShader_from_strings_c(shader_.source, VK_SHADER_STAGE_COMPUTE_BIT);
	computePipelineCreateInfo.stage = shaderStageCi;
	VkPipeline pipeline;
	VK_CHECK_RESULT(vkCreateComputePipelines(mDevice, mPipelineCache, 1, &computePipelineCreateInfo, nullptr, &pipeline));
	mPipelines.insert(COMP_GEN_BHE_LUT, pipeline);
}
#endif
{
	// Create compute shader pipeline
	VkComputePipelineCreateInfo computePipelineCreateInfo =
		vks::initializers::computePipelineCreateInfo(mPipelineLayout, 0);

	Shader_ shader_ = mShaders.getAssert(COMP_CAL_STD);
	DLOGD( "compiling : %s \r\n", shader_.name.c_str());
	VkPipelineShaderStageCreateInfo shaderStageCi = 
		mVulkanExample->loadShader_from_strings_c(shader_.source, VK_SHADER_STAGE_COMPUTE_BIT);
	computePipelineCreateInfo.stage = shaderStageCi;
	VkPipeline pipeline;
	VK_CHECK_RESULT(vkCreateComputePipelines(mDevice, mPipelineCache, 1, &computePipelineCreateInfo, nullptr, &pipeline));
	mPipelines.insert(COMP_CAL_STD, pipeline);
}
#if 1
{
	// Create compute shader pipeline
	VkComputePipelineCreateInfo computePipelineCreateInfo =
		vks::initializers::computePipelineCreateInfo(mPipelineLayout, 0);

	Shader_ shader_ = mShaders.getAssert(COMP_GEN_AGC_LUT);
	DLOGD( "compiling : %s \r\n", shader_.name.c_str());
	VkPipelineShaderStageCreateInfo shaderStageCi = 
		mVulkanExample->loadShader_from_strings_c(shader_.source, VK_SHADER_STAGE_COMPUTE_BIT);
	computePipelineCreateInfo.stage = shaderStageCi;
	VkPipeline pipeline;
	VK_CHECK_RESULT(vkCreateComputePipelines(mDevice, mPipelineCache, 1, &computePipelineCreateInfo, nullptr, &pipeline));
	mPipelines.insert(COMP_GEN_AGC_LUT, pipeline);
}
#endif
#if 1
{
	// Create compute shader pipeline
	VkComputePipelineCreateInfo computePipelineCreateInfo =
		vks::initializers::computePipelineCreateInfo(mPipelineLayout, 0);

	Shader_ shader_ = mShaders.getAssert(COMP_APPLY_LUT);
	DLOGD( "compiling : %s \r\n", shader_.name.c_str());
	VkPipelineShaderStageCreateInfo shaderStageCi = 
		mVulkanExample->loadShader_from_strings_c(shader_.source, VK_SHADER_STAGE_COMPUTE_BIT);
	const VkSpecializationMapEntry entries[] = {
	    {
	        100,                            // constantID
	        0 * sizeof(uint32_t),           // offset
	        sizeof(uint32_t)                // size
	    },
	};
	const uint32_t data[1] = { mLutSection };
	const VkSpecializationInfo info =
	{
		1,									// mapEntryCount
		entries,							// pMapEntries
		1 * sizeof(uint32_t),				// dataSize
		data,								// pData
	};
	shaderStageCi.pSpecializationInfo = &info;

	computePipelineCreateInfo.stage = shaderStageCi;
	VkPipeline pipeline;
	VK_CHECK_RESULT(vkCreateComputePipelines(mDevice, mPipelineCache, 1, &computePipelineCreateInfo, nullptr, &pipeline));
	mPipelines.insert(COMP_APPLY_LUT, pipeline);
}
#endif

	// Separate command pool as queue family for compute may be different than graphics
	VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.queueFamilyIndex = mQueueFamilyIndex;
	cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	VK_CHECK_RESULT(vkCreateCommandPool(mDevice, &cmdPoolInfo, nullptr, &mCommandPool));

	// Create a command buffer for compute operations
	VkCommandBufferAllocateInfo cmdBufAllocateInfo =
		vks::initializers::commandBufferAllocateInfo(
			mCommandPool,
			VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			1);

	VK_CHECK_RESULT(vkAllocateCommandBuffers(mDevice, &cmdBufAllocateInfo, &mCommandBuffer));

	// Fence for compute CB sync
	VkFenceCreateInfo fenceCreateInfo = vks::initializers::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
	VK_CHECK_RESULT(vkCreateFence(mDevice, &fenceCreateInfo, nullptr, &mFence));
	
	buildCommandBuffers();
}

// here will append our command into the command buffer under the main renderpass  !!!
void HistogramCompute::appendCommandBuffers(const VkCommandBuffer commandBuffer) {

}

void HistogramCompute::draw() {
	// submit our command buffer to the queue

#if 1
	// Submit compute commands
	// Use a fence to ensure that compute command buffer has finished executin before using it again
	vkWaitForFences(mDevice, 1, &mFence, VK_TRUE, UINT64_MAX);
	{
		IndirectStats__c *stats = &indirectStats;
		memset( &indirectStats, 0, sizeof(indirectStats));

		{
			std::unique_lock<std::mutex> caller_lock(mBHE_factor_mutex_);
			indirectStats.bhe_factor0 = mBHE_factor0;
			indirectStats.bhe_factor1 = mBHE_factor1;
			indirectStats.bhe_tuning0 = mBHE_tuning0;
			indirectStats.bhe_tuning1 = mBHE_tuning1;
			if (mHistogramGrapher_ != nullptr) {
				mHistogramGrapher_->setBHE_factor(mBHE_factor0, mBHE_factor1);
			}
		}
		indirectStats.bin_count = IndirectStats__c_HIST_BIN_SIZE;
		memcpy(indirectDrawCountBuffer.mapped, &indirectStats, sizeof(indirectStats));
	}
	{
		memset(mStdTempBuffer->getBuffer(), 0, mStdTempBuffer->getLength());
		memcpy(stdTempBuffer.mapped, mStdTempBuffer->getBuffer(), mStdTempBuffer->getLength());
	}

	vkResetFences(mDevice, 1, &mFence);

	VkSubmitInfo computeSubmitInfo = vks::initializers::submitInfo();
	computeSubmitInfo.commandBufferCount = 1;
	computeSubmitInfo.pCommandBuffers = &mCommandBuffer;

	VK_CHECK_RESULT(vkQueueSubmit(mQueue, 1, &computeSubmitInfo, mFence));

	vkWaitForFences(mDevice, 1, &mFence, VK_TRUE, UINT64_MAX);

	// Get draw count from compute
	memcpy(&indirectStats, indirectDrawCountBuffer.mapped, sizeof(indirectStats));
{
	IndirectStats__c *stats = &indirectStats;
	for(int i=0;i<IndirectStats__c_HIST_BIN_SIZE;i++) {
		stats->lodCount_d[i] = stats->lodCount[i];
		stats->lodCount_d[i] /= stats->drawCount;
		stats->lodCount_d[i] *= 100;  // %
	}
	DLOGD( "sizeof(indirectStats) = %d \r\n", (int)sizeof(indirectStats));

	DLOGD( "%s, drawCount:%d luminance_sumed : %d mean:%d bin_size:%d \r\n", __func__, 
		stats->drawCount, stats->luminance_sumed, stats->luminance_sumed/stats->drawCount, IndirectStats__c_HIST_BIN_SIZE);

	DLOGD( "  accCount:%d pointIndex[0] = %d pointIndex[1] = %d pointIndex[2] = %d \r\n", 
		stats->accCount, stats->pointIndex[0], stats->pointIndex[1], stats->pointIndex[2]);

	double mean_ = stats->luminance_mean;  mean_ /= IndirectStats__c_HIST_BIN_SIZE;
	double std_ = stats->luminance_std; std_ /= IndirectStats__c_HIST_BIN_SIZE;
	DLOGD( "  lumi_counter: %d luminance_std : %d mean: %d =>    [mean: %2.6f std:%2.6f] / {:%2.6f   %2.6f }\r\n",
		stats->lumi_counter, 
		stats->luminance_std, stats->luminance_mean, 
		mean_, std_, 
		stats->lumi_mean, stats->lumi_std);

	DLOGD( "  agc_counter: %d \r\n", stats->agc_counter);

}
	//indirectStats.print();  // PRINT_HIST
	//indirectStats.print(IndirectStats__c::PRINT_ACC_HIST);
	indirectStats.print(IndirectStats__c::PRINT_AGC_LUT);
	indirectStats.print(IndirectStats__c::PRINT_BHE_LUT);
	indirectStats.print(IndirectStats__c::PRINT_MERGED_LUT);

	if (mHistogramGrapher_ != nullptr) {
		mHistogramGrapher_->draw_stats(&indirectStats);
	}
	
	//indirectStats.print();
#endif

}

void HistogramCompute::updateSource_VkDescriptorImageInfo(
	ImportedTexture *importedTexture,
	uint32_t w_, uint32_t h_, VkDescriptorImageInfo *descriptor) {

	DLOGD( "HistogramCompute::%s size = %4d x %4d dest size = %4d x %4d \r\n",
		__func__, w_, h_, mScaledWidth, mScaledHeight);

	mSourceTextureWidth = w_;
	mSourceTextureHeight = h_;
{
	// blit src image to the dest image

	VkCommandBuffer layoutCmd = mVulkanExample->createCommandBuffer(
		VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

	int i = 1;
	VkImageBlit imageBlit{};
	imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageBlit.srcSubresource.layerCount = 1;
	imageBlit.srcSubresource.mipLevel = i - 1;
	imageBlit.srcOffsets[1].x = int32_t(w_ >> 0);
	imageBlit.srcOffsets[1].y = int32_t(h_ >> 0);
	imageBlit.srcOffsets[1].z = 1;
	
	imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageBlit.dstSubresource.layerCount = 1;
	imageBlit.dstSubresource.mipLevel = 0; // i;
	imageBlit.dstOffsets[1].x = int32_t(mScaledWidth >> 0);
	imageBlit.dstOffsets[1].y = int32_t(mScaledHeight >> 0);
	imageBlit.dstOffsets[1].z = 1;

	vkCmdBlitImage(layoutCmd, 
		importedTexture->image_, VK_IMAGE_LAYOUT_GENERAL,
		mSourceAlignScaled.image, VK_IMAGE_LAYOUT_GENERAL,
		1, &imageBlit,
		VK_FILTER_LINEAR);

	imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageBlit.dstSubresource.layerCount = 1;
	imageBlit.dstSubresource.mipLevel = 0; // i;
	imageBlit.dstOffsets[1].x = int32_t(mSourceAlignedWidth >> 0);
	imageBlit.dstOffsets[1].y = int32_t(mSourceAlignedHeight >> 0);
	imageBlit.dstOffsets[1].z = 1;

	vkCmdBlitImage(layoutCmd, 
		importedTexture->image_, VK_IMAGE_LAYOUT_GENERAL,
		mSourceAligned.image, VK_IMAGE_LAYOUT_GENERAL,
		1, &imageBlit,
		VK_FILTER_LINEAR);

	/*
	vks::tools::setImageLayout(
		layoutCmd, importedTexture->image_, VK_IMAGE_ASPECT_COLOR_BIT, 
		VK_IMAGE_LAYOUT_UNDEFINED, 
		VK_IMAGE_LAYOUT_GENERAL);
	*/
	mVulkanExample->flushCommandBuffer(layoutCmd, mQueue, true);

}

#if 0
	std::vector<VkWriteDescriptorSet> computeWriteDescriptorSets = {			
		vks::initializers::writeDescriptorSet(mDescriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0, descriptor)
	};
	vkUpdateDescriptorSets(mDevice, computeWriteDescriptorSets.size(), computeWriteDescriptorSets.data(), 0, NULL);
#endif

}
void HistogramCompute::setDebugWindow(int enable) {
	if (enable) {
		if (mHistogramGrapher_ == nullptr) {
			mHistogramGrapher_ = HistogramGrapher::create();
		}
	} else {
		if (mHistogramGrapher_ != nullptr) {
			delete mHistogramGrapher_; mHistogramGrapher_ = nullptr;
		}
	}
}
void HistogramCompute::setBHE_factor(float f0, float f1) {
	std::unique_lock<std::mutex> caller_lock(mBHE_factor_mutex_);
	mBHE_factor0 = f0; mBHE_factor1 = f1;
}
void HistogramCompute::setBHE_tuning(float t0, float t1) {
	std::unique_lock<std::mutex> caller_lock(mBHE_factor_mutex_);
	mBHE_tuning0 = t0;
	mBHE_tuning1 = t1;
}



};


