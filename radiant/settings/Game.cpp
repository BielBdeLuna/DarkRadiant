#include "Game.h"

#include "itextstream.h"
#include "iregistry.h"
#include "xmlutil/Document.h"
#include <iostream>
#include "error.h"

namespace game {

/** greebo: Constructor, call this with the filename relative to "games/"
 */
Game::Game(const std::string& path, const std::string& filename) {
	
	std::string fullPath = path + filename;
	
	// Load the XML using libxml2 to check for the <game> tag
	xmlDocPtr pDoc = xmlParseFile(fullPath.c_str());
	
	if (pDoc) {
		xml::Document doc(pDoc);
		
		// Check for a toplevel game node
		xml::NodeList list = doc.findXPath("/game");
	    if (list.size() == 0) {
			Error("Couldn't find <game> node in the game description file '%s'\n", fullPath.c_str());
		}
		else {
			xml::Node node = list[0];
			
			// Get the game name
			_type = node.getAttributeValue("type");
			
			const std::string enginePath =
#if defined(WIN32)
				"enginepath_win32"
#elif defined(__linux__) || defined (__FreeBSD__)
				"enginepath_linux"
#elif defined(__APPLE__)
				"enginepath_macos"
#else
#error "unknown platform"
#endif
			;
			
			if (!_type.empty()) {
				// Import the game file into the registry 
				GlobalRegistry().import(fullPath, "", Registry::treeStandard);
				
				// Get the engine path
				_enginePath = getKeyValue(enginePath);
			}
		}
		
		// Free the xml document memory
		xmlFreeDoc(pDoc);
	}
	else {
		globalErrorStream() << "Could not parse XML file: " << fullPath.c_str() << "\n"; 
	}
}

Game::Game(const Game& other) :
	_enginePath(other._enginePath),
	_type(other._type)
{}

std::string Game::getType() const {
	return _type;
}

/** greebo: Looks up the specified key
 */
const char* Game::getKeyValue(const std::string& key) {
	std::string gameXPath = std::string("//game[@type='") + _type + "']";
	
	xml::NodeList found = GlobalRegistry().findXPath(gameXPath);
	
	if (found.size() > 0) {
		return found[0].getAttributeValue(key).c_str();
	}
	else {
		return "";
	}
}

/** greebo: Emits an error if the keyvalue is empty
 */
const char* Game::getRequiredKeyValue(const std::string& key) {
	std::string returnValue = getKeyValue(key);
	if (!returnValue.empty()) {
		return returnValue.c_str();
	}
	else {
		ERROR_MESSAGE("game attribute " << makeQuoted(key.c_str()) << " not found for game type " << makeQuoted(_type.c_str()));
		return "";
	}
}

} // namespace game
