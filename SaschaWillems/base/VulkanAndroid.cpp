/*
* Android Vulkan function pointer loader
*
* Copyright (C) 2016 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#include "VulkanAndroid.h"

#if defined(__ANDROID__)
	#include <android/log.h>
	#include <dlfcn.h>
	#include <android/native_window_jni.h>

	#include "native_app_glue.h"  // frankie, add

#include "utils/football_debugger.h"

#undef __CLASS__
#define __CLASS__ "VulkanAndroid"


android_app_* androidApp;

PFN_vkCreateInstance vkCreateInstance;
PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr;
PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
PFN_vkCreateDevice vkCreateDevice;
PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;
PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties;
PFN_vkEnumerateDeviceExtensionProperties vkEnumerateDeviceExtensionProperties;
PFN_vkEnumerateDeviceLayerProperties vkEnumerateDeviceLayerProperties;
PFN_vkGetPhysicalDeviceFormatProperties vkGetPhysicalDeviceFormatProperties;
PFN_vkGetPhysicalDeviceFeatures vkGetPhysicalDeviceFeatures;
PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties;
PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties;
PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties;
PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties;
PFN_vkCmdPipelineBarrier vkCmdPipelineBarrier;
PFN_vkCreateShaderModule vkCreateShaderModule;
PFN_vkCreateBuffer vkCreateBuffer;
PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements;
PFN_vkMapMemory vkMapMemory;
PFN_vkUnmapMemory vkUnmapMemory;
PFN_vkFlushMappedMemoryRanges vkFlushMappedMemoryRanges;
PFN_vkInvalidateMappedMemoryRanges vkInvalidateMappedMemoryRanges;
PFN_vkBindBufferMemory vkBindBufferMemory;
PFN_vkDestroyBuffer vkDestroyBuffer;
PFN_vkCreateBufferView vkCreateBufferView;  // frankie, add
PFN_vkDestroyBufferView vkDestroyBufferView;  // frankie, add
PFN_vkAllocateMemory vkAllocateMemory;
PFN_vkBindImageMemory vkBindImageMemory;
PFN_vkGetImageSubresourceLayout vkGetImageSubresourceLayout;
PFN_vkCmdCopyBuffer vkCmdCopyBuffer;
PFN_vkCmdCopyBufferToImage vkCmdCopyBufferToImage;
PFN_vkCmdCopyImageToBuffer vkCmdCopyImageToBuffer;  // frankie, add
PFN_vkCmdCopyImage vkCmdCopyImage;
PFN_vkCmdBlitImage vkCmdBlitImage;
PFN_vkCmdClearAttachments vkCmdClearAttachments;
PFN_vkCreateSampler vkCreateSampler;
PFN_vkDestroySampler vkDestroySampler;
PFN_vkDestroyImage vkDestroyImage;
PFN_vkFreeMemory vkFreeMemory;
PFN_vkCreateRenderPass vkCreateRenderPass;
PFN_vkCmdBeginRenderPass vkCmdBeginRenderPass;
PFN_vkCmdEndRenderPass vkCmdEndRenderPass;
PFN_vkCmdNextSubpass vkCmdNextSubpass;
PFN_vkCmdExecuteCommands vkCmdExecuteCommands;
PFN_vkCreateImage vkCreateImage;
PFN_vkGetImageMemoryRequirements vkGetImageMemoryRequirements;
PFN_vkCreateImageView vkCreateImageView;
PFN_vkDestroyImageView vkDestroyImageView;
PFN_vkCreateSemaphore vkCreateSemaphore;
PFN_vkDestroySemaphore vkDestroySemaphore;
PFN_vkCreateFence vkCreateFence;
PFN_vkDestroyFence vkDestroyFence;
PFN_vkWaitForFences vkWaitForFences;
PFN_vkResetFences vkResetFences;
PFN_vkCreateCommandPool vkCreateCommandPool;
PFN_vkDestroyCommandPool vkDestroyCommandPool;
PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers;
PFN_vkBeginCommandBuffer vkBeginCommandBuffer;
PFN_vkEndCommandBuffer vkEndCommandBuffer;
PFN_vkGetDeviceQueue vkGetDeviceQueue;
PFN_vkQueueSubmit vkQueueSubmit;
PFN_vkQueueWaitIdle vkQueueWaitIdle;
PFN_vkDeviceWaitIdle vkDeviceWaitIdle;
PFN_vkCreateFramebuffer vkCreateFramebuffer;
PFN_vkCreatePipelineCache vkCreatePipelineCache;
PFN_vkCreatePipelineLayout vkCreatePipelineLayout;
PFN_vkCreateGraphicsPipelines vkCreateGraphicsPipelines;
PFN_vkCreateComputePipelines vkCreateComputePipelines;
PFN_vkCreateDescriptorPool vkCreateDescriptorPool;
PFN_vkCreateDescriptorSetLayout vkCreateDescriptorSetLayout;
PFN_vkAllocateDescriptorSets vkAllocateDescriptorSets;
PFN_vkUpdateDescriptorSets vkUpdateDescriptorSets;
PFN_vkCmdBindDescriptorSets vkCmdBindDescriptorSets;
PFN_vkCmdBindPipeline vkCmdBindPipeline;
PFN_vkCmdBindVertexBuffers vkCmdBindVertexBuffers;
PFN_vkCmdBindIndexBuffer vkCmdBindIndexBuffer;
PFN_vkCmdSetViewport vkCmdSetViewport;
PFN_vkCmdSetScissor vkCmdSetScissor;
PFN_vkCmdSetLineWidth vkCmdSetLineWidth;
PFN_vkCmdSetDepthBias vkCmdSetDepthBias;
PFN_vkCmdPushConstants vkCmdPushConstants;
PFN_vkCmdDrawIndexed vkCmdDrawIndexed;
PFN_vkCmdDraw vkCmdDraw;
PFN_vkCmdDrawIndexedIndirect vkCmdDrawIndexedIndirect;
PFN_vkCmdDrawIndirect vkCmdDrawIndirect;
PFN_vkCmdDispatch vkCmdDispatch;
PFN_vkDestroyPipeline vkDestroyPipeline;
PFN_vkDestroyPipelineLayout vkDestroyPipelineLayout;
PFN_vkDestroyDescriptorSetLayout vkDestroyDescriptorSetLayout;
PFN_vkDestroyDevice vkDestroyDevice;
PFN_vkDestroyInstance vkDestroyInstance;
PFN_vkDestroyDescriptorPool vkDestroyDescriptorPool;
PFN_vkFreeCommandBuffers vkFreeCommandBuffers;
PFN_vkDestroyRenderPass vkDestroyRenderPass;
PFN_vkDestroyFramebuffer vkDestroyFramebuffer;
PFN_vkDestroyShaderModule vkDestroyShaderModule;
PFN_vkDestroyPipelineCache vkDestroyPipelineCache;
PFN_vkCreateQueryPool vkCreateQueryPool;
PFN_vkDestroyQueryPool vkDestroyQueryPool;
PFN_vkGetQueryPoolResults vkGetQueryPoolResults;
PFN_vkCmdBeginQuery vkCmdBeginQuery;
PFN_vkCmdEndQuery vkCmdEndQuery;
PFN_vkCmdResetQueryPool vkCmdResetQueryPool;
PFN_vkCmdCopyQueryPoolResults vkCmdCopyQueryPoolResults;

PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR;
PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR;
PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR;
PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR;

// VK_KHR_swapchain
PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR;
PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR;
PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR;
PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR;
PFN_vkQueuePresentKHR vkQueuePresentKHR;

#ifdef VK_USE_PLATFORM_ANDROID_KHR
// VK_KHR_android_surface
PFN_vkCreateAndroidSurfaceKHR vkCreateAndroidSurfaceKHR;
PFN_vkGetAndroidHardwareBufferPropertiesANDROID vkGetAndroidHardwareBufferPropertiesANDROID;
PFN_vkGetMemoryAndroidHardwareBufferANDROID vkGetMemoryAndroidHardwareBufferANDROID;
#endif

int32_t vks::android::screenDensity;

void *libVulkan;

namespace vks
{
	namespace android
	{
		// Dynamically load Vulkan library and base function pointers
		bool loadVulkanLibrary()
		{
			__android_log_print(ANDROID_LOG_INFO, "vulkanandroid", "Loading libvulkan.so...\n");

			// Load vulkan library
			libVulkan = dlopen("libvulkan.so", RTLD_NOW | RTLD_LOCAL);
			if (!libVulkan)
			{
				__android_log_print(ANDROID_LOG_INFO, "vulkanandroid", "Could not load vulkan library : %s!\n", dlerror());
				return false;
			}

			// Load base function pointers
			vkEnumerateInstanceExtensionProperties = reinterpret_cast<PFN_vkEnumerateInstanceExtensionProperties>(dlsym(libVulkan, "vkEnumerateInstanceExtensionProperties"));
			vkEnumerateInstanceLayerProperties = reinterpret_cast<PFN_vkEnumerateInstanceLayerProperties>(dlsym(libVulkan, "vkEnumerateInstanceLayerProperties"));
			vkCreateInstance = reinterpret_cast<PFN_vkCreateInstance>(dlsym(libVulkan, "vkCreateInstance"));
			vkGetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(dlsym(libVulkan, "vkGetInstanceProcAddr"));
			vkGetDeviceProcAddr = reinterpret_cast<PFN_vkGetDeviceProcAddr>(dlsym(libVulkan, "vkGetDeviceProcAddr"));

			return true;
		}

		// Load instance based Vulkan function pointers
		void loadVulkanFunctions(VkInstance instance)
		{
			__android_log_print(ANDROID_LOG_INFO, "vulkanandroid", "Loading instance based function pointers...\n");

			vkEnumeratePhysicalDevices = reinterpret_cast<PFN_vkEnumeratePhysicalDevices>(vkGetInstanceProcAddr(instance, "vkEnumeratePhysicalDevices"));
			vkGetPhysicalDeviceProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties>(vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceProperties"));
			vkEnumerateDeviceLayerProperties = reinterpret_cast<PFN_vkEnumerateDeviceLayerProperties>(vkGetInstanceProcAddr(instance, "vkEnumerateDeviceLayerProperties"));
			vkEnumerateDeviceExtensionProperties = reinterpret_cast<PFN_vkEnumerateDeviceExtensionProperties>(vkGetInstanceProcAddr(instance, "vkEnumerateDeviceExtensionProperties"));
			vkGetPhysicalDeviceQueueFamilyProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceQueueFamilyProperties>(vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceQueueFamilyProperties"));
			vkGetPhysicalDeviceFeatures = reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures>(vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceFeatures"));
			vkCreateDevice = reinterpret_cast<PFN_vkCreateDevice>(vkGetInstanceProcAddr(instance, "vkCreateDevice"));
			vkGetPhysicalDeviceFormatProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceFormatProperties>(vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceFormatProperties"));
			vkGetPhysicalDeviceMemoryProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceMemoryProperties>(vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceMemoryProperties"));

			vkCmdPipelineBarrier = reinterpret_cast<PFN_vkCmdPipelineBarrier>(vkGetInstanceProcAddr(instance, "vkCmdPipelineBarrier"));
			vkCreateShaderModule = reinterpret_cast<PFN_vkCreateShaderModule>(vkGetInstanceProcAddr(instance, "vkCreateShaderModule"));

			vkCreateBuffer = reinterpret_cast<PFN_vkCreateBuffer>(vkGetInstanceProcAddr(instance, "vkCreateBuffer"));
			vkGetBufferMemoryRequirements = reinterpret_cast<PFN_vkGetBufferMemoryRequirements>(vkGetInstanceProcAddr(instance, "vkGetBufferMemoryRequirements"));
			vkMapMemory = reinterpret_cast<PFN_vkMapMemory>(vkGetInstanceProcAddr(instance, "vkMapMemory"));
			vkUnmapMemory = reinterpret_cast<PFN_vkUnmapMemory>(vkGetInstanceProcAddr(instance, "vkUnmapMemory"));
			vkFlushMappedMemoryRanges = reinterpret_cast<PFN_vkFlushMappedMemoryRanges>(vkGetInstanceProcAddr(instance, "vkFlushMappedMemoryRanges"));
			vkInvalidateMappedMemoryRanges = reinterpret_cast<PFN_vkInvalidateMappedMemoryRanges>(vkGetInstanceProcAddr(instance, "vkInvalidateMappedMemoryRanges"));
			vkBindBufferMemory = reinterpret_cast<PFN_vkBindBufferMemory>(vkGetInstanceProcAddr(instance, "vkBindBufferMemory"));
			vkDestroyBuffer = reinterpret_cast<PFN_vkDestroyBuffer>(vkGetInstanceProcAddr(instance, "vkDestroyBuffer"));
			vkCreateBufferView = reinterpret_cast<PFN_vkCreateBufferView>(vkGetInstanceProcAddr(instance, "vkCreateBufferView")); // frankie, add
			vkDestroyBufferView = reinterpret_cast<PFN_vkDestroyBufferView>(vkGetInstanceProcAddr(instance, "vkDestroyBufferView")); // frankie, add

			vkAllocateMemory = reinterpret_cast<PFN_vkAllocateMemory>(vkGetInstanceProcAddr(instance, "vkAllocateMemory"));
			vkFreeMemory = reinterpret_cast<PFN_vkFreeMemory>(vkGetInstanceProcAddr(instance, "vkFreeMemory"));
			vkCreateRenderPass = reinterpret_cast<PFN_vkCreateRenderPass>(vkGetInstanceProcAddr(instance, "vkCreateRenderPass"));
			vkCmdBeginRenderPass = reinterpret_cast<PFN_vkCmdBeginRenderPass>(vkGetInstanceProcAddr(instance, "vkCmdBeginRenderPass"));
			vkCmdEndRenderPass = reinterpret_cast<PFN_vkCmdEndRenderPass>(vkGetInstanceProcAddr(instance, "vkCmdEndRenderPass"));
			vkCmdNextSubpass = reinterpret_cast<PFN_vkCmdNextSubpass>(vkGetInstanceProcAddr(instance, "vkCmdNextSubpass"));
			vkCmdExecuteCommands = reinterpret_cast<PFN_vkCmdExecuteCommands>(vkGetInstanceProcAddr(instance, "vkCmdExecuteCommands"));

			vkCreateImage = reinterpret_cast<PFN_vkCreateImage>(vkGetInstanceProcAddr(instance, "vkCreateImage"));
			vkGetImageMemoryRequirements = reinterpret_cast<PFN_vkGetImageMemoryRequirements>(vkGetInstanceProcAddr(instance, "vkGetImageMemoryRequirements"));
			vkCreateImageView = reinterpret_cast<PFN_vkCreateImageView>(vkGetInstanceProcAddr(instance, "vkCreateImageView"));
			vkDestroyImageView = reinterpret_cast<PFN_vkDestroyImageView>(vkGetInstanceProcAddr(instance, "vkDestroyImageView"));
			vkBindImageMemory = reinterpret_cast<PFN_vkBindImageMemory>(vkGetInstanceProcAddr(instance, "vkBindImageMemory"));
			vkGetImageSubresourceLayout = reinterpret_cast<PFN_vkGetImageSubresourceLayout>(vkGetInstanceProcAddr(instance, "vkGetImageSubresourceLayout"));
			vkCmdCopyImage = reinterpret_cast<PFN_vkCmdCopyImage>(vkGetInstanceProcAddr(instance, "vkCmdCopyImage"));
			vkCmdBlitImage = reinterpret_cast<PFN_vkCmdBlitImage>(vkGetInstanceProcAddr(instance, "vkCmdBlitImage"));
			vkDestroyImage = reinterpret_cast<PFN_vkDestroyImage>(vkGetInstanceProcAddr(instance, "vkDestroyImage"));

			vkCmdClearAttachments = reinterpret_cast<PFN_vkCmdClearAttachments>(vkGetInstanceProcAddr(instance, "vkCmdClearAttachments"));

			vkCmdCopyBuffer = reinterpret_cast<PFN_vkCmdCopyBuffer>(vkGetInstanceProcAddr(instance, "vkCmdCopyBuffer"));
			vkCmdCopyBufferToImage = reinterpret_cast<PFN_vkCmdCopyBufferToImage>(vkGetInstanceProcAddr(instance, "vkCmdCopyBufferToImage"));
			vkCmdCopyImageToBuffer = 
				reinterpret_cast<PFN_vkCmdCopyImageToBuffer>(vkGetInstanceProcAddr(instance, "vkCmdCopyImageToBuffer"));

			vkCreateSampler = reinterpret_cast<PFN_vkCreateSampler>(vkGetInstanceProcAddr(instance, "vkCreateSampler"));
			vkDestroySampler = reinterpret_cast<PFN_vkDestroySampler>(vkGetInstanceProcAddr(instance, "vkDestroySampler"));;

			vkCreateSemaphore = reinterpret_cast<PFN_vkCreateSemaphore>(vkGetInstanceProcAddr(instance, "vkCreateSemaphore"));
			vkDestroySemaphore = reinterpret_cast<PFN_vkDestroySemaphore>(vkGetInstanceProcAddr(instance, "vkDestroySemaphore"));

			vkCreateFence = reinterpret_cast<PFN_vkCreateFence>(vkGetInstanceProcAddr(instance, "vkCreateFence"));
			vkDestroyFence = reinterpret_cast<PFN_vkDestroyFence>(vkGetInstanceProcAddr(instance, "vkDestroyFence"));
			vkWaitForFences = reinterpret_cast<PFN_vkWaitForFences>(vkGetInstanceProcAddr(instance, "vkWaitForFences"));
			vkResetFences = reinterpret_cast<PFN_vkResetFences>(vkGetInstanceProcAddr(instance, "vkResetFences"));;

			vkCreateCommandPool = reinterpret_cast<PFN_vkCreateCommandPool>(vkGetInstanceProcAddr(instance, "vkCreateCommandPool"));
			vkDestroyCommandPool = reinterpret_cast<PFN_vkDestroyCommandPool>(vkGetInstanceProcAddr(instance, "vkDestroyCommandPool"));;

			vkAllocateCommandBuffers = reinterpret_cast<PFN_vkAllocateCommandBuffers>(vkGetInstanceProcAddr(instance, "vkAllocateCommandBuffers"));
			vkBeginCommandBuffer = reinterpret_cast<PFN_vkBeginCommandBuffer>(vkGetInstanceProcAddr(instance, "vkBeginCommandBuffer"));
			vkEndCommandBuffer = reinterpret_cast<PFN_vkEndCommandBuffer>(vkGetInstanceProcAddr(instance, "vkEndCommandBuffer"));

			vkGetDeviceQueue = reinterpret_cast<PFN_vkGetDeviceQueue>(vkGetInstanceProcAddr(instance, "vkGetDeviceQueue"));
			vkQueueSubmit = reinterpret_cast<PFN_vkQueueSubmit>(vkGetInstanceProcAddr(instance, "vkQueueSubmit"));
			vkQueueWaitIdle = reinterpret_cast<PFN_vkQueueWaitIdle>(vkGetInstanceProcAddr(instance, "vkQueueWaitIdle"));

			vkDeviceWaitIdle = reinterpret_cast<PFN_vkDeviceWaitIdle>(vkGetInstanceProcAddr(instance, "vkDeviceWaitIdle"));

			vkCreateFramebuffer = reinterpret_cast<PFN_vkCreateFramebuffer>(vkGetInstanceProcAddr(instance, "vkCreateFramebuffer"));

			vkCreatePipelineCache = reinterpret_cast<PFN_vkCreatePipelineCache>(vkGetInstanceProcAddr(instance, "vkCreatePipelineCache"));
			vkCreatePipelineLayout = reinterpret_cast<PFN_vkCreatePipelineLayout>(vkGetInstanceProcAddr(instance, "vkCreatePipelineLayout"));
			vkCreateGraphicsPipelines = reinterpret_cast<PFN_vkCreateGraphicsPipelines>(vkGetInstanceProcAddr(instance, "vkCreateGraphicsPipelines"));
			vkCreateComputePipelines = reinterpret_cast<PFN_vkCreateComputePipelines>(vkGetInstanceProcAddr(instance, "vkCreateComputePipelines"));

			vkCreateDescriptorPool = reinterpret_cast<PFN_vkCreateDescriptorPool>(vkGetInstanceProcAddr(instance, "vkCreateDescriptorPool"));
			vkCreateDescriptorSetLayout = reinterpret_cast<PFN_vkCreateDescriptorSetLayout>(vkGetInstanceProcAddr(instance, "vkCreateDescriptorSetLayout"));

			vkAllocateDescriptorSets = reinterpret_cast<PFN_vkAllocateDescriptorSets>(vkGetInstanceProcAddr(instance, "vkAllocateDescriptorSets"));
			vkUpdateDescriptorSets = reinterpret_cast<PFN_vkUpdateDescriptorSets>(vkGetInstanceProcAddr(instance, "vkUpdateDescriptorSets"));

			vkCmdBindDescriptorSets = reinterpret_cast<PFN_vkCmdBindDescriptorSets>(vkGetInstanceProcAddr(instance, "vkCmdBindDescriptorSets"));
			vkCmdBindPipeline = reinterpret_cast<PFN_vkCmdBindPipeline>(vkGetInstanceProcAddr(instance, "vkCmdBindPipeline"));
			vkCmdBindVertexBuffers = reinterpret_cast<PFN_vkCmdBindVertexBuffers>(vkGetInstanceProcAddr(instance, "vkCmdBindVertexBuffers"));
			vkCmdBindIndexBuffer = reinterpret_cast<PFN_vkCmdBindIndexBuffer>(vkGetInstanceProcAddr(instance, "vkCmdBindIndexBuffer"));

			vkCmdSetViewport = reinterpret_cast<PFN_vkCmdSetViewport>(vkGetInstanceProcAddr(instance, "vkCmdSetViewport"));
			vkCmdSetScissor = reinterpret_cast<PFN_vkCmdSetScissor>(vkGetInstanceProcAddr(instance, "vkCmdSetScissor"));
			vkCmdSetLineWidth = reinterpret_cast<PFN_vkCmdSetLineWidth>(vkGetInstanceProcAddr(instance, "vkCmdSetLineWidth"));
			vkCmdSetDepthBias = reinterpret_cast<PFN_vkCmdSetDepthBias>(vkGetInstanceProcAddr(instance, "vkCmdSetDepthBias"));
			vkCmdPushConstants = reinterpret_cast<PFN_vkCmdPushConstants>(vkGetInstanceProcAddr(instance, "vkCmdPushConstants"));;

			vkCmdDrawIndexed = reinterpret_cast<PFN_vkCmdDrawIndexed>(vkGetInstanceProcAddr(instance, "vkCmdDrawIndexed"));
			vkCmdDraw = reinterpret_cast<PFN_vkCmdDraw>(vkGetInstanceProcAddr(instance, "vkCmdDraw"));
			vkCmdDrawIndexedIndirect = reinterpret_cast<PFN_vkCmdDrawIndexedIndirect>(vkGetInstanceProcAddr(instance, "vkCmdDrawIndexedIndirect"));
			vkCmdDrawIndirect = reinterpret_cast<PFN_vkCmdDrawIndirect>(vkGetInstanceProcAddr(instance, "vkCmdDrawIndirect"));
			vkCmdDispatch = reinterpret_cast<PFN_vkCmdDispatch>(vkGetInstanceProcAddr(instance, "vkCmdDispatch"));

			vkDestroyPipeline = reinterpret_cast<PFN_vkDestroyPipeline>(vkGetInstanceProcAddr(instance, "vkDestroyPipeline"));
			vkDestroyPipelineLayout = reinterpret_cast<PFN_vkDestroyPipelineLayout>(vkGetInstanceProcAddr(instance, "vkDestroyPipelineLayout"));;
			vkDestroyDescriptorSetLayout = reinterpret_cast<PFN_vkDestroyDescriptorSetLayout>(vkGetInstanceProcAddr(instance, "vkDestroyDescriptorSetLayout"));
			vkDestroyDevice = reinterpret_cast<PFN_vkDestroyDevice>(vkGetInstanceProcAddr(instance, "vkDestroyDevice"));
			vkDestroyInstance = reinterpret_cast<PFN_vkDestroyInstance>(vkGetInstanceProcAddr(instance, "vkDestroyInstance"));
			vkDestroyDescriptorPool = reinterpret_cast<PFN_vkDestroyDescriptorPool>(vkGetInstanceProcAddr(instance, "vkDestroyDescriptorPool"));
			vkFreeCommandBuffers = reinterpret_cast<PFN_vkFreeCommandBuffers>(vkGetInstanceProcAddr(instance, "vkFreeCommandBuffers"));
			vkDestroyRenderPass = reinterpret_cast<PFN_vkDestroyRenderPass>(vkGetInstanceProcAddr(instance, "vkDestroyRenderPass"));
			vkDestroyFramebuffer = reinterpret_cast<PFN_vkDestroyFramebuffer>(vkGetInstanceProcAddr(instance, "vkDestroyFramebuffer"));
			vkDestroyShaderModule = reinterpret_cast<PFN_vkDestroyShaderModule>(vkGetInstanceProcAddr(instance, "vkDestroyShaderModule"));
			vkDestroyPipelineCache = reinterpret_cast<PFN_vkDestroyPipelineCache>(vkGetInstanceProcAddr(instance, "vkDestroyPipelineCache"));

			vkCreateQueryPool = reinterpret_cast<PFN_vkCreateQueryPool>(vkGetInstanceProcAddr(instance, "vkCreateQueryPool"));
			vkDestroyQueryPool = reinterpret_cast<PFN_vkDestroyQueryPool>(vkGetInstanceProcAddr(instance, "vkDestroyQueryPool"));
			vkGetQueryPoolResults = reinterpret_cast<PFN_vkGetQueryPoolResults>(vkGetInstanceProcAddr(instance, "vkGetQueryPoolResults"));

			vkCmdBeginQuery = reinterpret_cast<PFN_vkCmdBeginQuery>(vkGetInstanceProcAddr(instance, "vkCmdBeginQuery"));
			vkCmdEndQuery = reinterpret_cast<PFN_vkCmdEndQuery>(vkGetInstanceProcAddr(instance, "vkCmdEndQuery"));
			vkCmdResetQueryPool = reinterpret_cast<PFN_vkCmdResetQueryPool>(vkGetInstanceProcAddr(instance, "vkCmdResetQueryPool"));
			vkCmdCopyQueryPoolResults = reinterpret_cast<PFN_vkCmdCopyQueryPoolResults>(vkGetInstanceProcAddr(instance, "vkCmdCopyQueryPoolResults"));

			vkDestroySurfaceKHR = reinterpret_cast<PFN_vkDestroySurfaceKHR>(vkGetInstanceProcAddr(instance, "vkDestroySurfaceKHR"));
			vkGetPhysicalDeviceSurfaceSupportKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceSupportKHR>(vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceSupportKHR"));
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR>(vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR"));
			vkGetPhysicalDeviceSurfaceFormatsKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceFormatsKHR>(vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceFormatsKHR"));
			vkGetPhysicalDeviceSurfacePresentModesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfacePresentModesKHR>(vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfacePresentModesKHR"));

			// VK_KHR_swapchain
			vkCreateSwapchainKHR = reinterpret_cast<PFN_vkCreateSwapchainKHR>(vkGetInstanceProcAddr(instance, "vkCreateSwapchainKHR"));
			vkDestroySwapchainKHR = reinterpret_cast<PFN_vkDestroySwapchainKHR>(vkGetInstanceProcAddr(instance, "vkDestroySwapchainKHR"));
			vkGetSwapchainImagesKHR = reinterpret_cast<PFN_vkGetSwapchainImagesKHR>(vkGetInstanceProcAddr(instance, "vkGetSwapchainImagesKHR"));
			vkAcquireNextImageKHR = reinterpret_cast<PFN_vkAcquireNextImageKHR>(vkGetInstanceProcAddr(instance, "vkAcquireNextImageKHR"));
			vkQueuePresentKHR = reinterpret_cast<PFN_vkQueuePresentKHR>(vkGetInstanceProcAddr(instance, "vkQueuePresentKHR"));

#ifdef VK_USE_PLATFORM_ANDROID_KHR
			vkCreateAndroidSurfaceKHR = reinterpret_cast<PFN_vkCreateAndroidSurfaceKHR>(vkGetInstanceProcAddr(instance, "vkCreateAndroidSurfaceKHR"));
			vkGetAndroidHardwareBufferPropertiesANDROID = 
				reinterpret_cast<PFN_vkGetAndroidHardwareBufferPropertiesANDROID>(vkGetInstanceProcAddr(instance, "vkGetAndroidHardwareBufferPropertiesANDROID"));
			vkGetMemoryAndroidHardwareBufferANDROID = 
				reinterpret_cast<PFN_vkGetMemoryAndroidHardwareBufferANDROID>(vkGetInstanceProcAddr(instance, "vkGetMemoryAndroidHardwareBufferANDROID"));
#endif

		}

		void freeVulkanLibrary()
		{
			dlclose(libVulkan);
		}

		void getDeviceConfig()
		{
			// Screen density
			#if 0
			AConfiguration* config = AConfiguration_new();
			AConfiguration_fromAssetManager(config, androidApp->activity->assetManager);
			vks::android::screenDensity = AConfiguration_getDensity(config);
			AConfiguration_delete(config);
			#else
			vks::android::screenDensity = ACONFIGURATION_DENSITY_XXHIGH;
			//vks::android::screenDensity = ACONFIGURATION_DENSITY_XHIGH;
			//vks::android::screenDensity = ACONFIGURATION_DENSITY_HIGH;
			#endif
		}

		// Displays a native alert dialog using JNI
		void showAlert(const char* message) {
			#if 0
			JNIEnv* jni;
			androidApp->activity->vm->AttachCurrentThread(&jni, NULL);

			jstring jmessage = jni->NewStringUTF(message);

			jclass clazz = jni->GetObjectClass(androidApp->activity->clazz);
			// Signature has to match java implementation (arguments)
			jmethodID methodID = jni->GetMethodID(clazz, "showAlert", "(Ljava/lang/String;)V");
			jni->CallVoidMethod(androidApp->activity->clazz, methodID, jmessage);
			jni->DeleteLocalRef(jmessage);

			androidApp->activity->vm->DetachCurrentThread();
			#else
			DLOGD( "************************************************** \r\n");
			DLOGD( "showAlert:%s \r\n", message);
			DLOGD( "************************************************** \r\n");
			abort();
			#endif
			return;
		}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static const char *get_VkFormatFeatureFlags_str(VkFormatFeatureFlags one_bit) {
	if (one_bit == VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) 
		{ return "VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT"; }
	else if (one_bit == VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT) 
		{ return "VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT"; }
	else if (one_bit == VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT) 
		{ return "VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT"; }
	else if (one_bit == VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT) 
		{ return "VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT"; }
	else if (one_bit == VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT) 
		{ return "VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT"; }
	else if (one_bit == VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT) 
		{ return "VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT"; }
	else if (one_bit == VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT) 
		{ return "VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT"; }
	else if (one_bit == VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) 
		{ return "VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT"; }
	else if (one_bit == VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT) 
		{ return "VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT"; }
	else if (one_bit == VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) 
		{ return "VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT"; }
	else if (one_bit == VK_FORMAT_FEATURE_BLIT_SRC_BIT) 
		{ return "VK_FORMAT_FEATURE_BLIT_SRC_BIT"; }
	else if (one_bit == VK_FORMAT_FEATURE_BLIT_DST_BIT) 
		{ return "VK_FORMAT_FEATURE_BLIT_DST_BIT"; }
	else if (one_bit == VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT) 
		{ return "VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT"; }
	else if (one_bit == VK_FORMAT_FEATURE_TRANSFER_SRC_BIT) 
		{ return "VK_FORMAT_FEATURE_TRANSFER_SRC_BIT"; }
	else if (one_bit == VK_FORMAT_FEATURE_TRANSFER_DST_BIT) 
		{ return "VK_FORMAT_FEATURE_TRANSFER_DST_BIT"; }
	else if (one_bit == VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT) 
		{ return "VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT"; }
	else if (one_bit == VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT) 
		{ return "VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT"; }
	else if (one_bit == VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT) 
		{ return "VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT"; }
	else if (one_bit == VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_BIT) 
		{ return "VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_BIT"; }
	else if (one_bit == VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT) 
		{ return "VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT"; }
	else if (one_bit == VK_FORMAT_FEATURE_DISJOINT_BIT) 
		{ return "VK_FORMAT_FEATURE_DISJOINT_BIT"; }
	else if (one_bit == VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT) 
		{ return "VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT"; }
	else if (one_bit == VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT_IMG) 
		{ return "VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT_IMG"; }
	else if (one_bit == VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_MINMAX_BIT_EXT) 
		{ return "VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_MINMAX_BIT_EXT"; }
	else if (one_bit == VK_FORMAT_FEATURE_FRAGMENT_DENSITY_MAP_BIT_EXT) 
		{ return "VK_FORMAT_FEATURE_FRAGMENT_DENSITY_MAP_BIT_EXT"; }
	
	return "***NOT_DEFINED***";
}
void print_VkFormatFeatureFlags(VkFormatFeatureFlags flags) {
	int i=0;
	uint32_t shift_bit = 0x01;
	for(i=0;i<32;i++) {
		uint32_t this_bit = flags & (shift_bit << i);
		if(this_bit) {
			DLOGD( "[%2d]:%s \r\n", i, get_VkFormatFeatureFlags_str(this_bit));
		}
	}
}






	}
}

#endif
