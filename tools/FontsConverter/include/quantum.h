/*------------------------------------------------------------------------------
 * Copyright (c) 2017-2019
 *     Michael Theall (mtheall)
 *
 * This file is part of tex3ds.
 *
 * tex3ds is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * tex3ds is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with tex3ds.  If not, see <http://www.gnu.org/licenses/>.
 *----------------------------------------------------------------------------*/
/** @file quantum.h
 *  @brief SDL alpha conversion helpers and gamma math
 */
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace
{
inline uint8_t clamp_u8 (int value)
{
	return static_cast<uint8_t> (std::max (0, std::min (255, value)));
}

inline uint8_t alpha_8_to_4 (uint8_t value)
{
	return static_cast<uint8_t> ((value * 15u + 127u) / 255u);
}

inline uint8_t alpha_4_to_8 (uint8_t value)
{
	return static_cast<uint8_t> ((value * 255u + 7u) / 15u);
}

inline double gamma_inverse (double v)
{
	if (v <= 0.04045)
		return v / 12.92;
	return std::pow ((v + 0.055) / 1.055, 2.4);
}

inline double gamma (double v)
{
	if (v <= 0.0031308)
		return v * 12.92;
	return 1.055 * std::pow (v, 1.0 / 2.4) - 0.055;
}

inline uint8_t luminance_from_rgb (uint8_t r, uint8_t g, uint8_t b)
{
	const double red   = 0.212655;
	const double green = 0.715158;
	const double blue  = 0.072187;

	double v = gamma (red * gamma_inverse (static_cast<double> (r) / 255.0) +
	                  green * gamma_inverse (static_cast<double> (g) / 255.0) +
	                  blue * gamma_inverse (static_cast<double> (b) / 255.0));

	return clamp_u8 (static_cast<int> (std::round (std::max (0.0, std::min (1.0, v)) * 255.0)));
}
}
