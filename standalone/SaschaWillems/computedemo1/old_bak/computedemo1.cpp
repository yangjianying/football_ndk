
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
#include "vulkanexamplebase.h"
#include "VulkanTexture.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanModel.hpp"

namespace computedemo1 {

class VulkanExample : public VulkanExampleBase
{
public:
	VulkanExample() : VulkanExampleBase(false)
	{
		settings.overlay = true;
	}
	~VulkanExample()
	{

	}

/*
	vulkanExample = new name::VulkanExample();
	void VulkanExampleBase::renderLoop() {
		while (1) {
			while ((ident = ALooper_pollAll(focused ? 0 : -1, NULL, &events, (void**)&source)) >= 0) {
				source->process(androidApp, source);
				// case APP_CMD_INIT_WINDOW:
					if (vulkanExample->initVulkan()) {
						vulkanExample->prepare();
					}
				// case APP_CMD_TERM_WINDOW:
					if (vulkanExample->prepared) {
						vulkanExample->swapChain.cleanup();
					}
			}
			if (prepared) {
				render();
				updateOverlay();
				viewChanged();
			}
		}
	}
	delete(vulkanExample);
*/
	virtual void render() {
		// submit commandbuffer to queue

	}
};


};

VULKAN_EXAMPLE_MAIN(computedemo1, "SaschaWillems/computedemo1/assets/")


