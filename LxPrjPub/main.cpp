// UTF8 (ü)
/**
\file   main.cpp

\author Dr. Olaf Simon (KEA)
\author KEA
\date   2024-07-16

\addtogroup G_publicV2G

\brief The publicV2G main file for Windows

\page Page_Main_Windows_Public The main code for the Windows implementation

\ref main.cpp

 *
 **/

#include "main.h"
#include "../LibCpp/Lab/cEAsource.h"
#include "../LibCpp/Xml/XMLObjectDocument.h"

using namespace std;
using namespace tinyxml2;

extern int mainEVCC();
extern int mainSECC();

cDebug                      dbg("main", enDebugLevel_Debug);
cLxConsoleInput            console;

//enStackType                 stackType;
stConfiguration             config;

cLxPacketSocket            packetSocket;
cLxUdpSocket               udp;
cLxTcpClient               clientPower;

cPlcStackPub                plc(&packetSocket);
cCodicoPilot                controlPilot(enStackType::secc, &packetSocket, &plc);
cExiIso15118_2013_openV2g   codecIso;
cExiDin70121openV2g         codecDin;
cEAsource                   powerSource(&clientPower);

XMLElement* writeXMLParameter(tinyxml2::XMLDocument* pDocument, XMLElement* pParent, string parameterName = "newParameter", string value = "value", string options = "Option1, Option 2")
{
    XMLElement* entry = pDocument->NewElement(parameterName.c_str());
    entry->SetText(value.c_str());
    XMLElement* optionEntry = pDocument->NewElement("Options");
    optionEntry->SetText(options.c_str());
    entry->InsertEndChild(optionEntry);
    pParent->InsertEndChild(entry);
    return entry;
}

void writeXML()
{
    XMLObjectDocument xmlWr;
    XMLElement* pRoot = xmlWr.OpenWrite("config.xml", "IsoCharging");
    tinyxml2::XMLDocument* pDoc = xmlWr.getDocument();

    pRoot->SetAttribute("Configuration", config.instance.c_str());

    XMLElement* conf1 = XMLObjectDocument::NewObjectElement(pDoc, "Configuration", (string(STDCONFIGNAME) + " secc").c_str());
    writeXMLParameter(pDoc, conf1, "StackType", "secc", "secc, evcc");
    writeXMLParameter(pDoc, conf1, "Interface", config.interfaceName.c_str(), "Ethernet, WLAN, eth0, ...");
    writeXMLParameter(pDoc, conf1, "SchemaID", to_string(config.schemaId).c_str(), "Integer - ID3: din=1 iso=0; IONIQ5: din=1 iso=2; Codico: din=2 iso=3; dSpace: iso=10");
    writeXMLParameter(pDoc, conf1, "EASourceIP", config.eaSourceIp.c_str(), "e.g.: 192.168.0.6");
    writeXMLParameter(pDoc, conf1, "TLS", config.provideTls ? "true" : "false", "Provide TLS communication: true, false");
    writeXMLParameter(pDoc, conf1, "TransferMode", iso1EnergyTransferModeType_toString(config.transferMode).c_str(), "AC-3phase, AC-1phase, DC-core, DC-combo, DC-unique, DC-extended");
    pRoot->InsertEndChild(conf1);

    XMLElement* conf2 = XMLObjectDocument::NewObjectElement(pDoc, "Configuration", (string(STDCONFIGNAME) + " evcc").c_str());
    writeXMLParameter(pDoc, conf2, "StackType", "evcc", "secc, evcc");
    writeXMLParameter(pDoc, conf2, "Interface", config.interfaceName.c_str(), "Ethernet, WLAN, eth0, ...");
    writeXMLParameter(pDoc, conf2, "SchemaID", to_string(config.schemaId).c_str(), "Integer - ID3: din=1 iso=0; IONIQ5: din=1 iso=2; Codico: din=2 iso=3; dSpace: iso=10");
    writeXMLParameter(pDoc, conf2, "EASourceIP", config.eaSourceIp.c_str(), "e.g.: 192.168.0.6");
    writeXMLParameter(pDoc, conf2, "TLS", config.provideTls ? "true" : "false", "Provide TLS communication: true, false");
    writeXMLParameter(pDoc, conf2, "TransferMode", iso1EnergyTransferModeType_toString(config.transferMode).c_str(), "AC-3phase, AC-1phase, DC-core, DC-combo, DC-unique, DC-extended");
    pRoot->InsertEndChild(conf2);

    xmlWr.Save();
}

stConfiguration readXMLConfiguration(XMLElement* pElement)
{
    stConfiguration res;

    const char* txt;
    if(pElement->QueryAttribute("instance", &txt) == XML_SUCCESS)
        res.instance = string(txt);

    XMLElement* pStackType = pElement->FirstChildElement("StackType");
    if (pStackType)
    {
        if (string(pStackType->GetText()) == "evcc" || string(pStackType->GetText()) == "EVCC")
            res.stackType = enStackType::evcc;
        else
            res.stackType = enStackType::secc;
    }

    XMLElement* pInterface = pElement->FirstChildElement("Interface");
    if (pInterface)
        res.interfaceName = string(pInterface->GetText());

    XMLElement* pSchemaId = pElement->FirstChildElement("SchemaID");
    if (pSchemaId)
        res.schemaId = atoi(pSchemaId->GetText());

    XMLElement* pEaSourceIp = pElement->FirstChildElement("EASourceIP");
    if (pEaSourceIp)
        res.eaSourceIp = string(pEaSourceIp->GetText());

    XMLElement* pTls = pElement->FirstChildElement("TLS");
    if (pTls)
    {
        if (string(pTls->GetText()) == string("false"))
            res.provideTls = false;
        else
            res.provideTls = true;
    }

    XMLElement* pTransferMode = pElement->FirstChildElement("TransferMode");
    if (pTransferMode)
        res.transferMode = iso1EnergyTransferModeType_fromString(string(pTransferMode->GetText()));

    return res;
}

bool readXML(stConfiguration& config)
{
    XMLObjectDocument xmlRd;
    XMLElement* pRoot = xmlRd.OpenRead("config.xml", "IsoCharging");

    if (pRoot)
    {
        const char* configName;
        string configNameStr;
        if(pRoot->QueryAttribute("Configuration", &configName) == XML_SUCCESS)
            configNameStr = string(configName);

        XMLElement* pEntry = XMLObjectDocument::FirstChildObjectElement(pRoot, "Configuration");
        while (pEntry)
        {
            config = readXMLConfiguration(pEntry);
            if (config.instance == configNameStr)
                return true;
            pEntry = XMLObjectDocument::NextSiblingObjectElement(pEntry, "Configuration");
        }
    }
    return false;
}

int main()
{
//    cDebug::instance().setDebugLevel(enDebugLevel_Debug);
//    cDebug dbg("main");

    // Read/Write XML file
    if (!readXML(config))
    {
        dbg.printf(enDebugLevel_Error, "Configuration file config.xml not available or requested configuration not found! Generating standard file.");
        writeXML();
    }
    config.hardware = enHardwareType::NONE;

    // Set operation parameters according to the configuration file
    controlPilot.setStackMode(config.stackType);
//    if (config.stackType == "evcc")
//        stackType = enStackType::evcc;
//    else
//        stackType = enStackType::secc;
//        controlPilot.setStackMode(enStackType::secc);

    // Run the application
    if (config.stackType == enStackType::evcc)
        return mainEVCC();
    else
        return mainSECC();
}

std::string enOperationState_toString(enOperationState state)
{
    switch (state)
    {
    case enOperationState::initialization: return "Initialization";
    case enOperationState::plug: return "Plug";
    case enOperationState::stateBtimer: return "stateBtimer";
    case enOperationState::slac: return "Slac";
    case enOperationState::communication: return "Communication";
    case enOperationState::processStopped: return "processStopped";
    case enOperationState::outOfOperation: return "Out of operation";
    default: return "<unknown>";
    }
}
