#pragma once

#include "lune/lune.hxx"

#include "tiny_gltf.h"

#include <filesystem>
#include <string_view>

namespace lune
{
	class Scene;
	namespace gltf
	{
		static bool loadInScene(std::filesystem::path gltfScene, std::string_view alias, class Scene* scene);
	}
} // namespace lune