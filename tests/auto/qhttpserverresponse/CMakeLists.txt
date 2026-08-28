# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_qhttpserverresponse LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

# Generated from qhttpserverresponse.pro.

#####################################################################
## tst_qhttpserverresponse Test:
#####################################################################

# Collect test data
list(APPEND test_data "data/")

qt_internal_add_test(tst_qhttpserverresponse
    SOURCES
        tst_qhttpserverresponse.cpp
    LIBRARIES
        Qt::HttpServer
    TESTDATA ${test_data}
)
