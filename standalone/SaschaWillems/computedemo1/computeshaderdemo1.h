#ifndef __COMPUTESHADERDEMO1_H___
#define __COMPUTESHADERDEMO1_H___

#include <vulkan/vulkan.h>
#include "vulkanexamplebase.h"
#include "VulkanTexture.hpp"
#include "VulkanBuffer.hpp"

namespace computeshaderdemo1 {

// Vertex layout for this example
struct Vertex {
	float pos[3];
	float uv[2];
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

	VulkanExample();
	
	~VulkanExample();

	// Prepare a texture target that is used to store compute shader calculations
	void prepareTextureTarget(vks::Texture *tex, uint32_t width, uint32_t height, VkFormat format);

	void loadAssets();

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

	void draw();

	void prepare();

	virtual void render();

	virtual void viewChanged();

	virtual void OnUpdateUIOverlay(vks::UIOverlay *overlay);

};


};


#endif

