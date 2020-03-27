#ifndef __COMPUTEDEMO1_1_HISTOGRAMCOMPUTE_H___
#define __COMPUTEDEMO1_1_HISTOGRAMCOMPUTE_H___

#include "computedemo1_1_Parasite.h"

namespace computedemo1_1 {

class HistogramCompute: public VulkanExample_Parasite {
public:
	HistogramCompute(VulkanExample *vulkanExample);
	virtual ~HistogramCompute();

	//
	void getComputeQueue();
	void createTextureTarget(vks::Texture *tex, uint32_t w_, uint32_t h_, VkFormat format, VkImageUsageFlags extra_flags);
	void createDescriptorPool();

	void buildCommandBuffers();
	
	// external 
	virtual void prepare();

	virtual bool isTextureTargetAvailable() { return true; }

	virtual vks::Texture2D & getTextureTarget() { return mTextureTarget; }

	virtual void appendCommandBuffers(const VkCommandBuffer commandBuffer);

	virtual void draw();

	virtual void updateSource_VkDescriptorImageInfo(
		ImportedTexture *importedTexture,
		uint32_t w_, uint32_t h_, VkDescriptorImageInfo *descriptor);

	virtual void setDebugWindow(int enable);

	virtual void setBHE_factor(float f0, float f1);

	virtual void setBHE_tuning(float t0, float t1);

public:
	uint32_t mLutSection = 0;

	vks::Texture2D mSourceAlignScaled;
	vks::Texture2D mSourceAligned;
	vks::Texture2D mTextureTarget; // image after processing !

	vks::Buffer indirectDrawCountBuffer;
	
	vks::Buffer stdTempBuffer;

	// Indirect draw statistics (updated via compute)
	IndirectStats__c indirectStats;

	stdTempBuffer_c *mStdTempBuffer = nullptr;

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
	float mBHE_tuning0 = 1.0f;
	float mBHE_tuning1 = 1.0f;
	std::mutex mBHE_factor_mutex_;

};

};

#endif

