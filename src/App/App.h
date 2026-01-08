#pragma once 
#include <GLFW/glfw3.h>
#include <memory>
#include <string_view>
#include <optional>
#include "UIManager.h"



enum class InitStatus
	{
		OK,
		GLFW_InitFailed,
		WindowCreationFailed,
		OpenGL_InitFailed,
		Fail
	};

class Application
{
public:
	Application();
	~Application();

	[[nodiscard]] InitStatus InitializeApplication();
	void RunApplication();
	void Shutdown();
	bool IsInitialized() const;

	static std::string_view GetInitStatus(InitStatus status);
private:
	static void GLFWErrorCallback(int error, const char* description);
	[[nodiscard]] InitStatus InitializeGLFW();
	void RenderScene();
	void RenderUIManager();


	GLFWwindow* window = nullptr;
	std::optional<InitStatus> initStatus;
	std::unique_ptr<UIManager> uiManager;
};