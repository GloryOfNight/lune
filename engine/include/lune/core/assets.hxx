#pragma once
#include <filesystem>
#include <string>

namespace lune
{
    extern "C++" std::filesystem::path& getEngineAssetsRootDir();
    extern "C++" std::filesystem::path& getEngineShadersRootDir();

	struct EngineAssetPath
	{
		EngineAssetPath() = default;
		EngineAssetPath(std::string_view relativeAssetPath);

		std::filesystem::path operator*() const { return mPath; }

	protected:
		std::filesystem::path mPath{};
	};

    struct EngineShaderPath : public EngineAssetPath
    {
        EngineShaderPath() = default;
        EngineShaderPath(std::string_view relativeAssetPath);
    };

} // namespace lune