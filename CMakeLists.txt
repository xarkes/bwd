cmake_minimum_required(VERSION 3.16)

project(bwd VERSION 0.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

qt_add_executable(bwd
    Source/Main.cpp
    Source/MainWindow.cpp

    Source/LoginWidget.cpp
    Source/VaultUnlockWidget.cpp
    Source/VaultWidget.cpp

    Source/BWLineEdit.cpp
    Source/BWEntry.cpp
    Source/BWListWidget.cpp
    Source/BWCategory.cpp
    Source/BWCategoryEntry.cpp
)

qt_add_resources(bwd
    PREFIX "/"
    BASE "Resources"
    FILES
        "Resources/Images/logo-dark@2x.png"
)

target_link_libraries(bwd PRIVATE Qt6::Widgets)

set_target_properties(bwd PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
