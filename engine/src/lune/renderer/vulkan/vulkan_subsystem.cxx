#include "vulkan_subsystem.hxx"

#include "SDL3/SDL_vulkan.h"
#include "vulkan/vulkan.hpp"

#include "log.hxx"

#include <vector>

#define LUNE_USE_VALIDATION

lune::vulkan_subsystem* gVulkanSubsystem{nullptr};

lune::vulkan_subsystem* lune::vulkan_subsystem::get()
{
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
	LN_LOG(Info, Vulkan, "Initializing subsystem. Vulkan - {0}.{1}.{2}", VK_VERSION_MAJOR(mApiVersion), VK_VERSION_MINOR(mApiVersion), VK_VERSION_PATCH(mApiVersion))

	createInstance();
	createPhysicalDevice();

	if (nullptr == mPhysicalDevice)
	{
		LN_LOG(Fatal, Vulkan, "Failed to find suitable device for Vulkan");
		return;
	}

	mPhysicalDeviceProperies = mPhysicalDevice.getProperties();
	mPhysicalDeviceFeatures = mPhysicalDevice.getFeatures();

	const uint32 deviceApiVersion = mPhysicalDeviceProperies.apiVersion;
	LN_LOG(Info, Vulkan, "Selected device:", mPhysicalDeviceProperies.deviceName.data(), mPhysicalDeviceProperies.deviceID);
	LN_LOG(Info, Vulkan, "	GPU: {0} (id: {1})", mPhysicalDeviceProperies.deviceName.data(), mPhysicalDeviceProperies.deviceID);
	LN_LOG(Info, Vulkan, "	API: {0}.{1}.{2}", VK_VERSION_MAJOR(deviceApiVersion), VK_VERSION_MINOR(deviceApiVersion), VK_VERSION_PATCH(deviceApiVersion))

	createDevice();
	createQueues();
	createCommandPools();
}

void lune::vulkan_subsystem::shutdown()
{
	for (auto& view : mViews)
	{
		view->destroy();
	}
	mViews.clear();

	if (mTransferQueue)
	{
		mTransferQueueIndex = 0;
		mTransferQueue = vk::Queue();
	}

	if (mGraphicsQueue)
	{
		mGraphicsQueueIndex = 0;
		mGraphicsQueue = vk::Queue();
	}

	if (mDevice)
	{
		mDevice.destroy();
		mDevice = vk::Device();
	}

	if (mPhysicalDevice)
	{
		mPhysicalDeviceProperies = vk::PhysicalDeviceProperties();
		mPhysicalDeviceFeatures = vk::PhysicalDeviceFeatures();
		mPhysicalDevice = vk::PhysicalDevice();
	}
	if (mInstance)
	{
		mInstance.destroy();
		mInstance = vk::Instance();
	}

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

void lune::vulkan_subsystem::createInstance()
{
	uint32_t instanceExtensionsCount{};
	char const* const* instanceExtensionsSdl = SDL_Vulkan_GetInstanceExtensions(&instanceExtensionsCount);

	std::vector<const char*> instanceExtensions{};
	for (int32_t i = 0; i < instanceExtensionsCount; ++i)
	{
		instanceExtensions.push_back(instanceExtensionsSdl[i]);
	}

	const auto allInstanceLayers = vk::enumerateInstanceLayerProperties();

	std::vector<const char*> instanceLayers{};
	for (const auto& layerProperty : allInstanceLayers)
	{
		const auto push_layer_if_available_lam = [&instanceLayers, &layerProperty](const std::string_view layer) -> void
		{
			if (layerProperty.layerName == layer)
				instanceLayers.push_back(layer.data());
		};

#ifdef LUNE_USE_VALIDATION
		push_layer_if_available_lam("VK_LAYER_KHRONOS_validation");
#endif
	}

	const vk::ApplicationInfo applicationInfo("", 0, "", 0, mApiVersion);
	const vk::InstanceCreateInfo instanceCreateInfo({}, &applicationInfo, instanceLayers, instanceExtensions);
	mInstance = vk::createInstance(instanceCreateInfo);
}

void lune::vulkan_subsystem::createPhysicalDevice()
{
	const auto physicalDevices = mInstance.enumeratePhysicalDevices();

	LN_LOG(Info, Vulkan, "Avaiable physical devices:");

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

		mPhysicalDevice = physicalDevice;
		if (vk::PhysicalDeviceType::eDiscreteGpu == physicalDeviceProperties.deviceType)
		{
			break;
		}
	}
}

void lune::vulkan_subsystem::createDevice()
{
	const auto queueFamilyProperties = mPhysicalDevice.getQueueFamilyProperties();

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

	const std::vector<vk::ExtensionProperties> availableExtensions = mPhysicalDevice.enumerateDeviceExtensionProperties();
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
													  .setPEnabledFeatures(&mPhysicalDeviceFeatures);

	mDevice = mPhysicalDevice.createDevice(deviceCreateInfo);
}

void lune::vulkan_subsystem::createQueues()
{
	const auto queueFamilyProperties = mPhysicalDevice.getQueueFamilyProperties();
	const size_t queueFamilyPropertiesSize = queueFamilyProperties.size();
	for (size_t i = 0; i < queueFamilyPropertiesSize; ++i)
	{
		const auto queueFlags = queueFamilyProperties[i].queueFlags;
		if ((queueFlags & vk::QueueFlagBits::eGraphics) && (queueFlags & vk::QueueFlagBits::eTransfer))
		{
			mGraphicsQueueIndex = i;
			mTransferQueueIndex = i;
		}
	}
	mGraphicsQueue = mDevice.getQueue(mGraphicsQueueIndex, 0);
	mTransferQueue = mDevice.getQueue(mTransferQueueIndex, 0);
}

void lune::vulkan_subsystem::createCommandPools()
{
	const vk::CommandPoolCreateInfo graphicsCreateInfo = vk::CommandPoolCreateInfo()
															 .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
															 .setQueueFamilyIndex(mGraphicsQueueIndex);
	mGraphicsCommandPool = mDevice.createCommandPool(graphicsCreateInfo);

	const vk::CommandPoolCreateInfo transferCreateInfo = vk::CommandPoolCreateInfo()
															 .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer | vk::CommandPoolCreateFlagBits::eTransient)
															 .setQueueFamilyIndex(mTransferQueueIndex);
	mTransferCommandPool = mDevice.createCommandPool(transferCreateInfo);
}
