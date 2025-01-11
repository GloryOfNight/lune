#pragma once

#include "vulkan/vulkan.hpp"

#include "vk_mem_alloc.h"

#include <SDL3_image/SDL_image.h>
#include <memory>

namespace lune::vulkan
{
	using UniqueTextureImage = std::unique_ptr<class TextureImage>;

	class TextureImage final
	{
	public:
		TextureImage() = default;
		TextureImage(const TextureImage&) = delete;
		TextureImage(TextureImage&&) = delete;
		~TextureImage();

		static UniqueTextureImage create(const SDL_Surface& surface);

		void destroy();

		vk::Format getFormat() const { return mFormat; }
		vk::Image getImage() const { return mImage; }
		vk::ImageView getImageView() const { return mImageView; }
		vk::Sampler getSampler() const { return mSampler; }

	private:
		void init(const SDL_Surface& surface);

		void createImage();
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