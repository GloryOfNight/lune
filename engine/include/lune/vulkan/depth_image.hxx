#pragma once

#include "lune/vulkan/vulkan_core.hxx"

#include <memory>

namespace lune::vulkan
{
	class DepthImage final
	{
	public:
		DepthImage() = default;
		DepthImage(DepthImage&) = delete;
		DepthImage(DepthImage&&) = default;
		~DepthImage() = default;

		static std::unique_ptr<DepthImage> create();

		void init(class View* view);
		void destroy();

		vk::Format getFormat() const { return mFormat; }
		vk::Image getImage() const { return mImage; }
		vk::ImageView getImageView() const { return mImageView; }

	private:
		void createImage();
		void allocateMemory();
		void createImageView();
		void transitionImageLayout(vk::ImageLayout oldLayout, vk::ImageLayout newLayout);

		vk::Format mFormat{vk::Format::eD32Sfloat};
		vk::Extent2D mExtent{};
		vk::SampleCountFlagBits mSampleCount{vk::SampleCountFlagBits::e1};

		vk::Image mImage{};
		VmaAllocation mVmaAllocation{};
		vk::ImageView mImageView{};
	};
} // namespace lune::vulkan