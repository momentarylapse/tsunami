#include "../../file/file.h"
#include "../kaba.h"
#include "../../config.h"
#include "common.h"

#ifdef _X_USE_HUI_
	#include "../../hui/hui.h"
#else
	we are re screwed.... TODO: test for _X_USE_HUI_
#endif

namespace Kaba{

#ifdef _X_USE_HUI_
	static hui::Event *_event;
	static hui::Painter *_painter;
	#define GetDAWindow(x)			long(&_win->x)-long(_win)
	#define GetDAEvent(x)	long(&_event->x)-long(_event)
	#define GetDAPainter(x)	long(&_painter->x)-long(_painter)
#else
	#define GetDAWindow(x)		0
	#define GetDAEvent(x)	0
	#define GetDAPainter(x)	0
#endif

#ifdef _X_USE_HUI_
	#define hui_p(p)		(void*)p
#else
	#define hui_p(p)		NULL
#endif


extern Class *TypeIntList;
extern Class *TypeIntPs;
extern Class *TypeComplexList;
extern Class *TypeImage;
Class *TypeHuiWindowP;

void SIAddPackageHui()
{
	add_package("hui", false);
	
	Class*
	TypeHuiMenu		= add_type  ("Menu",  sizeof(hui::Menu));
	Class*
	TypeHuiMenuP	= add_type_p("Menu*", TypeHuiMenu);
	Class*
	TypeHuiPanel	= add_type  ("Panel", sizeof(hui::Panel));
	Class*
	TypeHuiWindow	= add_type  ("Window", sizeof(hui::Window));
	TypeHuiWindowP	= add_type_p("Window*",	TypeHuiWindow);
	Class*
	TypeHuiNixWindow= add_type  ("NixWindow", sizeof(hui::Window));
	Class*
	TypeHuiDialog	= add_type  ("Dialog", sizeof(hui::Window));
	Class*
	TypeHuiFixedDialog= add_type  ("FixedDialog", sizeof(hui::Window));
	Class*
	TypeHuiEvent	= add_type  ("Event", sizeof(hui::Event));
	Class*
	TypeHuiEventP	= add_type_p("Event*", TypeHuiEvent);
	Class*
	TypeHuiPainter	= add_type  ("Painter", sizeof(hui::Painter));
	Class*
	TypeHuiPainterP	= add_type_p("Painter*", TypeHuiPainter);
	Class*
	TypeHuiTimer	= add_type  ("Timer", sizeof(hui::Timer));
	Class*
	TypeHuiConfiguration = add_type  ("Configuration", sizeof(hui::Configuration));

	
	add_func("GetKeyName",									TypeString,	hui_p(&hui::GetKeyName));
		func_add_param("id",		TypeInt);

	add_class(TypeHuiMenu);
		class_add_func(IDENTIFIER_FUNC_INIT,		TypeVoid,		mf(&hui::Menu::__init__));
		class_add_func("popup",	TypeVoid,		mf(&hui::Menu::openPopup));
			func_add_param("w",			TypeHuiWindowP);
			func_add_param("x",			TypeInt);
			func_add_param("y",			TypeInt);
		class_add_func("add",		TypeVoid,		mf(&hui::Menu::addItem));
			func_add_param("name",		TypeString);
			func_add_param("id",		TypeInt);
		class_add_func("addWithImage",	TypeVoid,		mf(&hui::Menu::addItemImage));
			func_add_param("name",		TypeString);
			func_add_param("image",		TypeInt);
			func_add_param("id",		TypeInt);
		class_add_func("addCheckable",	TypeVoid,		mf(&hui::Menu::addItemCheckable));
			func_add_param("name",		TypeString);
			func_add_param("id",		TypeInt);
		class_add_func("addSeparator",	TypeVoid,		mf(&hui::Menu::addSeparator));
		class_add_func("addSubMenu",	TypeVoid,		mf(&hui::Menu::addSubMenu));
			func_add_param("name",		TypeString);
			func_add_param("id",		TypeInt);
			func_add_param("sub_menu",	TypeHuiMenuP);

		add_class(TypeHuiPanel);
			class_add_func(IDENTIFIER_FUNC_INIT,		TypeVoid,		mf(&hui::Panel::__init__));
			class_add_func_virtual(IDENTIFIER_FUNC_DELETE,		TypeVoid,		mf(&hui::Panel::__delete__));
			class_add_func("setBorderWidth",			TypeVoid,		mf(&hui::Panel::setBorderWidth));
				func_add_param("width",		TypeInt);
			class_add_func("setDecimals",			TypeVoid,		mf(&hui::Panel::setDecimals));
				func_add_param("decimals",		TypeInt);
			class_add_func("activate",										TypeVoid,		mf(&hui::Panel::activate));
				func_add_param("id",		TypeInt);
			class_add_func("isActive",										TypeVoid,		mf(&hui::Panel::isActive));
				func_add_param("id",	TypeString);
			class_add_func("fromSource",	TypeVoid,		mf(&hui::Panel::fromSource));
				func_add_param("source",		TypeString);
			class_add_func("addButton",										TypeVoid,		mf(&hui::Panel::addButton));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addDefButton",										TypeVoid,		mf(&hui::Panel::addDefButton));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addCheckBox",								TypeVoid,		mf(&hui::Panel::addCheckBox));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addLabel",										TypeVoid,		mf(&hui::Panel::addLabel));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addEdit",										TypeVoid,		mf(&hui::Panel::addEdit));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addMultilineEdit",										TypeVoid,		mf(&hui::Panel::addMultilineEdit));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addGroup",										TypeVoid,		mf(&hui::Panel::addGroup));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addComboBox",								TypeVoid,		mf(&hui::Panel::addComboBox));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addTabControl",								TypeVoid,		mf(&hui::Panel::addTabControl));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("setTarget",				TypeVoid,		mf(&hui::Panel::setTarget));
				func_add_param("id",		TypeString);
				func_add_param("page",		TypeInt);
			class_add_func("addListView",								TypeVoid,		mf(&hui::Panel::addListView));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addTreeView",								TypeVoid,		mf(&hui::Panel::addTreeView));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addIconView",								TypeVoid,		mf(&hui::Panel::addIconView));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addProgressBar",						TypeVoid,		mf(&hui::Panel::addProgressBar));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addSlider",										TypeVoid,		mf(&hui::Panel::addSlider));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addDrawingArea",										TypeVoid,		mf(&hui::Panel::addDrawingArea));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addGrid",										TypeVoid,		mf(&hui::Panel::addGrid));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addSpinButton",										TypeVoid,		mf(&hui::Panel::addSpinButton));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addRadioButton",										TypeVoid,		mf(&hui::Panel::addRadioButton));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addScroller",								TypeVoid,		mf(&hui::Panel::addScroller));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addExpander",								TypeVoid,		mf(&hui::Panel::addExpander));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addSeparator",								TypeVoid,		mf(&hui::Panel::addSeparator));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addPaned",								TypeVoid,		mf(&hui::Panel::addPaned));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addRevealer",								TypeVoid,		mf(&hui::Panel::addRevealer));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("setString",						TypeVoid,		mf(&hui::Panel::setString));
				func_add_param("id",		TypeString);
				func_add_param("s",			TypeString);
			class_add_func("getString",						TypeString,		mf(&hui::Panel::getString));
				func_add_param("id",		TypeString);
			class_add_func("setFloat",						TypeVoid,		mf(&hui::Panel::setFloat));
				func_add_param("id",		TypeString);
				func_add_param("f",			TypeFloat32);
			class_add_func("getFloat",						TypeFloat32,		mf(&hui::Panel::getFloat));
				func_add_param("id",		TypeString);
			class_add_func("enable",								TypeVoid,		mf(&hui::Panel::enable));
				func_add_param("id",		TypeString);
				func_add_param("enabled",	TypeBool);
			class_add_func("isEnabled",					TypeBool,		mf(&hui::Panel::isEnabled));
				func_add_param("id",		TypeString);
			class_add_func("check",								TypeVoid,		mf(&hui::Panel::check));
				func_add_param("id",		TypeString);
				func_add_param("checked",	TypeBool);
			class_add_func("isChecked",					TypeBool,		mf(&hui::Panel::isChecked));
				func_add_param("id",		TypeString);
			class_add_func("getInt",			TypeInt,		mf(&hui::Panel::getInt));
				func_add_param("id",		TypeString);
			class_add_func("getSelection",			TypeIntList,		mf(&hui::Panel::getSelection));
				func_add_param("id",		TypeString);
			class_add_func("setInt",			TypeVoid,		mf(&hui::Panel::setInt));
				func_add_param("id",		TypeString);
				func_add_param("i",			TypeInt);
			class_add_func("setImage",			TypeVoid,		mf(&hui::Panel::setImage));
				func_add_param("id",		TypeString);
				func_add_param("image",		TypeString);
			class_add_func("getCell",						TypeString,		mf(&hui::Panel::getCell));
				func_add_param("id",		TypeString);
				func_add_param("row",		TypeInt);
				func_add_param("column",	TypeInt);
			class_add_func("setCell",						TypeVoid,		mf(&hui::Panel::setCell));
				func_add_param("id",		TypeString);
				func_add_param("row",		TypeInt);
				func_add_param("column",	TypeInt);
				func_add_param("s",			TypeString);
			class_add_func("setOptions",			TypeVoid,		mf(&hui::Panel::setOptions));
				func_add_param("id",		TypeString);
				func_add_param("options",	TypeString);
			class_add_func("reset",								TypeVoid,		mf(&hui::Panel::reset));
				func_add_param("id",		TypeString);
			class_add_func("redraw",								TypeVoid,		mf(&hui::Panel::redraw));
				func_add_param("id",		TypeString);
			class_add_func("event",						TypeInt,		mf(&hui::Panel::_kaba_event));
				func_add_param("id",			TypeString);
				func_add_param("func",			TypePointer);
			class_add_func("eventO",						TypeInt,		mf(&hui::Panel::_kaba_eventO));
				func_add_param("id",			TypeString);
				func_add_param("handler",		TypePointer);
				func_add_param("func",			TypePointer);
			class_add_func("eventX",						TypeInt,		mf(&hui::Panel::_kaba_eventX));
				func_add_param("id",			TypeString);
				func_add_param("msg",			TypeString);
				func_add_param("func",			TypePointer);
			class_add_func("eventOX",						TypeInt,		mf(&hui::Panel::_kaba_eventOX));
				func_add_param("id",			TypeString);
				func_add_param("msg",			TypeString);
				func_add_param("handler",		TypePointer);
				func_add_param("func",			TypePointer);
			class_add_func("removeEventHandler",		TypeVoid,		mf(&hui::Panel::removeEventHandler));
				func_add_param("uid",			TypeInt);
			//class_add_func("beginDraw",								TypeHuiPainterP,		mf(&hui::HuiPanel::beginDraw));
			//	func_add_param("id",		TypeString);
			class_set_vtable(hui::Panel);


	add_class(TypeHuiWindow);
		TypeHuiWindow->derive_from(TypeHuiPanel, false);
		TypeHuiWindow->vtable = TypeHuiPanel->vtable;
		class_add_func(IDENTIFIER_FUNC_INIT,		TypeVoid,		mf(&hui::Window::__init_ext__), FLAG_OVERRIDE);
			func_add_param("title",		TypeString);
			func_add_param("x",		TypeInt);
			func_add_param("y",		TypeInt);
			func_add_param("width",		TypeInt);
			func_add_param("height",		TypeInt);
		class_add_func_virtual(IDENTIFIER_FUNC_DELETE,		TypeVoid,		mf(&hui::Window::__delete__), FLAG_OVERRIDE);
		class_add_func("run",			TypeVoid,		mf(&hui::Window::run));
		class_add_func("destroy",		TypeVoid,		mf(&hui::Window::destroy));
		class_add_func("show",			TypeVoid,		mf(&hui::Window::show));
		class_add_func("hide",			TypeVoid,		mf(&hui::Window::hide));

		class_add_func("setMenu",			TypeVoid,		mf(&hui::Window::setMenu));
			func_add_param("menu",		TypeHuiMenuP);
		class_add_func("setMaximized",		TypeVoid,		mf(&hui::Window::setMaximized));
			func_add_param("max",		TypeBool);
		class_add_func("isMaximized",		TypeBool,		mf(&hui::Window::isMaximized));
		class_add_func("isMinimized",		TypeBool,		mf(&hui::Window::isMinimized));
		class_add_func("setID",			TypeVoid,		mf(&hui::Window::setID));
			func_add_param("id",		TypeInt);
		class_add_func("setFullscreen",				TypeVoid,		mf(&hui::Window::setFullscreen));
			func_add_param("fullscreen",TypeBool);
		class_add_func("setTitle",										TypeVoid,		mf(&hui::Window::setTitle));
			func_add_param("title",		TypeString);
		class_add_func("setPosition",								TypeVoid,		mf(&hui::Window::setPosition));
			func_add_param("x",			TypeInt);
			func_add_param("y",			TypeInt);
	//add_func("setOuterior",								TypeVoid,		2,	TypePointer,"win",
	//																										TypeIRect,"r");
	//add_func("getOuterior",								TypeIRect,		1,	TypePointer,"win");
	//add_func("setInerior",								TypeVoid,		2,	TypePointer,"win",
	//																										TypeIRect,"r");
	//add_func("getInterior",									TypeIRect,		1,	TypePointer,"win");
		class_add_func("setCursorPos",								TypeVoid,		mf(&hui::Window::setCursorPos));
			func_add_param("x",			TypeInt);
			func_add_param("y",			TypeInt);
		class_add_func("getMouse",								TypeBool,		mf(&hui::Window::getMouse));
			func_add_param("x",			TypeIntPs);
			func_add_param("y",			TypeIntPs);
			func_add_param("button",	TypeInt);
			func_add_param("change",	TypeInt);
		class_add_func("getKey",							TypeBool,		mf(&hui::Window::getKey));
			func_add_param("key",			TypeInt);
		class_add_func_virtual("onMouseMove", TypeVoid, mf(&hui::Window::onMouseMove));
		class_add_func_virtual("onMouseWheel", TypeVoid, mf(&hui::Window::onMouseWheel));
		class_add_func_virtual("onLeftButtonDown", TypeVoid, mf(&hui::Window::onLeftButtonDown));
		class_add_func_virtual("onMiddleButtonDown", TypeVoid, mf(&hui::Window::onMiddleButtonDown));
		class_add_func_virtual("onRightButtonDown", TypeVoid, mf(&hui::Window::onRightButtonDown));
		class_add_func_virtual("onLeftButtonUp", TypeVoid, mf(&hui::Window::onLeftButtonUp));
		class_add_func_virtual("onMiddleButtonUp", TypeVoid, mf(&hui::Window::onMiddleButtonUp));
		class_add_func_virtual("onRightButtonUp", TypeVoid, mf(&hui::Window::onRightButtonUp));
		class_add_func_virtual("onDoubleClick", TypeVoid, mf(&hui::Window::onDoubleClick));
		class_add_func_virtual("onCloseRequest", TypeVoid, mf(&hui::Window::onCloseRequest));
		class_add_func_virtual("onKeyDown", TypeVoid, mf(&hui::Window::onKeyDown));
		class_add_func_virtual("onKeyUp", TypeVoid, mf(&hui::Window::onKeyUp));
		class_add_func_virtual("onDraw", TypeVoid, mf(&hui::Window::onDraw));
			func_add_param("p", TypeHuiPainterP);
		class_set_vtable(hui::Window);

	add_class(TypeHuiNixWindow);
		TypeHuiNixWindow->derive_from(TypeHuiWindow, false);
		TypeHuiNixWindow->vtable = TypeHuiWindow->vtable;
		class_add_func(IDENTIFIER_FUNC_INIT,		TypeVoid,		mf(&hui::NixWindow::__init_ext__), FLAG_OVERRIDE);
			func_add_param("title",		TypeString);
			func_add_param("x",		TypeInt);
			func_add_param("y",		TypeInt);
			func_add_param("width",		TypeInt);
			func_add_param("height",		TypeInt);
		class_add_func_virtual(IDENTIFIER_FUNC_DELETE,		TypeVoid,		mf(&hui::Window::__delete__), FLAG_OVERRIDE);
		class_set_vtable(hui::Window);

	add_class(TypeHuiDialog);
		TypeHuiDialog->derive_from(TypeHuiWindow, false);
		TypeHuiDialog->vtable = TypeHuiWindow->vtable;
		class_add_func(IDENTIFIER_FUNC_INIT,		TypeVoid,		mf(&hui::Dialog::__init_ext__), FLAG_OVERRIDE);
			func_add_param("title",		TypeString);
			func_add_param("width",		TypeInt);
			func_add_param("height",		TypeInt);
			func_add_param("root",		TypeHuiWindowP);
			func_add_param("allow_root",TypeBool);
		class_add_func_virtual(IDENTIFIER_FUNC_DELETE,		TypeVoid,		mf(&hui::Window::__delete__), FLAG_OVERRIDE);
		class_set_vtable(hui::Window);

	add_class(TypeHuiFixedDialog);
		TypeHuiFixedDialog->derive_from(TypeHuiWindow, false);
		TypeHuiFixedDialog->vtable = TypeHuiWindow->vtable;
		class_add_func(IDENTIFIER_FUNC_INIT,		TypeVoid,		mf(&hui::FixedDialog::__init_ext__), FLAG_OVERRIDE);
			func_add_param("title",		TypeString);
			func_add_param("width",		TypeInt);
			func_add_param("height",		TypeInt);
			func_add_param("root",		TypeHuiWindowP);
			func_add_param("allow_root",TypeBool);
		class_add_func_virtual(IDENTIFIER_FUNC_DELETE,		TypeVoid,		mf(&hui::Window::__delete__), FLAG_OVERRIDE);
		class_set_vtable(hui::Window);
	
	add_class(TypeHuiPainter);
		class_add_element("width",		TypeInt,	GetDAPainter(width));
		class_add_element("height",		TypeInt,	GetDAPainter(height));
		//class_add_func_virtual("end",								TypeVoid,		mf(&hui::HuiPainter::end));
		class_add_func_virtual("setColor",								TypeVoid,		mf(&hui::Painter::setColor));
			func_add_param("c",			TypeColor);
		class_add_func_virtual("setLineWidth",								TypeVoid,		mf(&hui::Painter::setLineWidth));
			func_add_param("w",			TypeFloat32);
		class_add_func_virtual("setAntialiasing",								TypeVoid,		mf(&hui::Painter::setAntialiasing));
			func_add_param("enabled",			TypeBool);
		class_add_func_virtual("setFontSize",								TypeVoid,		mf(&hui::Painter::setFontSize));
			func_add_param("size",			TypeFloat32);
		class_add_func_virtual("setFill",								TypeVoid,		mf(&hui::Painter::setFill));
			func_add_param("fill",			TypeBool);
		class_add_func_virtual("clip",								TypeVoid,		mf(&hui::Painter::clip));
			func_add_param("r",			TypeRect);
		class_add_func_virtual("drawPoint",								TypeVoid,		mf(&hui::Painter::drawPoint));
			func_add_param("x",			TypeFloat32);
			func_add_param("y",			TypeFloat32);
		class_add_func_virtual("drawLine",								TypeVoid,		mf(&hui::Painter::drawLine));
			func_add_param("x1",		TypeFloat32);
			func_add_param("y1",		TypeFloat32);
			func_add_param("x2",		TypeFloat32);
			func_add_param("y2",		TypeFloat32);
		class_add_func_virtual("drawLines",								TypeVoid,		mf(&hui::Painter::drawLines));
			func_add_param("p",			TypeComplexList);
		class_add_func_virtual("drawPolygon",								TypeVoid,		mf(&hui::Painter::drawPolygon));
			func_add_param("p",			TypeComplexList);
		class_add_func_virtual("drawRect",								TypeVoid,		mf((void (hui::Painter::*) (float,float,float,float))&hui::Painter::drawRect));
			func_add_param("x",			TypeFloat32);
			func_add_param("y",			TypeFloat32);
			func_add_param("w",			TypeFloat32);
			func_add_param("h",			TypeFloat32);
		class_add_func_virtual("drawCircle",								TypeVoid,		mf(&hui::Painter::drawCircle));
			func_add_param("x",			TypeFloat32);
			func_add_param("y",			TypeFloat32);
			func_add_param("r",			TypeFloat32);
		class_add_func_virtual("drawStr",								TypeVoid,		mf(&hui::Painter::drawStr));
			func_add_param("x",			TypeFloat32);
			func_add_param("y",			TypeFloat32);
			func_add_param("str",		TypeString);
		class_add_func_virtual("drawImage",								TypeVoid,		mf(&hui::Painter::drawImage));
			func_add_param("x",			TypeFloat32);
			func_add_param("y",			TypeFloat32);
			func_add_param("image",		TypeImage);
		class_set_vtable(hui::Painter);


	add_class(TypeHuiTimer);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, mf(&hui::Timer::reset));
		class_add_func("get", TypeFloat32, mf(&hui::Timer::get));
		class_add_func("reset", TypeVoid, mf(&hui::Timer::reset));
		class_add_func("peek", TypeFloat32, mf(&hui::Timer::peek));


	add_class(TypeHuiConfiguration);
		class_add_func("setInt",								TypeVoid,	mf(&hui::Configuration::getInt));
			func_add_param("name",		TypeString);
			func_add_param("value",		TypeInt);
		class_add_func("setFloat",								TypeVoid,	mf(&hui::Configuration::getFloat));
			func_add_param("name",		TypeString);
			func_add_param("value",		TypeFloat32);
		class_add_func("setBool",								TypeVoid,	mf(&hui::Configuration::getBool));
			func_add_param("name",		TypeString);
			func_add_param("value",		TypeBool);
		class_add_func("setStr",								TypeVoid,	mf(&hui::Configuration::getStr));
			func_add_param("name",		TypeString);
			func_add_param("value",		TypeString);
		class_add_func("getInt",								TypeInt,	mf(&hui::Configuration::setInt));
			func_add_param("name",		TypeString);
			func_add_param("default",	TypeInt);
		class_add_func("getFloat",								TypeFloat32,	mf(&hui::Configuration::setFloat));
			func_add_param("name",		TypeString);
			func_add_param("default",	TypeFloat32);
		class_add_func("getBool",								TypeBool,	mf(&hui::Configuration::setBool));
			func_add_param("name",		TypeString);
			func_add_param("default",	TypeBool);
		class_add_func("getStr",								TypeString,	mf(&hui::Configuration::setStr));
			func_add_param("name",		TypeString);
			func_add_param("default",	TypeString);
	
	// user interface
	/*add_func("HuiSetIdleFunction",	TypeVoid,		(void*)&hui::SetIdleFunction);
		func_add_param("idle_func",	TypePointer);
	add_func("HuiAddKeyCode",	TypeVoid,		(void*)&hui::AddKeyCode);
		func_add_param("id",	TypeString);
		func_add_param("ley_code",	TypeInt);
	add_func("HuiAddCommand",	TypeVoid,		(void*)&hui::AddCommand);
		func_add_param("id",	TypeString);
		func_add_param("image",	TypeString);
		func_add_param("key_code",	TypeInt);
		func_add_param("func",	TypePointer);*/
	add_func("HuiGetEvent",	TypeHuiEventP,		(void*)&hui::GetEvent);
	/*add_func("HuiRun",				TypeVoid,		(void*)&hui::Run);
	add_func("HuiEnd",				TypeVoid,		(void*)&hui::End);
	add_func("HuiDoSingleMainLoop",	TypeVoid,	(void*)&hui::doSingleMainLoop);*/
	add_func("HuiSleep",			TypeVoid,	(void*)&hui::Sleep);
		func_add_param("duration",		TypeFloat32);
	add_func("HuiFileDialogOpen",	TypeBool,	(void*)&hui::FileDialogOpen);
		func_add_param("root",		TypeHuiWindowP);
		func_add_param("title",		TypeString);
		func_add_param("dir",		TypeString);
		func_add_param("show_filter",	TypeString);
		func_add_param("filter",	TypeString);
	add_func("HuiFileDialogSave",	TypeBool,	(void*)&hui::FileDialogSave);
		func_add_param("root",		TypeHuiWindowP);
		func_add_param("title",		TypeString);
		func_add_param("dir",		TypeString);
		func_add_param("show_filter",	TypeString);
		func_add_param("filter",	TypeString);
	add_func("HuiFileDialogDir",	TypeBool,	(void*)&hui::FileDialogDir);
		func_add_param("root",		TypeHuiWindowP);
		func_add_param("title",		TypeString);
		func_add_param("dir",		TypeString);
	add_func("HuiQuestionBox",		TypeString,	(void*)&hui::QuestionBox);
		func_add_param("root",		TypeHuiWindowP);
		func_add_param("title",		TypeString);
		func_add_param("text",		TypeString);
		func_add_param("allow_cancel",	TypeBool);
	add_func("HuiInfoBox",			TypeVoid,			(void*)&hui::InfoBox);
		func_add_param("root",		TypeHuiWindowP);
		func_add_param("title",		TypeString);
		func_add_param("text",		TypeString);
	add_func("HuiErrorBox",			TypeVoid,		(void*)&hui::ErrorBox);
		func_add_param("root",		TypeHuiWindowP);
		func_add_param("title",		TypeString);
		func_add_param("text",		TypeString);

	// clipboard
	add_func("HuiCopyToClipboard",	TypeVoid,			(void*)&hui::Clipboard::Copy);
		func_add_param("buffer",	TypeString);
	add_func("HuiPasteFromClipboard",	TypeString,		(void*)&hui::Clipboard::Paste);
	add_func("HuiOpenDocument",		TypeVoid,			(void*)&hui::OpenDocument);
		func_add_param("filename",	TypeString);
	add_func("HuiSetImage",			TypeString,			(void*)&hui::SetImage);
		func_add_param("image",		TypeImage);

	add_class(TypeHuiEvent);
		class_add_element("id",			TypeString,	GetDAEvent(id));
		class_add_element("message",	TypeString,	GetDAEvent(message));
		class_add_element("mouse_x",	TypeFloat32,	GetDAEvent(mx));
		class_add_element("mouse_y",	TypeFloat32,	GetDAEvent(my));
		class_add_element("scroll_x",		TypeFloat32,	GetDAEvent(scroll_x));
		class_add_element("scroll_y",		TypeFloat32,	GetDAEvent(scroll_y));
		class_add_element("key",		TypeInt,	GetDAEvent(key));
		class_add_element("key_code",	TypeInt,	GetDAEvent(key_code));
		class_add_element("width",		TypeInt,	GetDAEvent(width));
		class_add_element("height",		TypeInt,	GetDAEvent(height));
		class_add_element("button_l",	TypeBool,	GetDAEvent(lbut));
		class_add_element("button_m",	TypeBool,	GetDAEvent(mbut));
		class_add_element("button_r",	TypeBool,	GetDAEvent(rbut));
		class_add_element("text",		TypeString,	GetDAEvent(text));
		class_add_element("row",		TypeInt,	GetDAEvent(row));
		class_add_element("column",		TypeInt,	GetDAEvent(column));

	// key ids (int)
	add_const("KEY_CONTROL",TypeInt,(void*)hui::KEY_CONTROL);
	add_const("KEY_LEFT_CONTROL",TypeInt,(void*)hui::KEY_LCONTROL);
	add_const("KEY_RIGHT_CONTROL",TypeInt,(void*)hui::KEY_RCONTROL);
	add_const("KEY_SHIFT",TypeInt,(void*)hui::KEY_SHIFT);
	add_const("KEY_LEFT_SHIFT",TypeInt,(void*)hui::KEY_LSHIFT);
	add_const("KEY_RIGHT_SHIFT",TypeInt,(void*)hui::KEY_RSHIFT);
	add_const("KEY_ALT",TypeInt,(void*)hui::KEY_ALT);
	add_const("KEY_LEFT_ALT",TypeInt,(void*)hui::KEY_LALT);
	add_const("KEY_RIGHT_ALT",TypeInt,(void*)hui::KEY_RALT);
	add_const("KEY_PLUS",TypeInt,(void*)hui::KEY_ADD);
	add_const("KEY_MINUS",TypeInt,(void*)hui::KEY_SUBTRACT);
	add_const("KEY_FENCE",TypeInt,(void*)hui::KEY_FENCE);
	add_const("KEY_END",TypeInt,(void*)hui::KEY_END);
	add_const("KEY_NEXT",TypeInt,(void*)hui::KEY_NEXT);
	add_const("KEY_PRIOR",TypeInt,(void*)hui::KEY_PRIOR);
	add_const("KEY_UP",TypeInt,(void*)hui::KEY_UP);
	add_const("KEY_DOWN",TypeInt,(void*)hui::KEY_DOWN);
	add_const("KEY_LEFT",TypeInt,(void*)hui::KEY_LEFT);
	add_const("KEY_RIGHT",TypeInt,(void*)hui::KEY_RIGHT);
	add_const("KEY_RETURN",TypeInt,(void*)hui::KEY_RETURN);
	add_const("KEY_ESCAPE",TypeInt,(void*)hui::KEY_ESCAPE);
	add_const("KEY_INSERT",TypeInt,(void*)hui::KEY_INSERT);
	add_const("KEY_DELETE",TypeInt,(void*)hui::KEY_DELETE);
	add_const("KEY_SPACE",TypeInt,(void*)hui::KEY_SPACE);
	add_const("KEY_F1",TypeInt,(void*)hui::KEY_F1);
	add_const("KEY_F2",TypeInt,(void*)hui::KEY_F2);
	add_const("KEY_F3",TypeInt,(void*)hui::KEY_F3);
	add_const("KEY_F4",TypeInt,(void*)hui::KEY_F4);
	add_const("KEY_F5",TypeInt,(void*)hui::KEY_F5);
	add_const("KEY_F6",TypeInt,(void*)hui::KEY_F6);
	add_const("KEY_F7",TypeInt,(void*)hui::KEY_F7);
	add_const("KEY_F8",TypeInt,(void*)hui::KEY_F8);
	add_const("KEY_F9",TypeInt,(void*)hui::KEY_F9);
	add_const("KEY_F10",TypeInt,(void*)hui::KEY_F10);
	add_const("KEY_F11",TypeInt,(void*)hui::KEY_F11);
	add_const("KEY_F12",TypeInt,(void*)hui::KEY_F12);
	add_const("KEY_0",TypeInt,(void*)hui::KEY_0);
	add_const("KEY_1",TypeInt,(void*)hui::KEY_1);
	add_const("KEY_2",TypeInt,(void*)hui::KEY_2);
	add_const("KEY_3",TypeInt,(void*)hui::KEY_3);
	add_const("KEY_4",TypeInt,(void*)hui::KEY_4);
	add_const("KEY_5",TypeInt,(void*)hui::KEY_5);
	add_const("KEY_6",TypeInt,(void*)hui::KEY_6);
	add_const("KEY_7",TypeInt,(void*)hui::KEY_7);
	add_const("KEY_8",TypeInt,(void*)hui::KEY_8);
	add_const("KEY_9",TypeInt,(void*)hui::KEY_9);
	add_const("KEY_A",TypeInt,(void*)hui::KEY_A);
	add_const("KEY_B",TypeInt,(void*)hui::KEY_B);
	add_const("KEY_C",TypeInt,(void*)hui::KEY_C);
	add_const("KEY_D",TypeInt,(void*)hui::KEY_D);
	add_const("KEY_E",TypeInt,(void*)hui::KEY_E);
	add_const("KEY_F",TypeInt,(void*)hui::KEY_F);
	add_const("KEY_G",TypeInt,(void*)hui::KEY_G);
	add_const("KEY_H",TypeInt,(void*)hui::KEY_H);
	add_const("KEY_I",TypeInt,(void*)hui::KEY_I);
	add_const("KEY_J",TypeInt,(void*)hui::KEY_J);
	add_const("KEY_K",TypeInt,(void*)hui::KEY_K);
	add_const("KEY_L",TypeInt,(void*)hui::KEY_L);
	add_const("KEY_M",TypeInt,(void*)hui::KEY_M);
	add_const("KEY_N",TypeInt,(void*)hui::KEY_N);
	add_const("KEY_O",TypeInt,(void*)hui::KEY_O);
	add_const("KEY_P",TypeInt,(void*)hui::KEY_P);
	add_const("KEY_Q",TypeInt,(void*)hui::KEY_Q);
	add_const("KEY_R",TypeInt,(void*)hui::KEY_R);
	add_const("KEY_S",TypeInt,(void*)hui::KEY_S);
	add_const("KEY_T",TypeInt,(void*)hui::KEY_T);
	add_const("KEY_U",TypeInt,(void*)hui::KEY_U);
	add_const("KEY_V",TypeInt,(void*)hui::KEY_V);
	add_const("KEY_W",TypeInt,(void*)hui::KEY_W);
	add_const("KEY_X",TypeInt,(void*)hui::KEY_X);
	add_const("KEY_Y",TypeInt,(void*)hui::KEY_Y);
	add_const("KEY_Z",TypeInt,(void*)hui::KEY_Z);
	add_const("KEY_BACKSPACE",TypeInt,(void*)hui::KEY_BACKSPACE);
	add_const("KEY_TAB",TypeInt,(void*)hui::KEY_TAB);
	add_const("KEY_HOME",TypeInt,(void*)hui::KEY_HOME);
	add_const("KEY_NUM_0",TypeInt,(void*)hui::KEY_NUM_0);
	add_const("KEY_NUM_1",TypeInt,(void*)hui::KEY_NUM_1);
	add_const("KEY_NUM_2",TypeInt,(void*)hui::KEY_NUM_2);
	add_const("KEY_NUM_3",TypeInt,(void*)hui::KEY_NUM_3);
	add_const("KEY_NUM_4",TypeInt,(void*)hui::KEY_NUM_4);
	add_const("KEY_NUM_5",TypeInt,(void*)hui::KEY_NUM_5);
	add_const("KEY_NUM_6",TypeInt,(void*)hui::KEY_NUM_6);
	add_const("KEY_NUM_7",TypeInt,(void*)hui::KEY_NUM_7);
	add_const("KEY_NUM_8",TypeInt,(void*)hui::KEY_NUM_8);
	add_const("KEY_NUM_9",TypeInt,(void*)hui::KEY_NUM_9);
	add_const("KEY_NUM_PLUS",TypeInt,(void*)hui::KEY_NUM_ADD);
	add_const("KEY_NUM_MINUS",TypeInt,(void*)hui::KEY_NUM_SUBTRACT);
	add_const("KEY_NUM_MULTIPLY",TypeInt,(void*)hui::KEY_NUM_MULTIPLY);
	add_const("KEY_NUM_DIVIDE",TypeInt,(void*)hui::KEY_NUM_DIVIDE);
	add_const("KEY_NUM_COMMA",TypeInt,(void*)hui::KEY_NUM_COMMA);
	add_const("KEY_NUM_ENTER",TypeInt,(void*)hui::KEY_NUM_ENTER);
	add_const("KEY_COMMA",TypeInt,(void*)hui::KEY_COMMA);
	add_const("KEY_DOT",TypeInt,(void*)hui::KEY_DOT);
	add_const("KEY_SMALLER",TypeInt,(void*)hui::KEY_SMALLER);
	add_const("KEY_SZ",TypeInt,(void*)hui::KEY_SZ);
	add_const("KEY_AE",TypeInt,(void*)hui::KEY_AE);
	add_const("KEY_OE",TypeInt,(void*)hui::KEY_OE);
	add_const("KEY_UE",TypeInt,(void*)hui::KEY_UE);
	add_const("NUM_KEYS",TypeInt,(void*)hui::NUM_KEYS);
	add_const("KEY_ANY",TypeInt,(void*)hui::KEY_ANY);

	add_ext_var("AppFilename",		TypeString,		hui_p(&hui::Application::filename));
	add_ext_var("AppDirectory",		TypeString,		hui_p(&hui::Application::directory));
	add_ext_var("AppDirectoryStatic",TypeString,		hui_p(&hui::Application::directory_static));
	add_ext_var("HuiFilename",		TypeString,		hui_p(&hui::Filename));
	//add_ext_var("HuiRunning",		TypeBool,		hui_p(&hui::HuiRunning));
	add_ext_var("HuiConfig",		TypeHuiConfiguration,	hui_p(&hui::Config));
}

};
