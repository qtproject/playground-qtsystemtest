!contains(DEFINES, DEFAULT_AUT_PORT) {
    DEFINES += DEFAULT_AUT_PORT=6565
}

!contains(DEFINES, DEFAULT_AUTSSH_PORT) {
    DEFINES += DEFAULT_AUT_AUTPORT=22
}

!contains(DEFINES, DEFAULT_AUT_HOST) {
    DEFINES += DEFAULT_AUT_HOST="127.0.0.1"
}

!contains(DEFINES, QTUITEST_PORT_PATH) {
    DEFINES += 'QTUITEST_PORT_PATH=\\"/tmp/qtuitest_port\\"'
}
