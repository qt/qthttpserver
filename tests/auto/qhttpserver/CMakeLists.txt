# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

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
        Qt::HttpServerPrivate
    TESTDATA ${test_data}
)

qt_internal_extend_target(tst_qhttpserver CONDITION QT_FEATURE_concurrent
    LIBRARIES
        Qt::Concurrent
)
