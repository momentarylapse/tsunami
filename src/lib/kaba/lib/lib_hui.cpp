#include "../../file/file.h"
#include "../kaba.h"
#include "../../config.h"
#include "common.h"

#ifdef _X_USE_HUI_
	#include "../../hui/hui.h"
#else
	we are re screwed.... TODO: test for _X_USE_HUI_
#endif


namespace hui{
#ifdef _X_USE_HUI_
	Menu *CreateMenuFromSource(const string &source);
#endif
}

namespace kaba {

#ifdef _X_USE_HUI_
	static hui::Event *_event;
	static hui::Panel *_panel;
	static hui::Painter *_painter;
	#define GetDAPanel(x)			int_p(&_panel->x)-int_p(_panel)
	#define GetDAWindow(x)			int_p(&_win->x)-int_p(_win)
	#define GetDAEvent(x)	int_p(&_event->x)-int_p(_event)
	#define GetDAPainter(x)	int_p(&_painter->x)-int_p(_painter)
	void HuiSetIdleFunctionKaba(Function *f) {
		auto *ff = (hui::kaba_callback*)f->address;
		hui::SetIdleFunction(ff);
	}
	int HuiRunLaterKaba(float dt, hui::EventHandler *p, Function *f) {
		auto *ff = (hui::kaba_member_callback*)f->address;
		return hui::RunLater(dt, [ff,p]{ ff(p); });
	}
	int HuiRunRepeatedKaba(float dt, hui::EventHandler *p, Function *f) {
		auto *ff = (hui::kaba_member_callback*)f->address;
		return hui::RunRepeated(dt, [ff,p]{ ff(p); });
	}
	class KabaPanelWrapper : public hui::Panel {
	public:
		void _kaba_event(const string &id, Function *f) {
			_kaba_event_o(id, this, f);
		}
		void _kaba_event_o(const string &id, hui::EventHandler *handler, Function *f) {
			auto *ff = (hui::kaba_member_callback*)f->address;
			event(id, [=]{ ff(handler); });
		}
		void _kaba_event_x(const string &id, const string &msg, Function *f) {
			_kaba_event_ox(id, msg, this, f);
		}
		void _kaba_event_ox(const string &id, const string &msg, hui::EventHandler *handler, Function *f) {
			if (msg == "hui:draw"){
				auto *ff = (hui::kaba_member_callback_p*)f->address;
				event_xp(id, msg, [=](Painter *p){ ff(handler, p); });
			}else{
				auto *ff = (hui::kaba_member_callback*)f->address;
				event_x(id, msg, [=]{ ff(handler); });
			}
		}
	};
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


extern const Class *TypeObject;
extern const Class *TypeIntList;
extern const Class *TypeIntPs;
extern const Class *TypeStringList;
extern const Class *TypeFloatList;
extern const Class *TypeComplexList;
extern const Class *TypeImage;
extern const Class *TypeBasePainter;
extern const Class *TypePath;
//extern const Class *TypeTimer;
const Class *TypeHuiWindowP;

string _hui_config_get(hui::Configuration &c, const string &key) {
	return c.get_str(key, "");
}

void SIAddPackageHui() {
	add_package("hui");
	
	auto TypeHuiMenu = add_type("Menu",  sizeof(hui::Menu));
	auto TypeHuiMenuP = add_type_p(TypeHuiMenu);
	auto TypeHuiToolbar = add_type("Toolbar",  sizeof(hui::Toolbar));
	auto TypeHuiToolbarP = add_type_p(TypeHuiToolbar);
	auto TypeHuiPanel = add_type("Panel", sizeof(hui::Panel));
	//auto TypeHuiPanelP = add_type_p(TypeHuiPanel);
	auto TypeHuiWindow = add_type("Window", sizeof(hui::Window));
	TypeHuiWindowP = add_type_p(TypeHuiWindow);
	auto TypeHuiNixWindow = add_type("NixWindow", sizeof(hui::Window));
	auto TypeHuiDialog = add_type("Dialog", sizeof(hui::Window));
	auto TypeHuiEvent = add_type("Event", sizeof(hui::Event));
	auto TypeHuiEventP = add_type_p(TypeHuiEvent);
	auto TypeHuiPainter = add_type("Painter", sizeof(hui::Painter));
	auto TypeHuiConfiguration = add_type("Configuration", sizeof(hui::Configuration));

	auto TypeCallback = add_type_f(TypeVoid, {TypeObject});
	auto TypeCallbackP = add_type_f(TypeVoid, {TypeObject, TypeHuiPainter});


	add_class(TypeHuiMenu);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, mf(&hui::Menu::__init__));
		class_add_func("popup", TypeVoid, mf(&hui::Menu::open_popup));
			func_add_param("p", TypeHuiPanel);
		class_add_func("add", TypeVoid, mf(&hui::Menu::add));
			func_add_param("name", TypeString);
			func_add_param("id", TypeString);
		class_add_func("add_with_image", TypeVoid, mf(&hui::Menu::add_with_image));
			func_add_param("name", TypeString);
			func_add_param("image", TypeInt);
			func_add_param("id", TypeString);
		class_add_func("add_checkable", TypeVoid, mf(&hui::Menu::add_checkable));
			func_add_param("name", TypeString);
			func_add_param("id", TypeString);
		class_add_func("add_separator", TypeVoid, mf(&hui::Menu::add_separator));
		class_add_func("add_sub_menu", TypeVoid, mf(&hui::Menu::add_sub_menu));
			func_add_param("name", TypeString);
			func_add_param("id", TypeString);
			func_add_param("sub_menu", TypeHuiMenu);
		class_add_func("enable", TypeVoid, mf(&hui::Menu::enable));
			func_add_param("id", TypeString);
			func_add_param("enabled", TypeBool);
		class_add_func("check", TypeVoid, mf(&hui::Menu::check));
			func_add_param("id", TypeString);
			func_add_param("checked", TypeBool);

	add_class(TypeHuiToolbar);
		class_derive_from(TypeObject, false, true);
		class_add_func("set_by_id", TypeVoid, mf(&hui::Toolbar::set_by_id));
			func_add_param("id", TypeString);
		class_add_func("from_source", TypeVoid, mf(&hui::Toolbar::from_source));
			func_add_param("source", TypeString);

	add_class(TypeHuiPanel);
		class_derive_from(TypeObject, false, true);
		class_add_element("win", TypeHuiWindowP, GetDAPanel(win));
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, mf(&hui::Panel::__init__), Flags::OVERRIDE);
		class_add_func_virtual(IDENTIFIER_FUNC_DELETE, TypeVoid, mf(&hui::Panel::__delete__), Flags::OVERRIDE);
		class_add_func("set_border_width", TypeVoid, mf(&hui::Panel::set_border_width));
			func_add_param("width", TypeInt);
		class_add_func("set_decimals", TypeVoid, mf(&hui::Panel::set_decimals));
			func_add_param("decimals", TypeInt);
		class_add_func("activate", TypeVoid, mf(&hui::Panel::activate));
			func_add_param("id", TypeString);
		class_add_func("is_active", TypeVoid, mf(&hui::Panel::is_active));
			func_add_param("id", TypeString);
		class_add_func("from_source", TypeVoid, mf(&hui::Panel::from_source));
			func_add_param("source", TypeString);
		class_add_func("add_button", TypeVoid, mf(&hui::Panel::add_button));
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
			func_add_param("id", TypeString);
		class_add_func("add_def_button", TypeVoid, mf(&hui::Panel::add_def_button));
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
			func_add_param("id", TypeString);
		class_add_func("add_check_box", TypeVoid, mf(&hui::Panel::add_check_box));
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
			func_add_param("id", TypeString);
		class_add_func("add_label", TypeVoid, mf(&hui::Panel::add_label));
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
			func_add_param("id", TypeString);
		class_add_func("add_edit", TypeVoid, mf(&hui::Panel::add_edit));
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
			func_add_param("id", TypeString);
		class_add_func("add_multiline_edit", TypeVoid, mf(&hui::Panel::add_multiline_edit));
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
			func_add_param("id", TypeString);
		class_add_func("add_group", TypeVoid, mf(&hui::Panel::add_group));
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
			func_add_param("id", TypeString);
		class_add_func("add_combo_box", TypeVoid, mf(&hui::Panel::add_combo_box));
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
			func_add_param("id", TypeString);
		class_add_func("add_tab_control", TypeVoid, mf(&hui::Panel::add_tab_control));
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
			func_add_param("id", TypeString);
		class_add_func("set_target", TypeVoid, mf(&hui::Panel::set_target));
			func_add_param("id", TypeString);
		class_add_func("add_list_view", TypeVoid, mf(&hui::Panel::add_list_view));
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
			func_add_param("id", TypeString);
		class_add_func("add_tree_view", TypeVoid, mf(&hui::Panel::add_tree_view));
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
			func_add_param("id", TypeString);
		class_add_func("add_icon_view", TypeVoid, mf(&hui::Panel::add_icon_view));
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
			func_add_param("id", TypeString);
		class_add_func("add_progress_bar", TypeVoid, mf(&hui::Panel::add_progress_bar));
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
			func_add_param("id", TypeString);
		class_add_func("add_slider", TypeVoid, mf(&hui::Panel::add_slider));
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
			func_add_param("id", TypeString);
		class_add_func("add_drawing_area", TypeVoid, mf(&hui::Panel::add_drawing_area));
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
			func_add_param("id", TypeString);
		class_add_func("add_grid", TypeVoid, mf(&hui::Panel::add_grid));
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
			func_add_param("id", TypeString);
		class_add_func("add_spin_button", TypeVoid, mf(&hui::Panel::add_spin_button));
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
			func_add_param("id", TypeString);
		class_add_func("add_radio_button", TypeVoid, mf(&hui::Panel::add_radio_button));
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
			func_add_param("id", TypeString);
		class_add_func("add_scroller", TypeVoid, mf(&hui::Panel::add_scroller));
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
			func_add_param("id", TypeString);
		class_add_func("add_expander", TypeVoid, mf(&hui::Panel::add_expander));
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
			func_add_param("id", TypeString);
		class_add_func("add_separator", TypeVoid, mf(&hui::Panel::add_separator));
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
			func_add_param("id", TypeString);
		class_add_func("add_paned", TypeVoid, mf(&hui::Panel::add_paned));
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
			func_add_param("id", TypeString);
		class_add_func("add_revealer", TypeVoid, mf(&hui::Panel::add_revealer));
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
			func_add_param("id", TypeString);
		class_add_func("embed", TypeVoid, mf(&hui::Panel::embed));
			func_add_param("panel", TypeHuiPanel);
			func_add_param("id", TypeString);
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
		class_add_func("set_string", TypeVoid, mf(&hui::Panel::set_string));
			func_add_param("id", TypeString);
			func_add_param("s", TypeString);
		class_add_func("add_string", TypeVoid, mf(&hui::Panel::add_string));
			func_add_param("id", TypeString);
			func_add_param("s", TypeString);
		class_add_func("get_string", TypeString, mf(&hui::Panel::get_string));
			func_add_param("id", TypeString);
		class_add_func("set_float", TypeVoid, mf(&hui::Panel::set_float));
			func_add_param("id", TypeString);
			func_add_param("f", TypeFloat32);
		class_add_func("get_float", TypeFloat32, mf(&hui::Panel::get_float));
			func_add_param("id", TypeString);
		class_add_func("enable", TypeVoid, mf(&hui::Panel::enable));
			func_add_param("id", TypeString);
			func_add_param("enabled", TypeBool);
		class_add_func("is_enabled", TypeBool, mf(&hui::Panel::is_enabled));
			func_add_param("id", TypeString);
		class_add_func("check", TypeVoid, mf(&hui::Panel::check));
			func_add_param("id", TypeString);
			func_add_param("checked", TypeBool);
		class_add_func("is_checked", TypeBool, mf(&hui::Panel::is_checked));
			func_add_param("id", TypeString);
		class_add_func("hide_control", TypeVoid, mf(&hui::Panel::hide_control));
			func_add_param("id", TypeString);
			func_add_param("hide", TypeBool);
		class_add_func("delete_control", TypeVoid, mf(&hui::Panel::delete_control));
			func_add_param("id", TypeString);
		class_add_func("get_int", TypeInt, mf(&hui::Panel::get_int));
			func_add_param("id", TypeString);
		class_add_func("get_selection", TypeIntList, mf(&hui::Panel::get_selection));
			func_add_param("id", TypeString);
		class_add_func("set_int", TypeVoid, mf(&hui::Panel::set_int));
			func_add_param("id", TypeString);
			func_add_param("i", TypeInt);
		class_add_func("set_image", TypeVoid, mf(&hui::Panel::set_image));
			func_add_param("id", TypeString);
			func_add_param("image", TypeString);
		class_add_func("get_cell", TypeString, mf(&hui::Panel::get_cell));
			func_add_param("id", TypeString);
			func_add_param("row", TypeInt);
			func_add_param("column", TypeInt);
		class_add_func("set_cell", TypeVoid, mf(&hui::Panel::set_cell));
			func_add_param("id", TypeString);
			func_add_param("row", TypeInt);
			func_add_param("column", TypeInt);
			func_add_param("s", TypeString);
		class_add_func("set_options", TypeVoid, mf(&hui::Panel::set_options));
			func_add_param("id", TypeString);
			func_add_param("options", TypeString);
		class_add_func("reset", TypeVoid, mf(&hui::Panel::reset));
			func_add_param("id", TypeString);
		class_add_func("redraw", TypeVoid, mf(&hui::Panel::redraw));
			func_add_param("id", TypeString);
		class_add_func("reveal", TypeVoid, mf(&hui::Panel::reveal));
			func_add_param("id", TypeString);
			func_add_param("reveal", TypeBool);
		class_add_func("is_revealed", TypeBool, mf(&hui::Panel::is_revealed));
			func_add_param("id", TypeString);
		class_add_func("event", TypeInt, mf(&KabaPanelWrapper::_kaba_event));
			func_add_param("id", TypeString);
			func_add_param("func", TypeCallback);
		class_add_func("event_o", TypeInt, mf(&KabaPanelWrapper::_kaba_event_o));
			func_add_param("id", TypeString);
			func_add_param("handler", TypeObject);
			func_add_param("func", TypeCallback);
		class_add_func("event_x", TypeInt, mf(&KabaPanelWrapper::_kaba_event_x));
			func_add_param("id", TypeString);
			func_add_param("msg", TypeString);
			func_add_param("func", TypeCallback);
		class_add_func("event_x", TypeInt, mf(&KabaPanelWrapper::_kaba_event_x));
			func_add_param("id", TypeString);
			func_add_param("msg", TypeString);
			func_add_param("func", TypeCallbackP);
		class_add_func("event_ox", TypeInt, mf(&KabaPanelWrapper::_kaba_event_ox));
			func_add_param("id", TypeString);
			func_add_param("msg", TypeString);
			func_add_param("handler", TypeObject);
			func_add_param("func", TypeCallback);
		class_add_func("event_ox", TypeInt, mf(&KabaPanelWrapper::_kaba_event_ox));
			func_add_param("id", TypeString);
			func_add_param("msg", TypeString);
			func_add_param("handler", TypeObject);
			func_add_param("func", TypeCallbackP);
		class_add_func("remove_event_handler", TypeVoid, mf(&hui::Panel::remove_event_handler));
			func_add_param("uid", TypeInt);
		class_set_vtable(hui::Panel);


	add_class(TypeHuiWindow);
		class_derive_from(TypeHuiPanel, false, true);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, mf(&hui::Window::__init_ext__), Flags::OVERRIDE);
			func_add_param("title", TypeString);
			func_add_param("width", TypeInt);
			func_add_param("height", TypeInt);
		class_add_func_virtual(IDENTIFIER_FUNC_DELETE, TypeVoid, mf(&hui::Window::__delete__), Flags::OVERRIDE);
		class_add_func("run", TypeVoid, mf(&hui::Window::run));
		class_add_func("destroy", TypeVoid, mf(&hui::Window::request_destroy));
		class_add_func("show", TypeVoid, mf(&hui::Window::show));
		class_add_func("hide", TypeVoid, mf(&hui::Window::hide));

		class_add_func("set_menu", TypeVoid, mf(&hui::Window::set_menu));
			func_add_param("menu", TypeHuiMenu);
		class_add_func("toolbar", TypeHuiToolbarP, mf(&hui::Window::get_toolbar), Flags::SELFREF);
			func_add_param("index", TypeInt);
		class_add_func("set_maximized", TypeVoid, mf(&hui::Window::set_maximized));
			func_add_param("max", TypeBool);
		class_add_func("is_maximized", TypeBool, mf(&hui::Window::is_maximized));
		class_add_func("is_minimized", TypeBool, mf(&hui::Window::is_minimized));
		class_add_func("set_id", TypeVoid, mf(&hui::Window::set_id));
			func_add_param("id", TypeInt);
		class_add_func("set_fullscreen", TypeVoid, mf(&hui::Window::set_fullscreen));
			func_add_param("fullscreen",TypeBool);
		class_add_func("set_title", TypeVoid, mf(&hui::Window::set_title));
			func_add_param("title", TypeString);
		class_add_func("set_position", TypeVoid, mf(&hui::Window::set_position));
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
	//add_func("setOuterior", TypeVoid, 2, TypePointer,"win",
	//																										TypeIRect,"r");
	//add_func("getOuterior", TypeIRect, 1, TypePointer,"win");
	//add_func("setInerior", TypeVoid, 2, TypePointer,"win",
	//																										TypeIRect,"r");
	//add_func("getInterior", TypeIRect, 1, TypePointer,"win");
		class_add_func("set_cursor_pos", TypeVoid, mf(&hui::Window::set_cursor_pos));
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
		class_add_func("get_mouse", TypeBool, mf(&hui::Window::get_mouse));
			func_add_param("x", TypeIntPs);
			func_add_param("y", TypeIntPs);
			func_add_param("button", TypeInt);
			func_add_param("change", TypeInt);
		class_add_func("get_key", TypeBool, mf(&hui::Window::get_key));
			func_add_param("key", TypeInt);
		class_add_func_virtual("on_mouse_move", TypeVoid, mf(&hui::Window::on_mouse_move));
		class_add_func_virtual("on_mouse_wheel", TypeVoid, mf(&hui::Window::on_mouse_wheel));
		class_add_func_virtual("on_left_button_down", TypeVoid, mf(&hui::Window::on_left_button_down));
		class_add_func_virtual("on_middle_button_down", TypeVoid, mf(&hui::Window::on_middle_button_down));
		class_add_func_virtual("on_right_button_down", TypeVoid, mf(&hui::Window::on_right_button_down));
		class_add_func_virtual("on_left_button_up", TypeVoid, mf(&hui::Window::on_left_button_up));
		class_add_func_virtual("on_middle_button_up", TypeVoid, mf(&hui::Window::on_middle_button_up));
		class_add_func_virtual("on_right_button_up", TypeVoid, mf(&hui::Window::on_right_button_up));
		class_add_func_virtual("on_double_click", TypeVoid, mf(&hui::Window::on_double_click));
		class_add_func_virtual("on_close_request", TypeVoid, mf(&hui::Window::on_close_request));
		class_add_func_virtual("on_key_down", TypeVoid, mf(&hui::Window::on_key_down));
		class_add_func_virtual("on_key_up", TypeVoid, mf(&hui::Window::on_key_up));
		class_add_func_virtual("on_draw", TypeVoid, mf(&hui::Window::on_draw));
			func_add_param("p", TypeHuiPainter);
		class_set_vtable(hui::Window);

	add_class(TypeHuiNixWindow);
		class_derive_from(TypeHuiWindow, false, true);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, mf(&hui::NixWindow::__init_ext__), Flags::OVERRIDE);
			func_add_param("title", TypeString);
			func_add_param("width", TypeInt);
			func_add_param("height", TypeInt);
		class_add_func_virtual(IDENTIFIER_FUNC_DELETE, TypeVoid, mf(&hui::Window::__delete__), Flags::OVERRIDE);
		class_set_vtable(hui::Window);

	add_class(TypeHuiDialog);
		class_derive_from(TypeHuiWindow, false, true);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, mf(&hui::Dialog::__init_ext__), Flags::OVERRIDE);
			func_add_param("title", TypeString);
			func_add_param("width", TypeInt);
			func_add_param("height", TypeInt);
			func_add_param("parent", TypeHuiWindow);
			func_add_param("allow_parent",TypeBool);
		class_add_func_virtual(IDENTIFIER_FUNC_DELETE, TypeVoid, mf(&hui::Window::__delete__), Flags::OVERRIDE);
		class_set_vtable(hui::Window);

	
	add_class(TypeHuiPainter);
		class_derive_from(TypeBasePainter, true, true);


	add_class(TypeHuiConfiguration);
		class_add_funcx(IDENTIFIER_FUNC_INIT, TypeVoid, &hui::Configuration::__init__);
		class_add_funcx(IDENTIFIER_FUNC_DELETE, TypeVoid, &hui::Configuration::__del__);
		class_add_funcx("load", TypeBool, &hui::Configuration::load);
			func_add_param("path", TypePath);
		class_add_funcx("save", TypeVoid, &hui::Configuration::save);
			func_add_param("path", TypePath);
		class_add_funcx(IDENTIFIER_FUNC_SET, TypeVoid, &hui::Configuration::set_int);
			func_add_param("name", TypeString);
			func_add_param("value", TypeInt);
		class_add_funcx(IDENTIFIER_FUNC_SET, TypeVoid, &hui::Configuration::set_float);
			func_add_param("name", TypeString);
			func_add_param("value", TypeFloat32);
		class_add_funcx(IDENTIFIER_FUNC_SET, TypeVoid, &hui::Configuration::set_bool);
			func_add_param("name", TypeString);
			func_add_param("value", TypeBool);
		class_add_funcx(IDENTIFIER_FUNC_SET, TypeVoid, &hui::Configuration::set_str);
			func_add_param("name", TypeString);
			func_add_param("value", TypeString);
		class_add_funcx("get_int", TypeInt, &hui::Configuration::get_int, Flags::CONST);
			func_add_param("name", TypeString);
			func_add_param("default", TypeInt);
		class_add_funcx("get_float", TypeFloat32, &hui::Configuration::get_float, Flags::CONST);
			func_add_param("name", TypeString);
			func_add_param("default", TypeFloat32);
		class_add_funcx("get_bool", TypeBool, &hui::Configuration::get_bool, Flags::CONST);
			func_add_param("name", TypeString);
			func_add_param("default", TypeBool);
		class_add_funcx("get_str", TypeString, &hui::Configuration::get_str, Flags::CONST);
			func_add_param("name", TypeString);
			func_add_param("default", TypeString);
		class_add_funcx(IDENTIFIER_FUNC_GET, TypeString, &_hui_config_get, Flags::CONST);
			func_add_param("name", TypeString);
		class_add_funcx("keys", TypeStringList, &hui::Configuration::keys, Flags::CONST);
	
	// user interface
	add_func("set_idle_function", TypeVoid, (void*)&HuiSetIdleFunctionKaba, Flags::STATIC);
		func_add_param("idle_func", TypeCallback);
	add_func("run_later", TypeInt, (void*)&HuiRunLaterKaba, Flags::STATIC);
		func_add_param("dt", TypeFloat32);
		func_add_param("handler", TypeObject);
		func_add_param("f", TypeCallback);
	add_func("run_repeated", TypeInt, (void*)&HuiRunRepeatedKaba, Flags::STATIC);
		func_add_param("dt", TypeFloat32);
		func_add_param("handler", TypeObject);
		func_add_param("f", TypeCallback);
	add_func("cancel_runner", TypeVoid, (void*)&hui::CancelRunner, Flags::STATIC);
		func_add_param("id", TypeInt);
	/*add_func("HuiAddKeyCode", TypeVoid, (void*)&hui::AddKeyCode, Flags::STATIC);
		func_add_param("id", TypeString);
		func_add_param("key_code", TypeInt);
	add_func("HuiAddCommand", TypeVoid, (void*)&hui::AddCommand, Flags::STATIC);
		func_add_param("id", TypeString);
		func_add_param("image", TypeString);
		func_add_param("key_code", TypeInt);
		func_add_param("func", TypeFunctionP);*/
	add_func("get_event", TypeHuiEventP, (void*)&hui::GetEvent, Flags::STATIC);
	/*add_func("HuiRun", TypeVoid, (void*)&hui::Run);
	add_func("HuiEnd", TypeVoid, (void*)&hui::End, Flags::STATIC);*/
	add_func("do_single_main_loop", TypeVoid, (void*)&hui::Application::do_single_main_loop, Flags::STATIC);
	add_func("file_dialog_open", TypeBool, (void*)&hui::FileDialogOpen, Flags::STATIC);
		func_add_param("root", TypeHuiWindow);
		func_add_param("title", TypeString);
		func_add_param("dir", TypePath);
		func_add_param("show_filter", TypeString);
		func_add_param("filter", TypeString);
	add_func("file_dialog_save", TypeBool, (void*)&hui::FileDialogSave, Flags::STATIC);
		func_add_param("root", TypeHuiWindow);
		func_add_param("title", TypeString);
		func_add_param("dir", TypePath);
		func_add_param("show_filter", TypeString);
		func_add_param("filter", TypeString);
	add_func("file_dialog_dir", TypeBool, (void*)&hui::FileDialogDir, Flags::STATIC);
		func_add_param("root", TypeHuiWindow);
		func_add_param("title", TypeString);
		func_add_param("dir", TypePath);
	add_func("question_box", TypeString, (void*)&hui::QuestionBox, Flags::STATIC);
		func_add_param("root", TypeHuiWindow);
		func_add_param("title", TypeString);
		func_add_param("text", TypeString);
		func_add_param("allow_cancel", TypeBool);
	add_func("info_box", TypeVoid, (void*)&hui::InfoBox, Flags::STATIC);
		func_add_param("root", TypeHuiWindow);
		func_add_param("title", TypeString);
		func_add_param("text", TypeString);
	add_func("error_box", TypeVoid, (void*)&hui::ErrorBox, Flags::STATIC);
		func_add_param("root", TypeHuiWindow);
		func_add_param("title", TypeString);
		func_add_param("text", TypeString);
	add_func("create_menu_from_source", TypeHuiMenuP, (void*)&hui::CreateMenuFromSource, Flags::STATIC);
		func_add_param("source", TypeString);
	add_func("get_key_name", TypeString, hui_p(&hui::GetKeyCodeName), Flags::_STATIC__PURE);
		func_add_param("id", TypeInt);
//	add_func("get_key_char", TypeString, hui_p(&hui::GetKeyChar), Flags::_STATIC__PURE);
//		func_add_param("id", TypeInt);

	// clipboard
	add_func("copy_to_clipboard", TypeVoid, (void*)&hui::Clipboard::Copy, Flags::STATIC);
		func_add_param("buffer", TypeString);
	add_func("paste_from_clipboard", TypeString, (void*)&hui::Clipboard::Paste, Flags::STATIC);
	add_func("open_document", TypeVoid, (void*)&hui::OpenDocument, Flags::STATIC);
		func_add_param("filename", TypePath);
	add_func("make_gui_image", TypeString, (void*)&hui::SetImage, Flags::STATIC);
		func_add_param("image", TypeImage);

	add_class(TypeHuiEvent);
		class_add_element("id", TypeString, GetDAEvent(id));
		class_add_element("message", TypeString, GetDAEvent(message));
		class_add_element("mouse_x", TypeFloat32, GetDAEvent(mx));
		class_add_element("mouse_y", TypeFloat32, GetDAEvent(my));
		class_add_element("scroll_x", TypeFloat32, GetDAEvent(scroll_x));
		class_add_element("scroll_y", TypeFloat32, GetDAEvent(scroll_y));
		class_add_element("key", TypeInt, GetDAEvent(key_code));
		class_add_element("width", TypeInt, GetDAEvent(width));
		class_add_element("height", TypeInt, GetDAEvent(height));
		class_add_element("button_l", TypeBool, GetDAEvent(lbut));
		class_add_element("button_m", TypeBool, GetDAEvent(mbut));
		class_add_element("button_r", TypeBool, GetDAEvent(rbut));
		class_add_element("row", TypeInt, GetDAEvent(row));
		class_add_element("column", TypeInt, GetDAEvent(column));

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
	add_const("KEY_PLUS",TypeInt,(void*)hui::KEY_PLUS);
	add_const("KEY_MINUS",TypeInt,(void*)hui::KEY_MINUS);
	add_const("KEY_FENCE",TypeInt,(void*)hui::KEY_FENCE);
	add_const("KEY_END",TypeInt,(void*)hui::KEY_END);
	add_const("KEY_PAGE_UP",TypeInt,(void*)hui::KEY_PAGE_UP);
	add_const("KEY_PAGE_DOWN",TypeInt,(void*)hui::KEY_PAGE_DOWN);
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
	add_const("KEY_LESS",TypeInt,(void*)hui::KEY_LESS);
	add_const("KEY_SZ",TypeInt,(void*)hui::KEY_SZ);
	add_const("KEY_AE",TypeInt,(void*)hui::KEY_AE);
	add_const("KEY_OE",TypeInt,(void*)hui::KEY_OE);
	add_const("KEY_UE",TypeInt,(void*)hui::KEY_UE);
	add_const("NUM_KEYS",TypeInt,(void*)hui::NUM_KEYS);
	add_const("KEY_ANY",TypeInt,(void*)hui::KEY_ANY);

	add_ext_var("app_filename", TypePath, hui_p(&hui::Application::filename));
	add_ext_var("app_directory", TypePath, hui_p(&hui::Application::directory));
	add_ext_var("app_directory_static", TypePath, hui_p(&hui::Application::directory_static));
	add_ext_var("filename", TypePath, hui_p(&hui::Filename));
	add_ext_var("app_config", TypeHuiConfiguration, hui_p(&hui::Config));
}

};
