#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/dom/DOMDocumentType.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMNodeIterator.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <string>
#include <map>
#include <stdexcept>
#include <xercesc/framework/LocalFileInputSource.hpp>
#include <xercesc/framework/Wrapper4InputSource.hpp>

using namespace xercesc;
using namespace std;

class SamlParser
{
    private:
        /** Location of file containing a saml*/
        string m_samlFile;
        xercesc::XercesDOMParser *m_samlParser;
        DOMDocument *m_xmlDoc;
        DOMElement *m_elementRoot;
        DOMNodeList *m_rootChildren;
        DOMNodeList *m_attributeNodes;
        DOMNodeList *m_assertionNodes;
        DOMNodeList *m_statusNodes;
        DOMNodeList *m_attrStmtNodes;

        XMLCh *TAG_root;
        XMLCh *TAG_status;
        XMLCh *TAG_statusCode;
        XMLCh *TAG_attribute;
        XMLCh *TAG_assertion;
        XMLCh *TAG_attrstmt;
        XMLCh *TAG_AttributeValue;

        XMLCh *ATTR_OptionStatusCode;
        XMLCh *ATTR_AttributeName;
		XMLCh *ATTR_AttributeNamespace;

    public:
        SamlParser(string file) : m_samlFile(file), m_xmlDoc(NULL), m_elementRoot(NULL), m_rootChildren(NULL),
                                  m_attributeNodes(NULL), m_assertionNodes(NULL), m_statusNodes(NULL), m_attrStmtNodes(NULL)
        {
            XMLPlatformUtils::Initialize();
            TAG_root = XMLString::transcode("Response");
            TAG_status = XMLString::transcode("Status");
            TAG_statusCode = XMLString::transcode("StatusCode");
            TAG_attribute = XMLString::transcode("Attribute");
            TAG_assertion = XMLString::transcode("Assertion");
            TAG_attrstmt = XMLString::transcode("AttributeStatement");
            TAG_AttributeValue = XMLString::transcode("AttributeValue");

            ATTR_OptionStatusCode = XMLString::transcode("Value");
            ATTR_AttributeName = XMLString::transcode("AttributeName");
			ATTR_AttributeNamespace = XMLString::transcode("AttributeNamespace");

            m_samlParser = new XercesDOMParser;

            m_samlParser->setValidationScheme(XercesDOMParser::Val_Never);
            m_samlParser->setDoNamespaces(false);
            m_samlParser->setDoSchema(false);
            m_samlParser->setLoadExternalDTD(false);
        }

        ~SamlParser()
        {
            delete m_samlParser;
            XMLString::release(&TAG_root);
            XMLString::release(&TAG_status);
            XMLString::release(&TAG_statusCode);
            XMLString::release(&ATTR_OptionStatusCode);

            try
            {
                XMLPlatformUtils::Terminate();
            }
            catch (xercesc::XMLException &e)
            {
                char* message = xercesc::XMLString::transcode( e.getMessage() );
                cerr << "XML toolkit teardown error: " << message << endl;
                XMLString::release( &message );
            }
        }

		bool ParseSaml(std::string data)
		{
			MemBufInputSource is((XMLByte*)data.c_str(), strlen(data.c_str()), "bufId");
			m_samlParser->parse(is); 
			m_xmlDoc = m_samlParser->getDocument();
			m_elementRoot = m_xmlDoc->getDocumentElement();
            m_rootChildren = m_elementRoot->getChildNodes();
            m_attributeNodes = m_xmlDoc->getElementsByTagName(TAG_attribute); 
            if (m_attributeNodes)
				printf("m_attributeNodes length : %d\n", m_attributeNodes->getLength());
            m_assertionNodes = m_xmlDoc->getElementsByTagName(TAG_assertion); 
            if (m_assertionNodes)
                printf("m_assertionNodes length : %d\n", m_assertionNodes->getLength());
            m_statusNodes = m_xmlDoc->getElementsByTagName(TAG_status); 
            if (m_statusNodes)
				printf("m_statusNodes length : %d\n", m_statusNodes->getLength());
            m_attrStmtNodes = m_xmlDoc->getElementsByTagName(TAG_attrstmt); 
            if (m_attrStmtNodes)
				printf("m_attrStmtNodes length : %d\n", m_attrStmtNodes->getLength());
            return true;
		}

        string getStatusCode()
        {
            string value;
            const  XMLSize_t rootNodeCount = m_rootChildren->getLength();
            for (XMLSize_t xx = 0; xx < rootNodeCount; xx++)
            {
                DOMNode* currentNode = m_rootChildren->item(xx);
                if( currentNode->getNodeType() &&  // true is not NULL
                        currentNode->getNodeType() == DOMNode::ELEMENT_NODE ) // is element
                {
                    // Found node which is an Element. Re-cast node as element
                    DOMElement* currentElement
                        = dynamic_cast< xercesc::DOMElement* >( currentNode );
                    const XMLCh* elementName = currentElement->getTagName();
                    if( XMLString::equals(currentElement->getTagName(), TAG_status))
                    {
                        DOMNodeList *statusNodeChildren = currentNode->getChildNodes();
                        const XMLSize_t statusNodeCount = statusNodeChildren->getLength();
                        for (XMLSize_t yy = 0; yy < statusNodeCount; yy++)
                        {
                            DOMNode *currentStatusNode = statusNodeChildren->item(yy);
                            if( currentStatusNode->getNodeType() &&  // true is not NULL
                                    currentStatusNode->getNodeType() == DOMNode::ELEMENT_NODE ) // is element
                            {
                                DOMElement* statuscurrentElement
                                    = dynamic_cast< xercesc::DOMElement* >( currentStatusNode );
                                const XMLCh* statusElementName = statuscurrentElement->getTagName();
                                char *ele = XMLString::transcode(statusElementName);
                                if (strcmp(ele, "StatusCode") == 0)
                                {
                                    const XMLCh* statusValue = statuscurrentElement->getAttribute(ATTR_OptionStatusCode);
                                    value = string(XMLString::transcode(statusValue));
                                    break;
                                }
                            }

                        }
                        // Already tested node as type element and of name "ApplicationSettings".
                        // Read attributes of element "ApplicationSettings".
                        const XMLCh* xmlch_StatusCodeValue
                            = currentElement->getAttribute(ATTR_OptionStatusCode);
                        char *m_OptionA = XMLString::transcode(xmlch_StatusCodeValue);
                    }
                }
            }
            return value;
        }

		bool createAttributeDualMap()
		{
			map<string, pair<string,string>> dualAttributeMap;
			const XMLSize_t attrNodeCount = m_attributeNodes->getLength();
            for (XMLSize_t xx = 0; xx < attrNodeCount; xx++)
            {
				DOMNode *attrNode = m_attributeNodes->item(xx);
				if (attrNode->getNodeType() && attrNode->getNodeType() == DOMNode::ELEMENT_NODE)
                {
                    DOMElement *attrNodeEle = dynamic_cast<DOMElement *>(attrNode);
                    //try to get attributevalue for this tag
                    const XMLCh* attrVal = attrNodeEle->getAttribute(ATTR_AttributeName);
					string attrValStr = string(XMLString::transcode(attrVal));
					const XMLCh* attrNamespaceVal = attrNodeEle->getAttribute(ATTR_AttributeNamespace);
					string attrNameSpaceStr = string(XMLString::transcode(attrNamespaceVal));
					string value;
					DOMNodeList *attrChild = attrNode->getChildNodes();
                    const XMLSize_t attrChildCount = attrChild->getLength();
                    for (XMLSize_t yy = 0; yy < attrChildCount; yy++) 
                    {
                        DOMNode *attrValNode = attrChild->item(yy);
                        if (attrValNode->getNodeType() && attrValNode->getNodeType() == DOMNode::ELEMENT_NODE)
                        {
                            const XMLCh *data = attrValNode->getTextContent();
                            value = string(XMLString::transcode(data));
							std::pair<string, string> firstPair = make_pair(attrNameSpaceStr, value);
							dualAttributeMap.insert(std::make_pair(attrValStr, firstPair));
                        }
                    }
				}
			}
		}

		bool fillAttributeMap(map<string, string>& assertionMap)
		{
			const XMLSize_t attrNodeCount = m_attributeNodes->getLength();
            for (XMLSize_t xx = 0; xx < attrNodeCount; xx++)
            {
				DOMNode *attrNode = m_attributeNodes->item(xx);
				if (attrNode->getNodeType() && attrNode->getNodeType() == DOMNode::ELEMENT_NODE)
                {
                    DOMElement *attrNodeEle = dynamic_cast<DOMElement *>(attrNode);
                    //try to get attributevalue for this tag
                    const XMLCh* attrVal = attrNodeEle->getAttribute(ATTR_AttributeName);
					char *attrValStr = XMLString::transcode(attrVal);
					string attrValue(attrValStr);
					string value;
					DOMNodeList *attrChild = attrNode->getChildNodes();
                    const XMLSize_t attrChildCount = attrChild->getLength();
                    for (XMLSize_t yy = 0; yy < attrChildCount; yy++) 
                    {
                        DOMNode *attrValNode = attrChild->item(yy);
                        if (attrValNode->getNodeType() && attrValNode->getNodeType() == DOMNode::ELEMENT_NODE)
                        {
                            const XMLCh *data = attrValNode->getTextContent();
                            value = string(XMLString::transcode(data));
							assertionMap.insert(std::make_pair(attrValue, value));
                        }
                    }
				}
			}
			return true; 
		}

        string getAttributeValue(string attrName)
        {
            string value;
            const XMLSize_t attrNodeCount = m_attributeNodes->getLength();
            for (XMLSize_t xx = 0; xx < attrNodeCount; xx++)
            {
                DOMNode *attrNode = m_attributeNodes->item(xx);
                if (attrNode->getNodeType() && attrNode->getNodeType() == DOMNode::ELEMENT_NODE)
                {
                    DOMElement *attrNodeEle = dynamic_cast<DOMElement *>(attrNode);
                    //try to get attributevalue for this tag
                    const XMLCh* attrVal = attrNodeEle->getAttribute(ATTR_AttributeName);
                    char *attrValStr = XMLString::transcode(attrVal);
                    if (strcmp(attrValStr, attrName.c_str()) == 0)
                    {
                        DOMNodeList *attrChild = attrNode->getChildNodes();
                        const XMLSize_t attrChildCount = attrChild->getLength();
                        for (XMLSize_t yy = 0; yy < attrChildCount; yy++) 
                        {
                            DOMNode *attrValNode = attrChild->item(yy);
                            if (attrValNode->getNodeType() && attrValNode->getNodeType() == DOMNode::ELEMENT_NODE)
                            {
                                const XMLCh *data = attrValNode->getTextContent();
                                value = string(XMLString::transcode(data));
                            }
                        }
                        break;
                    }
                }
            }

            return value;
        }
};

