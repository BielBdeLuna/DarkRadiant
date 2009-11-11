#ifndef EFFECTARGUMENTITEM_H_
#define EFFECTARGUMENTITEM_H_

#include "StimTypes.h"
#include "ResponseEffect.h"

typedef struct _GtkWidget GtkWidget;
typedef struct _GtkTooltips GtkTooltips;
typedef struct _GtkListStore GtkListStore;

class EffectArgumentItem
{
protected:
	// The argument this row is referring to
	ResponseEffect::Argument& _arg;
	GtkWidget* _labelBox;
	GtkWidget* _descBox;
	GtkTooltips* _tooltips;

public:
	EffectArgumentItem(ResponseEffect::Argument& arg, GtkTooltips* tooltips);

	// destructor
	virtual ~EffectArgumentItem() {}
	
	/** greebo: This retrieves the string representation of the
	 * 			current value of this row. This has to be
	 * 			implemented by the derived classes.
	 */
	virtual std::string getValue() = 0;
	
	// Retrieve the label widget
	virtual GtkWidget* getLabelWidget();
	
	// Retrieve the edit widgets (abstract)
	virtual GtkWidget* getEditWidget() = 0;
	
	// Retrieves the help widget (a question mark with a tooltip)
	virtual GtkWidget* getHelpWidget();
	
	/** greebo: This saves the value to the according response effect.
	 */
	virtual void save();
};

/** greebo: This is an item querying a simple string
 */
class StringArgument :
	public EffectArgumentItem
{
protected:
	GtkWidget* _entry;

public:
	StringArgument(ResponseEffect::Argument& arg, GtkTooltips* tooltips);
	
	virtual GtkWidget* getEditWidget();
	virtual std::string getValue();
};

/** greebo: This is an item querying a float (derives from string)
 */
class FloatArgument :
	public StringArgument
{
public:
	FloatArgument(ResponseEffect::Argument& arg, GtkTooltips* tooltips) :
		StringArgument(arg, tooltips)
	{}
};

/** greebo: This is an item querying a vector (derives from string)
 */
class VectorArgument :
	public StringArgument
{
public:
	VectorArgument(ResponseEffect::Argument& arg, GtkTooltips* tooltips) :
		StringArgument(arg, tooltips)
	{}
};

class BooleanArgument :
	public EffectArgumentItem
{
	GtkWidget* _checkButton;
public:
	BooleanArgument(ResponseEffect::Argument& arg, GtkTooltips* tooltips);
	
	virtual GtkWidget* getEditWidget();
	virtual std::string getValue();
};

/** greebo: This is an item querying an entity name (entry/dropdown combo)
 */
class EntityArgument :
	public EffectArgumentItem
{
	GtkListStore* _entityStore;
	GtkWidget* _comboBox;	
public:
	// Pass the entity liststore to this item so that the auto-completion
	// of the entity combo box works correctly 
	EntityArgument(ResponseEffect::Argument& arg, 
				   GtkTooltips* tooltips, 
				   GtkListStore* entityStore);
	
	virtual GtkWidget* getEditWidget();
	virtual std::string getValue();
};

/** greebo: This is an item querying an stimtype (dropdown combo)
 */
class StimTypeArgument :
	public EffectArgumentItem
{
	GtkListStore* _stimTypeStore;
	GtkWidget* _comboBox;
public:
	// Pass the reference to the StimType helper class  
	StimTypeArgument(ResponseEffect::Argument& arg, 
				   GtkTooltips* tooltips,
				   GtkListStore* stimTypeStore);
	
	virtual GtkWidget* getEditWidget();
	virtual std::string getValue();
};

#endif /*EFFECTARGUMENTITEM_H_*/
