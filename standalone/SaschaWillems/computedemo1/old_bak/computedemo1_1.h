#ifndef __COMPUTESHADERDEMO1_H___
#define __COMPUTESHADERDEMO1_H___

#include <vulkan/vulkan.h>
#include "vulkanexamplebase.h"
#include "VulkanTexture.hpp"
#include "VulkanBuffer.hpp"

#include "utils/ANativeWindowUtils.h"

namespace computedemo1_1 {

// Vertex layout for this example
struct Vertex {
	float pos[3];
	float uv[2];
};


struct ImportedTexture {
	VkImage image_ = VK_NULL_HANDLE;
	VkDeviceMemory imageMem_ = VK_NULL_HANDLE;
	VkImageView imageView_ = VK_NULL_HANDLE;
	VkSampler sampler_ = VK_NULL_HANDLE;
};




#define USE_HistogramCompute_target 1 // 0 // 1


class HistogramGrapher ;

class HistogramCompute;  // frankie, forward declare

class HistogramGraphics;


class VulkanExample : public VulkanExampleBase
{
private:
	vks::Texture2D textureColorMap;
	vks::Texture2D textureComputeTarget;
public:
	struct {
		VkPipelineVertexInputStateCreateInfo inputState;
		std::vector<VkVertexInputBindingDescription> bindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	} vertices;

	// Resources for the graphics part of the example
	struct {
		VkDescriptorSetLayout descriptorSetLayout;	// Image display shader binding layout
		VkDescriptorSet descriptorSetPreCompute;	// Image display shader bindings before compute shader image manipulation
		VkDescriptorSet descriptorSetPostCompute;	// Image display shader bindings after compute shader image manipulation
		VkPipeline pipeline;						// Image display pipeline
		VkPipelineLayout pipelineLayout;			// Layout of the graphics pipeline
	} graphics;

	// Resources for the compute part of the example
	struct Compute {
		VkQueue queue;								// Separate queue for compute commands (queue family may differ from the one used for graphics)
		VkCommandPool commandPool;					// Use a separate command pool (queue family may differ from the one used for graphics)
		VkCommandBuffer commandBuffer;				// Command buffer storing the dispatch commands and barriers
		VkFence fence;								// Synchronization fence to avoid rewriting compute CB if still in use
		VkDescriptorSetLayout descriptorSetLayout;	// Compute shader binding layout
		VkDescriptorSet descriptorSet;				// Compute shader bindings
		VkPipelineLayout pipelineLayout;			// Layout of the compute pipeline
		std::vector<VkPipeline> pipelines;			// Compute pipelines for image filters
		int32_t pipelineIndex = 0;					// Current image filtering compute pipeline index
		uint32_t queueFamilyIndex;					// Family index of the graphics queue, used for barriers
	} compute;

	vks::Buffer vertexBuffer;
	vks::Buffer indexBuffer;
	uint32_t indexCount;

	vks::Buffer uniformBufferVS;

	struct {
		glm::mat4 projection;
		glm::mat4 model;
	} uboVS;

	int vertexBufferSize;

	std::vector<std::string> shaderNames;
	std::vector<const char *> shaderSources;  // frankie, add

	VulkanExample();
	
	~VulkanExample();

	// Prepare a texture target that is used to store compute shader calculations
	void prepareTextureTarget(vks::Texture *tex, uint32_t width, uint32_t height, VkFormat format);

	void loadAssets();

	void buildCommandBuffers_for_testscreen();

	void buildCommandBuffers_for_fullscreen();

	void buildCommandBuffers();

#if 1
	void buildComputeCommandBuffer();
#endif

	// Setup vertices for a single uv-mapped quad
	void generateQuad();

	void setupVertexDescriptions();

	void setupDescriptorPool();

	void setupDescriptorSetLayout();

	void setupDescriptorSet();

	void preparePipelines();

	// Find and create a compute capable device queue
	void getComputeQueue();

	void prepareCompute();

	// Prepare and initialize uniform buffer containing shader uniforms
	void prepareUniformBuffers();

	void updateUniformBuffers();

	void prepare();

	virtual void render();

	void draw();

	virtual void viewChanged();

	virtual void OnUpdateUIOverlay(vks::UIOverlay *overlay);

	// frankie, add below
	ImportedTexture importTexture;
	int importAHardwareBufferAsTexture(AHardwareBuffer *hardwareBuffer);
	void deleteImportTexture(int flags = 0x01);


	enum {
		INIT_TEST = 0,
		INIT_HISTOGRAM_GRAPHIC,
		
		INIT_COMPUTE_START, 
		INIT_COMPUTE_AGC = INIT_COMPUTE_START,
		INIT_COMPUTE_BHE,
		INIT_COMPUTE_AGC_BHE,
		INIT_COMPUTE_END,
		
	};
	int mInitFlag = INIT_TEST;

	enum {
		SCREEN_MODE_FULL = 0,
		SCREEN_MODE_HALF,
		SCREEN_MODE_MAX,
	};
	int mScreenMode = 0;  // 0 - full, 1 - half
	void setExtraInitFlag(int flag) { mInitFlag = flag; }
	void InitExtra();
	void setDebugWindow(int enable);
	void impl1_setBHE_factor(float f0, float f1);
	void setScreenMode(int mode_);

	HistogramCompute *mHistogramCompute = nullptr;
	HistogramGraphics *mHistogramGraphics = nullptr;

};

#ifdef ALIGN_SIZE
#error ALIGN_SIZE is defined in somewhere previously !!!
#endif
#define ALIGN_SIZE(x, a) ((((x)+((a)-1))/(a))* (a))

#define IndirectStats__c_HIST_BIN_SIZE 512 // 256 // 512 // 1024(will result the buffer overflow !!! ) //

#define __stringify(x)  __stringify_1(x)
#define __stringify_1(x)  #x

struct IndirectStats__c{
	//
	uint32_t drawCount;						// Total number of indirect draw counts to be issued
	uint32_t lodCount[IndirectStats__c_HIST_BIN_SIZE];	// Statistics for number of draws per LOD level (written by compute shader)
	uint32_t luminance_sumed = 0;

	//
	uint32_t accHist[IndirectStats__c_HIST_BIN_SIZE];
	float accHistNormal[IndirectStats__c_HIST_BIN_SIZE];
	uint32_t accCount = 0;
	uint32_t pointIndexFlags[8];
	uint32_t pointIndex[8];	

	//
	uint32_t bhe_lut[IndirectStats__c_HIST_BIN_SIZE];
	float bhe_lut0[IndirectStats__c_HIST_BIN_SIZE];
	float bhe_lut1[IndirectStats__c_HIST_BIN_SIZE];

	//
	uint32_t lumi_counter = 0;
	uint32_t luminance_mean = 0;
	uint32_t luminance_std = 0;
	float lumi_mean = 0.0;
	float lumi_std = 0.0;
	uint32_t type = 0;  // 0, 1, 2, 3
	float y1 = 0.0;
	float y2 = 0.0;
	uint32_t agc_lut[IndirectStats__c_HIST_BIN_SIZE];
	uint32_t agc_counter = 0;
	
	uint32_t merged_lut[IndirectStats__c_HIST_BIN_SIZE];
	float bhe_factor0 = 0.0f;
	float bhe_factor1 = 0.0f;

	//
	double lodCount_d[IndirectStats__c_HIST_BIN_SIZE];
	int bin_count = IndirectStats__c_HIST_BIN_SIZE;

	enum {
		PRINT_HIST = 0x01,
		PRINT_ACC_HIST = 0x02,
		PRINT_AGC_LUT = 0x04,
		PRINT_MERGED_LUT = 0x08,
	};
	void print(uint32_t flags = PRINT_HIST);
	void print_u32();
};
struct stdTempBuffer_c {
	stdTempBuffer_c(int w, int h) { mWidth = w; mHeight = h; length = sizeof(float)*w*h; buffer = (float *)malloc(length); }
	~stdTempBuffer_c() { if (buffer != nullptr) { free(buffer); buffer = nullptr; } }
	int getLength() { return length; }
	float *getBuffer() { return buffer; }
	float *buffer = nullptr;
	int mWidth = 0;
	int mHeight = 0;
	int length = 0;
};

class HistogramGrapher {
public:
	static HistogramGrapher *create();

	HistogramGrapher() {}
	virtual ~HistogramGrapher() {}

	virtual void setBHE_factor(float f0, float f1) = 0;
	virtual void draw_stats(IndirectStats__c *stats) = 0;
};

class HistogramCompute {
public:
	HistogramCompute(VulkanExample *vulkanExample);
	~HistogramCompute();

	//
	void getComputeQueue();
	void createTextureTarget(vks::Texture *tex, uint32_t w_, uint32_t h_, VkFormat format, VkImageUsageFlags extra_flags);
	void createDescriptorPool();

	void buildCommandBuffers();
	
	// external 
	void prepare();

	vks::Texture2D & getTextureTarget() {
		return mTextureTarget;
	}
	void appendCommandBuffers(const VkCommandBuffer commandBuffer);

	void draw();

	void updateSource_VkDescriptorImageInfo(
		ImportedTexture *importedTexture,
		uint32_t w_, uint32_t h_, VkDescriptorImageInfo *descriptor);

	void setDebugWindow(int enable);

	void setBHE_factor(float f0, float f1);

	VulkanExample *mVulkanExample = nullptr;
	uint32_t mLutSection = 0;

	vks::Texture2D mSourceAlignScaled;
	vks::Texture2D mSourceAligned;
	vks::Texture2D mTextureTarget; // image after processing !

	vks::Buffer indirectDrawCountBuffer;
	
	vks::Buffer stdTempBuffer;

	// Indirect draw statistics (updated via compute)
	IndirectStats__c indirectStats;

	stdTempBuffer_c *mStdTempBuffer = nullptr;

	VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
	VkDevice mDevice = VK_NULL_HANDLE;
	VkPipelineCache mPipelineCache = VK_NULL_HANDLE;

	uint32_t mQueueFamilyIndex = 0;	// Family index of the comute queue, used for barriers
	VkQueue mQueue = VK_NULL_HANDLE; 
	VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;

	uint32_t mSourceTextureWidth = 0;
	uint32_t mSourceTextureHeight = 0;

	uint32_t mScaledWidth = 0;
	uint32_t mScaledHeight = 0;

	uint32_t mSourceAlignedWidth = 0;
	uint32_t mSourceAlignedHeight = 0;

	VkCommandPool mCommandPool = VK_NULL_HANDLE; 	// Use a separate command pool (queue family may differ from the one used for graphics)
	VkCommandBuffer mCommandBuffer = VK_NULL_HANDLE; 
	
	VkFence mFence = VK_NULL_HANDLE;		// Synchronization fence to avoid rewriting compute CB if still in use

	//
	VkDescriptorSetLayout mDescriptorSetLayout = VK_NULL_HANDLE; // Compute shader binding layout
	VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;	// Layout of the compute pipeline
	VkDescriptorSet mDescriptorSet = VK_NULL_HANDLE;	// Compute shader bindings


	//
	enum {
		COMP_CONV1 = 0, 

		COMP_CAL_HIST_MEAN,
		COMP_GEN_ACC_HIST, 
		COMP_GEN_BHE_LUT, 
		COMP_CAL_STD,
		COMP_GEN_AGC_LUT,
		COMP_APPLY_LUT,
	};
	struct Shader_ {
		int stage = 0;
		std::string name = "";
		const char *source;
	};
	football::MapKeyedInt<Shader_> mShaders; // frankie, add
	football::MapKeyedInt<VkPipeline> mPipelines; // Compute pipelines for image filters	// std::vector<VkPipeline>

	HistogramGrapher *mHistogramGrapher_ = nullptr;

	float mBHE_factor0 = 0.3f;
	float mBHE_factor1 = 0.8f;
	std::mutex mBHE_factor_mutex_;

};



class VulkanSwapchainInfo {
public:
	VulkanSwapchainInfo();
	~VulkanSwapchainInfo();

	struct InitInfo {
		ANativeWindow* window_ = nullptr;
		VkInstance instance_ = VK_NULL_HANDLE;
		VkPhysicalDevice gpuDevice_ = VK_NULL_HANDLE;
		VkDevice device_ = VK_NULL_HANDLE;
		uint32_t queueFamilyIndex_ = 0;
	};
	void initSurface(InitInfo &info);

	void createSurface(ANativeWindow* platformWindow);
	void destroySurface() ;
	void createSwapChain();
	
	void createFrameBuffers(VkRenderPass& renderPass,
                        VkImageView depthView = VK_NULL_HANDLE) ;

	// ext
	InitInfo mInfo;

	// internal
	VkSurfaceKHR surface_ = VK_NULL_HANDLE;

	VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;
	uint32_t swapchainLength_ = 0;

	VkExtent2D displaySize_;
	VkFormat displayFormat_;

	// array of frame buffers and views
	VkFramebuffer* framebuffers_ = nullptr;
	VkImage* displayImages_ = nullptr;
	VkImageView* displayViews_ = nullptr;
};

class HistogramDataReader;

class HistogramGraphics {
public:
	HistogramGraphics(VulkanExample *vulkanExample);
	~ HistogramGraphics();

	const std::string getAssetPath() ;

	void setup_renderpass(VkFormat displayFormat_) ;
	void createTextureTarget(vks::Texture *tex, uint32_t w_, uint32_t h_, VkFormat format, VkImageUsageFlags extra_flags);
	void setup_target_texture() ;
	void setup_buffer() ;
	void setup_framebuffer(
		VkRenderPass renderPass);
	void setupDescriptorSetLayout() ;
	void setupPipeline() ;
	void setupDescriptorPool() ;
	void setupDescriptorSet() ;
	void setupCommandPool() ;
	void setupFence();
	void buildCommandBuffer() ;

	// external 
	void prepare();
	
	void appendCommandBuffers(const VkCommandBuffer commandBuffer);

	void draw();

	void updateSource_VkDescriptorImageInfo(
		ImportedTexture *importedTexture,
		uint32_t w_, uint32_t h_, VkDescriptorImageInfo *descriptor);
	
	void setDebugWindow(int enable);

	//
	VulkanExample *mVulkanExample = nullptr;

	VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
	VkDevice mDevice = VK_NULL_HANDLE;
	VkPipelineCache mPipelineCache = VK_NULL_HANDLE;

	uint32_t mQueueFamilyIndex = 0;	// Family index of the graphics queue, used for barriers
	VkQueue mQueue = VK_NULL_HANDLE; 

	// our objects :
	VkRenderPass renderPass_;

	HistogramDataReader *mHistogramDataReader = nullptr;
	football::DisplayWindow_ *mDisplayWindow_ = nullptr;
	
	ANativeWindow *mPlatformWindow_ = nullptr;
	VulkanSwapchainInfo *mSwapChain = nullptr;
	VkCommandBuffer* cmdBuffer_;
	uint32_t cmdBufferLen_;
	VkSemaphore semaphore_;

	class VertexDataPosBuffer {
	public:
		VertexDataPosBuffer(int w_, int h_);
		~VertexDataPosBuffer();
		float *vertex_data = nullptr;
		uint64_t vertex_length = 0;
	};
	VertexDataPosBuffer *_vertexDataPos = nullptr;
	VkBuffer vertexBuf_ = VK_NULL_HANDLE;
	int primitive_count = 0;

	VkFramebuffer framebuffer_ = VK_NULL_HANDLE;

	struct {
		glm::mat4 projection;
		glm::mat4 model;
	} uboVS;
	vks::Buffer uniformBufferVS;

	vks::Texture2D textureColorMap;
	vks::Texture2D mTextureSourceScaled;
	uint32_t mTextureScaled_width = 0;
	uint32_t mTextureScaled_height = 0;

	VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;

	uint32_t mSourceTextureWidth = 0;
	uint32_t mSourceTextureHeight = 0;

	uint32_t mDstWidth = 0;
	uint32_t mDstHeight = 0;

	VkCommandPool mCommandPool = VK_NULL_HANDLE; 	// Use a separate command pool (queue family may differ from the one used for graphics)
	VkCommandBuffer mCommandBuffer = VK_NULL_HANDLE; 
	VkFence mFence = VK_NULL_HANDLE;		// Synchronization fence to avoid rewriting 
	VkDescriptorSetLayout mDescriptorSetLayout = VK_NULL_HANDLE; // shader binding layout
	VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;	// Layout of the pipeline
	VkDescriptorSet mDescriptorSet = VK_NULL_HANDLE;	// shader bindings
	VkPipeline mPipeline;	// pipeline for image filters

};






};


#endif

