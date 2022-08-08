# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

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
