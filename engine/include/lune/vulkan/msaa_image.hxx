#pragma once

#include "lune/vulkan/vulkan_core.hxx"

namespace lune::vulkan
{
	using UniqueMsaaImage = std::unique_ptr<class MsaaImage>;

	class MsaaImage final
	{
	public:
		MsaaImage() = default;
		MsaaImage(MsaaImage&) = delete;
		MsaaImage(MsaaImage&&) = default;
		~MsaaImage();

		static UniqueMsaaImage create(vk::Extent2D extent);

		vk::Format getFormat() const { return mFormat; }
		vk::Image getImage() const { return mImage; }
		vk::ImageView getImageView() const { return mImageView; }

	private:
		void init(vk::Extent2D extent);

		void createImage(vk::Extent2D extent);
		void createImageView();

		vk::Format mFormat{};
		vk::SampleCountFlagBits mSampleCount{vk::SampleCountFlagBits::e1};

		vk::Image mImage{};
		VmaAllocation mVmaAllocation{};
		vk::ImageView mImageView{};
	};
} // namespace lune::vulkan