#include "../../file/file.h"
#include "../script.h"
#include "../../config.h"
#include "script_data_common.h"

#ifdef _X_USE_HUI_
	#include "../../hui/hui.h"
#else
	we are re screwed.... TODO: test for _X_USE_HUI_
#endif

namespace Script{

#ifdef _X_USE_HUI_
	static HuiWindow *_win;
	static HuiEvent *_event;
	static HuiPainter *_painter;
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


extern Type *TypeIntList;
extern Type *TypeIntPs;
extern Type *TypeComplexList;
extern Type *TypeImage;
Type *TypeHuiWindowP;

void SIAddPackageHui()
{
	msg_db_f("SIAddPackageHui", 3);

	add_package("hui", false);
	
	Type*
	TypeHuiMenu		= add_type  ("Menu",  sizeof(HuiMenu));
	Type*
	TypeHuiMenuP	= add_type_p("Menu*", TypeHuiMenu);
	Type*
	TypeHuiPanel	= add_type  ("Panel", sizeof(HuiPanel));
	Type*
	TypeHuiWindow	= add_type  ("Window", sizeof(HuiWindow));
	TypeHuiWindowP	= add_type_p("Window*",	TypeHuiWindow);
	Type*
	TypeHuiNixWindow= add_type  ("NixWindow", sizeof(HuiWindow));
	Type*
	TypeHuiDialog	= add_type  ("Dialog", sizeof(HuiWindow));
	Type*
	TypeHuiFixedDialog= add_type  ("FixedDialog", sizeof(HuiWindow));
	Type*
	TypeHuiEvent	= add_type  ("Event", sizeof(HuiEvent));
	Type*
	TypeHuiEventP	= add_type_p("Event*", TypeHuiEvent);
	Type*
	TypeHuiPainter	= add_type  ("Painter", sizeof(HuiPainter));
	Type*
	TypeHuiPainterP	= add_type_p("Painter*", TypeHuiPainter);
	Type*
	TypeHuiTimer	= add_type  ("Timer", sizeof(HuiTimer));
	Type*
	TypeHuiConfiguration = add_type  ("Configuration", sizeof(HuiConfiguration));

	
	add_func("GetKeyName",									TypeString,	hui_p(&HuiGetKeyName));
		func_add_param("id",		TypeInt);

	add_class(TypeHuiMenu);
		class_add_func("__init__",		TypeVoid,		mf(&HuiMenu::__init__));
		class_add_func("popup",	TypeVoid,		mf(&HuiMenu::OpenPopup));
			func_add_param("w",			TypeHuiWindowP);
			func_add_param("x",			TypeInt);
			func_add_param("y",			TypeInt);
		class_add_func("add",		TypeVoid,		mf(&HuiMenu::AddItem));
			func_add_param("name",		TypeString);
			func_add_param("id",		TypeInt);
		class_add_func("addWithImage",	TypeVoid,		mf(&HuiMenu::AddItemImage));
			func_add_param("name",		TypeString);
			func_add_param("image",		TypeInt);
			func_add_param("id",		TypeInt);
		class_add_func("addCheckable",	TypeVoid,		mf(&HuiMenu::AddItemCheckable));
			func_add_param("name",		TypeString);
			func_add_param("id",		TypeInt);
		class_add_func("addSeparator",	TypeVoid,		mf(&HuiMenu::AddSeparator));
		class_add_func("addSubMenu",	TypeVoid,		mf(&HuiMenu::AddSubMenu));
			func_add_param("name",		TypeString);
			func_add_param("id",		TypeInt);
			func_add_param("sub_menu",	TypeHuiMenuP);

		add_class(TypeHuiPanel);
			class_add_func("__init__",		TypeVoid,		mf(&HuiPanel::__init__));
			class_add_func_virtual("__delete__",		TypeVoid,		mf(&HuiPanel::__delete__));
			class_add_func("setBorderWidth",			TypeVoid,		mf(&HuiPanel::SetBorderWidth));
				func_add_param("width",		TypeInt);
			class_add_func("setDecimals",			TypeVoid,		mf(&HuiPanel::SetDecimals));
				func_add_param("decimals",		TypeInt);
			class_add_func("activate",										TypeVoid,		mf(&HuiPanel::Activate));
				func_add_param("id",		TypeInt);
			class_add_func("isActive",										TypeVoid,		mf(&HuiPanel::IsActive));
				func_add_param("id",	TypeString);
			class_add_func("fromSource",	TypeVoid,		mf(&HuiPanel::FromSource));
				func_add_param("source",		TypeString);
			class_add_func("addButton",										TypeVoid,		mf(&HuiPanel::AddButton));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addDefButton",										TypeVoid,		mf(&HuiPanel::AddDefButton));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addCheckBox",								TypeVoid,		mf(&HuiPanel::AddCheckBox));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addText",										TypeVoid,		mf(&HuiPanel::AddText));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addEdit",										TypeVoid,		mf(&HuiPanel::AddEdit));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addMultilineEdit",										TypeVoid,		mf(&HuiPanel::AddMultilineEdit));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addGroup",										TypeVoid,		mf(&HuiPanel::AddGroup));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addComboBox",								TypeVoid,		mf(&HuiPanel::AddComboBox));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addTabControl",								TypeVoid,		mf(&HuiPanel::AddTabControl));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("setTarget",				TypeVoid,		mf(&HuiPanel::SetTarget));
				func_add_param("id",		TypeString);
				func_add_param("page",		TypeInt);
			class_add_func("addListView",								TypeVoid,		mf(&HuiPanel::AddListView));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addTreeView",								TypeVoid,		mf(&HuiPanel::AddTreeView));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addIconView",								TypeVoid,		mf(&HuiPanel::AddIconView));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addProgressBar",						TypeVoid,		mf(&HuiPanel::AddProgressBar));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addSlider",										TypeVoid,		mf(&HuiPanel::AddSlider));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addDrawingArea",										TypeVoid,		mf(&HuiPanel::AddDrawingArea));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addGrid",										TypeVoid,		mf(&HuiPanel::AddControlTable));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addSpinButton",										TypeVoid,		mf(&HuiPanel::AddSpinButton));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("addRadioButton",										TypeVoid,		mf(&HuiPanel::AddRadioButton));
				func_add_param("title",		TypeString);
				func_add_param("x",			TypeInt);
				func_add_param("y",			TypeInt);
				func_add_param("width",		TypeInt);
				func_add_param("height",	TypeInt);
				func_add_param("id",		TypeString);
			class_add_func("setString",						TypeVoid,		mf(&HuiPanel::SetString));
				func_add_param("id",		TypeString);
				func_add_param("s",			TypeString);
			class_add_func("getString",						TypeString,		mf(&HuiPanel::GetString));
				func_add_param("id",		TypeString);
			class_add_func("setFloat",						TypeVoid,		mf(&HuiPanel::SetFloat));
				func_add_param("id",		TypeString);
				func_add_param("f",			TypeFloat32);
			class_add_func("getFloat",						TypeFloat32,		mf(&HuiPanel::GetFloat));
				func_add_param("id",		TypeString);
			class_add_func("enable",								TypeVoid,		mf(&HuiPanel::Enable));
				func_add_param("id",		TypeString);
				func_add_param("enabled",	TypeBool);
			class_add_func("isEnabled",					TypeBool,		mf(&HuiPanel::IsEnabled));
				func_add_param("id",		TypeString);
			class_add_func("check",								TypeVoid,		mf(&HuiPanel::Check));
				func_add_param("id",		TypeString);
				func_add_param("checked",	TypeBool);
			class_add_func("isChecked",					TypeBool,		mf(&HuiPanel::IsChecked));
				func_add_param("id",		TypeString);
			class_add_func("getInt",			TypeInt,		mf(&HuiPanel::GetInt));
				func_add_param("id",		TypeString);
			class_add_func("getMultiSelection",			TypeIntList,		mf(&HuiPanel::GetMultiSelection));
				func_add_param("id",		TypeString);
			class_add_func("setInt",			TypeVoid,		mf(&HuiPanel::SetInt));
				func_add_param("id",		TypeString);
				func_add_param("i",			TypeInt);
			class_add_func("setImage",			TypeVoid,		mf(&HuiPanel::SetImage));
				func_add_param("id",		TypeString);
				func_add_param("image",		TypeString);
			class_add_func("getCell",						TypeString,		mf(&HuiPanel::GetCell));
				func_add_param("id",		TypeString);
				func_add_param("row",		TypeInt);
				func_add_param("column",	TypeInt);
			class_add_func("setCell",						TypeVoid,		mf(&HuiPanel::SetCell));
				func_add_param("id",		TypeString);
				func_add_param("row",		TypeInt);
				func_add_param("column",	TypeInt);
				func_add_param("s",			TypeString);
			class_add_func("setOptions",			TypeVoid,		mf(&HuiPanel::SetOptions));
				func_add_param("id",		TypeString);
				func_add_param("options",	TypeString);
			class_add_func("reset",								TypeVoid,		mf(&HuiPanel::Reset));
				func_add_param("id",		TypeString);
			class_add_func("redraw",								TypeVoid,		mf(&HuiPanel::Redraw));
				func_add_param("id",		TypeString);
			class_add_func("event",						TypeVoid,		mf(&HuiPanel::Event));
				func_add_param("id",			TypeString);
				func_add_param("func",			TypePointer);
			class_add_func("eventX",						TypeVoid,		mf(&HuiPanel::EventX));
				func_add_param("id",			TypeString);
				func_add_param("msg",			TypeString);
				func_add_param("func",			TypePointer);
			class_add_func("eventM",						TypeVoid,		mf(&HuiPanel::_EventKM));
				func_add_param("id",			TypeString);
				func_add_param("handler",		TypePointer);
				func_add_param("func",			TypePointer);
			class_add_func("eventMX",						TypeVoid,		mf(&HuiPanel::_EventKMX));
				func_add_param("id",			TypeString);
				func_add_param("msg",			TypeString);
				func_add_param("handler",		TypePointer);
				func_add_param("func",			TypePointer);
			class_add_func("beginDraw",								TypeHuiPainterP,		mf(&HuiPanel::BeginDraw));
				func_add_param("id",		TypeString);
			class_set_vtable(HuiPanel);


	add_class(TypeHuiWindow);
		TypeHuiWindow->DeriveFrom(TypeHuiPanel, false);
		TypeHuiWindow->vtable = TypeHuiPanel->vtable;
		class_add_func("__init__",		TypeVoid,		mf(&HuiWindow::__init_ext__));
			func_add_param("title",		TypeString);
			func_add_param("x",		TypeInt);
			func_add_param("y",		TypeInt);
			func_add_param("width",		TypeInt);
			func_add_param("height",		TypeInt);
		class_add_func_virtual("__delete__",		TypeVoid,		mf(&HuiWindow::__delete__));
		class_add_func("run",			TypeString,		mf(&HuiWindow::Run));
		class_add_func("show",			TypeVoid,		mf(&HuiWindow::Show));
		class_add_func("hide",			TypeVoid,		mf(&HuiWindow::Hide));

		class_add_func("setMenu",			TypeVoid,		mf(&HuiWindow::SetMenu));
			func_add_param("menu",		TypeHuiMenuP);
		class_add_func("setMaximized",		TypeVoid,		mf(&HuiWindow::SetMaximized));
			func_add_param("max",		TypeBool);
		class_add_func("isMaximized",		TypeBool,		mf(&HuiWindow::IsMaximized));
		class_add_func("isMinimized",		TypeBool,		mf(&HuiWindow::IsMinimized));
		class_add_func("setID",			TypeVoid,		mf(&HuiWindow::SetID));
			func_add_param("id",		TypeInt);
		class_add_func("setFullscreen",				TypeVoid,		mf(&HuiWindow::SetFullscreen));
			func_add_param("fullscreen",TypeBool);
		class_add_func("setTitle",										TypeVoid,		mf(&HuiWindow::SetTitle));
			func_add_param("title",		TypeString);
		class_add_func("setPosition",								TypeVoid,		mf(&HuiWindow::SetPosition));
			func_add_param("x",			TypeInt);
			func_add_param("y",			TypeInt);
	//add_func("setOuterior",								TypeVoid,		2,	TypePointer,"win",
	//																										TypeIRect,"r");
	//add_func("getOuterior",								TypeIRect,		1,	TypePointer,"win");
	//add_func("setInerior",								TypeVoid,		2,	TypePointer,"win",
	//																										TypeIRect,"r");
	//add_func("getInterior",									TypeIRect,		1,	TypePointer,"win");
		class_add_func("setCursorPos",								TypeVoid,		mf(&HuiWindow::SetCursorPos));
			func_add_param("x",			TypeInt);
			func_add_param("y",			TypeInt);
		class_add_func("getMouse",								TypeBool,		mf(&HuiWindow::GetMouse));
			func_add_param("x",			TypeIntPs);
			func_add_param("y",			TypeIntPs);
			func_add_param("button",	TypeInt);
			func_add_param("change",	TypeInt);
		class_add_func("getKey",							TypeBool,		mf(&HuiWindow::GetKey));
			func_add_param("key",			TypeInt);
		class_add_func_virtual("onMouseMove", TypeVoid, mf(&HuiWindow::OnMouseMove));
		class_add_func_virtual("onMouseWheel", TypeVoid, mf(&HuiWindow::OnMouseWheel));
		class_add_func_virtual("onLeftButtonDown", TypeVoid, mf(&HuiWindow::OnLeftButtonDown));
		class_add_func_virtual("onMiddleButtonDown", TypeVoid, mf(&HuiWindow::OnMiddleButtonDown));
		class_add_func_virtual("onRightButtonDown", TypeVoid, mf(&HuiWindow::OnRightButtonDown));
		class_add_func_virtual("onLeftButtonUp", TypeVoid, mf(&HuiWindow::OnLeftButtonUp));
		class_add_func_virtual("onMiddleButtonUp", TypeVoid, mf(&HuiWindow::OnMiddleButtonUp));
		class_add_func_virtual("onRightButtonUp", TypeVoid, mf(&HuiWindow::OnRightButtonUp));
		class_add_func_virtual("onDoubleClick", TypeVoid, mf(&HuiWindow::OnDoubleClick));
		class_add_func_virtual("onCloseRequest", TypeVoid, mf(&HuiWindow::OnCloseRequest));
		class_add_func_virtual("onKeyDown", TypeVoid, mf(&HuiWindow::OnKeyDown));
		class_add_func_virtual("onKeyUp", TypeVoid, mf(&HuiWindow::OnKeyUp));
		class_add_func_virtual("onDraw", TypeVoid, mf(&HuiWindow::OnDraw));
		class_set_vtable(HuiWindow);

	add_class(TypeHuiNixWindow);
		TypeHuiNixWindow->DeriveFrom(TypeHuiWindow, false);
		TypeHuiNixWindow->vtable = TypeHuiWindow->vtable;
		class_add_func("__init__",		TypeVoid,		mf(&HuiNixWindow::__init_ext__));
			func_add_param("title",		TypeString);
			func_add_param("x",		TypeInt);
			func_add_param("y",		TypeInt);
			func_add_param("width",		TypeInt);
			func_add_param("height",		TypeInt);
		class_add_func_virtual("__delete__",		TypeVoid,		mf(&HuiWindow::__delete__));
		class_set_vtable(HuiWindow);

	add_class(TypeHuiDialog);
		TypeHuiDialog->DeriveFrom(TypeHuiWindow, false);
		TypeHuiDialog->vtable = TypeHuiWindow->vtable;
		class_add_func("__init__",		TypeVoid,		mf(&HuiDialog::__init_ext__));
			func_add_param("title",		TypeString);
			func_add_param("width",		TypeInt);
			func_add_param("height",		TypeInt);
			func_add_param("root",		TypeHuiWindowP);
			func_add_param("allow_root",TypeBool);
		class_add_func_virtual("__delete__",		TypeVoid,		mf(&HuiWindow::__delete__));
		class_set_vtable(HuiWindow);

	add_class(TypeHuiFixedDialog);
		TypeHuiFixedDialog->DeriveFrom(TypeHuiWindow, false);
		TypeHuiFixedDialog->vtable = TypeHuiWindow->vtable;
		class_add_func("__init__",		TypeVoid,		mf(&HuiFixedDialog::__init_ext__));
			func_add_param("title",		TypeString);
			func_add_param("width",		TypeInt);
			func_add_param("height",		TypeInt);
			func_add_param("root",		TypeHuiWindowP);
			func_add_param("allow_root",TypeBool);
		class_add_func_virtual("__delete__",		TypeVoid,		mf(&HuiWindow::__delete__));
		class_set_vtable(HuiWindow);
	
	add_class(TypeHuiPainter);
		class_add_element("width",		TypeInt,	GetDAPainter(width));
		class_add_element("height",		TypeInt,	GetDAPainter(height));
		class_add_func("end",								TypeVoid,		mf(&HuiPainter::end));
		class_add_func("setColor",								TypeVoid,		mf(&HuiPainter::setColor));
			func_add_param("c",			TypeColor);
		class_add_func("setLineWidth",								TypeVoid,		mf(&HuiPainter::setLineWidth));
			func_add_param("w",			TypeFloat32);
		class_add_func("setAntialiasing",								TypeVoid,		mf(&HuiPainter::setAntialiasing));
			func_add_param("enabled",			TypeBool);
		class_add_func("setFontSize",								TypeVoid,		mf(&HuiPainter::setFontSize));
			func_add_param("size",			TypeFloat32);
		class_add_func("clip",								TypeVoid,		mf(&HuiPainter::clip));
			func_add_param("r",			TypeRect);
		class_add_func("drawPoint",								TypeVoid,		mf(&HuiPainter::drawPoint));
			func_add_param("x",			TypeFloat32);
			func_add_param("y",			TypeFloat32);
		class_add_func("drawLine",								TypeVoid,		mf(&HuiPainter::drawLine));
			func_add_param("x1",		TypeFloat32);
			func_add_param("y1",		TypeFloat32);
			func_add_param("x2",		TypeFloat32);
			func_add_param("y2",		TypeFloat32);
		class_add_func("drawLines",								TypeVoid,		mf(&HuiPainter::drawLines));
			func_add_param("p",			TypeComplexList);
		class_add_func("drawPolygon",								TypeVoid,		mf(&HuiPainter::drawPolygon));
			func_add_param("p",			TypeComplexList);
		class_add_func("drawRect",								TypeVoid,		mf((void (HuiPainter::*) (float,float,float,float))&HuiPainter::drawRect));
			func_add_param("x",			TypeFloat32);
			func_add_param("y",			TypeFloat32);
			func_add_param("w",			TypeFloat32);
			func_add_param("h",			TypeFloat32);
		class_add_func("drawCircle",								TypeVoid,		mf(&HuiPainter::drawCircle));
			func_add_param("x",			TypeFloat32);
			func_add_param("y",			TypeFloat32);
			func_add_param("r",			TypeFloat32);
		class_add_func("drawStr",								TypeVoid,		mf(&HuiPainter::drawStr));
			func_add_param("x",			TypeFloat32);
			func_add_param("y",			TypeFloat32);
			func_add_param("str",		TypeString);
		class_add_func("drawImage",								TypeVoid,		mf(&HuiPainter::drawImage));
			func_add_param("x",			TypeFloat32);
			func_add_param("y",			TypeFloat32);
			func_add_param("image",		TypeImage);


	add_class(TypeHuiTimer);
		class_add_func("__init__", TypeVoid, mf(&HuiTimer::reset));
		class_add_func("get", TypeFloat32, mf(&HuiTimer::get));
		class_add_func("reset", TypeVoid, mf(&HuiTimer::reset));
		class_add_func("peek", TypeFloat32, mf(&HuiTimer::peek));


	add_class(TypeHuiConfiguration);
		class_add_func("setInt",								TypeVoid,	mf(&HuiConfiguration::getInt));
			func_add_param("name",		TypeString);
			func_add_param("value",		TypeInt);
		class_add_func("setFloat",								TypeVoid,	mf(&HuiConfiguration::getFloat));
			func_add_param("name",		TypeString);
			func_add_param("value",		TypeFloat32);
		class_add_func("setBool",								TypeVoid,	mf(&HuiConfiguration::getBool));
			func_add_param("name",		TypeString);
			func_add_param("value",		TypeBool);
		class_add_func("setStr",								TypeVoid,	mf(&HuiConfiguration::getStr));
			func_add_param("name",		TypeString);
			func_add_param("value",		TypeString);
		class_add_func("getInt",								TypeInt,	mf(&HuiConfiguration::setInt));
			func_add_param("name",		TypeString);
			func_add_param("default",	TypeInt);
		class_add_func("getFloat",								TypeFloat32,	mf(&HuiConfiguration::setFloat));
			func_add_param("name",		TypeString);
			func_add_param("default",	TypeFloat32);
		class_add_func("getBool",								TypeBool,	mf(&HuiConfiguration::setBool));
			func_add_param("name",		TypeString);
			func_add_param("default",	TypeBool);
		class_add_func("getStr",								TypeString,	mf(&HuiConfiguration::setStr));
			func_add_param("name",		TypeString);
			func_add_param("default",	TypeString);
	
	// user interface
	add_func("HuiSetIdleFunction",	TypeVoid,		(void*)HuiSetIdleFunction);
		func_add_param("idle_func",	TypePointer);
	add_func("HuiAddKeyCode",	TypeVoid,		(void*)HuiAddKeyCode);
		func_add_param("id",	TypeString);
		func_add_param("key_code",	TypeInt);
	add_func("HuiAddCommand",	TypeVoid,		(void*)HuiAddCommand);
		func_add_param("id",	TypeString);
		func_add_param("image",	TypeString);
		func_add_param("key_code",	TypeInt);
		func_add_param("func",	TypePointer);
	add_func("HuiGetEvent",	TypeHuiEventP,		(void*)HuiGetEvent);
	add_func("HuiRun",				TypeVoid,		(void*)&HuiRun);
	add_func("HuiEnd",				TypeVoid,		(void*)&HuiEnd);
	add_func("HuiDoSingleMainLoop",	TypeVoid,	(void*)&HuiDoSingleMainLoop);
	add_func("HuiSleep",			TypeVoid,	(void*)&HuiSleep);
		func_add_param("duration",		TypeFloat32);
	add_func("HuiFileDialogOpen",	TypeBool,	(void*)&HuiFileDialogOpen);
		func_add_param("root",		TypeHuiWindowP);
		func_add_param("title",		TypeString);
		func_add_param("dir",		TypeString);
		func_add_param("show_filter",	TypeString);
		func_add_param("filter",	TypeString);
	add_func("HuiFileDialogSave",	TypeBool,	(void*)&HuiFileDialogSave);
		func_add_param("root",		TypeHuiWindowP);
		func_add_param("title",		TypeString);
		func_add_param("dir",		TypeString);
		func_add_param("show_filter",	TypeString);
		func_add_param("filter",	TypeString);
	add_func("HuiFileDialogDir",	TypeBool,	(void*)&HuiFileDialogDir);
		func_add_param("root",		TypeHuiWindowP);
		func_add_param("title",		TypeString);
		func_add_param("dir",		TypeString);
	add_func("HuiQuestionBox",		TypeString,	(void*)&HuiQuestionBox);
		func_add_param("root",		TypeHuiWindowP);
		func_add_param("title",		TypeString);
		func_add_param("text",		TypeString);
		func_add_param("allow_cancel",	TypeBool);
	add_func("HuiInfoBox",			TypeVoid,			(void*)&HuiInfoBox);
		func_add_param("root",		TypeHuiWindowP);
		func_add_param("title",		TypeString);
		func_add_param("text",		TypeString);
	add_func("HuiErrorBox",			TypeVoid,		(void*)&HuiErrorBox);
		func_add_param("root",		TypeHuiWindowP);
		func_add_param("title",		TypeString);
		func_add_param("text",		TypeString);

	// clipboard
	add_func("HuiCopyToClipboard",	TypeVoid,			(void*)&HuiCopyToClipBoard);
		func_add_param("buffer",	TypeString);
	add_func("HuiPasteFromClipboard",	TypeString,		(void*)&HuiPasteFromClipBoard);
	add_func("HuiOpenDocument",		TypeVoid,			(void*)&HuiOpenDocument);
		func_add_param("filename",	TypeString);
	add_func("HuiSetImage",			TypeString,			(void*)&HuiSetImage);
		func_add_param("image",		TypeImage);

	add_class(TypeHuiEvent);
		class_add_element("id",			TypeString,	GetDAEvent(id));
		class_add_element("message",	TypeString,	GetDAEvent(message));
		class_add_element("mouse_x",	TypeFloat32,	GetDAEvent(mx));
		class_add_element("mouse_y",	TypeFloat32,	GetDAEvent(my));
		class_add_element("wheel",		TypeFloat32,	GetDAEvent(dz));
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
	add_const("KeyControl",TypeInt,(void*)KEY_CONTROL);
	add_const("KeyControlL",TypeInt,(void*)KEY_LCONTROL);
	add_const("KeyControlR",TypeInt,(void*)KEY_RCONTROL);
	add_const("KeyShift",TypeInt,(void*)KEY_SHIFT);
	add_const("KeyShiftL",TypeInt,(void*)KEY_LSHIFT);
	add_const("KeyShiftR",TypeInt,(void*)KEY_RSHIFT);
	add_const("KeyAlt",TypeInt,(void*)KEY_ALT);
	add_const("KeyAltL",TypeInt,(void*)KEY_LALT);
	add_const("KeyAltR",TypeInt,(void*)KEY_RALT);
	add_const("KeyPlus",TypeInt,(void*)KEY_ADD);
	add_const("KeyMinus",TypeInt,(void*)KEY_SUBTRACT);
	add_const("KeyFence",TypeInt,(void*)KEY_FENCE);
	add_const("KeyEnd",TypeInt,(void*)KEY_END);
	add_const("KeyNext",TypeInt,(void*)KEY_NEXT);
	add_const("KeyPrior",TypeInt,(void*)KEY_PRIOR);
	add_const("KeyUp",TypeInt,(void*)KEY_UP);
	add_const("KeyDown",TypeInt,(void*)KEY_DOWN);
	add_const("KeyLeft",TypeInt,(void*)KEY_LEFT);
	add_const("KeyRight",TypeInt,(void*)KEY_RIGHT);
	add_const("KeyReturn",TypeInt,(void*)KEY_RETURN);
	add_const("KeyEscape",TypeInt,(void*)KEY_ESCAPE);
	add_const("KeyInsert",TypeInt,(void*)KEY_INSERT);
	add_const("KeyDelete",TypeInt,(void*)KEY_DELETE);
	add_const("KeySpace",TypeInt,(void*)KEY_SPACE);
	add_const("KeyF1",TypeInt,(void*)KEY_F1);
	add_const("KeyF2",TypeInt,(void*)KEY_F2);
	add_const("KeyF3",TypeInt,(void*)KEY_F3);
	add_const("KeyF4",TypeInt,(void*)KEY_F4);
	add_const("KeyF5",TypeInt,(void*)KEY_F5);
	add_const("KeyF6",TypeInt,(void*)KEY_F6);
	add_const("KeyF7",TypeInt,(void*)KEY_F7);
	add_const("KeyF8",TypeInt,(void*)KEY_F8);
	add_const("KeyF9",TypeInt,(void*)KEY_F9);
	add_const("KeyF10",TypeInt,(void*)KEY_F10);
	add_const("KeyF11",TypeInt,(void*)KEY_F11);
	add_const("KeyF12",TypeInt,(void*)KEY_F12);
	add_const("Key0",TypeInt,(void*)KEY_0);
	add_const("Key1",TypeInt,(void*)KEY_1);
	add_const("Key2",TypeInt,(void*)KEY_2);
	add_const("Key3",TypeInt,(void*)KEY_3);
	add_const("Key4",TypeInt,(void*)KEY_4);
	add_const("Key5",TypeInt,(void*)KEY_5);
	add_const("Key6",TypeInt,(void*)KEY_6);
	add_const("Key7",TypeInt,(void*)KEY_7);
	add_const("Key8",TypeInt,(void*)KEY_8);
	add_const("Key9",TypeInt,(void*)KEY_9);
	add_const("KeyA",TypeInt,(void*)KEY_A);
	add_const("KeyB",TypeInt,(void*)KEY_B);
	add_const("KeyC",TypeInt,(void*)KEY_C);
	add_const("KeyD",TypeInt,(void*)KEY_D);
	add_const("KeyE",TypeInt,(void*)KEY_E);
	add_const("KeyF",TypeInt,(void*)KEY_F);
	add_const("KeyG",TypeInt,(void*)KEY_G);
	add_const("KeyH",TypeInt,(void*)KEY_H);
	add_const("KeyI",TypeInt,(void*)KEY_I);
	add_const("KeyJ",TypeInt,(void*)KEY_J);
	add_const("KeyK",TypeInt,(void*)KEY_K);
	add_const("KeyL",TypeInt,(void*)KEY_L);
	add_const("KeyM",TypeInt,(void*)KEY_M);
	add_const("KeyN",TypeInt,(void*)KEY_N);
	add_const("KeyO",TypeInt,(void*)KEY_O);
	add_const("KeyP",TypeInt,(void*)KEY_P);
	add_const("KeyQ",TypeInt,(void*)KEY_Q);
	add_const("KeyR",TypeInt,(void*)KEY_R);
	add_const("KeyS",TypeInt,(void*)KEY_S);
	add_const("KeyT",TypeInt,(void*)KEY_T);
	add_const("KeyU",TypeInt,(void*)KEY_U);
	add_const("KeyV",TypeInt,(void*)KEY_V);
	add_const("KeyW",TypeInt,(void*)KEY_W);
	add_const("KeyX",TypeInt,(void*)KEY_X);
	add_const("KeyY",TypeInt,(void*)KEY_Y);
	add_const("KeyZ",TypeInt,(void*)KEY_Z);
	add_const("KeyBackspace",TypeInt,(void*)KEY_BACKSPACE);
	add_const("KeyTab",TypeInt,(void*)KEY_TAB);
	add_const("KeyHome",TypeInt,(void*)KEY_HOME);
	add_const("KeyNum0",TypeInt,(void*)KEY_NUM_0);
	add_const("KeyNum1",TypeInt,(void*)KEY_NUM_1);
	add_const("KeyNum2",TypeInt,(void*)KEY_NUM_2);
	add_const("KeyNum3",TypeInt,(void*)KEY_NUM_3);
	add_const("KeyNum4",TypeInt,(void*)KEY_NUM_4);
	add_const("KeyNum5",TypeInt,(void*)KEY_NUM_5);
	add_const("KeyNum6",TypeInt,(void*)KEY_NUM_6);
	add_const("KeyNum7",TypeInt,(void*)KEY_NUM_7);
	add_const("KeyNum8",TypeInt,(void*)KEY_NUM_8);
	add_const("KeyNum9",TypeInt,(void*)KEY_NUM_9);
	add_const("KeyNumPlus",TypeInt,(void*)KEY_NUM_ADD);
	add_const("KeyNumMinus",TypeInt,(void*)KEY_NUM_SUBTRACT);
	add_const("KeyNumMultiply",TypeInt,(void*)KEY_NUM_MULTIPLY);
	add_const("KeyNumDivide",TypeInt,(void*)KEY_NUM_DIVIDE);
	add_const("KeyNumComma",TypeInt,(void*)KEY_NUM_COMMA);
	add_const("KeyNumEnter",TypeInt,(void*)KEY_NUM_ENTER);
	add_const("KeyComma",TypeInt,(void*)KEY_COMMA);
	add_const("KeyDot",TypeInt,(void*)KEY_DOT);
	add_const("KeySmaller",TypeInt,(void*)KEY_SMALLER);
	add_const("KeySz",TypeInt,(void*)KEY_SZ);
	add_const("KeyAe",TypeInt,(void*)KEY_AE);
	add_const("KeyOe",TypeInt,(void*)KEY_OE);
	add_const("KeyUe",TypeInt,(void*)KEY_UE);
	add_const("NumKeys",TypeInt,(void*)HUI_NUM_KEYS);
	add_const("KeyAny",TypeInt,(void*)KEY_ANY);

	add_ext_var("AppFilename",		TypeString,		hui_p(&HuiAppFilename));
	add_ext_var("AppDirectory",		TypeString,		hui_p(&HuiAppDirectory));
	add_ext_var("AppDirectoryStatic",TypeString,		hui_p(&HuiAppDirectoryStatic));
	add_ext_var("HuiFilename",		TypeString,		hui_p(&HuiFilename));
	//add_ext_var("HuiRunning",		TypeBool,		hui_p(&HuiRunning));
	add_ext_var("HuiConfig",		TypeHuiConfiguration,	hui_p(&HuiConfig));
}

};
