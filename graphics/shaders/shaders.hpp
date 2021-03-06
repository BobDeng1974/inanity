#ifndef ___INANITY_GRAPHICS_SHADERS_HPP___
#define ___INANITY_GRAPHICS_SHADERS_HPP___

/*
 * Общий файл для подсистемы шейдеров.
 * Подсистема шейдеров предназначена для отделения графической системы от
 * конкретных языков шейдеров, таких как HLSL или GLSL.
 */

#include "../graphics.hpp"

#define BEGIN_INANITY_SHADERS BEGIN_INANITY_GRAPHICS namespace Shaders {
#define END_INANITY_SHADERS } END_INANITY_GRAPHICS

BEGIN_INANITY_SHADERS
using namespace Inanity::Math;
END_INANITY_SHADERS

#endif
