# Generated from qthttpserver.pro.

cmake_minimum_required(VERSION 3.16)

include(.cmake.conf)
project(QtHttpServer
    VERSION "${QT_REPO_MODULE_VERSION}"
    DESCRIPTION "Qt HTTP Server"
    HOMEPAGE_URL "https://qt.io/"
    LANGUAGES CXX C
)

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals Core Network)
find_package(Qt6 ${PROJECT_VERSION} CONFIG OPTIONAL_COMPONENTS Concurrent WebSockets)

if(ANDROID)
    # Android tests needs to link against Qt6::Gui
    find_package(Qt6 ${PROJECT_VERSION} QUIET CONFIG OPTIONAL_COMPONENTS Gui)
endif()

qt_build_repo()
