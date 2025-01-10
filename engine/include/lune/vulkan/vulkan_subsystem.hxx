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
		void removeView(uint32 viewId);

		vulkan::SharedShader loadShader(std::filesystem::path spvPath);
		vulkan::SharedShader findShader(std::filesystem::path spvPath);

		void addPipeline(std::string name, vulkan::SharedGraphicsPipeline pipeline);
		vulkan::SharedGraphicsPipeline findPipeline(std::string name);

		bool beginNextFrame(uint32 viewId);
		std::pair<uint32, vk::CommandBuffer> getFrameInfo(uint32 viewId);
		void sumbitFrame(uint32 viewId);

	private:
		uint32 mApiVersion{};

		std::unordered_map<std::filesystem::path, vulkan::SharedShader> mShaders{};

		std::unordered_map<std::string, vulkan::SharedGraphicsPipeline> mGraphicsPipelines{};

		std::map<uint32, std::unique_ptr<vulkan::View>> mViews{};
	};
} // namespace lune