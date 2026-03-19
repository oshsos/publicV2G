//#include <cstring>
#include "main.h"

#include "../LibCpp/HAL/Tools.h"
#include "../LibCpp/HAL/HW_Tools.h"
#include "../LibCpp/HAL/Windows/cWinTcpServer.h"
#include "../LibCpp/HAL/Windows/cWinTcpClient.h"

#include "../LibIso15118/Codico/cCodicoPilot.h"
#include "../LibIso15118/PlcStack/cSlacProcessSECCPub.h"
#include "../LibIso15118/cExiIso15118_2013_openV2g.h"
#include "../LibIso15118/cIsoProcessSECC.h"
#include "../LibIso15118/IsoStackCtrlProtocol.h"

#include "../LibCpp/Lab/cEAsource.h"
//#include "../Hardware/cSePowerSource.h"

#include "../LibCpp/HAL/Windows/cAdvantech.h"
#include "../Hardware/cSeAcHardware.h"

void printHelpSECC()
{
    printf_flush("'q' to finish\n");
    printf_flush("'h' to help\n");
    printf_flush("'r' to reset from out of operation\n");
    //printf_flush("'s' to friendly stop charging\n");
}

//cSePowerSource seHardware(&powerSource);

cAdvantech advantechIO;
cSuperwisedOutput mainContactor;
cSeAcHardware seHardware(&mainContactor);

cWinTcpServer server;
cSlacProcessSECCPub slac(&plc);
cIsoProcessSECC isoProcessSECC(&codecIso, &udp, &server, config.schemaId, &controlPilot, &seHardware);

cConnectedClient* pClient = nullptr;

cTimer stateBtimer;
cTimer stateSendTimer(100, true);
cWinUdpSocket udpCtrl;

bool slacReady = false;

int mainSECC()
{

    dbg.setInstanceName("mainSECC");
    dbg.setDebugLevel(LibCpp::enDebugLevel_Debug);

    printf_flush("KEA Wired Power SECC Test Charger\n");
    printHelpSECC();

    packetSocket.dbg.setInstanceName("packetSocket", &dbg, enDebugLevel_Info);
    plc.dbg.setInstanceName("plc", &dbg, enDebugLevel_Info);
    slac.dbg.setInstanceName("slac", &dbg, enDebugLevel_Info);
    controlPilot.dbg.setInstanceName("controlPilot", &dbg, enDebugLevel_Info);
    udp.dbg.setInstanceName("udp", &dbg, enDebugLevel_Info);
    server.dbg.setInstanceName("server", &dbg, enDebugLevel_Info);
    codecIso.dbg.setInstanceName("codecIso", &dbg, enDebugLevel_Info);
    isoProcessSECC.dbg.setInstanceName("isoProcess", &dbg, enDebugLevel_Info);
    clientPower.dbg.setInstanceName("clientPower", &dbg, enDebugLevel_Info);
    powerSource.dbg.setInstanceName("powerSource", &dbg, enDebugLevel_Info);
    udpCtrl.dbg.setInstanceName("udpCtrl", &dbg, enDebugLevel_Info);

    enOperationState operationState = enOperationState::initialization;
    enOperationState newOperationState;
    bool enteredOperationState = false;
    dbg.printf(enDebugLevel_Info, "\n******** Operation state: %s ********", enOperationState_toString(operationState).c_str());

    //server.open(55100, "fe80::50b7:e175:9ea7:c233");
    //port.open(15118, "fe80::50b7:e175:9ea7:c233");
    if (udp.open(15118, config.interfaceName, LibCpp::enAddressFamily_IPv6) != LibCpp::enIpResult_Success)
    {
        cInterfaces interfaces(true);
        printf("Opening '%s' failed! Available interfaces are:\n%s\n", config.interfaceName.c_str(), interfaces.toString().c_str());
        interfaces.close();
    }
    //server.open(55100, config.interfaceName, LibCpp::enAddressFamily_IPv6);
    // For TLS operation the TCP server is required to be configured in non automatic opening mode of connected client instances.
    server.open(55100, config.interfaceName, LibCpp::enAddressFamily_IPv6, true, 0, false);
    packetSocket.open(config.interfaceName);

    if (config.hardware == enHardwareType::ADVANTECH && (config.transferMode == iso1EnergyTransferModeType_AC_three_phase_core || config.transferMode == iso1EnergyTransferModeType_AC_single_phase_core))
    {
        advantechIO.open();
        mainContactor.setOutput(&advantechIO.gpio(0x00));
    }
    else if (config.hardware == enHardwareType::EA_SOURCE && (config.transferMode == iso1EnergyTransferModeType_DC_combo_core || config.transferMode == iso1EnergyTransferModeType_DC_extended || config.transferMode == iso1EnergyTransferModeType_DC_core))
    {
        //powerSource.presetDemand = false;
        powerSource.presetNominalPower = false;
        powerSource.openTcpClient(&clientPower, config.eaSourceIp);
    }

    //packetSocket.setFilter();
    isoProcessSECC.seccInfo.seccId.set("XX00000", 7);
    isoProcessSECC.seccInfo.energyTransferModes.clear();
    isoProcessSECC.seccInfo.energyTransferModes.push_back(config.transferMode);
    isoProcessSECC.seccInfo.nominalVoltage = 400;
    isoProcessSECC.seccInfo.maxVoltage = 500;
    isoProcessSECC.seccInfo.maxCurrent = 16;
    isoProcessSECC.seccInfo.maxPower = 10000;
    isoProcessSECC.seccInfo.schemaId = config.schemaId;
    isoProcessSECC.provideTls = config.provideTls;

    isoProcessSECC.setServerIpAddress(server.getIpAddressLocal());
    memcpy(isoProcessSECC.seccInfo.macAddress, packetSocket.getMacAddressLocal(), ETHER_ADDR_LEN);

    isoProcessSECC.setupCommunication();

    udpCtrl.open(55201);
    udpCtrl.setDestination("127.0.0.1", 55200);

    dbg.printf(LibCpp::enDebugLevel_Info, "\n################################\n%s################################", isoProcessSECC.seccInfo.toString().c_str());

    while (1)
    {

        newOperationState = operationState;
        if (enteredOperationState)
            dbg.printf(enDebugLevel_Info, "\n******** Operation state: %s ********", enOperationState_toString(operationState).c_str());

        packetSocket.operate();
        udp.operate();
        clientPower.operate();
        controlPilot.operate();
        powerSource.operate();

        if (operationState == enOperationState::initialization)
        {
            if (isoProcessSECC.isInitialized() && slac.isInitialized())
            {
                newOperationState = enOperationState::plug;
            }
        }

        if (operationState == enOperationState::plug)
        {
            if (enteredOperationState)
            {
                controlPilot.setPwm(enCpPwm::Pwm_Off);
                //isoProcessSECC.clear();
            }
            if (controlPilot.getState() == enCpState::B_Connected)
                newOperationState = enOperationState::stateBtimer;
        }

        if (operationState == enOperationState::stateBtimer)
        {
            slacReady = false;
            if (enteredOperationState)
                stateBtimer.start(200);
            if (stateBtimer.elapsed())
                newOperationState = enOperationState::slac;
        }

        if (slac.operate(operationState==enOperationState::slac && !slacReady))
        {
            for (int i=0; i<ETHER_ADDR_LEN; i++)
                isoProcessSECC.evccInfo.macAddress[i] = slac.pPlcStack->values.PEV_MAC[i];
            slacReady = true;
            newOperationState = enOperationState::communication;
        }
        if (operationState == enOperationState::slac)
        {
            if (enteredOperationState)
                controlPilot.setPwm(enCpPwm::Pwm_Iso15118);
        }

        if (isoProcessSECC.operate(operationState==enOperationState::communication))
        {
            // newOperationState = enOperationState::processStopped;    // Holds the charging process inoperational until the plug is disconnected
            newOperationState = enOperationState::plug;                 // Restarts the charging process regardless of the plug still being plugged or not
        }

        if (controlPilot.getState() != enCpState::B_Connected && controlPilot.getState() != enCpState::C_Ready && controlPilot.getState() != enCpState::D_ReadyVentilation && operationState != enOperationState::initialization)
        {
            newOperationState = enOperationState::plug;
        }

        if (operationState == enOperationState::processStopped)
        {
            if (enteredOperationState)
                controlPilot.setPwm(enCpPwm::Pwm_Off);
            if (controlPilot.getState() == enCpState::A_Open)
                newOperationState = enOperationState::plug;
        }

        enteredOperationState = (newOperationState != operationState);
        operationState = newOperationState;

        // Stopping process for real vehicle tests
        // if (isoProcessSECC.seccOutput.v2gState >= Iso15118::enV2gSeState::answeredPowerDelivery)
        //    break;

        // Sending Ethernet controll state message
        if (stateSendTimer.elapsed())
        {
            stateSendTimer.start();
            stIsoStackState stateSecc;
            stateSecc.current = (int16_t)(isoProcessSECC.seccOutput.actualCurrent * 10);
            stateSecc.voltage = (int16_t)(isoProcessSECC.seccOutput.actualVoltage * 10);
            isoProcessSECC.seccInfo.seccId.asChar((char*)stateSecc.seccId, 7);
            isoProcessSECC.evccInfo.evccId.asChar((char*)stateSecc.evccId, 8);
            for(int i=0; i<ETHER_ADDR_LEN; i++) stateSecc.evccMac[i] = plc.values.PEV_MAC[i];
            stateSecc.energyRequest = isoProcessSECC.evccInfo.energyRequest / 100;
            stateSecc.energyCapacity = isoProcessSECC.evccInfo.energyCapacity / 100;
            stateSecc.departureTime = isoProcessSECC.evccInfo.departureTime;
            switch (isoProcessSECC.evccInfo.energyTransferMode)
            {
            case iso1EnergyTransferModeType_AC_single_phase_core:
                stateSecc.supplyPhases = enSupplyPhases::ac1;
                break;
            case iso1EnergyTransferModeType_AC_three_phase_core:
                stateSecc.supplyPhases = enSupplyPhases::ac3;
                break;
            case iso1EnergyTransferModeType_DC_core:
            case iso1EnergyTransferModeType_DC_extended:
            case iso1EnergyTransferModeType_DC_combo_core:
            case iso1EnergyTransferModeType_DC_unique:
                stateSecc.supplyPhases = enSupplyPhases::dc;
                break;
            }

            // Setting the HMI relevant state 'enIsoChargingState'
            switch (operationState)
            {
            case enOperationState::initialization:
                stateSecc.state = enIsoChargingState::off;
                break;
            case enOperationState::plug:
                stateSecc.state = enIsoChargingState::idle;
                break;
            case enOperationState::stateBtimer:
                stateSecc.state = enIsoChargingState::connected;
                break;
            case enOperationState::slac:
                stateSecc.state = enIsoChargingState::connected;
                break;
            case enOperationState::communication:
                switch (isoProcessSECC.state())
                {
                case enV2gSeState::idle:
                case enV2gSeState::awaitSdp:
                case enV2gSeState::answeredSdp:
                case enV2gSeState::answeredSupportedAppProtocol:
                case enV2gSeState::answeredSessionSetup:
                case enV2gSeState::answeredServiceDiscovery:
                case enV2gSeState::answeredPaymentServiceSelection:
                case enV2gSeState::answeredServiceDetail:
                    stateSecc.state = enIsoChargingState::connected;
                    break;
                case enV2gSeState::answeredAuthorization:
                    stateSecc.state = enIsoChargingState::identification;
                    break;
                case enV2gSeState::answeredPowerDelivery:
                case enV2gSeState::answeredCableCheck:
                case enV2gSeState::answeredPreCharge:
                case enV2gSeState::answeredWeldingDetection:
                    stateSecc.state = enIsoChargingState::preparation;
                    break;
                case enV2gSeState::answeredChargingStatus:
                case enV2gSeState::answeredCurrentDemand:
                    stateSecc.state = enIsoChargingState::charging;
                    break;
                case enV2gSeState::finished:
                    stateSecc.state = enIsoChargingState::finished;
                    break;
                default:
                    stateSecc.state = enIsoChargingState::off;
                }
                break;
            case enOperationState::processStopped:
                stateSecc.state = enIsoChargingState::finished;
                break;
            case enOperationState::outOfOperation:
                stateSecc.state = enIsoChargingState::error;
                break;
            }

            stIsoStackState msg = stateSecc.bigEndian();
            udpCtrl.send((char*)&msg, sizeof(stIsoStackState));
        }

        if (console.hasKey())
        {
            char input = console.getKey();
            printf_flush("\n");
            if (input == 'q')
                break;
            if (input == 'h')
            {
                printHelpSECC();
            }
            if (input == 'r')
            {
                operationState = enOperationState::plug;
                enteredOperationState = true;
            }
            if (input == 's')
            {
            }
        }
        LibCpp::sleep(10);
    }

    if (pClient) delete pClient;

    controlPilot.stopService();
    packetSocket.close();
    udp.close();
    server.close();
    clientPower.close();

    printf_flush("Ready\n");
    return 0;
}
