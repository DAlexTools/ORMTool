#pragma once
#include "UIManager.h"
#include <GLFW/glfw3.h>
#include <memory>
#include <optional>
#include <string_view>

/**
 * @brief Startup and initialization result codes.
 */
enum class InitStatus
{
	OK,                   ///< Initialization completed successfully.
	GLFW_InitFailed,      ///< GLFW could not be initialized.
	WindowCreationFailed, ///< The GLFW window could not be created.
	OpenGL_InitFailed,    ///< OpenGL could not be initialized.
	Fail                  ///< Generic initialization failure.
};

/**
 * @brief Owns the GLFW window, ImGui lifecycle, and main render loop.
 */
class Application
{
public:
	/**
	 * @brief Creates the application object and its UI manager.
	 */
	Application();

	/**
	 * @brief Shuts down application resources.
	 */
	~Application();

	/**
	 * @brief Initializes GLFW, the application window, and UI resources.
	 * @return The final initialization status.
	 */
	[[nodiscard]] InitStatus InitializeApplication();

	/**
	 * @brief Runs the frame loop until the window is closed.
	 */
	void RunApplication();

	/**
	 * @brief Releases UI, window, ImGui, and GLFW resources.
	 */
	void Shutdown();

	/**
	 * @brief Checks whether initialization completed successfully.
	 * @return True when the application is ready to run.
	 */
	bool IsInitialized() const;

	/**
	 * @brief Converts an initialization status to a readable message.
	 * @param status Status value to describe.
	 * @return Static text describing the status.
	 */
	static std::string_view GetInitStatus(InitStatus status);

private:
	/**
	 * @brief Receives GLFW error messages and writes them to stderr.
	 * @param error GLFW error code.
	 * @param description Human-readable GLFW error description.
	 */
	static void GLFWErrorCallback(int error, const char* description);

	/**
	 * @brief Initializes GLFW and creates the application window.
	 * @return Detailed initialization status.
	 */
	[[nodiscard]] InitStatus InitializeGLFW();

	/**
	 * @brief Clears and prepares the OpenGL framebuffer for UI rendering.
	 */
	void RenderScene();

	/**
	 * @brief Draws and renders the ImGui UI manager.
	 */
	void RenderUIManager();

	/**
	 * @brief Applies GLFW window and OpenGL context hints before window creation.
	 */
	void ConfigureGLFWHints();

	GLFWwindow* window = nullptr;              ///< Main GLFW window handle.
	std::optional<InitStatus> initStatus;      ///< Cached initialization status.
	std::unique_ptr<UIManager> uiManager;      ///< UI lifecycle and rendering manager.
};
