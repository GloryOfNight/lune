#include "vulkan_subsystem.hxx"

#include "SDL3/SDL_vulkan.h"
#include "vulkan/vulkan.hpp"

#include "log.hxx"
#include "lune.hxx"

#include <vector>

#define LUNE_USE_VALIDATION

lune::vulkan_subsystem* gVulkanSubsystem{nullptr};

lune::vulkan_subsystem* lune::vulkan_subsystem::get()
{
	return gVulkanSubsystem;
}

lune::vulkan_subsystem* lune::vulkan_subsystem::getChecked()
{
	if (nullptr == gVulkanSubsystem) [[unlikely]]
	{
		std::abort();
	}
	return gVulkanSubsystem;
}

bool lune::vulkan_subsystem::allowInitialize()
{
	return nullptr != vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceVersion");
}

void lune::vulkan_subsystem::initialize()
{
	gVulkanSubsystem = this;

	mApiVersion = VK_API_VERSION_1_0;
	LN_LOG(Info, Vulkan, "Vulkan version {0}.{1}.{2}", VK_VERSION_MAJOR(mApiVersion), VK_VERSION_MINOR(mApiVersion), VK_VERSION_PATCH(mApiVersion))

	const vk::ApplicationInfo applicationInfo = vk::ApplicationInfo()
													.setPApplicationName(lune::getApplicationName().c_str())
													.setApplicationVersion(lune::getApplicationVersion())
													.setPEngineName(lune::getEngineName())
													.setEngineVersion(lune::getEngineVersion())
													.setApiVersion(mApiVersion);

	uint32_t instanceExtensionsCount{};
	char const* const* instanceExtensionsSdl = SDL_Vulkan_GetInstanceExtensions(&instanceExtensionsCount);

	std::vector<const char*> instanceExtensions(instanceExtensionsSdl, instanceExtensionsSdl + instanceExtensionsCount);
	std::vector<const char*> instanceLayers = {"VK_LAYER_KHRONOS_validation"};

	vulkan::createInstance(applicationInfo, instanceExtensions, instanceLayers, gVulkanContext);
	vulkan::findPhysicalDevice(gVulkanContext);

	if (nullptr == gVulkanContext.physicalDevice)
	{
		LN_LOG(Fatal, Vulkan, "Failed to find suitable device for Vulkan");
		return;
	}

	const auto mPhysicalDeviceProperies = gVulkanContext.physicalDevice.getProperties();

	const uint32 deviceApiVersion = mPhysicalDeviceProperies.apiVersion;
	LN_LOG(Info, Vulkan, "Selected device:", mPhysicalDeviceProperies.deviceName.data(), mPhysicalDeviceProperies.deviceID);
	LN_LOG(Info, Vulkan, "	GPU: {0} (id: {1})", mPhysicalDeviceProperies.deviceName.data(), mPhysicalDeviceProperies.deviceID);
	LN_LOG(Info, Vulkan, "	API: {0}.{1}.{2}", VK_VERSION_MAJOR(deviceApiVersion), VK_VERSION_MINOR(deviceApiVersion), VK_VERSION_PATCH(deviceApiVersion))

	vulkan::createDevice(gVulkanContext);
	vulkan::createQueues(gVulkanContext);
	vulkan::createGraphicsCommandPool(gVulkanContext);
	vulkan::createTransferCommandPool(gVulkanContext);
}

void lune::vulkan_subsystem::shutdown()
{
	for (auto& view : mViews)
	{
		view->destroy();
	}
	mViews.clear();

	if (gVulkanContext.graphicsCommandPool)
	{
		gVulkanContext.device.destroyCommandPool(gVulkanContext.graphicsCommandPool);
	}

	if (gVulkanContext.transferCommandPool)
	{
		gVulkanContext.device.destroyCommandPool(gVulkanContext.transferCommandPool);
	}

	if (gVulkanContext.device)
	{
		gVulkanContext.device.destroy();
	}

	if (gVulkanContext.instance)
	{
		gVulkanContext.instance.destroy();
	}

	gVulkanContext = vulkan_context{};
	gVulkanSubsystem = nullptr;
}

void lune::vulkan_subsystem::createView(SDL_Window* window)
{
	auto newView = lune::vulkan::view::create(window);
	if (newView)
	{
		mViews.emplace_back(std::move(newView))->init();
	}
}

void lune::vulkan::createInstance(const vk::ApplicationInfo& applicationInfo, const std::vector<const char*>& instanceExtensions, const std::vector<const char*>& instanceLayers, vulkan_context& context)
{
	const vk::InstanceCreateInfo instanceCreateInfo = vk::InstanceCreateInfo()
														  .setPApplicationInfo(&applicationInfo)
														  .setPEnabledExtensionNames(instanceExtensions)
														  .setPEnabledLayerNames(instanceLayers);
	context.instance = vk::createInstance(instanceCreateInfo);
}

void lune::vulkan::findPhysicalDevice(vulkan_context& context)
{
	const auto physicalDevices = context.instance.enumeratePhysicalDevices();

	LN_LOG(Info, Vulkan, "Avaiable physical devices:");

	vk::PhysicalDevice selectedDevice{};

	for (size_t i = 0; i < physicalDevices.size(); ++i)
	{
		const auto physicalDevice = physicalDevices[i];
		const auto physicalDeviceProperties = physicalDevice.getProperties();

		LN_LOG(Info, Vulkan, "	{0}: {1} (id: {2})", i, physicalDeviceProperties.deviceName.data(), physicalDeviceProperties.deviceID);

		if (vk::PhysicalDeviceType::eCpu == physicalDeviceProperties.deviceType ||
			vk::PhysicalDeviceType::eOther == physicalDeviceProperties.deviceType)
		{
			continue;
		}

		selectedDevice = physicalDevice;
		if (vk::PhysicalDeviceType::eDiscreteGpu == physicalDeviceProperties.deviceType)
		{
			break;
		}
	}
	context.physicalDevice = selectedDevice;
}

void lune::vulkan::createDevice(vulkan_context& context)
{
	const auto queueFamilyProperties = context.physicalDevice.getQueueFamilyProperties();
	const auto enabledFeatures = context.physicalDevice.getFeatures();

	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfoList;
	queueCreateInfoList.reserve(queueFamilyProperties.size());

	for (size_t i = 0; i < queueFamilyProperties.size(); ++i)
	{
		constexpr std::array<float, 1> priorities{1.F};
		queueCreateInfoList.emplace_back()
			.setQueueFamilyIndex(i)
			.setQueueCount(priorities.size())
			.setQueuePriorities(priorities);
	}

	const std::vector<vk::ExtensionProperties> availableExtensions = context.physicalDevice.enumerateDeviceExtensionProperties();
	const std::vector<const char*> requiredExtensions{"VK_KHR_swapchain"};

	for (const auto& requiredExtension : requiredExtensions)
	{
		const auto cmpLam = [&requiredExtension](const vk::ExtensionProperties& extension) -> bool
		{
			return strcmp(extension.extensionName, requiredExtension) == 0;
		};

		const auto it = std::find_if(availableExtensions.begin(), availableExtensions.end(), cmpLam);
		if (it == availableExtensions.end())
		{
			LN_LOG(Fatal, Vulkan, "Required extension not found: {0}", requiredExtension);
			return;
		}
	}

	const std::vector<const char*> requiredLayers{};

	const vk::DeviceCreateInfo deviceCreateInfo = vk::DeviceCreateInfo()
													  .setQueueCreateInfos(queueCreateInfoList)
													  .setPEnabledExtensionNames(requiredExtensions)
													  .setPEnabledLayerNames(requiredLayers)
													  .setPEnabledFeatures(&enabledFeatures);

	context.device = context.physicalDevice.createDevice(deviceCreateInfo);
}

void lune::vulkan::createQueues(vulkan_context& context)
{
	const auto queueFamilyProperties = context.physicalDevice.getQueueFamilyProperties();
	const size_t queueFamilyPropertiesSize = queueFamilyProperties.size();
	for (size_t i = 0; i < queueFamilyPropertiesSize; ++i)
	{
		const auto queueFlags = queueFamilyProperties[i].queueFlags;
		if ((queueFlags & vk::QueueFlagBits::eGraphics) && (queueFlags & vk::QueueFlagBits::eTransfer))
		{
			context.graphicsQueueIndex = i;
			context.transferQueueIndex = i;
		}
	}

	context.graphicsQueue = context.device.getQueue(context.graphicsQueueIndex, 0);
	context.transferQueue = context.device.getQueue(context.transferQueueIndex, 0);
}

void lune::vulkan::createGraphicsCommandPool(vulkan_context& context)
{
	const vk::CommandPoolCreateInfo graphicsCreateInfo = vk::CommandPoolCreateInfo()
															 .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
															 .setQueueFamilyIndex(context.graphicsQueueIndex);
	context.graphicsCommandPool = context.device.createCommandPool(graphicsCreateInfo);
}

void lune::vulkan::createTransferCommandPool(vulkan_context& context)
{
	const vk::CommandPoolCreateInfo transferCreateInfo = vk::CommandPoolCreateInfo()
															 .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer | vk::CommandPoolCreateFlagBits::eTransient)
															 .setQueueFamilyIndex(context.transferQueueIndex);
	context.transferCommandPool = context.device.createCommandPool(transferCreateInfo);
}
