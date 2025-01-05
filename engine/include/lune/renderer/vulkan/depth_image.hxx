#pragma once

#include "vulkan_core.hxx"

#include <memory>

namespace lune::vulkan
{
	class depth_image final
	{
	public:
		depth_image() = default;
		depth_image(depth_image&) = delete;
		depth_image(depth_image&&) = default;
		~depth_image() = default;

		static std::unique_ptr<depth_image> create();

		void init(class view* view);
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