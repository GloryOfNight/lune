#pragma once

#include "lune/vulkan/vulkan_core.hxx"

#include <map>
#include <memory>
#include <vector>

namespace lune::vulkan
{
	class Pipeline;

	class DescriptorSets
	{
	public:
		DescriptorSets() = default;
		DescriptorSets(DescriptorSets&) = delete;
		DescriptorSets(DescriptorSets&&) = default;
		~DescriptorSets() = default;

		static std::unique_ptr<DescriptorSets> create();

		void init(std::shared_ptr<Pipeline> pipeline, uint32 maxSets);
		void destroy();

		void addBufferInfo(std::string_view name, uint32 index, vk::Buffer buffer, vk::DeviceSize offset, vk::DeviceSize range);

		void addImageInfo(std::string_view name, uint32 index, vk::ImageView imageView, vk::Sampler sampler);

		void updateSets(uint32 index);

		void cmdBind(vk::CommandBuffer commandBuffer, uint32 offsetSets);

	private:
		void createDescriptorPool();

		void allocateDescriptorSets();

		std::shared_ptr<Pipeline> mPipeline{};

		uint32 mMaxSets{};

		vk::DescriptorPool mDescriptorPool{};
		std::vector<vk::DescriptorSet> mDescriptorSets{};

		std::vector<std::map<std::string, vk::DescriptorBufferInfo>> mBufferInfos{};
		std::vector<std::map<std::string, vk::DescriptorImageInfo>> mImageInfos{};
	};
} // namespace lune::vulkan