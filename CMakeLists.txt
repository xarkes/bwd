cmake_minimum_required(VERSION 3.16)

project(bwd VERSION 0.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)
qt_standard_project_setup()

qt_add_executable(bwd
    Source/Main.cpp
    Source/MainWindow.cpp
    Source/BWNetworkService.cpp
    Source/EncryptedString.cpp

    Source/Views/LoginWidget.cpp
    Source/Views/VaultUnlockWidget.cpp
    Source/Views/VaultWidget.cpp

    Source/Components/BWCategory.cpp
    Source/Components/BWCategoryEntry.cpp
    Source/Components/BWEntry.cpp
    Source/Components/BWField.cpp
    Source/Components/BWLineEdit.cpp
    Source/Components/BWListWidget.cpp
    Source/Components/BWBusyIndicator.cpp

    Source/ThirdParty/aes.cc
)

qt_add_resources(bwd
    PREFIX "/"
    BASE "Resources"
    FILES
        "Resources/Images/logo-dark@2x.png"
        "Resources/Images/hide.png"
        "Resources/Images/globe.png"
        "Resources/Images/credit-card.png"
        "Resources/Images/identity.png"
        "Resources/Images/note.png"
)

set(CMAKE_CXX_FLAGS "-Wextra -Wall")
target_link_libraries(bwd PRIVATE Qt6::Widgets Qt6::Network)
target_include_directories(bwd PRIVATE Source)

set_target_properties(bwd PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
