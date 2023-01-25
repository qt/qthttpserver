requires(qtHaveModule(httpserver))

TEMPLATE = app

QT = httpserver
android: QT += gui

SOURCES += \
    main.cpp

target.path = $$[QT_INSTALL_EXAMPLES]/httpserver/afterrequest
INSTALLS += target

CONFIG += cmdline
