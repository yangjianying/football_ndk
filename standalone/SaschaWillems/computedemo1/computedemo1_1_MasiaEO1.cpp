#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

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
#include "computedemo1_1_MasiaEO1.h"

// Android log function wrappers
static const char* kTAG = "computedemo1_1";
#include "utils/android_logcat_.h"

#undef __CLASS__
#define __CLASS__ "MasiaEO1"


namespace computedemo1_1 {


static const char *shader_proc_L = 
	"#version 450 \n"
	"precision highp float; \n"
	"precision highp int;\n"
	"layout (local_size_x = 16, local_size_y = 16) in; \n"
	"layout (binding = 0, rgba8) uniform readonly image2D inputImage; \n"
	//"layout (binding = 0, rgba32f) uniform readonly image2D inputImage; \n"
	"layout (binding = 1, r8) uniform imageBuffer L_buffer; \n" // access this buffer is too slow , and the data is not correct !!!
	//"layout (binding = 1, r32f) uniform imageBuffer L_buffer; \n"// access this buffer is too slow , and the data is not correct !!!
	"layout (binding = 2) buffer StatisticOut \n"
	"{ \n"
	"	uint imNumPixels;  \n"
	"	uint constNumOfPixels; \n"
	"	float constMaxLuminance; \n"
	"	float p_ov_factor;  \n"
	"	uint p_ov_NumPixels; \n"
	"	uint histoBinCount[" __stringify(MasiaEO1_statistic_buffer_c_BIN_SIZE) "]; \n"
	"\n"
	"	float delta_;  \n"
	"	float log_delta_L_div_numPixels_sum_magnify;  \n"
	"	int log_delta_L_div_numPixels_sum; \n"
	"\n"
	"	uint cal_counter; \n"
	"	uint histoBinAccCount[" __stringify(MasiaEO1_statistic_buffer_c_BIN_SIZE) "]; \n"
	"	float histoBinAccNormal[" __stringify(MasiaEO1_statistic_buffer_c_BIN_SIZE) "]; \n"
	"\n"
	"	float percentile_for_MaxQuart; \n"
	"	uint iMinL;  \n"
	"	uint iMaxL;  \n"
	"	float fMinL; \n"
	"	float fMaxL; \n"
	"	float p_ov; \n"
	"	float Lav; \n"
	"	float key; \n"
	"	float m_gamma; \n"
	"} statisticOut; \n"
	"layout (binding = 3, rgba8) uniform imageBuffer HDR_image; \n"// access this buffer is too slow , and the data is not correct !!!
	//"layout (binding = 3, rgba32f) uniform imageBuffer HDR_image; \n" // access this buffer is too slow , and the data is not correct !!!
	"\n"
	//"layout (binding = 4, rgba32f) uniform image2D resultImage; \n"
	"layout (binding = 4, rgba8) uniform image2D resultImage; \n"
	"\n"
	//"layout (binding = 5, r32f) uniform image2D L_image; \n"
	"layout (binding = 5, rg32f) uniform image2D L_image; \n"
	"\n"
	"const vec3 W1 = vec3(0.2125, 0.7154, 0.0721); \n"
	"const vec3 W2 = vec3(0.299, 0.587, 0.114); \n"
	"const vec3 W_BT709 = vec3(0.2126, 0.7152, 0.0722); \n"
	"void main() \n"
	"{							\n"
//	"	atomicAdd(statisticOut.imNumPixels, 1);\n"
	"	vec3 rgb = imageLoad(inputImage, ivec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y)).rgb; \n"
	"\n"
	"	rgb.r = pow(rgb.r, 2.2); \n"
	"	rgb.g = pow(rgb.g, 2.2); \n"
	"	rgb.b = pow(rgb.b, 2.2); \n"
	//"	float luminance = dot(rgb.rgb, W1); \n"
	"	float luminance = dot(rgb.rgb, W_BT709); \n"
	"\n"
#if 0
	//"	int buffer_index_ = int(gl_GlobalInvocationID.x * gl_GlobalInvocationID.y * gl_GlobalInvocationID.z); \n"
	"	int buffer_index_ = int(gl_GlobalInvocationID.x * gl_GlobalInvocationID.y ); \n"
	//"	int buffer_index_ = int(( gl_GlobalInvocationID.x * gl_GlobalInvocationID.y) / 1); \n"
	//"	int buffer_index_ = int(( 1087 ) / 1); \n"
#endif
#if 0
	"\n"
	//"	imageStore(L_buffer, buffer_index_, vec4(luminance, 1.2, 1.3, 1.4)); \n"  // written data is broken
	"	imageStore(L_image, ivec2(gl_GlobalInvocationID.xy), vec4(luminance, luminance, 1.3, 1.4)); \r\n"
#endif
	"\n"
#if 1
	"	if (luminance >= statisticOut.p_ov_factor) { atomicAdd(statisticOut.p_ov_NumPixels, 1); } \n"
#endif
	"\n"
#if 1
	//"	float img_delta_mean_1 = (log(luminance + statisticOut.delta_) * statisticOut.log_delta_L_div_numPixels_sum_magnify) \n"
	//"			/float(statisticOut.constNumOfPixels); \n"
	"	float img_delta_mean_1 = log(clamp(luminance, statisticOut.delta_, 1.0)) * statisticOut.log_delta_L_div_numPixels_sum_magnify; \n"
	"	atomicAdd(statisticOut.log_delta_L_div_numPixels_sum, int(img_delta_mean_1)); \n"
	"\n"
	"	float luminance_range = luminance * (" __stringify(MasiaEO1_statistic_buffer_c_BIN_SIZE) ".0 - 1.0); \n"
	"	int gray_index = int(luminance_range + 0.5); \n"
	"	gray_index = clamp(gray_index, 0, " __stringify(MasiaEO1_statistic_buffer_c_BIN_SIZE) " - 1); \n"
	"	atomicAdd(statisticOut.histoBinCount[gray_index], 1);\n"
	"\n"
#endif
#if 0
	//"	rgb.x += 0.5; rgb.y += 0.5; rgb.z += 0.5; \n"
	"	vec4 res = vec4(rgb, 1.0); \n"
	//"	vec4 res = vec4(0.99, 0.05, 0.06, 1.0); \n"
	"	imageStore(resultImage, ivec2(gl_GlobalInvocationID.xy), res); \n"
#endif
	"\n"
	//"	imageStore(HDR_image, gl_PrimitiveID, res); \n" // error: 'gl_PrimitiveID' : undeclared identifier
	//"	imageStore(HDR_image, buffer_index_, res); \n" // written data is broken
	"\n"
	"}\n"
"";
static const char *shader_gen_gamma = " \n"
	"#version 450 \n"
	"precision highp float; \n"
	"precision highp int;\n"
	"layout (local_size_x = 1, local_size_y = 1) in; \n"
	"layout (binding = 0, rgba8) uniform readonly image2D inputImage; \n"
	//"layout (binding = 0, rgba32f) uniform readonly image2D inputImage; \n"
	"layout (binding = 1, r8) uniform imageBuffer L_buffer; \n" // access this buffer is too slow , and the data is not correct !!!
	//"layout (binding = 1, r32f) uniform imageBuffer L_buffer; \n"// access this buffer is too slow , and the data is not correct !!!
	"layout (binding = 2) buffer StatisticOut \n"
	"{ \n"
	"	uint imNumPixels;  \n"
	"	uint constNumOfPixels; \n"
	"	float constMaxLuminance; \n"
	"	float p_ov_factor;  \n"
	"	uint p_ov_NumPixels; \n"
	"	uint histoBinCount[" __stringify(MasiaEO1_statistic_buffer_c_BIN_SIZE) "]; \n"
	"\n"
	"	float delta_;  \n"
	"	float log_delta_L_div_numPixels_sum_magnify;  \n"
	"	int log_delta_L_div_numPixels_sum; \n"
	"\n"
	"	uint cal_counter; \n"
	"	uint histoBinAccCount[" __stringify(MasiaEO1_statistic_buffer_c_BIN_SIZE) "]; \n"
	"	float histoBinAccNormal[" __stringify(MasiaEO1_statistic_buffer_c_BIN_SIZE) "]; \n"
	"\n"
	"	float percentile_for_MaxQuart; \n"
	"	uint iMinL;  \n"
	"	uint iMaxL;  \n"
	"	float fMinL; \n"
	"	float fMaxL; \n"
	"	float p_ov; \n"
	"	float Lav; \n"
	"	float key; \n"
	"	float m_gamma; \n"
	"} statisticOut; \n"
	"layout (binding = 3, rgba8) uniform imageBuffer HDR_image; \n"// access this buffer is too slow , and the data is not correct !!!
	//"layout (binding = 3, rgba32f) uniform imageBuffer HDR_image; \n" // access this buffer is too slow , and the data is not correct !!!
	"\n"
	//"layout (binding = 4, rgba32f) uniform image2D resultImage; \n"
	"layout (binding = 4, rgba8) uniform image2D resultImage; \n"
	"\n"
	//"layout (binding = 5, r32f) uniform image2D L_image; \n"
	"layout (binding = 5, rg32f) uniform image2D L_image; \n"
	"\n"
	"const vec3 W1 = vec3(0.2125, 0.7154, 0.0721); \n"
	"const vec3 W2 = vec3(0.299, 0.587, 0.114); \n"
	"\n"
	"uint getAccHist(uint level) { \n"
	"	uint acc = 0;"
	"	for (uint i=0; i < " __stringify(MasiaEO1_statistic_buffer_c_BIN_SIZE) " && i <= level; i ++) { \n"
	"		acc += statisticOut.histoBinCount[i]; \n"
	"	} \n"
	"	return acc; \n"
	"} \n"
	"\n"
	"void main() \n"
	"{							\n"
	"	float normal_acc = float(getAccHist(gl_GlobalInvocationID.x)) / float(statisticOut.constNumOfPixels); \n"
	"	statisticOut.histoBinAccNormal[gl_GlobalInvocationID.x] = normal_acc; \n"
	"\n"
	"	atomicAdd(statisticOut.cal_counter, 1); \n"
	"\n"
	"	// all thread is finished , then do the last task here : \n"
	"	if (statisticOut.cal_counter == (gl_NumWorkGroups.x *gl_NumWorkGroups.y) * (gl_WorkGroupSize.x * gl_WorkGroupSize.y)) { \n"
	"		statisticOut.p_ov = float((statisticOut.p_ov_NumPixels*100)/statisticOut.constNumOfPixels); \n"
	"		\n"
	"		float Lav_1 = float(statisticOut.log_delta_L_div_numPixels_sum/int(statisticOut.log_delta_L_div_numPixels_sum_magnify)) "
	"												/ float(statisticOut.constNumOfPixels); \n"
	"		statisticOut.Lav = exp(Lav_1); \n"
	"		\n"
	"		int found_0 = 0; \n"
	"		int found_1 = 0; \n"
	"		float percentile = statisticOut.percentile_for_MaxQuart; \n"
	"					percentile = clamp(percentile, 0.0001, 1.0); \n"
	"		float percentle_max = 1.0 - percentile; \n"
	"		float normal_acc_cur = 0.0; \n"
	"		uint j = 0; \n"
	"		for(j=0; j<" __stringify(MasiaEO1_statistic_buffer_c_BIN_SIZE) "; j++) { \n"
	"			normal_acc_cur = statisticOut.histoBinAccNormal[j]; \n"
	"			if(found_0 == 0) {\n"
	"				if(normal_acc_cur >= percentile) { \n"
	"					found_0 = 1; statisticOut.iMinL = j; \n"
	"				} \n"
	"			}\n"
	"			if(found_1 == 0) {\n"
	"				if(normal_acc_cur >= percentle_max) { \n"
	"					found_1 = 1; statisticOut.iMaxL = j; \n"
	"				} \n"
	"			}\n"
	"		}\n"
	"		statisticOut.fMinL = float(statisticOut.iMinL + 1)/float(" __stringify(MasiaEO1_statistic_buffer_c_BIN_SIZE) ".0 ); \n"
	"		statisticOut.fMaxL = float(statisticOut.iMaxL + 1)/float(" __stringify(MasiaEO1_statistic_buffer_c_BIN_SIZE) ".0 ); \n"
	"\n"
	"		statisticOut.key = (log(statisticOut.Lav) - log(statisticOut.fMinL)) / (log(statisticOut.fMaxL) - log(statisticOut.fMinL)); \n"
	"\n"
	"		float gamma_ = 2.4379 + 0.2319 * log(statisticOut.Lav) - 1.1228 * statisticOut.key + 0.0085 * statisticOut.p_ov; \n"
	"		gamma_ = clamp(gamma_, 1.0, gamma_); \n"
	"		statisticOut.m_gamma = gamma_; \n"
	"\n"
	"	} \n"
	"}\n"
"";
static const char *shader_gen_hdr = " \n"
	"#version 450 \n"
	"precision highp float; \n"
	"precision highp int;\n"
	"layout (local_size_x = 16, local_size_y = 16) in; \n"
	"layout (binding = 0, rgba8) uniform readonly image2D inputImage; \n"
	//"layout (binding = 0, rgba32f) uniform readonly image2D inputImage; \n"
	"layout (binding = 1, r8) uniform imageBuffer L_buffer; \n" // access this buffer is too slow , and the data is not correct !!!
	//"layout (binding = 1, r32f) uniform imageBuffer L_buffer; \n"// access this buffer is too slow , and the data is not correct !!!
	"layout (binding = 2) buffer StatisticOut \n"
	"{ \n"
	"	uint imNumPixels;  \n"
	"	uint constNumOfPixels; \n"
	"	float constMaxLuminance; \n"
	"	float p_ov_factor;  \n"
	"	uint p_ov_NumPixels; \n"
	"	uint histoBinCount[" __stringify(MasiaEO1_statistic_buffer_c_BIN_SIZE) "]; \n"
	"\n"
	"	float delta_;  \n"
	"	float log_delta_L_div_numPixels_sum_magnify;  \n"
	"	int log_delta_L_div_numPixels_sum; \n"
	"\n"
	"	uint cal_counter; \n"
	"	uint histoBinAccCount[" __stringify(MasiaEO1_statistic_buffer_c_BIN_SIZE) "]; \n"
	"	float histoBinAccNormal[" __stringify(MasiaEO1_statistic_buffer_c_BIN_SIZE) "]; \n"
	"\n"
	"	float percentile_for_MaxQuart; \n"
	"	uint iMinL;  \n"
	"	uint iMaxL;  \n"
	"	float fMinL; \n"
	"	float fMaxL; \n"
	"	float p_ov; \n"
	"	float Lav; \n"
	"	float key; \n"
	"	float m_gamma; \n"
	"} statisticOut; \n"
	"layout (binding = 3, rgba8) uniform imageBuffer HDR_image; \n"// access this buffer is too slow , and the data is not correct !!!
	//"layout (binding = 3, rgba32f) uniform imageBuffer HDR_image; \n" // access this buffer is too slow , and the data is not correct !!!
	"\n"
	//"layout (binding = 4, rgba32f) uniform image2D resultImage; \n"
	"layout (binding = 4, rgba8) uniform image2D resultImage; \n"
	"\n"
	//"layout (binding = 5, r32f) uniform image2D L_image; \n"
	"layout (binding = 5, rg32f) uniform image2D L_image; \n"
	"\n"
	"const vec3 W1 = vec3(0.2125, 0.7154, 0.0721); \n"
	"const vec3 W2 = vec3(0.299, 0.587, 0.114); \n"
	"\n"
	"const vec3 W_BT709 = vec3(0.2126, 0.7152, 0.0722); \n"
	"const vec3 W_BT2446 = vec3(0.2627, 0.6780, 0.0593); \n"
	"const float SIGMA = 0.000001; \n"
	"void main() \n"
	"{							\n"
	"	vec3 rgb = imageLoad(inputImage, ivec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y)).rgb; \n"
	"	rgb.r = pow(rgb.r, 2.2); \n"
	"	rgb.g = pow(rgb.g, 2.2); \n"
	"	rgb.b = pow(rgb.b, 2.2); \n"
	"	float luminance = dot(rgb.rgb, W_BT709); \n"
	"	\n"
	"	float gamma_ = statisticOut.m_gamma; \n"
	//"	gamma_ = 2.0; \n"
	"	float ratio = 1.0; \n"
	//"	ratio = (pow(luminance, gamma_) * statisticOut.constMaxLuminance) / luminance; \n"
	"	ratio = ( pow(luminance, gamma_) ) / luminance; \n"
	"	rgb = rgb*ratio; \n"
#if 0
	"	imageStore(resultImage, ivec2(gl_GlobalInvocationID.xy), vec4(rgb, 1.0)); \n"
#else
	"	\n"
	"	rgb = clamp(rgb, SIGMA, 1.0); \n"
	"	rgb.r = pow(rgb.r, 1.0/2.4); \n"
	"	rgb.g = pow(rgb.g, 1.0/2.4); \n"
	"	rgb.b = pow(rgb.b, 1.0/2.4); \n"
	"	\n"
	"	float YSS = dot(rgb.rgb, W_BT2446); \n"
	"	float roHDR = 1.0 + 32.0 * pow(0.1, 1.0/2.4); \n"
	"	float YpSS = log(1.0 + (roHDR - 1.0) * YSS) / log(roHDR); "
	"	\n"
	"	float YcSS = YpSS; \n"
	"	if (YpSS < 0.0) { YpSS = 0.0; } \n"
	"\n"
	//"	if(YpSS >= 0.0 && YpSS <= 0.7399) {\n"
	"	if(YpSS <= 0.7399) {\n"
	"		YcSS = 1.0770 * YpSS; \n"
	//"	} else if(YpSS > 0.7399 && YpSS < 0.9909) { \n"
	"	} else if(YpSS < 0.9909) { \n"
	"		YcSS = -1.1510 * pow(YpSS, 2.0) + 2.7811 * YpSS - 0.6302; \n"
	"	} else \n"
	//"	if(YpSS >= 0.9909) \n"
	"	{ \n"
	"		YcSS = 0.5 * YpSS + 0.5; \n"
	"	} \n"
	"\n"
	"	float roSDR = 1.0 + 32.0 * pow(0.01, 1.0/2.4); \n"
	"	float YsdrSS = (pow(roSDR, YcSS) - 1.0) / (roSDR - 1.0); \n"
	"\n"
	"	float BSS = rgb.b; \n"
	"	float RSS = rgb.r; \n"
	"	float CbTMOSS = (YsdrSS / (1.1*YSS)) * ((BSS - YSS) / 1.8814); \n"
	"	float CrTMOSS = (YsdrSS / (1.1 * YSS)) * ((RSS - YSS) / 1.4746); \n"
	"	float YtmoSS = YsdrSS - max(0.1 * CrTMOSS, 0.0); \n"
	"	float B = 1.8814 * CbTMOSS + YtmoSS; \n"
	"	float R = 1.4746 * CrTMOSS + YtmoSS; \n"
	"	float G = (YtmoSS - 0.2627 * R - 0.0593 * B) / 0.6780; \n"
	"	rgb.r = R; \n"
	"	rgb.g = G; \n"
	"	rgb.b = B; \n"
	"	rgb = clamp(rgb, 0.0, 1.0); \n"
	"	imageStore(resultImage, ivec2(gl_GlobalInvocationID.xy), vec4(rgb, 1.0)); \n"
	"	\n"
	"\n"
#endif
	"}\n"
"";


MasiaEO1::MasiaEO1(VulkanExample *vulkanExample)
	:VulkanExample_Parasite(vulkanExample)    {
	DLOGD("%s created ! \r\n", __func__);

{
	Shader_ shader_proc_L_ = {COMP_PROC_L, "shader_proc_L", shader_proc_L};
	mShaders.insert(COMP_PROC_L, shader_proc_L_);
}
{
	Shader_ shader_gen_gamma_ = {COMP_GEN_GAMMA, "shader_gen_gamma", shader_gen_gamma};
	mShaders.insert(COMP_GEN_GAMMA, shader_gen_gamma_);
}
{
	Shader_ shader_gen_hdr_ = {COMP_GEN_HDR, "shader_gen_hdr", shader_gen_hdr};
	mShaders.insert(COMP_GEN_HDR, shader_gen_hdr_);
}

}
MasiaEO1::~MasiaEO1() {
	DLOGD("%s destroying ... \r\n", __func__);

	{
		VkPipeline pipeline_ = VK_NULL_HANDLE;
		mPipelines.next_begin();
		while(mPipelines.next(nullptr, &pipeline_) == 0) {
			vkDestroyPipeline(mDevice, pipeline_, nullptr);
		}
	}

	vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(mDevice, mDescriptorSetLayout, nullptr);
	if (mDescriptorPool != VK_NULL_HANDLE) {
		vkDestroyDescriptorPool(mDevice, mDescriptorPool, nullptr);
	}

	vkDestroyCommandPool(mDevice, mCommandPool, nullptr);
	vkDestroyFence(mDevice, mFence, nullptr);
	vkDestroyFence(mDevice, mFence_draw, nullptr);

	mLImage.destroy();
	mTextureTarget.destroy();
	
	vkDestroyBufferView(mDevice, mSourceAlignedBufferView, nullptr);
	mSourceAlignedBuffer.destroy();

	vkDestroyBufferView(mDevice, mLbuffer_bufferView, nullptr);
	mLbuffer.destroy();

	mStatisticBuffer.destroy();

	mSourceScaled.destroy();
	mSourceAligned.destroy();
	


	DLOGD("%s destroying done \r\n", __func__);
}

void MasiaEO1::getComputeQueue() {
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

	DLOGD( "* %s, compute.queueFamilyIndex = %d, queue=%p \r\n", __func__, mQueueFamilyIndex, mQueue);
}

void MasiaEO1::createTextureTarget(
	vks::Texture *tex, uint32_t w_, uint32_t h_, VkFormat format, VkImageUsageFlags extra_flags) {

	uint32_t width = w_;
	uint32_t height = h_;

	vks::VulkanDevice *vulkanDevice = mVulkanExample->vulkanDevice;

	VkFormatProperties formatProperties;

	// Get device properties for the requested texture format
	vkGetPhysicalDeviceFormatProperties(mPhysicalDevice, format, &formatProperties);
	// Check if requested image format supports image storage operations
	assert(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT);
#if 0
{
	DLOGD( "%s format 0x%08x(%d) \r\n" , __func__, format, format);
	DLOGD( "	 linearTilingFeatures:\r\n");
	::vks::android::print_VkFormatFeatureFlags(formatProperties.linearTilingFeatures);
	DLOGD( "	 optimalTilingFeatures:\r\n");
	::vks::android::print_VkFormatFeatureFlags(formatProperties.optimalTilingFeatures);
	DLOGD( "	 bufferFeatures:\r\n");
	::vks::android::print_VkFormatFeatureFlags(formatProperties.bufferFeatures);
}
#endif
	// Prepare blit target texture
	tex->width = width;
	tex->height = height;
	DLOGD( "%s, size = %4d x %4d \r\n", __func__, width, height);

	VkImageCreateInfo imageCreateInfo = vks::initializers::imageCreateInfo();
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = format;
	imageCreateInfo.extent = { width, height, 1 };
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	// Image will be sampled in the fragment shader and used as storage target in the compute shader
	imageCreateInfo.usage = extra_flags;
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

void MasiaEO1::createDescriptorPool() {
	std::vector<VkDescriptorPoolSize> poolSizes = {
		vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 3),
		vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 6),
		vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3)
	};
	VkDescriptorPoolCreateInfo descriptorPoolInfo = vks::initializers::descriptorPoolCreateInfo(
			static_cast<uint32_t>(poolSizes.size()), poolSizes.data(), 3);

	VK_CHECK_RESULT(vkCreateDescriptorPool(mDevice, &descriptorPoolInfo, nullptr, &mDescriptorPool));
	assert(mDescriptorPool != VK_NULL_HANDLE);
}

void MasiaEO1::buildCommandBuffers() {
	// mSourceAligned is ready
	// mSourceScaled is ready
	// mSourceAlignedBuffer is ready
/**
* 1, using mSourceAligned to calculate [key, Lav], p_ov -> m_gamma
* 1, using mSourceScaled to calculate [key, Lav], p_ov -> m_gamma
* 2, using m_gamma , to adjust the mSourceAligned'content(0 ~ 1.0) , write the result into mSourceAlignedBuffer (0 ~ 1000.0)
*/
	// Flush the queue if we're rebuilding the command buffer after a pipeline change to ensure it's not currently in use
	vkQueueWaitIdle(mQueue);

	VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();
	
	VK_CHECK_RESULT(vkBeginCommandBuffer(mCommandBuffer, &cmdBufInfo));
	
	//
	vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, mPipelines.getAssert(COMP_PROC_L));
	vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, mPipelineLayout, 0, 1, &mDescriptorSet, 0, 0);
	vkCmdDispatch(mCommandBuffer, mSourceAlignedSize.w / 16, mSourceAlignedSize.h / 16, 1);

	vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, mPipelines.getAssert(COMP_GEN_GAMMA));
	vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, mPipelineLayout, 0, 1, &mDescriptorSet, 0, 0);
	vkCmdDispatch(mCommandBuffer, MasiaEO1_statistic_buffer_c_BIN_SIZE, 1, 1);

	vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, mPipelines.getAssert(COMP_GEN_HDR));
	vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, mPipelineLayout, 0, 1, &mDescriptorSet, 0, 0);
	vkCmdDispatch(mCommandBuffer, mSourceAlignedSize.w / 16, mSourceAlignedSize.h / 16, 1);

#if 0
	{ // copy buffer to image
	
		VkBufferImageCopy region = {};
	
		region.bufferOffset = 0;
		region.bufferRowLength = 0; // mSourceAlignedBufferSize.w * sizeof(_buffer_item_t); // this is wrong !!!
		region.bufferImageHeight = 0; // mSourceAlignedBufferSize.h;
	
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
	
		region.imageOffset = {0,0,0};
		region.imageExtent = {
				.width = static_cast<uint32_t>(mTextureTargetSize.w),
				.height = static_cast<uint32_t>(mTextureTargetSize.h),
				.depth = 1,
			};
	
	   vks::tools::setImageLayout(mCommandBuffer, mTextureTarget.image,
		   VK_IMAGE_ASPECT_COLOR_BIT, 
		   VK_IMAGE_LAYOUT_GENERAL, 
		   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	
		vkCmdCopyBufferToImage(mCommandBuffer, mSourceAlignedBuffer.buffer, mTextureTarget.image,
							   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,1,&region);
	
		vks::tools::setImageLayout(mCommandBuffer, mTextureTarget.image,
			VK_IMAGE_ASPECT_COLOR_BIT, 
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	
	}
#endif

	vkEndCommandBuffer(mCommandBuffer);
}

// external 
void MasiaEO1::prepare() {
	vks::VulkanDevice *vulkanDevice = mVulkanExample->vulkanDevice;

	uint32_t w_ = 1080;
	uint32_t h_ = 2340;
	mSourceTextureSize.w = w_;
	mSourceTextureSize.h = h_;

	getComputeQueue();

	VkFormat format;

	// aligned source
	mSourceAlignedSize.w = ALIGN_SIZE(w_, 16);
	mSourceAlignedSize.h = ALIGN_SIZE(h_, 16);	// will be 1088 x 2352
	DLOGD("mSourceAlignedSize = %4d x %4d \r\n", mSourceAlignedSize.w, mSourceAlignedSize.h);
	format = VK_FORMAT_R8G8B8A8_UNORM;
	//format = VK_FORMAT_R32G32B32A32_SFLOAT;
	createTextureTarget(&mSourceAligned, mSourceAlignedSize.w, mSourceAlignedSize.h, format, 
		VK_IMAGE_USAGE_SAMPLED_BIT
			| VK_IMAGE_USAGE_STORAGE_BIT		// used by compute shader to read/write
			| VK_IMAGE_USAGE_TRANSFER_SRC_BIT
			| VK_IMAGE_USAGE_TRANSFER_DST_BIT
		);

	// scaled aligned source
	mSourceScaledSize.w = ALIGN_SIZE(w_/4, 16);
	mSourceScaledSize.h = ALIGN_SIZE(h_/4, 16);
	DLOGD("mSourceScaledSize = %4d x %4d \r\n", mSourceScaledSize.w, mSourceScaledSize.h);
	format = VK_FORMAT_R8G8B8A8_UNORM;
	//format = VK_FORMAT_R32G32B32A32_SFLOAT;
	createTextureTarget(&mSourceScaled, mSourceScaledSize.w, mSourceScaledSize.h, format, 
		VK_IMAGE_USAGE_SAMPLED_BIT
			| VK_IMAGE_USAGE_STORAGE_BIT
			| VK_IMAGE_USAGE_TRANSFER_SRC_BIT
			| VK_IMAGE_USAGE_TRANSFER_DST_BIT
		);

	// HDR output buffer
	buffer_memory_property_ = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	buffer_memory_property_ = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	
	mSourceAlignedBufferSize.w = ALIGN_SIZE(mSourceAlignedSize.w, 256);
	mSourceAlignedBufferSize.w = ALIGN_SIZE(mSourceAlignedSize.w, 16);
	mSourceAlignedBufferSize.h = mSourceAlignedSize.h;
	DLOGD("mSourceAlignedBufferSize = %4d x %4d \r\n", mSourceAlignedBufferSize.w, mSourceAlignedBufferSize.h);
	mSourceAlignedBuffer_size = (mSourceAlignedBufferSize.w + 128)* mSourceAlignedBufferSize.h * sizeof(_buffer_item_t);
												// sizeof(_buffer_item_t) == VK_FORMAT_R32G32B32A32_SFLOAT
	VK_CHECK_RESULT(vulkanDevice->createBuffer(
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT		// 
			| VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT	//
			| VK_BUFFER_USAGE_TRANSFER_SRC_BIT
			| VK_BUFFER_USAGE_TRANSFER_DST_BIT
		,
		buffer_memory_property_,
		&mSourceAlignedBuffer, mSourceAlignedBuffer_size));
	if (buffer_memory_property_ != VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
		// Map for host access
		VK_CHECK_RESULT(mSourceAlignedBuffer.map());
	}
	{
		VkBufferViewCreateInfo bvci{};
		bvci.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
		bvci.pNext = nullptr;
		bvci.flags = 0;
		bvci.buffer = mSourceAlignedBuffer.buffer;
		bvci.format = VK_FORMAT_R32G32B32A32_SFLOAT;
		//bvci.format = VK_FORMAT_R8G8B8A8_UNORM;
		bvci.offset = 0;
		bvci.range = VK_WHOLE_SIZE;
		VK_CHECK_RESULT(vkCreateBufferView(mDevice, &bvci, nullptr, &mSourceAlignedBufferView));
		DLOGD("mSourceAlignedBufferView %s \r\n", (mSourceAlignedBufferView != VK_NULL_HANDLE ? " != VK_NULL_HANDLE " : " == VK_NULL_HANDLE "));
	}

	// L buffer
	Lbuffer_memory_property_ = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	//Lbuffer_memory_property_ = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	mLbufferSize.w = ALIGN_SIZE(mSourceAlignedSize.w, 16);
	mLbufferSize.h = mSourceAlignedSize.h;
	DLOGD("mLbufferSize = %4d x %4d \r\n", mLbufferSize.w, mLbufferSize.h);
	mLbuffer_memory_size = (mLbufferSize.w + 128)* mLbufferSize.h * sizeof(float)*1;
												// sizeof(float)*1 == VK_FORMAT_R32_SFLOAT
	VK_CHECK_RESULT(vulkanDevice->createBuffer(
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT		// 
			| VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT	//
			| VK_BUFFER_USAGE_TRANSFER_SRC_BIT
			| VK_BUFFER_USAGE_TRANSFER_DST_BIT
		,
		Lbuffer_memory_property_,
		&mLbuffer, mLbuffer_memory_size));
	if (Lbuffer_memory_property_ != VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
		// Map for host access
		VK_CHECK_RESULT(mLbuffer.map());
	}
	{
		VkBufferViewCreateInfo bvci{};
		bvci.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
		bvci.pNext = nullptr;
		bvci.flags = 0;
		bvci.buffer = mLbuffer.buffer;
		//bvci.format = VK_FORMAT_R32_SFLOAT;
		bvci.format = VK_FORMAT_R8_UNORM;
		bvci.offset = 0;
		bvci.range = VK_WHOLE_SIZE;
		VK_CHECK_RESULT(vkCreateBufferView(mDevice, &bvci, nullptr, &mLbuffer_bufferView));
		DLOGD("mLbuffer_bufferView %s \r\n", (mLbuffer_bufferView != VK_NULL_HANDLE ? " != VK_NULL_HANDLE " : " == VK_NULL_HANDLE "));
	}
	
	// statistic buffer
	//mStatisticBuffer_memory_property_ = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	mStatisticBuffer_memory_property_ = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
									// cpu clear this buffer each time
	DLOGD("mStatisticBuffer size = %d \r\n", sizeof(mStatisticHostBuffer));
	VK_CHECK_RESULT(vulkanDevice->createBuffer(
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT		// 
			| VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT	//
			| VK_BUFFER_USAGE_TRANSFER_SRC_BIT
			| VK_BUFFER_USAGE_TRANSFER_DST_BIT
		,
		mStatisticBuffer_memory_property_,
		&mStatisticBuffer, sizeof(mStatisticHostBuffer)));
	if (mStatisticBuffer_memory_property_ != VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
		// Map for host access
		VK_CHECK_RESULT(mStatisticBuffer.map());
	}

	// target color image
	mTextureTargetSize = {.w = 640, .h = 480, };
	mTextureTargetSize.w= ALIGN_SIZE(w_, 16);
	mTextureTargetSize.h= ALIGN_SIZE(h_, 16);
	DLOGD("mTextureTargetSize = %4d x %4d \r\n", mTextureTargetSize.w, mTextureTargetSize.h);
	format = VK_FORMAT_R8G8B8A8_UNORM;
	//format = VK_FORMAT_R32G32B32A32_SFLOAT;
	createTextureTarget(&mTextureTarget, mTextureTargetSize.w, mTextureTargetSize.h, format,
		VK_IMAGE_USAGE_SAMPLED_BIT
			| VK_IMAGE_USAGE_STORAGE_BIT
			| VK_IMAGE_USAGE_TRANSFER_SRC_BIT
			| VK_IMAGE_USAGE_TRANSFER_DST_BIT
		);

	// target L image
	mLImageSize_ = {.w = 640, .h = 480, };
	mLImageSize_.w= ALIGN_SIZE(w_, 16);
	mLImageSize_.h= ALIGN_SIZE(h_, 16);
	DLOGD("mLImageSize_ = %4d x %4d \r\n", mLImageSize_.w, mLImageSize_.h);
	format = VK_FORMAT_R8G8B8A8_UNORM;
	format = VK_FORMAT_R32G32B32A32_SFLOAT;
	format = VK_FORMAT_R32_SFLOAT;
	format = VK_FORMAT_R32G32_SFLOAT;
	createTextureTarget(&mLImage, mLImageSize_.w, mLImageSize_.h, format,
		VK_IMAGE_USAGE_SAMPLED_BIT
			| VK_IMAGE_USAGE_STORAGE_BIT
			| VK_IMAGE_USAGE_TRANSFER_SRC_BIT
			| VK_IMAGE_USAGE_TRANSFER_DST_BIT
		);

	//
	createDescriptorPool();
	DLOGD("createDescriptorPool done line=%d \r\n", __LINE__);

	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
		// Binding 0: Input image (read-only)
		vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 0),
		// Binding 1: Output L texel buffer (output)
		vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1),
		// Binding 2: output statistic buffer (output)
		vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,VK_SHADER_STAGE_COMPUTE_BIT, 2),
		// Binding 3: output HDR texel buffer (output)
		vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,VK_SHADER_STAGE_COMPUTE_BIT, 3),
		vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,VK_SHADER_STAGE_COMPUTE_BIT, 4),
		vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,VK_SHADER_STAGE_COMPUTE_BIT, 5),
	};
	VkDescriptorSetLayoutCreateInfo descriptorLayout = vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings);
	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(mDevice, &descriptorLayout, nullptr, &mDescriptorSetLayout));
	assert(mDescriptorSetLayout != VK_NULL_HANDLE);

{
	VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
		vks::initializers::pipelineLayoutCreateInfo(&mDescriptorSetLayout, 1);
	VK_CHECK_RESULT(vkCreatePipelineLayout(mDevice, &pPipelineLayoutCreateInfo, nullptr, &mPipelineLayout));
	assert(mPipelineLayout != VK_NULL_HANDLE);
}

	VkDescriptorSetAllocateInfo allocInfo =
		vks::initializers::descriptorSetAllocateInfo(mDescriptorPool, &mDescriptorSetLayout, 1);
	VK_CHECK_RESULT(vkAllocateDescriptorSets(mDevice, &allocInfo, &mDescriptorSet));
	assert(mDescriptorSet != VK_NULL_HANDLE);

	std::vector<VkWriteDescriptorSet> computeWriteDescriptorSets = {
		// Binding 0: Input image (read-only)
		vks::initializers::writeDescriptorSet(mDescriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0, &mSourceAligned.descriptor),
		// Binding 1: Output L texel buffer (output)
		vks::initializers::writeDescriptorSet(mDescriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, &mLbuffer_bufferView),
		// Binding 2: statistic buffer,  // Atomic counter (written in shader)
		vks::initializers::writeDescriptorSet(mDescriptorSet,VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2, &mStatisticBuffer.descriptor),
		// Binding 3: for HDR output , a texel buffer (output)
		vks::initializers::writeDescriptorSet(mDescriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 3, &mSourceAlignedBufferView),
		vks::initializers::writeDescriptorSet(mDescriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 4, &mTextureTarget.descriptor),
		vks::initializers::writeDescriptorSet(mDescriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 5, &mLImage.descriptor),
	};
	vkUpdateDescriptorSets(mDevice, computeWriteDescriptorSets.size(), computeWriteDescriptorSets.data(), 0, NULL);

	DLOGD("prepare descriptor sets done line=%d \r\n", __LINE__);
	///////////////////////// create pipeline
#if 1
{
	// Create compute shader pipeline
	VkComputePipelineCreateInfo computePipelineCreateInfo =
		vks::initializers::computePipelineCreateInfo(mPipelineLayout, 0);

	Shader_ shader_ = mShaders.getAssert(COMP_PROC_L);
	DLOGD( "compiling : %s \r\n", shader_.name.c_str());
	VkPipelineShaderStageCreateInfo shaderStageCi = 
		mVulkanExample->loadShader_from_strings_c(shader_.source, VK_SHADER_STAGE_COMPUTE_BIT);
	computePipelineCreateInfo.stage = shaderStageCi;
	VkPipeline pipeline;
	VK_CHECK_RESULT(vkCreateComputePipelines(mDevice, mPipelineCache, 1, &computePipelineCreateInfo, nullptr, &pipeline));
	mPipelines.insert(COMP_PROC_L, pipeline);
}
#endif
#if 1
{
	// Create compute shader pipeline
	VkComputePipelineCreateInfo computePipelineCreateInfo =
		vks::initializers::computePipelineCreateInfo(mPipelineLayout, 0);

	Shader_ shader_ = mShaders.getAssert(COMP_GEN_GAMMA);
	DLOGD( "compiling : %s \r\n", shader_.name.c_str());
	VkPipelineShaderStageCreateInfo shaderStageCi = 
		mVulkanExample->loadShader_from_strings_c(shader_.source, VK_SHADER_STAGE_COMPUTE_BIT);
	computePipelineCreateInfo.stage = shaderStageCi;
	VkPipeline pipeline;
	VK_CHECK_RESULT(vkCreateComputePipelines(mDevice, mPipelineCache, 1, &computePipelineCreateInfo, nullptr, &pipeline));
	mPipelines.insert(COMP_GEN_GAMMA, pipeline);
}
#endif
#if 1
{
	// Create compute shader pipeline
	VkComputePipelineCreateInfo computePipelineCreateInfo =
		vks::initializers::computePipelineCreateInfo(mPipelineLayout, 0);

	Shader_ shader_ = mShaders.getAssert(COMP_GEN_HDR);
	DLOGD( "compiling : %s \r\n", shader_.name.c_str());
	VkPipelineShaderStageCreateInfo shaderStageCi = 
		mVulkanExample->loadShader_from_strings_c(shader_.source, VK_SHADER_STAGE_COMPUTE_BIT);
	computePipelineCreateInfo.stage = shaderStageCi;
	VkPipeline pipeline;
	VK_CHECK_RESULT(vkCreateComputePipelines(mDevice, mPipelineCache, 1, &computePipelineCreateInfo, nullptr, &pipeline));
	mPipelines.insert(COMP_GEN_HDR, pipeline);
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

	// create fence
	VkFenceCreateInfo fenceCreateInfo{};

	fenceCreateInfo = vks::initializers::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
	VK_CHECK_RESULT(vkCreateFence(mDevice, &fenceCreateInfo, nullptr, &mFence));

	fenceCreateInfo = vks::initializers::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
	VK_CHECK_RESULT(vkCreateFence(mDevice, &fenceCreateInfo, nullptr, &mFence_draw));

	buildCommandBuffers();

#if 0
	test_VkFormatProperties(VK_FORMAT_R8G8B8A8_UNORM, __stringify(VK_FORMAT_R8G8B8A8_UNORM));
	test_VkFormatProperties(VK_FORMAT_R16G16B16A16_SFLOAT, __stringify(VK_FORMAT_R16G16B16A16_SFLOAT));
	test_VkFormatProperties(VK_FORMAT_R32G32B32A32_SFLOAT, __stringify(VK_FORMAT_R32G32B32A32_SFLOAT));
	test_VkFormatProperties(VK_FORMAT_R64G64B64A64_SFLOAT, __stringify(VK_FORMAT_R64G64B64A64_SFLOAT)); // not supported !!!

	test_find_format_for_FORMAT_FEATURE_(VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT, 
		__stringify(VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT));
	
	test_find_format_for_FORMAT_FEATURE_(VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT, 
		__stringify(VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT));
	test_find_format_for_FORMAT_FEATURE_(VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT, 
		__stringify(VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT));
#endif
}

bool MasiaEO1::isTextureTargetAvailable() {
	return true; // false; // true;
}

vks::Texture2D & MasiaEO1::getTextureTarget() {
/* whatever this texture's format is ,	the parent always sampled the texture successfully !!! 
 * currently , i tried 
 * VK_FORMAT_R8G8B8A8_UNORM
 * VK_FORMAT_R32G32B32A32_SFLOAT
 **/
	//return mSourceAligned;
	//return mSourceScaled;
	//return mLImage;
	return mTextureTarget;
}

void MasiaEO1::appendCommandBuffers(const VkCommandBuffer commandBuffer) {

}

void MasiaEO1::draw() {
	DLOGD("\r\n");
#if 1

	if ((buffer_memory_property_ & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == 0) {
		memset(mSourceAlignedBuffer.mapped, 0, mSourceAlignedBuffer_size);
	}
	if ((Lbuffer_memory_property_ & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == 0) {
		memset(mLbuffer.mapped, 0, mLbuffer_memory_size);
	}

	memset( &mStatisticHostBuffer, 0, sizeof(mStatisticHostBuffer));
	mStatisticHostBuffer.constNumOfPixels = mSourceAlignedSize.w * mSourceAlignedSize.h;
	mStatisticHostBuffer.constMaxLuminance = 1000.0f;
	mStatisticHostBuffer.p_ov_factor = 254.0f/255.0f;
	mStatisticHostBuffer.delta_ = 0.000001f;
	mStatisticHostBuffer.log_delta_L_div_numPixels_sum_magnify = 100000.0f;
	mStatisticHostBuffer.log_delta_L_div_numPixels_sum_magnify = float(mStatisticHostBuffer.constNumOfPixels);
	mStatisticHostBuffer.log_delta_L_div_numPixels_sum_magnify = 100.0f; // 50 // should <= 119
	mStatisticHostBuffer.percentile_for_MaxQuart = 0.01f;

#if 1
	vkWaitForFences(mDevice, 1, &mFence_draw, VK_TRUE, UINT64_MAX);
	vkResetFences(mDevice, 1, &mFence_draw);
	
	// mStatisticBuffer_memory_property_
	memcpy(mStatisticBuffer.mapped, &mStatisticHostBuffer, sizeof(mStatisticHostBuffer));

	// submit
	VkSubmitInfo computeSubmitInfo = vks::initializers::submitInfo();
	computeSubmitInfo.commandBufferCount = 1;
	computeSubmitInfo.pCommandBuffers = &mCommandBuffer;
	VK_CHECK_RESULT(vkQueueSubmit(mQueue, 1, &computeSubmitInfo, mFence_draw));
	vkWaitForFences(mDevice, 1, &mFence_draw, VK_TRUE, UINT64_MAX);
#endif

#if 0
{
	if ((buffer_memory_property_ & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == 0) {
		_buffer_item_t *buf = static_cast<_buffer_item_t *>(mSourceAlignedBuffer.mapped);
		int i=0;
		for(;i<mSourceAlignedBufferSize.w * 300;i++) {
			buf[i].x.r = 1.0f; buf[i].x.g = 0.0f; buf[i].x.b = 0.0f; buf[i].x.a = 1.0f;
		}
		for(;i<mSourceAlignedBufferSize.w * 600;i++) {
			buf[i].x.r = 0.0f; buf[i].x.g = 1.0f; buf[i].x.b = 0.0f; buf[i].x.a = 1.0f;
		}
		for(;i<mSourceAlignedBufferSize.w * 900;i++) {
			buf[i].x.r = 0.0f; buf[i].x.g = 0.0f; buf[i].x.b = 1.0f; buf[i].x.a = 1.0f;
		}
		for(;i<mSourceAlignedBufferSize.w * mSourceAlignedBufferSize.h;i++) {
			buf[i].x.r = 1.0f; buf[i].x.g = 1.0f; buf[i].x.b = 1.0f; buf[i].x.a = 1.0f;
		}
	}

}
#endif

	//vkQueueWaitIdle(mQueue);

#if 0  // copy from mSourceAlignedBuffer to mTextureTarget
{
	vkWaitForFences(mDevice, 1, &mFence, VK_TRUE, UINT64_MAX);	
	vkResetFences(mDevice, 1, &mFence);

	VkCommandBuffer layoutCmd = mVulkanExample->createCommandBuffer(
		VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

#if 1
		{ // copy buffer to image
		
			VkBufferImageCopy region = {};
		
			region.bufferOffset = 0;
			region.bufferRowLength = 0; // mSourceAlignedBufferSize.w * sizeof(_buffer_item_t); // this is wrong !!!
			region.bufferImageHeight = 0; // mSourceAlignedBufferSize.h;
		
			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = 1;
		
			region.imageOffset = {0,0,0};
			region.imageExtent = {
					.width = static_cast<uint32_t>(mTextureTargetSize.w),
					.height = static_cast<uint32_t>(mTextureTargetSize.h),
					.depth = 1,
				};
		
		   vks::tools::setImageLayout(layoutCmd, mTextureTarget.image,
			   VK_IMAGE_ASPECT_COLOR_BIT, 
			   VK_IMAGE_LAYOUT_GENERAL, 
			   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		
			vkCmdCopyBufferToImage(layoutCmd, mSourceAlignedBuffer.buffer, mTextureTarget.image,
								   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,1,&region);
		
			vks::tools::setImageLayout(layoutCmd, mTextureTarget.image,
				VK_IMAGE_ASPECT_COLOR_BIT, 
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		
		}
#endif

	mVulkanExample->flushCommandBuffer(layoutCmd, mQueue, true);
	vkWaitForFences(mDevice, 1, &mFence, VK_TRUE, UINT64_MAX);

	vkQueueWaitIdle(mQueue);


}
#endif

#if 0 // copy from mTextureTarget to mSourceAlignedBuffer
{
	vkWaitForFences(mDevice, 1, &mFence, VK_TRUE, UINT64_MAX);	
	vkResetFences(mDevice, 1, &mFence);

	VkCommandBuffer layoutCmd = mVulkanExample->createCommandBuffer(
		VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

		{ // copy image to buffer
		
			VkBufferImageCopy bufferImageCopy{};
			bufferImageCopy.imageSubresource = {
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.mipLevel = 0,
					.baseArrayLayer = 0,
					.layerCount = 1,
				};
			bufferImageCopy.imageOffset = {0, 0, 0};
			bufferImageCopy.imageExtent = {
					.width = static_cast<uint32_t>(mTextureTargetSize.w),
					.height = static_cast<uint32_t>(mTextureTargetSize.h),
					.depth = 1,
				};
			bufferImageCopy.bufferOffset = 0;
			bufferImageCopy.bufferRowLength = 0; // mSourceAlignedBufferSize.w * sizeof(_buffer_item_t); // this is wrong !!!
			bufferImageCopy.bufferImageHeight = 0; // mSourceAlignedBufferSize.h;

			vks::tools::setImageLayout(layoutCmd, mTextureTarget.image,
				VK_IMAGE_ASPECT_COLOR_BIT, 
				VK_IMAGE_LAYOUT_GENERAL, 
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

			vkCmdCopyImageToBuffer(layoutCmd, mTextureTarget.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				mSourceAlignedBuffer.buffer, 
				1, &bufferImageCopy);
#if 1
			vks::tools::setImageLayout(layoutCmd, mTextureTarget.image,
				VK_IMAGE_ASPECT_COLOR_BIT, 
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
#endif
		}

	mVulkanExample->flushCommandBuffer(layoutCmd, mQueue, true);
	vkWaitForFences(mDevice, 1, &mFence, VK_TRUE, UINT64_MAX);

	vkQueueWaitIdle(mQueue);


}

#endif

{
	memcpy(&mStatisticHostBuffer, mStatisticBuffer.mapped, sizeof(mStatisticHostBuffer));
	DLOGD("*** imNumPixels = %d constNumOfPixels = %d \r\n", mStatisticHostBuffer.imNumPixels, mStatisticHostBuffer.constNumOfPixels);
	double mean_ = (double)mStatisticHostBuffer.log_delta_L_div_numPixels_sum
		/((double)mStatisticHostBuffer.log_delta_L_div_numPixels_sum_magnify * (double)mStatisticHostBuffer.constNumOfPixels);
	DLOGD("  log_delta_L_div_numPixels_sum:%d mean: %8.3f Lav = exp(mean) = %8.3f \r\n", mStatisticHostBuffer.log_delta_L_div_numPixels_sum,
		mean_, exp(mean_));
	DLOGD(" iMinL=%d iMaxL=%d fMinL=%4.3f fMaxL=%4.3f \r\n",
		mStatisticHostBuffer.iMinL, mStatisticHostBuffer.iMaxL, mStatisticHostBuffer.fMinL, mStatisticHostBuffer.fMaxL);
	DLOGD("  mStatisticHostBuffer, p_ov=%4.3f, Lav = %4.3f key = %4.3f \r\n",
		mStatisticHostBuffer.p_ov, mStatisticHostBuffer.Lav, mStatisticHostBuffer.key);
	DLOGD("  mStatisticHostBuffer, m_gamma = %4.3f \r\n", mStatisticHostBuffer.m_gamma);
}

#if 0
#define SKIP_LINES (1500)
	if ((buffer_memory_property_ & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == 0) {
		_buffer_item_rgba32f_t *mapped_address = (_buffer_item_rgba32f_t *)mSourceAlignedBuffer.mapped;
		//_buffer_item_rgba8_t *mapped_address = (_buffer_item_rgba8_t *)mSourceAlignedBuffer.mapped;
		mapped_address += mSourceAlignedBufferSize.w * SKIP_LINES;
		memcpy(mSourceAlignedBuffer_tmp_buffer,mapped_address,
			TMP__buffer_item_t_NUM*sizeof(_buffer_item_t));

		print__tmp_buffer((_buffer_item_rgba32f_t *)(mSourceAlignedBuffer_tmp_buffer), TMP__buffer_item_t_NUM);
		//print__tmp_buffer((_buffer_item_rgba8_t *)(mSourceAlignedBuffer_tmp_buffer), TMP__buffer_item_t_NUM);
	}
	#if 0
	if ((Lbuffer_memory_property_ & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == 0) {
		//_buffer_item_r32f_t *mapped_address = (_buffer_item_r32f_t *)mLbuffer.mapped;
		_buffer_item_r32f_t *mapped_address = (_buffer_item_r32f_t *)mLbuffer.mapped;
		mapped_address += mLbufferSize.w * SKIP_LINES;
		memcpy(mLbuffer_tmp_buffer,mapped_address,
			mLbuffer_tmp_buffer_SIZE*sizeof(float));
		//print__tmp_buffer((_buffer_item_r32f_t *)(mLbuffer_tmp_buffer), mLbuffer_tmp_buffer_SIZE);
		print__tmp_buffer((_buffer_item_r8_t *)(mLbuffer_tmp_buffer), mLbuffer_tmp_buffer_SIZE);
	}
	#endif
#endif


#endif

}

void MasiaEO1::updateSource_VkDescriptorImageInfo(
	ImportedTexture *importedTexture,
	uint32_t w_, uint32_t h_, VkDescriptorImageInfo *descriptor) {

	vkWaitForFences(mDevice, 1, &mFence, VK_TRUE, UINT64_MAX);	
	vkResetFences(mDevice, 1, &mFence);

{
#if 0
	if ((buffer_memory_property_ & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == 0) {
		memset(mSourceAlignedBuffer.mapped, 0, mSourceAlignedBuffer_size);
	}
#endif
}
{
	DLOGD("size = %4d x %4d \r\n", w_, h_);

	// blit src image to the dest image

	VkCommandBuffer layoutCmd = mVulkanExample->createCommandBuffer(
		VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

#if 0
{
	VkImageBlit imageBlit{};
	imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageBlit.srcSubresource.layerCount = 1;
	imageBlit.srcSubresource.mipLevel = 0; // i - 1;
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
}
#endif

{
	VkImageBlit imageBlit_1{};
	VkImageBlit *imageBlit = &imageBlit_1;
	imageBlit->srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageBlit->srcSubresource.layerCount = 1;
	imageBlit->srcSubresource.mipLevel = 0; // i - 1;
	imageBlit->srcOffsets[1].x = int32_t(w_ >> 0);
	imageBlit->srcOffsets[1].y = int32_t(h_ >> 0);
	imageBlit->srcOffsets[1].z = 1;

	imageBlit->dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageBlit->dstSubresource.layerCount = 1;
	imageBlit->dstSubresource.mipLevel = 0; // i;
	imageBlit->dstOffsets[1].x = int32_t(mSourceAlignedSize.w >> 0);
	imageBlit->dstOffsets[1].y = int32_t(mSourceAlignedSize.h >> 0);
	imageBlit->dstOffsets[1].z = 1;

	// blit aligned image !!!
	vkCmdBlitImage(layoutCmd, 
		importedTexture->image_, VK_IMAGE_LAYOUT_GENERAL,
		mSourceAligned.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1, &imageBlit_1,
		VK_FILTER_LINEAR);

	vks::tools::setImageLayout(layoutCmd, mSourceAligned.image,
		VK_IMAGE_ASPECT_COLOR_BIT, 
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

	// blit scaled image !!!
#if 0
	imageBlit->dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageBlit->dstSubresource.layerCount = 1;
	imageBlit->dstSubresource.mipLevel = 0; // i;
	imageBlit->dstOffsets[1].x = int32_t(mSourceScaledSize.w >> 0);
	imageBlit->dstOffsets[1].y = int32_t(mSourceScaledSize.h >> 0);
	imageBlit->dstOffsets[1].z = 1;
	
	vkCmdBlitImage(layoutCmd, 
		importedTexture->image_, VK_IMAGE_LAYOUT_GENERAL,
		mSourceScaled.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1, &imageBlit_1,
		VK_FILTER_LINEAR);
#if 1
	vks::tools::setImageLayout(layoutCmd, mSourceScaled.image,
		VK_IMAGE_ASPECT_COLOR_BIT, 
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
#else
	vks::tools::setImageLayout(layoutCmd, mSourceScaled.image,
		VK_IMAGE_ASPECT_COLOR_BIT, 
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
#endif
#endif

}

#if 0
{ // copy image to buffer

	VkBufferImageCopy bufferImageCopy{};
	bufferImageCopy.imageSubresource = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.mipLevel = 0,
			.baseArrayLayer = 0,
			.layerCount = 1,
		};
	bufferImageCopy.imageOffset = {0, 0, 0};
	bufferImageCopy.imageExtent = {
			.width = static_cast<uint32_t>(mSourceAlignedSize.w),
			.height = static_cast<uint32_t>(mSourceAlignedSize.h),
			.depth = 1,
		};
	bufferImageCopy.bufferOffset = 0;
	bufferImageCopy.bufferRowLength = 0; // mSourceAlignedBufferSize.w * sizeof(_buffer_item_t); // this is wrong !!!
	bufferImageCopy.bufferImageHeight = 0; // mSourceAlignedBufferSize.h;

	vkCmdCopyImageToBuffer(layoutCmd, mSourceAligned.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		mSourceAlignedBuffer.buffer, 
		1, &bufferImageCopy);
#if 1
	vks::tools::setImageLayout(layoutCmd, mSourceAligned.image,
		VK_IMAGE_ASPECT_COLOR_BIT, 
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
#endif
}
#endif

#if 0
{ // copy buffer to image

	VkBufferImageCopy region = {};

	region.bufferOffset = 0;
	region.bufferRowLength = 0; // mSourceAlignedBufferSize.w * sizeof(_buffer_item_t); // this is wrong !!!
	region.bufferImageHeight = 0; // mSourceAlignedBufferSize.h;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = {0,0,0};
	region.imageExtent = {
			.width = static_cast<uint32_t>(mTextureTargetSize.w),
			.height = static_cast<uint32_t>(mTextureTargetSize.h),
			.depth = 1,
		};

   vks::tools::setImageLayout(layoutCmd, mTextureTarget.image,
	   VK_IMAGE_ASPECT_COLOR_BIT, 
	   VK_IMAGE_LAYOUT_GENERAL, 
	   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	vkCmdCopyBufferToImage(layoutCmd, mSourceAlignedBuffer.buffer, mTextureTarget.image,
						   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,1,&region);

	vks::tools::setImageLayout(layoutCmd, mTextureTarget.image,
		VK_IMAGE_ASPECT_COLOR_BIT, 
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

}
#endif

	mVulkanExample->flushCommandBuffer(layoutCmd, mQueue, true);
	vkWaitForFences(mDevice, 1, &mFence, VK_TRUE, UINT64_MAX);

	// image is already copyed at this time !
	DLOGD("sizeof(_buffer_item_t) = %d sizeof(float) = %d _buffer_item_rgba8_t:%d _buffer_item_rgba32f_t:%d \r\n",
		sizeof(_buffer_item_t), sizeof(float), 
		sizeof(_buffer_item_rgba8_t), sizeof(_buffer_item_rgba32f_t));

#if 0
	if ((buffer_memory_property_ & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == 0) {
		memcpy(mSourceAlignedBuffer_tmp_buffer,
			static_cast<_buffer_item_t *>(mSourceAlignedBuffer.mapped)
				+ (mSourceAlignedBufferSize.w * 0)  // skip some of the first lines
			,
			TMP__buffer_item_t_NUM*sizeof(_buffer_item_t));

		print__tmp_buffer((_buffer_item_rgba32f_t *)(mSourceAlignedBuffer_tmp_buffer), TMP__buffer_item_t_NUM);
		//print__tmp_buffer((_buffer_item_rgba8_t *)(mSourceAlignedBuffer_tmp_buffer), TMP__buffer_item_t_NUM);
	}
#endif
}


}

void MasiaEO1::setDebugWindow(int enable) {

}

void MasiaEO1::setBHE_factor(float f0, float f1) {

}

///////////////////////////////////////////////////////
void MasiaEO1::test_VkFormatProperties(VkFormat format, const char *format_str) {
	vks::VulkanDevice *vulkanDevice = mVulkanExample->vulkanDevice;

	VkFormatProperties formatProperties;

	// Get device properties for the requested texture format
	vkGetPhysicalDeviceFormatProperties(mPhysicalDevice, format, &formatProperties);
	
	{
		DLOGD( "%s format:%s 0x%08x(%d) \r\n" , __func__, format_str, format, format);
		DLOGD( "	 linearTilingFeatures:\r\n");
		::vks::android::print_VkFormatFeatureFlags(formatProperties.linearTilingFeatures);
		DLOGD( "	 optimalTilingFeatures:\r\n");
		::vks::android::print_VkFormatFeatureFlags(formatProperties.optimalTilingFeatures);
		DLOGD( "	 bufferFeatures:\r\n");
		::vks::android::print_VkFormatFeatureFlags(formatProperties.bufferFeatures);
	}

}

void MasiaEO1::test_find_format_for_FORMAT_FEATURE_(VkFormatFeatureFlags format_feature_bit, const char *desc) {
	vks::VulkanDevice *vulkanDevice = mVulkanExample->vulkanDevice;

	DLOGD( "format list that support feature: %s /0x%08x(%d) \r\n", desc, format_feature_bit, format_feature_bit);
	for(int format_ = VkFormat::VK_FORMAT_BEGIN_RANGE; format_ < VkFormat::VK_FORMAT_END_RANGE; format_ ++) {
		VkFormatProperties formatProperties__;
		vkGetPhysicalDeviceFormatProperties(mPhysicalDevice, (VkFormat)format_, &formatProperties__);

		if (formatProperties__.linearTilingFeatures & format_feature_bit) {
			DLOGD( "    linearTilingFeatures, format 0x%08x(%d) \r\n" , format_, format_);
		}

		if (formatProperties__.optimalTilingFeatures & format_feature_bit) {
			DLOGD( "    optimalTilingFeatures, format 0x%08x(%d) \r\n" , format_, format_);
		}

		if (formatProperties__.bufferFeatures & format_feature_bit) {
			DLOGD( "    bufferFeatures, format 0x%08x(%d) \r\n" , format_, format_);
		}
	}

}
void MasiaEO1::print__tmp_buffer(_buffer_item_rgba8_t *buf, int num) {
	int height = 1;
	int width = 256;
	if (num > 256) {
		height = num/width;
	}
	else {
		width = num;
	}

	int line_end = 0;
	int line_num = 8;

	fprintf(stderr, "%s num = %d /size : %4d x %4d \r\n", __func__, num, width, height);
	int line = 0;
	for(;line<height
		;line++) {
		for(int col=0;col<width
			;col++) {
			line_end = 0;

				_buffer_item_rgba8_t *p_ = &buf[line*width + col*1];
				fprintf(stderr, "%4d,%4d,%4d,%4d ", p_->r, p_->g, p_->b, p_->a);
			
			if(col % line_num == (line_num - 1)) {
				line_end = 1;
				fprintf(stderr, "\r\n");
			}
		}
		if (line_end == 0) {
			fprintf(stderr, "\r\n");
		}
	}


}
void MasiaEO1::print__tmp_buffer(_buffer_item_rgba32f_t *buf, int num) {
	int height = 1;
	int width = 256;
	if (num > 256) {
		height = num/width;
	}
	else {
		width = num;
	}

	int line_end = 0;
	int line_num = 8;

	fprintf(stderr, "%s num = %d /size : %4d x %4d \r\n", __func__, num, width, height);
	int line = 0;
	for(;line<height
		;line++) {
		for(int col=0;col<width
			;col++) {
			line_end = 0;

				_buffer_item_rgba32f_t *p_ = &buf[line*width + col*1];
				fprintf(stderr, "{% 6.2f,% 6.2f,% 6.2f,% 6.2f} ", p_->r, p_->g, p_->b, p_->a);
			
			if(col % line_num == (line_num - 1)) {
				line_end = 1;
				fprintf(stderr, "\r\n");
			}
		}
		if (line_end == 0) {
			fprintf(stderr, "\r\n");
		}
	}


}
void MasiaEO1::print__tmp_buffer(_buffer_item_r8_t *buf, int num) {
	int height = 1;
	int width = 256;
	if (num > 256) {
		height = num/width;
	}
	else {
		width = num;
	}

	int line_end = 0;
	int line_num = 8;

	fprintf(stderr, "%s num = %d /size : %4d x %4d \r\n", __func__, num, width, height);
	int line = 0;
	for(;line<height
		;line++) {
		for(int col=0;col<width
			;col++) {
			line_end = 0;

				_buffer_item_r8_t *p_ = &buf[line*width + col*1];
				fprintf(stderr, "%4d ", p_->r);
			
			if(col % line_num == (line_num - 1)) {
				line_end = 1;
				fprintf(stderr, "\r\n");
			}
		}
		if (line_end == 0) {
			fprintf(stderr, "\r\n");
		}
	}


}

void MasiaEO1::print__tmp_buffer(_buffer_item_r32f_t *buf, int num) {
	int height = 1;
	int width = 256;
	if (num > 256) {
		height = num/width;
	}
	else {
		width = num;
	}

	int line_end = 0;
	int line_num = 8;

	fprintf(stderr, "%s num = %d /size : %4d x %4d \r\n", __func__, num, width, height);
	int line = 0;
	for(;line<height
		;line++) {
		for(int col=0;col<width
			;col++) {
			line_end = 0;

				_buffer_item_r32f_t *p_ = &buf[line*width + col*1];
				fprintf(stderr, "%6.2f ", p_->r);
			
			if(col % line_num == (line_num - 1)) {
				line_end = 1;
				fprintf(stderr, "\r\n");
			}
		}
		if (line_end == 0) {
			fprintf(stderr, "\r\n");
		}
	}


}


};


