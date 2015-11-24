cmake_minimum_required(VERSION 3.3)
project(MissingHUD2)

# Set mingw32 base directory
set(MINGW32_BASE_DIR D:/Programming/MSYS2/mingw32)

# Set output directories
if (${CMAKE_BUILD_TYPE} MATCHES Debug)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin/Debug")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin/Debug")
else()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin/Release")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin/Release")
endif()

# Global C++ flags and defines
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
add_definitions(-DWINVER=0x0601 -D_WIN32_WINNT=0x0601)  # Target Windows 7
add_definitions(-DBOOST_DATE_TIME_NO_LIB -DBOOST_INTERPROCESS_BOOTSTAMP_IS_LASTBOOTUPTIME)  # Boost options

# Strip targets if Release build
if (${CMAKE_BUILD_TYPE} MATCHES Release)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s")
endif()

# Generate boost::interprocess protobuf's
find_package(Protobuf REQUIRED)
set(PROTO_SRC_FILES
    src/proto/mhud2.proto
)
PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS ${PROTO_SRC_FILES})

# Build the Hook DLL
set(CMAKE_SHARED_MODULE_PREFIX "")
set(DLL_SOURCE_FILES
    src/dll/DllMain.cpp
    src/MHUD_MsgQueue.h src/MHUD_MsgQueue.cpp
    src/dll/IATHook.h src/dll/IATHook.cpp
    src/dll/GLStructs.h src/dll/GLStructs.cpp
    src/dll/GDISwapBuffers.h src/dll/GDISwapBuffers.cpp

    src/dll/IsaacMemSignatures.h
    src/dll/MemReader.h src/dll/MemReader.cpp
    src/dll/AfterbirthMemReader.h src/dll/AfterbirthMemReader.cpp
    src/dll/RebirthMemReader.h src/dll/RebirthMemReader.cpp
    src/dll/HUDOverlay.h src/dll/HUDOverlay.cpp
    src/dll/HUDStat.h src/dll/HUDStat.cpp

    src/dll/SpriteSheet.h src/dll/SpriteSheet.cpp
    src/dll/TextRenderer.h src/dll/TextRenderer.cpp
    src/dll/ShaderProgram.h src/dll/ShaderProgram.cpp
    src/dll/ResourceLoader.h src/dll/ResourceLoader.cpp
    src/dll/DLLPreferences.h src/dll/DLLPreferences.cpp
    res/DllResources.h res/DllResources.rc
)
add_library(MissingHUD2Hook MODULE ${DLL_SOURCE_FILES} ${PROTO_SRCS} ${PROTO_HDRS})

# We link all dep's statically so we don't have to inject them along with the DLL
set_target_properties(MissingHUD2Hook PROPERTIES LINK_FLAGS "-static")
target_compile_definitions(MissingHUD2Hook PRIVATE "-DGLEW_STATIC")
target_link_libraries(MissingHUD2Hook
    glew32
    soil2
    OpenGL32
    shlwapi
    protobuf-lite
)

# Qt5 settings
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Set Qt5 to static / dynamic based on buildtype
unset(CMAKE_PREFIX_PATH CACHE)
if (${CMAKE_BUILD_TYPE} MATCHES Release)
    set(CMAKE_PREFIX_PATH "${MINGW32_BASE_DIR}/qt5-static/lib/cmake ${CMAKE_PREFIX_PATH}")
endif()

# Find Qt libraries
unset(Qt5Core_DIR CACHE)
unset(Qt5Gui_DIR CACHE)
unset(Qt5Widgets_DIR CACHE)
unset(Qt5Network_DIR CACHE)
unset(Qt5WinExtras_DIR CACHE)
find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5WinExtras REQUIRED)

# Build the Loader application
set(SOURCE_FILES
    src/MissingHUD2.cpp

    src/LoaderGUI.h src/LoaderGUI.cpp src/LoaderGUI.ui
    src/BoIInjector.h src/BoIInjector.cpp
    src/BoIProcess.h src/BoIProcess.cpp
    src/MHUD_MsgQueue.h src/MHUD_MsgQueue.cpp
    src/MHUD_Options.h src/MHUD_Options.cpp
    src/mhud2_version.h

    res/MissingHUD2.qrc
    res/WinResources.h res/WinResource.rc
)
add_executable(MissingHUD2 WIN32 ${SOURCE_FILES} ${PROTO_SRCS} ${PROTO_HDRS})
target_compile_definitions(MissingHUD2 PRIVATE "-DELPP_THREAD_SAFE -DELPP_NO_DEFAULT_LOG_FILE")
target_link_libraries(MissingHUD2
    Qt5::Core Qt5::Widgets Qt5::Network Qt5::WinExtras
    shlwapi
    protobuf-lite
)

# Set Qt5 to static as well as GCC C++ libs if Release buildtype
if (${CMAKE_BUILD_TYPE} MATCHES Release)
    target_compile_definitions(MissingHUD2 PRIVATE "-DQT_STATIC")
    set_target_properties(MissingHUD2 PROPERTIES LINK_FLAGS "-static")
endif()