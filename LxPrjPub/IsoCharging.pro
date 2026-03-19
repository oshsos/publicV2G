TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += ../LibCpp/HAL/Tls

SOURCES += \
    ../Hardware/cEvPowerSource.cpp \
    ../Hardware/cSePowerSource.cpp \
    ../LibCpp/HAL/cConsoleInput.cpp \
    ../LibCpp/LibObCommon/LibOb_strptime.c \
    ../LibCpp/Time/cTimeStd.cpp \
    ../LibCpp/cTimer.cpp \
    ../LibCpp/HAL/Tls/cOpenSSL.cpp \
    ../LibCpp/HAL/Tls/cTls.cpp \
    ../LibCpp/HAL/Tls/cTlsConnectedClient.cpp \
    ../LibCpp/HAL/Tools.cpp \
    ../LibCpp/HAL/Linux/conio.cpp \
    ../LibCpp/HAL/cAdc.cpp \
    ../LibCpp/HAL/cDebug.cpp \
    ../LibCpp/HAL/cFramePort.cpp \
    ../LibCpp/HAL/cIp.cpp \
    ../LibCpp/HAL/cOutput.cpp \
    ../LibCpp/HAL/cSuperwisedOutput.cpp \
    ../LibCpp/HAL/cTcpServer.cpp \
    ../LibCpp/HAL/cTcpClient.cpp \
    ../LibCpp/HAL/cUdpSocket.cpp \
    ../LibCpp/HAL/cPacketSocket.cpp \
    ../LibCpp/HAL/cUpdate.cpp \
    ../LibCpp/HAL/Linux/HW_Tools.cpp \
    ../LibCpp/HAL/Linux/cLxConsoleInput.cpp \
    ../LibCpp/HAL/Linux/cLxIp.cpp \
    ../LibCpp/HAL/Linux/cLxTcpServer.cpp \
    ../LibCpp/HAL/Linux/cLxTcpClient.cpp \
    ../LibCpp/HAL/Linux/cLxUdpSocket.cpp \
    ../LibCpp/HAL/Linux/cLxPacketSocket.cpp \
    ../LibCpp/Lab/cEAsource.cpp \
    ../LibCpp/Lab/cPowerSource.cpp \
    ../LibCpp/Xml/XMLObjectDocument.cpp \
    ../LibCpp/Xml/tinyxml2.cpp \
    ../LibIso15118/IsoStackCtrlProtocol.cpp \
    ../LibIso15118/PlcStack/cSlacPub.cpp \
    ../LibIso15118/cEvHardware.cpp \
    ../LibIso15118/cExiCodec.cpp \
    ../LibIso15118/cExiDin70121openV2g.cpp \
    ../LibIso15118/cExiIso15118_2013_openV2g.cpp \
    ../LibIso15118/cSeHardware.cpp \
    ../LibIso15118/cIsoProcessEVCC.cpp \
    ../LibIso15118/cIsoProcessSECC.cpp \
    ../LibIso15118/isoTypes.cpp \
    ../LibIso15118/cControlPilot.cpp \
    ../LibIso15118/PlcStack/cPlcStackPub.cpp \
    ../LibIso15118/PlcStack/cSlacProcessSECCPub.cpp \
    ../LibIso15118/PlcStack/cSlacProcessEVCCPub.cpp \
    ../LibIso15118/PlcStack/tools/hexencode.c \
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
    ../LibIso15118/Codico/cCodicoPilot.cpp \
    main.cpp \
    mainEVCC.cpp \
    mainSECC.cpp

HEADERS += \
    ../Hardware/cEvPowerSource.h \
    ../Hardware/cSePowerSource.h \
    ../LibCpp/HAL/cConsoleInput.h \
    ../LibCpp/LibObCommon/LibOb_strptime.h \
    ../LibCpp/Time/cTime.h \
    ../LibCpp/cTimer.h \
    ../LibCpp/HAL/Tls/cOpenSSL.h \
    ../LibCpp/HAL/Tls/cTls.h \
    ../LibCpp/HAL/Tls/cTlsConnectedClient.h \
    ../LibCpp/HAL/Tools.h \
    ../LibCpp/HAL/Linux/conio.h \
    ../LibCpp/HAL/cAdc.h \
    ../LibCpp/HAL/cDebug.h \
    ../LibCpp/HAL/cFramePort.h \
    ../LibCpp/HAL/cIp.h \
    ../LibCpp/HAL/cOutput.h \
    ../LibCpp/HAL/cSuperwisedOutput.h \
    ../LibCpp/HAL/cTcpServer.h \
    ../LibCpp/HAL/cTcpClient.h \
    ../LibCpp/HAL/cUdpSocket.h \
    ../LibCpp/HAL/cPacketSocket.h \
    ../LibCpp/HAL/cUpdate.h \
    ../LibCpp/HAL/Linux/HW_Tools.h \
    ../LibCpp/HAL/Linux/cLxConsoleInput.h \
    ../LibCpp/HAL/Linux/cLxIp.h \
    ../LibCpp/HAL/Linux/cLxTcpServer.h \
    ../LibCpp/HAL/Linux/cLxTcpClient.h \
    ../LibCpp/HAL/Linux/cLxUdpSocket.h \
    ../LibCpp/HAL/Linux/cLxPacketSocket.h \
    ../LibCpp/Lab/cEAsource.h \
    ../LibCpp/Lab/cPowerSource.h \
    ../LibCpp/Xml/XMLObjectDocument.h \
    ../LibCpp/Xml/tinyxml2.h \
    ../LibIso15118/IsoStackCtrlProtocol.h \
    ../LibIso15118/PlcStack/cQcaMAC.h \
    ../LibIso15118/PlcStack/cSlacPub.h \
    ../LibIso15118/PlcStack/slac_messages.h \
    ../LibIso15118/cEvHardware.h \
    ../LibIso15118/cExiCodec.h \
    ../LibIso15118/cExiDin70121openV2g.h \
    ../LibIso15118/cExiIso15118_2013_openV2g.h \
    ../LibIso15118/cSeHardware.h \
    ../LibIso15118/cIsoProcessEVCC.h \
    ../LibIso15118/cIsoProcessSECC.h \
    ../LibIso15118/isoTypes.h \
    ../LibIso15118/cControlPilot.h \
    ../LibIso15118/PlcStack/cPlcStackPub.h \
    ../LibIso15118/PlcStack/cSlacProcessSECCPub.h \
    ../LibIso15118/PlcStack/cSlacProcessEVCCPub.h \
    ../LibIso15118/PlcStack/tools/hexencode.h \
     ../LibIso15118/openV2G/appHandshake/appHandEXIDatatypes.h \
    ../LibIso15118/openV2G/appHandshake/appHandEXIDatatypesDecoder.h \
    ../LibIso15118/openV2G/appHandshake/appHandEXIDatatypesEncoder.h \
    ../LibIso15118/openV2G/codec/BitInputStream.h \
    ../LibIso15118/openV2G/codec/BitOutputStream.h \
    ../LibIso15118/openV2G/codec/ByteStream.h \
    ../LibIso15118/openV2G/codec/DecoderChannel.h \
    ../LibIso15118/openV2G/codec/EXIHeaderDecoder.h \
    ../LibIso15118/openV2G/codec/EXIHeaderEncoder.h \
    ../LibIso15118/openV2G/codec/EncoderChannel.h \
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
    ../LibIso15118/Codico/cCodicoPilot.h \
    main.h

QMAKE_CXXFLAGS += -pthread
QMAKE_CFLAGS += -pthread

LIBS += -pthread
LIBS += -L/usr/local/lib64/openssl
LIBS += -Lbuild
LIBS += -L.
LIBS += -lcrypto
LIBS += -lssl

