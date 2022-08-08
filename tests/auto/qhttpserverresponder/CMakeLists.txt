# Generated from qhttpserverresponder.pro.

#####################################################################
## tst_qhttpserverresponder Test:
#####################################################################

# Collect test data
list(APPEND test_data "data/")

qt_internal_add_test(tst_qhttpserverresponder
    SOURCES
        tst_qhttpserverresponder.cpp
    PUBLIC_LIBRARIES
        Qt::HttpServer
    TESTDATA ${test_data}
)
