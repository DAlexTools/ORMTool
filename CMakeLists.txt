cmake_minimum_required(VERSION 3.16)
project(ORMTool)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
include(FetchContent)

# ---------------------------------------------------------
# Output folders
# ---------------------------------------------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out)

message(STATUS "📦 Starting to fetch dependencies...")

if(UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
    message(STATUS "GTK3 include dirs: ${GTK3_INCLUDE_DIRS}")
    message(STATUS "GTK3 libraries: ${GTK3_LIBRARIES}")
    message(STATUS "GTK3 flags: ${GTK3_CFLAGS_OTHER}")

    add_compile_options(${GTK3_CFLAGS_OTHER})
    include_directories(${GTK3_INCLUDE_DIRS})
endif()


#  -------------------------------------------------------------------------
# GLFW
message(STATUS "⬇️  Fetching GLFW...")
FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG        3.4
)
FetchContent_MakeAvailable(glfw)
message(STATUS "✅ GLFW is ready!")
#  -------------------------------------------------------------------------


#  -------------------------------------------------------------------------
# Dear ImGui
message(STATUS "⬇️  Fetching Dear ImGui...")
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG        docking
)
FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(imgui PUBLIC glfw OpenGL::GL)

message(STATUS "✅ Dear ImGui is ready!")
#  -------------------------------------------------------------------------


#  -------------------------------------------------------------------------
# NativeFileDialog
message(STATUS "⬇️  Fetching Native File Dialog...")

FetchContent_Declare(
  nativefiledialog
  GIT_REPOSITORY https://github.com/mlabbe/nativefiledialog.git
  GIT_TAG        master
)
if(NOT nativefiledialog_POPULATED)
  message(STATUS "🔽 Downloading NativeFileDialog from GitHub...")
endif()
FetchContent_MakeAvailable(nativefiledialog)

if(WIN32)
  set(NFD_SRC
    ${nativefiledialog_SOURCE_DIR}/src/nfd_common.c
    ${nativefiledialog_SOURCE_DIR}/src/nfd_win.cpp
  )
elseif(APPLE)
  set(NFD_SRC
    ${nativefiledialog_SOURCE_DIR}/src/nfd_common.c
    ${nativefiledialog_SOURCE_DIR}/src/nfd_cocoa.m
  )
elseif(UNIX)
  set(NFD_SRC
    ${nativefiledialog_SOURCE_DIR}/src/nfd_common.c
    ${nativefiledialog_SOURCE_DIR}/src/nfd_gtk.c
  )
endif()

add_library(nfd STATIC ${NFD_SRC})
target_include_directories(nfd PUBLIC
    ${nativefiledialog_SOURCE_DIR}/src/include
)

if(UNIX AND NOT APPLE)
    target_link_libraries(nfd PUBLIC ${GTK3_LIBRARIES})
    target_include_directories(nfd PUBLIC ${GTK3_INCLUDE_DIRS})
endif()

target_compile_definitions(nfd PRIVATE _CRT_SECURE_NO_WARNINGS)

message(STATUS "✅ Native File Dialog is ready!")
#  -------------------------------------------------------------------------


#  -------------------------------------------------------------------------
# stb_image implementation flags
add_compile_definitions(
    STB_IMAGE_IMPLEMENTATION
    STB_IMAGE_WRITE_IMPLEMENTATION
)


#  -------------------------------------------------------------------------
# Executable
add_executable(ORMTool
    src/main.cpp

    src/App/App.h
    src/App/App.cpp

    src/MVC/IView.h
    src/MVC/IController.h
    src/MVC/IModel.h
    src/MVC/BoilerplateMacro.h

    src/UI/UIManager.cpp
    src/UI/UIManager.h
    src/UI/ImNeo.h

    src/IO/IOService.cpp
    src/IO/IOService.h

    src/Utils/Constants.h
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src PREFIX "Source" FILES
    src/main.cpp

    src/App/App.h
    src/App/App.cpp

    src/MVC/IView.h
    src/MVC/IController.h
    src/MVC/IModel.h
    src/MVC/BoilerplateMacro.h

    src/UI/UIManager.cpp
    src/UI/UIManager.h
    src/UI/ImNeo.h

    src/IO/IOService.cpp
    src/IO/IOService.h

    src/Utils/Constants.h
    src/Utils/Types.h
)

#  -------------------------------------------------------------------------
# Include directories
target_include_directories(ORMTool PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/stb
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MVC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IO
    ${CMAKE_CURRENT_SOURCE_DIR}/src/App
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UI
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Utils

    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${nativefiledialog_SOURCE_DIR}/src/include
)

if(UNIX AND NOT APPLE)
    target_include_directories(ORMTool PRIVATE ${GTK3_INCLUDE_DIRS})
endif()


#  -------------------------------------------------------------------------
# Link libraries
find_package(OpenGL REQUIRED)

if(UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
    find_package(Threads REQUIRED)

    target_link_libraries(ORMTool PRIVATE
        imgui
        glfw
        OpenGL::GL
        nfd
        # X11 библиотеки
        ${X11_LIBRARIES}
        X11
        Xrandr
        Xi
        Xcursor
        Xinerama
        Xxf86vm
        Threads::Threads
        dl
        pthread
        m
        ${GTK3_LIBRARIES}
    )

    message(STATUS "🐧 Platform: Linux")
    message(STATUS "📚 X11 libraries: ${X11_LIBRARIES}")
    message(STATUS "🎨 GTK3 libraries: ${GTK3_LIBRARIES}")

elseif(WIN32)
    target_link_libraries(ORMTool PRIVATE
        imgui
        glfw
        OpenGL::GL
        nfd
    )
    message(STATUS "🖥  Platform: Windows")

elseif(APPLE)
    target_link_libraries(ORMTool PRIVATE
        imgui
        glfw
        OpenGL::GL
        nfd
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreFoundation"
        "-framework CoreVideo"
    )
    message(STATUS "🍎 Platform: macOS")
endif()


#  -------------------------------------------------------------------------
# Set default startup project in Visual Studio
if(MSVC)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ORMTool)
endif()


message(STATUS "🚀 Configuring ORMTool project")
message(STATUS "🧠 Using C++ standard: ${CMAKE_CXX_STANDARD}")

# ----------------------------------------------------------------------------
# 🔧 Compiler warnings
# ----------------------------------------------------------------------------
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    message(STATUS "⚠️  Enabling GCC/Clang warnings")
    add_compile_options(-Wall -Wextra -Wpedantic -Wno-unused-parameter)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    message(STATUS "⚠️  Enabling MSVC warnings")
    add_compile_options(/W4 /permissive- /D_CRT_SECURE_NO_WARNINGS)
endif()

message(STATUS "🎉 ✅ All dependencies are ready. ORMTool is ready to build!")
