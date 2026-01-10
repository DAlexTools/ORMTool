
#include "UIManager.h"
#include <imgui.h>
#include <imgui_internal.h>

#include <nfd.h>

#include <stb_image.h>
#include <stb_image_write.h>
#include <iostream>
#include <filesystem>
#include <GLFW/glfw3.h>

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <future>

#define STB_IMAGE_RESIZE_IMPLEMENTATION

namespace ORMTool
{
	constexpr const char* TitleProgram 	= "ORMTool";
	constexpr const char* UnrealCBoxTitle = "Unreal";
	constexpr const char* UnityCBoxTitle = "Unity ";
	constexpr const char* SavedTextureFormat = "png,jpg";
	constexpr const float CheckboxSize  = 14.0f;
	constexpr const auto& WindowFlags =
		ImGuiWindowFlags_NoResize		|
		ImGuiWindowFlags_NoMove 		|
		ImGuiWindowFlags_NoCollapse 	|
		ImGuiWindowFlags_NoScrollbar	|
		ImGuiWindowFlags_NoTitleBar;


	constexpr ImVec2 WindowSize			 	= ImVec2(686, 80);
	constexpr ImVec2 GenerateButtonSize 	= ImVec2(160, 26);
	constexpr ImVec2 ProgressBarWidgetSize 	= ImVec2(522, 26);
}

namespace fs = std::filesystem;

UIManager::UIManager() {}

UIManager::~UIManager() {}

void UIManager::Initialize(GLFWwindow* window)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 130");
}

void UIManager::DrawUI()
{
	ShowMainUI();
	UpdatePreviewIfNeeded();
}

void UIManager::Render()
{
}

void UIManager::Shutdown()
{
	aoPreview.Unload();
	roughPreview.Unload();
	metallicPreview.Unload();
	ormPreview.Unload();
}

bool PreviewTexture::Load(const std::string& p)
{
	Unload();

	path = p;
	int channels;
	data = stbi_load(p.c_str(), &width, &height, &channels, 3);

	if(!data)
	{
		std::cerr << "Failed to load image: " << p << std::endl;
		return false;
	}

	glGenTextures(1, &glId);
	glBindTexture(GL_TEXTURE_2D, glId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return true;
}

void PreviewTexture::Unload()
{
	if(glId)
	{
		glDeleteTextures(1, &glId);
	}
	if(channelR)
	{
		glDeleteTextures(1, &channelR);
	}
	if(channelG)
	{
		glDeleteTextures(1, &channelG);
	}
	if(channelB)
	{
		glDeleteTextures(1, &channelB);
	}
	if(data)
	{
		stbi_image_free(data);
	}

	glId = channelR = channelG = channelB = 0;
	data = nullptr;
}

void PreviewTexture::GenerateChannelsFromRGB(unsigned char* src, int w, int h)
{
	width = w;
	height = h;

	std::vector<unsigned char> red(w * h);
	std::vector<unsigned char> green(w * h);
	std::vector<unsigned char> blue(w * h);

	for(int i = 0; i < w * h; ++i)
	{
		red[i] = src[i * 3 + 0];
		green[i] = src[i * 3 + 1];
		blue[i] = src[i * 3 + 2];
	}

	const auto createTex = [] (GLuint& id, unsigned char* channelData, int w, int h)
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

unsigned char* LoadGrayscale(const std::string& path, int& width, int& height)
{
	int channels;
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 1);
	if(!data) std::cerr << "Failed to load: " << path << "\n";

	return data;
}

bool UIManager::SaveUnrealAndUnityORM(const std::string& ao, const std::string& rough, const std::string& metal, const std::string& unrealPath, const std::string& unityPath,
	bool doUnreal, bool doUnity,const std::function<void(float)>& progressCallback)
{
	ormPreview.Unload();
	int w1, h1, w2, h2, w3, h3;
	unsigned char* aoData = LoadGrayscale(ao, w1, h1);
	unsigned char* roughData = LoadGrayscale(rough, w2, h2);
	unsigned char* metalData = LoadGrayscale(metal, w3, h3);

	if(!aoData || !roughData || !metalData) return false;

	if(w1 != w2 || w1 != w3 || h1 != h2 || h1 != h3)
	{
		std::cerr << "Size mismatch!\n";
		return false;
	}

	size_t count = w1 * h1;
	float totalSteps = 0.0f;
	if(doUnreal || doUnity)
	{
		totalSteps += 2.0f;
	}

	float currentStep = 0.0f;

	if(doUnreal)
	{
		std::vector<unsigned char> ormRGB(count * 3);
		for(size_t i = 0; i < count; ++i)
		{
			ormRGB[i * 3 + 0] = aoData[i];
			ormRGB[i * 3 + 1] = roughData[i];
			ormRGB[i * 3 + 2] = metalData[i];

			if(progressCallback && (i % (count / 100 + 1) == 0))
			{
				float pixelProgress = static_cast<float>(i) / count;
				progressCallback((currentStep + pixelProgress) / totalSteps);
			}
		}
		currentStep += 1.0f;

		stbi_write_png(unrealPath.c_str(), w1, h1, 3, ormRGB.data(), w1 * 3);
		currentStep += 1.0f;
		if(progressCallback)
		{
			progressCallback(currentStep / totalSteps);
		}

		ormPreview.Unload();
		ormPreview.path = unrealPath;
		ormPreview.width = w1;
		ormPreview.height = h1;
		glGenTextures(1, &ormPreview.glId);
		glBindTexture(GL_TEXTURE_2D, ormPreview.glId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w1, h1, 0, GL_RGB, GL_UNSIGNED_BYTE, ormRGB.data());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		ormPreview.GenerateChannelsFromRGB(ormRGB.data(), w1, h1);
	}

	if(doUnity)
	{
		std::vector<unsigned char> ormRGBA(count * 4);

		for(size_t i = 0; i < count; ++i)
		{
			ormRGBA[i * 4 + 0] = metalData[i];
			ormRGBA[i * 4 + 1] = aoData[i];
			ormRGBA[i * 4 + 2] = 255;
			ormRGBA[i * 4 + 3] = 255 - roughData[i];

			if(progressCallback && (i % (count / 100 + 1) == 0))
			{
				float pixelProgress = static_cast<float>(i) / count;
				progressCallback((currentStep + pixelProgress) / totalSteps);
			}
		}
		currentStep += 1.0f;

		stbi_write_png(unityPath.c_str(), w1, h1, 4, ormRGBA.data(), w1 * 4);
		currentStep += 1.0f;
		if(progressCallback) progressCallback(currentStep / totalSteps);
	}

	stbi_image_free(aoData);
	stbi_image_free(roughData);
	stbi_image_free(metalData);

	if(progressCallback)
	{
		progressCallback(1.0f);
	}

	return true;
}

void UIManager::StartORMGeneration()
{
	SaveUnrealAndUnityORM(
		aoPreview.path, roughPreview.path, metallicPreview.path,
		outputUnreal, outputUnity,
		generateUnrealORM, generateUnityORM, [this](float p) { ormProgress = p; }
	);

	needsPreviewUpdate = true;
	generatingORM = false;
}

void UIManager::VisibleProgressBar(const float progress)
{
	ImVec4 backgroundColor = ImVec4(0.1f, 0.1f, 0.1f, 1.f);
	ImVec4 fillColor      = ImVec4(0.3f, 0.5f, 0.85f, 1.0f);
	ImGui::PushStyleColor(ImGuiCol_FrameBg,     	backgroundColor); 	// background color
	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, 	fillColor);     	// fill color progress_bar

	if (generatingORM)
	{
		ImGui::ProgressBar(progress,
			ORMTool::ProgressBarWidgetSize,
			ormProgress == 0.0f ?
			" " : "Generating");
	}
	else
		ImGui::Dummy(ORMTool::ProgressBarWidgetSize);

	ImGui::PopStyleColor(2);
}

void UIManager::LoadTextureDataFileDialog(PreviewTexture& tex, int& resolutionIndex)
{
	nfdchar_t* outPath = nullptr;
	if(NFD_OpenDialog(ORMTool::SavedTextureFormat, nullptr, &outPath) == NFD_OKAY)
	{
		tex.Unload();
		tex.path = outPath;
		if(tex.Load(outPath))
		{
			for(int i = 0; i < IM_ARRAYSIZE(resolutionValues); ++i)
			{
				if(tex.width == resolutionValues[i])
				{
					resolutionIndex = i;
					break;
				}
			}
		}
		free(outPath);
	}
}


void UIManager::ShowMainUI()
{
#pragma region Test
	if(ImGui::BeginMainMenuBar())
	{
		if(ImGui::BeginMenu("File"))
		{
			if(ImGui::BeginMenu("Save"))
			{
				if(ImGui::MenuItem("Save to PNG"))
				{
					std::cout << "Save selected\n";
				}
				if(ImGui::MenuItem("Save to JPG"))
				{
					std::cout << "Save selected\n";
				}
				ImGui::EndMenu();
			}

			if(ImGui::MenuItem("Exit"))
			{
				std::exit(0);
			}

			ImGui::EndMenu();
		}

		if(ImGui::BeginMenu("About"))
		{
			if(ImGui::MenuItem("About"))
			{
				std::cout << "Save selected\n";
			}

			if(ImGui::MenuItem("Git Link..."))
			{
				std::cout << "Save selected\n";
			}

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
#pragma endregion Test

	ImVec2 menuHeight = ImVec2(0, ImGui::GetFrameHeight());
	ImGui::SetNextWindowPos(menuHeight);
	ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y - menuHeight.y));

	ImGui::Begin(ORMTool::TitleProgram, nullptr, ORMTool::WindowFlags);
	ImGui::BeginChild("Header", ORMTool::WindowSize, true);

	std::string generatedStringButton = generatingORM ? "Cancel" : "Generate";
	if(ImNeo::Widgets::Button(generatedStringButton.c_str(), ORMTool::GenerateButtonSize,true) && !generatingORM)
	{
		generatingORM = true;
		ormProgress = 0.0f;
		loadingThread = std::thread(&UIManager::StartORMGeneration,this);
		loadingThread.detach();
	}

	static float DisplayedProgress = 0.0f;
	DisplayedProgress = ImLerp(DisplayedProgress, ormProgress, ImGui::GetIO().DeltaTime * 8.0f);
	if(!generatingORM)
	{
		ormProgress = 0.0f;
	}
	ImGui::SameLine();

	VisibleProgressBar(DisplayedProgress);

	ImNeo::Checkbox(ORMTool::UnrealCBoxTitle, &generateUnrealORM, ORMTool::CheckboxSize);
	ImGui::SameLine();
	ImNeo::Checkbox(ORMTool::UnityCBoxTitle, &generateUnityORM, ORMTool::CheckboxSize);
	ImGui::SameLine(400.f, 2.0f);

	ImGui::SetNextItemWidth(150.0f);
	ImGui::Combo(" ", (int*)&selectedChannel, "All (RGB)\0AO (R)\0Roughness (G)\0Metallic (B)\0");

	ImGui::EndChild();

	ImGui::Columns(2, nullptr, false);
	ImGui::SetColumnWidth(0, 145);


	auto showTextureBlock = [&] /* lambda */
	(const char* label, PreviewTexture& tex, const char* title, int& resolutionIndex, ImVec4 borderColor)
	{
		ImGui::PushID(label);

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.4f, 0.4f, 1.0f)); //
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, borderColor);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

		if(ImGui::ImageButton(label, (ImTextureID)(intptr_t)tex.glId, ImVec2(128, 128)))
		{
			LoadTextureDataFileDialog(tex, resolutionIndex);
		}

		ImGui::PopStyleColor(3);
		ImGui::SetNextItemWidth(135.0f);
		ImGui::Combo("##resCombo", &resolutionIndex, resolutionOptions, IM_ARRAYSIZE(resolutionOptions));
		ImGui::Dummy(ImVec2(0, 26));
		ImGui::PopID();
	};


	showTextureBlock("AO", aoPreview, "AO", aoResolutionIndex, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
	showTextureBlock("Rough", roughPreview, "Roughness", roughResolutionIndex, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
	showTextureBlock("Metal", metallicPreview, "Metallic", metalResolutionIndex, ImVec4(0.0f, 0.5f, 1.0f, 1.0f));

	ImGui::NextColumn();

	ImGui::BeginChild("Viewport", ImVec2(0, 544), true);
	float previewWidth = ImGui::GetContentRegionAvail().x - 1.0f;
	float aspect = ormPreview.width > 0 ? (float)ormPreview.height / ormPreview.width : 1.0f;
	float previewHeight = previewWidth * aspect;

	GLuint texId = ormPreview.glId;
	if(selectedChannel == ORMChannel::AO_R) 			texId = ormPreview.channelR;
	else if(selectedChannel == ORMChannel::Roughness_G) texId = ormPreview.channelG;
	else if(selectedChannel == ORMChannel::Metallic_B) 	texId = ormPreview.channelB;





	if(texId)
	{
		ImGui::Image((ImTextureID)(intptr_t)texId, ImVec2(previewWidth, previewHeight));
	}
	else
	{
		ImVec2 pos = ImGui::GetCursorScreenPos();
		ImVec2 loaderPos = ImVec2
		(
			pos.x + previewWidth - 85,
			pos.y + previewHeight - 90
		);

		if (generatingORM )
			ImNeo::AddLoadingCube("Generate", loaderPos);

	}
	ImGui::EndChild();
	ImGui::Columns(1);
	ImGui::End();


}

void UIManager::UpdatePreviewIfNeeded()
{
	if(!needsPreviewUpdate) return;

	ormPreview.Unload();
	ormPreview.path = outputUnreal;

	int w, h, channels;
	unsigned char* data = stbi_load(outputUnreal.c_str(), &w, &h, &channels, 3);
	if(data) {
		ormPreview.width = w;
		ormPreview.height = h;
		glGenTextures(1, &ormPreview.glId);
		glBindTexture(GL_TEXTURE_2D, ormPreview.glId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		ormPreview.GenerateChannelsFromRGB(data, w, h);
		stbi_image_free(data);
	}

	needsPreviewUpdate = false;
}
