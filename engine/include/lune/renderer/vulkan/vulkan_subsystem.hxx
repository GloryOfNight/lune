#pragma once

#include "subsystem.hxx"
#include "view.hxx"
#include "vulkan_core.hxx"

namespace lune
{
	namespace vulkan
	{
		static void createInstance(const vk::ApplicationInfo& applicationInfo, const std::vector<const char*>& instanceExtensions, const std::vector<const char*>& instanceLayers, vulkan_context& context);

		static void findPhysicalDevice(vulkan_context& context);

		static void createDevice(vulkan_context& context);

		static void createQueues(vulkan_context& context);

		static void createGraphicsCommandPool(vulkan_context& context);

		static void createTransferCommandPool(vulkan_context& context);

		static void createVmaAllocator(vulkan_context& context);
	} // namespace vulkan

	class vulkan_subsystem final : public subsystem
	{
	public:
		static vulkan_subsystem* get();
		static vulkan_subsystem* getChecked();

		vulkan_subsystem() = default;
		vulkan_subsystem(const subsystem&) = delete;
		vulkan_subsystem(subsystem&&) = delete;
		virtual ~vulkan_subsystem() = default;

		virtual bool allowInitialize() override;
		virtual void initialize() override;
		virtual void shutdown() override;

		void createView(SDL_Window* window);

	private:
		uint32 mApiVersion{};

		std::vector<std::unique_ptr<vulkan::view>> mViews{};
	};
} // namespace lune