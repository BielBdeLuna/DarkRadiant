#ifndef RESPONSEEDITOR_H_
#define RESPONSEEDITOR_H_

#include <gtkmm/window.h>
#include "ClassEditor.h"

namespace ui {

class ResponseEditor :
	public ClassEditor
{
	struct ListContextMenu {
		GtkWidget* menu;
		GtkWidget* remove;
		GtkWidget* add;
		GtkWidget* enable;
		GtkWidget* disable;
		GtkWidget* duplicate;
	} _contextMenu;
	
	struct EffectWidgets {
		GtkWidget* view;
		GtkTreeSelection* selection;
		GtkWidget* contextMenu;
		GtkWidget* deleteMenuItem;
		GtkWidget* addMenuItem;
		GtkWidget* editMenuItem;
		GtkWidget* upMenuItem;
		GtkWidget* downMenuItem;
	} _effectWidgets;
	
	struct PropertyWidgets {
		GtkWidget* vbox;
		GtkWidget* active;
		GtkWidget* chanceToggle;
		GtkWidget* chanceEntry;
		GtkWidget* randomEffectsToggle;
		GtkWidget* randomEffectsEntry;
	} _propertyWidgets;
	
	Glib::RefPtr<Gtk::Window> _parent;
	
public:
	/** greebo: Constructor creates all the widgets
	 */
	ResponseEditor(const Glib::RefPtr<Gtk::Window>& parent, StimTypes& stimTypes);

	/** greebo: Sets the new entity (updates the treeviews)
	 */
	virtual void setEntity(SREntityPtr entity);

	/** greebo: Updates the widgets (e.g. after a selection change) 
	 */
	void update();

private:
	/** greebo: Updates the associated text fields when a check box
	 * 			is toggled.
	 */
	void checkBoxToggled(GtkToggleButton* toggleButton);

	/** greebo: Adds a new response effect to the list.
	 */
	void addEffect();

	/** greebo: Removes the currently selected response effect
	 */
	void removeEffect();
	
	/** greebo: Edits the currently selected effect 
	 */
	void editEffect();
	
	/** greebo: Moves the selected effect up or down (i.e. increasing
	 * 			or decreasing its index).
	 * 
	 * @direction: +1 for moving it down (increasing the index)
	 * 			   -1 for moving it up (decreasing the index)
	 */
	void moveEffect(int direction);

	/** greebo: Updates the sensitivity of the effects context menu
	 */
	void updateEffectContextMenu();

	/** greebo: Selects the effect with the given index in the treeview.
	 */
	void selectEffectIndex(const unsigned int index);

	/** greebo: Returns the ID of the currently selected response effect
	 * 		
	 * @returns: the index of the selected effect or -1 on failure 
	 */
	int getEffectIdFromSelection();

	/** greebo: Adds a new default response to the entity
	 */
	void addSR();

	// Widget creator helpers
	void createContextMenu();
	GtkWidget* createEffectWidgets(); // Response effect list 

	/** greebo: Gets called when the response selection gets changed 
	 */
	virtual void selectionChanged();
	
	void openContextMenu(GtkTreeView* view);

	/** greebo: Creates all the widgets
	 */
	void populatePage();
	
	// Context menu GTK callbacks
	static void onContextMenuAdd(GtkWidget* w, ResponseEditor* self);
	static void onContextMenuDelete(GtkWidget* w, ResponseEditor* self);
	static void onContextMenuEffectUp(GtkWidget* widget, ResponseEditor* self);
	static void onContextMenuEffectDown(GtkWidget* widget, ResponseEditor* self);
	static void onContextMenuEdit(GtkWidget* widget, ResponseEditor* self);
	
	// To catch double-clicks in the response effect list 
	static gboolean onTreeViewButtonPress(GtkTreeView*, GdkEventButton*, ResponseEditor* self);
	
	// Callback for Stim/Response and effect selection changes
	static void onEffectSelectionChange(GtkTreeSelection* selection, ResponseEditor* self); 
};

} // namespace ui

#endif /*RESPONSEEDITOR_H_*/
