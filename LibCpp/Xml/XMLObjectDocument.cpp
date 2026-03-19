// utf-8 (ü)

// MIT License
// Copyright © 2023 Olaf Simon
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software
// and associated documentation files (the “Software”), to deal in the Software without restriction,
// including without limitation the rights to use, copy, modify, merge, publish, distribute,
// sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or
// substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
// BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

/**
 * @file   XMLObjectDocument.cpp
 * @author Olaf Simon
 * @date   04.10.2023
 * @brief  Class tinyxml2::XMLObjectDocument
 *
 * \addtogroup LibCpp_tinyxml2
 * @{
 *
 * \class tinyxml2::XMLObjectDocument
 *
 * Published under MIT License (see cTimeStd.cpp)
 *
 *  \code
    #include <iostream>
    #include <vector>
    #include "XMLObjectDocument.h"

    class SubChild
    {
    public:
        string myData;

        SubChild(string data = "")
        {
            myData = data;
        }

        void WriteXML(XMLDocument* pDocument, XMLElement* pParent, string instanceName = "")
        {
            XMLElement* element = XMLObjectDocument::NewObjectElement(pDocument, "SubChild", instanceName);
            element->SetText(myData.c_str());
            pParent->InsertEndChild(element);
        }

        bool ReadXML(XMLElement* pMyElement)
        {
            if (!pMyElement) return false;

            const char* pText = pMyElement->GetText();
            if (pText)
            {
                string text(pText);
                myData = text;
                return true;
            }
            return false;
        }
    };

    class Child
    {
    public:
        SubChild subChild1;
        SubChild subChild2;
        int value=0;

        Child(int value = 0) :
            subChild1("data1"),
            subChild2("data2")
        {
            this->value = value;
        }

        void WriteXML(XMLDocument* pDocument, XMLElement* pParent, string instanceName = "")
        {
            XMLElement* element = XMLObjectDocument::NewObjectElement(pDocument, "Child", instanceName);
            element->SetAttribute("value", value);
            subChild1.WriteXML(pDocument, element, "subChild1");
            subChild2.WriteXML(pDocument, element, "subChild2");
            pParent->InsertEndChild(element);
        }

        bool ReadXML(XMLElement* pMyElement)
        {
            if (!pMyElement) return false;

            bool result = (pMyElement->QueryAttribute("value", &value) == XML_SUCCESS);
            XMLElement* pEntry = XMLObjectDocument::FirstChildObjectElement(pMyElement, "SubChild", "subChild1");
            result = result && subChild1.ReadXML(pEntry);
            pEntry = XMLObjectDocument::FirstChildObjectElement(pMyElement, "SubChild", "subChild2");
            result = result && subChild2.ReadXML(pEntry);

            return result;
        }
    };

    int main()
    {
        cout << "XmlDemo" << endl;

        // Data initialization for writing:
        Child childWr(5);
        vector<SubChild> subChildListWr;

        SubChild subChildA("dataA");
        SubChild subChildB("dataB");
        subChildListWr.push_back(subChildA);
        subChildListWr.push_back(subChildB);

        // Writing XML file
        XMLObjectDocument xmlWr;
        XMLDocument* pDoc = xmlWr.getDocument();
        XMLElement* pRoot = xmlWr.OpenWrite("data.xml", "Main");

        childWr.WriteXML(pDoc, pRoot, "child");
        for (SubChild& subChild : subChildListWr)
            subChild.WriteXML(pDoc, pRoot, "list");   // or subChild.WriteXML(pDoc, pRoot) to write no instance Attribute;

        xmlWr.Save();

        // Data declaration for reading
        Child childRd;
        vector<SubChild> subChildListRd;

        // Read a XML file
        XMLObjectDocument xmlRd;
        pRoot = xmlWr.OpenRead("data.xml", "Main");

        if (pRoot)
        {
            XMLElement* pEntry = XMLObjectDocument::FirstChildObjectElement(pRoot, "Child", "child");
            childRd.ReadXML(pEntry);
            pEntry = XMLObjectDocument::FirstChildObjectElement(pRoot, "SubChild", "list"); // or XMLObjectDocument::FirstChildObjectElement(pRoot, "SubChild") to read all entries having no instance parameters set or having instance parameter set to "".
            while (pEntry)
            {
                SubChild child;
                child.ReadXML(pEntry);
                subChildListRd.push_back(child);
                pEntry = XMLObjectDocument::NextSiblingObjectElement(pEntry, "SubChild", "list");
            }
        }

        cout << "Ready" << endl;
        return 0;
    }
 *  \endcode
**/

#include "XMLObjectDocument.h"

using namespace std;
using namespace tinyxml2;

/**
 * @brief Retruns the pointer to the assossiated XML document
 * @return XMLDocument*
 */
XMLDocument* XMLObjectDocument::getDocument()
{
    return &document;
}

// Methods for writing
/**
 * @brief Opens the XMLObjectDocument instance for writing a XML file
 * @param fileName
 * @param rootName Name of the XML root entry
 * @return Pointer to the root XMLElement
 */
XMLElement* XMLObjectDocument::OpenWrite(string fileName, string rootName)
{
    document.InsertFirstChild(document.NewDeclaration());
    pRoot = document.NewElement(rootName.c_str());
    document.InsertEndChild(pRoot);
    this->fileName = fileName;
    return pRoot;
}

/**
 * @brief Saving generated XML data
 * @return XML_SUCCESS or Error code
 */
XMLError XMLObjectDocument::Save()
{
    return document.SaveFile(fileName.c_str());
}

/**
 * @brief Attaches a new XMLElement aimed for object member values to the XML document.
 * @param pDocument The XMLDocument to write to. It can be received by the getDocument method.
 * @param className Name of the class the new XMLElement belongs to.
 * @param instanceName (optional) Name of the instance the XMLElement belongs to.
 * @return The new XMLElement pointer
 * The method is typically used within the WriteXML method of an object aimed to store member values
 * in XML format. Member values can be stored to this XMLElement either by defining properties,
 * setting text or subsequent XMLElement entries by element->InsertEndChild().
 * Further more object instances prepared for XML member value storage can attach
 * their own XML entries. The typical usage is as follows:
 * \code
    void myObject::WriteXML(XMLDocument* pDocument, XMLElement* pParent, string instanceName = "")
    {
        XMLElement* element = XMLObjectDocument::NewObjectElement(pDocument, "Child", instanceName);
        element->SetAttribute("value", value);
        element->SetText("Hello");
        subChild1.WriteXML(pDocument, element, "subChild1");
        pParent->InsertEndChild(element);
    }
 * \endcode
 */
XMLElement* XMLObjectDocument::NewObjectElement(XMLDocument* pDocument, string className, string instanceName)
{
    XMLElement* pElement = pDocument->NewElement(className.c_str());
    if (!instanceName.empty())
        pElement->SetAttribute("instance", instanceName.c_str());
    return pElement;
}

// Methods for reading/parsing
/**
 * @brief Opens the XMLObjectDocument instance for reading a XML file
 * @param fileName
 * @param rootName Name of the root entry expected in the xml file.
 * @return The XMLElement pointer to the root element
 */
XMLElement* XMLObjectDocument::OpenRead(string fileName, string rootName)
{
    XMLError res = document.LoadFile(fileName.c_str());
    if (res == XML_SUCCESS)
    {
        pRoot = document.FirstChildElement(rootName.c_str());
        return pRoot;
    }
    return nullptr;
}

/**
 * @brief Finds a first object data storage of a given class type and opional given instance name
 * @param pParentElement The XMLElement which should contain the requested object and instance
 * @param className
 * @param instanceName (optional) Standard is "". In this case entries are accepted which have no instance attribute set, or are set to "".
 * @param allInstances (optional) Standard is false. If set to true the instanceName is ignored and each entry of the given className is returned.
 * @return XMLElemnent pointer of stored data of the requested class and instance name. nullptr in case the requested class and instance is not found.
 */
XMLElement* XMLObjectDocument::FirstChildObjectElement(XMLElement* pParentElement, string className, string instanceName, bool allInstances)
{
    XMLElement* pEntry = pParentElement->FirstChildElement(className.c_str());
    while (pEntry)
    {
        if (allInstances)
            return pEntry;
        const char* pInstance = nullptr;
        pEntry->QueryAttribute("instance", &pInstance);
        if(pInstance)
        {
            if(strcmp(pInstance, instanceName.c_str())==0)
                return pEntry;
        }
        else if (instanceName.empty())
        {
            return pEntry;
        }
        pEntry = pEntry->NextSiblingElement(className.c_str());
    }
    return nullptr;
}

/**
 * @brief ///< Scans a following object data storage of a given class type and given instance name
 * @param pLastObjectElement
 * @param className
 * @param instanceName
 * @param allInstances
 * @return
 * For description see tinyxml2::XMLObjectDocument::FirstChildObjectElement
 */
XMLElement* XMLObjectDocument::NextSiblingObjectElement(XMLElement* pLastObjectElement, string className, string instanceName, bool allInstances)
{
    XMLElement* pEntry = pLastObjectElement->NextSiblingElement(className.c_str());
    while (pEntry)
    {
        if (allInstances)
            return pEntry;
        const char* pInstance = nullptr;
        pEntry->QueryAttribute("instance", &pInstance);
        if(pInstance)
        {
            if(strcmp(pInstance, instanceName.c_str())==0)
                return pEntry;
        }
        else if (instanceName.empty())
        {
            return pEntry;
        }
        pEntry = pEntry->NextSiblingElement(className.c_str());
    }
    return nullptr;
}

/**
 * @brief Finds a first object data storage of a given class type
 * @param pParentElement
 * @param className
 * @return
 */
XMLElement* XMLObjectDocument::FirstChildObjectElement(XMLElement* pParentElement, std::string className)
{
    return FirstChildObjectElement(pParentElement, className, "", true);
}

/**
 * @brief Scans a following object data storage of a given class type
 * @param pLastObjectElement
 * @param className
 * @return
 */
XMLElement* XMLObjectDocument::NextSiblingObjectElement(XMLElement* pLastObjectElement, std::string className)
{
    return NextSiblingObjectElement(pLastObjectElement, className, "", true);
}

/** @} */
