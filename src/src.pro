TEMPLATE = subdirs
CONFIG += ordered

isEmpty(PARTS) {
    SUBDIRS += protocol interpreter remote testapp objectBrowser
} else {
    # done this way since order matters
    contains(PARTS, protocol):SUBDIRS += protocol
    contains(PARTS, interpreter):SUBDIRS += interpreter
    contains(PARTS, remote):SUBDIRS += remote
    contains(PARTS, testapp):SUBDIRS += testapp
    contains(PARTS, objectBrowser):SUBDIRS += objectBrowser
}
