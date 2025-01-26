#pragma once

#include "lune/lune.hxx"
#include "lune/vulkan/buffer.hxx"
#include "lune/vulkan/pipeline.hxx"
#include "lune/vulkan/vulkan_core.hxx"
#include "lune/vulkan/vulkan_custom_resource.hxx"

#include <filesystem>
#include <memory>
#include <string_view>

namespace tinygltf
{
	struct Model;
	struct Material;
} // namespace tinygltf

namespace lune
{
	class Scene;
}

namespace lune
{
	namespace gltf
	{
		extern "C++" std::vector<uint64> loadInScene(std::filesystem::path gltfScene, std::string_view alias, class Scene* scene);
	}; // namespace gltf

	namespace vulkan::gltf
	{
		using SharedMaterial = std::shared_ptr<class Material>;
		class Material : public VulkanCustomResource
		{
		public:
			void init(const tinygltf::Model* tinyModel, const tinygltf::Material* tinyMaterial, const std::string_view alias);

			const SharedGraphicsPipeline& getPipeline() const { return mPipeline; }
			const UniqueBuffer& getBuffer() const { return mBuffer; }
			const std::vector<SharedTextureImage> getTextures() const { return mTextures; }
			const std::vector<SharedSampler> getSamplers() const { return mSamplers; }

		private:
			SharedGraphicsPipeline mPipeline{};
			UniqueBuffer mBuffer{};
			std::vector<SharedTextureImage> mTextures{};
			std::vector<SharedSampler> mSamplers{};
		};
	} // namespace vulkan::gltf
} // namespace lune