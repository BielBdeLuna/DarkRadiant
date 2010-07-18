#include "StimTypes.h"

#include "iuimanager.h"
#include "itextstream.h"
#include "iregistry.h"
#include "string/string.h"
#include "gtkutil/TreeModel.h"
#include "entitylib.h"
#include "SREntity.h"
#include "i18n.h"
#include <gtk/gtk.h>
#include <boost/algorithm/string/predicate.hpp>

	namespace {
		const std::string RKEY_STIM_DEFINITIONS = 
			"game/stimResponseSystem/stims//stim";
		const std::string RKEY_STORAGE_ECLASS = 
			"game/stimResponseSystem/customStimStorageEClass";
		const std::string RKEY_STORAGE_PREFIX = 
			"game/stimResponseSystem/customStimKeyPrefix";
		const std::string RKEY_LOWEST_CUSTOM_STIM_ID = 
			"game/stimResponseSystem/lowestCustomStimId";
		const std::string RKEY_SHOW_STIM_TYPE_IDS = 
			"user/ui/stimResponseEditor/showStimTypeIDs";
		
		/* greebo: Finds an entity with the given classname
		 */
		Entity* findEntityByClass(const std::string& className) {
			// Instantiate a walker to find the entity
			EntityNodeFindByClassnameWalker walker(className);
			
			// Walk the scenegraph
			Node_traverseSubgraph(GlobalSceneGraph().root(), walker);
			
			return walker.getEntity();
		}
		
		// Helper visitor class to remove custom stim definitions from 
		// the storage entity. First, all the keys are gathered and
		// on destruction the keys are deleted. The deletion may not
		// happen during the visit process (due to iterators becoming invalid).
		class CustomStimRemover :
			public Entity::Visitor
		{
			// This list will be populated with all the keys that 
			// should be removed.
			typedef std::vector<std::string> RemoveList;
			RemoveList _removeList;
			
			Entity* _entity;
			
		public:
			CustomStimRemover(Entity* entity) :
				_entity(entity)
			{}
		
			~CustomStimRemover() {
				// Delete all the keys that are tagged for deletion
				for (unsigned int i = 0; i < _removeList.size(); i++) {
					_entity->setKeyValue(_removeList[i], "");
				}
			}
		
			void visit(const std::string& key, const std::string& value) {
				std::string prefix = GlobalRegistry().get(RKEY_STORAGE_PREFIX);
				
				if (boost::algorithm::starts_with(key, prefix)) {
					// We have a match, add the key to the removal list
					_removeList.push_back(key);
				}
			}
		};
	}

StimTypes::StimTypes() {
	// Create a new liststore
	_listStore = gtk_list_store_new(ST_NUM_COLS, 
									G_TYPE_INT, 
									G_TYPE_STRING, 
									GDK_TYPE_PIXBUF,
									G_TYPE_STRING,
									G_TYPE_STRING,
									G_TYPE_BOOLEAN);	
}

void StimTypes::reload() {
	_stimTypes.clear();
	gtk_list_store_clear(_listStore);
	
	// Find all the relevant nodes
	xml::NodeList stimNodes = GlobalRegistry().findXPath(RKEY_STIM_DEFINITIONS);
	
	for (unsigned int i = 0; i < stimNodes.size(); i++) {
		// Add the new stim type
		add(strToInt(stimNodes[i].getAttributeValue("id")), 
			stimNodes[i].getAttributeValue("name"),
			stimNodes[i].getAttributeValue("caption"),
			stimNodes[i].getAttributeValue("description"),
			stimNodes[i].getAttributeValue("icon"),
			false	// non-custom stim
		);
	}
	
	// Load the custom stims from the storage entity
	std::string storageEClass = GlobalRegistry().get(RKEY_STORAGE_ECLASS);
	Entity* storageEntity = findEntityByClass(storageEClass);
	
	if (storageEntity != NULL) {
		// Visit each keyvalue with the <self> class as visitor 
		storageEntity->forEachKeyValue(*this);
	}
}

void StimTypes::save() {
	// Find the storage entity
	std::string storageEClass = GlobalRegistry().get(RKEY_STORAGE_ECLASS);
	Entity* storageEntity = findEntityByClass(storageEClass);
	
	if (storageEntity != NULL) {
		std::string prefix = GlobalRegistry().get(RKEY_STORAGE_PREFIX);
		
		// Clean the storage entity from any previous definitions
		{
			// Instantiate a visitor to gather the keys to delete
			CustomStimRemover remover(storageEntity);	
			// Visit each keyvalue with the <self> class as visitor 
			storageEntity->forEachKeyValue(remover);
			// Scope ends here, the keys are deleted now
			// as the CustomStimRemover gets destructed
		}
		
		// Now store all custom stim types to the storage entity
		for (StimTypeMap::iterator i = _stimTypes.begin(); i != _stimTypes.end(); i++) {
			StimType& s = i->second;
			std::string idStr = intToStr(i->first);
			
			if (s.custom) {
				// spawnarg is something like "editor_dr_stim_1002" => "MyStim" 
				storageEntity->setKeyValue(prefix + idStr, s.caption);
			}
		}
	}
}

void StimTypes::remove(int id) {
	StimTypeMap::iterator found = _stimTypes.find(id);
	
	if (found != _stimTypes.end()) {
		// Erase the item from the map
		_stimTypes.erase(found);
		
		// Erase the row in the liststore
		GtkTreeIter iter = getIterForId(id);
		gtk_list_store_remove(_listStore, &iter);
	}
}

GtkTreeIter StimTypes::getIterForId(int id) {
	// Setup the selectionfinder to search for the id
	gtkutil::TreeModel::SelectionFinder finder(id, ST_ID_COL);

	gtk_tree_model_foreach(
		GTK_TREE_MODEL(_listStore),
		gtkutil::TreeModel::SelectionFinder::forEach,
		&finder
	);
	
	GtkTreeIter iter;
	memset(&iter, 0, sizeof(GtkTreeIter));
	
	if (finder.getPath() != NULL)
	{
		iter = finder.getIter();
	}
	
	return iter;
}

void StimTypes::setStimTypeCaption(int id, const std::string& caption) {
	StimTypeMap::iterator found = _stimTypes.find(id);
	
	if (found != _stimTypes.end()) {
		_stimTypes[id].caption = caption;
		
		// Combine the ID and the caption
		std::string captionPlusId = caption;
		bool showStimTypeIds = GlobalRegistry().get(RKEY_SHOW_STIM_TYPE_IDS) == "1";
		captionPlusId += showStimTypeIds ? " (" + intToStr(id) + ")" : "";
	
		// Update the list store
		GtkTreeIter iter = getIterForId(id);
		gtk_list_store_set(_listStore, &iter, 
							ST_CAPTION_COL, caption.c_str(),
							ST_CAPTION_PLUS_ID_COL, captionPlusId.c_str(),
							-1);
	}
}

void StimTypes::add(int id, 
					const std::string& name,
					const std::string& caption,
					const std::string& description,
					const std::string& icon,
					bool custom)
{
	StimType newStimType;
	newStimType.name = name;
	newStimType.caption = caption;
	newStimType.description = description;
	newStimType.icon = icon;
	newStimType.custom = custom;
	
	// Add the stim to the map
	_stimTypes[id] = newStimType;
	
	GtkTreeIter iter;
	
	// Combine the ID and the caption
	std::string captionPlusId = caption;
	bool showStimTypeIds = GlobalRegistry().get(RKEY_SHOW_STIM_TYPE_IDS) == "1";
	captionPlusId += showStimTypeIds ? " (" + intToStr(id) + ")" : "";
	
	gtk_list_store_append(_listStore, &iter);
	gtk_list_store_set(_listStore, &iter, 
						ST_ID_COL, id,
						ST_CAPTION_COL, _stimTypes[id].caption.c_str(),
						ST_CAPTION_PLUS_ID_COL, captionPlusId.c_str(),
						ST_ICON_COL, GlobalUIManager().getLocalPixbufWithMask(newStimType.icon),
						ST_NAME_COL, _stimTypes[id].name.c_str(),
						ST_CUSTOM_COL, custom,
						-1);
}

void StimTypes::visit(const std::string& key, const std::string& value) {
	std::string prefix = GlobalRegistry().get(RKEY_STORAGE_PREFIX);
	int lowestCustomId = GlobalRegistry().getInt(RKEY_LOWEST_CUSTOM_STIM_ID);
	
	if (boost::algorithm::starts_with(key, prefix)) {
		// Extract the stim name from the key (the part after the prefix) 
		std::string idStr = key.substr(prefix.size());
		int id = strToInt(idStr);
		std::string stimCaption= value;
		
		if (id < lowestCustomId) {
			globalErrorStream() << "Warning: custom stim Id " << id << " is lower than " 
								<< lowestCustomId << "\n";
		}
		
		// Add this as new stim type
		add(id,
			idStr,	// The name is the id in string format: e.g. "1002"
			stimCaption,	// The caption
			_("Custom Stim"),
			ICON_CUSTOM_STIM,
			true	// custom stim
		);
	}
}

StimTypes::operator GtkListStore* () {
	return _listStore;
}

StimTypeMap& StimTypes::getStimMap() {
	return _stimTypes;
}

int StimTypes::getFreeCustomStimId() {
	int freeId = GlobalRegistry().getInt(RKEY_LOWEST_CUSTOM_STIM_ID);
	
	StimTypeMap::iterator found = _stimTypes.find(freeId);
	while (found != _stimTypes.end()) {
		freeId++;
		found = _stimTypes.find(freeId);
	}
	
	return freeId;
}

GtkTreeIter StimTypes::getIterForName(const std::string& name) {
	// Setup the selectionfinder to search for the name string
	gtkutil::TreeModel::SelectionFinder finder(name, ST_NAME_COL);
	
	gtk_tree_model_foreach(
		GTK_TREE_MODEL(_listStore), 
		gtkutil::TreeModel::SelectionFinder::forEach, 
		&finder
	);
	
	return finder.getIter();
}

StimType StimTypes::get(int id) {
	StimTypeMap::iterator i = _stimTypes.find(id);
	
	if (i != _stimTypes.end()) {
		return i->second;
	}
	else {
		return _emptyStimType;
	}
}

std::string StimTypes::getFirstName() {
	StimTypeMap::iterator i = _stimTypes.begin();
	
	return (i != _stimTypes.end()) ? i->second.name : "noname";
}

StimType StimTypes::get(const std::string& name) {
	for (StimTypeMap::iterator i = _stimTypes.begin(); i!= _stimTypes.end(); i++) {
		if (i->second.name == name) {
			return i->second;
		}
	}
	// Nothing found
	return _emptyStimType;
}
