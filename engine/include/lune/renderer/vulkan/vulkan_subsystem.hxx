#pragma once

#include "vulkan/vulkan.hpp"

#include "lune.hxx"
#include "subsystem.hxx"
#include "view.hxx"

namespace lune
{
	class vulkan_subsystem final : public subsystem
	{
	public:
		static vulkan_subsystem* get();

		vulkan_subsystem() = default;
		vulkan_subsystem(const subsystem&) = delete;
		vulkan_subsystem(subsystem&&) = delete;
		virtual ~vulkan_subsystem() = default;

		virtual bool allowInitialize() override;
		virtual void initialize() override;
		virtual void shutdown() override;

		void createView(SDL_Window* window);

		vk::Instance getInstance() const { return mInstance; }
		vk::PhysicalDevice getPhysicalDevice() const { return mPhysicalDevice; }
		vk::Device getDevice() const { return mDevice; }
		vk::Queue getGraphicsQueue() const { return mGraphicsQueue; }
		vk::Queue getTransferQueue() const { return mTransferQueue; }
		std::vector<uint32> getQueueFamilyIndices() const { return mGraphicsQueueIndex == mTransferQueueIndex // if graphics and transfer queues are the same return only one index
																	   ? std::vector<uint32>{mGraphicsQueueIndex}
																	   : std::vector<uint32>{mGraphicsQueueIndex, mTransferQueueIndex}; }

		vk::CommandPool getGraphicsCommandPool() const { return mGraphicsCommandPool; }
		vk::CommandPool getTransferCommandPool() const { return mTransferCommandPool; }

	private:
		void createInstance();

		void createPhysicalDevice();

		void createDevice();

		void createQueues();

		void createCommandPools();

		uint32 mApiVersion{};
		vk::Instance mInstance{};

		vk::PhysicalDevice mPhysicalDevice{};
		vk::PhysicalDeviceProperties mPhysicalDeviceProperies{};
		vk::PhysicalDeviceFeatures mPhysicalDeviceFeatures{};

		vk::Device mDevice{};
		uint32 mGraphicsQueueIndex{}, mTransferQueueIndex{};
		vk::Queue mGraphicsQueue{}, mTransferQueue{};

		vk::CommandPool mGraphicsCommandPool{}, mTransferCommandPool{};

		std::vector<std::unique_ptr<vulkan::view>> mViews{};
	};
} // namespace lune