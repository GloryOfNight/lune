#pragma once

#include <stdint.h>
#include <string>
#include <uchar.h>

using int8 = int8_t;
using uint8 = uint8_t;
using int16 = int16_t;
using uint16 = uint16_t;
using int32 = int32_t;
using uint32 = uint32_t;
using int64 = int64_t;
using uint64 = uint64_t;

using float32 = float;
using float64 = double;
using float128 = long double;

using char8 = char8_t;
using char16 = char16_t;
using char32 = char32_t;

namespace lune
{
	static constexpr uint32 makeVersion(uint32 major, uint32 minor, uint32 patch) noexcept
	{
		return (((uint32_t)(major)) << 22U) | (((uint32_t)(minor)) << 12U) | ((uint32_t)(patch));
	}

	static void getVersion(uint32 version, uint32& major, uint32& minor, uint32& patch) noexcept
	{
		major = (version >> 22U) & 0x7FU;
		minor = (version >> 12U) & 0x3FFU;
		patch = version & 0xFFFU;
	}

	extern "C++" uint32& getApplicationVersion() noexcept;

	extern "C++" std::string& getApplicationName() noexcept;

	static uint32 getEngineVersion() noexcept
	{
		static constexpr uint32 version = makeVersion(0, 0, 1);
		return version;
	}

	static const char* getEngineName() noexcept
	{
		return "Lune";
	}
} // namespace lune
namespace ln = lune;