TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    ../LibCpp/HAL/Linux/conio.cpp \
    ../LibCpp/HAL/Tools.cpp \
    ../LibCpp/HAL/Linux/cLxConsoleInput.cpp \
    ../LibCpp/HAL/Linux/cLxTcpClient.cpp \
    ../LibCpp/HAL/cAdc.cpp \
    ../LibCpp/HAL/cDebug.cpp \
    ../LibCpp/HAL/cFramePort.cpp \
    ../LibCpp/HAL/cIp.cpp \
    ../LibCpp/HAL/cOutput.cpp \
    ../LibCpp/HAL/cSuperwisedOutput.cpp \
    ../LibCpp/HAL/cTcpClient.cpp \
    ../LibCpp/HAL/cTcpServer.cpp \
    ../LibCpp/HAL/cUdpSocket.cpp \
    ../LibCpp/HAL/Linux/HW_Tools.cpp \
    ../LibCpp/HAL/Linux/cLxIp.cpp \
    ../LibCpp/HAL/Linux/cLxTcpServer.cpp \
    ../LibCpp/HAL/Linux/cLxUdpSocket.cpp \
    ../LibCpp/HAL/cUpdate.cpp \
    ../LibCpp/Lab/cPowerSource.cpp \
    ../LibCpp/cTimer.cpp \
    ../LibIso15118/cControlPilot.cpp \
    ../LibIso15118/cExiDin70121openV2g.cpp \
    ../LibIso15118/cEvHardware.cpp \
    ../LibIso15118/cExiCodec.cpp \
    ../LibIso15118/cExiIso15118_2013_openV2g.cpp \
    ../LibIso15118/cIsoProcessEVCC.cpp \
    ../LibIso15118/cIsoProcessSECC.cpp \
    ../LibIso15118/cSeHardware.cpp \
    ../LibIso15118/isoTypes.cpp \
    ../LibIso15118/openV2G/appHandshake/appHandEXIDatatypes.c \
    ../LibIso15118/openV2G/appHandshake/appHandEXIDatatypesDecoder.c \
    ../LibIso15118/openV2G/appHandshake/appHandEXIDatatypesEncoder.c \
    ../LibIso15118/openV2G/codec/BitInputStream.c \
    ../LibIso15118/openV2G/codec/BitOutputStream.c \
    ../LibIso15118/openV2G/codec/ByteStream.c \
    ../LibIso15118/openV2G/codec/DecoderChannel.c \
    ../LibIso15118/openV2G/codec/EXIHeaderDecoder.c \
    ../LibIso15118/openV2G/codec/EXIHeaderEncoder.c \
    ../LibIso15118/openV2G/codec/EncoderChannel.c \
    ../LibIso15118/openV2G/codec/MethodsBag.c \
    ../LibIso15118/openV2G/din/dinEXIDatatypes.c \
    ../LibIso15118/openV2G/din/dinEXIDatatypesDecoder.c \
    ../LibIso15118/openV2G/din/dinEXIDatatypesEncoder.c \
    ../LibIso15118/openV2G/iso1/iso1EXIDatatypes.c \
    ../LibIso15118/openV2G/iso1/iso1EXIDatatypesDecoder.c \
    ../LibIso15118/openV2G/iso1/iso1EXIDatatypesEncoder.c \
    ../LibIso15118/openV2G/iso2/iso2EXIDatatypes.c \
    ../LibIso15118/openV2G/iso2/iso2EXIDatatypesDecoder.c \
    ../LibIso15118/openV2G/iso2/iso2EXIDatatypesEncoder.c \
    main.cpp \
    mainEVCC.cpp \
    mainSECC.cpp

HEADERS += \
    ../LibCpp/HAL/Linux/conio.h \
    ../LibCpp/HAL/Tools.h \
    ../LibCpp/HAL/HW_Tools.h \
    ../LibCpp/HAL/Linux/cLxConsoleInput.h \
    ../LibCpp/HAL/Linux/cLxTcpClient.h \
    ../LibCpp/HAL/cAdc.h \
    ../LibCpp/HAL/cDebug.h \
    ../LibCpp/HAL/cFramePort.h \
    ../LibCpp/HAL/cIp.h \
    ../LibCpp/HAL/cOutput.h \
    ../LibCpp/HAL/cSuperwisedOutput.h \
    ../LibCpp/HAL/cTcpClient.h \
    ../LibCpp/HAL/cTcpServer.h \
    ../LibCpp/HAL/cUdpSocket.h \
    ../LibCpp/HAL/Linux/cLxIp.h \
    ../LibCpp/HAL/Linux/cLxTcpServer.h \
    ../LibCpp/HAL/Linux/cLxUdpSocket.h \
    ../LibCpp/HAL/cUpdate.h \
    ../LibCpp/Lab/cPowerSource.h \
    ../LibCpp/cTimer.h \
    ../LibIso15118/cControlPilot.h \
    ../LibIso15118/cExiDin70121openV2g.h \
    ../LibIso15118/cEvHardware.h \
    ../LibIso15118/cExiCodec.h \
    ../LibIso15118/cExiIso15118_2013_openV2g.h \
    ../LibIso15118/cIsoProcessEVCC.h \
    ../LibIso15118/cIsoProcessSECC.h \
    ../LibIso15118/cSeHardware.h \
    ../LibIso15118/isoTypes.h \
    ../LibIso15118/openV2G/appHandshake/appHandEXIDatatypes.h \
    ../LibIso15118/openV2G/appHandshake/appHandEXIDatatypesDecoder.h \
    ../LibIso15118/openV2G/appHandshake/appHandEXIDatatypesEncoder.h \
    ../LibIso15118/openV2G/codec/BitInputStream.h \
    ../LibIso15118/openV2G/codec/BitOutputStream.h \
    ../LibIso15118/openV2G/codec/ByteStream.h \
    ../LibIso15118/openV2G/codec/DecoderChannel.h \
    ../LibIso15118/openV2G/codec/EXIConfig.h \
    ../LibIso15118/openV2G/codec/EXIHeaderDecoder.h \
    ../LibIso15118/openV2G/codec/EXIHeaderEncoder.h \
    ../LibIso15118/openV2G/codec/EXIOptions.h \
    ../LibIso15118/openV2G/codec/EXITypes.h \
    ../LibIso15118/openV2G/codec/EncoderChannel.h \
    ../LibIso15118/openV2G/codec/ErrorCodes.h \
    ../LibIso15118/openV2G/codec/MethodsBag.h \
    ../LibIso15118/openV2G/din/dinEXIDatatypes.h \
    ../LibIso15118/openV2G/din/dinEXIDatatypesDecoder.h \
    ../LibIso15118/openV2G/din/dinEXIDatatypesEncoder.h \
    ../LibIso15118/openV2G/iso1/iso1EXIDatatypes.h \
    ../LibIso15118/openV2G/iso1/iso1EXIDatatypesDecoder.h \
    ../LibIso15118/openV2G/iso1/iso1EXIDatatypesEncoder.h \
    ../LibIso15118/openV2G/iso2/iso2EXIDatatypes.h \
    ../LibIso15118/openV2G/iso2/iso2EXIDatatypesDecoder.h \
    ../LibIso15118/openV2G/iso2/iso2EXIDatatypesEncoder.h \
    ../LibIso15118/types.h \
    main.h

QMAKE_CXXFLAGS += -pthread
QMAKE_CFLAGS += -pthread

LIBS += -pthread

# Default rules for deployment.
#qnx: target.path = /tmp/$${TARGET}/bin
#else: unix:!android: target.path = /opt/$${TARGET}/bin
#!isEmpty(target.path): INSTALLS += target

#win32 {
#DESTDIR = $$PWD/bin
#QMAKE_POST_LINK =  windeployqt $$shell_path($$DESTDIR/$${TARGET}.exe)
#}
