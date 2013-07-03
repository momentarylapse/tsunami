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
extern Type *TypeFloatList;
extern Type *TypeImage;
Type *TypeHuiWindowP;

void SIAddPackageHui()
{
	msg_db_f("SIAddPackageHui", 3);

	add_package("hui", false);
	
	Type*
	TypeHuiMenu		= add_type  ("Menu",	0);
	Type*
	TypeHuiMenuP	= add_type_p("menu",		TypeHuiMenu);
	Type*
	TypeHuiWindow	= add_type  ("Window", sizeof(HuiWindow));
	TypeHuiWindowP	= add_type_p("window",	TypeHuiWindow);
	Type*
	TypeHuiNixWindow= add_type  ("NixWindow", sizeof(HuiWindow));
	Type*
	TypeHuiDialog	= add_type  ("Dialog", sizeof(HuiWindow));
	Type*
	TypeHuiFixedDialog= add_type  ("FixedDialog", sizeof(HuiWindow));
	Type*
	TypeHuiEvent	= add_type  ("Event",		0);
	Type*
	TypeHuiEventP	= add_type_p("event",	TypeHuiEvent);
	Type*
	TypeHuiPainter	= add_type  ("Painter",	0);
	Type*
	TypeHuiPainterP	= add_type_p("painter",	TypeHuiPainter);
	Type*
	TypeHuiTimer	= add_type  ("Timer",	0);

	
	add_func("GetKeyName",									TypeString,	hui_p(&HuiGetKeyName));
		func_add_param("id",		TypeInt);

	add_class(TypeHuiMenu);
		class_add_func("OpenPopup",	TypeVoid,		mf((tmf)&HuiMenu::OpenPopup));
			func_add_param("w",			TypeHuiWindowP);
			func_add_param("x",			TypeInt);
			func_add_param("y",			TypeInt);
		class_add_func("AddItem",		TypeVoid,		mf((tmf)&HuiMenu::AddItem));
			func_add_param("name",		TypeString);
			func_add_param("id",		TypeInt);
		class_add_func("AddItemImage",	TypeVoid,		mf((tmf)&HuiMenu::AddItemImage));
			func_add_param("name",		TypeString);
			func_add_param("image",		TypeInt);
			func_add_param("id",		TypeInt);
		class_add_func("AddItemCheckable",	TypeVoid,		mf((tmf)&HuiMenu::AddItemCheckable));
			func_add_param("name",		TypeString);
			func_add_param("id",		TypeInt);
		class_add_func("AddSeparator",	TypeVoid,		mf((tmf)&HuiMenu::AddSeparator));
		class_add_func("AddSubMenu",	TypeVoid,		mf((tmf)&HuiMenu::AddSubMenu));
			func_add_param("name",		TypeString);
			func_add_param("id",		TypeInt);
			func_add_param("sub_menu",	TypeHuiMenuP);
	
	add_class(TypeHuiWindow);
		class_set_vtable(HuiWindow);
		TypeHuiWindow->vtable = new VirtualTable[30];
		class_add_func("__init__",		TypeVoid,		mf((tmf)&HuiWindow::__init_ext__));
			func_add_param("title",		TypeString);
			func_add_param("x",		TypeInt);
			func_add_param("y",		TypeInt);
			func_add_param("width",		TypeInt);
			func_add_param("height",		TypeInt);
		class_add_func("__delete__",		TypeVoid,		mf((tmf)&HuiWindow::__delete__));
		class_add_func("Run",			TypeString,		mf((tmf)&HuiWindow::Run));
		class_add_func("Show",			TypeVoid,		mf((tmf)&HuiWindow::Show));
		class_add_func("Hide",			TypeVoid,		mf((tmf)&HuiWindow::Hide));
		class_add_func("SetMenu",			TypeVoid,		mf((tmf)&HuiWindow::SetMenu));
			func_add_param("menu",		TypeHuiMenuP);
		class_add_func("SetBorderWidth",			TypeVoid,		mf((tmf)&HuiWindow::SetBorderWidth));
			func_add_param("width",		TypeInt);
		class_add_func("SetDecimals",			TypeVoid,		mf((tmf)&HuiWindow::SetDecimals));
			func_add_param("decimals",		TypeInt);
		class_add_func("SetMaximized",		TypeVoid,		mf((tmf)&HuiWindow::SetMaximized));
			func_add_param("max",		TypeBool);
		class_add_func("IsMaximized",		TypeBool,		mf((tmf)&HuiWindow::IsMaximized));
		class_add_func("IsMinimized",		TypeBool,		mf((tmf)&HuiWindow::IsMinimized));
		class_add_func("SetID",			TypeVoid,		mf((tmf)&HuiWindow::SetID));
			func_add_param("id",		TypeInt);
		class_add_func("SetFullscreen",				TypeVoid,		mf((tmf)&HuiWindow::SetFullscreen));
			func_add_param("fullscreen",TypeBool);
		class_add_func("SetTitle",										TypeVoid,		mf((tmf)&HuiWindow::SetTitle));
			func_add_param("title",		TypeString);
		class_add_func("SetPosition",								TypeVoid,		mf((tmf)&HuiWindow::SetPosition));
			func_add_param("x",			TypeInt);
			func_add_param("y",			TypeInt);
	//add_func("SetOuterior",								TypeVoid,		2,	TypePointer,"win",
	//																										TypeIRect,"r");
	//add_func("GetOuterior",								TypeIRect,		1,	TypePointer,"win");
	//add_func("SetInerior",								TypeVoid,		2,	TypePointer,"win",
	//																										TypeIRect,"r");
	//add_func("GetInterior",									TypeIRect,		1,	TypePointer,"win");
		class_add_func("SetCursorPos",								TypeVoid,		mf((tmf)&HuiWindow::SetCursorPos));
			func_add_param("x",			TypeInt);
			func_add_param("y",			TypeInt);
		class_add_func("Activate",										TypeVoid,		mf((tmf)&HuiWindow::Activate));
			func_add_param("id",		TypeInt);
		class_add_func("IsActive",										TypeVoid,		mf((tmf)&HuiWindow::IsActive));
			func_add_param("recursive",	TypeBool);
		class_add_func("AddButton",										TypeVoid,		mf((tmf)&HuiWindow::AddButton));
			func_add_param("title",		TypeString);
			func_add_param("x",			TypeInt);
			func_add_param("y",			TypeInt);
			func_add_param("width",		TypeInt);
			func_add_param("height",	TypeInt);
			func_add_param("id",		TypeString);
		class_add_func("AddDefButton",										TypeVoid,		mf((tmf)&HuiWindow::AddDefButton));
			func_add_param("title",		TypeString);
			func_add_param("x",			TypeInt);
			func_add_param("y",			TypeInt);
			func_add_param("width",		TypeInt);
			func_add_param("height",	TypeInt);
			func_add_param("id",		TypeString);
		class_add_func("AddCheckBox",								TypeVoid,		mf((tmf)&HuiWindow::AddCheckBox));
			func_add_param("title",		TypeString);
			func_add_param("x",			TypeInt);
			func_add_param("y",			TypeInt);
			func_add_param("width",		TypeInt);
			func_add_param("height",	TypeInt);
			func_add_param("id",		TypeString);
		class_add_func("AddText",										TypeVoid,		mf((tmf)&HuiWindow::AddText));
			func_add_param("title",		TypeString);
			func_add_param("x",			TypeInt);
			func_add_param("y",			TypeInt);
			func_add_param("width",		TypeInt);
			func_add_param("height",	TypeInt);
			func_add_param("id",		TypeString);
		class_add_func("AddEdit",										TypeVoid,		mf((tmf)&HuiWindow::AddEdit));
			func_add_param("title",		TypeString);
			func_add_param("x",			TypeInt);
			func_add_param("y",			TypeInt);
			func_add_param("width",		TypeInt);
			func_add_param("height",	TypeInt);
			func_add_param("id",		TypeString);
		class_add_func("AddMultilineEdit",										TypeVoid,		mf((tmf)&HuiWindow::AddMultilineEdit));
			func_add_param("title",		TypeString);
			func_add_param("x",			TypeInt);
			func_add_param("y",			TypeInt);
			func_add_param("width",		TypeInt);
			func_add_param("height",	TypeInt);
			func_add_param("id",		TypeString);
		class_add_func("AddGroup",										TypeVoid,		mf((tmf)&HuiWindow::AddGroup));
			func_add_param("title",		TypeString);
			func_add_param("x",			TypeInt);
			func_add_param("y",			TypeInt);
			func_add_param("width",		TypeInt);
			func_add_param("height",	TypeInt);
			func_add_param("id",		TypeString);
		class_add_func("AddComboBox",								TypeVoid,		mf((tmf)&HuiWindow::AddComboBox));
			func_add_param("title",		TypeString);
			func_add_param("x",			TypeInt);
			func_add_param("y",			TypeInt);
			func_add_param("width",		TypeInt);
			func_add_param("height",	TypeInt);
			func_add_param("id",		TypeString);
		class_add_func("AddTabControl",								TypeVoid,		mf((tmf)&HuiWindow::AddTabControl));
			func_add_param("title",		TypeString);
			func_add_param("x",			TypeInt);
			func_add_param("y",			TypeInt);
			func_add_param("width",		TypeInt);
			func_add_param("height",	TypeInt);
			func_add_param("id",		TypeString);
		class_add_func("SetTarget",				TypeVoid,		mf((tmf)&HuiWindow::SetTarget));
			func_add_param("id",		TypeString);
			func_add_param("page",		TypeInt);
		class_add_func("AddListView",								TypeVoid,		mf((tmf)&HuiWindow::AddListView));
			func_add_param("title",		TypeString);
			func_add_param("x",			TypeInt);
			func_add_param("y",			TypeInt);
			func_add_param("width",		TypeInt);
			func_add_param("height",	TypeInt);
			func_add_param("id",		TypeString);
		class_add_func("AddTreeView",								TypeVoid,		mf((tmf)&HuiWindow::AddTreeView));
			func_add_param("title",		TypeString);
			func_add_param("x",			TypeInt);
			func_add_param("y",			TypeInt);
			func_add_param("width",		TypeInt);
			func_add_param("height",	TypeInt);
			func_add_param("id",		TypeString);
		class_add_func("AddIconView",								TypeVoid,		mf((tmf)&HuiWindow::AddIconView));
			func_add_param("title",		TypeString);
			func_add_param("x",			TypeInt);
			func_add_param("y",			TypeInt);
			func_add_param("width",		TypeInt);
			func_add_param("height",	TypeInt);
			func_add_param("id",		TypeString);
		class_add_func("AddProgressBar",						TypeVoid,		mf((tmf)&HuiWindow::AddProgressBar));
			func_add_param("title",		TypeString);
			func_add_param("x",			TypeInt);
			func_add_param("y",			TypeInt);
			func_add_param("width",		TypeInt);
			func_add_param("height",	TypeInt);
			func_add_param("id",		TypeString);
		class_add_func("AddSlider",										TypeVoid,		mf((tmf)&HuiWindow::AddSlider));
			func_add_param("title",		TypeString);
			func_add_param("x",			TypeInt);
			func_add_param("y",			TypeInt);
			func_add_param("width",		TypeInt);
			func_add_param("height",	TypeInt);
			func_add_param("id",		TypeString);
		class_add_func("AddImage",										TypeVoid,		mf((tmf)&HuiWindow::AddImage));
			func_add_param("title",		TypeString);
			func_add_param("x",			TypeInt);
			func_add_param("y",			TypeInt);
			func_add_param("width",		TypeInt);
			func_add_param("height",	TypeInt);
			func_add_param("id",		TypeString);
		class_add_func("AddDrawingArea",										TypeVoid,		mf((tmf)&HuiWindow::AddDrawingArea));
			func_add_param("title",		TypeString);
			func_add_param("x",			TypeInt);
			func_add_param("y",			TypeInt);
			func_add_param("width",		TypeInt);
			func_add_param("height",	TypeInt);
			func_add_param("id",		TypeString);
		class_add_func("AddControlTable",										TypeVoid,		mf((tmf)&HuiWindow::AddControlTable));
			func_add_param("title",		TypeString);
			func_add_param("x",			TypeInt);
			func_add_param("y",			TypeInt);
			func_add_param("width",		TypeInt);
			func_add_param("height",	TypeInt);
			func_add_param("id",		TypeString);
		class_add_func("AddSpinButton",										TypeVoid,		mf((tmf)&HuiWindow::AddSpinButton));
			func_add_param("title",		TypeString);
			func_add_param("x",			TypeInt);
			func_add_param("y",			TypeInt);
			func_add_param("width",		TypeInt);
			func_add_param("height",	TypeInt);
			func_add_param("id",		TypeString);
		class_add_func("SetString",						TypeVoid,		mf((tmf)&HuiWindow::SetString));
			func_add_param("id",		TypeString);
			func_add_param("s",			TypeString);
		class_add_func("GetString",						TypeString,		mf((tmf)&HuiWindow::GetString));
			func_add_param("id",		TypeString);
		class_add_func("SetFloat",						TypeVoid,		mf((tmf)&HuiWindow::SetFloat));
			func_add_param("id",		TypeString);
			func_add_param("f",			TypeFloat);
		class_add_func("GetFloat",						TypeFloat,		mf((tmf)&HuiWindow::GetFloat));
			func_add_param("id",		TypeString);
		class_add_func("Enable",								TypeVoid,		mf((tmf)&HuiWindow::Enable));
			func_add_param("id",		TypeString);
			func_add_param("enabled",	TypeBool);
		class_add_func("IsEnabled",					TypeBool,		mf((tmf)&HuiWindow::IsEnabled));
			func_add_param("id",		TypeString);
		class_add_func("Check",								TypeVoid,		mf((tmf)&HuiWindow::Check));
			func_add_param("id",		TypeString);
			func_add_param("checked",	TypeBool);
		class_add_func("IsChecked",					TypeBool,		mf((tmf)&HuiWindow::IsChecked));
			func_add_param("id",		TypeString);
		class_add_func("GetInt",			TypeInt,		mf((tmf)&HuiWindow::GetInt));
			func_add_param("id",		TypeString);
		class_add_func("GetMultiSelection",			TypeIntList,		mf((tmf)&HuiWindow::GetMultiSelection));
			func_add_param("id",		TypeString);
		class_add_func("SetInt",			TypeVoid,		mf((tmf)&HuiWindow::SetInt));
			func_add_param("id",		TypeString);
			func_add_param("i",			TypeInt);
		class_add_func("SetImage",			TypeVoid,		mf((tmf)&HuiWindow::SetImage));
			func_add_param("id",		TypeString);
			func_add_param("image",		TypeString);
		class_add_func("GetCell",						TypeString,		mf((tmf)&HuiWindow::GetCell));
			func_add_param("id",		TypeString);
			func_add_param("row",		TypeInt);
			func_add_param("column",	TypeInt);
		class_add_func("SetCell",						TypeVoid,		mf((tmf)&HuiWindow::SetCell));
			func_add_param("id",		TypeString);
			func_add_param("row",		TypeInt);
			func_add_param("column",	TypeInt);
			func_add_param("s",			TypeString);
		class_add_func("CompletionAdd",			TypeVoid,		mf((tmf)&HuiWindow::CompletionAdd));
			func_add_param("id",		TypeString);
			func_add_param("text",		TypeString);
		class_add_func("CompletionClear",			TypeVoid,		mf((tmf)&HuiWindow::CompletionClear));
			func_add_param("id",		TypeString);
		class_add_func("Reset",								TypeVoid,		mf((tmf)&HuiWindow::Reset));
			func_add_param("id",		TypeString);
		class_add_func("Redraw",								TypeVoid,		mf((tmf)&HuiWindow::Redraw));
			func_add_param("id",		TypeString);
		class_add_func("GetMouse",								TypeBool,		mf((tmf)&HuiWindow::GetMouse));
			func_add_param("x",			TypeIntPs);
			func_add_param("y",			TypeIntPs);
			func_add_param("button",	TypeInt);
			func_add_param("change",	TypeInt);
		class_add_func("GetKey",							TypeBool,		mf((tmf)&HuiWindow::GetKey));
			func_add_param("key",			TypeInt);
		class_add_func("Event",						TypeVoid,		mf((tmf)&HuiWindow::Event));
			func_add_param("id",			TypeString);
			func_add_param("func",			TypePointer);
		class_add_func("EventX",						TypeVoid,		mf((tmf)&HuiWindow::EventX));
			func_add_param("id",			TypeString);
			func_add_param("msg",			TypeString);
			func_add_param("func",			TypePointer);
		class_add_func("OnMouseMove", TypeVoid, mf((tmf)&HuiWindow::OnMouseMove));
		class_add_func("OnMouseWheel", TypeVoid, mf((tmf)&HuiWindow::OnMouseWheel));
		class_add_func("OnLeftButtonDown", TypeVoid, mf((tmf)&HuiWindow::OnLeftButtonDown));
		class_add_func("OnMiddleButtonDown", TypeVoid, mf((tmf)&HuiWindow::OnMiddleButtonDown));
		class_add_func("OnRightButtonDown", TypeVoid, mf((tmf)&HuiWindow::OnRightButtonDown));
		class_add_func("OnLeftButtonUp", TypeVoid, mf((tmf)&HuiWindow::OnLeftButtonUp));
		class_add_func("OnMiddleButtonUp", TypeVoid, mf((tmf)&HuiWindow::OnMiddleButtonUp));
		class_add_func("OnRightButtonUp", TypeVoid, mf((tmf)&HuiWindow::OnRightButtonUp));
		class_add_func("OnDoubleClick", TypeVoid, mf((tmf)&HuiWindow::OnDoubleClick));
		class_add_func("CanClose", TypeBool, mf((tmf)&HuiWindow::CanClose));
		class_add_func("BeginDraw",								TypeHuiPainterP,		mf((tmf)&HuiWindow::BeginDraw));
			func_add_param("id",		TypeString);
		TypeHuiWindow->LinkVirtualTable();

	add_class(TypeHuiNixWindow);
		TypeHuiNixWindow->DeriveFrom(TypeHuiWindow);
		TypeHuiNixWindow->vtable = TypeHuiWindow->vtable;
		class_add_func("__init__",		TypeVoid,		mf((tmf)&HuiNixWindow::__init_ext__));
			func_add_param("title",		TypeString);
			func_add_param("x",		TypeInt);
			func_add_param("y",		TypeInt);
			func_add_param("width",		TypeInt);
			func_add_param("height",		TypeInt);

	add_class(TypeHuiDialog);
		TypeHuiDialog->DeriveFrom(TypeHuiWindow);
		TypeHuiDialog->vtable = TypeHuiWindow->vtable;
		class_add_func("__init__",		TypeVoid,		mf((tmf)&HuiDialog::__init_ext__));
			func_add_param("title",		TypeString);
			func_add_param("width",		TypeInt);
			func_add_param("height",		TypeInt);
			func_add_param("root",		TypeHuiWindowP);
			func_add_param("allow_root",TypeBool);

	add_class(TypeHuiFixedDialog);
		TypeHuiFixedDialog->DeriveFrom(TypeHuiWindow);
		TypeHuiFixedDialog->vtable = TypeHuiWindow->vtable;
		class_add_func("__init__",		TypeVoid,		mf((tmf)&HuiFixedDialog::__init_ext__));
			func_add_param("title",		TypeString);
			func_add_param("width",		TypeInt);
			func_add_param("height",		TypeInt);
			func_add_param("root",		TypeHuiWindowP);
			func_add_param("allow_root",TypeBool);
	
	add_class(TypeHuiPainter);
		class_add_element("width",		TypeInt,	GetDAPainter(width));
		class_add_element("height",		TypeInt,	GetDAPainter(height));
		class_add_func("End",								TypeVoid,		mf((tmf)&HuiPainter::End));
		class_add_func("SetColor",								TypeVoid,		mf((tmf)&HuiPainter::SetColor));
			func_add_param("c",			TypeColor);
		class_add_func("SetLineWidth",								TypeVoid,		mf((tmf)&HuiPainter::SetLineWidth));
			func_add_param("w",			TypeFloat);
		class_add_func("SetAntialiasing",								TypeVoid,		mf((tmf)&HuiPainter::SetAntialiasing));
			func_add_param("enabled",			TypeBool);
		class_add_func("SetFontSize",								TypeVoid,		mf((tmf)&HuiPainter::SetFontSize));
			func_add_param("size",			TypeFloat);
		class_add_func("DrawPoint",								TypeVoid,		mf((tmf)&HuiPainter::DrawPoint));
			func_add_param("x",			TypeFloat);
			func_add_param("y",			TypeFloat);
		class_add_func("DrawLine",								TypeVoid,		mf((tmf)&HuiPainter::DrawLine));
			func_add_param("x1",		TypeFloat);
			func_add_param("y1",		TypeFloat);
			func_add_param("x2",		TypeFloat);
			func_add_param("y2",		TypeFloat);
		class_add_func("DrawLines",								TypeVoid,		mf((tmf)&HuiPainter::DrawLinesMA));
			func_add_param("x",			TypeFloatList);
			func_add_param("y",			TypeFloatList);
		class_add_func("DrawPolygon",								TypeVoid,		mf((tmf)&HuiPainter::DrawPolygonMA));
			func_add_param("x",			TypeFloatList);
			func_add_param("y",			TypeFloatList);
		class_add_func("DrawRect",								TypeVoid,		mf((tmf)(void (HuiPainter::*) (float,float,float,float))&HuiPainter::DrawRect));
			func_add_param("x",			TypeFloat);
			func_add_param("y",			TypeFloat);
			func_add_param("w",			TypeFloat);
			func_add_param("h",			TypeFloat);
		class_add_func("DrawCircle",								TypeVoid,		mf((tmf)&HuiPainter::DrawCircle));
			func_add_param("x",			TypeFloat);
			func_add_param("y",			TypeFloat);
			func_add_param("r",			TypeFloat);
		class_add_func("DrawStr",								TypeVoid,		mf((tmf)&HuiPainter::DrawStr));
			func_add_param("x",			TypeFloat);
			func_add_param("y",			TypeFloat);
			func_add_param("str",		TypeString);
		class_add_func("DrawImage",								TypeVoid,		mf((tmf)&HuiPainter::DrawImage));
			func_add_param("x",			TypeFloat);
			func_add_param("y",			TypeFloat);
			func_add_param("image",		TypeImage);


	add_class(TypeHuiTimer);
		class_add_func("__init__", TypeVoid, mf((tmf)&HuiTimer::reset));
		class_add_func("get", TypeFloat, mf((tmf)&HuiTimer::get));
		class_add_func("reset", TypeVoid, mf((tmf)&HuiTimer::reset));
		class_add_func("peek", TypeFloat, mf((tmf)&HuiTimer::peek));
	
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
		func_add_param("duration",		TypeFloat);
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
	add_func("HuiConfigWriteInt",								TypeVoid,	(void*)&HuiConfigWriteInt);
		func_add_param("name",		TypeString);
		func_add_param("value",		TypeInt);
	add_func("HuiConfigWriteFloat",								TypeVoid,	(void*)&HuiConfigWriteFloat);
		func_add_param("name",		TypeString);
		func_add_param("value",		TypeFloat);
	add_func("HuiConfigWriteBool",								TypeVoid,	(void*)&HuiConfigWriteBool);
		func_add_param("name",		TypeString);
		func_add_param("value",		TypeBool);
	add_func("HuiConfigWriteStr",								TypeVoid,	(void*)&HuiConfigWriteStr);
		func_add_param("name",		TypeString);
		func_add_param("value",		TypeString);
	add_func("HuiConfigReadInt",								TypeInt,	(void*)&HuiConfigReadInt);
		func_add_param("name",		TypeString);
		func_add_param("default",	TypeInt);
	add_func("HuiConfigReadFloat",								TypeFloat,	(void*)&HuiConfigReadFloat);
		func_add_param("name",		TypeString);
		func_add_param("default",	TypeFloat);
	add_func("HuiConfigReadBool",								TypeBool,	(void*)&HuiConfigReadBool);
		func_add_param("name",		TypeString);
		func_add_param("default",	TypeBool);
	add_func("HuiConfigReadStr",								TypeString,	(void*)&HuiConfigReadStr);
		func_add_param("name",		TypeString);
		func_add_param("default",	TypeString);

	// clipboard
	add_func("HuiCopyToClipboard",	TypeVoid,			(void*)&HuiCopyToClipBoard);
		func_add_param("buffer",	TypeString);
	add_func("HuiPasteFromClipboard",	TypeString,		(void*)&HuiPasteFromClipBoard);
	add_func("HuiOpenDocument",		TypeVoid,			(void*)&HuiOpenDocument);
		func_add_param("filename",	TypeString);
	add_func("HuiSetImage",			TypeString,			(void*)&HuiSetImage);
		func_add_param("image",		TypeImage);
	// menu
	add_func("HuiCreateMenu",		TypeHuiMenuP,		(void*)&HuiCreateMenu);

	add_class(TypeHuiEvent);
		class_add_element("id",			TypeString,	GetDAEvent(id));
		class_add_element("message",	TypeString,	GetDAEvent(message));
		class_add_element("mouse_x",	TypeInt,	GetDAEvent(mx));
		class_add_element("mouse_y",	TypeInt,	GetDAEvent(my));
		class_add_element("wheel",		TypeInt,	GetDAEvent(dz));
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
	add_ext_var("HuiFilename",		TypeString,		hui_p(&HuiFilename));
	add_ext_var("HuiRunning",		TypeBool,		hui_p(&HuiRunning));
}

};
