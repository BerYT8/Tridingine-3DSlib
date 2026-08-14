# Third-Party Notices

Tridingine-3DSlib is primarily licensed under the Apache License 2.0.
The Apache License 2.0 applies only to the original Tridingine-3DSlib
code for which the project has the necessary copyright rights.

This repository also contains third-party software and code. Those
components remain under their respective licenses. The Apache-2.0 license
of the main project does not relicense third-party components.

The corresponding license texts are provided in the `LICENSES/` directory
where appropriate. Original license and copyright notices contained in
third-party source files must also be preserved.

---

## 1. devkitPro tex3ds / mkbcfnt

**Component:** `FontsConverter`

**License:** GNU General Public License v3.0 or later (GPL-3.0-or-later)

**Upstream:** https://github.com/devkitPro/tex3ds

`tools/FontsConverter/` contains code derived from the `tex3ds` project
developed by devkitPro, including code used by `mkbcfnt` for BCFNT font
generation.

The relevant source files contain their original copyright and GPL
license notices.

The original `COPYING` file from `mkbcfnt`/`tex3ds` is retained in
`tools/FontsConverter/`.

The GPL-3.0-or-later license text is also available at:

`LICENSES/GPL-3.0-or-later.txt`

In particular, the GPL-covered code in `tools/FontsConverter/` is NOT
licensed under Apache-2.0. Modifications and redistribution of that
code must comply with the GPL-3.0-or-later terms.

---

## 2. SDL

### SDL

**Location:** `external/sdl2`

**License:** zlib

**Upstream:** https://github.com/libsdl-org/SDL

SDL is distributed under the zlib license.

---

### SDL_image

**Location:** `external/sdl2-image`

**License:** zlib

**Upstream:** https://github.com/libsdl-org/SDL_image

SDL_image is distributed under the zlib license.

---

### SDL_mixer

**Location:** `external/sdl2-mixer`

**License:** zlib

**Upstream:** https://github.com/libsdl-org/SDL_mixer

SDL_mixer is distributed under the zlib license.

---

### SDL_ttf

**Location:** `external/sdl2-ttf`

**License:** zlib

**Upstream:** https://github.com/libsdl-org/SDL_ttf

SDL_ttf is distributed under the zlib license.

SDL_ttf may additionally use third-party libraries such as FreeType,
HarfBuzz, PlutoSVG and PlutoVG. Their respective licenses apply to those
components when they are redistributed.

---

## 3. nlohmann/json

**Locations:**

- `src/nlohmann/json.hpp`
- `tools/LocalizationMaker/json.hpp`

**License:** MIT

**Upstream:** https://github.com/nlohmann/json

JSON for Modern C++ is licensed under the MIT License.

The original copyright and license notices contained in `json.hpp` must
be preserved.

---

## 4. Opus

**Locations include:**

- `src/sound/opus.h`
- `src/sound/opus_defines.h`
- `src/sound/opus_multistream.h`
- `src/sound/opus_projection.h`
- `src/sound/opus_types.h`
- `tools/SoundMaker3DS/include/`

**License:** BSD-style / BSD-3-Clause

**Upstream:** https://github.com/xiph/opus

The copied Opus headers retain their upstream copyright and license
notices. Those notices must not be removed from the source files.

---

## 5. libogg / Ogg

**Locations include:**

- `src/sound/ogg/`
- `tools/SoundMaker3DS/include/ogg/`
- `tools/SoundMaker3DS/src/bitwise.c`
- `tools/SoundMaker3DS/src/framing.c`

**License:** BSD-style

**Upstream:** https://github.com/xiph/ogg

The original copyright and license notices contained in the source files
must be preserved.

---

## 6. dr_libs

### dr_flac

**Location:** `tools/SoundMaker3DS/include/dr_flac.h`

### dr_mp3

**Location:** `tools/SoundMaker3DS/include/dr_mp3.h`

### dr_wav

**Location:** `tools/SoundMaker3DS/include/dr_wav.h`

**License:** Public Domain or MIT-0, according to the respective upstream
source-file notices.

**Upstream:** https://github.com/mackron/dr_libs

The original license/public-domain notices contained in each source file
must be preserved.

---

## 7. stb_vorbis

**Location:** `tools/SoundMaker3DS/stb_vorbis.c`

**License:** Public Domain / MIT

**Upstream:** https://github.com/nothings/stb

The applicable license notice is contained in the source file itself and
must be preserved.

---

## 8. libopusenc

**Location:** `tools/SoundMaker3DS/libopusenc`

**License:** BSD-3-Clause

**Upstream:** https://github.com/xiph/libopusenc

The upstream `COPYING` file should be retained when the submodule is
distributed as source.

---

## 9. Project_CTR

**Location:** `tools/Project_CTR`

**Upstream:** https://github.com/3DSGuy/Project_CTR

This submodule contains tools including `ctrtool` and `makerom`.

The exact license and copyright notices must be taken from the exact
commit of Project_CTR used by the project. The license of individual
components must not be assumed from the repository as a whole.

When distributing the submodule, retain its original license and notice
files.

---

## 10. 3dstools

**Location:** `tools/3dstool`

**Upstream:** https://github.com/devkitPro/3dstools

The exact license and copyright notices must be taken from the exact
commit of 3dstools used by the project.

When distributing the submodule, retain its original license and notice
files.

---

## 11. bannertool

**Location:** `tools/bannertool`

**Upstream:** https://github.com/diasurgical/bannertool

The exact license and copyright notices must be taken from the exact
commit of bannertool used by the project.

Do not assume a license merely because the project is hosted on GitHub.
When distributing the submodule, retain its original license and notice
files.

---

## 12. FreeType

**Usage:** external dependency of `FontsConverter`

**License:** FreeType License (FTL), with GPLv2 also available as an
alternative licensing option from the upstream project.

**Upstream:** https://freetype.org/license.html

If FreeType itself is redistributed with a release, its applicable license
and copyright notices must also be redistributed.

---

## 13. ImageMagick

**Usage:** external dependency of `FontsConverter`

**License:** ImageMagick License

**Upstream:** https://imagemagick.org/license/

If ImageMagick itself is redistributed with a release, its applicable
license and copyright notices must also be redistributed.

---

## 14. GLEW

**Usage:** external dependency for desktop/PC builds

**Upstream:** https://github.com/nigels-com/glew

GLEW contains components distributed under multiple permissive licenses.
If GLEW itself is redistributed, the license and notice files from the
exact version being distributed must be retained.

---

## 15. Fonts and other proprietary assets

### Arial

**Location:** `content/engine/fonts/arial.ttf`

This font is NOT covered by the Apache-2.0 license of Tridingine-3DSlib.

The font contains Microsoft/Monotype copyright and licensing metadata.

Unless the project has a separate license granting redistribution rights,
this font must not be redistributed as part of Tridingine-3DSlib.

---

### PopHappinessStd-EB

**Location:** `content/engine/fonts/PopHappinessStd-EB.ttf`

This font is NOT covered by the Apache-2.0 license of Tridingine-3DSlib.

The font contains Fontworks copyright and licensing metadata.

Unless the project has a separate license granting redistribution rights,
this font must not be redistributed as part of Tridingine-3DSlib.

---

## 16. License compliance

When redistributing Tridingine-3DSlib or a product containing it:

1. Keep the original copyright notices of third-party components.
2. Keep the original license files and notices required by those
   components.
3. Do not claim that third-party code is Apache-2.0 merely because the
   main project is Apache-2.0.
4. Comply with GPL-3.0-or-later for the GPL-covered code in
   `tools/FontsConverter/`.
5. Keep the `COPYING` file supplied with the tex3ds/mkbcfnt-derived code.
6. Verify the exact commit and license of every submodule before
   distributing its source or binaries.
7. Do not redistribute proprietary fonts or other assets without the
   necessary permission.

This document is an inventory of third-party components and is not legal
advice.