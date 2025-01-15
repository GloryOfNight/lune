#pragma once
#include <filesystem>
#include <string>

namespace lune
{
	extern "C++" std::filesystem::path& getEngineAssetsRootDir();

	struct EngineAssetPath
	{
		EngineAssetPath() = default;
		EngineAssetPath(std::string_view relativeAssetPath);

		std::filesystem::path operator*() const { return mPath; }

	protected:
		std::filesystem::path mPath{};
	};

	struct EngineShadersSourceDir : public EngineAssetPath
	{
		EngineShadersSourceDir();
	};

	struct EngineShadersBinaryDir : public EngineAssetPath
	{
		EngineShadersBinaryDir();
	};

	struct EngineShaderPath : public EngineAssetPath
	{
		EngineShaderPath() = default;
		EngineShaderPath(std::string_view relativeAssetPath);
	};
} // namespace lune

namespace lune // app paths
{
	extern "C++" std::filesystem::path& getAppAssetsRootDir();
	struct AppAssetPath : EngineAssetPath
	{
		AppAssetPath(std::string_view relativeAssetPath);
	};

	struct AppShadersSourceDir : public EngineAssetPath
	{
		AppShadersSourceDir();
	};

	struct AppShadersBinaryDir : public EngineAssetPath
	{
		AppShadersBinaryDir();
	};

	struct AppAssetShaderPath : EngineAssetPath
	{
		AppAssetShaderPath(std::string_view relativeAssetPath);
	};
} // namespace lune