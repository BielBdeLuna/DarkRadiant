/*
Copyright (C) 2001-2006, William Joseph.
All Rights Reserved.

This file is part of GtkRadiant.

GtkRadiant is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

GtkRadiant is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GtkRadiant; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "parse.h"

#include <list>

#include "ientity.h"
#include "iregistry.h"
#include "brush/TexDef.h"
#include "ibrush.h"
#include "ipatch.h"
#include "ieclass.h"
#include "iscriplib.h"
#include "qerplugin.h"
#include "scenelib.h"
#include "traverselib.h"
#include "stringio.h"

#include "gtkutil/ModalProgressDialog.h"

#include <boost/lexical_cast.hpp>

inline MapImporter* Node_getMapImporter(scene::Node& node)
{
  return NodeTypeCast<MapImporter>::cast(node);
}


typedef std::list< std::pair<std::string, std::string> > KeyValues;

NodeSmartReference g_nullNode(NewNullNode());


NodeSmartReference Entity_create(EntityCreator& entityTable, IEntityClass* entityClass, const KeyValues& keyValues)
{
  scene::Node& entity(entityTable.createEntity(entityClass));
  for(KeyValues::const_iterator i = keyValues.begin(); i != keyValues.end(); ++i)
  {
    Node_getEntity(entity)->setKeyValue((*i).first.c_str(), (*i).second.c_str());
  }
  return NodeSmartReference(entity);
}

NodeSmartReference Entity_parseTokens(Tokeniser& tokeniser, EntityCreator& entityTable, const PrimitiveParser& parser, int index)
{
  NodeSmartReference entity(g_nullNode);
  KeyValues keyValues;
  std::string classname = "";

  int count_primitives = 0;
  while(1)
  {
    tokeniser.nextLine();
    const char* token = tokeniser.getToken();
    if(token == 0)
    {
      Tokeniser_unexpectedError(tokeniser, token, "#entity-token");
      return g_nullNode;
    }
    if (!strcmp(token, "}")) // end entity
    {
      if(entity == g_nullNode)
      {
        // entity does not have brushes
        entity = Entity_create(entityTable, GlobalEntityClassManager().findOrInsert(classname, false), keyValues);
      }
      return entity;
    }
    else if(!strcmp(token, "{")) // begin primitive
    {
      if(entity == g_nullNode)
      {
        // entity has brushes
        entity = Entity_create(entityTable, GlobalEntityClassManager().findOrInsert(classname, true), keyValues);
      }

      tokeniser.nextLine();

      NodeSmartReference primitive(parser.parsePrimitive(tokeniser));
      if(primitive == g_nullNode || !Node_getMapImporter(primitive)->importTokens(tokeniser))
      {
        globalErrorStream() << "brush " << count_primitives << ": parse error\n";
        return g_nullNode;
      }

		scene::Traversable* traversable = Node_getTraversable(entity);
		if(Node_getEntity(entity)->isContainer() 
		   && traversable != 0) 
		{
			// Try to insert the primitive into the entity. This may throw an exception if
			// the entity should not contain brushes (e.g. a func_static with a model key)
	        try {
	        	traversable->insert(primitive);
	        }
	        catch (std::runtime_error e) {
	        	// Warn, but just ignore the brush
	        	globalErrorStream() << "[mapdoom3] Entity " << index << " failed to accept brush, discarding\n";
	        }
		}
		else {
			globalErrorStream() << "entity " << index << ": type " << classname << ": discarding brush " << count_primitives << "\n";
		}
		
		++count_primitives;
    }
    else // epair
    {
      std::string key(token);
      token = tokeniser.getToken();
      if(token == 0)
      {
        Tokeniser_unexpectedError(tokeniser, token, "#epair-value");
        return g_nullNode;
      }
      keyValues.push_back(KeyValues::value_type(key, token));
      if(key == "classname")
      {
        classname = token;
      }
    }
  }
  // unreachable code
  return g_nullNode;
}

// Insert an entity node into the scenegraph, checking if the debug flags
// exclude this class type
void checkInsert(NodeSmartReference node, scene::Node& root) {

	// Static "debug" flag obtained from the registry
	static bool _debug = GlobalRegistry().get("user/debug") == "1";
	if (_debug) {

		// Obtain the entity class of this node
		const IEntityClass& entityClass = 
				Node_getEntity(node)->getEntityClass();

		// Obtain list of entityclasses to skip
		xml::NodeList skipLst = 
			GlobalRegistry().findXPath("debug/mapdoom3//discardEntityClass");

		// Skip this entity class if it is in the list
		for (xml::NodeList::const_iterator i = skipLst.begin();
			 i != skipLst.end();
			 ++i)
		{
			if (i->getAttributeValue("value") == entityClass.getName()) {
				std::cout << "DEBUG: discarding entity class " 
						  << entityClass.getName() << std::endl;
				return;
			}
		}
	}
	
	// Insert the node into the scenegraph root
	Node_getTraversable(root)->insert(node);
}
		
	
	

void Map_Read(scene::Node& root, 
			  Tokeniser& tokeniser, 
			  EntityCreator& entityTable, 
			  const PrimitiveParser& parser)
{
	// Create an info display panel to track load progress
	gtkutil::ModalProgressDialog dialog(GlobalRadiant().getMainWindow(),
										"Loading map");

	// Read each entity in the map, until EOF is reached
	for (int entCount = 0; ; entCount++) {

		// Update the dialog text
		dialog.setText("Loading entity " + boost::lexical_cast<std::string>(entCount));

		// Check for end of file
		tokeniser.nextLine();
		if (!tokeniser.getToken()) // { or 0
			break;

		// Create an entity node by parsing from the stream
		NodeSmartReference entity(Entity_parseTokens(tokeniser, entityTable, parser, entCount));

		if(entity == g_nullNode) {
			globalErrorStream() << "entity " << entCount << ": parse error\n";
			return;
		}
		
		// Insert the entity
		checkInsert(entity, root);
	}
}
