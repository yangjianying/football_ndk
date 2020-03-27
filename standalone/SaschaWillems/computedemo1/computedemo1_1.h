#ifndef __COMPUTESHADERDEMO1_H___
#define __COMPUTESHADERDEMO1_H___

#include <vulkan/vulkan.h>
#include "vulkanexamplebase.h"
#include "VulkanTexture.hpp"
#include "VulkanBuffer.hpp"

#include "utils/ANativeWindowUtils.h"

namespace computedemo1_1 {

#define USE_VulkanExample_Parasite_target 1 // 0 // 1
class VulkanExample_Parasite;


/////////////////////////////////////////////////////////////////////////////////////////////

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

		//
		INIT_COMPUTE_MasiaEO,
		INIT_COMPUTE_BT2446_m1_TMO,

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
	void impl1_setBHE_tuning(float t0, float t1);
	void setScreenMode(int mode_);

	VulkanExample_Parasite *mParasite = nullptr;

};











};


#endif

