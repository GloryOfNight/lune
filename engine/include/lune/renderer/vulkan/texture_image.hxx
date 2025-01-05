#pragma once

#include "vulkan/vulkan.hpp"

#include "vk_mem_alloc.h"

#include <SDL3_image/SDL_image.h>
#include <memory>

namespace lune::vulkan
{
	class texture_image
	{
	public:
		texture_image() = default;
		texture_image(const texture_image&) = delete;
		texture_image(texture_image&&) = delete;
		~texture_image() = default;

		static std::unique_ptr<texture_image> create();

		void init(const SDL_Surface& surface);
		void destroy();

		vk::Format getFormat() const { return mFormat; }
		vk::Image getImage() const { return mImage; }
		vk::ImageView getImageView() const { return mImageView; }

	private:
		void createImage();
		void allocateMemory();
		void createImageView();
		void createSampler();

		void copyPixelsToImage(const SDL_Surface& surface);

		vk::Format mFormat{};
		vk::Extent2D mExtent{};
		vk::SampleCountFlagBits mSampleCount{vk::SampleCountFlagBits::e1};

		vk::Image mImage{};
		VmaAllocation mVmaAllocation{};
		vk::ImageView mImageView{};
		vk::Sampler mSampler{};
	};
} // namespace lune::vulkan