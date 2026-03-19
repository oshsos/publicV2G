// utf-8 (ü)
/**
 * @file   XMLObjectDocument.h
 * @author Olaf Simon
 * @date   04.10.2023
 * @brief  Class tinyxml2::XMLObjectDocument
 *
 * \addtogroup LibCpp_tinyxml2
 * @{
**/

#ifndef h_XMLObjectDocument
#define h_XMLObjectDocument

#include <string>
#include "tinyxml2.h"

namespace tinyxml2
{
    /**
     * @brief Addition to TinyXml2 for simplification of XML file writing or reading (parsing) of object member values in a recursive style
    **/
    class XMLObjectDocument
    {
    private:
        XMLDocument document;                                                   ///< XMLDocument member
        XMLElement* pRoot = nullptr;                                            ///< Pointer to the root XMLElement
        std::string fileName;                                                   ///< File name for reading or storage.

    public:
        XMLDocument* getDocument();                                             ///< Pointer to the XMLDocument
        // Methods for writing
        XMLElement* OpenWrite(std::string fileName, std::string rootName);      ///< Opens for writing a XML file
        XMLError Save();                                                        ///< Saving generated XML data
        static XMLElement* NewObjectElement(XMLDocument* pDocument, std::string className, std::string instanceName = "");  ///< Attaches a new XMLElement aimed for object member values to the XML document
        // Methods for reading/parsing
        XMLElement* OpenRead(std::string fileName, std::string rootName);       ///< Opens for reading a XML file
        static XMLElement* FirstChildObjectElement(XMLElement* pParentElement, std::string className, std::string instanceName, bool allInstances = false);        ///< Finds a first object data storage of a given class type and opional given instance name
        static XMLElement* NextSiblingObjectElement(XMLElement* pLastObjectElement, std::string className, std::string instanceName, bool allInstances = false);   ///< Scans a following object data storage of a given class type and opional given instance name
        static XMLElement* FirstChildObjectElement(XMLElement* pParentElement, std::string className);        ///< Finds a first object data storage of a given class type and opional given instance name
        static XMLElement* NextSiblingObjectElement(XMLElement* pLastObjectElement, std::string className);   ///< Scans a following object data storage of a given class type and opional given instance name
    };
}

#endif

/** @} */
