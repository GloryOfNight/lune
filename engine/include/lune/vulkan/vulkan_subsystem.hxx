#pragma once

// this object defines and initializes Vulkan renderer
// if Vulkan is not available, the engine will not try to initialize it therefore it will not be available
// also subsystem maybe be not available on some configurations

#include "lune/core/engine_subsystem.hxx"
#include "lune/vulkan/view.hxx"

#include "vulkan_core.hxx"

#include <filesystem>
#include <map>
#include <unordered_map>
#include <utility>
#include <vector>

namespace lune
{
	struct FrameInfo
	{
		uint32 viewId{};
		uint32 imageIndex{};
		vk::CommandBuffer commandBuffer{};
	};

	namespace vulkan
	{
		static void createInstance(const vk::ApplicationInfo& applicationInfo, const std::vector<const char*>& instanceExtensions, const std::vector<const char*>& instanceLayers, VulkanContext& context);

		static void findPhysicalDevice(VulkanContext& context);

		static void createDevice(VulkanContext& context);

		static void createQueues(VulkanContext& context);

		static void createGraphicsCommandPool(VulkanContext& context);

		static void createTransferCommandPool(VulkanContext& context);

		static void createRenderPass(VulkanContext& context);

		static void createVmaAllocator(VulkanContext& context);
	} // namespace vulkan

	class vulkan_subsystem final : public EngineSubsystem
	{
	public:
		static vulkan_subsystem* get();

		vulkan_subsystem() = default;
		vulkan_subsystem(const EngineSubsystem&) = delete;
		vulkan_subsystem(EngineSubsystem&&) = delete;
		virtual ~vulkan_subsystem() = default;

		virtual bool allowInitialize() override;
		virtual void initialize() override;
		virtual void shutdown() override;

		uint32 createView(SDL_Window* window);
		vulkan::View* findView(uint32 viewId);
		void removeView(uint32 viewId);

		vulkan::SharedShader loadShader(std::filesystem::path spvPath);
		vulkan::SharedShader findShader(std::filesystem::path spvPath);

		void addPipeline(std::string name, vulkan::SharedGraphicsPipeline pipeline);
		vulkan::SharedGraphicsPipeline findPipeline(const std::string& name);

		void addPrimitive(std::string name, vulkan::SharedPrimitive primitive);
		vulkan::SharedPrimitive findPrimitive(const std::string& name);

		void addTextureImage(std::string name, vulkan::SharedTextureImage texImage);
		vulkan::SharedTextureImage findTextureImage(const std::string& name);

		bool beginNextFrame(uint32 viewId);
		FrameInfo getFrameInfo();
		void beginRenderPass();
		void sumbitFrame();

	private:
		void loadDefaultAssets();

		uint32 mApiVersion{};

		uint32 mCurrentFrameViewId{UINT32_MAX};

		std::unordered_map<std::filesystem::path, vulkan::SharedShader> mShaders{};

		std::unordered_map<std::string, vulkan::SharedGraphicsPipeline> mGraphicsPipelines{};

		std::unordered_map<std::string, vulkan::SharedPrimitive> mPrimitives{};

		std::unordered_map<std::string, vulkan::SharedTextureImage> mTextureImages{};

		std::map<uint32, std::unique_ptr<vulkan::View>> mViews{};
	};
} // namespace lune