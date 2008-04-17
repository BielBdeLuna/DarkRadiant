#ifndef SPECIFIEREDITCOMBO_H_
#define SPECIFIEREDITCOMBO_H_

#include "../Specifier.h"
#include "specpanel/SpecifierPanel.h"

namespace objectives
{

namespace ce
{

/**
 * Compound widget for changing specifier types and editing their values.
 * 
 * A SpecifierEditCombo is a horizontal box containing two main elements. On the
 * left is a GtkComboBox which contains the names of a number of different
 * Specifier types, and on the right is a SpecifierPanel which is switched 
 * depending on the selected dropdown value. The SpecifierEditCombo therefore
 * provides a means for the user to choose both a Specifier and its associated
 * value.
 * 
 * Since some Component types accept a different subset of the Specifier types,
 * the SpecifierEditCombo can accept a std::set of Specifier types to display
 * in its dropdown list. Alternatively the entire set of Specifiers can be
 * made available.
 */
class SpecifierEditCombo
{
	// Main widget
	GtkWidget* _widget;
	
	// Current SpecifierPanel
	SpecifierPanelPtr _specPanel;
	
    // Combo box containing Specifiers
    GtkWidget* _specifierCombo;

private:
	
    // Get the selected Specifier string
    std::string getSpecName() const;

	/* GTK CALLBACKS */
	static void _onChange(GtkWidget* w, SpecifierEditCombo* self);
	
public:
	
	/**
	 * Construct a SpecifierEditCombo with a subset of Specifier types
	 * available.
	 * 
	 * @param set
	 * A SpecifierSet containing the subset of Specifiers which should be
	 * displayed in this edit combo. The default is the complete set of
	 * specifiers.
	 */
	SpecifierEditCombo(const SpecifierSet& set = Specifier::SET_ALL());

    /**
     * Return the main GtkWidget for this edit panel.
     *
     * @return
     * A GtkWidget containing all widgets involved in this edit panel.
     */
    GtkWidget* getWidget() const;

    /**
     * Return the selected Specifier.
     *
     * @return
     * A const reference to the Specifier object corresponding to the ComboBox
     * selection.
     */
    const Specifier& getSpecifier() const;

    /**
     * Set the Specifier to display in the ComboBox.
     *
     * @param
     * The Specifier object to select.
     */
    void setSpecifier(const Specifier& spec);

    /**
     * Return the string value associated with the selected Specifier.
     *
     * @return
     * The string value contained within the currently-active SpecifierPanel.
     */
    std::string getValue() const;
};

}

}

#endif /*SPECIFIEREDITCOMBO_H_*/
