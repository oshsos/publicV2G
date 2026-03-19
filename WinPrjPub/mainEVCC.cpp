#include "main.h"
#include "../LibCpp/HAL/Windows/cWinTcpClient.h"

#include "../LibCpp/HAL/Tools.h"
#include "../LibCpp/HAL/HW_Tools.h"

#include "../LibIso15118/Codico/cCodicoPilot.h"
#include "../LibIso15118/PlcStack/cSlacProcessEVCCPub.h"
#include "../LibIso15118/cExiIso15118_2013_openV2g.h"
#include "../LibIso15118/cIsoProcessEVCC.h"

#include "../Hardware/cEvPowerSource.h"

cWinTcpClient client;
cSlacProcessEVCCPub slacEVCC(&plc);
cEvPowerSource evHardware(&powerSource);
//cIsoProcessEVCCPub isoProcessEVCC(&codecIso, &udp, &client, &controlPilot, &evHardware);
cIsoProcessEVCC isoProcessEVCC(&codecIso, &udp, &client, config.interfaceName, &controlPilot);


void printHelpEVCC()
{
    printf_flush("'q' to finish\n");
    printf_flush("'h' to help\n");
    printf_flush("'s' to friendly stop charging\n");
    printf_flush("\n");
}

int mainEVCC()
{

    printf_flush("KEA Wired Power EVCC Test Vehicle\n");
    printHelpEVCC();

    dbg.setInstanceName("mainEVCC");
    packetSocket.dbg.setInstanceName("packetSocket", &dbg);
    udp.dbg.setInstanceName("port", &dbg);
    client.dbg.setInstanceName("client", &dbg);
    controlPilot.dbg.setInstanceName("codico", &dbg);
    plc.dbg.setInstanceName("plc", &dbg);
    slacEVCC.dbg.setInstanceName("slacEVCC", &dbg);
    codecIso.dbg.setInstanceName("codecIso", &dbg);
    codecDin.dbg.setInstanceName("codecDin", &dbg);
    isoProcessEVCC.dbg.setInstanceName("isoProcessEVCC", &dbg);

    //cDebug::instance().setDebugLevel(enDebugLevel_Info);
    //dbg.setDebugLevel(enDebugLevel_Info);
    packetSocket.dbg.setDebugLevel(enDebugLevel_Info);
    udp.dbg.setDebugLevel(enDebugLevel_Info);
    client.dbg.setDebugLevel(enDebugLevel_Info);
    controlPilot.dbg.setDebugLevel(enDebugLevel_Info);
    plc.dbg.setDebugLevel(enDebugLevel_Info);
    slacEVCC.dbg.setDebugLevel(enDebugLevel_Info);
    codecIso.dbg.setDebugLevel(enDebugLevel_Info);
    codecDin.dbg.setDebugLevel(enDebugLevel_Info);
    isoProcessEVCC.dbg.setDebugLevel(enDebugLevel_Info);
    powerSource.dbg.setDebugLevel(enDebugLevel_Info);

    isoProcessEVCC.evccInfo.energyTransferMode = config.transferMode;
    isoProcessEVCC.clientInterface = config.interfaceName;
    isoProcessEVCC.evccInfo.schemaId = config.schemaId;

    if (udp.open(55100, config.interfaceName, LibCpp::enAddressFamily_IPv6) != LibCpp::enIpResult_Success)
    {
        cInterfaces interfaces(true);
        printf("Opening '%s' failed! Available interfaces are:\n%s\n", INTERFACE_NAME, interfaces.toString().c_str());
        interfaces.close();
    }
    udp.setDestination(stIpAddress("FF02::1"), 15118);
    packetSocket.open(config.interfaceName);
    //powerSource.presetDemand = false;
    powerSource.presetNominalPower = false;
    powerSource.openTcpClient(&clientPower, config.eaSourceIp);

    //packetSocket.setFilter();
    isoProcessEVCC.setupCommunication();

    enOperationState operationState = enOperationState::initialization;
    enOperationState newOperationState;
    bool enteredOperationState = false;
    dbg.printf(enDebugLevel_Info, "\n******** Operation state: %s ********", enOperationState_toString(operationState).c_str());

    while (1)
    {
        // Charge process state machine

        packetSocket.operate();
        udp.operate();
        client.operate();
        clientPower.operate();
        controlPilot.operate();
        powerSource.operate();
        evHardware.operate();

        newOperationState = operationState;
        if (enteredOperationState)
            dbg.printf(enDebugLevel_Info, "\n******** Operation state: %s ********", enOperationState_toString(operationState).c_str());

        if (operationState == enOperationState::initialization)
        {
            static bool controlPilotInitialized = false;
            static bool slacInitialized = false;
            static bool powerSourceInitialized = false;
            //if (!isoProcessEVCC.pEv)
                //powerSourceInitialized = true;

            if (controlPilot.isInitialized() && !controlPilotInitialized)
            {
                controlPilotInitialized = true;
//                if (isoProcess.pControlPilot)
                dbg.printf(enDebugLevel_Info, "Hardware controlPilot initialized.");
//                else
//                dbg.printf(enDebugLevel_Info, "Hardware controlPilot is not configured.");
            }
            if (slacEVCC.isInitialized() && !slacInitialized)
            {
                slacInitialized = true;
                dbg.printf(enDebugLevel_Info, "Hardware plc (qca-chip) initialized.");
            }
            if ((powerSource.isInitialized() || !isoProcessEVCC.evHardware()) && !powerSourceInitialized)
            {
                powerSourceInitialized = true;
                evHardware.setActualBatteryVoltage(20);
                if (isoProcessEVCC.evHardware())
                    dbg.printf(enDebugLevel_Info, "Hardware powerSource initialized.");
                else
                    dbg.printf(enDebugLevel_Info, "Hardware powerSource is not configured.");
            }

            //  if (controlPilotInitialized && slacInitialized && powerSourceInitialized)
            if (controlPilotInitialized && slacInitialized)
                newOperationState = enOperationState::plug;
        }

        if (operationState == enOperationState::plug)
        {
//            if (enteredOperationState)
//                controlPilot.setPwm(enCpPwm::Pwm_Off);
//            if (controlPilot.getState() == enCpState::B_Connected)
//                newOperationState = enOperationState::stateBtimer;
            newOperationState = enOperationState::slac;
        }

//        if (operationState == enOperationState::stateBtimer)
//        {
//            if (enteredOperationState)
//                stateBtimer.start(100);
//            if (stateBtimer.elapsed())
//                newOperationState = enOperationState::slac;
//        }

        if (slacEVCC.operate(operationState==enOperationState::slac))
        {
            if (slacEVCC.getState() == enSlacState::Matched)
                newOperationState = enOperationState::communication;
        }
//        if (operationState == enOperationState::slac)
//        {
//            if (enteredOperationState)
//                controlPilot.setPwm(enCpPwm::Pwm_Iso15118);
//        }

        if (isoProcessEVCC.operate(operationState==enOperationState::communication))
            newOperationState = enOperationState::outOfOperation;

        enteredOperationState = (newOperationState != operationState);
        operationState = newOperationState;

//        if (isoProcessEVCC.v2gState >= Iso15118::enV2gState_awaitPowerDelivery)
//            break;

        if (console.hasKey())
        {
            char input = console.getKey();
            printf_flush("\n%c\n", input);
            if (input == 'q')
                break;
            if (input == 'h')
            {
                printHelpEVCC();
            }
            if (input == 's')
            {
                isoProcessEVCC.evccInput.enable = false;
            }
        }

        LibCpp::sleep(10);
    }
    
    controlPilot.stopService();
    packetSocket.close();
    udp.close();
    client.close();

    printf_flush("Ready\n");
    return 0;
}
