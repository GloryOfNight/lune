#include "vulkan_subsystem.hxx"

#include "SDL3/SDL_vulkan.h"
#include "vulkan/vulkan.hpp"

#include "log.hxx"

#include <vector>

bool lune::vulkan_subsystem::allowInitialize()
{
	return nullptr != vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceVersion");
}

void lune::vulkan_subsystem::initialize()
{
	mApiVersion = VK_API_VERSION_1_3;
	createInstance();

	const auto& physicalDeviceProperties = mPhysicalDevice.getProperties();
	const uint32 deviceApiVersion = physicalDeviceProperties.apiVersion;

	LN_LOG(Info, Vulkan, "Initialized subsystem. Vulkan - {0}.{1}.{2}", VK_VERSION_MAJOR(mApiVersion), VK_VERSION_MINOR(mApiVersion), VK_VERSION_PATCH(mApiVersion))
	LN_LOG(Info, Vulkan, "Selected physical device: {0} ({1})", physicalDeviceProperties.deviceName.data(), physicalDeviceProperties.deviceID);
	LN_LOG(Info, Vulkan, "Physical device vulkan api version: {0}.{1}.{2}", VK_VERSION_MAJOR(deviceApiVersion), VK_VERSION_MINOR(deviceApiVersion), VK_VERSION_PATCH(deviceApiVersion))
}

void lune::vulkan_subsystem::shutdown()
{
	if (mPhysicalDevice)
	{
		mPhysicalDevice = vk::PhysicalDevice();
	}
	if (mInstance)
	{
		mInstance.destroy();
		mInstance = vk::Instance();
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

#ifdef DRECO_VK_USE_VALIDATION
		push_layer_if_available_lam("VK_LAYER_KHRONOS_validation");
#endif

#ifdef DRECO_VK_USE_MESA_OVERLAY
		push_layer_if_available_lam("VK_LAYER_MESA_overlay");
#endif

#ifdef DRECO_VK_USE_LUNAR_MONITOR
		push_layer_if_available_lam("VK_LAYER_LUNARG_monitor");
#endif
	}

	const vk::ApplicationInfo applicationInfo("", 0, "", 0, mApiVersion);
	const vk::InstanceCreateInfo instanceCreateInfo({}, &applicationInfo, instanceLayers, instanceExtensions);
	mInstance = vk::createInstance(instanceCreateInfo);

	const auto physicalDevices = mInstance.enumeratePhysicalDevices();
	for (vk::PhysicalDevice physicalDevice : physicalDevices)
	{
		const auto physicalDeviceProperties = physicalDevice.getProperties();
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
