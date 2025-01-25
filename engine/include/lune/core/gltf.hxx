#pragma once

#include "lune/lune.hxx"

#include <filesystem>
#include <string_view>

namespace lune
{
	class Scene;
	struct gltf
	{
		static bool loadInScene(std::filesystem::path gltfScene, std::string_view alias, class Scene* scene);
	};
} // namespace lune