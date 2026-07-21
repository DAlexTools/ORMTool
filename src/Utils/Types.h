#pragma once

#include <string>
#include <functional>
#include <GLFW/glfw3.h>

/**
 * @brief Available texture channel preview modes for ORM textures.
 */
enum class ORMChannel : int
{
	AllRGB,      ///< Displays the combined RGB texture.
	AO_R,        ///< Displays the ambient occlusion channel stored in red.
	Roughness_G, ///< Displays the roughness channel stored in green.
	Metallic_B   ///< Displays the metallic channel stored in blue.
};

/**
 * @brief CPU and OpenGL resources for an image preview.
 */
struct PreviewTexture
{
	unsigned char* data = nullptr; ///< Raw loaded image bytes, owned by stb_image until Unload().
	std::string path;              ///< Original file path used to load the texture.
	int width = 0;                 ///< Texture width in pixels.
	int height = 0;                ///< Texture height in pixels.
	GLuint glId = 0;               ///< OpenGL texture id for the full RGB preview.
	GLuint channelR = 0;           ///< OpenGL texture id for the red channel preview.
	GLuint channelG = 0;           ///< OpenGL texture id for the green channel preview.
	GLuint channelB = 0;           ///< OpenGL texture id for the blue channel preview.

	/**
	 * @brief Loads an image file and uploads it as an OpenGL texture.
	 * @param p Source image path.
	 * @return True when the image was loaded and uploaded successfully.
	 */
	bool Load(const std::string& p);

	/**
	 * @brief Releases CPU image data and OpenGL textures owned by this preview.
	 */
	void Unload();

	/**
	 * @brief Creates single-channel preview textures from RGB source data.
	 * @param src Source RGB pixel buffer.
	 * @param w Source image width in pixels.
	 * @param h Source image height in pixels.
	 */
	void GenerateChannelsFromRGB(unsigned char* src, int w, int h);
};

/**
 * @brief Paths and progress callback used when saving generated ORM textures.
 */
struct SaveData
{
	std::string ao;             ///< Ambient occlusion input path.
	std::string rough;          ///< Roughness input path.
	std::string metal;          ///< Metallic input path.
	std::string saveUnrealPath; ///< Output path for Unreal Engine ORM layout.
	std::string saveUnityPath;  ///< Output path for Unity ORM layout.

	std::function<void(float*)> progressCallback; ///< Optional progress callback.
};
