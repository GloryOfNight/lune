#pragma once

#include "lune/vulkan/vulkan_core.hxx"

namespace lune::vulkan
{
	class MsaaImage
	{
	public:
		MsaaImage() = default;
		MsaaImage(MsaaImage&) = delete;
		MsaaImage(MsaaImage&&) = default;
		~MsaaImage() = default;

		static std::unique_ptr<MsaaImage> create();

		void init(class View* view);
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