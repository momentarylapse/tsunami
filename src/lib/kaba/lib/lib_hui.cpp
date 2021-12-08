#include "../kaba.h"
#include "../../config.h"
#include "lib.h"
#include "../../base/callable.h"

#ifdef _X_USE_HUI_
	#include "../../hui/hui.h"
#elif defined(_X_USE_HUI_MINIMAL_)
	#include "../../hui_minimal/hui.h"
#else
	we are re screwed.... TODO: test for _X_USE_HUI_
#endif


namespace hui{
#ifdef _X_USE_HUI_
	Menu *CreateMenuFromSource(const string &source);
#endif
#ifdef _X_USE_HUI_MINIMAL_
	typedef int Menu;
	typedef int Toolbar;
	typedef int Window;
	typedef int Dialog;
	typedef int Panel;
	typedef int Event;
	typedef int Painter;
#endif
}

namespace kaba {

#ifdef _X_USE_HUI_
	static hui::Event *_event;
	static hui::Panel *_panel;
	#define GetDAPanel(x)			int_p(&_panel->x)-int_p(_panel)
	#define GetDAWindow(x)			int_p(&_win->x)-int_p(_win)
	#define GetDAEvent(x)	int_p(&_event->x)-int_p(_event)
	void HuiSetIdleFunctionKaba(Callable<void()> &c) {
		hui::SetIdleFunction([&c]{ c(); });
	}
	int HuiRunLaterKaba(float dt, hui::EventHandler *p, Callable<void(hui::EventHandler*)> &c) {
		return hui::RunLater(dt, [&c,p]{ c(p); });
	}
	int HuiRunRepeatedKaba(float dt, hui::EventHandler *p, Callable<void(hui::EventHandler*)> &c) {
		return hui::RunRepeated(dt, [&c,p]{ c(p); });
	}
	class KabaPanelWrapper : public hui::Panel {
	public:
		void _kaba_event(const string &id, Callable<void(hui::EventHandler*)> &c) {
			_kaba_event_o(id, this, c);
		}
		void _kaba_event_o(const string &id, hui::EventHandler *handler, Callable<void(hui::EventHandler*)> &c) {
			event(id, [&c,handler]{ c(handler); });
		}
		void _kaba_event_x(const string &id, const string &msg, void *f) {
			_kaba_event_ox(id, msg, this, f);
		}
		void _kaba_event_ox(const string &id, const string &msg, hui::EventHandler *handler, void *f) {
			if (msg == "hui:draw"){
				auto &ff = *(Callable<void(hui::EventHandler*, Painter*)>*)f;
				event_xp(id, msg, [&ff,handler](Painter *p){ ff(handler, p); });
			}else{
				auto &ff = *(Callable<void(hui::EventHandler*)>*)f;
				event_x(id, msg, [&ff,handler]{ ff(handler); });
			}
		}
	};
#else
	#define GetDAWindow(x)		0
	#define GetDAEvent(x)	0
	#define GetDAPanel(x) 0
#endif

#ifdef _X_USE_HUI_
	#define hui_p(p)		p
#else
	#define hui_p(p)		nullptr
#endif


extern const Class *TypeObject;
extern const Class *TypeIntList;
extern const Class *TypeIntPs;
extern const Class *TypeStringList;
extern const Class *TypeImage;
extern const Class *TypeBasePainter;
extern const Class *TypePath;
extern const Class *TypeVec2;
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

	auto TypeCallback = add_type_f(TypeVoid, {});
	auto TypeCallbackObject = add_type_f(TypeVoid, {TypeObject});
	auto TypeCallbackObjectP = add_type_f(TypeVoid, {TypeObject, TypeHuiPainter});


	add_class(TypeHuiMenu);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, hui_p(&hui::Menu::__init__));
		class_add_func("popup", TypeVoid, hui_p(&hui::Menu::open_popup));
			func_add_param("p", TypeHuiPanel);
		class_add_func("add", TypeVoid, hui_p(&hui::Menu::add));
			func_add_param("name", TypeString);
			func_add_param("id", TypeString);
		class_add_func("add_with_image", TypeVoid, hui_p(&hui::Menu::add_with_image));
			func_add_param("name", TypeString);
			func_add_param("image", TypeInt);
			func_add_param("id", TypeString);
		class_add_func("add_checkable", TypeVoid, hui_p(&hui::Menu::add_checkable));
			func_add_param("name", TypeString);
			func_add_param("id", TypeString);
		class_add_func("add_separator", TypeVoid, hui_p(&hui::Menu::add_separator));
		class_add_func("add_sub_menu", TypeVoid, hui_p(&hui::Menu::add_sub_menu));
			func_add_param("name", TypeString);
			func_add_param("id", TypeString);
			func_add_param("sub_menu", TypeHuiMenu);
		class_add_func("enable", TypeVoid, hui_p(&hui::Menu::enable));
			func_add_param("id", TypeString);
			func_add_param("enabled", TypeBool);
		class_add_func("check", TypeVoid, hui_p(&hui::Menu::check));
			func_add_param("id", TypeString);
			func_add_param("checked", TypeBool);

	add_class(TypeHuiToolbar);
		class_derive_from(TypeObject, false, true);
		class_add_func("set_by_id", TypeVoid, hui_p(&hui::Toolbar::set_by_id));
			func_add_param("id", TypeString);
		class_add_func("from_source", TypeVoid, hui_p(&hui::Toolbar::from_source));
			func_add_param("source", TypeString);

	add_class(TypeHuiPanel);
		class_derive_from(TypeObject, false, true);
		class_add_element("win", TypeHuiWindowP, GetDAPanel(win));
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, hui_p(&hui::Panel::__init__), Flags::OVERRIDE);
		class_add_func_virtual(IDENTIFIER_FUNC_DELETE, TypeVoid, hui_p(&hui::Panel::__delete__), Flags::OVERRIDE);
		class_add_func("set_border_width", TypeVoid, hui_p(&hui::Panel::set_border_width));
			func_add_param("width", TypeInt);
		class_add_func("set_decimals", TypeVoid, hui_p(&hui::Panel::set_decimals));
			func_add_param("decimals", TypeInt);
		class_add_func("activate", TypeVoid, hui_p(&hui::Panel::activate));
			func_add_param("id", TypeString);
		class_add_func("is_active", TypeVoid, hui_p(&hui::Panel::is_active));
			func_add_param("id", TypeString);
		class_add_func("from_source", TypeVoid, hui_p(&hui::Panel::from_source));
			func_add_param("source", TypeString);
		class_add_func("add_button", TypeVoid, hui_p(&hui::Panel::add_button));
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
			func_add_param("id", TypeString);
		class_add_func("add_def_button", TypeVoid, hui_p(&hui::Panel::add_def_button));
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
			func_add_param("id", TypeString);
		class_add_func("add_check_box", TypeVoid, hui_p(&hui::Panel::add_check_box));
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
			func_add_param("id", TypeString);
		class_add_func("add_label", TypeVoid, hui_p(&hui::Panel::add_label));
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
			func_add_param("id", TypeString);
		class_add_func("add_edit", TypeVoid, hui_p(&hui::Panel::add_edit));
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
			func_add_param("id", TypeString);
		class_add_func("add_multiline_edit", TypeVoid, hui_p(&hui::Panel::add_multiline_edit));
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
			func_add_param("id", TypeString);
		class_add_func("add_group", TypeVoid, hui_p(&hui::Panel::add_group));
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
			func_add_param("id", TypeString);
		class_add_func("add_combo_box", TypeVoid, hui_p(&hui::Panel::add_combo_box));
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
			func_add_param("id", TypeString);
		class_add_func("add_tab_control", TypeVoid, hui_p(&hui::Panel::add_tab_control));
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
			func_add_param("id", TypeString);
		class_add_func("set_target", TypeVoid, hui_p(&hui::Panel::set_target));
			func_add_param("id", TypeString);
		class_add_func("add_list_view", TypeVoid, hui_p(&hui::Panel::add_list_view));
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
			func_add_param("id", TypeString);
		class_add_func("add_tree_view", TypeVoid, hui_p(&hui::Panel::add_tree_view));
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
			func_add_param("id", TypeString);
		class_add_func("add_icon_view", TypeVoid, hui_p(&hui::Panel::add_icon_view));
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
			func_add_param("id", TypeString);
		class_add_func("add_progress_bar", TypeVoid, hui_p(&hui::Panel::add_progress_bar));
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
			func_add_param("id", TypeString);
		class_add_func("add_slider", TypeVoid, hui_p(&hui::Panel::add_slider));
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
			func_add_param("id", TypeString);
		class_add_func("add_drawing_area", TypeVoid, hui_p(&hui::Panel::add_drawing_area));
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
			func_add_param("id", TypeString);
		class_add_func("add_grid", TypeVoid, hui_p(&hui::Panel::add_grid));
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
			func_add_param("id", TypeString);
		class_add_func("add_spin_button", TypeVoid, hui_p(&hui::Panel::add_spin_button));
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
			func_add_param("id", TypeString);
		class_add_func("add_radio_button", TypeVoid, hui_p(&hui::Panel::add_radio_button));
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
			func_add_param("id", TypeString);
		class_add_func("add_scroller", TypeVoid, hui_p(&hui::Panel::add_scroller));
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
			func_add_param("id", TypeString);
		class_add_func("add_expander", TypeVoid, hui_p(&hui::Panel::add_expander));
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
			func_add_param("id", TypeString);
		class_add_func("add_separator", TypeVoid, hui_p(&hui::Panel::add_separator));
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
			func_add_param("id", TypeString);
		class_add_func("add_paned", TypeVoid, hui_p(&hui::Panel::add_paned));
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
			func_add_param("id", TypeString);
		class_add_func("add_revealer", TypeVoid, hui_p(&hui::Panel::add_revealer));
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
			func_add_param("id", TypeString);
		class_add_func("embed", TypeVoid, hui_p(&hui::Panel::embed));
			func_add_param("panel", TypeHuiPanel);
			func_add_param("id", TypeString);
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
		class_add_func("set_string", TypeVoid, hui_p(&hui::Panel::set_string));
			func_add_param("id", TypeString);
			func_add_param("s", TypeString);
		class_add_func("add_string", TypeVoid, hui_p(&hui::Panel::add_string));
			func_add_param("id", TypeString);
			func_add_param("s", TypeString);
		class_add_func("get_string", TypeString, hui_p(&hui::Panel::get_string));
			func_add_param("id", TypeString);
		class_add_func("set_float", TypeVoid, hui_p(&hui::Panel::set_float));
			func_add_param("id", TypeString);
			func_add_param("f", TypeFloat32);
		class_add_func("get_float", TypeFloat32, hui_p(&hui::Panel::get_float));
			func_add_param("id", TypeString);
		class_add_func("enable", TypeVoid, hui_p(&hui::Panel::enable));
			func_add_param("id", TypeString);
			func_add_param("enabled", TypeBool);
		class_add_func("is_enabled", TypeBool, hui_p(&hui::Panel::is_enabled));
			func_add_param("id", TypeString);
		class_add_func("check", TypeVoid, hui_p(&hui::Panel::check));
			func_add_param("id", TypeString);
			func_add_param("checked", TypeBool);
		class_add_func("is_checked", TypeBool, hui_p(&hui::Panel::is_checked));
			func_add_param("id", TypeString);
		class_add_func("hide_control", TypeVoid, hui_p(&hui::Panel::hide_control));
			func_add_param("id", TypeString);
			func_add_param("hide", TypeBool);
		class_add_func("delete_control", TypeVoid, hui_p(&hui::Panel::delete_control));
			func_add_param("id", TypeString);
		class_add_func("get_int", TypeInt, hui_p(&hui::Panel::get_int));
			func_add_param("id", TypeString);
		class_add_func("get_selection", TypeIntList, hui_p(&hui::Panel::get_selection));
			func_add_param("id", TypeString);
		class_add_func("set_int", TypeVoid, hui_p(&hui::Panel::set_int));
			func_add_param("id", TypeString);
			func_add_param("i", TypeInt);
		class_add_func("set_image", TypeVoid, hui_p(&hui::Panel::set_image));
			func_add_param("id", TypeString);
			func_add_param("image", TypeString);
		class_add_func("get_cell", TypeString, hui_p(&hui::Panel::get_cell));
			func_add_param("id", TypeString);
			func_add_param("row", TypeInt);
			func_add_param("column", TypeInt);
		class_add_func("set_cell", TypeVoid, hui_p(&hui::Panel::set_cell));
			func_add_param("id", TypeString);
			func_add_param("row", TypeInt);
			func_add_param("column", TypeInt);
			func_add_param("s", TypeString);
		class_add_func("set_options", TypeVoid, hui_p(&hui::Panel::set_options));
			func_add_param("id", TypeString);
			func_add_param("options", TypeString);
		class_add_func("reset", TypeVoid, hui_p(&hui::Panel::reset));
			func_add_param("id", TypeString);
		class_add_func("redraw", TypeVoid, hui_p(&hui::Panel::redraw));
			func_add_param("id", TypeString);
		class_add_func("reveal", TypeVoid, hui_p(&hui::Panel::reveal));
			func_add_param("id", TypeString);
			func_add_param("reveal", TypeBool);
		class_add_func("is_revealed", TypeBool, hui_p(&hui::Panel::is_revealed));
			func_add_param("id", TypeString);
		class_add_func("event", TypeInt, hui_p(&KabaPanelWrapper::_kaba_event));
			func_add_param("id", TypeString);
			func_add_param("func", TypeCallbackObject);
		class_add_func("event_o", TypeInt, hui_p(&KabaPanelWrapper::_kaba_event_o));
			func_add_param("id", TypeString);
			func_add_param("handler", TypeObject);
			func_add_param("func", TypeCallbackObject);
		class_add_func("event_x", TypeInt, hui_p(&KabaPanelWrapper::_kaba_event_x));
			func_add_param("id", TypeString);
			func_add_param("msg", TypeString);
			func_add_param("func", TypeCallbackObject);
		class_add_func("event_x", TypeInt, hui_p(&KabaPanelWrapper::_kaba_event_x));
			func_add_param("id", TypeString);
			func_add_param("msg", TypeString);
			func_add_param("func", TypeCallbackObjectP);
		class_add_func("event_ox", TypeInt, hui_p(&KabaPanelWrapper::_kaba_event_ox));
			func_add_param("id", TypeString);
			func_add_param("msg", TypeString);
			func_add_param("handler", TypeObject);
			func_add_param("func", TypeCallbackObject);
		class_add_func("event_ox", TypeInt, hui_p(&KabaPanelWrapper::_kaba_event_ox));
			func_add_param("id", TypeString);
			func_add_param("msg", TypeString);
			func_add_param("handler", TypeObject);
			func_add_param("func", TypeCallbackObjectP);
		class_add_func("remove_event_handler", TypeVoid, hui_p(&hui::Panel::remove_event_handler));
			func_add_param("uid", TypeInt);
#ifdef _X_USE_HUI_
		class_set_vtable(hui::Panel);
#endif


	add_class(TypeHuiWindow);
		class_derive_from(TypeHuiPanel, false, true);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, hui_p(&hui::Window::__init_ext__), Flags::OVERRIDE);
			func_add_param("title", TypeString);
			func_add_param("width", TypeInt);
			func_add_param("height", TypeInt);
		class_add_func_virtual(IDENTIFIER_FUNC_DELETE, TypeVoid, hui_p(&hui::Window::__delete__), Flags::OVERRIDE);
		class_add_func("run", TypeVoid, hui_p(&hui::Window::run));
		class_add_func("destroy", TypeVoid, hui_p(&hui::Window::request_destroy));
		class_add_func("show", TypeVoid, hui_p(&hui::Window::show));
		class_add_func("hide", TypeVoid, hui_p(&hui::Window::hide));

		class_add_func("set_menu", TypeVoid, hui_p(&hui::Window::set_menu));
			func_add_param("menu", TypeHuiMenu);
		class_add_func("toolbar", TypeHuiToolbarP, hui_p(&hui::Window::get_toolbar), Flags::SELFREF);
			func_add_param("index", TypeInt);
		class_add_func("set_maximized", TypeVoid, hui_p(&hui::Window::set_maximized));
			func_add_param("max", TypeBool);
		class_add_func("is_maximized", TypeBool, hui_p(&hui::Window::is_maximized));
		class_add_func("is_minimized", TypeBool, hui_p(&hui::Window::is_minimized));
		class_add_func("set_id", TypeVoid, hui_p(&hui::Window::set_id));
			func_add_param("id", TypeInt);
		class_add_func("set_fullscreen", TypeVoid, hui_p(&hui::Window::set_fullscreen));
			func_add_param("fullscreen",TypeBool);
		class_add_func("set_title", TypeVoid, hui_p(&hui::Window::set_title));
			func_add_param("title", TypeString);
		class_add_func("set_position", TypeVoid, hui_p(&hui::Window::set_position));
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
	//add_func("setOuterior", TypeVoid, 2, TypePointer,"win",
	//																										TypeIRect,"r");
	//add_func("getOuterior", TypeIRect, 1, TypePointer,"win");
	//add_func("setInerior", TypeVoid, 2, TypePointer,"win",
	//																										TypeIRect,"r");
	//add_func("getInterior", TypeIRect, 1, TypePointer,"win");
		class_add_func("set_cursor_pos", TypeVoid, hui_p(&hui::Window::set_cursor_pos));
			func_add_param("x", TypeInt);
			func_add_param("y", TypeInt);
		class_add_func("get_mouse", TypeBool, hui_p(&hui::Window::get_mouse));
			func_add_param("x", TypeIntPs);
			func_add_param("y", TypeIntPs);
			func_add_param("button", TypeInt);
			func_add_param("change", TypeInt);
		class_add_func("get_key", TypeBool, hui_p(&hui::Window::get_key));
			func_add_param("key", TypeInt);
		class_add_func_virtual("on_mouse_move", TypeVoid, hui_p(&hui::Window::on_mouse_move));
		class_add_func_virtual("on_mouse_wheel", TypeVoid, hui_p(&hui::Window::on_mouse_wheel));
		class_add_func_virtual("on_left_button_down", TypeVoid, hui_p(&hui::Window::on_left_button_down));
		class_add_func_virtual("on_middle_button_down", TypeVoid, hui_p(&hui::Window::on_middle_button_down));
		class_add_func_virtual("on_right_button_down", TypeVoid, hui_p(&hui::Window::on_right_button_down));
		class_add_func_virtual("on_left_button_up", TypeVoid, hui_p(&hui::Window::on_left_button_up));
		class_add_func_virtual("on_middle_button_up", TypeVoid, hui_p(&hui::Window::on_middle_button_up));
		class_add_func_virtual("on_right_button_up", TypeVoid, hui_p(&hui::Window::on_right_button_up));
		class_add_func_virtual("on_double_click", TypeVoid, hui_p(&hui::Window::on_double_click));
		class_add_func_virtual("on_close_request", TypeVoid, hui_p(&hui::Window::on_close_request));
		class_add_func_virtual("on_key_down", TypeVoid, hui_p(&hui::Window::on_key_down));
		class_add_func_virtual("on_key_up", TypeVoid, hui_p(&hui::Window::on_key_up));
		class_add_func_virtual("on_draw", TypeVoid, hui_p(&hui::Window::on_draw));
			func_add_param("p", TypeHuiPainter);
#ifdef _X_USE_HUI_
		class_set_vtable(hui::Window);
#endif

	add_class(TypeHuiNixWindow);
		class_derive_from(TypeHuiWindow, false, true);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, hui_p(&hui::NixWindow::__init_ext__), Flags::OVERRIDE);
			func_add_param("title", TypeString);
			func_add_param("width", TypeInt);
			func_add_param("height", TypeInt);
		class_add_func_virtual(IDENTIFIER_FUNC_DELETE, TypeVoid, hui_p(&hui::Window::__delete__), Flags::OVERRIDE);
#ifdef _X_USE_HUI_
		class_set_vtable(hui::Window);
#endif

	add_class(TypeHuiDialog);
		class_derive_from(TypeHuiWindow, false, true);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, hui_p(&hui::Dialog::__init_ext__), Flags::OVERRIDE);
			func_add_param("title", TypeString);
			func_add_param("width", TypeInt);
			func_add_param("height", TypeInt);
			func_add_param("parent", TypeHuiWindow);
			func_add_param("allow_parent",TypeBool);
		class_add_func_virtual(IDENTIFIER_FUNC_DELETE, TypeVoid, hui_p(&hui::Window::__delete__), Flags::OVERRIDE);
#ifdef _X_USE_HUI_
		class_set_vtable(hui::Window);
#endif

	
	add_class(TypeHuiPainter);
		class_derive_from(TypeBasePainter, true, true);


	add_class(TypeHuiConfiguration);
		class_add_func(IDENTIFIER_FUNC_INIT, TypeVoid, &hui::Configuration::__init__);
		class_add_func(IDENTIFIER_FUNC_DELETE, TypeVoid, &hui::Configuration::__del__);
		class_add_func("load", TypeBool, &hui::Configuration::load);
			func_add_param("path", TypePath);
		class_add_func("save", TypeVoid, &hui::Configuration::save);
			func_add_param("path", TypePath);
		class_add_func(IDENTIFIER_FUNC_SET, TypeVoid, &hui::Configuration::set_int);
			func_add_param("name", TypeString);
			func_add_param("value", TypeInt);
		class_add_func(IDENTIFIER_FUNC_SET, TypeVoid, &hui::Configuration::set_float);
			func_add_param("name", TypeString);
			func_add_param("value", TypeFloat32);
		class_add_func(IDENTIFIER_FUNC_SET, TypeVoid, &hui::Configuration::set_bool);
			func_add_param("name", TypeString);
			func_add_param("value", TypeBool);
		class_add_func(IDENTIFIER_FUNC_SET, TypeVoid, &hui::Configuration::set_str);
			func_add_param("name", TypeString);
			func_add_param("value", TypeString);
		class_add_func("get_int", TypeInt, &hui::Configuration::get_int, Flags::CONST);
			func_add_param("name", TypeString);
			func_add_param("default", TypeInt);
		class_add_func("get_float", TypeFloat32, &hui::Configuration::get_float, Flags::CONST);
			func_add_param("name", TypeString);
			func_add_param("default", TypeFloat32);
		class_add_func("get_bool", TypeBool, &hui::Configuration::get_bool, Flags::CONST);
			func_add_param("name", TypeString);
			func_add_param("default", TypeBool);
		class_add_func("get_str", TypeString, &hui::Configuration::get_str, Flags::CONST);
			func_add_param("name", TypeString);
			func_add_param("default", TypeString);
		class_add_func(IDENTIFIER_FUNC_GET, TypeString, &_hui_config_get, Flags::CONST);
			func_add_param("name", TypeString);
		class_add_func("keys", TypeStringList, &hui::Configuration::keys, Flags::CONST);
	
	// user interface
	add_func("set_idle_function", TypeVoid, hui_p(&HuiSetIdleFunctionKaba), Flags::STATIC);
		func_add_param("idle_func", TypeCallback);
	add_func("run_later", TypeInt, hui_p(&HuiRunLaterKaba), Flags::STATIC);
		func_add_param("dt", TypeFloat32);
		func_add_param("handler", TypeObject);
		func_add_param("f", TypeCallbackObject);
	add_func("run_repeated", TypeInt, hui_p(&HuiRunRepeatedKaba), Flags::STATIC);
		func_add_param("dt", TypeFloat32);
		func_add_param("handler", TypeObject);
		func_add_param("f", TypeCallbackObject);
	add_func("cancel_runner", TypeVoid, hui_p(&hui::CancelRunner), Flags::STATIC);
		func_add_param("id", TypeInt);
	/*add_func("HuiAddKeyCode", TypeVoid, (void*)&hui::AddKeyCode, Flags::STATIC);
		func_add_param("id", TypeString);
		func_add_param("key_code", TypeInt);
	add_func("HuiAddCommand", TypeVoid, (void*)&hui::AddCommand, Flags::STATIC);
		func_add_param("id", TypeString);
		func_add_param("image", TypeString);
		func_add_param("key_code", TypeInt);
		func_add_param("func", TypeFunctionP);*/
	add_func("get_event", TypeHuiEventP, hui_p(&hui::GetEvent), Flags::STATIC);
	/*add_func("HuiRun", TypeVoid, (void*)&hui::Run);
	add_func("HuiEnd", TypeVoid, (void*)&hui::End, Flags::STATIC);*/
	add_func("do_single_main_loop", TypeVoid, hui_p(&hui::Application::do_single_main_loop), Flags::STATIC);
	add_func("file_dialog_open", TypeBool, hui_p(&hui::FileDialogOpen), Flags::STATIC);
		func_add_param("root", TypeHuiWindow);
		func_add_param("title", TypeString);
		func_add_param("dir", TypePath);
		func_add_param("show_filter", TypeString);
		func_add_param("filter", TypeString);
	add_func("file_dialog_save", TypeBool, hui_p(&hui::FileDialogSave), Flags::STATIC);
		func_add_param("root", TypeHuiWindow);
		func_add_param("title", TypeString);
		func_add_param("dir", TypePath);
		func_add_param("show_filter", TypeString);
		func_add_param("filter", TypeString);
	add_func("file_dialog_dir", TypeBool, hui_p(&hui::FileDialogDir), Flags::STATIC);
		func_add_param("root", TypeHuiWindow);
		func_add_param("title", TypeString);
		func_add_param("dir", TypePath);
	add_func("question_box", TypeString, hui_p(&hui::QuestionBox), Flags::STATIC);
		func_add_param("root", TypeHuiWindow);
		func_add_param("title", TypeString);
		func_add_param("text", TypeString);
		func_add_param("allow_cancel", TypeBool);
	add_func("info_box", TypeVoid, hui_p(&hui::InfoBox), Flags::STATIC);
		func_add_param("root", TypeHuiWindow);
		func_add_param("title", TypeString);
		func_add_param("text", TypeString);
	add_func("error_box", TypeVoid, hui_p(&hui::ErrorBox), Flags::STATIC);
		func_add_param("root", TypeHuiWindow);
		func_add_param("title", TypeString);
		func_add_param("text", TypeString);
	add_func("create_menu_from_source", TypeHuiMenuP, hui_p(&hui::CreateMenuFromSource), Flags::STATIC);
		func_add_param("source", TypeString);
	add_func("get_key_name", TypeString, hui_p(&hui::GetKeyCodeName), Flags::_STATIC__PURE);
		func_add_param("id", TypeInt);
//	add_func("get_key_char", TypeString, hui_p(&hui::GetKeyChar), Flags::_STATIC__PURE);
//		func_add_param("id", TypeInt);

	// clipboard
	add_func("copy_to_clipboard", TypeVoid, hui_p(&hui::Clipboard::Copy), Flags::STATIC);
		func_add_param("buffer", TypeString);
	add_func("paste_from_clipboard", TypeString, hui_p(&hui::Clipboard::Paste), Flags::STATIC);
	add_func("open_document", TypeVoid, hui_p(&hui::OpenDocument), Flags::STATIC);
		func_add_param("filename", TypePath);
	add_func("make_gui_image", TypeString, hui_p(&hui::SetImage), Flags::STATIC);
		func_add_param("image", TypeImage);

	add_class(TypeHuiEvent);
		class_add_element("id", TypeString, GetDAEvent(id));
		class_add_element("message", TypeString, GetDAEvent(message));
		class_add_element("mouse", TypeVec2, GetDAEvent(m));
		class_add_element("scroll", TypeVec2, GetDAEvent(scroll));
		class_add_element("key", TypeInt, GetDAEvent(key_code));
		class_add_element("width", TypeInt, GetDAEvent(width));
		class_add_element("height", TypeInt, GetDAEvent(height));
		class_add_element("button_l", TypeBool, GetDAEvent(lbut));
		class_add_element("button_m", TypeBool, GetDAEvent(mbut));
		class_add_element("button_r", TypeBool, GetDAEvent(rbut));
		class_add_element("row", TypeInt, GetDAEvent(row));
		class_add_element("column", TypeInt, GetDAEvent(column));

	// key ids (int)
	add_enum("KEY_CONTROL", TypeInt, hui::KEY_CONTROL);
	add_enum("KEY_LEFT_CONTROL", TypeInt, hui::KEY_LCONTROL);
	add_enum("KEY_RIGHT_CONTROL", TypeInt, hui::KEY_RCONTROL);
	add_enum("KEY_SHIFT", TypeInt, hui::KEY_SHIFT);
	add_enum("KEY_LEFT_SHIFT", TypeInt, hui::KEY_LSHIFT);
	add_enum("KEY_RIGHT_SHIFT", TypeInt, hui::KEY_RSHIFT);
	add_enum("KEY_ALT", TypeInt, hui::KEY_ALT);
	add_enum("KEY_LEFT_ALT", TypeInt, hui::KEY_LALT);
	add_enum("KEY_RIGHT_ALT", TypeInt, hui::KEY_RALT);
	add_enum("KEY_PLUS", TypeInt, hui::KEY_PLUS);
	add_enum("KEY_MINUS", TypeInt, hui::KEY_MINUS);
	add_enum("KEY_FENCE", TypeInt, hui::KEY_FENCE);
	add_enum("KEY_END", TypeInt, hui::KEY_END);
	add_enum("KEY_PAGE_UP", TypeInt, hui::KEY_PAGE_UP);
	add_enum("KEY_PAGE_DOWN", TypeInt, hui::KEY_PAGE_DOWN);
	add_enum("KEY_UP", TypeInt, hui::KEY_UP);
	add_enum("KEY_DOWN", TypeInt, hui::KEY_DOWN);
	add_enum("KEY_LEFT", TypeInt, hui::KEY_LEFT);
	add_enum("KEY_RIGHT", TypeInt, hui::KEY_RIGHT);
	add_enum("KEY_RETURN", TypeInt, hui::KEY_RETURN);
	add_enum("KEY_ESCAPE", TypeInt, hui::KEY_ESCAPE);
	add_enum("KEY_INSERT", TypeInt, hui::KEY_INSERT);
	add_enum("KEY_DELETE", TypeInt, hui::KEY_DELETE);
	add_enum("KEY_SPACE", TypeInt, hui::KEY_SPACE);
	add_enum("KEY_F1", TypeInt, hui::KEY_F1);
	add_enum("KEY_F2", TypeInt, hui::KEY_F2);
	add_enum("KEY_F3", TypeInt, hui::KEY_F3);
	add_enum("KEY_F4", TypeInt, hui::KEY_F4);
	add_enum("KEY_F5", TypeInt, hui::KEY_F5);
	add_enum("KEY_F6", TypeInt, hui::KEY_F6);
	add_enum("KEY_F7", TypeInt, hui::KEY_F7);
	add_enum("KEY_F8", TypeInt, hui::KEY_F8);
	add_enum("KEY_F9", TypeInt, hui::KEY_F9);
	add_enum("KEY_F10", TypeInt, hui::KEY_F10);
	add_enum("KEY_F11", TypeInt, hui::KEY_F11);
	add_enum("KEY_F12", TypeInt, hui::KEY_F12);
	add_enum("KEY_0", TypeInt, hui::KEY_0);
	add_enum("KEY_1", TypeInt, hui::KEY_1);
	add_enum("KEY_2", TypeInt, hui::KEY_2);
	add_enum("KEY_3", TypeInt, hui::KEY_3);
	add_enum("KEY_4", TypeInt, hui::KEY_4);
	add_enum("KEY_5", TypeInt, hui::KEY_5);
	add_enum("KEY_6", TypeInt, hui::KEY_6);
	add_enum("KEY_7", TypeInt, hui::KEY_7);
	add_enum("KEY_8", TypeInt, hui::KEY_8);
	add_enum("KEY_9", TypeInt, hui::KEY_9);
	add_enum("KEY_A", TypeInt, hui::KEY_A);
	add_enum("KEY_B", TypeInt, hui::KEY_B);
	add_enum("KEY_C", TypeInt, hui::KEY_C);
	add_enum("KEY_D", TypeInt, hui::KEY_D);
	add_enum("KEY_E", TypeInt, hui::KEY_E);
	add_enum("KEY_F", TypeInt, hui::KEY_F);
	add_enum("KEY_G", TypeInt, hui::KEY_G);
	add_enum("KEY_H", TypeInt, hui::KEY_H);
	add_enum("KEY_I", TypeInt, hui::KEY_I);
	add_enum("KEY_J", TypeInt, hui::KEY_J);
	add_enum("KEY_K", TypeInt, hui::KEY_K);
	add_enum("KEY_L", TypeInt, hui::KEY_L);
	add_enum("KEY_M", TypeInt, hui::KEY_M);
	add_enum("KEY_N", TypeInt, hui::KEY_N);
	add_enum("KEY_O", TypeInt, hui::KEY_O);
	add_enum("KEY_P", TypeInt, hui::KEY_P);
	add_enum("KEY_Q", TypeInt, hui::KEY_Q);
	add_enum("KEY_R", TypeInt, hui::KEY_R);
	add_enum("KEY_S", TypeInt, hui::KEY_S);
	add_enum("KEY_T", TypeInt, hui::KEY_T);
	add_enum("KEY_U", TypeInt, hui::KEY_U);
	add_enum("KEY_V", TypeInt, hui::KEY_V);
	add_enum("KEY_W", TypeInt, hui::KEY_W);
	add_enum("KEY_X", TypeInt, hui::KEY_X);
	add_enum("KEY_Y", TypeInt, hui::KEY_Y);
	add_enum("KEY_Z", TypeInt, hui::KEY_Z);
	add_enum("KEY_BACKSPACE", TypeInt, hui::KEY_BACKSPACE);
	add_enum("KEY_TAB", TypeInt, hui::KEY_TAB);
	add_enum("KEY_HOME", TypeInt, hui::KEY_HOME);
	add_enum("KEY_NUM_0", TypeInt, hui::KEY_NUM_0);
	add_enum("KEY_NUM_1", TypeInt, hui::KEY_NUM_1);
	add_enum("KEY_NUM_2", TypeInt, hui::KEY_NUM_2);
	add_enum("KEY_NUM_3", TypeInt, hui::KEY_NUM_3);
	add_enum("KEY_NUM_4", TypeInt, hui::KEY_NUM_4);
	add_enum("KEY_NUM_5", TypeInt, hui::KEY_NUM_5);
	add_enum("KEY_NUM_6", TypeInt, hui::KEY_NUM_6);
	add_enum("KEY_NUM_7", TypeInt, hui::KEY_NUM_7);
	add_enum("KEY_NUM_8", TypeInt, hui::KEY_NUM_8);
	add_enum("KEY_NUM_9", TypeInt, hui::KEY_NUM_9);
	add_enum("KEY_NUM_PLUS", TypeInt, hui::KEY_NUM_ADD);
	add_enum("KEY_NUM_MINUS", TypeInt, hui::KEY_NUM_SUBTRACT);
	add_enum("KEY_NUM_MULTIPLY", TypeInt, hui::KEY_NUM_MULTIPLY);
	add_enum("KEY_NUM_DIVIDE", TypeInt, hui::KEY_NUM_DIVIDE);
	add_enum("KEY_NUM_COMMA", TypeInt, hui::KEY_NUM_COMMA);
	add_enum("KEY_NUM_ENTER", TypeInt, hui::KEY_NUM_ENTER);
	add_enum("KEY_COMMA", TypeInt, hui::KEY_COMMA);
	add_enum("KEY_DOT", TypeInt, hui::KEY_DOT);
	add_enum("KEY_LESS", TypeInt, hui::KEY_LESS);
	add_enum("KEY_SZ", TypeInt, hui::KEY_SZ);
	add_enum("KEY_AE", TypeInt, hui::KEY_AE);
	add_enum("KEY_OE", TypeInt, hui::KEY_OE);
	add_enum("KEY_UE", TypeInt, hui::KEY_UE);
	add_enum("NUM_KEYS", TypeInt,hui::NUM_KEYS);
	add_enum("KEY_ANY", TypeInt, hui::KEY_ANY);

	add_ext_var("app_filename", TypePath, hui_p(&hui::Application::filename));
	add_ext_var("app_directory", TypePath, hui_p(&hui::Application::directory));
	add_ext_var("app_directory_static", TypePath, hui_p(&hui::Application::directory_static));
	add_ext_var("filename", TypePath, hui_p(&hui::Filename));
	add_ext_var("app_config", TypeHuiConfiguration, hui_p(&hui::Config));
}

};
