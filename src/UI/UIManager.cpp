#ifdef _WIN32
#	ifndef WIN32_LEAN_AND_MEAN
#		define WIN32_LEAN_AND_MEAN
#	endif
#	ifndef NOMINMAX
#		define NOMINMAX
#	endif
#	include <windows.h>
#endif
#include "UIManager.h"
#include <imgui.h>
#include <imgui_internal.h>

#include "IOService.h"
#include <nfd.h>

#include <stb_image.h>
#include <stb_image_write.h>
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cstdio>
#include <iostream>
#include <limits>
#include <memory>
#include <stb_image_resize2.h>
#include <vector>

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <future>

namespace ORMTool
{
	constexpr const char* TitleProgram = "ORMTool";
	constexpr const char* UnrealCBoxTitle = "Unreal";
	constexpr const char* UnityCBoxTitle = "Unity ";
	constexpr const char* SavedTextureFormat = "png,jpg";
	constexpr const char* SaveFormatOptions = "PNG\0JPG\0BMP\0TGA\0";
	constexpr const char* SaveFormatExtensions[] = {"png", "jpg", "bmp", "tga"};
	constexpr const char* SaveFormatFilters[] = {"png", "jpg,jpeg", "bmp", "tga"};
	constexpr const float CheckboxSize = 14.0f;
	constexpr const char* ProcessingModeOptions = "3 textures -> ORM\0ORM -> 3 textures\0";
	constexpr const char* SplitLayoutOptions = "Unreal ORM\0Unity Mask\0";
	constexpr const auto& WindowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar;

	constexpr ImVec2 GenerateButtonSize = ImVec2(118, 28);
	constexpr ImVec2 LoadButtonSize = ImVec2(100, 26);
	constexpr ImVec2 ProgressBarWidgetSize = ImVec2(300, 24);
	constexpr ImVec2 TextureSlotSize = ImVec2(0, 164);
	constexpr ImVec2 TextureThumbSize = ImVec2(132, 88);

	const ImVec4 AccentBlue = ImVec4(0.24f, 0.54f, 0.88f, 1.0f);
	const ImVec4 AccentGreen = ImVec4(0.28f, 0.72f, 0.48f, 1.0f);
	const ImVec4 AccentAmber = ImVec4(0.90f, 0.64f, 0.26f, 1.0f);
	const ImVec4 AccentRed = ImVec4(0.88f, 0.28f, 0.30f, 1.0f);
	const ImVec4 TextMuted = ImVec4(0.62f, 0.66f, 0.70f, 1.0f);
	const ImVec4 PanelBg = ImVec4(0.105f, 0.112f, 0.122f, 1.0f);
	const ImVec4 PanelAltBg = ImVec4(0.075f, 0.080f, 0.090f, 1.0f);
	const ImVec4 BorderColor = ImVec4(0.22f, 0.24f, 0.27f, 1.0f);

	using StbiImagePtr = std::unique_ptr<unsigned char, void (*)(void*)>;

	struct ImageBuffer
	{
		StbiImagePtr pixels{nullptr, stbi_image_free};
		int width = 0;
		int height = 0;

		bool IsValid() const
		{
			return pixels != nullptr && width > 0 && height > 0;
		}
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

	bool ReadBinaryFile(const std::string& path, std::vector<unsigned char>& bytes)
	{
		FILE* file = OpenFile(path, L"rb", "rb");
		if (!file)
		{
			std::cerr << "Failed to open image file: " << path << "\n";
			return false;
		}

		if (fseek(file, 0, SEEK_END) != 0)
		{
			fclose(file);
			return false;
		}

		const long fileSize = ftell(file);
		if (fileSize <= 0 || fileSize > std::numeric_limits<int>::max())
		{
			fclose(file);
			return false;
		}

		rewind(file);
		bytes.resize(static_cast<size_t>(fileSize));
		const size_t bytesRead = fread(bytes.data(), 1, bytes.size(), file);
		fclose(file);

		if (bytesRead != bytes.size())
		{
			bytes.clear();
			return false;
		}

		return true;
	}

	bool WriteBinaryFile(const std::string& path, const std::vector<unsigned char>& bytes)
	{
		FILE* file = OpenFile(path, L"wb", "wb");
		if (!file)
		{
			std::cerr << "Failed to open output file: " << path << "\n";
			return false;
		}

		const size_t bytesWritten = fwrite(bytes.data(), 1, bytes.size(), file);
		fclose(file);
		return bytesWritten == bytes.size();
	}

	void AppendPNGBytes(void* context, void* data, int size)
	{
		auto* bytes = static_cast<std::vector<unsigned char>*>(context);
		const auto* source = static_cast<unsigned char*>(data);
		bytes->insert(bytes->end(), source, source + size);
	}

	bool WritePNG(const std::string& path, int width, int height, int channels, const unsigned char* data, int stride)
	{
		std::vector<unsigned char> bytes;
		if (stbi_write_png_to_func(AppendPNGBytes, &bytes, width, height, channels, data, stride) == 0)
		{
			return false;
		}
		return WriteBinaryFile(path, bytes);
	}

	ImageBuffer LoadImage(const std::string& path, int requestedChannels)
	{
		ImageBuffer image;
		int channels = 0;
		std::vector<unsigned char> bytes;
		if (ReadBinaryFile(path, bytes))
		{
			image.pixels.reset(stbi_load_from_memory(bytes.data(), static_cast<int>(bytes.size()), &image.width, &image.height, &channels, requestedChannels));
		}
		if (!image.pixels)
		{
			std::cerr << "Failed to decode image: " << path << "\n";
		}
		return image;
	}

	std::vector<unsigned char> ResizeGrayscale(const unsigned char* source, int sourceWidth, int sourceHeight, int targetResolution)
	{
		if (!source || sourceWidth <= 0 || sourceHeight <= 0 || targetResolution <= 0)
		{
			return {};
		}

		std::vector<unsigned char> result(static_cast<size_t>(targetResolution) * targetResolution);
		if (sourceWidth == targetResolution && sourceHeight == targetResolution)
		{
			std::copy(source, source + result.size(), result.begin());
			return result;
		}

		unsigned char* resized = stbir_resize_uint8_linear(source, sourceWidth, sourceHeight, 0, result.data(), targetResolution, targetResolution, 0, STBIR_1CHANNEL);

		if (!resized)
		{
			std::cerr << "Failed to resize grayscale image to " << targetResolution << "x" << targetResolution << "\n";
			return {};
		}

		return result;
	}

	std::vector<unsigned char> ResizeGrayscale(const std::vector<unsigned char>& source, int sourceResolution, int targetResolution)
	{
		return ResizeGrayscale(source.data(), sourceResolution, sourceResolution, targetResolution);
	}

	bool WriteGrayscalePNG(const std::string& path, const std::vector<unsigned char>& data, int resolution)
	{
		return WritePNG(path, resolution, resolution, 1, data.data(), resolution);
	}

	std::string MakeSiblingOutputPath(const std::string& sourcePath, const char* suffix)
	{
		const size_t slash = sourcePath.find_last_of("/\\");
		const std::string directory = slash == std::string::npos ? std::string() : sourcePath.substr(0, slash + 1);
		const std::string fileName = slash == std::string::npos ? sourcePath : sourcePath.substr(slash + 1);
		const size_t dot = fileName.find_last_of('.');
		const std::string stem = dot == std::string::npos ? fileName : fileName.substr(0, dot);
		return directory + stem + suffix;
	}

	std::string ReplaceExtension(const std::string& path, const char* extension)
	{
		const size_t slash = path.find_last_of("/\\");
		const size_t dot = path.find_last_of('.');
		const bool hasExtension = dot != std::string::npos && (slash == std::string::npos || dot > slash);
		const std::string prefix = hasExtension ? path.substr(0, dot) : path;
		return prefix + "." + extension;
	}

	ImU32 U32(const ImVec4& color)
	{
		return ImGui::ColorConvertFloat4ToU32(color);
	}

	void ApplyTheme()
	{
		ImGuiStyle& style = ImGui::GetStyle();
		style.WindowPadding = ImVec2(10.0f, 10.0f);
		style.FramePadding = ImVec2(8.0f, 5.0f);
		style.ItemSpacing = ImVec2(8.0f, 8.0f);
		style.ItemInnerSpacing = ImVec2(6.0f, 5.0f);
		style.WindowRounding = 0.0f;
		style.ChildRounding = 6.0f;
		style.FrameRounding = 5.0f;
		style.PopupRounding = 6.0f;
		style.ScrollbarRounding = 6.0f;
		style.GrabRounding = 5.0f;
		style.WindowBorderSize = 0.0f;
		style.ChildBorderSize = 1.0f;
		style.FrameBorderSize = 1.0f;

		ImVec4* colors = style.Colors;
		colors[ImGuiCol_Text] = ImVec4(0.91f, 0.93f, 0.95f, 1.0f);
		colors[ImGuiCol_TextDisabled] = TextMuted;
		colors[ImGuiCol_WindowBg] = ImVec4(0.058f, 0.064f, 0.072f, 1.0f);
		colors[ImGuiCol_ChildBg] = PanelBg;
		colors[ImGuiCol_PopupBg] = ImVec4(0.10f, 0.11f, 0.12f, 1.0f);
		colors[ImGuiCol_Border] = BorderColor;
		colors[ImGuiCol_FrameBg] = ImVec4(0.13f, 0.14f, 0.15f, 1.0f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.18f, 0.20f, 0.22f, 1.0f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.22f, 0.27f, 0.31f, 1.0f);
		colors[ImGuiCol_TitleBg] = PanelBg;
		colors[ImGuiCol_TitleBgActive] = PanelBg;
		colors[ImGuiCol_Button] = ImVec4(0.15f, 0.17f, 0.18f, 1.0f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.54f, 0.88f, 1.0f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.18f, 0.42f, 0.70f, 1.0f);
		colors[ImGuiCol_Header] = ImVec4(0.17f, 0.20f, 0.22f, 1.0f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.24f, 0.54f, 0.88f, 0.75f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.24f, 0.54f, 0.88f, 1.0f);
		colors[ImGuiCol_CheckMark] = AccentGreen;
		colors[ImGuiCol_SliderGrab] = AccentBlue;
		colors[ImGuiCol_SliderGrabActive] = AccentAmber;
		colors[ImGuiCol_Separator] = BorderColor;
		colors[ImGuiCol_PlotHistogram] = AccentBlue;
	}

	ImVec2 FitImageSize(int imageWidth, int imageHeight, const ImVec2& maxSize)
	{
		if (imageWidth <= 0 || imageHeight <= 0)
		{
			return maxSize;
		}

		const float scale = std::min(maxSize.x / static_cast<float>(imageWidth), maxSize.y / static_cast<float>(imageHeight));
		return ImVec2(std::max(1.0f, imageWidth * scale), std::max(1.0f, imageHeight * scale));
	}

	void DrawAppBackground()
	{
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		const ImVec2 min = ImGui::GetWindowPos();
		const ImVec2 windowSize = ImGui::GetWindowSize();
		const ImVec2 max = ImVec2(min.x + windowSize.x, min.y + windowSize.y);
		drawList->AddRectFilled(min, max, U32(ImVec4(0.055f, 0.060f, 0.066f, 1.0f)));

		const ImU32 gridColor = U32(ImVec4(1.0f, 1.0f, 1.0f, 0.025f));
		for (float x = min.x; x < max.x; x += 24.0f)
		{
			drawList->AddLine(ImVec2(x, min.y), ImVec2(x, max.y), gridColor);
		}
		for (float y = min.y; y < max.y; y += 24.0f)
		{
			drawList->AddLine(ImVec2(min.x, y), ImVec2(max.x, y), gridColor);
		}
		drawList->AddRectFilledMultiColor(min, ImVec2(max.x, min.y + 130.0f), U32(ImVec4(0.12f, 0.15f, 0.16f, 0.82f)), U32(ImVec4(0.08f, 0.11f, 0.13f, 0.82f)),
										  U32(ImVec4(0.055f, 0.060f, 0.066f, 0.0f)), U32(ImVec4(0.055f, 0.060f, 0.066f, 0.0f)));
	}

	void DrawAccentLine(const ImVec2& min, const ImVec2& max, const ImVec4& accent)
	{
		ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(min.x, min.y), ImVec2(min.x + 4.0f, max.y), U32(accent), 6.0f, ImDrawFlags_RoundCornersLeft);
	}

	void TextMutedLine(const char* text)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, TextMuted);
		ImGui::TextUnformatted(text);
		ImGui::PopStyleColor();
	}

	std::string SizeLabel(const PreviewTexture& texture)
	{
		if (texture.width <= 0 || texture.height <= 0)
		{
			return "Empty";
		}

		return std::to_string(texture.width) + " x " + std::to_string(texture.height);
	}
} // namespace ORMTool

UIManager::UIManager() {}

UIManager::~UIManager()
{
	if (loadingThread.joinable())
	{
		loadingThread.join();
	}
}

void UIManager::Initialize(GLFWwindow* window)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ORMTool::ApplyTheme();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 130");
}

void UIManager::DrawUI()
{
	ShowMainUI();
	UpdatePreviewIfNeeded();
	JoinFinishedThread();
}

void UIManager::Render() {}

void UIManager::Shutdown()
{
	if (loadingThread.joinable())
	{
		loadingThread.join();
	}

	aoPreview.Unload();
	roughPreview.Unload();
	metallicPreview.Unload();
	ormPreview.Unload();
}

bool PreviewTexture::Load(const std::string& p)
{
	Unload();

	path = p;
	ORMTool::ImageBuffer image = ORMTool::LoadImage(p, 3);

	if (!image.IsValid())
	{
		std::cerr << "Failed to load image: " << p << std::endl;
		return false;
	}
	width = image.width;
	height = image.height;
	data = image.pixels.release();

	glGenTextures(1, &glId);
	glBindTexture(GL_TEXTURE_2D, glId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return true;
}

void PreviewTexture::Unload()
{
	if (glId)
	{
		glDeleteTextures(1, &glId);
	}
	if (channelR)
	{
		glDeleteTextures(1, &channelR);
	}
	if (channelG)
	{
		glDeleteTextures(1, &channelG);
	}
	if (channelB)
	{
		glDeleteTextures(1, &channelB);
	}
	if (data)
	{
		stbi_image_free(data);
	}

	glId = channelR = channelG = channelB = 0;
	data = nullptr;
	width = 0;
	height = 0;
}

void PreviewTexture::GenerateChannelsFromRGB(unsigned char* src, int w, int h)
{
	if (channelR)
	{
		glDeleteTextures(1, &channelR);
		channelR = 0;
	}
	if (channelG)
	{
		glDeleteTextures(1, &channelG);
		channelG = 0;
	}
	if (channelB)
	{
		glDeleteTextures(1, &channelB);
		channelB = 0;
	}

	width = w;
	height = h;

	std::vector<unsigned char> red(w * h);
	std::vector<unsigned char> green(w * h);
	std::vector<unsigned char> blue(w * h);

	for (int i = 0; i < w * h; ++i)
	{
		red[i] = src[i * 3 + 0];
		green[i] = src[i * 3 + 1];
		blue[i] = src[i * 3 + 2];
	}

	const auto createTex = [](GLuint& id, unsigned char* channelData, int w, int h)
	{
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, channelData);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	};

	createTex(channelR, red.data(), w, h);
	createTex(channelG, green.data(), w, h);
	createTex(channelB, blue.data(), w, h);
}

bool UIManager::SaveUnrealAndUnityORM(const std::string& ao, const std::string& rough, const std::string& metal, const std::string& unrealPath, const std::string& unityPath, int aoResolution,
									  int roughResolution, int metalResolution, bool doUnreal, bool doUnity, const std::function<void(float)>& progressCallback)
{
	if (!doUnreal && !doUnity)
	{
		std::cerr << "No ORM output format selected\n";
		return false;
	}

	ORMTool::ImageBuffer aoImage = ORMTool::LoadImage(ao, 1);
	ORMTool::ImageBuffer roughImage = ORMTool::LoadImage(rough, 1);
	ORMTool::ImageBuffer metalImage = ORMTool::LoadImage(metal, 1);
	if (!aoImage.IsValid() || !roughImage.IsValid() || !metalImage.IsValid())
	{
		return false;
	}

	const int outputResolution = std::max({aoResolution, roughResolution, metalResolution});
	std::vector<unsigned char> aoMap = ORMTool::ResizeGrayscale(aoImage.pixels.get(), aoImage.width, aoImage.height, aoResolution);
	std::vector<unsigned char> roughMap = ORMTool::ResizeGrayscale(roughImage.pixels.get(), roughImage.width, roughImage.height, roughResolution);
	std::vector<unsigned char> metalMap = ORMTool::ResizeGrayscale(metalImage.pixels.get(), metalImage.width, metalImage.height, metalResolution);
	if (aoMap.empty() || roughMap.empty() || metalMap.empty())
	{
		return false;
	}

	if (aoResolution != outputResolution)
	{
		aoMap = ORMTool::ResizeGrayscale(aoMap, aoResolution, outputResolution);
	}
	if (roughResolution != outputResolution)
	{
		roughMap = ORMTool::ResizeGrayscale(roughMap, roughResolution, outputResolution);
	}
	if (metalResolution != outputResolution)
	{
		metalMap = ORMTool::ResizeGrayscale(metalMap, metalResolution, outputResolution);
	}
	if (aoMap.empty() || roughMap.empty() || metalMap.empty())
	{
		return false;
	}

	if (progressCallback)
	{
		progressCallback(0.15f);
	}

	const size_t count = static_cast<size_t>(outputResolution) * outputResolution;
	const int formatCount = (doUnreal ? 1 : 0) + (doUnity ? 1 : 0);
	int completedFormats = 0;

	if (doUnreal)
	{
		std::vector<unsigned char> ormRGB(count * 3);
		for (size_t i = 0; i < count; ++i)
		{
			ormRGB[i * 3 + 0] = aoMap[i];
			ormRGB[i * 3 + 1] = roughMap[i];
			ormRGB[i * 3 + 2] = metalMap[i];

			if (progressCallback && (i % (count / 100 + 1) == 0))
			{
				const float pixelProgress = static_cast<float>(i) / static_cast<float>(count);
				progressCallback(0.15f + (completedFormats + pixelProgress) / formatCount * 0.8f);
			}
		}

		if (!ORMTool::WritePNG(unrealPath, outputResolution, outputResolution, 3, ormRGB.data(), outputResolution * 3))
		{
			std::cerr << "Failed to write: " << unrealPath << "\n";
			return false;
		}
		++completedFormats;
	}

	if (doUnity)
	{
		std::vector<unsigned char> ormRGBA(count * 4);

		for (size_t i = 0; i < count; ++i)
		{
			ormRGBA[i * 4 + 0] = metalMap[i];
			ormRGBA[i * 4 + 1] = aoMap[i];
			ormRGBA[i * 4 + 2] = 255;
			ormRGBA[i * 4 + 3] = 255 - roughMap[i];

			if (progressCallback && (i % (count / 100 + 1) == 0))
			{
				const float pixelProgress = static_cast<float>(i) / static_cast<float>(count);
				progressCallback(0.15f + (completedFormats + pixelProgress) / formatCount * 0.8f);
			}
		}

		if (!ORMTool::WritePNG(unityPath, outputResolution, outputResolution, 4, ormRGBA.data(), outputResolution * 4))
		{
			std::cerr << "Failed to write: " << unityPath << "\n";
			return false;
		}
		++completedFormats;
	}

	if (progressCallback)
	{
		progressCallback(1.0f);
	}

	return true;
}

bool UIManager::SplitORMTexture(const std::string& orm, const std::string& aoPath, const std::string& roughPath, const std::string& metalPath, int aoResolution, int roughResolution,
								int metalResolution, bool unityLayout, const std::function<void(float)>& progressCallback)
{
	ORMTool::ImageBuffer ormImage = ORMTool::LoadImage(orm, unityLayout ? 4 : 3);
	if (!ormImage.IsValid())
	{
		return false;
	}

	const size_t count = static_cast<size_t>(ormImage.width) * ormImage.height;
	std::vector<unsigned char> ao(count);
	std::vector<unsigned char> rough(count);
	std::vector<unsigned char> metal(count);
	const int channelCount = unityLayout ? 4 : 3;

	for (size_t i = 0; i < count; ++i)
	{
		const unsigned char* pixel = ormImage.pixels.get() + i * channelCount;
		if (unityLayout)
		{
			metal[i] = pixel[0];
			ao[i] = pixel[1];
			rough[i] = 255 - pixel[3];
		}
		else
		{
			ao[i] = pixel[0];
			rough[i] = pixel[1];
			metal[i] = pixel[2];
		}

		if (progressCallback && (i % (count / 100 + 1) == 0))
		{
			progressCallback(static_cast<float>(i) / static_cast<float>(count) * 0.5f);
		}
	}

	std::vector<unsigned char> aoOut = ORMTool::ResizeGrayscale(ao.data(), ormImage.width, ormImage.height, aoResolution);
	std::vector<unsigned char> roughOut = ORMTool::ResizeGrayscale(rough.data(), ormImage.width, ormImage.height, roughResolution);
	std::vector<unsigned char> metalOut = ORMTool::ResizeGrayscale(metal.data(), ormImage.width, ormImage.height, metalResolution);
	if (aoOut.empty() || roughOut.empty() || metalOut.empty())
	{
		return false;
	}

	if (progressCallback)
	{
		progressCallback(0.75f);
	}

	if (!ORMTool::WriteGrayscalePNG(aoPath, aoOut, aoResolution) || !ORMTool::WriteGrayscalePNG(roughPath, roughOut, roughResolution) ||
		!ORMTool::WriteGrayscalePNG(metalPath, metalOut, metalResolution))
	{
		std::cerr << "Failed to write split ORM textures\n";
		return false;
	}

	if (progressCallback)
	{
		progressCallback(1.0f);
	}

	return true;
}

void UIManager::JoinFinishedThread()
{
	if (!generatingORM.load() && loadingThread.joinable())
	{
		loadingThread.join();
	}
}

void UIManager::VisibleProgressBar(const float progress)
{
	ImVec4 backgroundColor = ImVec4(0.1f, 0.1f, 0.1f, 1.f);
	ImVec4 fillColor = ImVec4(0.3f, 0.5f, 0.85f, 1.0f);
	ImGui::PushStyleColor(ImGuiCol_FrameBg, backgroundColor); // background color
	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, fillColor); // fill color progress_bar

	if (generatingORM.load())
	{
		ImGui::ProgressBar(progress, ORMTool::ProgressBarWidgetSize, progress <= 0.0f ? " " : "Generating");
	}
	else
		ImGui::Dummy(ORMTool::ProgressBarWidgetSize);

	ImGui::PopStyleColor(2);
}

void UIManager::LoadTextureDataFileDialog(PreviewTexture& tex, int& resolutionIndex, bool generateChannels)
{
	nfdchar_t* outPath = nullptr;
	if (NFD_OpenDialog(ORMTool::SavedTextureFormat, nullptr, &outPath) == NFD_OKAY)
	{
		if (tex.Load(outPath))
		{
			if (generateChannels)
			{
				tex.GenerateChannelsFromRGB(tex.data, tex.width, tex.height);
			}

			for (int i = 0; i < IM_ARRAYSIZE(resolutionValues); ++i)
			{
				if (tex.width == resolutionValues[i])
				{
					resolutionIndex = i;
					break;
				}
			}
		}
		free(outPath);
	}
}

void UIManager::LoadPreviewFromFile(PreviewTexture& tex, const std::string& path, bool generateChannels)
{
	if (tex.Load(path) && generateChannels)
	{
		tex.GenerateChannelsFromRGB(tex.data, tex.width, tex.height);
	}
}

GLuint UIManager::GetSelectedPreviewTexture() const
{
	if (selectedChannel == ORMChannel::AO_R && ormPreview.channelR)
	{
		return ormPreview.channelR;
	}
	if (selectedChannel == ORMChannel::Roughness_G && ormPreview.channelG)
	{
		return ormPreview.channelG;
	}
	if (selectedChannel == ORMChannel::Metallic_B && ormPreview.channelB)
	{
		return ormPreview.channelB;
	}

	return ormPreview.glId;
}

void UIManager::SaveCurrentPreviewDialog()
{
	const GLuint textureId = GetSelectedPreviewTexture();
	if (textureId == 0 || ormPreview.width <= 0 || ormPreview.height <= 0)
	{
		std::cerr << "Nothing to save: preview texture is empty\n";
		return;
	}

	saveFormatIndex = std::clamp(saveFormatIndex, 0, IM_ARRAYSIZE(ORMTool::SaveFormatExtensions) - 1);
	nfdchar_t* outPath = nullptr;
	const nfdresult_t result = NFD_SaveDialog(ORMTool::SaveFormatFilters[saveFormatIndex], nullptr, &outPath);
	if (result == NFD_CANCEL)
	{
		return;
	}
	if (result == NFD_ERROR)
	{
		std::cerr << "Save dialog error: " << NFD_GetError() << "\n";
		return;
	}

	const std::string savePath = ORMTool::ReplaceExtension(outPath, ORMTool::SaveFormatExtensions[saveFormatIndex]);
	free(outPath);

	bool saved = false;
	switch (saveFormatIndex)
	{
		case 0:
			saved = IOService::SavePNG(savePath, textureId, ormPreview.width, ormPreview.height);
			break;
		case 1:
			saved = IOService::SaveJPG(savePath, textureId, ormPreview.width, ormPreview.height, 95);
			break;
		case 2:
			saved = IOService::SaveBMP(savePath, textureId, ormPreview.width, ormPreview.height);
			break;
		case 3:
			saved = IOService::SaveTGA(savePath, textureId, ormPreview.width, ormPreview.height);
			break;
		default:
			break;
	}

	if (!saved)
	{
		std::cerr << "Failed to save preview: " << savePath << "\n";
	}
}

void UIManager::ShowMainUI()
{
	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
	ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
	ImGui::Begin(ORMTool::TitleProgram, nullptr, ORMTool::WindowFlags);
	ORMTool::DrawAppBackground();

	auto startProcessing = [&]()
	{
		JoinFinishedThread();

		const ProcessingMode requestedMode = processingMode;
		const int aoResolution = resolutionValues[aoResolutionIndex];
		const int roughResolution = resolutionValues[roughResolutionIndex];
		const int metalResolution = resolutionValues[metalResolutionIndex];
		const bool doUnreal = generateUnrealORM;
		const bool doUnity = generateUnityORM;
		const bool useUnityLayout = splitUnityLayout;
		const std::string aoInput = aoPreview.path;
		const std::string roughInput = roughPreview.path;
		const std::string metalInput = metallicPreview.path;
		const std::string ormInput = ormPreview.path;
		const std::string unrealOutput = outputUnreal;
		const std::string unityOutput = outputUnity;

		if (requestedMode == ProcessingMode::SplitFromORM)
		{
			outputAO = ORMTool::MakeSiblingOutputPath(ormInput, "_AO.png");
			outputRoughness = ORMTool::MakeSiblingOutputPath(ormInput, "_Roughness.png");
			outputMetallic = ORMTool::MakeSiblingOutputPath(ormInput, "_Metallic.png");
		}

		const std::string aoOutput = outputAO;
		const std::string roughOutput = outputRoughness;
		const std::string metalOutput = outputMetallic;

		generatingORM.store(true);
		ormProgress.store(0.0f);
		loadingThread = std::thread(
			[this, requestedMode, aoInput, roughInput, metalInput, ormInput, unrealOutput, unityOutput, aoOutput, roughOutput, metalOutput, aoResolution, roughResolution, metalResolution, doUnreal,
			 doUnity, useUnityLayout]()
			{
				bool success = false;
				if (requestedMode == ProcessingMode::PackToORM)
				{
					success = SaveUnrealAndUnityORM(aoInput, roughInput, metalInput, unrealOutput, unityOutput, aoResolution, roughResolution, metalResolution, doUnreal, doUnity,
													[this](float p) { ormProgress.store(p); });

					if (success)
					{
						previewUnityOutput.store(!doUnreal && doUnity);
						pendingPreviewUpdateMode.store(1);
					}
				}
				else
				{
					success = SplitORMTexture(ormInput, aoOutput, roughOutput, metalOutput, aoResolution, roughResolution, metalResolution, useUnityLayout, [this](float p) { ormProgress.store(p); });

					if (success)
					{
						pendingPreviewUpdateMode.store(2);
					}
				}

				if (!success)
				{
					ormProgress.store(0.0f);
				}
				generatingORM.store(false);
			});
	};

	const bool isGenerating = generatingORM.load();
	int modeIndex = static_cast<int>(processingMode);

	ImGui::BeginChild("Header", ImVec2(0.0f, 114.0f), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
	{
		const ImVec2 headerMin = ImGui::GetWindowPos();
		const ImVec2 headerSize = ImGui::GetWindowSize();
		const ImVec2 headerMax = ImVec2(headerMin.x + headerSize.x, headerMin.y + headerSize.y);
		ORMTool::DrawAccentLine(headerMin, headerMax, ORMTool::AccentBlue);

		ImGui::SetCursorPos(ImVec2(16.0f, 12.0f));
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.97f, 0.98f, 1.0f));
		ImGui::TextUnformatted("ORMTool");
		ImGui::PopStyleColor();
		ImGui::SameLine();
		ORMTool::TextMutedLine(processingMode == ProcessingMode::PackToORM ? "Pack" : "Split");

		ImGui::SetCursorPos(ImVec2(16.0f, 42.0f));
		ImGui::SetNextItemWidth(168.0f);
		if (ImGui::Combo("##processingMode", &modeIndex, ORMTool::ProcessingModeOptions))
		{
			processingMode = static_cast<ProcessingMode>(modeIndex);
		}

		const bool isPackMode = processingMode == ProcessingMode::PackToORM;
		const bool canGeneratePack = aoPreview.glId && roughPreview.glId && metallicPreview.glId && (generateUnrealORM || generateUnityORM);
		const bool canGenerateSplit = ormPreview.glId;
		const bool canGenerate = !isGenerating && (isPackMode ? canGeneratePack : canGenerateSplit);
		const char* statusText = isGenerating ? "Processing" : canGenerate ? "Ready" : isPackMode ? "Waiting for maps" : "Waiting for ORM";
		const ImVec4 statusColor = isGenerating ? ORMTool::AccentAmber : canGenerate ? ORMTool::AccentGreen : ORMTool::TextMuted;

		ImGui::SameLine();
		ImGui::BeginDisabled(!canGenerate);
		if (canGenerate && ImNeo::Widgets::Button(isGenerating ? "Working" : "Generate", ORMTool::GenerateButtonSize, true))
		{
			startProcessing();
		}
		ImGui::EndDisabled();

		static float DisplayedProgress = 0.0f;
		DisplayedProgress = ImLerp(DisplayedProgress, ormProgress.load(), ImGui::GetIO().DeltaTime * 8.0f);
		const bool isGenerationActive = generatingORM.load();
		if (!isGenerationActive)
		{
			ormProgress.store(0.0f);
		}
		ImGui::SameLine();
		VisibleProgressBar(DisplayedProgress);

		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Text, statusColor);
		ImGui::TextUnformatted(statusText);
		ImGui::PopStyleColor();

		ImGui::SetCursorPos(ImVec2(16.0f, 78.0f));
		if (isPackMode)
		{
			ImNeo::Checkbox(ORMTool::UnrealCBoxTitle, &generateUnrealORM, ORMTool::CheckboxSize);
			ImGui::SameLine();
			ImNeo::Checkbox(ORMTool::UnityCBoxTitle, &generateUnityORM, ORMTool::CheckboxSize);
		}
		else
		{
			int splitLayoutIndex = splitUnityLayout ? 1 : 0;
			ImGui::SetNextItemWidth(148.0f);
			if (ImGui::Combo("##splitLayout", &splitLayoutIndex, ORMTool::SplitLayoutOptions))
			{
				splitUnityLayout = splitLayoutIndex == 1;
			}
			ImGui::SameLine();
			ImGui::BeginDisabled(isGenerationActive);
			if (!isGenerationActive && ImNeo::Widgets::Button("Load ORM", ORMTool::LoadButtonSize, true))
			{
				LoadTextureDataFileDialog(ormPreview, ormResolutionIndex, true);
			}
			ImGui::EndDisabled();
		}

		ImGui::SameLine(306.0f);
		ImGui::SetNextItemWidth(150.0f);
		ImGui::Combo("##channel", (int*)&selectedChannel, "All (RGB)\0AO (R)\0Roughness (G)\0Metallic (B)\0");

		const bool canSavePreview = !isGenerationActive && GetSelectedPreviewTexture() != 0 && ormPreview.width > 0 && ormPreview.height > 0;
		ImGui::SameLine();
		ImGui::SetNextItemWidth(58.0f);
		ImGui::Combo("##saveFormat", &saveFormatIndex, ORMTool::SaveFormatOptions);
		ImGui::SameLine();
		ImGui::BeginDisabled(!canSavePreview);
		if (canSavePreview && ImGui::Button("Save", ImVec2(58.0f, 26.0f)))
		{
			SaveCurrentPreviewDialog();
		}
		ImGui::EndDisabled();
	}
	ImGui::EndChild();

	const bool isPackMode = processingMode == ProcessingMode::PackToORM;
	const bool isGenerationActive = generatingORM.load();
	auto showTextureBlock = [&] /* lambda */
		(const char* label, PreviewTexture& tex, const char* title, int& resolutionIndex, ImVec4 borderColor)
	{
		ImGui::PushID(label);
		ImGui::BeginChild("slot", ORMTool::TextureSlotSize, true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		{
			const ImVec2 slotMin = ImGui::GetWindowPos();
			const ImVec2 slotSize = ImGui::GetWindowSize();
			const ImVec2 slotMax = ImVec2(slotMin.x + slotSize.x, slotMin.y + slotSize.y);
			ORMTool::DrawAccentLine(slotMin, slotMax, borderColor);

			ImGui::SetCursorPos(ImVec2(14.0f, 10.0f));
			ImGui::PushStyleColor(ImGuiCol_Text, borderColor);
			ImGui::TextUnformatted(title);
			ImGui::PopStyleColor();

			const std::string sizeLabel = ORMTool::SizeLabel(tex);
			const float sizeTextWidth = ImGui::CalcTextSize(sizeLabel.c_str()).x;
			ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - sizeTextWidth);
			ImGui::PushStyleColor(ImGuiCol_Text, ORMTool::TextMuted);
			ImGui::TextUnformatted(sizeLabel.c_str());
			ImGui::PopStyleColor();

			ImGui::SetCursorPosY(35.0f);
			const float xOffset = (ImGui::GetContentRegionAvail().x - ORMTool::TextureThumbSize.x) * 0.5f;
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + std::max(0.0f, xOffset));
			const ImVec2 thumbMin = ImGui::GetCursorScreenPos();
			const ImVec2 thumbMax = ImVec2(thumbMin.x + ORMTool::TextureThumbSize.x, thumbMin.y + ORMTool::TextureThumbSize.y);
			const bool clickable = isPackMode && !isGenerationActive;
			bool loadClicked = false;
			if (clickable)
			{
				loadClicked = ImGui::InvisibleButton("thumbButton", ORMTool::TextureThumbSize);
			}
			else
			{
				ImGui::Dummy(ORMTool::TextureThumbSize);
			}

			ImDrawList* drawList = ImGui::GetWindowDrawList();
			const bool hovered = clickable && ImGui::IsItemHovered();
			drawList->AddRectFilled(thumbMin, thumbMax, ORMTool::U32(ORMTool::PanelAltBg), 5.0f);
			drawList->AddRect(thumbMin, thumbMax, ORMTool::U32(hovered ? borderColor : ORMTool::BorderColor), 5.0f, 0, hovered ? 2.0f : 1.0f);

			if (tex.glId)
			{
				const ImVec2 imageSize = ORMTool::FitImageSize(tex.width, tex.height, ImVec2(ORMTool::TextureThumbSize.x - 12.0f, ORMTool::TextureThumbSize.y - 12.0f));
				const ImVec2 imageMin = ImVec2(thumbMin.x + (ORMTool::TextureThumbSize.x - imageSize.x) * 0.5f, thumbMin.y + (ORMTool::TextureThumbSize.y - imageSize.y) * 0.5f);
				drawList->AddImage((ImTextureID)(intptr_t)tex.glId, imageMin, ImVec2(imageMin.x + imageSize.x, imageMin.y + imageSize.y));
			}
			else
			{
				const char* emptyText = clickable ? "Load" : "Empty";
				const ImVec2 textSize = ImGui::CalcTextSize(emptyText);
				drawList->AddText(ImVec2(thumbMin.x + (ORMTool::TextureThumbSize.x - textSize.x) * 0.5f, thumbMin.y + (ORMTool::TextureThumbSize.y - textSize.y) * 0.5f),
								  ORMTool::U32(ORMTool::TextMuted), emptyText);
			}

			if (loadClicked)
			{
				LoadTextureDataFileDialog(tex, resolutionIndex);
			}

			ImGui::SetCursorPosY(132.0f);
			ImGui::TextUnformatted("Res");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(92.0f);
			ImGui::Combo("##resCombo", &resolutionIndex, resolutionOptions, IM_ARRAYSIZE(resolutionOptions));
		}
		ImGui::EndChild();
		ImGui::Spacing();
		ImGui::PopID();
	};

	ImGui::BeginChild("TextureInputs", ImVec2(178.0f, 0.0f), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
	showTextureBlock("AO", aoPreview, "AO", aoResolutionIndex, ORMTool::AccentRed);
	showTextureBlock("Rough", roughPreview, "Roughness", roughResolutionIndex, ORMTool::AccentGreen);
	showTextureBlock("Metal", metallicPreview, "Metallic", metalResolutionIndex, ORMTool::AccentBlue);
	ImGui::EndChild();
	ImGui::SameLine();

	ImGui::BeginChild("Viewport", ImVec2(0.0f, 0.0f), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
	const ImVec2 panelMin = ImGui::GetWindowPos();
	const ImVec2 panelSize = ImGui::GetWindowSize();
	const ImVec2 panelMax = ImVec2(panelMin.x + panelSize.x, panelMin.y + panelSize.y);
	ORMTool::DrawAccentLine(panelMin, panelMax, isPackMode ? ORMTool::AccentBlue : ORMTool::AccentAmber);

	ImGui::SetCursorPos(ImVec2(16.0f, 12.0f));
	ImGui::TextUnformatted(isPackMode ? "ORM Preview" : "ORM Source");
	const std::string previewSizeLabel = ORMTool::SizeLabel(ormPreview);
	const float previewSizeWidth = ImGui::CalcTextSize(previewSizeLabel.c_str()).x;
	ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - previewSizeWidth);
	ImGui::PushStyleColor(ImGuiCol_Text, ORMTool::TextMuted);
	ImGui::TextUnformatted(previewSizeLabel.c_str());
	ImGui::PopStyleColor();

	ImGui::SetCursorPos(ImVec2(16.0f, 42.0f));
	ImVec2 canvasSize = ImGui::GetContentRegionAvail();
	canvasSize.x = std::max(1.0f, canvasSize.x);
	canvasSize.y = std::max(1.0f, canvasSize.y - 8.0f);
	const ImVec2 canvasMin = ImGui::GetCursorScreenPos();
	const ImVec2 canvasMax = ImVec2(canvasMin.x + canvasSize.x, canvasMin.y + canvasSize.y);
	const bool canvasClickable = !isPackMode && !isGenerationActive;
	bool previewClicked = false;
	if (canvasClickable)
	{
		previewClicked = ImGui::InvisibleButton("previewCanvas", canvasSize);
	}
	else
	{
		ImGui::Dummy(canvasSize);
	}

	ImDrawList* drawList = ImGui::GetWindowDrawList();
	drawList->AddRectFilled(canvasMin, canvasMax, ORMTool::U32(ORMTool::PanelAltBg), 6.0f);
	for (float y = canvasMin.y; y < canvasMax.y; y += 12.0f)
	{
		drawList->AddLine(ImVec2(canvasMin.x, y), ImVec2(canvasMax.x, y), ORMTool::U32(ImVec4(1.0f, 1.0f, 1.0f, 0.018f)));
	}
	drawList->AddRect(canvasMin, canvasMax, ORMTool::U32(canvasClickable && ImGui::IsItemHovered() ? ORMTool::AccentAmber : ORMTool::BorderColor), 6.0f);

	GLuint texId = ormPreview.glId;
	if (selectedChannel == ORMChannel::AO_R)
		texId = ormPreview.channelR;
	else if (selectedChannel == ORMChannel::Roughness_G)
		texId = ormPreview.channelG;
	else if (selectedChannel == ORMChannel::Metallic_B)
		texId = ormPreview.channelB;
	if (texId)
	{
		const ImVec2 imageSize = ORMTool::FitImageSize(ormPreview.width, ormPreview.height, ImVec2(canvasSize.x - 24.0f, canvasSize.y - 24.0f));
		const ImVec2 imageMin = ImVec2(canvasMin.x + (canvasSize.x - imageSize.x) * 0.5f, canvasMin.y + (canvasSize.y - imageSize.y) * 0.5f);
		drawList->AddImage((ImTextureID)(intptr_t)texId, imageMin, ImVec2(imageMin.x + imageSize.x, imageMin.y + imageSize.y));
		drawList->AddRect(imageMin, ImVec2(imageMin.x + imageSize.x, imageMin.y + imageSize.y), ORMTool::U32(ImVec4(0.0f, 0.0f, 0.0f, 0.55f)), 4.0f);
	}
	else
	{
		const char* emptyText = canvasClickable ? "Load ORM" : "No preview";
		const ImVec2 textSize = ImGui::CalcTextSize(emptyText);
		drawList->AddText(ImVec2(canvasMin.x + (canvasSize.x - textSize.x) * 0.5f, canvasMin.y + (canvasSize.y - textSize.y) * 0.5f), ORMTool::U32(ORMTool::TextMuted), emptyText);

		if (generatingORM.load())
		{
			ImVec2 loaderPos = ImVec2(canvasMin.x + canvasSize.x * 0.5f - 45.0f, canvasMin.y + canvasSize.y * 0.5f - 40.0f);
			ImNeo::AddLoadingCube("Generate", loaderPos);
		}
	}

	if (previewClicked)
	{
		LoadTextureDataFileDialog(ormPreview, ormResolutionIndex, true);
	}
	ImGui::EndChild();
	ImGui::End();
}

void UIManager::UpdatePreviewIfNeeded()
{
	const int updateMode = pendingPreviewUpdateMode.exchange(0);
	if (updateMode == 1)
	{
		LoadPreviewFromFile(ormPreview, previewUnityOutput.load() ? outputUnity : outputUnreal, true);
	}
	else if (updateMode == 2)
	{
		LoadPreviewFromFile(aoPreview, outputAO, false);
		LoadPreviewFromFile(roughPreview, outputRoughness, false);
		LoadPreviewFromFile(metallicPreview, outputMetallic, false);
	}
}
