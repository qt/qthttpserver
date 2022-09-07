requires(qtHaveModule(httpserver))
requires(qtHaveModule(gui))
requires(qtHaveModule(concurrent))

TEMPLATE = app

QT += httpserver gui concurrent

SOURCES += \
    main.cpp

target.path = $$[QT_INSTALL_EXAMPLES]/httpserver/colorpalette
INSTALLS += target

RESOURCES += \
    assets.qrc

CONFIG += cmdline
