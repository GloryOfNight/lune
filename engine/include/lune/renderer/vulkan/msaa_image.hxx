#pragma once

#include "vulkan/vulkan.hpp"

#include "vk_mem_alloc.h"

namespace lune::vulkan
{
	class msaa_image
	{
	public:
		msaa_image() = default;
		msaa_image(msaa_image&) = delete;
		msaa_image(msaa_image&&) = default;
		~msaa_image() = default;

		static std::unique_ptr<msaa_image> create();

		void init(class view* view);
		void destroy();

		vk::Format getFormat() const { return mFormat; }
		vk::Image getImage() const { return mImage; }
		vk::ImageView getImageView() const { return mImageView; }

	private:
		vk::Format mFormat{};
		vk::Extent2D mExtent{};
		vk::SampleCountFlagBits mSampleCount{vk::SampleCountFlagBits::e1};

		vk::Image mImage{};
		VmaAllocation mVmaAllocation{};
		vk::ImageView mImageView{};

        void createImage();
        void allocateMemory();
        void createImageView();
	};
} // namespace lune