# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_qhttpserver LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

# Generated from qhttpserver.pro.

#####################################################################
## tst_qhttpserver Test:
#####################################################################

# Collect test data
list(APPEND test_data "data/")

qt_internal_add_test(tst_qhttpserver
    SOURCES
        tst_qhttpserver.cpp
    LIBRARIES
        Qt::HttpServer
        Qt::TestPrivate
    TESTDATA ${test_data}
)

qt_internal_extend_target(tst_qhttpserver CONDITION QT_FEATURE_concurrent
    LIBRARIES
        Qt::Concurrent
)
