TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += ../LibCpp/HAL/Windows/WpdPack/Include
INCLUDEPATH += ../LibCpp/HAL/Windows/WpdPack/Include/pcap
INCLUDEPATH += ../LibCpp/HAL/Tls

SOURCES += \
    ../Hardware/cEvPowerSource.cpp \
    ../Hardware/cSeAcHardware.cpp \
    ../Hardware/cSePowerSource.cpp \
    ../LibCpp/HAL/Tls/cOpenSSL.cpp \
    ../LibCpp/HAL/Tls/cTls.cpp \
    ../LibCpp/HAL/Tls/cTlsConnectedClient.cpp \
    ../LibCpp/HAL/Tools.cpp \
    ../LibCpp/HAL/Windows/cAdvantech.cpp \
    ../LibCpp/HAL/Windows/cWinConsoleInput.cpp \
    ../LibCpp/HAL/Windows/cWinTcpClient.cpp \
    ../LibCpp/HAL/cAdc.cpp \
    ../LibCpp/HAL/cConsoleInput.cpp \
    ../LibCpp/HAL/cDac.cpp \
    ../LibCpp/HAL/cDebug.cpp \
    ../LibCpp/HAL/cFramePort.cpp \
    ../LibCpp/HAL/cGpio.cpp \
    ../LibCpp/HAL/cInput.cpp \
    ../LibCpp/HAL/cIp.cpp \
    ../LibCpp/HAL/cOutput.cpp \
    ../LibCpp/HAL/cSuperwisedOutput.cpp \
    ../LibCpp/HAL/cTcpClient.cpp \
    ../LibCpp/HAL/cTcpServer.cpp \
    ../LibCpp/HAL/cUdpSocket.cpp \
    ../LibCpp/HAL/cPacketSocket.cpp \
    ../LibCpp/HAL/Windows/HW_Tools.cpp \
    ../LibCpp/HAL/Windows/cWinIp.cpp \
    ../LibCpp/HAL/Windows/cWinTcpServer.cpp \
    ../LibCpp/HAL/Windows/cWinUdpSocket.cpp \
    ../LibCpp/HAL/Windows/cWinPacketSocket.cpp \
    ../LibCpp/HAL/cUpdate.cpp \
    ../LibCpp/Lab/cEAsource.cpp \
    ../LibCpp/Lab/cPowerSource.cpp \
    ../LibCpp/LibObCommon/LibOb_strptime.c \
    ../LibCpp/Time/cTimeStd.cpp \
    ../LibCpp/Xml/XMLObjectDocument.cpp \
    ../LibCpp/Xml/tinyxml2.cpp \
    ../LibCpp/cTimer.cpp \
    ../LibIso15118/IsoStackCtrlProtocol.cpp \
    ../LibIso15118/PlcStack/cSlacProcessSECCPub.cpp \
    ../LibIso15118/PlcStack/cSlacPub.cpp \
    ../LibIso15118/Support/Configuration.cpp \
    ../LibIso15118/cControlPilot.cpp \
    ../LibIso15118/cEvHardware.cpp \
    ../LibIso15118/cExiCodec.cpp \
    ../LibIso15118/cExiDin70121openV2g.cpp \
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
    ../LibIso15118/Codico/cCodicoPilot.cpp \
    ../LibIso15118/PlcStack/cPlcStackPub.cpp \
    ../LibIso15118/PlcStack/cSlacProcessEVCCPub.cpp \
    ../LibIso15118/PlcStack/tools/hexencode.c \
    main.cpp \
    mainEVCC.cpp \
    mainSECC.cpp

HEADERS += \
    ../Hardware/cEvPowerSource.h \
    ../Hardware/cSeAcHardware.h \
    ../Hardware/cSePowerSource.h \
    ../LibCpp/HAL/Tls/cOpenSSL.h \
    ../LibCpp/HAL/Tls/cTls.h \
    ../LibCpp/HAL/Tls/cTlsConnectedClient.h \
    ../LibCpp/HAL/Tools.h \
    ../LibCpp/HAL/HW_Tools.h \
    ../LibCpp/HAL/Windows/cAdvantech.h \
    ../LibCpp/HAL/Windows/cWinConsoleInput.h \
    ../LibCpp/HAL/Windows/cWinTcpClient.h \
    ../LibCpp/HAL/cAdc.h \
    ../LibCpp/HAL/cConsoleInput.h \
    ../LibCpp/HAL/cDac.h \
    ../LibCpp/HAL/cDebug.h \
    ../LibCpp/HAL/cFramePort.h \
    ../LibCpp/HAL/cGpio.h \
    ../LibCpp/HAL/cInput.h \
    ../LibCpp/HAL/cIp.h \
    ../LibCpp/HAL/cOutput.h \
    ../LibCpp/HAL/cSuperwisedOutput.h \
    ../LibCpp/HAL/cTcpClient.h \
    ../LibCpp/HAL/cTcpServer.h \
    ../LibCpp/HAL/cUdpSocket.h \
    ../LibCpp/HAL/cPacketSocket.h \
    ../LibCpp/HAL/Windows/cWinPacketSocket.h \
    ../LibCpp/HAL/Windows/cWinIp.h \
    ../LibCpp/HAL/Windows/cWinTcpServer.h \
    ../LibCpp/HAL/Windows/cWinUdpSocket.h \
    ../LibCpp/HAL/cUpdate.h \
    ../LibCpp/Lab/cEAsource.h \
    ../LibCpp/Lab/cPowerSource.h \
    ../LibCpp/LibObCommon/LibOb_strptime.h \
    ../LibCpp/Time/cTime.h \
    ../LibCpp/Xml/XMLObjectDocument.h \
    ../LibCpp/Xml/tinyxml2.h \
    ../LibCpp/cTimer.h \
    ../LibIso15118/IsoStackCtrlProtocol.h \
    ../LibIso15118/PlcStack/cSlacProcessSECCPub.h \
    ../LibIso15118/PlcStack/cSlacPub.h \
    ../LibIso15118/Support/Configuration.h \
    ../LibIso15118/cControlPilot.h \
    ../LibIso15118/cEvHardware.h \
    ../LibIso15118/cExiCodec.h \
    ../LibIso15118/cExiDin70121openV2g.h \
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
    ../LibIso15118/Codico/cCodicoPilot.h \
    ../LibIso15118/PlcStack/cSlacProcessEVCCPub.h \
    ../LibIso15118/PlcStack/cPlcStackPub.h \
    ../LibIso15118/PlcStack/slac_messages.h \
    ../LibIso15118/types.h \
    main.h

QMAKE_CXXFLAGS += -pthread
QMAKE_CFLAGS += -pthread

LIBS += -pthread
LIBS += $$PWD/../LibCpp/HAL/Windows/iphlpapi.lib
LIBS += $$PWD/../LibCpp/HAL/Windows/ws2_32.lib
LIBS += $$PWD/../LibCpp/HAL/Windows/WpdPack/Lib/X64/wpcap.lib
LIBS += $$PWD/../LibCpp/HAL/Windows/WpdPack/Lib/X64/packet.lib
LIBS += $$PWD/../LibCpp/HAL/Tls/libcrypto-3-x64.dll
LIBS += $$PWD/../LibCpp/HAL/Tls/libssl-3-x64.dll

# Default rules for deployment.
# qnx: target.path = /tmp/$${TARGET}/bin
# else: unix:!android: target.path = /opt/$${TARGET}/bin
# !isEmpty(target.path): INSTALLS += target

#win32 {
#DESTDIR = $$PWD/bin
#QMAKE_POST_LINK =  windeployqt $$shell_path($$DESTDIR/$${TARGET}.exe)
#}

DISTFILES += \
	../Dox/DoxIsoCharging.dox \
	../LibCpp/Dox/LibCpp.dox \
	../LibIso15118/Dox/LibIso15118.dox \
	build/debug/config.xml
