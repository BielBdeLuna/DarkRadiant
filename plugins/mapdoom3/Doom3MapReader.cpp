#include "Doom3MapReader.h"

#include "itextstream.h"
#include "imap.h"
#include "iradiant.h"
#include "imainframe.h"
#include "iregistry.h"
#include "ieclass.h"
#include "igame.h"
#include "string/string.h"

//#include "gtkutil/dialog.h"
//#include "MapImportInfo.h"
#include "scenelib.h"

#include "Tokens.h"
#include "Doom3MapFormat.h"

#include "i18n.h"
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

namespace map {

	/*namespace
	{
		const std::string RKEY_MAP_LOAD_STATUS_INTERLEAVE = "user/ui/map/loadStatusInterleave";
	}*/

// Constructor
Doom3MapReader::Doom3MapReader(IMapImportFilter& importFilter) : 
	_importFilter(importFilter),
  //_fileSize(importInfo.inputStreamSize),
	_entityCount(0),
	_primitiveCount(0),
  //_dialogEventLimiter(GlobalRegistry().getInt(RKEY_MAP_LOAD_STATUS_INTERLEAVE)),
	_debug(GlobalRegistry().get("user/debug") == "1")
{
	/*bool showProgressDialog = (GlobalRegistry().get(RKEY_MAP_SUPPRESS_LOAD_STATUS_DIALOG) != "1");

	if (showProgressDialog)
	{
		_dialog = gtkutil::ModalProgressDialogPtr(
			new gtkutil::ModalProgressDialog(
				GlobalMainFrame().getTopLevelWindow(), _("Loading map")
			)
		);
	}*/
}

void Doom3MapReader::readFromStream(std::istream& stream)
{
	// The tokeniser used to split the stream into pieces
	parser::BasicDefTokeniser<std::istream> tok(stream);

	// Try to parse the map version (throws on failure)
	parseMapVersion(tok);

	// Read each entity in the map, until EOF is reached
	while (tok.hasMoreTokens())
	{
		// Update the dialog text. This will throw an exception if the cancel
		// button is clicked, which we must catch and handle.
		/*if (_dialog && _dialogEventLimiter.readyForEvent())
		{
			try
			{
				std::string text = (boost::format(_("Loading entity %d")) % _entityCount).str();
				_dialog->setTextAndFraction(text, getProgressFraction());
			}
			catch (gtkutil::ModalProgressDialog::OperationAbortedException&)
			{
				gtkutil::errorDialog(
					_("Map loading cancelled"),
					GlobalMainFrame().getTopLevelWindow()
				);

				// Clear out the root node, otherwise we end up with half a map
				scene::NodeRemover remover;
				_root->traverse(remover);

				return false;
			}
		}*/

		// Create an entity node by parsing from the stream. If there is an
		// exception, display it and return
		try
		{
			parseEntity(tok);
		}
		catch (FailureException& e)
		{
			std::string text = (boost::format(_("Failed parsing entity %d:\n%s")) % _entityCount % e.what()).str();

			/*gtkutil::errorDialog(
				text + "\n\n" + e.what(),
				GlobalMainFrame().getTopLevelWindow()
			);

			// Clear out the root node, otherwise we end up with half a map
			scene::NodeRemover remover;
			_root->traverse(remover);*/

			//return false;

			// Re-throw with more text
			throw FailureException(text);
		}

		_entityCount++;
	}

	// EOF reached, success
}

void Doom3MapReader::parseMapVersion(parser::DefTokeniser& tok)
{
	// Parse the map version
    float version = 0;

    try
	{
        tok.assertNextToken(VERSION);
        version = boost::lexical_cast<float>(tok.nextToken());
    }
    catch (parser::ParseException& e)
	{
		// failed => quit
		globalErrorStream()
            << "[mapdoom3] Unable to parse map version: "
            << e.what() << std::endl;

		throw FailureException(_("Unable to parse map version (parse exception)."));
    }
    catch (boost::bad_lexical_cast& e)
	{
        globalErrorStream()
            << "[mapdoom3] Unable to parse map version: "
            << e.what() << std::endl;

		throw FailureException(_("Could not recognise map version number format."));
    }

	// Load the required version from the .game file
	xml::NodeList nodes = GlobalGameManager().currentGame()->getLocalXPath(RKEY_GAME_MAP_VERSION);
	assert(!nodes.empty());

	float requiredVersion = strToFloat(nodes[0].getAttributeValue("value"));

    // Check we have the correct version for this module
    if (version != requiredVersion)
	{
		std::string errMsg = (boost::format(_("Incorrect map version: required %f, found %f")) % requiredVersion % version).str();

        globalErrorStream() << errMsg << std::endl;

		throw FailureException(errMsg);
    }

	// success
}

void Doom3MapReader::parsePrimitive(parser::DefTokeniser& tok, const scene::INodePtr& parentEntity)
{
    // Update the dialog
    /*if (_dialog && _dialogEventLimiter.readyForEvent())
    {
        _dialog->setTextAndFraction(
            _dlgEntityText + "\nPrimitive " + sizetToStr(_primitiveCount),
			getProgressFraction()
        );
    }*/

    _primitiveCount++;

	std::string primitiveKeyword = tok.nextToken();

	// Get a parser for this keyword
	PrimitiveParserPtr parser = GlobalMapFormatManager().getPrimitiveParser(primitiveKeyword);

	if (parser == NULL)
	{
		throw FailureException("Unknown primitive type: " + primitiveKeyword);
	}

	// Try to parse the primitive, throwing exception if failed
	try
	{
		scene::INodePtr primitive = parser->parse(tok);

		if (!primitive)
		{
			std::string text = (boost::format(_("Primitive #%d: parse error")) % _primitiveCount).str();
			throw FailureException(text);
		}

		// Now add the primitive as a child of the entity
		_importFilter.addPrimitiveToEntity(primitive, parentEntity); 
	}
	catch (parser::ParseException& e)
	{
		// Translate ParseExceptions to FailureExceptions
		std::string text = (boost::format(_("Primitive #%d: parse exception %s")) % _primitiveCount % e.what()).str();
		throw FailureException(text);
	}

    /*if (Node_getEntity(parentEntity)->isContainer())
	{
        parentEntity->addChildNode(primitive);
    }*/
}

scene::INodePtr Doom3MapReader::createEntity(const EntityKeyValues& keyValues) {
    // Get the classname from the EntityKeyValues
    EntityKeyValues::const_iterator found = keyValues.find("classname");

    if (found == keyValues.end()) {
		throw std::runtime_error("Doom3MapReader::createEntity(): could not find classname.");
    }

    // Otherwise create the entity and add all of the properties
    std::string className = found->second;
	IEntityClassPtr classPtr = GlobalEntityClassManager().findClass(className);

	if (classPtr == NULL) {
		globalErrorStream() << "[mapdoom3]: Could not find entity class: "
			<< className << std::endl;

		// greebo: EntityClass not found, insert a brush-based one
		classPtr = GlobalEntityClassManager().findOrInsert(className, true);
	}

	// Create the actual entity node
    scene::INodePtr entity(GlobalEntityCreator().createEntity(classPtr));

	Entity* ent = Node_getEntity(entity);
	assert(ent != NULL); // entity cast must not fail

    for (EntityKeyValues::const_iterator i = keyValues.begin();
         i != keyValues.end();
         ++i)
    {
        ent->setKeyValue(i->first, i->second);
    }

    return entity;
}

void Doom3MapReader::parseEntity(parser::DefTokeniser& tok)
{
	// Set up the progress dialog text
	//_dlgEntityText = (boost::format(_("Loading entity %d")) % _entityCount).str();

    // Map of keyvalues for this entity
    EntityKeyValues keyValues;

    // The actual entity. This is initially null, and will be created when
    // primitives start or the end of the entity is reached
    scene::INodePtr entity;

	// Start parsing, first token must be an open brace
	tok.assertNextToken("{");

	std::string token = tok.nextToken();

	// Reset the primitive counter, we're starting a new entity
	_primitiveCount = 0;

	while (true)
	{
	    // Token must be either a key, a "{" to indicate the start of a
	    // primitive, or a "}" to indicate the end of the entity

	    if (token == "{") // PRIMITIVE
		{ 
			// Create the entity right now, if not yet done
			if (entity == NULL)
			{
				entity = createEntity(keyValues);
			}

			// Parse the primitive block, and pass the parent entity
			parsePrimitive(tok, entity);
	    }
	    else if (token == "}") // END OF ENTITY
		{
            // Create the entity if necessary and return it
	        if (entity == NULL)
			{
	            entity = createEntity(keyValues);
	        }

			break;
	    }
	    else // KEY
		{ 
	        std::string value = tok.nextToken();

	        // Sanity check (invalid number of tokens will get us out of sync)
	        if (value == "{" || value == "}")
			{
				std::string text = (boost::format(_("Parsed invalid value '%s' for key '%s'")) % value % token).str();
	            throw FailureException(text);
	        }

	        // Otherwise add the keyvalue pair to our map
	        keyValues.insert(EntityKeyValues::value_type(token, value));
	    }

	    // Get the next token
	    token = tok.nextToken();
	}

	// Insert the entity
	_importFilter.addEntity(entity);
}

#if 0
bool Doom3MapReader::checkEntityClass(const scene::INodePtr& entity) {
	// Obtain list of entityclasses to skip
	static xml::NodeList skipLst =
		GlobalRegistry().findXPath("debug/mapdoom3//discardEntityClass");

	// Obtain the entity class of this node
	IEntityClassConstPtr entityClass =
			Node_getEntity(entity)->getEntityClass();

	// Skip this entity class if it is in the list
	for (xml::NodeList::const_iterator i = skipLst.begin();
		 i != skipLst.end();
		 ++i)
	{
		if (i->getAttributeValue("value") == entityClass->getName()) {
			std::cout << "DEBUG: discarding entity class "
					  << entityClass->getName() << std::endl;
			return false;
		}
	}

	return true;
}

bool Doom3MapReader::checkEntityNum() {
	// Entity range XPath
	static xml::NodeList entityRange =
					GlobalRegistry().findXPath("debug/mapdoom3/entityRange");
	static xml::NodeList::iterator i = entityRange.begin();

	// Test the entity number is in the range
	if (i != entityRange.end()) {
		static std::size_t lower = strToSizet(i->getAttributeValue("start"));
		static std::size_t upper = strToSizet(i->getAttributeValue("end"));

		if (_entityCount < lower || _entityCount > upper) {
			std::cout << "DEBUG: Discarding entity " << _entityCount << ", out of range"
					  << std::endl;
			return false;
		}
	}
	return true;
}

void Doom3MapReader::insertEntity(const scene::INodePtr& entity) {
	// Abort if any of the tests fail
	if (_debug && (!checkEntityClass(entity) || !checkEntityNum())) {
		return;
	}

	// Insert the node into the given root
	_root->addChildNode(entity);
}

double Doom3MapReader::getProgressFraction()
{
	long readBytes = static_cast<long>(_inputStream.tellg());
	return static_cast<double>(readBytes) / _fileSize;
}
#endif
} // namespace map
