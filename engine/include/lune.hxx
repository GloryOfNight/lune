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
	static uint32 GetEngineVersion()
	{
        uint32_t major{0}, minor{1}, patch{0};
		return (((uint32_t)(major)) << 22U) | (((uint32_t)(minor)) << 12U) | ((uint32_t)(patch));
	}

	static const char* GetEngineName()
	{
		return "Lune";
	}
} // namespace lune
namespace ln = lune;