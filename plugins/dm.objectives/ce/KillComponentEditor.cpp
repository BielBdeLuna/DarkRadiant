#include "KillComponentEditor.h"
#include "../SpecifierType.h"

#include "gtkutil/LeftAlignment.h"
#include "gtkutil/LeftAlignedLabel.h"

#include <gtk/gtk.h>

namespace objectives
{

namespace ce
{

// Registration helper
KillComponentEditor::RegHelper KillComponentEditor::regHelper;

// Constructor
KillComponentEditor::KillComponentEditor(Component& component)
: _component(&component),
  _targetCombo(SpecifierType::SET_STANDARD_AI())
{
	// Main vbox
	_widget = gtk_vbox_new(FALSE, 6);
    gtk_container_set_border_width(GTK_CONTAINER(_widget), 6);

    gtk_box_pack_start(
        GTK_BOX(_widget), 
        gtkutil::LeftAlignedLabel("<b>Kill target:</b>"),
        FALSE, FALSE, 0
    );
	gtk_box_pack_start(
		GTK_BOX(_widget), _targetCombo.getWidget(), FALSE, FALSE, 0
	);
}

// Destructor
KillComponentEditor::~KillComponentEditor() {
	if (GTK_IS_WIDGET(_widget))
		gtk_widget_destroy(_widget);
}

// Get the main widget
GtkWidget* KillComponentEditor::getWidget() const
{
	return _widget;
}

}

}
