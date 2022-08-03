# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

# Generated from qhttpserverresponder.pro.

if(NOT QT_FEATURE_private_tests)
    return()
endif()

#####################################################################
## tst_qhttpserverresponder Test:
#####################################################################

# Collect test data
list(APPEND test_data "data/")

qt_internal_add_test(tst_qhttpserverresponder
    SOURCES
        tst_qhttpserverresponder.cpp
    LIBRARIES
        Qt::HttpServer
        Qt::HttpServerPrivate
    TESTDATA ${test_data}
)
