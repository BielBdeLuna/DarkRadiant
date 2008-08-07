#include "ComponentsDialog.h"
#include "Objective.h"
#include "ce/ComponentEditorFactory.h"
#include "util/TwoColumnTextCombo.h"

#include "gtkutil/ScrolledFrame.h"
#include "gtkutil/LeftAlignedLabel.h"
#include "gtkutil/RightAlignment.h"
#include "gtkutil/TextColumn.h"
#include "gtkutil/TreeModel.h"

#include <gtk/gtk.h>
#include <boost/algorithm/string/predicate.hpp>

namespace objectives
{

/* CONSTANTS */

namespace {

	const char* DIALOG_TITLE = "Edit conditions";

	// Widget enum
	enum {
		WIDGET_EDIT_PANEL,
		WIDGET_TYPE_COMBO,
		WIDGET_STATE_FLAG,
		WIDGET_IRREVERSIBLE_FLAG,
		WIDGET_INVERTED_FLAG,
		WIDGET_COMPEDITOR_PANEL
	};
	
}

// Main constructor
ComponentsDialog::ComponentsDialog(GtkWindow* parent, Objective& objective)
: gtkutil::BlockingTransientWindow(DIALOG_TITLE, parent),
  _objective(objective),
  _componentList(gtk_list_store_new(2, G_TYPE_INT, G_TYPE_STRING))
{
	// Dialog contains list view, edit panel and buttons
	GtkWidget* vbx = gtk_vbox_new(FALSE, 12);
	gtk_box_pack_start(GTK_BOX(vbx), createListView(), TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbx), createEditPanel(), FALSE, FALSE, 0);
	gtk_box_pack_start(
		GTK_BOX(vbx), createComponentEditorPanel(), TRUE, TRUE, 0
	);
	gtk_box_pack_start(GTK_BOX(vbx), gtk_hseparator_new(), FALSE, FALSE, 0);
	gtk_box_pack_end(GTK_BOX(vbx), createButtons(), FALSE, FALSE, 0);
	
	// Populate the list of components
	populateComponents();
	
	// Add contents to main window
	gtk_container_set_border_width(GTK_CONTAINER(getWindow()), 12);
	gtk_container_add(GTK_CONTAINER(getWindow()), vbx);
}

// Create list view
GtkWidget* ComponentsDialog::createListView() {

	// Create tree view and connect selection changed callback	
	GtkWidget* tv = 
		gtk_tree_view_new_with_model(GTK_TREE_MODEL(_componentList));
	_componentSel = gtk_tree_view_get_selection(GTK_TREE_VIEW(tv));
	g_signal_connect(G_OBJECT(_componentSel), "changed",
					 G_CALLBACK(_onSelectionChanged), this);

	// Number column
	gtk_tree_view_append_column(
		GTK_TREE_VIEW(tv), gtkutil::TextColumn("#", 0, false));
	gtk_tree_view_append_column(
		GTK_TREE_VIEW(tv), gtkutil::TextColumn("Type", 1, false));

	// Create Add and Delete buttons for components
	GtkWidget* addButton = gtk_button_new_from_stock(GTK_STOCK_ADD);
	GtkWidget* delButton = gtk_button_new_from_stock(GTK_STOCK_DELETE);
	g_signal_connect(G_OBJECT(addButton), "clicked", 
					 G_CALLBACK(_onAddComponent), this);
	g_signal_connect(G_OBJECT(delButton), "clicked", 
					 G_CALLBACK(_onDeleteComponent), this);
	
	GtkWidget* buttonsBox = gtk_vbox_new(FALSE, 6);
	gtk_box_pack_start(GTK_BOX(buttonsBox), addButton, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(buttonsBox), delButton, TRUE, TRUE, 0);
	
	// Put the buttons box next to the list view
	GtkWidget* hbx = gtk_hbox_new(FALSE, 6);
	gtk_box_pack_start(GTK_BOX(hbx), gtkutil::ScrolledFrame(tv), TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbx), buttonsBox, FALSE, FALSE, 0);

	return hbx;	
}

// Create edit panel
GtkWidget* ComponentsDialog::createEditPanel() {
	
	// Table
	GtkWidget* table = gtk_table_new(2, 2, FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table), 12);
	gtk_table_set_col_spacings(GTK_TABLE(table), 12);
	gtk_widget_set_sensitive(table, FALSE); // disabled until selection
	
	// Component type dropdown
	GtkWidget* cmb = util::TwoColumnTextCombo();
	_widgets[WIDGET_TYPE_COMBO] = cmb;

	// Pack dropdown into table
	gtk_table_attach(GTK_TABLE(table), 
					 gtkutil::LeftAlignedLabel("<b>Type</b>"),
					 0, 1, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
	g_signal_connect(G_OBJECT(_widgets[WIDGET_TYPE_COMBO]), "changed",
					 G_CALLBACK(_onTypeChanged), this);
	gtk_table_attach_defaults(GTK_TABLE(table),
							  _widgets[WIDGET_TYPE_COMBO],
							  1, 2, 0, 1);
							  
	// Populate the combo box. The set is in ID order.
	for (ComponentTypeSet::const_iterator i = ComponentType::SET_ALL().begin();
		 i != ComponentType::SET_ALL().end();
		 ++i)
	{
		GtkListStore* ls = GTK_LIST_STORE(
				gtk_combo_box_get_model(GTK_COMBO_BOX(cmb))
		);
		GtkTreeIter iter;
		gtk_list_store_append(ls, &iter);
		gtk_list_store_set(
			ls, &iter, 
			0, i->getDisplayName().c_str(), 
			1, i->getName().c_str(),
			-1
		);	
	}
	
	// Flags hbox
	_widgets[WIDGET_STATE_FLAG] = 
		gtk_check_button_new_with_label("Satisfied at start");
	_widgets[WIDGET_IRREVERSIBLE_FLAG] = 
		gtk_check_button_new_with_label("Irreversible");  
	_widgets[WIDGET_INVERTED_FLAG] =
		gtk_check_button_new_with_label("Boolean NOT");  
	
	GtkWidget* flagsBox = gtk_hbox_new(FALSE, 12);
	gtk_box_pack_start(GTK_BOX(flagsBox), _widgets[WIDGET_STATE_FLAG],
					   FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(flagsBox), _widgets[WIDGET_IRREVERSIBLE_FLAG],
					   FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(flagsBox), _widgets[WIDGET_INVERTED_FLAG],
					   FALSE, FALSE, 0);
	
	gtk_table_attach(GTK_TABLE(table), 
					 gtkutil::LeftAlignedLabel("<b>Flags</b>"),
					 0, 1, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
	gtk_table_attach(GTK_TABLE(table), flagsBox, 1, 2, 1, 2, 
					 GTK_FILL, GTK_FILL, 0, 0);
	
	// Save and return the panel table
	_widgets[WIDGET_EDIT_PANEL] = table;
	return table;
}

// ComponentEditor panel
GtkWidget* ComponentsDialog::createComponentEditorPanel()
{
    // Invisible frame to contain the ComponentEditor
	_widgets[WIDGET_COMPEDITOR_PANEL] = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(
        GTK_FRAME(_widgets[WIDGET_COMPEDITOR_PANEL]), GTK_SHADOW_NONE
    );
    gtk_container_set_border_width(
        GTK_CONTAINER(_widgets[WIDGET_COMPEDITOR_PANEL]), 6
    );
    
    // Visible frame 
    GtkContainer* borderFrame = GTK_CONTAINER(gtk_frame_new(NULL));
    gtk_container_add(borderFrame, _widgets[WIDGET_COMPEDITOR_PANEL]);

	return GTK_WIDGET(borderFrame);
}

// Create buttons
GtkWidget* ComponentsDialog::createButtons() {
	
	GtkWidget* hbx = gtk_hbox_new(TRUE, 6);

	GtkWidget* closeButton = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
	
	g_signal_connect(
		G_OBJECT(closeButton), "clicked", G_CALLBACK(_onClose), this
    );
	
	gtk_box_pack_end(GTK_BOX(hbx), closeButton, TRUE, TRUE, 0);

	return gtkutil::RightAlignment(hbx);
}

// Populate the component list
void ComponentsDialog::populateComponents() {
	
	// Clear the list store
	gtk_list_store_clear(_componentList);
	
	// Add components from the Objective to the list store
	Objective::ComponentMap& components = _objective.components;
	for (Objective::ComponentMap::const_iterator i = components.begin();
		 i != components.end();
		 ++i)
	{
		GtkTreeIter iter;
		gtk_list_store_append(_componentList, &iter);
		gtk_list_store_set(_componentList, &iter, 
						   0, i->first, 
						   1, i->second.getString().c_str(),
						   -1);	
	}
	
}

// Populate the edit panel
void ComponentsDialog::populateEditPanel(int index) {
	// Get the component
	Component& comp = _objective.components[index];
	
	// Set the flags
	gtk_toggle_button_set_active(
		GTK_TOGGLE_BUTTON(_widgets[WIDGET_STATE_FLAG]), 
		comp.isSatisfied() ? TRUE : FALSE
	);
	gtk_toggle_button_set_active(
		GTK_TOGGLE_BUTTON(_widgets[WIDGET_IRREVERSIBLE_FLAG]), 
		comp.isIrreversible() ? TRUE : FALSE
	);
	gtk_toggle_button_set_active(
		GTK_TOGGLE_BUTTON(_widgets[WIDGET_INVERTED_FLAG]), 
		comp.isInverted() ? TRUE : FALSE
	);
	
    // Change the type combo if necessary. Since the combo box was populated in
    // ID order, we can simply use our ComponentType's ID as an index.
    GtkComboBox* typeCombo = GTK_COMBO_BOX(_widgets[WIDGET_TYPE_COMBO]); 
    if (gtk_combo_box_get_active(typeCombo) != comp.getType().getId())
    {
        // Change the combo selection (this triggers a change of the
        // ComponentEditor panel)
        gtk_combo_box_set_active(typeCombo, comp.getType().getId());
    }
    else 
    {
        // Update the ComponentEditor ourselves, since the new Component has the
        // same type but we still want to refresh the panel with the new
        // contents
        changeComponentEditor(comp);
    }
}

// Get selected component index
int ComponentsDialog::getSelectedIndex() {
	// Get the selection if valid
	GtkTreeModel* model;
	GtkTreeIter iter;
	if (gtk_tree_selection_get_selected(_componentSel, &model, &iter)) {

		// Valid selection, return the contents of the index column
		int idx;
		gtk_tree_model_get(model, &iter, 0, &idx, -1);
		
		return idx;
	}
	else {
		return -1;
	}
}

// Change component editor
void ComponentsDialog::changeComponentEditor(Component& compToEdit) {

	// greebo: Get a new component editor, any previous one will auto-destroy and
	// remove its widget from the container.
	_componentEditor = ce::ComponentEditorFactory::create(
        compToEdit.getType().getName(), compToEdit
	);

	if (_componentEditor != NULL) {
		// Get the widget from the ComponentEditor and show it
		GtkWidget* editor = _componentEditor->getWidget();
		gtk_widget_show_all(editor);
		
		// Pack the widget into the containing frame
		gtk_container_add(
			GTK_CONTAINER(_widgets[WIDGET_COMPEDITOR_PANEL]),
			editor
		);
	}
}

// Safely write the ComponentEditor contents to the Component
void ComponentsDialog::checkWriteComponent() {
	if (_componentEditor) {
        _componentEditor->writeToComponent();
	}
}

/* GTK CALLBACKS */

// Close button
void ComponentsDialog::_onClose(GtkWidget* w, ComponentsDialog* self) {
    self->checkWriteComponent();
	self->destroy();
}

// Selection changed
void ComponentsDialog::_onSelectionChanged(GtkTreeSelection* sel,
										   ComponentsDialog* self)
{
    // Save the existing ComponentEditor contents if req'd
    self->checkWriteComponent();

	// Get the selection if valid
	GtkTreeModel* model;
	GtkTreeIter iter;
	if (!gtk_tree_selection_get_selected(sel, &model, &iter)) 
    {
		// Disable the edit panel and remove the ComponentEditor
		gtk_widget_set_sensitive(self->_widgets[WIDGET_EDIT_PANEL], FALSE);
        self->_componentEditor = objectives::ce::ComponentEditorPtr();
	}
	else {
		// Otherwise populate edit panel with the current component index
		int component;
		gtk_tree_model_get(model, &iter, 0, &component, -1); 
		
		self->populateEditPanel(component);

		// Enable the edit panel
		gtk_widget_set_sensitive(self->_widgets[WIDGET_EDIT_PANEL], TRUE);
	}
}

// Add a new component
void ComponentsDialog::_onAddComponent(GtkWidget* w, ComponentsDialog* self) 
{
	Objective::ComponentMap& components = self->_objective.components;
	
	// Find an unused component number
	for (int idx = 0; idx < INT_MAX; ++idx) {
		if (components.find(idx) == components.end()) {
			// Unused, add a new component here
			Component comp;
			components.insert(std::make_pair(idx, comp));
			break;
		}
	}
	
	// Refresh the component list
	self->populateComponents();
}

// Remove a component
void ComponentsDialog::_onDeleteComponent(GtkWidget* w, ComponentsDialog* self) 
{
	// Delete the selected component
	int idx = self->getSelectedIndex();
	if (idx != -1) 
    {
        // Remove the selection first, so our selection-changed callback does not
        // attempt to writeToComponent() after the Component has already been deleted
        gtk_tree_selection_unselect_all(self->_componentSel);

        // Erase the actual component
		self->_objective.components.erase(idx);
	}
	
	// Refresh the list
	self->populateComponents();		
}

// Type combo changed
void ComponentsDialog::_onTypeChanged(GtkWidget* w, ComponentsDialog* self) 
{
	// Get the current selection
	GtkTreeIter iter;
	gtk_combo_box_get_active_iter(GTK_COMBO_BOX(w), &iter);
	std::string selectedText = gtkutil::TreeModel::getString(
			gtk_combo_box_get_model(GTK_COMBO_BOX(w)),
			&iter,
			1
	); 
	
	// Update the Objective object. The selected index must be valid, since the
	// edit panel is only sensitive if a component is selected
	int idx = self->getSelectedIndex();
	assert(idx >= 0);
	Component& comp(self->_objective.components[idx]);

    // Store the newly-selected type in the Component
	comp.setType(ComponentType::getComponentType(selectedText));
	
    // Change the ComponentEditor
    self->changeComponentEditor(comp);

	// Update the components list with the new display string
	GtkTreeModel* model;
	GtkTreeIter compIter;
	gtk_tree_selection_get_selected(self->_componentSel, &model, &compIter);
	gtk_list_store_set(
		GTK_LIST_STORE(model), &compIter, 1, comp.getString().c_str(), -1
	);
}

} // namespace objectives
