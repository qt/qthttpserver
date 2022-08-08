# Generated from qhttpserverresponse.pro.

#####################################################################
## tst_qhttpserverresponse Test:
#####################################################################

# Collect test data
list(APPEND test_data "data/")

qt_internal_add_test(tst_qhttpserverresponse
    SOURCES
        tst_qhttpserverresponse.cpp
    PUBLIC_LIBRARIES
        Qt::HttpServer
    TESTDATA ${test_data}
)
