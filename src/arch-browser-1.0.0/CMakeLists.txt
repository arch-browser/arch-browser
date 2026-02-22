# Arch Linux Native Browser - Chromium-based web browser
# Requires: Qt5/Qt6, QtWebEngine (Chromium-based rendering)
cmake_minimum_required(VERSION 3.16)
project(arch-browser VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# Qt MOC/RCC/UIC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt5 with WebEngine (QtWebEngine embeds Chromium)
find_package(Qt5 REQUIRED COMPONENTS Widgets WebEngineWidgets WebEngine Core Gui Network)

# Source files (headers included for MOC processing)
set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.hpp
    src/WebView.cpp
    src/WebView.hpp
    src/FindBar.cpp
    src/FindBar.hpp
)

# Executable
add_executable(arch-browser ${SOURCES})

# Link Qt libraries
target_link_libraries(arch-browser PRIVATE
    Qt5::Widgets
    Qt5::WebEngineWidgets
    Qt5::WebEngine
    Qt5::Core
    Qt5::Gui
    Qt5::Network
)

# Include directories
target_include_directories(arch-browser PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Install target
install(TARGETS arch-browser DESTINATION bin)

# Desktop entry for application launcher
install(FILES arch-browser.desktop DESTINATION share/applications)
