#pragma once 

#include <string_view>
namespace ORM
{
	static constexpr const char* TitleStr = "ORMTool";
	static constexpr const int WindowWidth = 706;
	static constexpr const int WindowHeight = 677;

	namespace GraphicsConfig
	{
		constexpr int OPENGL_MAJOR = 4;
		constexpr int OPENGL_MINOR = 6;
		constexpr int MSAA_SAMPLES = 4;
		constexpr bool ENABLE_DEBUG = true;
	}
}

