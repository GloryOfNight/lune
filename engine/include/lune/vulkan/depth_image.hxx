#pragma once

#include "lune/vulkan/vulkan_core.hxx"

#include <memory>

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

		static UniqueDepthImage create(class View* view);

		vk::Format getFormat() const { return mFormat; }
		vk::Image getImage() const { return mImage; }
		vk::ImageView getImageView() const { return mImageView; }

	private:
		void init(class View* view);

		void createImage();
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