#include "Configuration.h"
#include "../../LibCpp/Xml/XMLObjectDocument.h"

using namespace std;
using namespace Iso15118;
using namespace tinyxml2;

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

void writeXML(stConfiguration& config)
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

