#pragma once

#include "lune/vulkan/vulkan_core.hxx"

#include <map>
#include <memory>
#include <vector>

namespace lune::vulkan
{
	class GraphicsPipeline;

	using UniqueDescriptorSets = std::unique_ptr<class DescriptorSets>;

	class DescriptorSets final
	{
	public:
		DescriptorSets() = default;
		DescriptorSets(SharedGraphicsPipeline pipeline, uint32 maxSets);
		DescriptorSets(DescriptorSets&) = delete;
		DescriptorSets(DescriptorSets&&) = default;
		~DescriptorSets();

		static UniqueDescriptorSets create(SharedGraphicsPipeline pipeline, uint32 maxSets);

		void setBufferInfo(std::string_view name, uint32 index, vk::Buffer buffer, vk::DeviceSize offset, vk::DeviceSize range);
		void setImageInfo(std::string_view name, uint32 index, vk::ImageView imageView, vk::Sampler sampler);

		void updateSets(uint32 index);

		void cmdBind(vk::CommandBuffer commandBuffer, uint32 offsetSets);

	private:
		void init();
		void createDescriptorPool();

		void allocateDescriptorSets();

		SharedGraphicsPipeline mPipeline{};

		uint32 mMaxSets{};

		vk::DescriptorPool mDescriptorPool{};
		std::vector<vk::DescriptorSet> mDescriptorSets{};

		std::vector<std::map<std::string, vk::DescriptorBufferInfo>> mBufferInfos{};
		std::vector<std::map<std::string, vk::DescriptorImageInfo>> mImageInfos{};
	};
} // namespace lune::vulkan