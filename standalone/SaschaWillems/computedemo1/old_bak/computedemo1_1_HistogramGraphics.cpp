#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <vector>
#include <iostream>
#include <string>
#include <map>
#include <thread>
#include <mutex>
#include <condition_variable>   // NOLINT

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vulkan/vulkan.h>

#include "FootballConfig.h"
#include "utils/football_debugger.h"
#undef __CLASS__
#define __CLASS__ "HistogramGraphics"

#include "utils/ANativeWindowUtils.h"

#include "ndk_extend/NativeHooApi_Loader.h"

#include "computedemo1_1.h"

// Android log function wrappers
static const char* kTAG = "computedemo1_1";
#include "utils/android_logcat_.h"

// Vulkan call wrapper
#define CALL_VK(func)                                                 \
  if (VK_SUCCESS != (func)) {                                         \
    __android_log_print(ANDROID_LOG_ERROR, kTAG,                      \
                        "Vulkan error. File[%s], line[%d]", __FILE__, \
                        __LINE__);                                    \
    assert(false);                                                    \
  }

#define CALL_VK0(func) (func)

namespace computedemo1_1 {

static const char *source_vert_ = "\n"
"#version 400   \n"
	"precision highp float; \n"
	"precision highp int;\n"

"#extension GL_ARB_separate_shader_objects : enable   \n"
"#extension GL_ARB_shading_language_420pack : enable   \n"
"	\n"
"	layout (location = 0) in vec4 pos;   \n"
"	layout (location = 1) in vec2 attr;   \n"
"   \n"
"	layout (binding = 0) uniform UBO \n"
"	{  \n"
"		mat4 projection;  \n"
"		mat4 model;  \n"
"	} ubo;  \n"

"	layout (binding = 2) uniform sampler2D tex_1;   \n"

"	layout (location = 0) out vec2 texcoord;   \n"

"	layout (location = 1) out vec3 colorFactor;   \n"

"	const vec3 W1 = vec3(0.2125, 0.7154, 0.0721); \n"

"	const vec3 W2 = vec3(0.299, 0.587, 0.114); \n"

"	void main() {   \n"
"		texcoord = attr;   \n"

//"		gl_Position = pos;   \n"

"		vec4 pos_n = pos; \n"

"		vec2 uv = pos.xy; \n"
"		vec4 rgb = texture(tex_1, uv);   \n"

"		float luminance = dot(rgb.xyz, W1); \n"
//"		float luminance = dot(rgb.xyz, W2); \n"

#if 1
	#if 1
"		if (luminance >= 1.0) { luminance = 0.99999999999999; } \n"
"		int segment = int(floor(luminance*4.0)); \n"
"		if (segment >= 4) { segment = 3; }\n"
"		luminance = luminance - segment * (1.0/4.0); \n"
"		luminance *= 4.0; \n"
"		pos_n.x = luminance*2 - 1;  pos_n.z = 0.0; \n"
"		pos_n.y = -1.0 + (1.0/4.0) + segment * (2.0/4.0);  \n"
"		gl_Position = pos_n;  \n"
"		colorFactor = vec3(1.0, 1.0, 1.0); \n"
	#else
"		pos_n.x = luminance*2 - 1;	pos_n.y = 0.0; pos_n.z = 0.0; \n"
"		gl_Position = pos_n;  \n"
"		colorFactor = vec3(1.0, 1.0, 1.0); \n"
	#endif
#else
"		pos_n.x = pos.x*2 - 1;	pos_n.y = pos.y*2 - 1; pos_n.z = 0.0; \n"
							//"		pos_n.x = pos.x; pos_n.y = pos.y; pos_n.z = pos_n.z; \n"  // debug
							//"		pos_n.x = 0.0; pos_n.y = 0.0; pos_n.z = 0.0; \n"		// debug
"		gl_Position = pos_n;  \n"
//"		colorFactor = vec3(luminance, luminance, luminance); \n"
"		colorFactor = rgb.rgb; \n"
#endif

"		gl_PointSize = 1.0; \n"

"	}   \n"
"   \n";

static const char *source_frag_ = "\n"
"#version 400   \n"
	"precision highp float; \n"
	"precision highp int;\n"

"#extension GL_ARB_separate_shader_objects : enable   \n"
"#extension GL_ARB_shading_language_420pack : enable   \n"
"   \n"
"	layout (binding = 1) uniform sampler2D tex;   \n"

"	layout (location = 0) in vec2 texcoord;   \n"

"	layout (location = 1) in vec3 colorFactor;   \n"

"	layout (location = 0) out vec4 uFragColor;   \n"


//"		const float scalingFactor = 1.0 / 1.0; \n"
"		const float scalingFactor = 1.0 / 256.0; \n"
//"		const float scalingFactor = 1.0 / 1024.0; \n" // if the value is samller than 1.0/256.0 , the value can not be written into the 8-bit channel !!!
//"		const float scalingFactor = 1.0 / 2048.0; \n" // if the value is samller than 1.0/256.0 , the value can not be written into the 8-bit channel !!!
//"		const float scalingFactor = 1.0 / 161024.0; \n" // if the value is samller than 1.0/256.0 , the value can not be written into the 8-bit channel !!!
//"   	const float scalingFactor = 1.0 / (1080.0 * 2340.0); \n" // if the value is samller than 1.0/256.0 , the value can not be written into the 8-bit channel !!!


"	void main() {   \n"

//"		uFragColor = texture(tex, texcoord);   \n"
//"		uFragColor = vec4(colorFactor * scalingFactor, 1.0);   \n"
"		uFragColor = vec4(colorFactor * scalingFactor, 1.0);	\n"
//"		uFragColor = vec4(1.0, 0.0, 0.0, 1.0);	\n"

"	}   \n"
"   \n";


VulkanSwapchainInfo::VulkanSwapchainInfo() {}

VulkanSwapchainInfo::~VulkanSwapchainInfo() {
	for (int i = 0; i < swapchainLength_; i++) {
	  vkDestroyFramebuffer(mInfo.device_, framebuffers_[i], nullptr);
	  vkDestroyImageView(mInfo.device_, displayViews_[i], nullptr);
	}
	if (framebuffers_ != nullptr) {
		delete[] framebuffers_;
		framebuffers_ = nullptr;
	}
	if (displayViews_ != nullptr) {
		delete[] displayViews_;
		displayViews_ = nullptr;
	}
	if (displayImages_ != nullptr) {
		delete[] displayImages_;
		displayImages_ = nullptr;
	}
	
	vkDestroySwapchainKHR(mInfo.device_, swapchain_, nullptr);

	destroySurface();
}

void VulkanSwapchainInfo::initSurface(InitInfo &info) {
	mInfo = info;

	createSurface(mInfo.window_);
	createSwapChain();
}
void VulkanSwapchainInfo::createSurface(ANativeWindow* platformWindow) {
	VkAndroidSurfaceCreateInfoKHR createInfo{
		.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
		.pNext = nullptr,
		.flags = 0,
		.window = platformWindow};
	
	CALL_VK(vkCreateAndroidSurfaceKHR(mInfo.instance_, &createInfo, nullptr, &surface_));
}
void VulkanSwapchainInfo::destroySurface() {
	vkDestroySurfaceKHR(mInfo.instance_, surface_, nullptr);
	surface_ = VK_NULL_HANDLE;
}
void VulkanSwapchainInfo::createSwapChain() {
	
	// **********************************************************
	// Get the surface capabilities because:
	//	 - It contains the minimal and max length of the chain, we will need it
	//	 - It's necessary to query the supported surface format (R8G8B8A8 for
	//	 instance ...)
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mInfo.gpuDevice_, surface_, &surfaceCapabilities);

	// Query the list of supported surface format and choose one we like
	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(mInfo.gpuDevice_, surface_, &formatCount, nullptr);
	VkSurfaceFormatKHR* formats = new VkSurfaceFormatKHR[formatCount];
	vkGetPhysicalDeviceSurfaceFormatsKHR(mInfo.gpuDevice_, surface_, &formatCount, formats);
	LOGI("Got %d formats", formatCount);
	DLOGD( "formatCount : %d \r\n", formatCount);
	
	uint32_t chosenFormat;
	for (chosenFormat = 0; chosenFormat < formatCount; chosenFormat++) {
	  if (formats[chosenFormat].format == VK_FORMAT_R8G8B8A8_UNORM) break;
	}
	assert(chosenFormat < formatCount);
	
	displaySize_ = surfaceCapabilities.currentExtent;
	displayFormat_ = formats[chosenFormat].format;
	
	  LOGW("surfaceCapabilities, minImageCount:%d maxImageCount:%d ",
		  surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount);
	  LOGW("displaySize_ : %4d x %4d ", displaySize_.width, displaySize_.height);

	DLOGD( "displaySize_ : %4d x %4d \r\n", displaySize_.width, displaySize_.height);
	DLOGD( "displayFormat_ : 0x%08x \r\n", displayFormat_);

	// Create a swap chain (here we choose the minimum available number of surface in the chain)
	VkSwapchainCreateInfoKHR swapchainCreateInfo {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.pNext = nullptr,
		.surface = surface_,
		.minImageCount = surfaceCapabilities.minImageCount,
		.imageFormat = formats[chosenFormat].format,
		.imageColorSpace = formats[chosenFormat].colorSpace,
		.imageExtent = surfaceCapabilities.currentExtent,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
		.imageArrayLayers = 1,
		.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 1,
		.pQueueFamilyIndices = &mInfo.queueFamilyIndex_,
		.presentMode = VK_PRESENT_MODE_FIFO_KHR,
		.oldSwapchain = VK_NULL_HANDLE,
		.clipped = VK_FALSE,
	};
	CALL_VK(vkCreateSwapchainKHR(mInfo.device_, &swapchainCreateInfo, nullptr,
								 &swapchain_));
	
	// Get the length of the created swap chain
	CALL_VK(vkGetSwapchainImagesKHR(mInfo.device_, swapchain_,
									&swapchainLength_, nullptr));
	DLOGD( "swapchainLength_ = %d \r\n", swapchainLength_);

	delete[] formats;


	// query display attachment to swapchain
	uint32_t SwapchainImagesCount = 0;
	CALL_VK(vkGetSwapchainImagesKHR(mInfo.device_, swapchain_,
									&SwapchainImagesCount, nullptr));
	displayImages_ = new VkImage[SwapchainImagesCount];
	CALL_VK(vkGetSwapchainImagesKHR(mInfo.device_, swapchain_,
									&SwapchainImagesCount,
									displayImages_));
	
	// create image view for each swapchain image
	displayViews_ = new VkImageView[SwapchainImagesCount];
	for (uint32_t i = 0; i < SwapchainImagesCount; i++) {
	  VkImageViewCreateInfo viewCreateInfo = {
		  .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		  .pNext = nullptr,
		  .image = displayImages_[i],
		  .viewType = VK_IMAGE_VIEW_TYPE_2D,
		  .format = displayFormat_,
		  .components =
			  {
				  .r = VK_COMPONENT_SWIZZLE_R,
				  .g = VK_COMPONENT_SWIZZLE_G,
				  .b = VK_COMPONENT_SWIZZLE_B,
				  .a = VK_COMPONENT_SWIZZLE_A,
			  },
		  .subresourceRange =
			  {
				  .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				  .baseMipLevel = 0,
				  .levelCount = 1,
				  .baseArrayLayer = 0,
				  .layerCount = 1,
			  },
		  .flags = 0,
	  };
	  CALL_VK(vkCreateImageView(mInfo.device_, &viewCreateInfo, nullptr, &displayViews_[i]));
	}
	
}

void VulkanSwapchainInfo::createFrameBuffers(VkRenderPass& renderPass,
                        VkImageView depthView) {
  // create a framebuffer from each swapchain image
  framebuffers_ = new VkFramebuffer[swapchainLength_];
  for (uint32_t i = 0; i < swapchainLength_; i++) {
    VkImageView attachments[2] = {
        displayViews_[i], depthView,
    };
    VkFramebufferCreateInfo fbCreateInfo{
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .pNext = nullptr,
        .renderPass = renderPass,
        .layers = 1,
        .attachmentCount = 1,  // 2 if using depth
        .pAttachments = attachments,
        .width = static_cast<uint32_t>(displaySize_.width),
        .height = static_cast<uint32_t>(displaySize_.height),
    };
    fbCreateInfo.attachmentCount = (depthView == VK_NULL_HANDLE ? 1 : 2);

    CALL_VK(vkCreateFramebuffer(mInfo.device_, &fbCreateInfo, nullptr, &framebuffers_[i]));
  }
}

////////////////////////////////////////////////////////////////////////////////

#define USING_DISPLAY_WINDOW   (0) // (1)
#define HistogramDataReader_WIDTH (4096) // (4096) // (1024)
#define HistogramDataReader_HEIGHT (4)

class HistogramDataReader: public football::TestReader {
public:
	HistogramDataReader(int width, int height): TestReader(width, height, AIMAGE_FORMAT_RGBA_8888, 3, 
		AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN
			| AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER) {

#if 1  // always open the debug window !!!
		{
			std::unique_lock<std::mutex> lock_(stats_mutex_);
			if (mHistogramGrapher_ == nullptr) {
				mHistogramGrapher_ = HistogramGrapher::create();
			}
		}
#endif
	}
	~HistogramDataReader() {
		{
			std::unique_lock<std::mutex> lock_(stats_mutex_);
			if (mHistogramGrapher_ != nullptr) {
				delete mHistogramGrapher_; mHistogramGrapher_ = nullptr;
			}
		}
	}
	void setStatisticSourceImageSize(int w, int h) {
		mSourceImageWidth = w;
		mSourceImageHeight = h;
	}
	virtual void onImageAvailableCallback_2_l(AImageReader *reader) override {
		AImage *image_ = nullptr;
		media_status_t ret = AImageReader_acquireLatestImage(reader, &image_);
		if (ret == AMEDIA_OK && image_ != nullptr) {
			football::AImageAndroidInfo info;
			football::getAImageAndroidInfo(image_, &info);
			football::printAImageAndroidInfo("HistogramDataReader default: ",info);

{
	std::unique_lock<std::mutex> lock_(stats_mutex_);
	memset( &indirectStats, 0, sizeof(indirectStats));
	indirectStats.bin_count = 256;
	{
		// assuming info size is (width x 1), width will be 256, 512, 1024, etc ...
		// assuming info size is (width x height), height will be 2, 4, 6, 8, etc ...
		uint32_t color_[128];
		int hist_block_size = info.width/256;
		hist_block_size *= HistogramDataReader_HEIGHT;
		DLOGD( "hist_block_size = %d \r\n", hist_block_size);
		indirectStats.drawCount = mSourceImageWidth * mSourceImageHeight;
		for(int i=0;i < 256; i ++) {
			int color_sum = 0;
			for(int e_ = 0; e_ < hist_block_size; e_ ++) {
				int linear_pos_in_image = i*hist_block_size + e_;
				int x_ = linear_pos_in_image%HistogramDataReader_WIDTH;
				int y_ = linear_pos_in_image/HistogramDataReader_WIDTH;
				//DLOGD( "linear_pos_in_image = %d x_:%4d y_:%4d \r\n", linear_pos_in_image, x_, y_);
				
				color_[e_] = info.getColor32(x_, y_, 0)&0xff;
				color_sum += color_[e_];
			}
			indirectStats.lodCount_d[i] = 0;
			indirectStats.lodCount[i] = color_sum;
		}
	}
	//indirectStats.print_u32();
#if 1
	{
		IndirectStats__c *stats = &indirectStats;
		for(int i=0;i<256;i++) {
			stats->lodCount_d[i] = stats->lodCount[i];
			stats->lodCount_d[i] /= stats->drawCount;
			stats->lodCount_d[i] *= 100;  // %
		}
	}
	
	//indirectStats.print();

	if (mHistogramGrapher_ != nullptr) {
		mHistogramGrapher_->draw_stats(&indirectStats);
	}
	//indirectStats.print();
#endif
}

			AImage_delete(image_);
		}
	}
	void setDebugWindow(int enable) {
		{
			std::unique_lock<std::mutex> lock_(stats_mutex_);
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
	}

	IndirectStats__c indirectStats;
	std::mutex stats_mutex_;
	HistogramGrapher *mHistogramGrapher_ = nullptr;
	uint32_t mSourceImageWidth = 0;
	uint32_t mSourceImageHeight = 0;
};

/* in this object, i encapsulate a second graphic draw , this draw use importedTexture, then render into a framebuffer of 256 * 1 size ! */
HistogramGraphics::HistogramGraphics(VulkanExample *vulkanExample)
	: mVulkanExample(vulkanExample) {

	mSwapChain = new VulkanSwapchainInfo();
}
HistogramGraphics::~HistogramGraphics() {


	if (mPipeline != VK_NULL_HANDLE) {
		vkDestroyPipeline(mDevice, mPipeline, nullptr);
	}

	vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(mDevice, mDescriptorSetLayout, nullptr);

	if (cmdBufferLen_ > 0) { vkFreeCommandBuffers(mDevice, mCommandPool, cmdBufferLen_, cmdBuffer_);}
	if (cmdBuffer_ != nullptr) {delete[] cmdBuffer_;}
	if (semaphore_ != VK_NULL_HANDLE) {vkDestroySemaphore(mDevice, semaphore_, nullptr); }

	if (mFence != VK_NULL_HANDLE) {vkDestroyFence(mDevice, mFence, nullptr);}
	if (mCommandPool != VK_NULL_HANDLE) {vkDestroyCommandPool(mDevice, mCommandPool, nullptr);}

	if (renderPass_ != VK_NULL_HANDLE) {vkDestroyRenderPass(mDevice, renderPass_, nullptr);}

	if (_vertexDataPos != nullptr) {delete _vertexDataPos; _vertexDataPos = nullptr;}
	if (vertexBuf_ != VK_NULL_HANDLE) {vkDestroyBuffer(mDevice, vertexBuf_, nullptr); }

	textureColorMap.destroy();
	mTextureSourceScaled.destroy();
	uniformBufferVS.destroy();

	if (framebuffer_ != VK_NULL_HANDLE) {vkDestroyFramebuffer(mDevice, framebuffer_, nullptr);}

	if (mDescriptorPool != VK_NULL_HANDLE) {vkDestroyDescriptorPool(mDevice, mDescriptorPool, nullptr);}
	
	if (mSwapChain != nullptr) {delete mSwapChain;mSwapChain = nullptr;
	}

	mPlatformWindow_ = nullptr;
	if (mDisplayWindow_ != nullptr) { delete mDisplayWindow_;mDisplayWindow_ = nullptr; }
	if (mHistogramDataReader != nullptr) { delete mHistogramDataReader; mHistogramDataReader = nullptr; }

}

void HistogramGraphics::setup_renderpass(VkFormat displayFormat_) {
	// -----------------------------------------------------------------
	// Create render pass
	VkAttachmentDescription attachmentDescriptions{
		.format = displayFormat_,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
	};
	
	VkAttachmentReference colourReference = {
		.attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
	
	VkSubpassDescription subpassDescription{
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.flags = 0,
		.inputAttachmentCount = 0,
		.pInputAttachments = nullptr,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colourReference,
		.pResolveAttachments = nullptr,
		.pDepthStencilAttachment = nullptr,
		.preserveAttachmentCount = 0,
		.pPreserveAttachments = nullptr,
	};
	VkRenderPassCreateInfo renderPassCreateInfo{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.pNext = nullptr,
		.attachmentCount = 1,
		.pAttachments = &attachmentDescriptions,
		.subpassCount = 1,
		.pSubpasses = &subpassDescription,
		.dependencyCount = 0,
		.pDependencies = nullptr,
	};
	CALL_VK(vkCreateRenderPass(mDevice, &renderPassCreateInfo, nullptr, &renderPass_));

}

const std::string HistogramGraphics::getAssetPath() { return ""; }


void HistogramGraphics::createTextureTarget(vks::Texture *tex, uint32_t w_, uint32_t h_, VkFormat format, VkImageUsageFlags extra_flags) {
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

void HistogramGraphics::setup_target_texture() {
	vks::VulkanDevice *vulkanDevice = mVulkanExample->vulkanDevice;

	textureColorMap.loadFromFile(getAssetPath() + "textures/vulkan_11_rgba.ktx", 
		VK_FORMAT_R8G8B8A8_UNORM, vulkanDevice, mQueue, 
		VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_LAYOUT_GENERAL);

	VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
	createTextureTarget(&mTextureSourceScaled, mTextureScaled_width, mTextureScaled_height, format, VK_IMAGE_USAGE_TRANSFER_DST_BIT);

}

HistogramGraphics::VertexDataPosBuffer::VertexDataPosBuffer(int w_, int h_) {
	const float OFFSET_H = 0.5f/(float)w_;
	const float OFFSET_V = 0.5f/(float)h_;
	float *vertex_ = new float[5*w_*h_];
	uint64_t size_ = sizeof(float) * 5*w_*h_;
	int index = 0;
	for(int line = 0; line < h_; line++) {
		for(int c = 0; c < w_; c++) {
			index = line*w_ + c;
			float pos_x = c; pos_x /= (float)w_; pos_x += OFFSET_H;
			float pos_y = line; pos_y /= (float)h_; pos_y += OFFSET_V;
			//pos_x = pos_x*2 - 1;
			//pos_y = pos_y*2 - 1;
			vertex_[index*5 + 0] = pos_x;
			vertex_[index*5 + 1] = pos_y;
			vertex_[index*5 + 2] = 0.0f;
			vertex_[index*5 + 3] = 0.0f;
			vertex_[index*5 + 4] = 0.0f;
		}
	}
	vertex_data = vertex_;
	vertex_length = size_;
}
HistogramGraphics::VertexDataPosBuffer::~VertexDataPosBuffer() {
	//delete vertex_data;
	delete[] vertex_data;
	vertex_data = nullptr;
	vertex_length = 0;
}

void HistogramGraphics::setup_buffer() {
	vks::VulkanDevice *vulkanDevice = mVulkanExample->vulkanDevice;

	// Vertex shader uniform buffer block
	VK_CHECK_RESULT(vulkanDevice->createBuffer(
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&uniformBufferVS,
		sizeof(uboVS)));

	// Map persistent
	VK_CHECK_RESULT(uniformBufferVS.map());


	/////////////////////////////////// another buffer
	  // Vertex positions
	  //const
	  float vertexData[] = {
#if 1
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 
		1.0f, -1.0f, 0.0f, 1.0f, 0.0f,	
		-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
	
		-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
#else
		  -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 
		  1.0f, -1.0f, 0.0f, 1.0f, 0.0f,  
		  0.0f, 1.0f, 0.0f, 0.5f, 1.0f,
#endif
	  };
		uint64_t _vertexData_size = sizeof(vertexData);
		float *_vertexData_buf = static_cast<float *>(vertexData);
			// error: static_cast from 'const float *' to 'float *' is not allowed

	#if 1
		DLOGD( "HistogramGraphics::%s _vertexDataPos size = %4d x %4d \r\n", __func__, 
			mTextureScaled_width, mTextureScaled_height);

		if (_vertexDataPos != nullptr) {delete _vertexDataPos; _vertexDataPos = nullptr;}
		_vertexDataPos = new VertexDataPosBuffer(mTextureScaled_width, mTextureScaled_height);
		_vertexData_buf = _vertexDataPos->vertex_data;
		_vertexData_size = _vertexDataPos->vertex_length;
	#endif

	  // Create a vertex buffer
	  VkBufferCreateInfo createBufferInfo{
		  .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		  .pNext = nullptr,
		  .size = _vertexData_size,
		  .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		  .flags = 0,
		  .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		  .queueFamilyIndexCount = 1,
		  .pQueueFamilyIndices = &mQueueFamilyIndex,
	  };
	
	  CALL_VK(vkCreateBuffer(mDevice, &createBufferInfo, nullptr, &vertexBuf_));
	
	  VkMemoryRequirements memReq;
	  vkGetBufferMemoryRequirements(mDevice, vertexBuf_, &memReq);
	
	  VkMemoryAllocateInfo allocInfo{
		  .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		  .pNext = nullptr,
		  .allocationSize = memReq.size,
		  .memoryTypeIndex = 0,  // Memory type assigned in the next step
	  };
	
	  // Assign the proper memory type for that buffer
	  allocInfo.memoryTypeIndex = vulkanDevice->getMemoryType(memReq.memoryTypeBits, 
	  				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
					VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	  	/*
	  MapMemoryTypeToIndex(memReq.memoryTypeBits,
						   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
						   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						   &allocInfo.memoryTypeIndex);
		*/
	  // Allocate memory for the buffer
	  VkDeviceMemory deviceMemory;
	  CALL_VK(vkAllocateMemory(mDevice, &allocInfo, nullptr, &deviceMemory));
	
	  void* data;
	  CALL_VK(vkMapMemory(mDevice, deviceMemory, 0, allocInfo.allocationSize, 0, &data));
	  memcpy(data, _vertexData_buf, _vertexData_size);
	  vkUnmapMemory(mDevice, deviceMemory);
	
	  CALL_VK(vkBindBufferMemory(mDevice, vertexBuf_, deviceMemory, 0));

#if 1
	  if (_vertexDataPos != nullptr) {delete _vertexDataPos; _vertexDataPos = nullptr;}
#endif
}
void HistogramGraphics::setup_framebuffer(
	VkRenderPass renderPass) {
#if 0
	VkImageView depthView = VK_NULL_HANDLE;
	VkImageView colorImageView = VK_NULL_HANDLE;


    VkImageView attachments[2] = {
        colorImageView, depthView,
    };
    VkFramebufferCreateInfo fbCreateInfo{
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .pNext = nullptr,
        .renderPass = renderPass,
        .layers = 1,
        .attachmentCount = 1,  // 2 if using depth
        .pAttachments = attachments,
        .width = static_cast<uint32_t>(mDstWidth),
        .height = static_cast<uint32_t>(mDstHeight),
    };
    fbCreateInfo.attachmentCount = (depthView == VK_NULL_HANDLE ? 1 : 2);

    CALL_VK(vkCreateFramebuffer(mDevice, &fbCreateInfo, nullptr, &framebuffer_));
#endif
}
void HistogramGraphics::setupDescriptorSetLayout() {
	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = { 		
		// Binding 0: Vertex shader uniform buffer
		vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0),
		// Binding 1: Fragment shader input image
		vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1),
#if 1
		// Binding 2: Vertex shader input image
		vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_VERTEX_BIT, 2)
#endif
	};

	VkDescriptorSetLayoutCreateInfo descriptorLayout = vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings);
	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(mDevice, &descriptorLayout, nullptr, &mDescriptorSetLayout));
	
	VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = vks::initializers::pipelineLayoutCreateInfo(&mDescriptorSetLayout, 1);
	VK_CHECK_RESULT(vkCreatePipelineLayout(mDevice, &pPipelineLayoutCreateInfo, nullptr, &mPipelineLayout));
}
void HistogramGraphics::setupPipeline() {

	// Specify vertex and fragment shader stages
	VkPipelineShaderStageCreateInfo shaderStages[2] = {
		mVulkanExample->loadShader_from_strings_c(source_vert_, VK_SHADER_STAGE_VERTEX_BIT),
		mVulkanExample->loadShader_from_strings_c(source_frag_, VK_SHADER_STAGE_FRAGMENT_BIT),
	};

	// Specify vertex input state
	VkVertexInputBindingDescription vertex_input_bindings{
		.binding = 0,
		.stride = 5 * sizeof(float),
		.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
	};
	VkVertexInputAttributeDescription vertex_input_attributes[2]{
		{
			.binding = 0,
			.location = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = 0,
		},
		{
			.binding = 0,
			.location = 1,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = sizeof(float) * 3,
		}};
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.pNext = nullptr,
		.vertexBindingDescriptionCount = 1,
		.pVertexBindingDescriptions = &vertex_input_bindings,
		.vertexAttributeDescriptionCount = 2,
		.pVertexAttributeDescriptions = vertex_input_attributes,
	};

	// Specify input assembler state
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.pNext = nullptr,
		// .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST,  // frankie, add 
		.primitiveRestartEnable = VK_FALSE,
	};


				VkViewport viewport{
					.minDepth = 0.0f,
					.maxDepth = 1.0f,
					.x = 0,
					.y = 0,
					.width = (float)mDstWidth,
					.height = (float)mDstHeight,
				};

				VkExtent2D scissor_extent = {
					.width = mDstWidth,
					.height = mDstHeight,
					};
				VkRect2D scissor = {
					.extent = scissor_extent,
					.offset = {.x = 0, .y = 0,}
					};
	
	// Specify viewport info
	VkPipelineViewportStateCreateInfo viewportInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.pNext = nullptr,
		.viewportCount = 1,
		.pViewports = &viewport,
		.scissorCount = 1,
		.pScissors = &scissor,
	};
		
	// Specify rasterizer info
	VkPipelineRasterizationStateCreateInfo rasterInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.pNext = nullptr,
		.depthClampEnable = VK_FALSE,
		.rasterizerDiscardEnable = VK_FALSE,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.cullMode = VK_CULL_MODE_NONE,
		.frontFace = VK_FRONT_FACE_CLOCKWISE,
		.depthBiasEnable = VK_FALSE,
		.lineWidth = 1,
	};

	// Specify multisample info
	VkSampleMask sampleMask = ~0u;
	VkPipelineMultisampleStateCreateInfo multisampleInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.pNext = nullptr,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
		.sampleShadingEnable = VK_FALSE,
		.minSampleShading = 0,
		.pSampleMask = &sampleMask,
		.alphaToCoverageEnable = VK_FALSE,
		.alphaToOneEnable = VK_FALSE,
	};

	// Specify color blend state
	VkPipelineColorBlendAttachmentState attachmentStates{
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT 
						| VK_COLOR_COMPONENT_G_BIT 
						| VK_COLOR_COMPONENT_B_BIT 
						| VK_COLOR_COMPONENT_A_BIT,
		.blendEnable = VK_FALSE,
		.srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
		.dstColorBlendFactor = VK_BLEND_FACTOR_ONE, // VK_BLEND_FACTOR_ZERO,
		.colorBlendOp = VK_BLEND_OP_ADD,
		.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
		.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO, // VK_BLEND_FACTOR_ONE, // VK_BLEND_FACTOR_ZERO,
		.alphaBlendOp = VK_BLEND_OP_ADD,
	};
	attachmentStates.blendEnable = VK_TRUE;

	VkPipelineColorBlendStateCreateInfo colorBlendInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.pNext = nullptr,
		.logicOpEnable = VK_FALSE,
		.logicOp = VK_LOGIC_OP_COPY,
		.attachmentCount = 1,
		.pAttachments = &attachmentStates,
		.flags = 0,
	};

	// No dynamic state in that tutorial
	VkPipelineDynamicStateCreateInfo dynamicStateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.pNext = nullptr,
		.dynamicStateCount = 0,
		.pDynamicStates = nullptr};

	// Create the pipeline
	VkGraphicsPipelineCreateInfo pipelineCreateInfo{
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.stageCount = 2,
		.pStages = shaderStages,
		.pVertexInputState = &vertexInputInfo,
		.pInputAssemblyState = &inputAssemblyInfo,
		.pTessellationState = nullptr,
		.pViewportState = &viewportInfo,
		.pRasterizationState = &rasterInfo,
		.pMultisampleState = &multisampleInfo,
		.pDepthStencilState = nullptr,
		.pColorBlendState = &colorBlendInfo,
		.pDynamicState = &dynamicStateInfo,
		.layout = mPipelineLayout,
		.renderPass = renderPass_,
		.subpass = 0,
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = 0,
	};

	VK_CHECK_RESULT(vkCreateGraphicsPipelines(
		mDevice, mPipelineCache, 1, &pipelineCreateInfo, nullptr,&mPipeline));
/** frankie, note, here the pipeline layout shoud be consistent with the shader (ie binding point ), otherwise this pipeline creation will fail !!! */

}
void HistogramGraphics::setupDescriptorPool() {
	// create descriptor pool
	std::vector<VkDescriptorPoolSize> poolSizes = {
		// Graphics pipelines uniform buffers 
		vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1),
		// Graphics pipelines image samplers
		vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2),
		vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2),
	};
	VkDescriptorPoolCreateInfo descriptorPoolInfo = vks::initializers::descriptorPoolCreateInfo(poolSizes, 3);
	VK_CHECK_RESULT(vkCreateDescriptorPool(mDevice, &descriptorPoolInfo, nullptr, &mDescriptorPool));
}

void HistogramGraphics::setupDescriptorSet() {

	VkDescriptorSetAllocateInfo allocInfo =
		vks::initializers::descriptorSetAllocateInfo(mDescriptorPool, &mDescriptorSetLayout, 1);
	VK_CHECK_RESULT(vkAllocateDescriptorSets(mDevice, &allocInfo, &mDescriptorSet));
	
	std::vector<VkWriteDescriptorSet> baseImageWriteDescriptorSets = {
		vks::initializers::writeDescriptorSet(mDescriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &uniformBufferVS.descriptor),
		vks::initializers::writeDescriptorSet(mDescriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &textureColorMap.descriptor),
#if 1
		vks::initializers::writeDescriptorSet(mDescriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, &mTextureSourceScaled.descriptor)
#endif
	};
	vkUpdateDescriptorSets(mDevice, baseImageWriteDescriptorSets.size(), baseImageWriteDescriptorSets.data(), 0, nullptr);
}
void HistogramGraphics::setupCommandPool() {
	// Separate command pool as queue family for compute may be different than graphics
	VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.queueFamilyIndex = mQueueFamilyIndex;
	cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	VK_CHECK_RESULT(vkCreateCommandPool(mDevice, &cmdPoolInfo, nullptr, &mCommandPool));
}
void HistogramGraphics::setupFence() {
	// Fence for compute CB sync
	VkFenceCreateInfo fenceCreateInfo = vks::initializers::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
	VK_CHECK_RESULT(vkCreateFence(mDevice, &fenceCreateInfo, nullptr, &mFence));

	// We need to create a semaphore to be able to wait, in the main loop, for our
	// framebuffer to be available for us before drawing.
	VkSemaphoreCreateInfo semaphoreCreateInfo{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
	};
	CALL_VK(vkCreateSemaphore(mDevice, &semaphoreCreateInfo, nullptr,&semaphore_));
}
void HistogramGraphics::buildCommandBuffer() {
	// Record a command buffer that just clear the screen
	// 1 command buffer draw in 1 framebuffer
	// In our case we need 2 command as we have 2 framebuffer
	cmdBufferLen_ = mSwapChain->swapchainLength_;

	cmdBuffer_ = new VkCommandBuffer[cmdBufferLen_];
	DLOGD( "HistogramGraphics::%s, cmdBufferLen_ = %d \r\n", __func__, cmdBufferLen_);

#if 0
	VkCommandBufferAllocateInfo cmdBufferCreateInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext = nullptr,
		.commandPool = mCommandPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = cmdBufferLen_,
	};
	CALL_VK(vkAllocateCommandBuffers(mDevice, &cmdBufferCreateInfo, cmdBuffer_));
#else
	for (int bufferIndex = 0; bufferIndex < mSwapChain->swapchainLength_; bufferIndex++) {
		VkCommandBuffer commandBuffer;
		VkCommandBufferAllocateInfo cmdBufAllocateInfo =
			vks::initializers::commandBufferAllocateInfo(mCommandPool,VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
		VK_CHECK_RESULT(vkAllocateCommandBuffers(mDevice, &cmdBufAllocateInfo, &commandBuffer));
		cmdBuffer_[bufferIndex] = commandBuffer;
	}
#endif

	VkClearValue clearVals{ .color.float32 = {0.0f, 0.34f, 0.90f, 1.0f}};
	
	VkClearColorValue defaultClearColor = { { 0.0f, 0.0f, 0.0f, 1.0f } };
	clearVals.color = defaultClearColor;

	VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
	renderPassBeginInfo.renderPass = renderPass_;
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = mSwapChain->displaySize_.width;
	renderPassBeginInfo.renderArea.extent.height = mSwapChain->displaySize_.height;
	renderPassBeginInfo.clearValueCount = 1;
	renderPassBeginInfo.pClearValues = &clearVals;

	for (int bi = 0; bi < mSwapChain->swapchainLength_; bi++) {

		DLOGD( "HistogramGraphics::%s, cmdBuffer_[%d] %s ... \r\n", __func__, 
			bi, cmdBuffer_[bi] != VK_NULL_HANDLE ? "!= VK_NULL_HANDLE" : "== VK_NULL_HANDLE");
		VkCommandBuffer commandBuffer = cmdBuffer_[bi];

		// Set target frame buffer
		renderPassBeginInfo.framebuffer = mSwapChain->framebuffers_[bi];

		// We start by creating and declare the "beginning" our command buffer
		VkCommandBufferBeginInfo cmdBufferBeginInfo = vks::initializers::commandBufferBeginInfo();
		CALL_VK(vkBeginCommandBuffer(commandBuffer, &cmdBufferBeginInfo));

		//DLOGD( "HistogramGraphics::%s,%d \r\n", __func__, __LINE__);
		// transition the buffer into color attachment
		vks::tools::setImageLayout_(commandBuffer, mSwapChain->displayImages_[bi],
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
		//DLOGD( "HistogramGraphics::%s,%d \r\n", __func__, __LINE__);
		
		// Now we start a renderpass. Any draw command has to be recorded in a
		// renderpass
		CALL_VK0(vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE));
		//DLOGD( "HistogramGraphics::%s,%d \r\n", __func__, __LINE__);

		// Bind what is necessary to the command buffer
		CALL_VK0(vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline));
		//DLOGD( "HistogramGraphics::%s,%d \r\n", __func__, __LINE__);

		CALL_VK0(vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0, 1, &mDescriptorSet, 0, nullptr));
		//DLOGD( "HistogramGraphics::%s,%d \r\n", __func__, __LINE__);

		VkDeviceSize offset = 0;
		CALL_VK0(vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuf_, &offset));
		//DLOGD( "HistogramGraphics::%s,%d \r\n", __func__, __LINE__);

		// Draw Triangle
		CALL_VK0(vkCmdDraw(commandBuffer, primitive_count, 1, 0, 0));
		//DLOGD( "HistogramGraphics::%s,%d \r\n", __func__, __LINE__);

		CALL_VK0(vkCmdEndRenderPass(commandBuffer));
		//DLOGD( "HistogramGraphics::%s,%d \r\n", __func__, __LINE__);

		vks::tools::setImageLayout_(commandBuffer, mSwapChain->displayImages_[bi],
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
		//DLOGD( "HistogramGraphics::%s,%d \r\n", __func__, __LINE__);

		CALL_VK(vkEndCommandBuffer(commandBuffer));
		DLOGD( "HistogramGraphics::%s, bufferIndex = %d done \r\n", __func__, bi);

	}

}


// external 
/*
mHistogramGraphics->mPhysicalDevice = physicalDevice;
mHistogramGraphics->mDevice = device;
mHistogramGraphics->mPipelineCache = pipelineCache;
*/
void HistogramGraphics::prepare() {
	vks::VulkanDevice *vulkanDevice = mVulkanExample->vulkanDevice;
	mQueueFamilyIndex = vulkanDevice->queueFamilyIndices.graphics;
	// Get a graphics queue from the device
	vkGetDeviceQueue(mDevice, vulkanDevice->queueFamilyIndices.graphics, 0, &mQueue);

	uint32_t w_ = 1080;
	uint32_t h_ = 2340;

	mSourceTextureWidth = w_;
	mSourceTextureHeight = h_;

	mTextureScaled_width = ALIGN_SIZE(w_, 16);
	mTextureScaled_height = ALIGN_SIZE(h_, 16);
#if 1
	mTextureScaled_width = ALIGN_SIZE(w_/4, 16);
	mTextureScaled_height = ALIGN_SIZE(h_/4, 16);
#endif

	primitive_count = 6;  // 2 triangles !
	primitive_count = mTextureScaled_width * mTextureScaled_height;  // for point primitive !

#if USING_DISPLAY_WINDOW
	#if 0
	mDisplayWindow_ = new football::DisplayWindow_(FootballPPTester_special_SURFACE_NAME, 256, 1, AIMAGE_FORMAT_RGBA_8888);
	#else
	mDisplayWindow_ = new football::DisplayWindow_(FootballPPTester_special_SURFACE_NAME, 
		HistogramDataReader_WIDTH, HistogramDataReader_HEIGHT, AIMAGE_FORMAT_RGBA_8888); // test with the same size as texture !
	#endif
	mDisplayWindow_->setPos(0, 400);
	mDisplayWindow_->show();
	mPlatformWindow_ = mDisplayWindow_->getANativeWindow();
#else
	mHistogramDataReader = new HistogramDataReader(HistogramDataReader_WIDTH, HistogramDataReader_HEIGHT);
	mPlatformWindow_ = mHistogramDataReader->getANativeWindow();
#endif

	DLOGD( "HistogramGraphics::%s texture size = %4d x %4d \r\n", __func__, 
		mTextureScaled_width, mTextureScaled_height);

	if (mHistogramDataReader != nullptr) {
		mHistogramDataReader->setStatisticSourceImageSize(
			mTextureScaled_width, mTextureScaled_height);
	}

	mDstWidth = 256;
	mDstHeight = 1;
	mDstWidth = ANativeWindow_getWidth(mPlatformWindow_);
	mDstHeight = ANativeWindow_getHeight(mPlatformWindow_);
	DLOGD( "HistogramGraphics::%s window size = %4d x %4d \r\n", __func__, mDstWidth, mDstHeight);

{
	VulkanSwapchainInfo::InitInfo info = {
		.window_ = mPlatformWindow_,
		.instance_ = mVulkanExample->instance,
		.gpuDevice_ = vulkanDevice->physicalDevice,
		.device_ = vulkanDevice->logicalDevice,
		.queueFamilyIndex_ = vulkanDevice->queueFamilyIndices.graphics,
	};
	mSwapChain->initSurface(info);
	
	VkFormat displayFormat_ = VK_FORMAT_R8G8B8A8_UNORM;
	displayFormat_ = mSwapChain->displayFormat_;
	setup_renderpass(displayFormat_);

	mSwapChain->createFrameBuffers(renderPass_);

	mDstWidth = mSwapChain->displaySize_.width;
	mDstHeight = mSwapChain->displaySize_.height;
	DLOGD( "HistogramGraphics::%s mSwapChain size = %4d x %4d \r\n", __func__, mDstWidth, mDstHeight);
}

	setup_target_texture();

	setup_buffer();
	
	//setup_framebuffer(renderPass_);

	setupDescriptorSetLayout();

	setupPipeline();

	setupDescriptorPool();

	setupDescriptorSet();

	setupCommandPool();

	setupFence();

	buildCommandBuffer();


}
void HistogramGraphics::appendCommandBuffers(const VkCommandBuffer commandBuffer) {
#if 1

#endif
}

#define WAIT_TIMEOUT_millis(m) (m*1000*1000)
	// when using camera,  100ms is not enought for vkWaitForFences() timeout

void HistogramGraphics::draw() {
	// submit command buffer to the queue , then wait for the rendering result !
	uint32_t nextIndex = 0;
	// Get the framebuffer index we should draw in
	CALL_VK(vkAcquireNextImageKHR(mDevice, mSwapChain->swapchain_, UINT64_MAX, semaphore_, VK_NULL_HANDLE, &nextIndex));
	DLOGD( "%s, nextIndex = %d dest size = %4d x %4d \r\n", __func__, nextIndex, mDstWidth, mDstHeight);

	CALL_VK(vkResetFences(mDevice, 1, &mFence));

	VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	VkSubmitInfo submit_info = {	.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
								.pNext = nullptr,
								.waitSemaphoreCount = 1,
								.pWaitSemaphores = &semaphore_,
								.pWaitDstStageMask = &waitStageMask,
								.commandBufferCount = 1,
								.pCommandBuffers = &cmdBuffer_[nextIndex],
								.signalSemaphoreCount = 0,
								.pSignalSemaphores = nullptr};
	CALL_VK(vkQueueSubmit(mQueue, 1, &submit_info, mFence));
	CALL_VK(vkWaitForFences(mDevice, 1, &mFence, VK_TRUE, WAIT_TIMEOUT_millis(1000))); // in units of nanoseconds.

	LOGI("Drawing frames......");

	VkResult result;
	VkPresentInfoKHR presentInfo{
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.pNext = nullptr,
		.swapchainCount = 1,
		.pSwapchains = &(mSwapChain->swapchain_),
		.pImageIndices = &nextIndex,
		.waitSemaphoreCount = 0,
		.pWaitSemaphores = nullptr,
		.pResults = &result,
	};
	vkQueuePresentKHR(mQueue, &presentInfo);

}
void HistogramGraphics::updateSource_VkDescriptorImageInfo(
	ImportedTexture *importedTexture,
	uint32_t w_, uint32_t h_, VkDescriptorImageInfo *descriptor) {

	DLOGD( "HistogramGraphics::%s size = %4d x %4d \r\n", __func__, w_, h_);

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
		imageBlit.dstOffsets[1].x = int32_t(mTextureScaled_width >> 0);
		imageBlit.dstOffsets[1].y = int32_t(mTextureScaled_height >> 0);
		imageBlit.dstOffsets[1].z = 1;
	
		vkCmdBlitImage(layoutCmd, 
			importedTexture->image_, VK_IMAGE_LAYOUT_GENERAL,
			mTextureSourceScaled.image, VK_IMAGE_LAYOUT_GENERAL,
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
{
	std::vector<VkWriteDescriptorSet> computeWriteDescriptorSets = {			
		//vks::initializers::writeDescriptorSet(mDescriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, descriptor),
		vks::initializers::writeDescriptorSet(mDescriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, descriptor),
	};
	vkUpdateDescriptorSets(mDevice, computeWriteDescriptorSets.size(), computeWriteDescriptorSets.data(), 0, NULL);
}
#endif

}

void HistogramGraphics::setDebugWindow(int enable) {
	if (mHistogramDataReader != nullptr) {
		mHistogramDataReader->setDebugWindow(enable);
	}
}





};


