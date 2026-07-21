#pragma once 

#include <string_view>

/**
 * @brief Application-wide constants for the ORM tool.
 */
namespace ORM
{
	static constexpr const char* TitleStr = "ORMTool"; ///< Window title.
	static constexpr const int WindowWidth = 706;      ///< Fixed window width in pixels.
	static constexpr const int WindowHeight = 677;     ///< Fixed window height in pixels.

	/**
	 * @brief OpenGL context and rendering options.
	 */
	namespace GraphicsConfig
	{
		constexpr int OPENGL_MAJOR = 4;     ///< Requested OpenGL major version.
		constexpr int OPENGL_MINOR = 6;     ///< Requested OpenGL minor version.
		constexpr int MSAA_SAMPLES = 4;     ///< Multisample anti-aliasing sample count.
		constexpr bool ENABLE_DEBUG = true; ///< Enables OpenGL debug context in debug builds.
	}
}

