cmake_minimum_required(VERSION 3.16)
project(simple LANGUAGES CXX)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/httpserver/${PROJECT_NAME}")

find_package(Qt6 REQUIRED COMPONENTS HttpServer)

qt_add_executable(simple
    main.cpp
)

target_link_libraries(simple PUBLIC
    Qt::HttpServer
)

set(assets_resource_files
    "assets/qt-logo.png"
)

qt_add_resources(simple "assets"
    PREFIX
        "/"
    FILES
        ${assets_resource_files}
)

install(TARGETS simple
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
