TEMPLATE = subdirs

SUBDIRS = \
    afterrequest \
    simple

qtHaveModule(gui): qtHaveModule(concurrent) {
    SUBDIRS += colorpalette
}
