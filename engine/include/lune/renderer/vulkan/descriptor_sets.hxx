#pragma once

#include "vulkan_core.hxx"

#include <memory>
#include <vector>
#include <map>

namespace lune::vulkan
{
	class pipeline;

	class descriptor_sets
	{
	public:
		descriptor_sets() = default;
		descriptor_sets(descriptor_sets&) = delete;
		descriptor_sets(descriptor_sets&&) = default;
		~descriptor_sets() = default;

		static std::unique_ptr<descriptor_sets> create();

		void init(std::shared_ptr<pipeline> pipeline, uint32 maxSets);
		void destroy();

        void addBufferInfo(std::string_view name, uint32 index, vk::Buffer buffer, vk::DeviceSize offset, vk::DeviceSize range);

        void addImageInfo(std::string_view name, uint32 index, vk::ImageView imageView, vk::Sampler sampler);

        void updateSets(uint32 index);
	private:
		void createDescriptorPool();

        void allocateDescriptorSets();

		std::shared_ptr<pipeline> mPipeline{};

        uint32 mMaxSets{};

		vk::DescriptorPool mDescriptorPool{};
		std::vector<vk::DescriptorSet> mDescriptorSets{};

        std::vector<std::map<std::string, vk::DescriptorBufferInfo>> mBufferInfos{};
        std::vector<std::map<std::string, vk::DescriptorImageInfo>> mImageInfos{};
	};
} // namespace lune::vulkan