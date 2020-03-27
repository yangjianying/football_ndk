#ifndef __COMPUTEDEMO1_1_HISTOGRAMGRAPHICS_H___
#define __COMPUTEDEMO1_1_HISTOGRAMGRAPHICS_H___


#include "computedemo1_1_Parasite.h"

namespace computedemo1_1 {

class HistogramDataReader;


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

class HistogramGraphics  : public VulkanExample_Parasite {
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
	virtual void prepare();

	virtual bool isTextureTargetAvailable() { return true; }
	
	virtual vks::Texture2D & getTextureTarget() { return mTextureSourceScaled; }

	virtual void appendCommandBuffers(const VkCommandBuffer commandBuffer);

	virtual void draw();

	virtual void updateSource_VkDescriptorImageInfo(
		ImportedTexture *importedTexture,
		uint32_t w_, uint32_t h_, VkDescriptorImageInfo *descriptor);
	
	virtual void setDebugWindow(int enable);
	
	virtual void setBHE_factor(float f0, float f1) {}
	virtual void setBHE_tuning(float t0, float t1) {}

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

