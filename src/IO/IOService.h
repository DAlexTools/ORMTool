#pragma once 

#include <string>

class IOService
{
public:
	static bool SavePNG(const std::string& filename, unsigned int textureId, int width, int height);
	static bool SaveTGA(const std::string& filename, unsigned int textureId, int width, int height);
	static bool SaveBMP(const std::string& filename, unsigned int textureId, int width, int height);
	static bool SaveJPG(const std::string& filename, unsigned int textureId, int width, int height, int quality = 90);

};