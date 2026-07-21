#include "IOService.h"

#ifdef _WIN32
#	ifndef WIN32_LEAN_AND_MEAN
#		define WIN32_LEAN_AND_MEAN
#	endif
#	ifndef NOMINMAX
#		define NOMINMAX
#	endif
#	include <windows.h>
#endif
#include <GLFW/glfw3.h>
#undef STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

namespace
{
	struct TexturePixels
	{
		std::vector<unsigned char> data;
		int width = 0;
		int height = 0;
		int channels = 0;
	};

#ifdef _WIN32
	std::wstring ToWidePath(const std::string& path)
	{
		if (path.empty())
		{
			return {};
		}

		int size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path.c_str(), -1, nullptr, 0);
		UINT codePage = CP_UTF8;
		DWORD flags = MB_ERR_INVALID_CHARS;
		if (size <= 0)
		{
			codePage = CP_ACP;
			flags = 0;
			size = MultiByteToWideChar(codePage, flags, path.c_str(), -1, nullptr, 0);
		}
		if (size <= 0)
		{
			return {};
		}

		std::wstring widePath(static_cast<size_t>(size), L'\0');
		MultiByteToWideChar(codePage, flags, path.c_str(), -1, widePath.data(), size);
		if (!widePath.empty() && widePath.back() == L'\0')
		{
			widePath.pop_back();
		}

		return widePath;
	}

	FILE* OpenFile(const std::string& path, const wchar_t* wideMode, const char* mode)
	{
		FILE* file = nullptr;
		const std::wstring widePath = ToWidePath(path);
		if (!widePath.empty())
		{
			file = _wfopen(widePath.c_str(), wideMode);
		}
		if (!file)
		{
			file = fopen(path.c_str(), mode);
		}
		return file;
	}
#else
	FILE* OpenFile(const std::string& path, const wchar_t*, const char* mode)
	{
		return fopen(path.c_str(), mode);
	}
#endif

	void AppendBytes(void* context, void* data, int size)
	{
		auto* bytes = static_cast<std::vector<unsigned char>*>(context);
		const auto* source = static_cast<const unsigned char*>(data);
		bytes->insert(bytes->end(), source, source + size);
	}

	bool WriteBytes(const std::string& filename, const std::vector<unsigned char>& bytes)
	{
		FILE* file = OpenFile(filename, L"wb", "wb");
		if (!file)
		{
			std::cerr << "Failed to open output file: " << filename << "\n";
			return false;
		}

		const size_t written = fwrite(bytes.data(), 1, bytes.size(), file);
		fclose(file);
		return written == bytes.size();
	}

	int GetTextureChannelCount()
	{
		GLint redBits = 0;
		GLint greenBits = 0;
		GLint blueBits = 0;
		GLint alphaBits = 0;
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_RED_SIZE, &redBits);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_GREEN_SIZE, &greenBits);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_BLUE_SIZE, &blueBits);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_ALPHA_SIZE, &alphaBits);

		if (alphaBits > 0)
		{
			return 4;
		}
		if (blueBits > 0 || greenBits > 0)
		{
			return 3;
		}
		if (redBits > 0)
		{
			return 1;
		}

		return 3;
	}

	TexturePixels ReadTexture(unsigned int textureId, int width, int height)
	{
		TexturePixels texture;
		if (textureId == 0)
		{
			return texture;
		}

		GLint previousTexture = 0;
		GLint previousPackAlignment = 4;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &previousTexture);
		glGetIntegerv(GL_PACK_ALIGNMENT, &previousPackAlignment);

		glBindTexture(GL_TEXTURE_2D, textureId);
		GLint actualWidth = width;
		GLint actualHeight = height;
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &actualWidth);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &actualHeight);
		if (actualWidth <= 0 || actualHeight <= 0)
		{
			glBindTexture(GL_TEXTURE_2D, previousTexture);
			return texture;
		}

		texture.width = actualWidth;
		texture.height = actualHeight;
		texture.channels = GetTextureChannelCount();
		const GLenum format = texture.channels == 4 ? GL_RGBA : texture.channels == 3 ? GL_RGB : GL_RED;

		texture.data.resize(static_cast<size_t>(texture.width) * texture.height * texture.channels);
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glGetTexImage(GL_TEXTURE_2D, 0, format, GL_UNSIGNED_BYTE, texture.data.data());
		const GLenum error = glGetError();

		glPixelStorei(GL_PACK_ALIGNMENT, previousPackAlignment);
		glBindTexture(GL_TEXTURE_2D, previousTexture);

		if (error != GL_NO_ERROR)
		{
			texture.data.clear();
			texture.width = 0;
			texture.height = 0;
			texture.channels = 0;
		}

		return texture;
	}
} // namespace

bool IOService::SavePNG(const std::string& filename, unsigned int textureId, int width, int height)
{
	TexturePixels texture = ReadTexture(textureId, width, height);
	if (texture.data.empty())
	{
		return false;
	}

	std::vector<unsigned char> bytes;
	if (stbi_write_png_to_func(AppendBytes, &bytes, texture.width, texture.height, texture.channels,
							   texture.data.data(), texture.width * texture.channels) == 0)
	{
		return false;
	}

	return WriteBytes(filename, bytes);
}

bool IOService::SaveTGA(const std::string& filename, unsigned int textureId, int width, int height)
{
	TexturePixels texture = ReadTexture(textureId, width, height);
	if (texture.data.empty())
	{
		return false;
	}

	std::vector<unsigned char> bytes;
	if (stbi_write_tga_to_func(AppendBytes, &bytes, texture.width, texture.height, texture.channels,
							   texture.data.data()) == 0)
	{
		return false;
	}

	return WriteBytes(filename, bytes);
}

bool IOService::SaveBMP(const std::string& filename, unsigned int textureId, int width, int height)
{
	TexturePixels texture = ReadTexture(textureId, width, height);
	if (texture.data.empty())
	{
		return false;
	}

	std::vector<unsigned char> bytes;
	if (stbi_write_bmp_to_func(AppendBytes, &bytes, texture.width, texture.height, texture.channels,
							   texture.data.data()) == 0)
	{
		return false;
	}

	return WriteBytes(filename, bytes);
}

bool IOService::SaveJPG(const std::string& filename, unsigned int textureId, int width, int height, int quality)
{
	TexturePixels texture = ReadTexture(textureId, width, height);
	if (texture.data.empty())
	{
		return false;
	}

	std::vector<unsigned char> bytes;
	if (stbi_write_jpg_to_func(AppendBytes, &bytes, texture.width, texture.height, texture.channels,
							   texture.data.data(), quality) == 0)
	{
		return false;
	}

	return WriteBytes(filename, bytes);
}
