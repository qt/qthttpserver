# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

cmake_minimum_required(VERSION 3.16)
project(qabstracthttpserver_test)

find_package(Qt5 COMPONENTS HttpServer REQUIRED)

add_executable(qabstracthttpserver_test "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
target_link_libraries(qabstracthttpserver_test Qt5::HttpServer)
