#pragma once

#include "lune/vulkan/vulkan_core.hxx"

#include "buffer.hxx"

#include <memory>

namespace lune::vulkan
{
	using UniqueMaterial = std::shared_ptr<class Material>;

	// Material class meant to be overloaded to create custom materials
	class Material
	{
	public:
		const SharedGraphicsPipeline& getPipeline() const { return mPipeline; }
		const UniqueBuffer& getBuffer() const { return mBuffer; }
		const std::vector<SharedTextureImage> getTextures() const { return mTextures; }
		const std::vector<SharedSampler> getSamplers() const { return mSamplers; }

	protected:
		SharedGraphicsPipeline mPipeline{};
		UniqueBuffer mBuffer{};
		std::vector<SharedTextureImage> mTextures{};
		std::vector<SharedSampler> mSamplers{};
	};
} // namespace lune::vulkan