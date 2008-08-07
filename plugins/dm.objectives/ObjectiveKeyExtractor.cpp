#include "ObjectiveKeyExtractor.h"

#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include "string/string.h"

namespace objectives {

// Shortcut for boost::algorithm::split
typedef std::vector<std::string> StringParts;
	
// Required entity visit function
void ObjectiveKeyExtractor::visit(const std::string& key, 
								  const std::string& value)
{
	// Quick discard of any non-objective keys
	if (key.substr(0, 3) != "obj")
		return;
		
	// Extract the objective number
	static const boost::regex reObjNum("obj(\\d+)_(.*)");
	boost::smatch results;
	int iNum;
	
	if (boost::regex_match(key, results, reObjNum)) {
		// Get the objective number
		iNum = strToInt(results[1]);			
	}
	else {
		// No match, abort
		return;
	}

	// We now have the objective number and the substring (everything after
	// "obj<n>_" which applies to this objective.
	std::string objSubString = results[2];
	
	// Switch on the substring
	if (objSubString == "desc") {
		_objMap[iNum].description = value;			
	}
	else if (objSubString == "ongoing") {
		_objMap[iNum].ongoing = (value == "1");			
	}
	else if (objSubString == "mandatory") {
		_objMap[iNum].mandatory = (value == "1");			
	}
	else if (objSubString == "visible") {
		_objMap[iNum].visible = (value == "1");			
	}
	else if (objSubString == "irreversible") {
		_objMap[iNum].irreversible = (value == "1");			
	}
	else if (objSubString == "state") {
		_objMap[iNum].state = 
			static_cast<Objective::State>(strToInt(value));
	}
	else {
	
		// Use another regex to check for components (obj1_1_blah)
		static const boost::regex reComponent("(\\d+)_(.*)");
		boost::smatch results;
		
		if (!boost::regex_match(objSubString, results, reComponent)) {
			return;
		}
		else {
			
			// Get the component number and key string
			int componentNum = strToInt(results[1]);
			std::string componentStr = results[2];
			
			Component& comp = _objMap[iNum].components[componentNum];
			
			// Switch on the key string
			if (componentStr == "type") {
				comp.setType(ComponentType::getComponentType(value)); 
			}
			else if (componentStr == "state") {
				comp.setSatisfied(value == "1");
			}
			else if (componentStr == "not") {
				comp.setInverted(value == "1");
			}
			else if (componentStr == "irreversible") {
				comp.setIrreversible(value == "1");
			}
			else if (componentStr == "args") {
				// We have a component argument string
				StringParts parts;
				boost::algorithm::split(parts, value, boost::algorithm::is_any_of(" "));

				comp.clearArguments();

				// Add all found arguments to the component
				for (std::size_t i = 0; i < parts.size(); i++) {
					comp.addArgument(parts[i]);
				}
			}
		}
			
		
	}

}
	
} // namespace objectives
