#include "StimEditor.h"

#include <gtk/gtk.h>
#include "gtkutil/LeftAlignment.h"
#include "gtkutil/RightAlignedLabel.h"
#include "gtkutil/LeftAlignedLabel.h"
#include "gtkutil/ScrolledFrame.h"
#include "gtkutil/TreeModel.h"
#include "gtkutil/StockIconMenuItem.h"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/classification.hpp>
#include "string/string.h" 

#include "i18n.h"
#include "SREntity.h"

namespace ui {

	namespace {
		// Needed for boost::algorithm::split
		typedef std::vector<std::string> StringParts;
	}

StimEditor::StimEditor(StimTypes& stimTypes) :
	ClassEditor(stimTypes)
{
	populatePage();
	
	// Setup the context menu items and connect them to the callbacks
	createContextMenu();
}

void StimEditor::populatePage() {
	GtkWidget* srHBox = gtk_hbox_new(FALSE, 12);
	gtk_box_pack_start(GTK_BOX(_pageVBox), GTK_WIDGET(srHBox), TRUE, TRUE, 0);
	
	GtkWidget* vbox = gtk_vbox_new(FALSE, 6);
	gtk_box_pack_start(GTK_BOX(vbox), 
		gtkutil::ScrolledFrame(_list), TRUE, TRUE, 0);
	
	// Create the type selector plus buttons and pack them
	gtk_box_pack_start(GTK_BOX(vbox), createListButtons(), FALSE, FALSE, 0);
	
	gtk_box_pack_start(GTK_BOX(srHBox),	vbox, FALSE, FALSE, 0);
	
	// The property pane
	gtk_box_pack_start(GTK_BOX(srHBox), createPropertyWidgets(), TRUE, TRUE, 0);
}

void StimEditor::setEntity(SREntityPtr entity) {
	// Pass the call to the base class
	ClassEditor::setEntity(entity);
	
	if (entity != NULL) {
		GtkListStore* listStore = _entity->getStimStore();
		gtk_tree_view_set_model(GTK_TREE_VIEW(_list), GTK_TREE_MODEL(listStore));
		g_object_unref(listStore); // treeview owns reference now
	}
}

GtkWidget* StimEditor::createPropertyWidgets() {
	_propertyWidgets.vbox = gtk_vbox_new(FALSE, 6);
	
	// Type Selector
	_type = createStimTypeSelector();
	gtk_box_pack_start(GTK_BOX(_propertyWidgets.vbox), _type.hbox, FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT(_type.list), "changed", G_CALLBACK(onStimTypeSelect), this);
	
	// Create the table for the widget alignment
	GtkTable* table = GTK_TABLE(gtk_table_new(12, 2, FALSE));
	gtk_table_set_row_spacings(table, 6);
	gtk_table_set_col_spacings(table, 6);
	gtk_box_pack_start(GTK_BOX(_propertyWidgets.vbox), GTK_WIDGET(table), FALSE, FALSE, 0);

	int curRow = 0;
	
	// Active
	_propertyWidgets.active = gtk_check_button_new_with_label(_("Active"));
	 gtk_table_attach_defaults(table, _propertyWidgets.active, 0, 2, curRow, curRow+1);

	 curRow++;
	
	// Timer Time
	_propertyWidgets.timer.toggle = gtk_check_button_new_with_label(_("Activation Timer:"));
	
	_propertyWidgets.timer.hour =  gtk_spin_button_new_with_range(0, 200, 1);
	_propertyWidgets.timer.minute = gtk_spin_button_new_with_range(0, 59, 1);
	_propertyWidgets.timer.second = gtk_spin_button_new_with_range(0, 59, 1);
	_propertyWidgets.timer.millisecond = gtk_spin_button_new_with_range(0, 999, 10);
	gtk_spin_button_set_digits(GTK_SPIN_BUTTON(_propertyWidgets.timer.hour), 0);
	gtk_spin_button_set_digits(GTK_SPIN_BUTTON(_propertyWidgets.timer.minute), 0);
	gtk_spin_button_set_digits(GTK_SPIN_BUTTON(_propertyWidgets.timer.second), 0);
	gtk_spin_button_set_digits(GTK_SPIN_BUTTON(_propertyWidgets.timer.millisecond), 0);
	
	_propertyWidgets.timer.entryHBox = gtk_hbox_new(FALSE, 3);
	GtkBox* entryHBox = GTK_BOX(_propertyWidgets.timer.entryHBox); // shortcut cast
	gtk_box_pack_start(entryHBox, _propertyWidgets.timer.hour, FALSE, FALSE, 0);
	gtk_box_pack_start(entryHBox, gtk_label_new("h"), FALSE, FALSE, 0);
	gtk_box_pack_start(entryHBox, _propertyWidgets.timer.minute, FALSE, FALSE, 0);
	gtk_box_pack_start(entryHBox, gtk_label_new("m"), FALSE, FALSE, 0);
	gtk_box_pack_start(entryHBox, _propertyWidgets.timer.second, FALSE, FALSE, 0);
	gtk_box_pack_start(entryHBox, gtk_label_new("s"), FALSE, FALSE, 0);
	gtk_box_pack_start(entryHBox, _propertyWidgets.timer.millisecond, FALSE, FALSE, 0);
	gtk_box_pack_start(entryHBox, gtkutil::LeftAlignedLabel("ms"), FALSE, FALSE, 0);
		
	gtk_table_attach(table, _propertyWidgets.timer.toggle, 0, 1, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
	gtk_table_attach_defaults(table, _propertyWidgets.timer.entryHBox, 1, 2, curRow, curRow+1);

	curRow++;
	
	// Timer type
	GtkWidget* timerTypeHBox = gtk_hbox_new(FALSE, 12); 
	_propertyWidgets.timer.typeToggle = gtk_check_button_new_with_label(_("Timer restarts after firing"));
			
	_propertyWidgets.timer.reloadHBox = gtk_hbox_new(FALSE, 3);
	_propertyWidgets.timer.reloadEntry = gtk_spin_button_new_with_range(0, 1000, 1);
	gtk_spin_button_set_digits(GTK_SPIN_BUTTON(_propertyWidgets.timer.reloadEntry), 0);
	gtk_widget_set_size_request(_propertyWidgets.timer.reloadEntry, 50, -1);
	_propertyWidgets.timer.reloadToggle = gtk_check_button_new_with_label(_("Timer reloads"));
	_propertyWidgets.timer.reloadLabel = gtkutil::LeftAlignedLabel(_("times"));
	GtkBox* reloadHBox = GTK_BOX(_propertyWidgets.timer.reloadHBox); // shortcut
	gtk_box_pack_start(reloadHBox, _propertyWidgets.timer.reloadEntry, FALSE, FALSE, 0);
	gtk_box_pack_start(reloadHBox, _propertyWidgets.timer.reloadLabel, TRUE, TRUE, 0);
	
	gtk_box_pack_start(GTK_BOX(timerTypeHBox), _propertyWidgets.timer.typeToggle, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(timerTypeHBox), _propertyWidgets.timer.reloadToggle, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(timerTypeHBox), GTK_WIDGET(reloadHBox), TRUE, TRUE, 0);
	
	gtk_table_attach_defaults(table, timerTypeHBox, 0, 2, curRow, curRow+1);

	curRow++;
	
	_propertyWidgets.timer.waitToggle = 
		gtk_check_button_new_with_label(_("Timer waits for start (when disabled: starts at spawn time)"));
	gtk_table_attach_defaults(table, _propertyWidgets.timer.waitToggle, 0, 2, curRow, curRow+1);

	curRow++;

	// Time Interval
	GtkWidget* timeHBox = gtk_hbox_new(FALSE, 6);
	_propertyWidgets.timeIntToggle = gtk_check_button_new_with_label(_("Time interval:"));
	_propertyWidgets.timeIntEntry = gtk_spin_button_new_with_range(0, 9999999, 10);
	gtk_spin_button_set_digits(GTK_SPIN_BUTTON(_propertyWidgets.timeIntEntry), 0);
	_propertyWidgets.timeUnitLabel = gtkutil::RightAlignedLabel(_("ms"));
		
	gtk_box_pack_start(GTK_BOX(timeHBox), _propertyWidgets.timeIntEntry, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(timeHBox), _propertyWidgets.timeUnitLabel, FALSE, FALSE, 0);
	
	gtk_table_attach(table, _propertyWidgets.timeIntToggle, 0, 1, curRow, curRow+1, GTK_FILL, GTK_FILL, 0, 0);
	gtk_table_attach(table, timeHBox, 1, 2, curRow, curRow+1, GTK_FILL, GTK_FILL, 0, 0);

	curRow++;

	// Duration
	GtkWidget* durationHBox = gtk_hbox_new(FALSE, 6);
	_propertyWidgets.durationToggle = gtk_check_button_new_with_label(_("Duration:"));
	_propertyWidgets.durationEntry = gtk_spin_button_new_with_range(0, 9999999, 10);
	gtk_spin_button_set_digits(GTK_SPIN_BUTTON(_propertyWidgets.durationEntry), 0);
	_propertyWidgets.durationUnitLabel = gtkutil::RightAlignedLabel(_("ms"));
	
	gtk_box_pack_start(GTK_BOX(durationHBox), _propertyWidgets.durationEntry, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(durationHBox), _propertyWidgets.durationUnitLabel, FALSE, FALSE, 0);
	
	gtk_table_attach(table, _propertyWidgets.durationToggle, 0, 1, curRow, curRow+1, GTK_FILL, GTK_FILL, 0, 0);
	gtk_table_attach(table, durationHBox, 1, 2, curRow, curRow+1, GTK_FILL, GTK_FILL, 0, 0);

	curRow++;
	
	// Radius / Use Bounds
	GtkWidget* radiusHBox = gtk_hbox_new(FALSE, 0);
	_propertyWidgets.radiusToggle = gtk_check_button_new_with_label(_("Radius:"));
	
	_propertyWidgets.radiusEntry = gtk_spin_button_new_with_range(0, 99999, 1);
	gtk_spin_button_set_digits(GTK_SPIN_BUTTON(_propertyWidgets.radiusEntry), 1);
	_propertyWidgets.useBounds = gtk_check_button_new_with_label(_("Use bounds"));
	gtk_box_pack_start(GTK_BOX(radiusHBox), _propertyWidgets.radiusEntry, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(radiusHBox), _propertyWidgets.useBounds, FALSE, FALSE, 6);
	
	gtk_table_attach(table, _propertyWidgets.radiusToggle, 0, 1, curRow, curRow+1, GTK_FILL, GTK_FILL, 0, 0);
	gtk_table_attach(table, radiusHBox, 1, 2, curRow, curRow+1, GTK_FILL, GTK_FILL, 0, 0);

	curRow++;

	// Final Radius
	GtkWidget* finalRadiusHBox = gtk_hbox_new(FALSE, 0);
	_propertyWidgets.finalRadiusToggle = gtk_check_button_new_with_label(_("Radius changes over time to:"));
	
	_propertyWidgets.finalRadiusEntry = gtk_spin_button_new_with_range(0, 99999, 1);
	gtk_spin_button_set_digits(GTK_SPIN_BUTTON(_propertyWidgets.finalRadiusEntry), 1);
	gtk_box_pack_start(GTK_BOX(finalRadiusHBox), _propertyWidgets.finalRadiusEntry, TRUE, TRUE, 0);
	
	gtk_table_attach(table, _propertyWidgets.finalRadiusToggle, 0, 1, curRow, curRow+1, GTK_FILL, GTK_FILL, 0, 0);
	gtk_table_attach(table, finalRadiusHBox, 1, 2, curRow, curRow+1, GTK_FILL, GTK_FILL, 0, 0);

	curRow++;
	
	// Magnitude
	_propertyWidgets.magnToggle = gtk_check_button_new_with_label(_("Magnitude:"));
	
	GtkWidget* magnHBox = gtk_hbox_new(FALSE, 6);
	_propertyWidgets.magnEntry = gtk_spin_button_new_with_range(0, 10000, 1);
	gtk_spin_button_set_digits(GTK_SPIN_BUTTON(_propertyWidgets.magnEntry), 2);
	gtk_entry_set_width_chars(GTK_ENTRY(_propertyWidgets.magnEntry), 7);
	
	// Falloff exponent
	_propertyWidgets.falloffToggle = gtk_check_button_new_with_label(_("Falloff Exponent:"));
	_propertyWidgets.falloffEntry = gtk_spin_button_new_with_range(-10, 10, 0.1f);
	gtk_spin_button_set_digits(GTK_SPIN_BUTTON(_propertyWidgets.falloffEntry), 2);
	gtk_entry_set_width_chars(GTK_ENTRY(_propertyWidgets.falloffEntry), 7);
	
	gtk_box_pack_start(GTK_BOX(magnHBox), _propertyWidgets.magnEntry, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(magnHBox), _propertyWidgets.falloffToggle, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(magnHBox), _propertyWidgets.falloffEntry, TRUE, TRUE, 0);
	
	gtk_table_attach(table, _propertyWidgets.magnToggle, 0, 1, curRow, curRow+1, GTK_FILL, GTK_FILL, 0, 0);
	gtk_table_attach(table, magnHBox, 1, 2, curRow, curRow+1, GTK_FILL, GTK_FILL, 0, 0);

	curRow++;

	// Max fire count
	_propertyWidgets.maxFireCountToggle = gtk_check_button_new_with_label(_("Max Fire Count:"));
	_propertyWidgets.maxFireCountEntry = gtk_spin_button_new_with_range(0, 1000000, 1);

	gtk_spin_button_set_digits(GTK_SPIN_BUTTON(_propertyWidgets.maxFireCountEntry), 0);
	gtk_entry_set_width_chars(GTK_ENTRY(_propertyWidgets.maxFireCountEntry), 7);
	
	gtk_table_attach(table, _propertyWidgets.maxFireCountToggle, 0, 1, curRow, curRow+1, GTK_FILL, GTK_FILL, 0, 0);
	gtk_table_attach(table, _propertyWidgets.maxFireCountEntry, 1, 2, curRow, curRow+1, GTK_FILL, GTK_FILL, 0, 0);

	curRow++;
	
	// Chance variable
	_propertyWidgets.chanceToggle = gtk_check_button_new_with_label(_("Chance:"));
	_propertyWidgets.chanceEntry = gtk_spin_button_new_with_range(0.0f, 1.0f, 0.01f);
	gtk_spin_button_set_digits(GTK_SPIN_BUTTON(_propertyWidgets.chanceEntry), 2);
	
	gtk_table_attach(table, _propertyWidgets.chanceToggle, 0, 1, curRow, curRow+1, GTK_FILL, GTK_FILL, 0, 0);
	gtk_table_attach(table, _propertyWidgets.chanceEntry, 1, 2, curRow, curRow+1, GTK_FILL, GTK_FILL, 0, 0);

	curRow++;
	
	// Velocity variable
	_propertyWidgets.velocityToggle = gtk_check_button_new_with_label(_("Velocity:"));
	_propertyWidgets.velocityEntry = gtk_entry_new();
	
	gtk_table_attach(table, _propertyWidgets.velocityToggle, 0, 1, curRow, curRow+1, GTK_FILL, GTK_FILL, 0, 0);
	gtk_table_attach(table, _propertyWidgets.velocityEntry, 1, 2, curRow, curRow+1, GTK_FILL, GTK_FILL, 0, 0);

	curRow++;
	
	// Bounds mins and maxs
	_propertyWidgets.bounds.hbox = gtk_hbox_new(FALSE, 6);
	_propertyWidgets.bounds.toggle = gtk_check_button_new_with_label(_("Bounds "));
	_propertyWidgets.bounds.minLabel = gtk_label_new(_("Min:"));
	_propertyWidgets.bounds.maxLabel = gtk_label_new(_("Max:"));
	_propertyWidgets.bounds.minEntry = gtk_entry_new();
	_propertyWidgets.bounds.maxEntry = gtk_entry_new();
	gtk_widget_set_size_request(_propertyWidgets.bounds.minEntry, 100, -1);
	gtk_widget_set_size_request(_propertyWidgets.bounds.maxEntry, 100, -1);
	GtkBox* boundsHBox = GTK_BOX(_propertyWidgets.bounds.hbox); // shortcut cast
	gtk_box_pack_start(boundsHBox, _propertyWidgets.bounds.minLabel, FALSE, FALSE, 0);
	gtk_box_pack_start(boundsHBox, _propertyWidgets.bounds.minEntry, TRUE, TRUE, 0);
	gtk_box_pack_start(boundsHBox, _propertyWidgets.bounds.maxLabel, FALSE, FALSE, 0);
	gtk_box_pack_start(boundsHBox, _propertyWidgets.bounds.maxEntry, TRUE, TRUE, 0);
	
	gtk_table_attach(table, _propertyWidgets.bounds.toggle, 0, 1, curRow, curRow+1, GTK_FILL, GTK_FILL, 0, 0);
	gtk_table_attach(table, _propertyWidgets.bounds.hbox, 1, 2, curRow, curRow+1, GTK_FILL, GTK_FILL, 0, 0);

	curRow++;
	
	// The map associating entry fields to stim property keys  
	_spinWidgets[GTK_SPIN_BUTTON(_propertyWidgets.radiusEntry)] = "radius";
	_spinWidgets[GTK_SPIN_BUTTON(_propertyWidgets.finalRadiusEntry)] = "radius_final";
	_spinWidgets[GTK_SPIN_BUTTON(_propertyWidgets.timeIntEntry)] = "time_interval";
	_spinWidgets[GTK_SPIN_BUTTON(_propertyWidgets.magnEntry)] = "magnitude";
	_spinWidgets[GTK_SPIN_BUTTON(_propertyWidgets.falloffEntry)] = "falloffexponent";
	_spinWidgets[GTK_SPIN_BUTTON(_propertyWidgets.chanceEntry)] = "chance";
	_spinWidgets[GTK_SPIN_BUTTON(_propertyWidgets.maxFireCountEntry)] = "max_fire_count";
	_spinWidgets[GTK_SPIN_BUTTON(_propertyWidgets.durationEntry)] = "duration";
	_spinWidgets[GTK_SPIN_BUTTON(_propertyWidgets.timer.reloadEntry)] = "timer_reload";
	
	_entryWidgets[GTK_EDITABLE(_propertyWidgets.velocityEntry)] = "velocity";
	_entryWidgets[GTK_EDITABLE(_propertyWidgets.bounds.minEntry)] = "bounds_mins";
	_entryWidgets[GTK_EDITABLE(_propertyWidgets.bounds.maxEntry)] = "bounds_maxs";
	
	// Connect the checkboxes
	g_signal_connect(G_OBJECT(_propertyWidgets.active), "toggled", G_CALLBACK(onCheckboxToggle), this);
	g_signal_connect(G_OBJECT(_propertyWidgets.useBounds), "toggled", G_CALLBACK(onCheckboxToggle), this);
	g_signal_connect(G_OBJECT(_propertyWidgets.radiusToggle), "toggled", G_CALLBACK(onCheckboxToggle), this);
	g_signal_connect(G_OBJECT(_propertyWidgets.finalRadiusToggle), "toggled", G_CALLBACK(onCheckboxToggle), this);
	g_signal_connect(G_OBJECT(_propertyWidgets.timeIntToggle), "toggled", G_CALLBACK(onCheckboxToggle), this);
	g_signal_connect(G_OBJECT(_propertyWidgets.magnToggle), "toggled", G_CALLBACK(onCheckboxToggle), this);
	g_signal_connect(G_OBJECT(_propertyWidgets.falloffToggle), "toggled", G_CALLBACK(onCheckboxToggle), this);
	g_signal_connect(G_OBJECT(_propertyWidgets.timer.typeToggle), "toggled", G_CALLBACK(onCheckboxToggle), this);
	g_signal_connect(G_OBJECT(_propertyWidgets.chanceToggle), "toggled", G_CALLBACK(onCheckboxToggle), this);
	g_signal_connect(G_OBJECT(_propertyWidgets.maxFireCountToggle), "toggled", G_CALLBACK(onCheckboxToggle), this);
	g_signal_connect(G_OBJECT(_propertyWidgets.durationToggle), "toggled", G_CALLBACK(onCheckboxToggle), this);
	g_signal_connect(G_OBJECT(_propertyWidgets.timer.toggle), "toggled", G_CALLBACK(onCheckboxToggle), this);
	g_signal_connect(G_OBJECT(_propertyWidgets.timer.reloadToggle), "toggled", G_CALLBACK(onCheckboxToggle), this);
	g_signal_connect(G_OBJECT(_propertyWidgets.timer.waitToggle), "toggled", G_CALLBACK(onCheckboxToggle), this);
	g_signal_connect(G_OBJECT(_propertyWidgets.velocityToggle), "toggled", G_CALLBACK(onCheckboxToggle), this);
	g_signal_connect(G_OBJECT(_propertyWidgets.bounds.toggle), "toggled", G_CALLBACK(onCheckboxToggle), this);
	
	// Connect the entry fields
	g_signal_connect(G_OBJECT(_propertyWidgets.velocityEntry), "changed", G_CALLBACK(onEntryChanged), this);
	g_signal_connect(G_OBJECT(_propertyWidgets.bounds.minEntry), "changed", G_CALLBACK(onEntryChanged), this);
	g_signal_connect(G_OBJECT(_propertyWidgets.bounds.maxEntry), "changed", G_CALLBACK(onEntryChanged), this);
	
	// Connect the spin button fields
	g_signal_connect(G_OBJECT(_propertyWidgets.magnEntry), "value-changed", G_CALLBACK(onSpinButtonChanged), this);
	g_signal_connect(G_OBJECT(_propertyWidgets.falloffEntry), "value-changed", G_CALLBACK(onSpinButtonChanged), this);
	g_signal_connect(G_OBJECT(_propertyWidgets.radiusEntry), "value-changed", G_CALLBACK(onSpinButtonChanged), this);
	g_signal_connect(G_OBJECT(_propertyWidgets.finalRadiusEntry), "value-changed", G_CALLBACK(onSpinButtonChanged), this);
	g_signal_connect(G_OBJECT(_propertyWidgets.timeIntEntry), "value-changed", G_CALLBACK(onSpinButtonChanged), this);
	g_signal_connect(G_OBJECT(_propertyWidgets.chanceEntry), "value-changed", G_CALLBACK(onSpinButtonChanged), this);
	g_signal_connect(G_OBJECT(_propertyWidgets.maxFireCountEntry), "value-changed", G_CALLBACK(onSpinButtonChanged), this);
	g_signal_connect(G_OBJECT(_propertyWidgets.durationEntry), "value-changed", G_CALLBACK(onSpinButtonChanged), this);
	g_signal_connect(G_OBJECT(_propertyWidgets.timer.hour), "value-changed", G_CALLBACK(onSpinButtonChanged), this);
	g_signal_connect(G_OBJECT(_propertyWidgets.timer.minute), "value-changed", G_CALLBACK(onSpinButtonChanged), this);
	g_signal_connect(G_OBJECT(_propertyWidgets.timer.second), "value-changed", G_CALLBACK(onSpinButtonChanged), this);
	g_signal_connect(G_OBJECT(_propertyWidgets.timer.millisecond), "value-changed", G_CALLBACK(onSpinButtonChanged), this);
	g_signal_connect(G_OBJECT(_propertyWidgets.timer.reloadEntry), "value-changed", G_CALLBACK(onSpinButtonChanged), this);
	
	return _propertyWidgets.vbox;
}

std::string StimEditor::getTimerString() {
	std::string hour = intToStr(gtk_spin_button_get_value_as_int(
		GTK_SPIN_BUTTON(_propertyWidgets.timer.hour)));
	std::string minute = intToStr(gtk_spin_button_get_value_as_int(
		GTK_SPIN_BUTTON(_propertyWidgets.timer.minute)));
	std::string second = intToStr(gtk_spin_button_get_value_as_int(
		GTK_SPIN_BUTTON(_propertyWidgets.timer.second)));
	std::string ms = intToStr(gtk_spin_button_get_value_as_int(
		GTK_SPIN_BUTTON(_propertyWidgets.timer.millisecond)));
		
	return hour + ":" + minute + ":" + second + ":" + ms;
}

void StimEditor::spinButtonChanged(GtkSpinButton* spinButton) {
	// Pass the call to the base class
	ClassEditor::spinButtonChanged(spinButton);
	
	if (spinButton == GTK_SPIN_BUTTON(_propertyWidgets.timer.hour) || 
		spinButton == GTK_SPIN_BUTTON(_propertyWidgets.timer.minute) || 
		spinButton == GTK_SPIN_BUTTON(_propertyWidgets.timer.second) ||
		spinButton == GTK_SPIN_BUTTON(_propertyWidgets.timer.millisecond))
	{
		setProperty("timer_time", getTimerString());
	}
}

void StimEditor::checkBoxToggled(GtkToggleButton* toggleButton) {
	GtkWidget* toggleWidget = GTK_WIDGET(toggleButton);
	bool active = gtk_toggle_button_get_active(toggleButton) ? true : false;
	
	if (toggleWidget == _propertyWidgets.active) {
		setProperty("state", active ? "1" : "0");
	}
	else if (toggleWidget == _propertyWidgets.useBounds) {
		setProperty("use_bounds", active ? "1" : "");
	}
	else if (toggleWidget == _propertyWidgets.timer.typeToggle) {
		setProperty("timer_type", active ? "RELOAD" : "");
	}
	else if (toggleWidget == _propertyWidgets.radiusToggle) {
		setProperty("radius", active ? "10" : "");

		// Clear final radius if disabled
		if (!active) {
			setProperty("radius_final", "");	
		}
	}
	if (toggleWidget == _propertyWidgets.finalRadiusToggle) {
		setProperty("radius_final", active ? "10" : "");
	}
	else if (toggleWidget == _propertyWidgets.magnToggle) {
		setProperty("magnitude", active ? "10" : "");
	}
	else if (toggleWidget == _propertyWidgets.maxFireCountToggle) {
		setProperty("max_fire_count", active ? "10" : "");
	}
	else if (toggleWidget == _propertyWidgets.falloffToggle) {
		setProperty("falloffexponent", active ? "1" : "");
	}
	else if (toggleWidget == _propertyWidgets.timeIntToggle) {
		setProperty("time_interval", active ? "1000" : "");
	}
	else if (toggleWidget == _propertyWidgets.chanceToggle) {
		std::string entryText = doubleToStr(gtk_spin_button_get_value_as_float(
			GTK_SPIN_BUTTON(_propertyWidgets.chanceEntry)
		));

		setProperty("chance", active ? entryText : "");
	}
	else if (toggleWidget == _propertyWidgets.velocityToggle) {
		std::string entryText = 
			gtk_entry_get_text(GTK_ENTRY(_propertyWidgets.velocityEntry));
	
		// Enter a default value for the entry text, if it's empty up till now.
		if (active) {
			entryText += (entryText.empty()) ? "0 0 100" : "";	
		}
		else {
			entryText = "";
		}
		setProperty("velocity", entryText);
	}
	else if (toggleWidget == _propertyWidgets.bounds.toggle) {
		std::string entryText = 
			gtk_entry_get_text(GTK_ENTRY(_propertyWidgets.bounds.minEntry));
	
		// Enter a default value for the entry text, if it's empty up till now.
		if (active) {
			entryText += (entryText.empty()) ? "-10 -10 -10" : "";	
		}
		else {
			entryText = "";
		}
		setProperty("bounds_mins", entryText);
		
		entryText = gtk_entry_get_text(GTK_ENTRY(_propertyWidgets.bounds.maxEntry));
		// Enter a default value for the entry text, if it's empty up till now.
		if (active) {
			entryText += (entryText.empty()) ? "10 10 10" : "";	
		}
		else {
			entryText = "";
		}
		setProperty("bounds_maxs", entryText);
	}
	else if (toggleWidget == _propertyWidgets.durationToggle) {
		setProperty("duration", active ? "1000" : "");

		// Clear final radius if disabled
		if (!active) {
			setProperty("radius_final", "");	
		}
	}
	else if (toggleWidget == _propertyWidgets.timer.toggle) {
		std::string timerStr = getTimerString();
		setProperty("timer_time", active ? timerStr : "");
	}
	else if (toggleWidget == _propertyWidgets.timer.reloadToggle) {
		setProperty("timer_reload", active ? "1" : "");
	}
	else if (toggleWidget == _propertyWidgets.timer.waitToggle) {
		setProperty("timer_waitforstart", active ? "1" : "");
	}
}

void StimEditor::openContextMenu(GtkTreeView* view) {
	gtk_menu_popup(GTK_MENU(_contextMenu.menu), NULL, NULL, NULL, NULL, 1, GDK_CURRENT_TIME);
}

void StimEditor::addSR() {
	if (_entity == NULL) return;

	// Create a new StimResponse object
	int id = _entity->add();
	
	// Get a reference to the newly allocated object
	StimResponse& sr = _entity->get(id);
	sr.set("class", "S");
	
	// Get the selected stim type name from the combo box
	std::string name = getStimTypeIdFromSelector(GTK_COMBO_BOX(_addType.list));
	sr.set("type", (!name.empty()) ? name : _stimTypes.getFirstName());
	
	sr.set("state", "1");

	// Update the list stores AFTER the type has been set
	_entity->updateListStores();

	// Select the newly created stim
	selectId(id);
}

// Create the context menus
void StimEditor::createContextMenu() {
	// Menu widgets
	_contextMenu.menu = gtk_menu_new();
		
	// Each menu gets a delete item
	_contextMenu.remove = gtkutil::StockIconMenuItem(GTK_STOCK_DELETE,
														   _("Delete"));
	//_contextMenu.add = gtkutil::StockIconMenuItem(GTK_STOCK_ADD, "Add Stim");
	_contextMenu.disable = gtkutil::StockIconMenuItem(GTK_STOCK_NO,
														   _("Deactivate"));
	_contextMenu.enable = gtkutil::StockIconMenuItem(GTK_STOCK_YES,
														   _("Activate"));
	_contextMenu.duplicate = gtkutil::StockIconMenuItem(GTK_STOCK_COPY,
														   _("Duplicate"));

	//gtk_menu_shell_append(GTK_MENU_SHELL(_contextMenu.menu), _contextMenu.add);
	gtk_menu_shell_append(GTK_MENU_SHELL(_contextMenu.menu), 
						  _contextMenu.enable);
	gtk_menu_shell_append(GTK_MENU_SHELL(_contextMenu.menu), 
						  _contextMenu.disable);
	gtk_menu_shell_append(GTK_MENU_SHELL(_contextMenu.menu), 
						  _contextMenu.duplicate);
	gtk_menu_shell_append(GTK_MENU_SHELL(_contextMenu.menu), 
						  _contextMenu.remove);

	// Connect up the signals
	g_signal_connect(G_OBJECT(_contextMenu.remove), "activate",
					 G_CALLBACK(onContextMenuDelete), this);
	/*g_signal_connect(G_OBJECT(_contextMenu.add), "activate",
					 G_CALLBACK(onContextMenuAdd), this);*/
	g_signal_connect(G_OBJECT(_contextMenu.enable), "activate",
					 G_CALLBACK(onContextMenuEnable), this);
	g_signal_connect(G_OBJECT(_contextMenu.disable), "activate",
					 G_CALLBACK(onContextMenuDisable), this);
	g_signal_connect(G_OBJECT(_contextMenu.duplicate), "activate",
					 G_CALLBACK(onContextMenuDuplicate), this);
	
	// Show menus (not actually visible until popped up)
	gtk_widget_show_all(_contextMenu.menu);
}

void StimEditor::update() {
	_updatesDisabled = true; // avoid unwanted callbacks
	
	int id = getIdFromSelection();
	
	if (id > 0) {
		// Update all the widgets
		gtk_widget_set_sensitive(_propertyWidgets.vbox, TRUE);
		
		StimResponse& sr = _entity->get(id);
		
		// Get the iter into the liststore pointing at the correct STIM_YYYY type
		GtkTreeIter typeIter = _stimTypes.getIterForName(sr.get("type"));
		gtk_combo_box_set_active_iter(GTK_COMBO_BOX(_type.list), &typeIter);
		
		// Active
		gtk_toggle_button_set_active(
			GTK_TOGGLE_BUTTON(_propertyWidgets.active),
			(sr.get("state") == "1")
		);
		
		// Use Radius
		bool useRadius = (sr.get("radius") != "");
		gtk_toggle_button_set_active(
			GTK_TOGGLE_BUTTON(_propertyWidgets.radiusToggle),
			useRadius
		);
		gtk_spin_button_set_value(
			GTK_SPIN_BUTTON(_propertyWidgets.radiusEntry), 
			strToFloat(sr.get("radius"))
		);
		gtk_widget_set_sensitive(
			_propertyWidgets.radiusEntry, 
			useRadius
		);

		// Use Bounds
		gtk_toggle_button_set_active(
			GTK_TOGGLE_BUTTON(_propertyWidgets.useBounds),
			sr.get("use_bounds") == "1" && useRadius
		);
		gtk_widget_set_sensitive(_propertyWidgets.useBounds, useRadius);

		// Use Duration
		bool useDuration = (sr.get("duration") != "");
		gtk_toggle_button_set_active(
			GTK_TOGGLE_BUTTON(_propertyWidgets.durationToggle),
			useDuration
		);
		gtk_spin_button_set_value(
			GTK_SPIN_BUTTON(_propertyWidgets.durationEntry), 
			strToInt(sr.get("duration"))
		);
		gtk_widget_set_sensitive(
			_propertyWidgets.durationEntry, 
			useDuration
		);
		gtk_widget_set_sensitive(
			_propertyWidgets.durationUnitLabel, 
			useDuration
		);
			
		// Use Time interval
		bool useTimeInterval = (sr.get("time_interval") != "");
		gtk_toggle_button_set_active(
			GTK_TOGGLE_BUTTON(_propertyWidgets.timeIntToggle),
			useTimeInterval
		);
		gtk_spin_button_set_value(
			GTK_SPIN_BUTTON(_propertyWidgets.timeIntEntry), 
			strToInt(sr.get("time_interval"))
		);
		gtk_widget_set_sensitive(
			_propertyWidgets.timeIntEntry, 
			useTimeInterval
		);
		gtk_widget_set_sensitive(
			_propertyWidgets.timeUnitLabel, 
			useTimeInterval
		);

		// Use Final radius (duration must be enabled for this to work)
		bool useFinalRadius = (sr.get("radius_final") != "");
		gtk_toggle_button_set_active(
			GTK_TOGGLE_BUTTON(_propertyWidgets.finalRadiusToggle),
			useFinalRadius && useDuration
		);
		gtk_spin_button_set_value(
			GTK_SPIN_BUTTON(_propertyWidgets.finalRadiusEntry), 
			strToFloat(sr.get("radius_final"))
		);
		gtk_widget_set_sensitive(
			_propertyWidgets.finalRadiusToggle, 
			useRadius && useDuration
		);
		gtk_widget_set_sensitive(
			_propertyWidgets.finalRadiusEntry, 
			useFinalRadius && useDuration && useRadius
		);
		
		// Timer time
		bool useTimerTime = !sr.get("timer_time").empty();
		gtk_toggle_button_set_active(
			GTK_TOGGLE_BUTTON(_propertyWidgets.timer.toggle),
			useTimerTime
		);
		gtk_widget_set_sensitive(_propertyWidgets.timer.toggle,	TRUE);
		gtk_widget_set_sensitive(_propertyWidgets.timer.entryHBox, useTimerTime);
		
		// Split the property string and distribute the parts into the entry fields
		StringParts parts;
		std::string timerTime = sr.get("timer_time");
		boost::algorithm::split(parts, timerTime, boost::algorithm::is_any_of(":"));
		std::string hour = (parts.size() > 0) ? parts[0] : "";
		std::string minute = (parts.size() > 1) ? parts[1] : "";
		std::string second = (parts.size() > 2) ? parts[2] : "";
		std::string ms = (parts.size() > 3) ? parts[3] : "";
		
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(_propertyWidgets.timer.hour), strToInt(hour));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(_propertyWidgets.timer.minute), strToInt(minute));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(_propertyWidgets.timer.second), strToInt(second));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(_propertyWidgets.timer.millisecond), strToInt(ms));
		
		gtk_toggle_button_set_active(
			GTK_TOGGLE_BUTTON(_propertyWidgets.timer.waitToggle),
			useTimerTime && sr.get("timer_waitforstart") == "1"
		);
		gtk_widget_set_sensitive(_propertyWidgets.timer.waitToggle, useTimerTime);
		
		// Timer Type
		bool useTimerType = sr.get("timer_type") == "RELOAD" && useTimerTime;
		gtk_toggle_button_set_active(
			GTK_TOGGLE_BUTTON(_propertyWidgets.timer.typeToggle),
			useTimerType
		);
		gtk_widget_set_sensitive(
			_propertyWidgets.timer.typeToggle, 
			useTimerTime
		);
		
		bool userTimerReload = useTimerType && !sr.get("timer_reload").empty(); 
		gtk_toggle_button_set_active(
			GTK_TOGGLE_BUTTON(_propertyWidgets.timer.reloadToggle),
			userTimerReload
		);
		gtk_widget_set_sensitive(
			_propertyWidgets.timer.reloadToggle, 
			useTimerType
		);
		gtk_spin_button_set_value(
			GTK_SPIN_BUTTON(_propertyWidgets.timer.reloadEntry),
			strToInt(sr.get("timer_reload"))
		);
		gtk_widget_set_sensitive(_propertyWidgets.timer.reloadHBox, userTimerReload);
		
		// Use Magnitude
		bool useMagnitude = (sr.get("magnitude") != "");
		gtk_toggle_button_set_active(
			GTK_TOGGLE_BUTTON(_propertyWidgets.magnToggle),
			useMagnitude
		);
		gtk_spin_button_set_value(
			GTK_SPIN_BUTTON(_propertyWidgets.magnEntry),
			strToFloat(sr.get("magnitude"))
		);
		gtk_widget_set_sensitive(
			_propertyWidgets.magnEntry, 
			useMagnitude
		);
		
		// Use falloff exponent widgets
		bool useFalloff = (sr.get("falloffexponent") != "");
		
		gtk_toggle_button_set_active(
			GTK_TOGGLE_BUTTON(_propertyWidgets.falloffToggle),
			useFalloff
		);
		gtk_spin_button_set_value(
			GTK_SPIN_BUTTON(_propertyWidgets.falloffEntry),
			strToFloat(sr.get("falloffexponent"))
		);
		gtk_widget_set_sensitive(
			_propertyWidgets.falloffToggle, 
			useMagnitude
		);
		gtk_widget_set_sensitive(
			_propertyWidgets.falloffEntry, 
			useMagnitude && useFalloff
		);
		
		// Use Chance
		bool useChance = (sr.get("chance") != "");
		gtk_toggle_button_set_active(
			GTK_TOGGLE_BUTTON(_propertyWidgets.chanceToggle),
			useChance
		);
		gtk_spin_button_set_value(
			GTK_SPIN_BUTTON(_propertyWidgets.chanceEntry),
			strToFloat(sr.get("chance"))
		);
		gtk_widget_set_sensitive(
			_propertyWidgets.chanceEntry, 
			useChance
		);

		// Use Max Fire Count
		bool useMaxFireCount = (sr.get("max_fire_count") != "");
		gtk_toggle_button_set_active(
			GTK_TOGGLE_BUTTON(_propertyWidgets.maxFireCountToggle),
			useMaxFireCount
		);
		gtk_spin_button_set_value(
			GTK_SPIN_BUTTON(_propertyWidgets.maxFireCountEntry),
			strToFloat(sr.get("max_fire_count"))
		);
		gtk_widget_set_sensitive(
			_propertyWidgets.maxFireCountEntry, 
			useMaxFireCount
		);
		
		// Use Velocity
		bool useVelocity = (sr.get("velocity") != "");
		gtk_toggle_button_set_active(
			GTK_TOGGLE_BUTTON(_propertyWidgets.velocityToggle),
			useVelocity
		);
		gtk_entry_set_text(
			GTK_ENTRY(_propertyWidgets.velocityEntry),
			sr.get("velocity").c_str()
		);
		gtk_widget_set_sensitive(
			_propertyWidgets.velocityEntry, 
			useVelocity
		);
		
		// Use Velocity
		bool useBoundsMinMax = (sr.get("bounds_mins") != "");
		gtk_toggle_button_set_active(
			GTK_TOGGLE_BUTTON(_propertyWidgets.bounds.toggle),
			useBoundsMinMax
		);
		gtk_entry_set_text(
			GTK_ENTRY(_propertyWidgets.bounds.minEntry),
			sr.get("bounds_mins").c_str()
		);
		gtk_entry_set_text(
			GTK_ENTRY(_propertyWidgets.bounds.maxEntry),
			sr.get("bounds_maxs").c_str()
		);
		gtk_widget_set_sensitive(
			_propertyWidgets.bounds.hbox, 
			useBoundsMinMax
		);
		
		// Disable the editing of inherited properties completely
		if (sr.inherited()) {
			gtk_widget_set_sensitive(_propertyWidgets.vbox, FALSE);
		}
		
		// If there is anything selected, the duplicate item is always active
		gtk_widget_set_sensitive(_contextMenu.duplicate, TRUE);
		
		// Update the delete context menu item
		gtk_widget_set_sensitive(_contextMenu.remove, !sr.inherited());
				
		// Update the "enable/disable" menu items
		bool state = sr.get("state") == "1";
		gtk_widget_set_sensitive(_contextMenu.enable, !state);
		gtk_widget_set_sensitive(_contextMenu.disable, state);
	}
	else {
		gtk_widget_set_sensitive(_propertyWidgets.vbox, FALSE);
		// Disable the "non-Add" context menu items
		gtk_widget_set_sensitive(_contextMenu.remove, FALSE);
		gtk_widget_set_sensitive(_contextMenu.enable, FALSE);
		gtk_widget_set_sensitive(_contextMenu.disable, FALSE);
		gtk_widget_set_sensitive(_contextMenu.duplicate, FALSE);
	}
	
	_updatesDisabled = false;
}

void StimEditor::selectionChanged() {
	update();
}

// Delete context menu items activated
void StimEditor::onContextMenuDelete(GtkWidget* w, StimEditor* self) {
	// Delete the selected stim from the list
	self->removeSR(GTK_TREE_VIEW(self->_list));
}

// Delete context menu items activated
void StimEditor::onContextMenuAdd(GtkWidget* w, StimEditor* self) {
	self->addSR();
}

} // namespace ui
