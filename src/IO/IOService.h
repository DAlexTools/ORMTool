#pragma once

#include <string>

/**
 * @brief Exports OpenGL textures to image files.
 */
class IOService
{
public:
	/**
	 * @brief Saves an OpenGL texture to a PNG file.
	 * @param filename Destination file path.
	 * @param textureId OpenGL texture id to read from.
	 * @param width Expected texture width in pixels.
	 * @param height Expected texture height in pixels.
	 * @return True when the file was written successfully.
	 */
	static bool SavePNG(const std::string& filename, unsigned int textureId, int width, int height);

	/**
	 * @brief Saves an OpenGL texture to a TGA file.
	 * @param filename Destination file path.
	 * @param textureId OpenGL texture id to read from.
	 * @param width Expected texture width in pixels.
	 * @param height Expected texture height in pixels.
	 * @return True when the file was written successfully.
	 */
	static bool SaveTGA(const std::string& filename, unsigned int textureId, int width, int height);

	/**
	 * @brief Saves an OpenGL texture to a BMP file.
	 * @param filename Destination file path.
	 * @param textureId OpenGL texture id to read from.
	 * @param width Expected texture width in pixels.
	 * @param height Expected texture height in pixels.
	 * @return True when the file was written successfully.
	 */
	static bool SaveBMP(const std::string& filename, unsigned int textureId, int width, int height);

	/**
	 * @brief Saves an OpenGL texture to a JPEG file.
	 * @param filename Destination file path.
	 * @param textureId OpenGL texture id to read from.
	 * @param width Expected texture width in pixels.
	 * @param height Expected texture height in pixels.
	 * @param quality JPEG quality in the range accepted by stb_image_write.
	 * @return True when the file was written successfully.
	 */
	static bool SaveJPG(const std::string& filename, unsigned int textureId, int width, int height, int quality = 90);
};
