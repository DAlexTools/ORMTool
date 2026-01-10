#pragma once
#include <atomic>
#include <future>
#include <thread>
#include <mutex>
#include <functional>
#include <array>
#include <cmath>
#include <map>


#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "ImNeo.h"
#include "Utils/Types.h"

class UIManager final
{
public:
	UIManager();
	~UIManager();

	void Initialize(GLFWwindow* window);
	void DrawUI();
	void Render();
	void Shutdown();

private:
	// UI state and logic
	void ShowMainUI();
	void UpdatePreviewIfNeeded();
	void StartORMGeneration();
	void VisibleProgressBar(const float progress);
	void LoadTextureDataFileDialog(PreviewTexture& tex, int& resolutionIndex);

	// Image loading/generation
	bool SaveUnrealAndUnityORM(const std::string& ao, const std::string& rough, const std::string& metal, const std::string& unrealPath, const std::string& unityPath,
	bool doUnreal, bool doUnity, const std::function<void(float)>& progressCallback = nullptr);

	// Internal state
	PreviewTexture aoPreview, roughPreview, metallicPreview, ormPreview;

	bool generateUnrealORM = true;
	bool generateUnityORM = true;
	ORMChannel selectedChannel = ORMChannel::AllRGB;

	int aoResolutionIndex = 0;
	int roughResolutionIndex = 0;
	int metalResolutionIndex = 0;


	float ormProgress = 0.0f;
	std::atomic<bool> needsPreviewUpdate = false;
	std::atomic<bool> generatingORM = false;
	std::atomic<bool> loadingTexture;
	std::string outputUnreal = "orm_unreal.png";
	std::string outputUnity = "orm_unity.png";

	std::mutex loadingMutex;
	std::thread loadingThread;

	static constexpr int resolutionValues[6] = { 128, 256, 512, 1024, 2048, 4096 };
	static constexpr const char* resolutionOptions[6] = { "128","256","512","1024","2048","4096" };
};

