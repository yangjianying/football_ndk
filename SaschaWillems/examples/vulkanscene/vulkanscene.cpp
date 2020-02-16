/*
* Vulkan Demo Scene 
*
* Don't take this a an example, it's more of a personal playground
*
* Copyright (C) 2016 by Sascha Willems - www.saschawillems.de
*
* Note : Different license than the other examples!
*
* This code is licensed under the Mozilla Public License Version 2.0 (http://opensource.org/licenses/MPL-2.0)
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
#include <glm/gtc/matrix_inverse.hpp>

#include <vulkan/vulkan.h>
#include "vulkanexamplebase.h"
#include "VulkanTexture.hpp"
#include "VulkanModel.hpp"

#define VERTEX_BUFFER_BIND_ID 0
#define ENABLE_VALIDATION false

namespace vulkanscene {

class VulkanExample : public VulkanExampleBase
{
public:

	// Vertex layout for the models
	vks::VertexLayout vertexLayout = vks::VertexLayout({
		vks::VERTEX_COMPONENT_POSITION,
		vks::VERTEX_COMPONENT_NORMAL,
		vks::VERTEX_COMPONENT_UV,
		vks::VERTEX_COMPONENT_COLOR,
	});

	struct DemoModel
	{
		vks::Model model;
		VkPipeline *pipeline;

		void draw(VkCommandBuffer cmdBuffer)
		{
			VkDeviceSize offsets[1] = { 0 };
			vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline);
			vkCmdBindVertexBuffers(cmdBuffer, VERTEX_BUFFER_BIND_ID, 1, &model.vertices.buffer, offsets);
			vkCmdBindIndexBuffer(cmdBuffer, model.indices.buffer, 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(cmdBuffer, model.indexCount, 1, 0, 0, 0);
		}
	};

	std::vector<DemoModel> demoModels;

	struct {
		vks::Buffer meshVS;
	} uniformData;

	struct {
		glm::mat4 projection;
		glm::mat4 model;
		glm::mat4 normal;
		glm::mat4 view;
		glm::vec4 lightPos;
	} uboVS;

	struct
	{
		vks::TextureCubeMap skybox;
	} textures;

	struct {
		VkPipeline logos;
		VkPipeline models;
		VkPipeline skybox;
	} pipelines;

	VkPipelineLayout pipelineLayout;
	VkDescriptorSet descriptorSet;
	VkDescriptorSetLayout descriptorSetLayout;

	glm::vec4 lightPos = glm::vec4(1.0f, 2.0f, 0.0f, 0.0f);

	VulkanExample() : VulkanExampleBase(ENABLE_VALIDATION)
	{
		zoom = -3.75f;
		rotationSpeed = 0.5f;
		rotation = glm::vec3(15.0f, 0.f, 0.0f);
		title = "Vulkan Demo Scene - (c) 2016 by Sascha Willems";
		settings.overlay = true;
	}

	~VulkanExample()
	{
		// Clean up used Vulkan resources 
		// Note : Inherited destructor cleans up resources stored in base class
		vkDestroyPipeline(device, pipelines.logos, nullptr);
		vkDestroyPipeline(device, pipelines.models, nullptr);
		vkDestroyPipeline(device, pipelines.skybox, nullptr);

		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

		uniformData.meshVS.destroy();

		for (auto& model : demoModels) {
			model.model.destroy();
		}

		textures.skybox.destroy();
	}

	void loadAssets()
	{
		fprintf(stderr, "%s,%d \r\n", __FILE__, __LINE__);
		// Models
		std::vector<std::string> modelFiles = { "vulkanscenelogos.dae", "vulkanscenebackground.dae", "vulkanscenemodels.dae", "cube.obj" };
		std::vector<VkPipeline*> modelPipelines = { &pipelines.logos, &pipelines.models, &pipelines.models, &pipelines.skybox };
		for (auto i = 0; i < modelFiles.size(); i++) {
			DemoModel model;
			model.pipeline = modelPipelines[i];
			vks::ModelCreateInfo modelCreateInfo(glm::vec3(1.0f), glm::vec3(1.0f), glm::vec3(0.0f));
			if (modelFiles[i] != "cube.obj") {
				modelCreateInfo.center.y += 1.15f;
			}
			model.model.loadFromFile(getAssetPath() + "models/" + modelFiles[i], vertexLayout, &modelCreateInfo, vulkanDevice, queue);
			demoModels.push_back(model);
		}
		fprintf(stderr, "%s,%d \r\n", __FILE__, __LINE__);
		// Textures
		textures.skybox.loadFromFile(getAssetPath() + "textures/cubemap_vulkan.ktx", VK_FORMAT_R8G8B8A8_UNORM, vulkanDevice, queue);
		fprintf(stderr, "%s,%d \r\n", __FILE__, __LINE__);
	}

	void buildCommandBuffers()
	{
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

		for (int32_t i = 0; i < drawCmdBuffers.size(); ++i)
		{
			renderPassBeginInfo.framebuffer = frameBuffers[i];

			VK_CHECK_RESULT(vkBeginCommandBuffer(drawCmdBuffers[i], &cmdBufInfo));

			vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			VkViewport viewport = vks::initializers::viewport((float)width, (float)height, 0.0f, 1.0f);
			vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);

			VkRect2D scissor = vks::initializers::rect2D(width, height, 0, 0);
			vkCmdSetScissor(drawCmdBuffers[i], 0, 1, &scissor);

			vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, NULL);

			for (auto model : demoModels) {
				model.draw(drawCmdBuffers[i]);
			}

			drawUI(drawCmdBuffers[i]);

			vkCmdEndRenderPass(drawCmdBuffers[i]);

			VK_CHECK_RESULT(vkEndCommandBuffer(drawCmdBuffers[i]));
		}
	}

	void setupDescriptorPool()
	{
		// Example uses one ubo and one image sampler
		std::vector<VkDescriptorPoolSize> poolSizes =
		{
			vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2),
			vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
		};

		VkDescriptorPoolCreateInfo descriptorPoolInfo =
			vks::initializers::descriptorPoolCreateInfo(
				poolSizes.size(),
				poolSizes.data(),
				2);

		VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &descriptorPool));
	}

	void setupDescriptorSetLayout()
	{
		std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings =
		{
			// Binding 0 : Vertex shader uniform buffer
			vks::initializers::descriptorSetLayoutBinding(
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				VK_SHADER_STAGE_VERTEX_BIT,
				0),
			// Binding 1 : Fragment shader color map image sampler
			vks::initializers::descriptorSetLayoutBinding(
				VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				VK_SHADER_STAGE_FRAGMENT_BIT,
				1)
		};

		VkDescriptorSetLayoutCreateInfo descriptorLayout =
			vks::initializers::descriptorSetLayoutCreateInfo(
				setLayoutBindings.data(),
				setLayoutBindings.size());

		VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorLayout, nullptr, &descriptorSetLayout));

		VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
			vks::initializers::pipelineLayoutCreateInfo(
				&descriptorSetLayout,
				1);

		VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, nullptr, &pipelineLayout));
	}

	void setupDescriptorSet()
	{
		VkDescriptorSetAllocateInfo allocInfo =
			vks::initializers::descriptorSetAllocateInfo(
				descriptorPool,
				&descriptorSetLayout,
				1);

		VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet));

		// Cube map image descriptor
		VkDescriptorImageInfo texDescriptorCubeMap =
			vks::initializers::descriptorImageInfo(
				textures.skybox.sampler,
				textures.skybox.view,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		std::vector<VkWriteDescriptorSet> writeDescriptorSets =
		{
			// Binding 0 : Vertex shader uniform buffer
			vks::initializers::writeDescriptorSet(
				descriptorSet,
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				0,
				&uniformData.meshVS.descriptor),
			// Binding 1 : Fragment shader image sampler
			vks::initializers::writeDescriptorSet(
				descriptorSet,
				VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				1,
				&texDescriptorCubeMap)
		};

		vkUpdateDescriptorSets(device, writeDescriptorSets.size(), writeDescriptorSets.data(), 0, NULL);
	}

	void preparePipelines()
	{
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
			vks::initializers::pipelineInputAssemblyStateCreateInfo(
				VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
				0,
				VK_FALSE);

		VkPipelineRasterizationStateCreateInfo rasterizationState =
			vks::initializers::pipelineRasterizationStateCreateInfo(
				VK_POLYGON_MODE_FILL,
				VK_CULL_MODE_BACK_BIT,
				VK_FRONT_FACE_CLOCKWISE,
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

		// Pipeline for the meshes (armadillo, bunny, etc.)
		// Load shaders
		std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

		VkGraphicsPipelineCreateInfo pipelineCreateInfo =
			vks::initializers::pipelineCreateInfo(
				pipelineLayout,
				renderPass,
				0);

		pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
		pipelineCreateInfo.pRasterizationState = &rasterizationState;
		pipelineCreateInfo.pColorBlendState = &colorBlendState;
		pipelineCreateInfo.pMultisampleState = &multisampleState;
		pipelineCreateInfo.pViewportState = &viewportState;
		pipelineCreateInfo.pDepthStencilState = &depthStencilState;
		pipelineCreateInfo.pDynamicState = &dynamicState;
		pipelineCreateInfo.stageCount = shaderStages.size();
		pipelineCreateInfo.pStages = shaderStages.data();

		VkVertexInputBindingDescription vertexInputBinding =
			vks::initializers::vertexInputBindingDescription(VERTEX_BUFFER_BIND_ID, vertexLayout.stride(), VK_VERTEX_INPUT_RATE_VERTEX);

		// Attribute descriptions
		// Describes memory layout and shader positions
		std::vector<VkVertexInputAttributeDescription> vertexInputAttributes = {
			vks::initializers::vertexInputAttributeDescription(VERTEX_BUFFER_BIND_ID, 0, VK_FORMAT_R32G32B32_SFLOAT, 0),						// Location 0: Position		
			vks::initializers::vertexInputAttributeDescription(VERTEX_BUFFER_BIND_ID, 1, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 3),		// Location 1: Normal		
			vks::initializers::vertexInputAttributeDescription(VERTEX_BUFFER_BIND_ID, 2, VK_FORMAT_R32G32_SFLOAT, sizeof(float) * 5),			// Location 2: Texture coordinates		
			vks::initializers::vertexInputAttributeDescription(VERTEX_BUFFER_BIND_ID, 3, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 8),		// Location 3: Color		
		};

		VkPipelineVertexInputStateCreateInfo vertexInputState = vks::initializers::pipelineVertexInputStateCreateInfo();
		vertexInputState.vertexBindingDescriptionCount = 1;
		vertexInputState.pVertexBindingDescriptions = &vertexInputBinding;
		vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributes.size());
		vertexInputState.pVertexAttributeDescriptions = vertexInputAttributes.data();

		pipelineCreateInfo.pVertexInputState = &vertexInputState;

		// Default mesh rendering pipeline
		shaderStages[0] = loadShader(getAssetPath() + "shaders/vulkanscene/mesh.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
		shaderStages[1] = loadShader(getAssetPath() + "shaders/vulkanscene/mesh.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
		VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCreateInfo, nullptr, &pipelines.models));

		// Pipeline for the logos
		shaderStages[0] = loadShader(getAssetPath() + "shaders/vulkanscene/logo.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
		shaderStages[1] = loadShader(getAssetPath() + "shaders/vulkanscene/logo.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
		VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCreateInfo, nullptr, &pipelines.logos));

		// Pipeline for the sky sphere
		rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT; // Inverted culling
		depthStencilState.depthWriteEnable = VK_FALSE; // No depth writes
		shaderStages[0] = loadShader(getAssetPath() + "shaders/vulkanscene/skybox.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
		shaderStages[1] = loadShader(getAssetPath() + "shaders/vulkanscene/skybox.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
		VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCreateInfo, nullptr, &pipelines.skybox));
	}

	// Prepare and initialize uniform buffer containing shader uniforms
	void prepareUniformBuffers()
	{
		vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&uniformData.meshVS,
			sizeof(uboVS));

		updateUniformBuffers();
	}

	void updateUniformBuffers()
	{
		uboVS.projection = glm::perspective(glm::radians(60.0f), (float)width / (float)height, 0.1f, 256.0f);

		uboVS.view = glm::lookAt(
			glm::vec3(0, 0, -zoom),
			cameraPos,
			glm::vec3(0, 1, 0)
			);

		uboVS.model = glm::mat4(1.0f);
		uboVS.model = glm::rotate(uboVS.model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		uboVS.model = glm::rotate(uboVS.model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		uboVS.model = glm::rotate(uboVS.model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

		uboVS.normal = glm::inverseTranspose(uboVS.view * uboVS.model);

		uboVS.lightPos = lightPos;

		VK_CHECK_RESULT(uniformData.meshVS.map());
		memcpy(uniformData.meshVS.mapped, &uboVS, sizeof(uboVS));
		uniformData.meshVS.unmap();
	}

	void draw()
	{
		VulkanExampleBase::prepareFrame();

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &drawCmdBuffers[currentBuffer];
		VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));

		VulkanExampleBase::submitFrame();
	}

	void prepare()
	{
		fprintf(stderr, "%s:%s ...\r\n", __FILE__, __func__);
		VulkanExampleBase::prepare();
		loadAssets();
		prepareUniformBuffers();
		setupDescriptorSetLayout();
		preparePipelines();
		setupDescriptorPool();
		setupDescriptorSet();
		buildCommandBuffers();
		prepared = true;
		fprintf(stderr, "%s:%s done \r\n", __FILE__, __func__);
	}

	virtual void render()
	{
		if (!prepared)
			return;
		draw();
	}

	virtual void viewChanged()
	{
		updateUniformBuffers();
	}


#if defined(VK_USE_PLATFORM_ANDROID_KHR)
	virtual void renderLoop_prepare() {
		if (benchmark.active) {
			benchmark.run([=] { render(); }, vulkanDevice->properties);
			vkDeviceWaitIdle(device);
			if (benchmark.filename != "") {
				benchmark.saveResults();
			}
			return;
		}
		
		destWidth = width;
		destHeight = height;
		lastTimestamp = std::chrono::high_resolution_clock::now();
	}
	virtual void renderLoop_standalone() {

	#if 1
		//while (1)
		{
			int ident;
			int events;
			struct android_poll_source* source;
			bool destroy = false;
		
			focused = true;

			#if 0
			while ((ident = ALooper_pollAll(focused ? 0 : -1, NULL, &events, (void**)&source)) >= 0)
			{
				if (source != NULL)
				{
					source->process(androidApp, source);
				}
				if (androidApp->destroyRequested != 0)
				{
					LOGD("Android app destroy requested");
					destroy = true;
					break;
				}
			}
		
			// App destruction requested
			// Exit loop, example will be destroyed in application main
			if (destroy)
			{
				ANativeActivity_finish(androidApp->activity);
				break;
			}
			#endif
	
			// Render frame
			if (prepared)
			{
				auto tStart = std::chrono::high_resolution_clock::now();
				render();
				frameCounter++;
				auto tEnd = std::chrono::high_resolution_clock::now();
				auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
				frameTimer = tDiff / 1000.0f;
				camera.update(frameTimer);
				// Convert to clamped timer value
				if (!paused)
				{
					timer += timerSpeed * frameTimer;
					if (timer > 1.0)
					{
						timer -= 1.0f;
					}
				}
				float fpsTimer = std::chrono::duration<double, std::milli>(tEnd - lastTimestamp).count();
				if (fpsTimer > 1000.0f)
				{
					lastFPS = (float)frameCounter * (1000.0f / fpsTimer);
					frameCounter = 0;
					lastTimestamp = tEnd;
				}
		
				// TODO: Cap UI overlay update rates/only issue when update requested
				updateOverlay();
		
				bool updateView = false;
		
				// Check touch state (for movement)
				if (touchDown) {
					touchTimer += frameTimer;
				}
				if (touchTimer >= 1.0) {
					camera.keys.up = true;
					viewChanged();
				}
		
				// Check gamepad state
				const float deadZone = 0.0015f;
				// todo : check if gamepad is present
				// todo : time based and relative axis positions
				if (camera.type != Camera::CameraType::firstperson)
				{
					// Rotate
					if (std::abs(gamePadState.axisLeft.x) > deadZone)
					{
						rotation.y += gamePadState.axisLeft.x * 0.5f * rotationSpeed;
						camera.rotate(glm::vec3(0.0f, gamePadState.axisLeft.x * 0.5f, 0.0f));
						updateView = true;
					}
					if (std::abs(gamePadState.axisLeft.y) > deadZone)
					{
						rotation.x -= gamePadState.axisLeft.y * 0.5f * rotationSpeed;
						camera.rotate(glm::vec3(gamePadState.axisLeft.y * 0.5f, 0.0f, 0.0f));
						updateView = true;
					}
					// Zoom
					if (std::abs(gamePadState.axisRight.y) > deadZone)
					{
						zoom -= gamePadState.axisRight.y * 0.01f * zoomSpeed;
						updateView = true;
					}
					if (updateView)
					{
						viewChanged();
					}
				}
				else
				{
					updateView = camera.updatePad(gamePadState.axisLeft, gamePadState.axisRight, frameTimer);
					if (updateView)
					{
						viewChanged();
					}
				}
			}
		}
	#endif

	}
	virtual void renderLoop_end() {
		// Flush device to make sure all resources can be freed
		if (device != VK_NULL_HANDLE) {
			vkDeviceWaitIdle(device);
		}

	}
#endif
};

};

VULKAN_EXAMPLE_MAIN(vulkanscene, "/sdcard/data/SaschaWillems/vulkanscene/assets/")


