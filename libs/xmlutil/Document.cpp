#include "Document.h"
#include "XPathException.h"

#include <libxml/xpath.h>

#include <iostream>

namespace xml
{

// Construct a wrapper around the provided xmlDocPtr.
Document::Document(xmlDocPtr doc):
    _xmlDoc(doc)
{}

Document::Document(const std::string& filename) :
	_xmlDoc(xmlParseFile(filename.c_str()))
{}

Document::Document(const Document& other) :
	_xmlDoc(other._xmlDoc) 
{}

Document::~Document() {
	if (_xmlDoc != NULL) {
		// Free the xml document memory
		xmlFreeDoc(_xmlDoc);
	}
}

bool Document::isValid() const {
	return _xmlDoc != NULL;
}

// Evaluate an XPath expression and return matching Nodes.
NodeList Document::findXPath(const std::string& path) const {
    // Set up the XPath context
    xmlXPathContextPtr context;
    xmlXPathObjectPtr result;
    
    context = xmlXPathNewContext(_xmlDoc);
    if (context == NULL) {
        std::cerr << "ERROR: xml::findPath() failed to create XPath context "
                  << "when searching for " << path << std::endl;
        throw XPathException("Failed to create XPath context");
    }
    
    // Evaluate the expression  
    const xmlChar* xpath = reinterpret_cast<const xmlChar*>(path.c_str());
    result = xmlXPathEvalExpression(xpath, context);
    xmlXPathFreeContext(context);

    if (result == NULL) {
        std::cerr << "ERROR: xml::findPath() failed to evaluate expression "
                  << path << std::endl;
        throw XPathException("Failed to evaluate XPath expression");
    }
    
    // Construct the return vector. This may be empty if the provided XPath
    // expression does not identify any nodes.
    NodeList retval;
    xmlNodeSetPtr nodeset = result->nodesetval;
	if (nodeset != NULL) {
	    for (int i = 0; i < nodeset->nodeNr; i++) {
	        retval.push_back(Node(nodeset->nodeTab[i]));
	    }
	}

    xmlXPathFreeObject(result);
    return retval;
}

// Saves the file to the disk via xmlSaveFile
void Document::saveToFile(const std::string& filename) {
	xmlSaveFile(filename.c_str(), _xmlDoc);
}

}
