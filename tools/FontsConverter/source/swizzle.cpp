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
/** @file swizzle.cpp
 *  @brief Swizzle routines for SDL alpha surfaces
 */

#include "swizzle.h"

#include <array>
#include <cstdint>
#include <cstring>
#include <utility>

namespace
{
using Tile = std::array<std::uint8_t, 64>;

static const unsigned char table[][4] = {
    /* clang-format off */
    {  2,  8, 16,  4, },
    {  3,  9, 17,  5, },
    {  6, 10, 24, 20, },
    {  7, 11, 25, 21, },
    { 14, 26, 28, 22, },
    { 15, 27, 29, 23, },
    { 34, 40, 48, 36, },
    { 35, 41, 49, 37, },
    { 38, 42, 56, 52, },
    { 39, 43, 57, 53, },
    { 46, 58, 60, 54, },
    { 47, 59, 61, 55, },
    /* clang-format on */
};

Uint8 getAlpha (SDL_Surface *surface, int x, int y)
{
	if (!surface || x < 0 || y < 0 || x >= surface->w || y >= surface->h)
		return 0;

	const auto bytesPerPixel = surface->format->BytesPerPixel;
	auto *row = static_cast<Uint8 *> (surface->pixels) + static_cast<size_t> (y) * surface->pitch;
	auto *pixel = row + static_cast<size_t> (x) * bytesPerPixel;

	Uint8 r, g, b, a;
	const Uint32 value = *(reinterpret_cast<Uint32 *> (pixel));
	SDL_GetRGBA (value, surface->format, &r, &g, &b, &a);
	return a;
}

void setAlpha (SDL_Surface *surface, int x, int y, Uint8 alpha)
{
	if (!surface || x < 0 || y < 0 || x >= surface->w || y >= surface->h)
		return;

	const Uint32 color = SDL_MapRGBA (surface->format, 0, 0, 0, alpha);
	auto *row = static_cast<Uint8 *> (surface->pixels) + static_cast<size_t> (y) * surface->pitch;
	auto *pixel = row + static_cast<size_t> (x) * surface->format->BytesPerPixel;
	std::memcpy (pixel, &color, surface->format->BytesPerPixel);
}

void swizzleTile (Tile &tile, bool reverse)
{
	if (!reverse)
	{
		for (const auto &entry : table)
		{
			std::uint8_t tmp = tile[entry[0]];
			tile[entry[0]] = tile[entry[1]];
			tile[entry[1]] = tile[entry[2]];
			tile[entry[2]] = tile[entry[3]];
			tile[entry[3]] = tmp;
		}
	}
	else
	{
		for (const auto &entry : table)
		{
			std::uint8_t tmp = tile[entry[3]];
			tile[entry[3]] = tile[entry[2]];
			tile[entry[2]] = tile[entry[1]];
			tile[entry[1]] = tile[entry[0]];
			tile[entry[0]] = tmp;
		}
	}

	std::swap (tile[12], tile[18]);
	std::swap (tile[13], tile[19]);
	std::swap (tile[44], tile[50]);
	std::swap (tile[45], tile[51]);
}
}

void swizzle (SDL_Surface *surface, bool reverse)
{
	if (!surface)
		return;

	for (int j = 0; j < surface->h; j += 8)
	{
		for (int i = 0; i < surface->w; i += 8)
		{
			Tile tile{};
			for (int y = 0; y < 8; ++y)
			{
				for (int x = 0; x < 8; ++x)
				{
					const int sx = i + x;
					const int sy = j + y;
					if (sx < surface->w && sy < surface->h)
						tile[y * 8 + x] = getAlpha (surface, sx, sy);
				}
			}

			swizzleTile (tile, reverse);

			for (int y = 0; y < 8; ++y)
			{
				for (int x = 0; x < 8; ++x)
				{
					const int sx = i + x;
					const int sy = j + y;
					if (sx < surface->w && sy < surface->h)
						setAlpha (surface, sx, sy, tile[y * 8 + x]);
				}
			}
		}
	}
}
