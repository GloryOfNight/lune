#pragma once

#include <stdint.h>
#include <uchar.h>

using int8 = int8_t;
using uint8 = uint8_t;
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
	static uint32 getEngineVersion()
	{
		uint32_t major{0}, minor{0}, patch{1};
		return (((uint32_t)(major)) << 22U) | (((uint32_t)(minor)) << 12U) | ((uint32_t)(patch));
	}

	static uint32 getEngineVersionMajor()
	{
		return (getEngineVersion() >> 22U) & 0x7FU;
	}

	static uint32 getEngineVersionMinor()
	{
		return (getEngineVersion() >> 12U) & 0x3FFU;
	}

	static uint32 getEngineVersionPatch()
	{
		return getEngineVersion() & 0xFFFU;
	}

	static const char* getEngineName()
	{
		return "Lune";
	}
} // namespace lune
namespace ln = lune;