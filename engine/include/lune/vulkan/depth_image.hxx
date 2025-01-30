#pragma once

#include "lune/vulkan/vulkan_core.hxx"

#include <memory>
#include <vulkan/vulkan_structs.hpp>

namespace lune::vulkan
{
	using UniqueDepthImage = std::unique_ptr<class DepthImage>;

	class DepthImage final
	{
	public:
		DepthImage() = default;
		DepthImage(DepthImage&) = delete;
		DepthImage(DepthImage&&) = default;
		~DepthImage();

		static UniqueDepthImage create(vk::Extent2D extent);

		vk::Format getFormat() const { return mFormat; }
		vk::Image getImage() const { return mImage; }
		vk::ImageView getImageView() const { return mImageView; }

	private:
		void init(vk::Extent2D extent);

		void createImage(vk::Extent2D extent);
		void createImageView();
		void transitionImageLayout(vk::ImageLayout oldLayout, vk::ImageLayout newLayout);

		vk::Format mFormat{vk::Format::eD32Sfloat};
		vk::SampleCountFlagBits mSampleCount{vk::SampleCountFlagBits::e1};

		vk::Image mImage{};
		VmaAllocation mVmaAllocation{};
		vk::ImageView mImageView{};
	};
} // namespace lune::vulkan