#include "App.h"

#include <iostream>
#include "Constants.h"
#ifdef __linux__
    #include <X11/Xlib.h>
    #include <X11/Xatom.h>

#elif _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#elif __APPLE__
    #include <ApplicationServices/ApplicationServices.h>
#endif

Application::Application() : uiManager(std::make_unique<UIManager>()) { }

Application::~Application()
{
	Shutdown();
}

[[nodiscard]] InitStatus Application::InitializeApplication()
{
	if(initStatus.has_value())
	{
		return *initStatus;
	}

	const auto glfwStatus = InitializeGLFW();
	if(glfwStatus != InitStatus::OK)
	{
		std::cerr << GetInitStatus(glfwStatus) << " at " << __FUNCTION__ << ":" << __LINE__ << "\n";
		initStatus = glfwStatus;
		return glfwStatus;
	}
	uiManager->Initialize(window);

	int major, minor, rev;
	glfwGetVersion(&major, &minor, &rev);
	std::cout << "GLFW Version: " << major << "." << minor << "." << rev << "\n";
	std::cout << "GLFW Initialized successfully\n";

	initStatus = InitStatus::OK;
	return *initStatus;
}

void Application::RunApplication()
{
	if(!IsInitialized())
	{
		std::cerr << "Application not initialized!\n";
		return;
	}

	while(!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		RenderUIManager();
		RenderScene();
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);
	}
}

void Application::Shutdown()
{
	if(uiManager)
	{
		uiManager->Shutdown();
		uiManager.reset();
	}

	if(window)
	{
		glfwDestroyWindow(window);
		window = nullptr;
	}

	// uiManager->Shutdown();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();
	initStatus.reset();
}
bool Application::IsInitialized() const
{
	return initStatus.has_value() && *initStatus == InitStatus::OK;
}

std::string_view Application::GetInitStatus(InitStatus status)
{
	switch(status)
	{
	case InitStatus::OK: return "Success";
	case InitStatus::GLFW_InitFailed: return "GLFW initialization failed";
	case InitStatus::WindowCreationFailed: return "Window creation failed";
	case InitStatus::OpenGL_InitFailed: return "OpenGL initialization failed";
	case InitStatus::Fail: return "Generic failure";
	default: return "Unknown error";
	}
}

[[nodiscard]] InitStatus Application::InitializeGLFW()
{
    glfwSetErrorCallback(GLFWErrorCallback);
    if(!glfwInit())
    {
        return InitStatus::GLFW_InitFailed;
    }

	ConfigureGLFWHints();

    window = glfwCreateWindow( //
		ORM::WindowWidth, //
		ORM::WindowHeight, //
		ORM::TitleStr, //
		nullptr, //
		nullptr);

    if(!window)
    {
        return InitStatus::WindowCreationFailed;
    }

    glfwSetWindowCloseCallback(window, [](GLFWwindow* w)
	{
        glfwSetWindowShouldClose(w, GLFW_TRUE);
    });

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwRestoreWindow(window);
	glfwShowWindow(window);

    return InitStatus::OK;
}


void Application::RenderScene()
{
	glViewport(0, 0, ORM::WindowWidth, ORM::WindowHeight);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f); //
	glClear(GL_COLOR_BUFFER_BIT);
}

void Application::RenderUIManager()
{
	uiManager->DrawUI();
	uiManager->Render();
}

void Application::ConfigureGLFWHints()
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, ORM::GraphicsConfig::OPENGL_MAJOR);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, ORM::GraphicsConfig::OPENGL_MINOR);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
    glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
    glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_TRUE);
    glfwWindowHint(GLFW_FLOATING, GLFW_FALSE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_FALSE);

#ifdef _DEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

	glfwWindowHint(GLFW_SAMPLES, ORM::GraphicsConfig::ENABLE_DEBUG);
}

void Application::GLFWErrorCallback(int error, const char* description)
{
	std::cerr << "[GLFW Error] (" << error << "): " << description << std::endl;
}



