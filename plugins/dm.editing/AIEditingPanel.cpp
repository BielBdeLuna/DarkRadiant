#include "AIEditingPanel.h"

#include "i18n.h"
#include "iselection.h"
#include "ieclass.h"
#include "iradiant.h"
#include "igroupdialog.h"
#include "iuimanager.h"
#include "selectionlib.h"
#include "scenelib.h"

#include <gtkmm/button.h>
#include <gtkmm/image.h>
#include "gtkutil/LeftAlignedLabel.h"
#include "gtkutil/LeftAlignment.h"

#include "SpawnargLinkedCheckbox.h"

namespace ui
{

AIEditingPanel::AIEditingPanel() :
	_queueUpdate(true),
	_entity(NULL)
{
	constructWidgets();

	GlobalRadiant().signal_radiantShutdown().connect(
		sigc::mem_fun(*this, &AIEditingPanel::onRadiantShutdown)
	);	

	_selectionChangedSignal = GlobalSelectionSystem().signal_selectionChanged().connect(
		sigc::mem_fun(*this, &AIEditingPanel::onSelectionChanged)
	);
}

void AIEditingPanel::constructWidgets()
{
	set_border_width(12);
	set_spacing(6);

	// Populate the map with all the widgets linked to 1/0 spawnargs
	_checkboxes["canOperateDoors"] = Gtk::manage(new SpawnargLinkedCheckbox("Can operate doors", "canOperateDoors"));

	{
		// Appearance widgets
		pack_start(*Gtk::manage(new gtkutil::LeftAlignedLabel(
			std::string("<b>") + _("Appearance") + "</b>")), false, false, 0);

		// Skin
		Gtk::HBox* skinRow = Gtk::manage(new Gtk::HBox(false, 6));

		skinRow->pack_start(*Gtk::manage(new gtkutil::LeftAlignedLabel(_("Skin: "))), false, false, 0);
		skinRow->pack_start(*Gtk::manage(new gtkutil::LeftAlignedLabel("skins/uriels")), true, true, 0);

		// Create the skin browse button
		Gtk::Button* browseButton = Gtk::manage(new Gtk::Button(_("Choose skin...")));
		browseButton->set_image(*Gtk::manage(new Gtk::Image(GlobalUIManager().getLocalPixbuf("icon_skin.png"))));
		// TODO: browseButton->signal_clicked().connect(sigc::mem_fun(*this, &SkinPropertyEditor::_onBrowseButton));
		skinRow->pack_start(*browseButton, false, false, 0);

		pack_start(*Gtk::manage(new gtkutil::LeftAlignment(*skinRow, 18, 1.0)), false, false, 0);

		// Head
		Gtk::HBox* headRow = Gtk::manage(new Gtk::HBox(false, 6));

		headRow->pack_start(*Gtk::manage(new gtkutil::LeftAlignedLabel(_("Head: "))), false, false, 0);
		headRow->pack_start(*Gtk::manage(new gtkutil::LeftAlignedLabel("heads/eric")), true, true, 0);

		// Create the skin browse button
		Gtk::Button* headBrowseButton = Gtk::manage(new Gtk::Button(_("Choose AI head...")));
		headBrowseButton->set_image(*Gtk::manage(new Gtk::Image(GlobalUIManager().getLocalPixbuf("icon_model.png"))));
		// TODO: browseButton->signal_clicked().connect(sigc::mem_fun(*this, &SkinPropertyEditor::_onBrowseButton));
		headRow->pack_start(*headBrowseButton, false, false, 0);

		pack_start(*Gtk::manage(new gtkutil::LeftAlignment(*headRow, 18, 1.0)), false, false, 0);
	}

	{
		// Abilities widgets
		pack_start(*Gtk::manage(new gtkutil::LeftAlignedLabel(
			std::string("<b>") + _("Abilities") + "</b>")), false, false, 0);
		
		pack_start(*Gtk::manage(new gtkutil::LeftAlignment(*_checkboxes["canOperateDoors"], 18, 1.0)), false, false, 0);
	}

	show_all();
}

AIEditingPanel& AIEditingPanel::Instance()
{
	AIEditingPanelPtr& instance = InstancePtr();

	if (!instance)
	{
		instance.reset(new AIEditingPanel);
	}

	return *instance;
}

void AIEditingPanel::Shutdown()
{
	if (InstancePtr() != NULL)
	{
		//Instance().shutdown();
		InstancePtr().reset();
	}
} 

void AIEditingPanel::onRadiantStartup()
{
	// Add the Media Browser page
	GlobalGroupDialog().addPage(
    	"aieditingpanel",	// name
    	"AI", // tab title
    	"icon_model.png", // tab icon
    	Instance(), // page widget
    	_("AI"),
		"mediabrowser"
	);

	GlobalUndoSystem().addObserver(InstancePtr().get());
}

void AIEditingPanel::onRadiantShutdown()
{
	GlobalUndoSystem().removeObserver(this);

	_selectionChangedSignal.disconnect();
}

Entity* AIEditingPanel::getEntityFromSelection()
{
	Entity* entity = NULL;

	const SelectionInfo& info = GlobalSelectionSystem().getSelectionInfo();

	if (info.entityCount == 1 && info.totalCount == info.entityCount)
	{
		GlobalSelectionSystem().foreachSelected([&] (const scene::INodePtr& node)
		{
			Entity* candidate = Node_getEntity(node);

			if (candidate->isOfType("atdm:ai_base"))
			{
				entity = candidate;
			}
		});
	}

	return entity;
}

void AIEditingPanel::updatePanelSensitivity()
{
	set_sensitive(_entity != NULL);
}

void AIEditingPanel::onKeyInsert(const std::string& key, EntityKeyValue& value)
{
	// On entity key change, we load all values afresh
	updateWidgetsFromSelection();
}

void AIEditingPanel::onKeyChange(const std::string& key, const std::string& val)
{
	// On entity key change, we load all values afresh
	updateWidgetsFromSelection();
}

void AIEditingPanel::onKeyErase(const std::string& key, EntityKeyValue& value)
{
	// On entity key change, we load all values afresh
	updateWidgetsFromSelection();
}

void AIEditingPanel::postUndo()
{
	updateWidgetsFromSelection();
}

void AIEditingPanel::postRedo()
{
	updateWidgetsFromSelection();
}

void AIEditingPanel::updateWidgetsFromSelection()
{
	// Write the entity object to each checkbox even if it is NULL
	// This will update the widget state from the entity's properties
	std::for_each(_checkboxes.begin(), _checkboxes.end(), [&] (CheckboxMap::value_type& pair)
	{
		pair.second->setEntity(_entity);
	});
}

void AIEditingPanel::rescanSelection()
{
	_queueUpdate = false;

	// Load the new entity from the selection
	_entity = getEntityFromSelection();

	if (_entity != NULL)
	{
		_entity->attachObserver(this);
	}

	updatePanelSensitivity();
	updateWidgetsFromSelection();
}

void AIEditingPanel::onSelectionChanged(const Selectable& selectable)
{
	// Immediately disconnect from the current entity in any case
	if (_entity != NULL)
	{
		_entity->detachObserver(this);
	}

	if (GlobalGroupDialog().getPage() == this)
	{
		rescanSelection();
	}
	else
	{
		// We don't scan the selection if the page is not visible
		_queueUpdate = true;
	}
}

bool AIEditingPanel::on_expose_event(GdkEventExpose* event)
{
	if (_queueUpdate)
	{
		// Wake up from sleep mode and inspect the current selection
		rescanSelection();
	}

	// Propagate the call
	return Gtk::VBox::on_expose_event(event);
}

AIEditingPanelPtr& AIEditingPanel::InstancePtr()
{
	static AIEditingPanelPtr _instancePtr;
	return _instancePtr;
} 

} // namespace