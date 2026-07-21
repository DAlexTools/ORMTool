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

/**
 * @brief Coordinates ImGui rendering, texture previewing, and ORM packing/splitting.
 */
class UIManager final
{
public:
	/**
	 * @brief Creates an uninitialized UI manager.
	 */
	UIManager();

	/**
	 * @brief Releases UI manager resources.
	 */
	~UIManager();

	/**
	 * @brief Initializes ImGui bindings for the supplied GLFW window.
	 * @param window Window that owns the ImGui context.
	 */
	void Initialize(GLFWwindow* window);

	/**
	 * @brief Builds the current frame's UI commands.
	 */
	void DrawUI();

	/**
	 * @brief Performs post-draw UI rendering work.
	 */
	void Render();

	/**
	 * @brief Releases preview textures and joins background work.
	 */
	void Shutdown();

private:
	/**
	 * @brief Draws the main application window.
	 */
	void ShowMainUI();

	/**
	 * @brief Reloads generated preview textures after background processing finishes.
	 */
	void UpdatePreviewIfNeeded();

	/**
	 * @brief Draws a progress bar when an operation is running.
	 * @param progress Current progress value in the range [0, 1].
	 */
	void VisibleProgressBar(const float progress);

	/**
	 * @brief Opens a file picker and loads the selected image into a preview texture.
	 * @param tex Preview texture to update.
	 * @param resolutionIndex Resolution selector updated from the loaded image.
	 * @param generateChannels Whether to generate per-channel preview textures.
	 */
	void LoadTextureDataFileDialog(PreviewTexture& tex, int& resolutionIndex, bool generateChannels = false);

	/**
	 * @brief Loads a preview texture from a known file path.
	 * @param tex Preview texture to update.
	 * @param path Source file path.
	 * @param generateChannels Whether to generate per-channel preview textures.
	 */
	void LoadPreviewFromFile(PreviewTexture& tex, const std::string& path, bool generateChannels);

	/**
	 * @brief Opens a save dialog for the currently selected preview texture.
	 */
	void SaveCurrentPreviewDialog();

	/**
	 * @brief Joins the background loading thread when it has finished.
	 */
	void JoinFinishedThread();

	/**
	 * @brief Returns the OpenGL texture id for the selected preview mode.
	 * @return The currently selected preview texture id, or 0 if none is available.
	 */
	GLuint GetSelectedPreviewTexture() const;

	/**
	 * @brief Packs AO, roughness, and metallic maps into Unreal and/or Unity ORM outputs.
	 * @param ao Ambient occlusion input path.
	 * @param rough Roughness input path.
	 * @param metal Metallic input path.
	 * @param unrealPath Output path for Unreal Engine ORM layout.
	 * @param unityPath Output path for Unity ORM layout.
	 * @param aoResolution AO input resolution.
	 * @param roughResolution Roughness input resolution.
	 * @param metalResolution Metallic input resolution.
	 * @param doUnreal Whether to write the Unreal Engine layout.
	 * @param doUnity Whether to write the Unity layout.
	 * @param progressCallback Optional progress callback receiving [0, 1].
	 * @return True when all requested outputs were written successfully.
	 */
	bool SaveUnrealAndUnityORM(const std::string& ao, const std::string& rough, const std::string& metal,
		const std::string& unrealPath, const std::string& unityPath,
		int aoResolution, int roughResolution, int metalResolution,
		bool doUnreal, bool doUnity, const std::function<void(float)>& progressCallback = nullptr);

	/**
	 * @brief Splits a packed ORM texture into separate AO, roughness, and metallic maps.
	 * @param orm Packed ORM input path.
	 * @param aoPath Output path for ambient occlusion.
	 * @param roughPath Output path for roughness.
	 * @param metalPath Output path for metallic.
	 * @param aoResolution Output AO resolution.
	 * @param roughResolution Output roughness resolution.
	 * @param metalResolution Output metallic resolution.
	 * @param unityLayout Whether to interpret the input as Unity channel layout.
	 * @param progressCallback Optional progress callback receiving [0, 1].
	 * @return True when all split maps were written successfully.
	 */
	bool SplitORMTexture(const std::string& orm, const std::string& aoPath, const std::string& roughPath,
		const std::string& metalPath, int aoResolution, int roughResolution, int metalResolution,
		bool unityLayout, const std::function<void(float)>& progressCallback = nullptr);

	/**
	 * @brief Current image processing workflow.
	 */
	enum class ProcessingMode : int
	{
		PackToORM,    ///< Pack separate texture maps into an ORM texture.
		SplitFromORM  ///< Split a packed ORM texture into separate maps.
	};

	/** @brief Preview textures shown in the UI. */
	PreviewTexture aoPreview, roughPreview, metallicPreview, ormPreview;

	ProcessingMode processingMode = ProcessingMode::PackToORM; ///< Active processing workflow.
	bool splitUnityLayout = false;                             ///< Uses Unity channel layout while splitting.
	bool generateUnrealORM = true;                             ///< Writes Unreal ORM output when packing.
	bool generateUnityORM = true;                              ///< Writes Unity ORM output when packing.
	ORMChannel selectedChannel = ORMChannel::AllRGB;           ///< Active preview channel selection.

	int aoResolutionIndex = 0;        ///< Selected AO resolution index.
	int roughResolutionIndex = 0;     ///< Selected roughness resolution index.
	int metalResolutionIndex = 0;     ///< Selected metallic resolution index.
	int ormResolutionIndex = 0;       ///< Selected ORM input resolution index.
	int saveFormatIndex = 0;          ///< Selected output file format index.


	std::atomic<float> ormProgress{0.0f};          ///< Current background operation progress.
	std::atomic<int> pendingPreviewUpdateMode{0};  ///< Pending preview reload mode after background work.
	std::atomic<bool> generatingORM = false;       ///< True while ORM processing is running.
	std::atomic<bool> loadingTexture{false};       ///< True while texture loading is running.
	std::string outputUnreal = "orm_unreal.png";   ///< Default Unreal ORM output path.
	std::string outputUnity = "orm_unity.png";     ///< Default Unity ORM output path.
	std::string outputAO = "ao.png";               ///< Default split AO output path.
	std::string outputRoughness = "roughness.png"; ///< Default split roughness output path.
	std::string outputMetallic = "metallic.png";   ///< Default split metallic output path.
	std::atomic<bool> previewUnityOutput{false};   ///< True when the Unity output should be previewed.

	std::mutex loadingMutex;   ///< Guards background loading thread state.
	std::thread loadingThread; ///< Background worker for loading and image processing.

	static constexpr int resolutionValues[6] = { 128, 256, 512, 1024, 2048, 4096 };        ///< Supported square texture sizes.
	static constexpr const char* resolutionOptions[6] = { "128","256","512","1024","2048","4096" }; ///< UI labels for supported texture sizes.
};
