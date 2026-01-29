#pragma once

#include "lune/lune.hxx"

#include <filesystem>
#include <memory>
#include <string_view>
#include <vector>

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

} // namespace lune