#include "lune/vulkan/vulkan_subsystem.hxx"

#include "SDL3/SDL_vulkan.h"
#include "SDL3_image/SDL_image.h"
#include "lune/core/assets.hxx"
#include "lune/core/log.hxx"
#include "lune/lune.hxx"
#include "lune/vulkan/pipeline.hxx"
#include "lune/vulkan/primitive.hxx"
#include "lune/vulkan/shader.hxx"
#include "lune/vulkan/texture_image.hxx"

#include <vector>

#define LUNE_USE_VALIDATION

vk::Format findSupportedDepthFormat(const vk::PhysicalDevice physicalDevice) noexcept
{
	constexpr auto formatCandidates = std::array{vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint};
	const auto imageTiling = vk::ImageTiling::eOptimal;
	const auto formatFeatureFlags = vk::FormatFeatureFlagBits::eDepthStencilAttachment;

	for (const auto format : formatCandidates)
	{
		const auto formatProperties = physicalDevice.getFormatProperties(format);
		if (imageTiling == vk::ImageTiling::eLinear &&
			(formatProperties.linearTilingFeatures & formatFeatureFlags) == formatFeatureFlags)
		{
			return format;
		}
		else if (imageTiling == vk::ImageTiling::eOptimal &&
				 (formatProperties.optimalTilingFeatures & formatFeatureFlags) == formatFeatureFlags)
		{
			return format;
		}
	}
	return vk::Format::eUndefined;
}

lune::VulkanSubsystem::~VulkanSubsystem()
{
	getVulkanContext().device.waitIdle();

	mTextureImages.clear();

	for (auto& [path, shader] : mShaders)
	{
		shader->destroy();
	}
	mShaders.clear();

	for (auto& [name, pipeline] : mGraphicsPipelines)
	{
		pipeline->destroy();
	}
	mGraphicsPipelines.clear();

	for (auto& [name, primitive] : mPrimitives)
	{
		primitive->destroy();
	}
	mPrimitives.clear();
	mViews.clear();

	getVulkanDeleteQueue().cleanup();

	if (getVulkanContext().graphicsCommandPool)
		getVulkanContext().device.destroyCommandPool(getVulkanContext().graphicsCommandPool);

	if (getVulkanContext().transferCommandPool)
		getVulkanContext().device.destroyCommandPool(getVulkanContext().transferCommandPool);

	if (getVulkanContext().renderPass)
		getVulkanContext().device.destroyRenderPass(getVulkanContext().renderPass);

	if (getVulkanContext().vmaAllocator)
		vmaDestroyAllocator(getVulkanContext().vmaAllocator);

	if (getVulkanContext().device)
		getVulkanContext().device.destroy();

	if (getVulkanContext().instance)
		getVulkanContext().instance.destroy();

	getVulkanContext() = VulkanContext{};
}

bool lune::VulkanSubsystem::allowInitialize()
{
	return nullptr != vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceVersion");
}

void lune::VulkanSubsystem::initialize()
{
	mApiVersion = VK_API_VERSION_1_3;
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
	std::vector<const char*> instanceLayers = {};

	vulkan::createInstance(applicationInfo, instanceExtensions, instanceLayers, getVulkanContext());
	vulkan::findPhysicalDevice(getVulkanContext());

	if (nullptr == getVulkanContext().physicalDevice)
	{
		LN_LOG(Fatal, Vulkan, "Failed to find suitable device for Vulkan");
		return;
	}

	getVulkanConfig().colorFormat = vk::Format::eB8G8R8A8Unorm;
	getVulkanConfig().depthFormat = findSupportedDepthFormat(getVulkanContext().physicalDevice);
	getVulkanConfig().sampleCount = vk::SampleCountFlagBits::e1;

	const auto mPhysicalDeviceProperies = getVulkanContext().physicalDevice.getProperties();

	const uint32 deviceApiVersion = mPhysicalDeviceProperies.apiVersion;
	LN_LOG(Info, Vulkan, "Selected device:", mPhysicalDeviceProperies.deviceName.data(), mPhysicalDeviceProperies.deviceID);
	LN_LOG(Info, Vulkan, "	GPU: {0} (id: {1})", mPhysicalDeviceProperies.deviceName.data(), mPhysicalDeviceProperies.deviceID);
	LN_LOG(Info, Vulkan, "	API: {0}.{1}.{2}", VK_VERSION_MAJOR(deviceApiVersion), VK_VERSION_MINOR(deviceApiVersion), VK_VERSION_PATCH(deviceApiVersion))

	vulkan::createDevice(getVulkanContext());
	vulkan::createRenderPass(getVulkanContext());
	vulkan::createQueues(getVulkanContext());
	vulkan::createGraphicsCommandPool(getVulkanContext());
	vulkan::createTransferCommandPool(getVulkanContext());
	vulkan::createVmaAllocator(getVulkanContext());

	loadDefaultAssets();
}

uint32 lune::VulkanSubsystem::createView(SDL_Window* window)
{
	static uint32 viewIdsCounter = 0;

	auto newView = lune::vulkan::View::create(window);
	if (newView) [[likely]]
	{
		const auto& [it, res] = mViews.emplace(viewIdsCounter++, std::move(newView));
		auto& [viewId, view] = *it;
		return viewId;
	}
	return UINT32_MAX;
}

lune::vulkan::View* lune::VulkanSubsystem::findView(uint32 viewId)
{
	if (const auto it = mViews.find(viewId); it != mViews.end()) [[likely]]
	{
		auto& [viewId, view] = *it;
		return view.get();
	}
	return nullptr;
}

void lune::VulkanSubsystem::removeView(uint32 viewId)
{
	if (const auto it = mViews.find(viewId); it != mViews.end())
	{
		auto& [viewId, view] = *it;
		mViews.erase(it);
	}
}

lune::vulkan::SharedShader lune::VulkanSubsystem::loadShader(std::filesystem::path spvPath)
{
	if (auto shader = findShader(spvPath); shader)
		return shader;

	auto shader = vulkan::Shader::create(spvPath);
	if (shader)
	{
		mShaders.emplace(spvPath, shader);
	}
	return shader;
}

lune::vulkan::SharedShader lune::VulkanSubsystem::findShader(std::filesystem::path spvPath)
{
	auto findRes = mShaders.find(spvPath);
	return findRes != mShaders.end() ? findRes->second : nullptr;
}

void lune::VulkanSubsystem::addPipeline(std::string name, vulkan::SharedGraphicsPipeline pipeline)
{
	if (mGraphicsPipelines.find(name) != mGraphicsPipelines.end())
	{
		LN_LOG(Fatal, Vulkan, "Can't emplace new pipeline, name already taken: {}", name);
		return;
	}
	mGraphicsPipelines.emplace(name, std::move(pipeline));
}

lune::vulkan::SharedGraphicsPipeline lune::VulkanSubsystem::findPipeline(const std::string& name)
{
	auto findRes = mGraphicsPipelines.find(name);
	return findRes != mGraphicsPipelines.end() ? findRes->second : nullptr;
}

void lune::VulkanSubsystem::addPrimitive(std::string name, vulkan::SharedPrimitive primitive)
{
	if (mPrimitives.find(name) != mPrimitives.end())
	{
		LN_LOG(Fatal, Vulkan, "Can't emplace new primitive, name already taken: {}", name);
		return;
	}
	mPrimitives.emplace(name, std::move(primitive));
}

lune::vulkan::SharedPrimitive lune::VulkanSubsystem::findPrimitive(const std::string& name)
{
	auto findRes = mPrimitives.find(name);
	return findRes != mPrimitives.end() ? findRes->second : nullptr;
}

void lune::VulkanSubsystem::addTextureImage(std::string name, vulkan::SharedTextureImage texImage)
{
	if (mTextureImages.find(name) != mTextureImages.end())
	{
		LN_LOG(Fatal, Vulkan, "Can't emplace new primitive, name already taken: {}", name);
		return;
	}
	mTextureImages.emplace(name, std::move(texImage));
}

lune::vulkan::SharedTextureImage lune::VulkanSubsystem::findTextureImage(const std::string& name)
{
	auto findRes = mTextureImages.find(name);
	return findRes != mTextureImages.end() ? findRes->second : nullptr;
}

bool lune::VulkanSubsystem::beginNextFrame(uint32 viewId)
{
	if (const auto it = mViews.find(viewId); it != mViews.end()) [[likely]]
	{
		auto& [viewId, view] = *it;
		mCurrentFrameViewId = viewId;
		return view->beginNextFrame();
	}
	return false;
}

lune::FrameInfo lune::VulkanSubsystem::getFrameInfo()
{
	if (const auto it = mViews.find(mCurrentFrameViewId); it != mViews.end()) [[likely]]
	{
		auto& [viewId, view] = *it;

		FrameInfo info{};
		info.viewId = mCurrentFrameViewId;
		info.imageIndex = view->getImageIndex();
		info.commandBuffer = view->getCurrentImageCmdBuffer();

		return std::move(info);
	}
	return FrameInfo();
}

void lune::VulkanSubsystem::beginRenderPass()
{
	if (const auto it = mViews.find(mCurrentFrameViewId); it != mViews.end()) [[likely]]
	{
		auto& [viewId, view] = *it;
		view->beginRenderPass();
	}
}

void lune::VulkanSubsystem::sumbitFrame()
{
	if (const auto it = mViews.find(mCurrentFrameViewId); it != mViews.end()) [[likely]]
	{
		auto& [viewId, view] = *it;
		view->sumbit();
	}
	mCurrentFrameViewId = UINT32_MAX;
}

void lune::VulkanSubsystem::loadDefaultAssets()
{
	{
		auto shVert = loadShader(*EngineShaderPath("sprite.vert.spv"));
		auto shFrag = loadShader(*EngineShaderPath("sprite.frag.spv"));
		addPipeline("lune::sprite", vulkan::GraphicsPipeline::create(shVert, shFrag));
	}
	{
		auto shVert = loadShader(*EngineShaderPath("skybox.vert.spv"));
		auto shFrag = loadShader(*EngineShaderPath("skybox.frag.spv"));
		addPipeline("lune::skybox", vulkan::GraphicsPipeline::create(shVert, shFrag));
	}

	{
		SDL_Surface* scarlet = IMG_Load((*EngineAssetPath("scarlet.png")).generic_string().data());
		addTextureImage("lune::scarlet", vulkan::TextureImage::create(scarlet));
	}
	{
		SDL_Surface* right = IMG_Load((*EngineAssetPath("skyboxes/sea/right.png")).generic_string().data());
		SDL_Surface* left = IMG_Load((*EngineAssetPath("skyboxes/sea/left.png")).generic_string().data());
		SDL_Surface* top = IMG_Load((*EngineAssetPath("skyboxes/sea/top.png")).generic_string().data());
		SDL_Surface* bottom = IMG_Load((*EngineAssetPath("skyboxes/sea/bottom.png")).generic_string().data());
		SDL_Surface* front = IMG_Load((*EngineAssetPath("skyboxes/sea/front.png")).generic_string().data());
		SDL_Surface* back = IMG_Load((*EngineAssetPath("skyboxes/sea/back.png")).generic_string().data());
		std::array<const SDL_Surface*, 6> surfaces = {right, left, top, bottom, front, back};
		addTextureImage("lune::skyboxes::sea", vulkan::TextureImage::create(std::span<const SDL_Surface*, 6>(surfaces)));
	}

	{
		std::vector<Vertex32> vertices = {
			{{-1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}, // Top-left
			{{1.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},	 // Top-right
			{{1.0f, -1.0f, 0.0f}, {1.0f, 0.0f}}, // Bottom-right
			{{-1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}, // Top-left
			{{1.0f, -1.0f, 0.0f}, {1.0f, 0.0f}}, // Bottom-right
			{{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}} // Bottom-left
		};
		addPrimitive("lune::plane", vulkan::Primitive::create<Vertex32>(vertices));
	}

	{
		std::vector<Vertex32> vertices = {
			{{-1.0f, -1.0f, 1.0f}, {0.0f, 0.0f}},  // Front face; Bottom-left
			{{1.0f, -1.0f, 1.0f}, {1.0f, 0.0f}},   // Bottom-right
			{{1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},	   // Top-right
			{{-1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},   // Top-left
			{{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f}}, // Back face; Bottom-left
			{{1.0f, -1.0f, -1.0f}, {1.0f, 0.0f}},  // Bottom-right
			{{1.0f, 1.0f, -1.0f}, {1.0f, 1.0f}},   // Top-right
			{{-1.0f, 1.0f, -1.0f}, {0.0f, 1.0f}}   // Top-left
		};

		std::vector<Index16> indices = {
			0, 1, 2, 0, 2, 3, // Front face
			4, 5, 6, 4, 6, 7, // Back face
			4, 0, 3, 4, 3, 7, // Left face
			1, 5, 6, 1, 6, 2, // Right face
			3, 2, 6, 3, 6, 7, // Top face
			4, 5, 1, 4, 1, 0  // Bottom face
		};
		addPrimitive("lune::box", vulkan::Primitive::create<Vertex32, Index16>(vertices, indices));
	}

	{
		std::vector<Vertex3> vertices = {
			{{-1.0f, 1.0f, -1.0f}},
			{{-1.0f, -1.0f, -1.0f}},
			{{1.0f, -1.0f, -1.0f}},
			{{1.0f, -1.0f, -1.0f}},
			{{1.0f, 1.0f, -1.0f}},
			{{-1.0f, 1.0f, -1.0f}},
			{{-1.0f, -1.0f, 1.0f}},
			{{-1.0f, -1.0f, -1.0f}},
			{{-1.0f, 1.0f, -1.0f}},
			{{-1.0f, 1.0f, -1.0f}},
			{{-1.0f, 1.0f, 1.0f}},
			{{-1.0f, -1.0f, 1.0f}},
			{{1.0f, -1.0f, -1.0f}},
			{{1.0f, -1.0f, 1.0f}},
			{{1.0f, 1.0f, 1.0f}},
			{{1.0f, 1.0f, 1.0f}},
			{{1.0f, 1.0f, -1.0f}},
			{{1.0f, -1.0f, -1.0f}},
			{{-1.0f, -1.0f, 1.0f}},
			{{-1.0f, 1.0f, 1.0f}},
			{{1.0f, 1.0f, 1.0f}},
			{{1.0f, 1.0f, 1.0f}},
			{{1.0f, -1.0f, 1.0f}},
			{{-1.0f, -1.0f, 1.0f}},
			{{-1.0f, 1.0f, -1.0f}},
			{{1.0f, 1.0f, -1.0f}},
			{{1.0f, 1.0f, 1.0f}},
			{{1.0f, 1.0f, 1.0f}},
			{{-1.0f, 1.0f, 1.0f}},
			{{-1.0f, 1.0f, -1.0f}},
			{{-1.0f, -1.0f, -1.0f}},
			{{-1.0f, -1.0f, 1.0f}},
			{{1.0f, -1.0f, -1.0f}},
			{{1.0f, -1.0f, -1.0f}},
			{{-1.0f, -1.0f, 1.0f}},
			{{1.0f, -1.0f, 1.0f}}};
		addPrimitive("lune::skybox", vulkan::Primitive::create<Vertex3>(vertices));
	}
}

void lune::vulkan::createInstance(const vk::ApplicationInfo& applicationInfo, const std::vector<const char*>& instanceExtensions, const std::vector<const char*>& instanceLayers, VulkanContext& context)
{
	const vk::InstanceCreateInfo instanceCreateInfo = vk::InstanceCreateInfo()
														  .setPApplicationInfo(&applicationInfo)
														  .setPEnabledExtensionNames(instanceExtensions)
														  .setPEnabledLayerNames(instanceLayers);
	context.instance = vk::createInstance(instanceCreateInfo);
}

void lune::vulkan::findPhysicalDevice(VulkanContext& context)
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

void lune::vulkan::createDevice(VulkanContext& context)
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

void lune::vulkan::createQueues(VulkanContext& context)
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

	context.queueFamilyIndices.push_back(context.graphicsQueueIndex);
	if (context.graphicsQueueIndex != context.transferQueueIndex)
	{
		context.queueFamilyIndices.push_back(context.transferQueueIndex);
	}

	context.graphicsQueue = context.device.getQueue(context.graphicsQueueIndex, 0);
	context.transferQueue = context.device.getQueue(context.transferQueueIndex, 0);
}

void lune::vulkan::createGraphicsCommandPool(VulkanContext& context)
{
	const vk::CommandPoolCreateInfo graphicsCreateInfo = vk::CommandPoolCreateInfo()
															 .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
															 .setQueueFamilyIndex(context.graphicsQueueIndex);
	context.graphicsCommandPool = context.device.createCommandPool(graphicsCreateInfo);
}

void lune::vulkan::createTransferCommandPool(VulkanContext& context)
{
	const vk::CommandPoolCreateInfo transferCreateInfo = vk::CommandPoolCreateInfo()
															 .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer | vk::CommandPoolCreateFlagBits::eTransient)
															 .setQueueFamilyIndex(context.transferQueueIndex);
	context.transferCommandPool = context.device.createCommandPool(transferCreateInfo);
}

void lune::vulkan::createRenderPass(VulkanContext& context)
{
	const bool msaaEnabled = getVulkanConfig().sampleCount != vk::SampleCountFlagBits::e1;

	std::vector<vk::AttachmentDescription> attachmentsDescriptions;

	std::vector<vk::AttachmentReference> attachmentReferences;
	std::vector<vk::AttachmentReference> resolveAttachmentReferences;

	attachmentsDescriptions.emplace_back() // color
		.setFormat(getVulkanConfig().colorFormat)
		.setSamples(getVulkanConfig().sampleCount)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(msaaEnabled ? vk::AttachmentStoreOp::eDontCare : vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(msaaEnabled ? vk::ImageLayout::eColorAttachmentOptimal : vk::ImageLayout::ePresentSrcKHR);
	attachmentReferences.push_back(vk::AttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal));

	attachmentsDescriptions.emplace_back() // depth
		.setFormat(getVulkanConfig().depthFormat)
		.setSamples(getVulkanConfig().sampleCount)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
	attachmentReferences.push_back(vk::AttachmentReference(1, vk::ImageLayout::eDepthStencilAttachmentOptimal));

	if (msaaEnabled)
	{
		attachmentsDescriptions.emplace_back() // color msaa
			.setFormat(getVulkanConfig().colorFormat)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);
		resolveAttachmentReferences.push_back(vk::AttachmentReference(2, vk::ImageLayout::eColorAttachmentOptimal));
	}

	const vk::SubpassDescription subpassDescription =
		vk::SubpassDescription()
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
			.setColorAttachments({1, &attachmentReferences[0]})
			.setPDepthStencilAttachment(&attachmentReferences[1])
			.setPResolveAttachments(resolveAttachmentReferences.data());

	const vk::SubpassDependency subpassDependecy =
		vk::SubpassDependency()
			.setSrcSubpass(VK_SUBPASS_EXTERNAL)
			.setDstSubpass(0)
			.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests)
			.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests)
			.setSrcAccessMask(vk::AccessFlagBits(0))
			.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite);

	const vk::RenderPassCreateInfo renderPassCreateInfo =
		vk::RenderPassCreateInfo()
			.setAttachments(attachmentsDescriptions)
			.setSubpasses({1, &subpassDescription})
			.setDependencies({1, &subpassDependecy});

	getVulkanContext().renderPass = getVulkanContext().device.createRenderPass(renderPassCreateInfo);
}

void lune::vulkan::createVmaAllocator(VulkanContext& context)
{
	VmaAllocatorCreateInfo allocatorCreateInfo = {};
	allocatorCreateInfo.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
	allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_3;
	allocatorCreateInfo.instance = context.instance;
	allocatorCreateInfo.physicalDevice = context.physicalDevice;
	allocatorCreateInfo.device = context.device;

	vmaCreateAllocator(&allocatorCreateInfo, &context.vmaAllocator);
}
