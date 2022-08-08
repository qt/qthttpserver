# Generated from qhttpserver.pro.

#####################################################################
## tst_qhttpserver Test:
#####################################################################

# Collect test data
list(APPEND test_data "data/")

qt_internal_add_test(tst_qhttpserver
    SOURCES
        tst_qhttpserver.cpp
    PUBLIC_LIBRARIES
        Qt::HttpServer
    TESTDATA ${test_data}
)

qt_internal_extend_target(tst_qhttpserver CONDITION QT_FEATURE_concurrent
    PUBLIC_LIBRARIES
        Qt::Concurrent
)
