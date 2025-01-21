#pragma once

#include "lune/lune.hxx"
#include "vulkan/vulkan.hpp"

#include "vma.hxx"

#include <SDL3_image/SDL_image.h>
#include <memory>
#include <span>

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

		static UniqueTextureImage create(std::span<const SDL_Surface*, 6> cubeSurfaces);

		static UniqueTextureImage create(const SDL_Surface* surface);

		void destroy();

		vk::Format getFormat() const { return mFormat; }
		vk::Image getImage() const { return mImage; }
		vk::ImageView getImageView() const { return mImageView; }

	private:
		void init(std::span<const SDL_Surface*> surfaces);

		void createImage(vk::ImageCreateFlagBits flags, uint32 layerCount, vk::Extent3D extent);
		void createImageView(vk::ImageViewType type, uint32 layerCount);

		void copyPixelsToImage(std::span<const SDL_Surface*> surfaces, uint32 layerCount, vk::Extent3D extent);

		vk::Format mFormat{};
		vk::Image mImage{};
		VmaAllocation mVmaAllocation{};
		vk::ImageView mImageView{};
		vk::Sampler mSampler{};
	};
} // namespace lune::vulkan