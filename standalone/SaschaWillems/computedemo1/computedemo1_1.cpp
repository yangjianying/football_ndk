/*
* Vulkan Example - Compute shader image processing
*
* Copyright (C) 2016 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

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


#include "computedemo1_1.h"
#include "computedemo1_1_Parasite.h"

// Android log function wrappers
static const char* kTAG = "computedemo1_1";
#include "utils/android_logcat_.h"


#define VERTEX_BUFFER_BIND_ID 0
#define ENABLE_VALIDATION false


#undef __CLASS__
#define __CLASS__ "VulkanExample"

// Vulkan call wrapper
#define CALL_VK(func)                                                 \
  if (VK_SUCCESS != (func)) {                                         \
    __android_log_print(ANDROID_LOG_ERROR, kTAG,                      \
                        "Vulkan error. File[%s], line[%d]", __FILE__, \
                        __LINE__);                                    \
    assert(false);                                                    \
  }

namespace computedemo1_1 {

static const char *shader_emboss = " \n"
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

static const char *shader_edgedetect = ""
"#version 450 \n"
"layout (local_size_x = 16, local_size_y = 16) in; \n"
"layout (binding = 0, rgba8) uniform readonly image2D inputImage; \n"
"layout (binding = 1, rgba8) uniform image2D resultImage; \n"
"\n"
"float conv(in float[9] kernel, in float[9] data, in float denom, in float offset)  \n"
"{ \n"
"   float res = 0.0; \n"
"   for (int i=0; i<9; ++i)  \n"
"   { \n"
"      res += kernel[i] * data[i]; \n"
"   } \n"
"   return clamp(res/denom + offset, 0.0, 1.0); \n"
"} \n"
"struct ImageData  \n"
"{ \n"
"	float avg[9]; \n"
"} imageData;	 \n"
"void main() \n"
"{	 \n"
"	// Fetch neighbouring texels \n"
"	int n = -1; \n"
"	for (int i=-1; i<2; ++i)  \n"
"	{    \n"
"		for(int j=-1; j<2; ++j)  \n"
"		{     \n"
"			n++;     \n"
"			vec3 rgb = imageLoad(inputImage, ivec2(gl_GlobalInvocationID.x + i, gl_GlobalInvocationID.y + j)).rgb; \n"
"			imageData.avg[n] = (rgb.r + rgb.g + rgb.b) / 3.0; \n"
"		} \n"
"	} \n"
"\n"
"	float[9] kernel; \n"
"	kernel[0] = -1.0/8.0; kernel[1] = -1.0/8.0; kernel[2] = -1.0/8.0; \n"
"	kernel[3] = -1.0/8.0; kernel[4] =  1.0;     kernel[5] = -1.0/8.0; \n"
"	kernel[6] = -1.0/8.0; kernel[7] = -1.0/8.0; kernel[8] = -1.0/8.0; \n"
"	vec4 res = vec4(vec3(conv(kernel, imageData.avg, 0.1, 0.0)), 1.0); \n"
"	imageStore(resultImage, ivec2(gl_GlobalInvocationID.xy), res); \n"
"} \n"
"";

static const char *shader_sharpen = ""
"#version 450 \n"
"layout (local_size_x = 16, local_size_y = 16) in; \n"
"layout (binding = 0, rgba8) uniform readonly image2D inputImage; \n"
"layout (binding = 1, rgba8) uniform image2D resultImage; \n"
"	\n"
"float conv(in float[9] kernel, in float[9] data, in float denom, in float offset)  \n"
"{ \n"
"   float res = 0.0; \n"
"   for (int i=0; i<9; ++i)  \n"
"   { \n"
"      res += kernel[i] * data[i]; \n"
"   } \n"
"   return clamp(res/denom + offset, 0.0, 1.0); \n"
"} \n"
"struct ImageData  \n"
"{ \n"
"	float r[9]; \n"
"	float g[9]; \n"
"	float b[9]; \n"
"} imageData;	 \n"
"void main() \n"
"{ \n"
"	// Fetch neighbouring texels \n"
"	int n = -1; \n"
"	for (int i=-1; i<2; ++i)  \n"
"	{    \n"
"		for(int j=-1; j<2; ++j)  \n"
"		{     \n"
"			n++;     \n"
"			vec3 rgb = imageLoad(inputImage, ivec2(gl_GlobalInvocationID.x + i, gl_GlobalInvocationID.y + j)).rgb; \n"
"			imageData.r[n] = rgb.r; \n"
"			imageData.g[n] = rgb.g; \n"
"			imageData.b[n] = rgb.b; \n"
"		} \n"
"	} \n"
"	float[9] kernel; \n"
"	kernel[0] = -1.0; kernel[1] = -1.0; kernel[2] = -1.0; \n"
"	kernel[3] = -1.0; kernel[4] =  9.0; kernel[5] = -1.0; \n"
"	kernel[6] = -1.0; kernel[7] = -1.0; kernel[8] = -1.0; \n"
"			\n"
"	vec4 res = vec4(\n"
"		conv(kernel, imageData.r, 1.0, 0.0), \n"
"		conv(kernel, imageData.g, 1.0, 0.0), \n"
"		conv(kernel, imageData.b, 1.0, 0.0),\n"
"		1.0);\n"
"	imageStore(resultImage, ivec2(gl_GlobalInvocationID.xy), res);\n"
"}\n"
"";


VulkanExample::VulkanExample() : VulkanExampleBase(ENABLE_VALIDATION)
{
	DLOGD( "%s:%s ...\r\n", __FILE__, __func__);
	zoom = -2.0f;
	zoom = -4.0f;  // frankie, add

	title = "Compute shader image load/store";
	settings.overlay = true;		// frankie, using overlay !!!

	DLOGD( "%s:%s done \r\n", __FILE__, __func__);
}
void VulkanExample::InitExtra() {
	if (INIT_TEST == mInitFlag) {
		mParasite = VulkanExample_Parasite::create(this, 1);
		setDebugWindow(1);
		mScreenMode = SCREEN_MODE_HALF; 
	}
	else if (mInitFlag == INIT_HISTOGRAM_GRAPHIC) {
		mParasite = VulkanExample_Parasite::create(this, 0);

		setDebugWindow(1);
		mScreenMode = SCREEN_MODE_HALF; 
	}
	else if (mInitFlag == INIT_COMPUTE_AGC) {
		mParasite = VulkanExample_Parasite::create(this, 1);
		// normal screen , close the overlay
		settings.overlay = false;
		setDebugWindow(0);
		mScreenMode = SCREEN_MODE_FULL; 

	}
	else if(mInitFlag == INIT_COMPUTE_BHE) {
		mParasite = VulkanExample_Parasite::create(this, 1);
		// normal screen , close the overlay
		settings.overlay = false;
		setDebugWindow(0);
		mScreenMode = SCREEN_MODE_FULL; 

	}
	else if(mInitFlag == INIT_COMPUTE_AGC_BHE) {
		mParasite = VulkanExample_Parasite::create(this, 1);
		// normal screen , close the overlay
		settings.overlay = false;
		setDebugWindow(0);
		mScreenMode = SCREEN_MODE_FULL; 
	}
	else if(INIT_COMPUTE_MasiaEO == mInitFlag) {
		mParasite = VulkanExample_Parasite::create(this, 2);
	#if 0
		// debug mode !
		setDebugWindow(1);
		mScreenMode = SCREEN_MODE_HALF; 
	#else
		settings.overlay = false;
		setDebugWindow(0);
		mScreenMode = SCREEN_MODE_FULL; 
	#endif
	}
	else {
		mParasite = VulkanExample_Parasite::create(this, 1);
		setDebugWindow(1);
		mScreenMode = SCREEN_MODE_HALF; 
	}
}
void VulkanExample::setDebugWindow(int enable) {
	if (mParasite != nullptr) { mParasite->setDebugWindow(enable); }
}
void VulkanExample::impl1_setBHE_factor(float f0, float f1) {
	if (mParasite != nullptr) { mParasite->setBHE_factor(f0, f1); }
}
void VulkanExample::impl1_setBHE_tuning(float t0, float t1) {
	if (mParasite != nullptr) { mParasite->setBHE_tuning(t0, t1); }
}

void VulkanExample::setScreenMode(int mode_) {
	if (mScreenMode == mode_) { return ; }
	if (mode_ >= SCREEN_MODE_MAX) {
		mode_ = SCREEN_MODE_HALF;
	}
	mScreenMode = mode_;
	updateUniformBuffers();
	buildCommandBuffers();
}

VulkanExample::~VulkanExample()
{
	DLOGD( "%s ... \r\n", __func__);
	
	if (mParasite != nullptr) {  // frankie, add
		delete mParasite;
		mParasite = nullptr;
	}

	// Graphics
	vkDestroyPipeline(device, graphics.pipeline, nullptr);
	vkDestroyPipelineLayout(device, graphics.pipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(device, graphics.descriptorSetLayout, nullptr);

	// Compute
	for (auto& pipeline : compute.pipelines)
	{
		vkDestroyPipeline(device, pipeline, nullptr);
	}
	vkDestroyPipelineLayout(device, compute.pipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(device, compute.descriptorSetLayout, nullptr);
	vkDestroyFence(device, compute.fence, nullptr);
	vkDestroyCommandPool(device, compute.commandPool, nullptr);

	deleteImportTexture(0x01);

	vertexBuffer.destroy();
	indexBuffer.destroy();
	uniformBufferVS.destroy();

	textureColorMap.destroy();
	textureComputeTarget.destroy();

	DLOGD( "%s done !\r\n", __func__);
}
// Prepare a texture target that is used to store compute shader calculations
void VulkanExample::prepareTextureTarget(vks::Texture *tex, uint32_t width, uint32_t height, VkFormat format)
{
	VkFormatProperties formatProperties;

	// Get device properties for the requested texture format
	vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProperties);
	// Check if requested image format supports image storage operations
	assert(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT);
#if 0
{
	DLOGD( "VulkanExample::%s format 0x%08x(%d) \r\n" , __func__, format, format);
	DLOGD( "    linearTilingFeatures:\r\n");
	::vks::android::print_VkFormatFeatureFlags(formatProperties.linearTilingFeatures);
	DLOGD( "    optimalTilingFeatures:\r\n");
	::vks::android::print_VkFormatFeatureFlags(formatProperties.optimalTilingFeatures);
	DLOGD( "    bufferFeatures:\r\n");
	::vks::android::print_VkFormatFeatureFlags(formatProperties.bufferFeatures);
}
#endif
	// Prepare blit target texture
	tex->width = width;
	tex->height = height;

	VkImageCreateInfo imageCreateInfo = vks::initializers::imageCreateInfo();
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = format;
	imageCreateInfo.extent = { width, height, 1 };
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	// Image will be sampled in the fragment shader and used as storage target in the compute shader
	imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
	imageCreateInfo.flags = 0;
	// Sharing mode exclusive means that ownership of the image does not need to be explicitly transferred between the compute and graphics queue
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkMemoryAllocateInfo memAllocInfo = vks::initializers::memoryAllocateInfo();
	VkMemoryRequirements memReqs;

	VK_CHECK_RESULT(vkCreateImage(device, &imageCreateInfo, nullptr, &tex->image));

	vkGetImageMemoryRequirements(device, tex->image, &memReqs);
	memAllocInfo.allocationSize = memReqs.size;
	memAllocInfo.memoryTypeIndex = vulkanDevice->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VK_CHECK_RESULT(vkAllocateMemory(device, &memAllocInfo, nullptr, &tex->deviceMemory));
	VK_CHECK_RESULT(vkBindImageMemory(device, tex->image, tex->deviceMemory, 0));

	VkCommandBuffer layoutCmd = VulkanExampleBase::createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

	tex->imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	vks::tools::setImageLayout(
		layoutCmd, tex->image, 
		VK_IMAGE_ASPECT_COLOR_BIT, 
		VK_IMAGE_LAYOUT_UNDEFINED, 
		tex->imageLayout);

	VulkanExampleBase::flushCommandBuffer(layoutCmd, queue, true);

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
	VK_CHECK_RESULT(vkCreateSampler(device, &sampler, nullptr, &tex->sampler));

	// Create image view
	VkImageViewCreateInfo view = vks::initializers::imageViewCreateInfo();
	view.image = VK_NULL_HANDLE;
	view.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view.format = format;
	view.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	view.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	view.image = tex->image;
	VK_CHECK_RESULT(vkCreateImageView(device, &view, nullptr, &tex->view));

	// Initialize a descriptor for later use
	tex->descriptor.imageLayout = tex->imageLayout;
	tex->descriptor.imageView = tex->view;
	tex->descriptor.sampler = tex->sampler;
	tex->device = vulkanDevice;
}

void VulkanExample::loadAssets()
{
#if 0  // frankie, add
	textureColorMap.loadFromFile(getAssetPath() + "textures/cb_0000.ktx", 
		VK_FORMAT_R8G8B8A8_UNORM, vulkanDevice, queue, 
		VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_LAYOUT_GENERAL);
#else
	textureColorMap.loadFromFile(getAssetPath() + "textures/vulkan_11_rgba.ktx", 
		VK_FORMAT_R8G8B8A8_UNORM, vulkanDevice, queue, 
		VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_LAYOUT_GENERAL);
#endif
}

void VulkanExample::buildCommandBuffers_for_testscreen() {
	// Destroy command buffers if already present
	if (!checkCommandBuffers())
	{
		destroyCommandBuffers();
		createCommandBuffers();
	}

	VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

	VkClearValue clearValues[2];
	clearValues[0].color = defaultClearColor;
	clearValues[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
	renderPassBeginInfo.renderPass = renderPass;
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = width;
	renderPassBeginInfo.renderArea.extent.height = height;
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValues;

	DLOGD( "VulkanExample::%s, drawCmdBuffers.size() = %" PRIu64 " \r\n", __func__, drawCmdBuffers.size());
	for (int32_t i = 0; i < drawCmdBuffers.size(); ++i)
	{
		// Set target frame buffer
		renderPassBeginInfo.framebuffer = frameBuffers[i];

		VK_CHECK_RESULT(vkBeginCommandBuffer(drawCmdBuffers[i], &cmdBufInfo));

#if 0
	// frankie, add the compute command, then graphics command
	// here, will result the low fps , as the compute shader and graphics shader is executed serially!
		vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_COMPUTE, compute.pipelines[compute.pipelineIndex]);
		vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_COMPUTE, compute.pipelineLayout, 0, 1, &compute.descriptorSet, 0, 0);

		// vkCmdDispatch(drawCmdBuffers[i], textureComputeTarget.width / 16, textureComputeTarget.height / 16, 1);
		vkCmdDispatch(drawCmdBuffers[i], width / 16, height / 16, 1);
#endif
		if (mParasite != nullptr) { mParasite->appendCommandBuffers(drawCmdBuffers[i]); }
	
		vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport = vks::initializers::viewport((float)width * 0.5f, (float)height, 0.0f, 1.0f);  // left half screen
		vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);

		VkRect2D scissor = vks::initializers::rect2D(width, height, 0, 0);
		vkCmdSetScissor(drawCmdBuffers[i], 0, 1, &scissor);

		VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(drawCmdBuffers[i], VERTEX_BUFFER_BIND_ID, 1, &vertexBuffer.buffer, offsets);
		vkCmdBindIndexBuffer(drawCmdBuffers[i], indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics.pipeline);
	
		// Left (pre compute)
		vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics.pipelineLayout, 0, 1, &graphics.descriptorSetPreCompute, 0, NULL);
		vkCmdDrawIndexed(drawCmdBuffers[i], indexCount, 1, 0, 0, 0);

		// Image memory barrier to make sure that compute shader writes are finished before sampling from the texture
		VkImageMemoryBarrier imageMemoryBarrier = {};
		imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		// We won't be changing the layout of the image
		imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
		imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
		imageMemoryBarrier.image = textureComputeTarget.image;
#if USE_VulkanExample_Parasite_target
		if (mParasite != nullptr) {
			if (mParasite->isTextureTargetAvailable()) {
			vks::Texture2D & target_ = mParasite->getTextureTarget();
			imageMemoryBarrier.image = target_.image;
			}
		}
#endif
		imageMemoryBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		vkCmdPipelineBarrier(
			drawCmdBuffers[i],
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			VK_FLAGS_NONE,
			0, nullptr,
			0, nullptr,
			1, &imageMemoryBarrier);

		// Right (post compute)
		vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics.pipelineLayout, 0, 1, &graphics.descriptorSetPostCompute, 0, NULL);
		viewport.x = (float)width / 2.0f;				// right half screen
		vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);
		vkCmdDrawIndexed(drawCmdBuffers[i], indexCount, 1, 0, 0, 0);

		drawUI(drawCmdBuffers[i]);

		vkCmdEndRenderPass(drawCmdBuffers[i]);

		VK_CHECK_RESULT(vkEndCommandBuffer(drawCmdBuffers[i]));
	}

}

void VulkanExample::buildCommandBuffers_for_fullscreen() {
	// Destroy command buffers if already present
	if (!checkCommandBuffers())
	{
		destroyCommandBuffers();
		createCommandBuffers();
	}

	VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

	VkClearValue clearValues[2];
	clearValues[0].color = defaultClearColor;
	clearValues[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
	renderPassBeginInfo.renderPass = renderPass;
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = width;
	renderPassBeginInfo.renderArea.extent.height = height;
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValues;

	DLOGD( "VulkanExample::%s, drawCmdBuffers.size() = %" PRIu64 " \r\n", __func__, drawCmdBuffers.size());
	for (int32_t i = 0; i < drawCmdBuffers.size(); ++i)
	{
		// Set target frame buffer
		renderPassBeginInfo.framebuffer = frameBuffers[i];

		VK_CHECK_RESULT(vkBeginCommandBuffer(drawCmdBuffers[i], &cmdBufInfo));

		if (mParasite != nullptr) { mParasite->appendCommandBuffers(drawCmdBuffers[i]); }
	
		vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport = vks::initializers::viewport((float)width * 1.0f, (float)height, 0.0f, 1.0f);  // full screen
		vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);

		VkRect2D scissor = vks::initializers::rect2D(width, height, 0, 0);
		vkCmdSetScissor(drawCmdBuffers[i], 0, 1, &scissor);

		VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(drawCmdBuffers[i], VERTEX_BUFFER_BIND_ID, 1, &vertexBuffer.buffer, offsets);
		vkCmdBindIndexBuffer(drawCmdBuffers[i], indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics.pipeline);

#if 0
		// Left (pre compute)
		vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics.pipelineLayout, 0, 1, &graphics.descriptorSetPreCompute, 0, NULL);
		vkCmdDrawIndexed(drawCmdBuffers[i], indexCount, 1, 0, 0, 0);
#endif

		// Image memory barrier to make sure that compute shader writes are finished before sampling from the texture
		VkImageMemoryBarrier imageMemoryBarrier = {};
		imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		// We won't be changing the layout of the image
		imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
		imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
		imageMemoryBarrier.image = textureComputeTarget.image;
#if USE_VulkanExample_Parasite_target
		if (mParasite != nullptr) {
			if (mParasite->isTextureTargetAvailable()) {
			vks::Texture2D & target_ = mParasite->getTextureTarget();
			imageMemoryBarrier.image = target_.image;
			}
		}
#endif
		imageMemoryBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		vkCmdPipelineBarrier(
			drawCmdBuffers[i],
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			VK_FLAGS_NONE,
			0, nullptr,
			0, nullptr,
			1, &imageMemoryBarrier);

		// Right (post compute)
		vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics.pipelineLayout, 0, 1, &graphics.descriptorSetPostCompute, 0, NULL);
#if 0
		viewport.x = (float)width / 2.0f;
		vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);
#endif
		vkCmdDrawIndexed(drawCmdBuffers[i], indexCount, 1, 0, 0, 0);

		drawUI(drawCmdBuffers[i]);

		vkCmdEndRenderPass(drawCmdBuffers[i]);

		VK_CHECK_RESULT(vkEndCommandBuffer(drawCmdBuffers[i]));
	}

}

void VulkanExample::buildCommandBuffers() {
	if (mScreenMode == SCREEN_MODE_FULL) {
		buildCommandBuffers_for_fullscreen();
	} else {
		buildCommandBuffers_for_testscreen();
	}
}

void VulkanExample::buildComputeCommandBuffer()
{
	// Flush the queue if we're rebuilding the command buffer after a pipeline change to ensure it's not currently in use
	vkQueueWaitIdle(compute.queue);

	VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

	VK_CHECK_RESULT(vkBeginCommandBuffer(compute.commandBuffer, &cmdBufInfo));

	vkCmdBindPipeline(compute.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute.pipelines[compute.pipelineIndex]);
	vkCmdBindDescriptorSets(compute.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute.pipelineLayout, 0, 1, &compute.descriptorSet, 0, 0);

	vkCmdDispatch(compute.commandBuffer, textureComputeTarget.width / 16, textureComputeTarget.height / 16, 1);

	vkEndCommandBuffer(compute.commandBuffer);
}
// Setup vertices for a single uv-mapped quad
void VulkanExample::generateQuad()
{
	// Setup vertices for a single uv-mapped quad made from two triangles
	std::vector<Vertex> vertices =
	{
		{ { 1.0f, 1.0f, 0.0f }, { 1.0f, 1.0f } },
		{ { -1.0f, 1.0f, 0.0f }, { 0.0f, 1.0f } },
		{ { -1.0f, -1.0f, 0.0f }, { 0.0f, 0.0f } },
		{ { 1.0f, -1.0f, 0.0f }, { 1.0f, 0.0f } }
	};

	// Setup indices
	std::vector<uint32_t> indices = { 0,1,2, 2,3,0 };
	indexCount = static_cast<uint32_t>(indices.size());

	// Create buffers
	// For the sake of simplicity we won't stage the vertex data to the gpu memory
	// Vertex buffer
	VK_CHECK_RESULT(vulkanDevice->createBuffer(
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&vertexBuffer,
		vertices.size() * sizeof(Vertex),
		vertices.data()));
	// Index buffer
	VK_CHECK_RESULT(vulkanDevice->createBuffer(
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&indexBuffer,
		indices.size() * sizeof(uint32_t),
		indices.data()));
}
void VulkanExample::setupVertexDescriptions()
{
	// Binding description
	vertices.bindingDescriptions = {
		vks::initializers::vertexInputBindingDescription(VERTEX_BUFFER_BIND_ID, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX)
	};

	// Attribute descriptions
	// Describes memory layout and shader positions
	vertices.attributeDescriptions = {
		// Location 0: Position
		vks::initializers::vertexInputAttributeDescription(VERTEX_BUFFER_BIND_ID, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos)),
		// Location 1: Texture coordinates
		vks::initializers::vertexInputAttributeDescription(VERTEX_BUFFER_BIND_ID, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)),
	};

	// Assign to vertex buffer
	vertices.inputState = vks::initializers::pipelineVertexInputStateCreateInfo();
	vertices.inputState.vertexBindingDescriptionCount = vertices.bindingDescriptions.size();
	vertices.inputState.pVertexBindingDescriptions = vertices.bindingDescriptions.data();
	vertices.inputState.vertexAttributeDescriptionCount = vertices.attributeDescriptions.size();
	vertices.inputState.pVertexAttributeDescriptions = vertices.attributeDescriptions.data();
}
void VulkanExample::setupDescriptorPool()
{
	std::vector<VkDescriptorPoolSize> poolSizes = {
		// Graphics pipelines uniform buffers 
		vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2),
		// Graphics pipelines image samplers for displaying compute output image
		vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2),
		// Compute pipelines uses a storage image for image reads and writes
		vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 2),
	};
	VkDescriptorPoolCreateInfo descriptorPoolInfo = vks::initializers::descriptorPoolCreateInfo(poolSizes, 3);
	VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &descriptorPool));
}
void VulkanExample::setupDescriptorSetLayout()
{
	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = { 		
		// Binding 0: Vertex shader uniform buffer
		vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0),
		// Binding 1: Fragment shader input image
		vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1)
	};

	VkDescriptorSetLayoutCreateInfo descriptorLayout = vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings);
	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorLayout, nullptr, &graphics.descriptorSetLayout));
	
	VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = vks::initializers::pipelineLayoutCreateInfo(&graphics.descriptorSetLayout, 1);
	VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, nullptr, &graphics.pipelineLayout));
}
void VulkanExample::setupDescriptorSet()
{
	VkDescriptorSetAllocateInfo allocInfo =
		vks::initializers::descriptorSetAllocateInfo(descriptorPool, &graphics.descriptorSetLayout, 1);

	// Input image (before compute post processing)
	VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &graphics.descriptorSetPreCompute));
	std::vector<VkWriteDescriptorSet> baseImageWriteDescriptorSets = {
		vks::initializers::writeDescriptorSet(graphics.descriptorSetPreCompute, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &uniformBufferVS.descriptor),
		vks::initializers::writeDescriptorSet(graphics.descriptorSetPreCompute, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &textureColorMap.descriptor)
	};
	vkUpdateDescriptorSets(device, baseImageWriteDescriptorSets.size(), baseImageWriteDescriptorSets.data(), 0, nullptr);

	// Final image (after compute shader processing)
	VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &graphics.descriptorSetPostCompute));
	std::vector<VkWriteDescriptorSet> writeDescriptorSets = {			
		vks::initializers::writeDescriptorSet(graphics.descriptorSetPostCompute, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &uniformBufferVS.descriptor),
		vks::initializers::writeDescriptorSet(graphics.descriptorSetPostCompute, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &textureComputeTarget.descriptor)
	};
	vkUpdateDescriptorSets(device, writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);

}
void VulkanExample::preparePipelines()
{
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
		vks::initializers::pipelineInputAssemblyStateCreateInfo(
			VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			0,
			VK_FALSE);

	VkPipelineRasterizationStateCreateInfo rasterizationState =
		vks::initializers::pipelineRasterizationStateCreateInfo(
			VK_POLYGON_MODE_FILL,
			VK_CULL_MODE_NONE,
			VK_FRONT_FACE_COUNTER_CLOCKWISE,
			0);

	VkPipelineColorBlendAttachmentState blendAttachmentState =
		vks::initializers::pipelineColorBlendAttachmentState(
			0xf,
			VK_FALSE);

	VkPipelineColorBlendStateCreateInfo colorBlendState =
		vks::initializers::pipelineColorBlendStateCreateInfo(
			1,
			&blendAttachmentState);

	VkPipelineDepthStencilStateCreateInfo depthStencilState =
		vks::initializers::pipelineDepthStencilStateCreateInfo(
			VK_TRUE,
			VK_TRUE,
			VK_COMPARE_OP_LESS_OR_EQUAL);

	VkPipelineViewportStateCreateInfo viewportState =
		vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);

	VkPipelineMultisampleStateCreateInfo multisampleState =
		vks::initializers::pipelineMultisampleStateCreateInfo(
			VK_SAMPLE_COUNT_1_BIT,
			0);

	std::vector<VkDynamicState> dynamicStateEnables = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	VkPipelineDynamicStateCreateInfo dynamicState =
		vks::initializers::pipelineDynamicStateCreateInfo(
			dynamicStateEnables.data(),
			dynamicStateEnables.size(),
			0);

	// Rendering pipeline
	// Load shaders
	std::array<VkPipelineShaderStageCreateInfo,2> shaderStages;

	shaderStages[0] = loadShader(getAssetPath() + "shaders/computeshader/texture.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = loadShader(getAssetPath() + "shaders/computeshader/texture.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

	VkGraphicsPipelineCreateInfo pipelineCreateInfo =
		vks::initializers::pipelineCreateInfo(
			graphics.pipelineLayout,
			renderPass,
			0);

	pipelineCreateInfo.pVertexInputState = &vertices.inputState;
	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
	pipelineCreateInfo.pRasterizationState = &rasterizationState;
	pipelineCreateInfo.pColorBlendState = &colorBlendState;
	pipelineCreateInfo.pMultisampleState = &multisampleState;
	pipelineCreateInfo.pViewportState = &viewportState;
	pipelineCreateInfo.pDepthStencilState = &depthStencilState;
	pipelineCreateInfo.pDynamicState = &dynamicState;
	pipelineCreateInfo.stageCount = shaderStages.size();
	pipelineCreateInfo.pStages = shaderStages.data();
	pipelineCreateInfo.renderPass = renderPass;

	VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCreateInfo, nullptr, &graphics.pipeline));
}
void VulkanExample::getComputeQueue()
{
	uint32_t queueFamilyCount;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, NULL);
	assert(queueFamilyCount >= 1);

	std::vector<VkQueueFamilyProperties> queueFamilyProperties;
	queueFamilyProperties.resize(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

	// Some devices have dedicated compute queues, so we first try to find a queue that supports compute and not graphics
	bool computeQueueFound = false;
	for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++)
	{
		if ((queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) && ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
		{
			compute.queueFamilyIndex = i;
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
				compute.queueFamilyIndex = i;
				computeQueueFound = true;
				break;
			}
		}
	}

	// Compute is mandatory in Vulkan, so there must be at least one queue family that supports compute
	assert(computeQueueFound);
	// Get a compute queue from the device
	vkGetDeviceQueue(device, compute.queueFamilyIndex, 0, &compute.queue);

	DLOGD( "* %s, compute.queueFamilyIndex = %d, queue=%p \r\n", __func__, compute.queueFamilyIndex, compute.queue);
}
void VulkanExample::prepareCompute()
{
	getComputeQueue();

	// Create compute pipeline
	// Compute pipelines are created separate from graphics pipelines even if they use the same queue

	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
		// Binding 0: Input image (read-only)
		vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 0),
		// Binding 1: Output image (write)
		vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 1),
	};

	VkDescriptorSetLayoutCreateInfo descriptorLayout = vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings);
	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device,	&descriptorLayout, nullptr, &compute.descriptorSetLayout));

	VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
		vks::initializers::pipelineLayoutCreateInfo(&compute.descriptorSetLayout, 1);
	VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, nullptr, &compute.pipelineLayout));

	VkDescriptorSetAllocateInfo allocInfo =
		vks::initializers::descriptorSetAllocateInfo(descriptorPool, &compute.descriptorSetLayout, 1);
	VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &compute.descriptorSet));
	std::vector<VkWriteDescriptorSet> computeWriteDescriptorSets = {			
		vks::initializers::writeDescriptorSet(compute.descriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0, &textureColorMap.descriptor),
		vks::initializers::writeDescriptorSet(compute.descriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, &textureComputeTarget.descriptor)
	};
	vkUpdateDescriptorSets(device, computeWriteDescriptorSets.size(), computeWriteDescriptorSets.data(), 0, NULL);

	// Create compute shader pipelines
	VkComputePipelineCreateInfo computePipelineCreateInfo =
		vks::initializers::computePipelineCreateInfo(compute.pipelineLayout, 0);

	// One pipeline for each effect

shaderNames = { "emboss", "edgedetect", "sharpen" };
shaderSources = {shader_emboss, shader_edgedetect, shader_sharpen};

	int shader_index_ = 0;
	for (auto& shaderName : shaderNames) {
#if 1
		const char *source_ = shaderSources[shader_index_++];
		computePipelineCreateInfo.stage = loadShader_from_strings_c(source_, VK_SHADER_STAGE_COMPUTE_BIT);
#else
		std::string fileName = getAssetPath() + "shaders/computeshader/" + shaderName + ".comp.spv";
		computePipelineCreateInfo.stage = loadShader(fileName, VK_SHADER_STAGE_COMPUTE_BIT);
#endif
		VkPipeline pipeline;
		VK_CHECK_RESULT(vkCreateComputePipelines(device, pipelineCache, 1, &computePipelineCreateInfo, nullptr, &pipeline));
		compute.pipelines.push_back(pipeline);
	}

	// Separate command pool as queue family for compute may be different than graphics
	VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.queueFamilyIndex = compute.queueFamilyIndex;
	cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	VK_CHECK_RESULT(vkCreateCommandPool(device, &cmdPoolInfo, nullptr, &compute.commandPool));

	// Create a command buffer for compute operations
	VkCommandBufferAllocateInfo cmdBufAllocateInfo =
		vks::initializers::commandBufferAllocateInfo(
			compute.commandPool,
			VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			1);

	VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &cmdBufAllocateInfo, &compute.commandBuffer));

	// Fence for compute CB sync
	VkFenceCreateInfo fenceCreateInfo = vks::initializers::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
	VK_CHECK_RESULT(vkCreateFence(device, &fenceCreateInfo, nullptr, &compute.fence));

#if 0  // frankie, add
	// Build a single command buffer containing the compute dispatch commands
	buildComputeCommandBuffer();  // frankie, move the compute command before the draw command!
#endif
}
// Prepare and initialize uniform buffer containing shader uniforms
void VulkanExample::prepareUniformBuffers()
{
	// Vertex shader uniform buffer block
	VK_CHECK_RESULT(vulkanDevice->createBuffer(
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&uniformBufferVS,
		sizeof(uboVS)));

	// Map persistent
	VK_CHECK_RESULT(uniformBufferVS.map());

	updateUniformBuffers();
}
void VulkanExample::updateUniformBuffers()
{
	// Vertex shader uniform buffer block

	if (mScreenMode == SCREEN_MODE_FULL) {
		uboVS.projection = glm::mat4(1.0f);
		uboVS.model = glm::mat4(1.0f);
	} else {

		//uboVS.projection = glm::perspective(glm::radians(60.0f), (float)width*0.5f / (float)height, 0.1f, 256.0f);
		
		uboVS.projection = glm::perspective(glm::radians(60.0f), (float)width / (float)height, 0.1f, 256.0f);  // frankie, 
		
		glm::mat4 viewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, zoom));

		uboVS.model = viewMatrix * glm::translate(glm::mat4(1.0f), cameraPos);
		uboVS.model = glm::rotate(uboVS.model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		uboVS.model = glm::rotate(uboVS.model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		uboVS.model = glm::rotate(uboVS.model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
	}

	memcpy(uniformBufferVS.mapped, &uboVS, sizeof(uboVS));
}
void VulkanExample::prepare()
{
	DLOGD( "%s:%s ...\r\n", __FILE__, __func__);
	VulkanExampleBase::prepare();

	loadAssets();
	generateQuad();
	setupVertexDescriptions();
	prepareUniformBuffers();
		//prepareTextureTarget(&textureComputeTarget, textureColorMap.width, textureColorMap.height, VK_FORMAT_R8G8B8A8_UNORM);
	prepareTextureTarget(&textureComputeTarget, width, height, VK_FORMAT_R8G8B8A8_UNORM);
	setupDescriptorSetLayout();
	preparePipelines();
	setupDescriptorPool();
	setupDescriptorSet();
	prepareCompute();
	if (mParasite != nullptr) {
		mParasite->setVulkanEnv(physicalDevice, device, pipelineCache);
		mParasite->prepare();

	#if USE_VulkanExample_Parasite_target
		if (mParasite->isTextureTargetAvailable()) {
			vks::Texture2D & target_ = mParasite->getTextureTarget();
			std::vector<VkWriteDescriptorSet> writeDescriptorSets = {			
				vks::initializers::writeDescriptorSet(graphics.descriptorSetPostCompute, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &target_.descriptor)
			};
			vkUpdateDescriptorSets(device, writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
		}
	#endif

	}

	buildCommandBuffers(); 

	prepared = true;
	DLOGD( "%s:%s done \r\n", __FILE__, __func__);
}
/*virtual*/void VulkanExample::render()
{
	if (!prepared)
		return;
	draw();
}
void VulkanExample::draw()
{
	VulkanExampleBase::prepareFrame();

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &drawCmdBuffers[currentBuffer];
	VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));

	VulkanExampleBase::submitFrame();

#if 0 // frankie, add
	// Submit compute commands
	// Use a fence to ensure that compute command buffer has finished executin before using it again
	vkWaitForFences(device, 1, &compute.fence, VK_TRUE, UINT64_MAX);
	vkResetFences(device, 1, &compute.fence);

	VkSubmitInfo computeSubmitInfo = vks::initializers::submitInfo();
	computeSubmitInfo.commandBufferCount = 1;
	computeSubmitInfo.pCommandBuffers = &compute.commandBuffer;

	VK_CHECK_RESULT(vkQueueSubmit(compute.queue, 1, &computeSubmitInfo, compute.fence));
#endif

	if (mParasite != nullptr) { mParasite->draw(); }
	

}

/*virtual*/void VulkanExample::viewChanged()
{
	updateUniformBuffers();
}
/*virtual*/void VulkanExample::OnUpdateUIOverlay(vks::UIOverlay *overlay)
{
	if (overlay->header("Settings")) {
		if (overlay->comboBox("Shader", &compute.pipelineIndex, shaderNames)) {
			DLOGD( "VulkanExample::%s need to rebuild the commandbuffer for UI \r\n", __func__);
	#if 0  // frankie, add
			buildComputeCommandBuffer();
	#else
			buildCommandBuffers();
	#endif
		}
	}
}

int VulkanExample::importAHardwareBufferAsTexture(AHardwareBuffer *hardwareBuffer) {
	DLOGD( "%s ... \r\n", __func__);

	AHardwareBuffer_Desc hardwareBufferDesc;
	AHardwareBuffer_describe(hardwareBuffer, &hardwareBufferDesc);

	DLOGD( "  hardwareBufferDesc, size:%4d x %4d \r\n", hardwareBufferDesc.width, hardwareBufferDesc.height);

	VkDevice device_ = vulkanDevice->logicalDevice;
	device_ = device;
	
	if (importTexture.image_ != VK_NULL_HANDLE) { // release old
		vkDestroyImage(device_, importTexture.image_, nullptr);
		importTexture.image_ = VK_NULL_HANDLE;
	}
	if (importTexture.imageMem_ != VK_NULL_HANDLE) {
		vkFreeMemory(device_, importTexture.imageMem_, nullptr);
		importTexture.imageMem_ = VK_NULL_HANDLE;
	}

	bool needBlit = false; // must not set true, if import HadrdwareBuffer into vulkan !!! 
				// as the image usage bit should not be VK_IMAGE_USAGE_TRANSFER_SRC_BIT except VK_IMAGE_USAGE_SAMPLED_BIT

//{
	//
	VkAndroidHardwareBufferFormatPropertiesANDROID bufferFormatProperties = {
		.sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID,
		.pNext = nullptr,
		//.format,
		//.externalFormat,
		//.formatFeatures,
		//.samplerYcbcrConversionComponents,
		//.suggestedYcbcrModel,
		//.suggestedYcbcrRange,
		//.suggestedXChromaOffset,
		//.suggestedYChromaOffset,
		};
	
	VkAndroidHardwareBufferPropertiesANDROID bufferProperities = {
		.sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID,
		.pNext = &bufferFormatProperties,
		.allocationSize = 0,
		.memoryTypeBits = 0,
		};
	VkResult ret = vkGetAndroidHardwareBufferPropertiesANDROID(device_, 
		hardwareBuffer, &bufferProperities);

	DLOGD( "  bufferFormatProperties.format:0x%" PRIx32 " / %" PRId32 "\r\n", 
		bufferFormatProperties.format, bufferFormatProperties.format);
		//0x25/37 VK_FORMAT_R8G8B8A8_UNORM
	DLOGD( "  bufferFormatProperties.externalFormat:0x%" PRIx64 " / %" PRId64 "\r\n", 
		bufferFormatProperties.externalFormat, bufferFormatProperties.externalFormat);

	// create image
	VkExternalMemoryImageCreateInfo externalMemoryImageCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO,
		.pNext = nullptr,
		.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID,
		};
	VkExternalFormatANDROID  externalFormatAndroid = {
		.sType = VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID,
		.pNext = &externalMemoryImageCreateInfo,
		.externalFormat = bufferFormatProperties.externalFormat,
		};

	VkImageCreateInfo imageCreateInfo = {
	      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
	      .pNext = &externalFormatAndroid,
		  .flags = 0,
	      .imageType = VK_IMAGE_TYPE_2D,
	      .format = VK_FORMAT_UNDEFINED,
	      .extent = {static_cast<uint32_t>(hardwareBufferDesc.width),
	                 static_cast<uint32_t>(hardwareBufferDesc.height), 1},
	      .mipLevels = 1,
	      .arrayLayers = 1,
	      .samples = VK_SAMPLE_COUNT_1_BIT,
	      .tiling = VK_IMAGE_TILING_LINEAR,
	      .usage = (needBlit ? VK_IMAGE_USAGE_TRANSFER_SRC_BIT
	                         : VK_IMAGE_USAGE_SAMPLED_BIT),
	      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
	      .queueFamilyIndexCount = 0,
	      .pQueueFamilyIndices = nullptr,
	      	// queueFamilyIndexCount, pQueueFamilyIndices (ignored if sharingMode is not VK_SHARING_MODE_CONCURRENT).
	      .initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED,
	  };

/*

If the pNext chain includes a VkExternalFormatANDROID structure, 
and its externalFormat member is non-zero the format must be VK_FORMAT_UNDEFINED.

If the pNext chain does not include a VkExternalFormatANDROID structure, 
or does and its externalFormat member is 0, the format must not be VK_FORMAT_UNDEFINED.

If the pNext chain includes a VkExternalMemoryImageCreateInfo structure 
whose handleTypes member includes VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID, 
imageType must be VK_IMAGE_TYPE_2D.

If the pNext chain includes a VkExternalMemoryImageCreateInfo structure 
whose handleTypes member includes VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID, 
mipLevels must either be 1 or equal to the number of levels in the complete mipmap chain based on extent.width, extent.height, and extent.depth.

If the pNext chain includes a VkExternalFormatANDROID structure 
whose externalFormat member is not 0, flags must not include VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT.

If the pNext chain includes a VkExternalFormatANDROID structure 
whose externalFormat member is not 0, usage must not include any usages except VK_IMAGE_USAGE_SAMPLED_BIT.

If the pNext chain includes a VkExternalFormatANDROID structure 
whose externalFormat member is not 0, tiling must be VK_IMAGE_TILING_OPTIMAL.

*/
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = VK_FORMAT_UNDEFINED;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

	imageCreateInfo.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
		// note here, if want to use the HardwareBuffer as the compute shader storage image , must set this bit !!!
	imageCreateInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

	imageCreateInfo.flags &= ~VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;  // must not include

	CALL_VK(vkCreateImage(device_, &imageCreateInfo, nullptr,
						  &importTexture.image_));

	VkMemoryRequirements mem_reqs;
	vkGetImageMemoryRequirements(device_, importTexture.image_, &mem_reqs);
	
	// create memory
	VkImportAndroidHardwareBufferInfoANDROID  importHardwareBufferAndroid = {
		.sType = VK_STRUCTURE_TYPE_IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID,
		.pNext = nullptr,
		.buffer = hardwareBuffer,
		};
	VkMemoryAllocateInfo allocateInfo = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = &importHardwareBufferAndroid,
		.allocationSize = 0,
		.memoryTypeIndex = 0,
		};

#if 0  // when import hardwarebuffer, must not set the .allocationSize, .memoryTypeIndex
#endif
	CALL_VK(vkAllocateMemory(device_, &allocateInfo, nullptr, &importTexture.imageMem_));

	CALL_VK(vkBindImageMemory(device_, importTexture.image_, importTexture.imageMem_, 0));
//}

	// image view
	{
		if (importTexture.imageView_ != VK_NULL_HANDLE) { // release old
			vkDestroyImageView(device_, importTexture.imageView_, nullptr);
			importTexture.imageView_ = VK_NULL_HANDLE;
		}
		VkImageViewCreateInfo viewCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.pNext = nullptr,
			.image = VK_NULL_HANDLE,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = VK_FORMAT_UNDEFINED,
			.components =
			{
				VK_COMPONENT_SWIZZLE_R, 
				VK_COMPONENT_SWIZZLE_G,
				VK_COMPONENT_SWIZZLE_B, 
				VK_COMPONENT_SWIZZLE_A,
			},
			.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
			.flags = 0,
			};
		viewCreateInfo.image = importTexture.image_;
		viewCreateInfo.format = bufferFormatProperties.format;
		CALL_VK(vkCreateImageView(device_, &viewCreateInfo, nullptr, &importTexture.imageView_));
	}


	if (importTexture.sampler_ == VK_NULL_HANDLE) {
		const VkSamplerCreateInfo samplerCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.pNext = nullptr,
			.magFilter = VK_FILTER_NEAREST,
			.minFilter = VK_FILTER_NEAREST,
			.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
			.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
			.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
			.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
			.mipLodBias = 0.0f,
			.maxAnisotropy = 1,
			.compareOp = VK_COMPARE_OP_NEVER,
			.minLod = 0.0f,
			.maxLod = 0.0f,
			.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
			.unnormalizedCoordinates = VK_FALSE,
			};
		CALL_VK(vkCreateSampler(device_, &samplerCreateInfo, nullptr,
					&importTexture.sampler_));
	}

/** note: here how to determine the resources is already used by graphic/comput commands, then can update the resource safely !!! */
	// update into descriptorSet

	VkDescriptorImageInfo texDst = {
			.sampler = importTexture.sampler_,
			.imageView = importTexture.imageView_,
			.imageLayout = VK_IMAGE_LAYOUT_GENERAL,
		};

{
	std::vector<VkWriteDescriptorSet> baseImageWriteDescriptorSets = {
		vks::initializers::writeDescriptorSet(graphics.descriptorSetPreCompute, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &texDst)
	};
	vkUpdateDescriptorSets(device_, baseImageWriteDescriptorSets.size(), baseImageWriteDescriptorSets.data(), 0, nullptr);

	std::vector<VkWriteDescriptorSet> computeWriteDescriptorSets = {			
		vks::initializers::writeDescriptorSet(compute.descriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0, &texDst),
	};
	vkUpdateDescriptorSets(device_, computeWriteDescriptorSets.size(), computeWriteDescriptorSets.data(), 0, NULL);

	if (mParasite != nullptr) {
		mParasite->updateSource_VkDescriptorImageInfo(&importTexture, 
			hardwareBufferDesc.width, hardwareBufferDesc.height, &texDst);
	}
}

	DLOGD( "%s done! \r\n", __func__);
	return 0;
}
void VulkanExample::deleteImportTexture(int flags) {
	DLOGD( "%s ... \r\n", __func__);
	VkDevice device_ = vulkanDevice->logicalDevice;
	device_ = device;

	if (importTexture.image_ != VK_NULL_HANDLE) {
		vkDestroyImage(device_,importTexture.image_, nullptr);
		importTexture.image_ = VK_NULL_HANDLE;
	}
	if (importTexture.imageMem_ != VK_NULL_HANDLE) {
		vkFreeMemory(device_, importTexture.imageMem_, nullptr);
		importTexture.imageMem_ = VK_NULL_HANDLE;
	}
	if (importTexture.imageView_ != VK_NULL_HANDLE) {
		vkDestroyImageView(device_, importTexture.imageView_, nullptr);
		importTexture.imageView_ = VK_NULL_HANDLE;
	}
	if (flags & 0x01) {
		if (importTexture.sampler_ != VK_NULL_HANDLE) {
			vkDestroySampler(device_, importTexture.sampler_, nullptr);
			importTexture.sampler_ = VK_NULL_HANDLE;
		}
	}
	DLOGD( "%s done! \r\n", __func__);
}


};

VULKAN_EXAMPLE_MAIN(computedemo1_1, "SaschaWillems/computeshader_foot/assets/")





