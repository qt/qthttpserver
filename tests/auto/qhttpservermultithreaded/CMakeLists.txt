# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_qhttpservermultithreaded LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

#####################################################################
## tst_qhttpservermultithreaded Test:
#####################################################################

qt_internal_add_test(tst_qhttpservermultithreaded
    DEFINES
        QTEST_THROW_ON_FAIL
        QTEST_THROW_ON_SKIP
    SOURCES
        tst_qhttpservermultithreaded.cpp
    LIBRARIES
        Qt::HttpServer Qt::Concurrent Qt::TestPrivate
)
