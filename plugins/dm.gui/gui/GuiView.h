#ifndef GuiView_h__
#define GuiView_h__

#include "gtkutil/ifc/Widget.h"
#include "gtkutil/GLWidget.h"
#include "math/Vector2.h"
#include "GuiRenderer.h"
#include "Gui.h"

#include "GuiManager.h"

namespace gui
{

/**
 * greebo: Use this class to add a GUI display to your dialogs.
 * It is owning a GL widget as well as a GuiRenderer which
 * is taking care of rendering the GUI elements to GL.
 */
class GuiView :
	public gtkutil::Widget
{
protected:
	// The top-level widget for packing this into a parent container
	GtkWidget* _widget;

	// The GL widget
	gtkutil::GLWidgetPtr _glWidget;

	// The GUI renderer is submitting stuff to GL
	GuiRenderer _renderer;

	// The GUI to render
	GuiPtr _gui;

	// The dimensions of the GL widget in pixels.
	Vector2 _windowDims;

public:
	GuiView();

	virtual ~GuiView() {}

	// Sets the GUI to render (by VFS path)
	void setGui(const std::string& gui)
	{
		setGui(GuiManager::Instance().getGui(gui));
	}

	// Sets the GUI to render (can be NULL to clear this view)
	virtual void setGui(const GuiPtr& gui);

	// Returns the current GUI (can be NULL)
	const GuiPtr& getGui();

	/** 
	 * Initialise the GL view. This clears the window and sets up the 
	 * initial matrices.
	 */
	void initialiseView();

	// Triggers a redraw
	void redraw();

protected:
	// Widget implementation
	virtual GtkWidget* _getWidget() const
	{
		return _widget;
	}

	// Performs the actual GL setup and drawing
	virtual void draw();

	// Calculates the visible area
	virtual void setGLViewPort();

private:
	static void onSizeAllocate(GtkWidget* widget, GtkAllocation* allocation, GuiView* self);
	static void onGLDraw(GtkWidget*, GdkEventExpose*, GuiView* self);
};
typedef boost::shared_ptr<GuiView> GuiViewPtr;

}

#endif // GuiView_h__