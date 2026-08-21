#pragma once

#include "maths/vector3.h"
#define TRIDINGINE_VERSION_MAJOR 0
#define TRIDINGINE_VERSION_MINOR 1
#define TRIDINGINE_VERSION_MICRO 0

#include "screens.h"
#include "maths.h"
#include "localization.h"
#include "input.h"
#include "delta_time.h"
#include "color.h"
#include "animation/animation.h"
#include "console/console.h"
#include "draw/2d/2d_shapes.h"
#include "draw/3d/3d_shapes.h"
#include "pak_loader/pak_loader.h"
#include "sound/sound.h"
#include "textures/textures.h"
#include "utils/random.h"
#include "utils/save_system.h"
#include "sys/system_fonts.h"
#include "sys/system_language.h"
#include "sys/system_time.h"
#include "sys/system_memory.h"
// #include "html/html-ds.h"

Vec3 getVersion();