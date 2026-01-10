#pragma once

#include <string>
#include <GLFW/glfw3.h>

/** enum channels  */
enum class ORMChannel : int
{
	AllRGB,
	AO_R,
	Roughness_G,
	Metallic_B
};

/** preview texture data */
struct PreviewTexture
{
	unsigned char* data = nullptr;
	std::string path;
	int width = 0;
	int height = 0;			//
	GLuint glId = 0;		// 4 byte
	GLuint channelR = 0; 	// 4 byte
	GLuint channelG = 0; 	// 4 byte
	GLuint channelB = 0; 	// 4 byte

	/** loading texture */
	bool Load(const std::string& p);

	/** unloading texture */
	void Unload();

	/** generate texture */
	void GenerateChannelsFromRGB(unsigned char* src, int w, int h);
};

struct SaveData
{
	std::string ao;
	std::string rough;
	std::string metal;
	std::string saveUnrealPath;
	std::string saveUnityPath;

	std::function<void(float*)> progressCallback;

};