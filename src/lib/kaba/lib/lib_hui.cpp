#include "../kaba.h"
#include "../../config.h"
#include "lib.h"
#include "shared.h"
#include "future.h"
#include "../../base/callable.h"

#if __has_include("../../hui/hui.h")
	#include "../../hui/hui.h"
	#include "../../hui/config.h"
	#define KABA_EXPORT_HUI
#elif __has_include("../../hui_minimal/hui.h")
	#include "../../hui_minimal/hui.h"
	#include "../../hui_minimal/config.h"
	#define KABA_EXPORT_HUI_MINIMAL
#else
	#warning("we are screwed.... no hui or hui_minimal")
#endif

#if defined(KABA_EXPORT_HUI_MINIMAL) || defined(KABA_EXPORT_HUI)

namespace hui{
#ifdef KABA_EXPORT_HUI_MINIMAL
	typedef int Menu;
	typedef int Toolbar;
	class Panel : public Sharable<base::Empty> {
	};
	using Window = Panel;
	using Dialog = Panel;
	typedef int Event;
	typedef int Painter;
#endif
#ifdef KABA_EXPORT_HUI
	xfer<hui::Menu> create_menu_from_source(const string &source, hui::Panel*);
#endif
}

namespace kaba {

#ifdef KABA_EXPORT_HUI
	static hui::Event *_event;
	static hui::Panel *_panel;
	#define GetDAPanel(x)			int_p(&_panel->x)-int_p(_panel)
	#define GetDAWindow(x)			int_p(&_win->x)-int_p(_win)
	#define GetDAEvent(x)	int_p(&_event->x)-int_p(_event)

	// capturing all function pointers as pointers or references!!!

	void hui_set_idle_function_kaba(Callable<void()> &c) {
		hui::set_idle_function([&c]{ c(); });
	}
	int hui_run_later_kaba(float dt, Callable<void()> &c) {
		return hui::run_later(dt, [&c]{ c(); });
	}
	int hui_run_repeated_kaba(float dt, Callable<void()> &c) {
		return hui::run_repeated(dt, [&c]{ c(); });
	}
	class KabaPanelWrapper : public hui::Panel {
	public:
		KabaPanelWrapper() : hui::Panel() {}
		KabaPanelWrapper(const string &id, hui::Panel *parent) : hui::Panel(id, parent) {}
		void __init0__() {
			new(this) KabaPanelWrapper();
		}
		void __init2__(const string &id, hui::Panel *parent) {
			new(this) KabaPanelWrapper(id, parent);
		}
		virtual void __delete__() {
			this->KabaPanelWrapper::~KabaPanelWrapper();
		}
		void _kaba_event(const string &id, Callable<void()> &c) {
			event(id, [&c]{ c(); });
		}
		void _kaba_event_x(const string &id, const string &msg, void *f) {
			if (msg == "hui:draw"){
				auto &ff = *(Callable<void(Painter*)>*)f;
				event_xp(id, msg, [&ff](Painter *p){ ff(p); });
			}else{
				auto &ff = *(Callable<void()>*)f;
				event_x(id, msg, [&ff]{ ff(); });
			}
		}
	};
#else
	#define GetDAWindow(x)		0
	#define GetDAEvent(x)	0
	#define GetDAPanel(x) 0
#endif

#ifdef KABA_EXPORT_HUI
	#define hui_p(p)		p
#else
	#define hui_p(p)		nullptr
#endif


extern const Class *TypeObject;
extern const Class *TypeIntList;
extern const Class *TypeStringList;
extern const Class *TypeImage;
extern const Class *TypeBasePainter;
extern const Class *TypePath;
extern const Class *TypeVec2;
extern const Class* TypeCallback;
extern const Class* TypeOsConfiguration;
extern const Class* TypeVoidFuture;
//extern const Class* TypeVoidPromise;
extern const Class* TypeStringFuture;
extern const Class* TypeStringPromise;
extern const Class* TypePathFuture;
extern const Class* TypeBoolFuture;
const Class *TypeHuiWindowP;



void SIAddPackageHui(Context *c) {
	add_internal_package(c, "hui");
	
	auto TypeHuiMenu = add_type("Menu",  sizeof(hui::Menu));
	auto TypeHuiMenuXfer = add_type_p_xfer(TypeHuiMenu);
	auto TypeHuiToolbar = add_type("Toolbar",  sizeof(hui::Toolbar));
	auto TypeHuiToolbarP = add_type_p_raw(TypeHuiToolbar);
	auto TypeHuiPanel = add_type("Panel", sizeof(hui::Panel));
	auto TypeHuiPanelP = add_type_p_raw(TypeHuiPanel); // TODO use ref instead (after owned![X])
	auto TypeHuiPanelXfer = add_type_p_xfer(TypeHuiPanel);
	auto TypeHuiPanelShared = add_type_p_shared(TypeHuiPanel);
	auto TypeHuiWindow = add_type("Window", sizeof(hui::Window));
	TypeHuiWindowP = add_type_p_raw(TypeHuiWindow);
	auto TypeHuiWindowXfer = add_type_p_xfer(TypeHuiWindow);
	auto TypeHuiWindowShared = add_type_p_shared(TypeHuiWindow);
	auto TypeHuiGlWindow = add_type("GlWindow", sizeof(hui::Window));
	auto TypeHuiDialog = add_type("Dialog", sizeof(hui::Window));
	auto TypeHuiEvent = add_type("Event", sizeof(hui::Event));
	auto TypeHuiEventRef = add_type_ref(TypeHuiEvent);
	auto TypeHuiPainter = add_type("Painter", sizeof(hui::Painter));
	auto TypeHuiClipboard = add_type("clipboard", 0);
	const_cast<Class*>(TypeHuiClipboard)->from_template = TypeNamespaceT;

	auto TypeCallbackPainter = add_type_func(TypeVoid, {TypeHuiPainter});

	lib_create_pointer_xfer(TypeHuiMenuXfer);
	lib_create_pointer_xfer(TypeHuiPanelXfer);
	lib_create_pointer_xfer(TypeHuiWindowXfer);

	lib_create_pointer_shared<hui::Panel>(TypeHuiPanelShared, TypeHuiPanelXfer);
	lib_create_pointer_shared<hui::Window>(TypeHuiWindowShared, TypeHuiWindowXfer);

	add_class(TypeHuiMenu);
		class_add_func(Identifier::func::Init, TypeVoid, hui_p(&hui::Menu::__init__), Flags::Mutable);
			func_add_param("p", TypeHuiPanelP);
		class_add_func("popup", TypeVoid, hui_p(&hui::Menu::open_popup));
			func_add_param("p", TypeHuiPanelP);
		class_add_func("add", TypeVoid, hui_p(&hui::Menu::add), Flags::Mutable);
			func_add_param("name", TypeString);
			func_add_param("id", TypeString);
		class_add_func("add_with_image", TypeVoid, hui_p(&hui::Menu::add_with_image), Flags::Mutable);
			func_add_param("name", TypeString);
			func_add_param("image", TypeInt32);
			func_add_param("id", TypeString);
		class_add_func("add_checkable", TypeVoid, hui_p(&hui::Menu::add_checkable), Flags::Mutable);
			func_add_param("name", TypeString);
			func_add_param("id", TypeString);
		class_add_func("add_separator", TypeVoid, hui_p(&hui::Menu::add_separator), Flags::Mutable);
		class_add_func("add_sub_menu", TypeVoid, hui_p(&hui::Menu::add_sub_menu), Flags::Mutable);
			func_add_param("name", TypeString);
			func_add_param("id", TypeString);
			func_add_param("sub_menu", TypeHuiMenuXfer);
		class_add_func("enable", TypeVoid, hui_p(&hui::Menu::enable), Flags::Mutable);
			func_add_param("id", TypeString);
			func_add_param("enabled", TypeBool);
		class_add_func("check", TypeVoid, hui_p(&hui::Menu::check), Flags::Mutable);
			func_add_param("id", TypeString);
			func_add_param("checked", TypeBool);

	add_class(TypeHuiToolbar);
		class_derive_from(TypeObject);
		class_add_func("set_by_id", TypeVoid, hui_p(&hui::Toolbar::set_by_id), Flags::Mutable);
			func_add_param("id", TypeString);
		class_add_func("from_source", TypeVoid, hui_p(&hui::Toolbar::from_source), Flags::Mutable);
			func_add_param("source", TypeString);

	add_class(TypeHuiPanel);
		class_derive_from(TypeObject);
		class_add_element(Identifier::SharedCount, TypeInt32, hui_p(&hui::Panel::_pointer_ref_counter));
		class_add_element("win", TypeHuiWindowP, GetDAPanel(win));
		class_add_func(Identifier::func::Init, TypeVoid, hui_p(&KabaPanelWrapper::__init0__), Flags::Mutable);
		class_add_func(Identifier::func::Init, TypeVoid, hui_p(&KabaPanelWrapper::__init2__), Flags::Mutable);
			func_add_param("parent", TypeHuiPanelP);
			func_add_param("id", TypeString);
		class_add_func_virtual(Identifier::func::Delete, TypeVoid, hui_p(&KabaPanelWrapper::__delete__), Flags::Override | Flags::Mutable);
		class_add_func("set_border_width", TypeVoid, hui_p(&hui::Panel::set_border_width), Flags::Mutable);
			func_add_param("width", TypeInt32);
		class_add_func("set_decimals", TypeVoid, hui_p(&hui::Panel::set_decimals), Flags::Mutable);
			func_add_param("decimals", TypeInt32);
		class_add_func("activate", TypeVoid, hui_p(&hui::Panel::activate), Flags::Mutable);
			func_add_param("id", TypeString);
		class_add_func("is_active", TypeVoid, hui_p(&hui::Panel::is_active));
			func_add_param("id", TypeString);
		class_add_func("from_source", TypeVoid, hui_p(&hui::Panel::from_source), Flags::Mutable);
			func_add_param("source", TypeString);
		class_add_func("add_button", TypeVoid, hui_p(&hui::Panel::add_button), Flags::Mutable);
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt32);
			func_add_param("y", TypeInt32);
			func_add_param("id", TypeString);
		class_add_func("add_toggle_button", TypeVoid, hui_p(&hui::Panel::add_toggle_button), Flags::Mutable);
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt32);
			func_add_param("y", TypeInt32);
			func_add_param("id", TypeString);
		class_add_func("add_check_box", TypeVoid, hui_p(&hui::Panel::add_check_box), Flags::Mutable);
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt32);
			func_add_param("y", TypeInt32);
			func_add_param("id", TypeString);
		class_add_func("add_label", TypeVoid, hui_p(&hui::Panel::add_label), Flags::Mutable);
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt32);
			func_add_param("y", TypeInt32);
			func_add_param("id", TypeString);
		class_add_func("add_edit", TypeVoid, hui_p(&hui::Panel::add_edit), Flags::Mutable);
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt32);
			func_add_param("y", TypeInt32);
			func_add_param("id", TypeString);
		class_add_func("add_multiline_edit", TypeVoid, hui_p(&hui::Panel::add_multiline_edit), Flags::Mutable);
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt32);
			func_add_param("y", TypeInt32);
			func_add_param("id", TypeString);
		class_add_func("add_group", TypeVoid, hui_p(&hui::Panel::add_group), Flags::Mutable);
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt32);
			func_add_param("y", TypeInt32);
			func_add_param("id", TypeString);
		class_add_func("add_combo_box", TypeVoid, hui_p(&hui::Panel::add_combo_box), Flags::Mutable);
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt32);
			func_add_param("y", TypeInt32);
			func_add_param("id", TypeString);
		class_add_func("add_tab_control", TypeVoid, hui_p(&hui::Panel::add_tab_control), Flags::Mutable);
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt32);
			func_add_param("y", TypeInt32);
			func_add_param("id", TypeString);
		class_add_func("set_target", TypeVoid, hui_p(&hui::Panel::set_target), Flags::Mutable);
			func_add_param("id", TypeString);
		class_add_func("add_list_view", TypeVoid, hui_p(&hui::Panel::add_list_view), Flags::Mutable);
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt32);
			func_add_param("y", TypeInt32);
			func_add_param("id", TypeString);
		class_add_func("add_tree_view", TypeVoid, hui_p(&hui::Panel::add_tree_view), Flags::Mutable);
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt32);
			func_add_param("y", TypeInt32);
			func_add_param("id", TypeString);
		class_add_func("add_icon_view", TypeVoid, hui_p(&hui::Panel::add_icon_view), Flags::Mutable);
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt32);
			func_add_param("y", TypeInt32);
			func_add_param("id", TypeString);
		class_add_func("add_progress_bar", TypeVoid, hui_p(&hui::Panel::add_progress_bar), Flags::Mutable);
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt32);
			func_add_param("y", TypeInt32);
			func_add_param("id", TypeString);
		class_add_func("add_slider", TypeVoid, hui_p(&hui::Panel::add_slider), Flags::Mutable);
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt32);
			func_add_param("y", TypeInt32);
			func_add_param("id", TypeString);
		class_add_func("add_drawing_area", TypeVoid, hui_p(&hui::Panel::add_drawing_area), Flags::Mutable);
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt32);
			func_add_param("y", TypeInt32);
			func_add_param("id", TypeString);
		class_add_func("add_grid", TypeVoid, hui_p(&hui::Panel::add_grid), Flags::Mutable);
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt32);
			func_add_param("y", TypeInt32);
			func_add_param("id", TypeString);
		class_add_func("add_spin_button", TypeVoid, hui_p(&hui::Panel::add_spin_button), Flags::Mutable);
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt32);
			func_add_param("y", TypeInt32);
			func_add_param("id", TypeString);
		class_add_func("add_radio_button", TypeVoid, hui_p(&hui::Panel::add_radio_button), Flags::Mutable);
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt32);
			func_add_param("y", TypeInt32);
			func_add_param("id", TypeString);
		class_add_func("add_scroller", TypeVoid, hui_p(&hui::Panel::add_scroller), Flags::Mutable);
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt32);
			func_add_param("y", TypeInt32);
			func_add_param("id", TypeString);
		class_add_func("add_expander", TypeVoid, hui_p(&hui::Panel::add_expander), Flags::Mutable);
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt32);
			func_add_param("y", TypeInt32);
			func_add_param("id", TypeString);
		class_add_func("add_separator", TypeVoid, hui_p(&hui::Panel::add_separator), Flags::Mutable);
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt32);
			func_add_param("y", TypeInt32);
			func_add_param("id", TypeString);
		class_add_func("add_paned", TypeVoid, hui_p(&hui::Panel::add_paned), Flags::Mutable);
			func_add_param("title", TypeString);
			func_add_param("x", TypeInt32);
			func_add_param("y", TypeInt32);
			func_add_param("id", TypeString);
		class_add_func("embed", TypeVoid, hui_p(&hui::Panel::embed), Flags::Mutable);
			func_add_param("panel", TypeHuiPanelShared);
			func_add_param("id", TypeString);
			func_add_param("x", TypeInt32);
			func_add_param("y", TypeInt32);
		class_add_func("unembed", TypeVoid, hui_p(&hui::Panel::unembed), Flags::Mutable);
			func_add_param("panel", TypeHuiPanel);
		class_add_func("set_string", TypeVoid, hui_p(&hui::Panel::set_string), Flags::Mutable);
			func_add_param("id", TypeString);
			func_add_param("s", TypeString);
		class_add_func("add_string", TypeVoid, hui_p(&hui::Panel::add_string), Flags::Mutable);
			func_add_param("id", TypeString);
			func_add_param("s", TypeString);
		class_add_func("get_string", TypeString, hui_p(&hui::Panel::get_string));
			func_add_param("id", TypeString);
		class_add_func("set_float", TypeVoid, hui_p(&hui::Panel::set_float), Flags::Mutable);
			func_add_param("id", TypeString);
			func_add_param("f", TypeFloat32);
		class_add_func("get_float", TypeFloat32, hui_p(&hui::Panel::get_float));
			func_add_param("id", TypeString);
		class_add_func("enable", TypeVoid, hui_p(&hui::Panel::enable), Flags::Mutable);
			func_add_param("id", TypeString);
			func_add_param("enabled", TypeBool);
		class_add_func("is_enabled", TypeBool, hui_p(&hui::Panel::is_enabled));
			func_add_param("id", TypeString);
		class_add_func("check", TypeVoid, hui_p(&hui::Panel::check), Flags::Mutable);
			func_add_param("id", TypeString);
			func_add_param("checked", TypeBool);
		class_add_func("is_checked", TypeBool, hui_p(&hui::Panel::is_checked));
			func_add_param("id", TypeString);
		class_add_func("hide_control", TypeVoid, hui_p(&hui::Panel::hide_control), Flags::Mutable);
			func_add_param("id", TypeString);
			func_add_param("hide", TypeBool);
		class_add_func("delete_control", TypeVoid, hui_p(&hui::Panel::remove_control), Flags::Mutable);
			func_add_param("id", TypeString);
		class_add_func("set_int", TypeVoid, hui_p(&hui::Panel::set_int), Flags::Mutable);
			func_add_param("id", TypeString);
			func_add_param("i", TypeInt32);
		class_add_func("get_int", TypeInt32, hui_p(&hui::Panel::get_int));
			func_add_param("id", TypeString);
		class_add_func("set_color", TypeVoid, hui_p(&hui::Panel::set_color), Flags::Mutable);
			func_add_param("id", TypeString);
			func_add_param("c", TypeColor);
		class_add_func("get_color", TypeColor, hui_p(&hui::Panel::get_color));
			func_add_param("id", TypeString);
		class_add_func("set_selection", TypeVoid, hui_p(&hui::Panel::set_selection), Flags::Mutable);
			func_add_param("id", TypeString);
			func_add_param("sel", TypeIntList);
		class_add_func("get_selection", TypeIntList, hui_p(&hui::Panel::get_selection));
			func_add_param("id", TypeString);
		class_add_func("set_image", TypeVoid, hui_p(&hui::Panel::set_image), Flags::Mutable);
			func_add_param("id", TypeString);
			func_add_param("image", TypeString);
		class_add_func("set_cell", TypeVoid, hui_p(&hui::Panel::set_cell), Flags::Mutable);
			func_add_param("id", TypeString);
			func_add_param("row", TypeInt32);
			func_add_param("column", TypeInt32);
			func_add_param("s", TypeString);
		class_add_func("get_cell", TypeString, hui_p(&hui::Panel::get_cell));
			func_add_param("id", TypeString);
			func_add_param("row", TypeInt32);
			func_add_param("column", TypeInt32);
		class_add_func("set_options", TypeVoid, hui_p(&hui::Panel::set_options), Flags::Mutable);
			func_add_param("id", TypeString);
			func_add_param("options", TypeString);
		class_add_func("reset", TypeVoid, hui_p(&hui::Panel::reset), Flags::Mutable);
			func_add_param("id", TypeString);
		class_add_func("redraw", TypeVoid, hui_p(&hui::Panel::redraw));
			func_add_param("id", TypeString);
		class_add_func("expand", TypeVoid, hui_p(&hui::Panel::expand_row), Flags::Mutable);
			func_add_param("id", TypeString);
			func_add_param("row", TypeInt32);
			func_add_param("expand", TypeBool);
		class_add_func("expand", TypeVoid, hui_p(&hui::Panel::expand), Flags::Mutable);
			func_add_param("id", TypeString);
			func_add_param("expand", TypeBool);
		class_add_func("is_expanded", TypeBool, hui_p(&hui::Panel::is_expanded));
			func_add_param("id", TypeString);
			func_add_param_def("row", TypeInt32, -1);
		class_add_func("event", TypeInt32, hui_p(&KabaPanelWrapper::_kaba_event), Flags::Mutable);
			func_add_param("id", TypeString);
			func_add_param("func", TypeCallback);
		class_add_func("event_x", TypeInt32, hui_p(&KabaPanelWrapper::_kaba_event_x), Flags::Mutable);
			func_add_param("id", TypeString);
			func_add_param("msg", TypeString);
			func_add_param("func", TypeCallback);
		class_add_func("event_x", TypeInt32, hui_p(&KabaPanelWrapper::_kaba_event_x), Flags::Mutable);
			func_add_param("id", TypeString);
			func_add_param("msg", TypeString);
			func_add_param("func", TypeCallbackPainter);
		class_add_func("remove_event_handler", TypeVoid, hui_p(&hui::Panel::remove_event_handler), Flags::Mutable);
			func_add_param("uid", TypeInt32);
#ifdef KABA_EXPORT_HUI
		class_set_vtable(hui::Panel);
#endif


	add_class(TypeHuiWindow);
		class_derive_from(TypeHuiPanel);
		class_add_func(Identifier::func::Init, TypeVoid, hui_p(&hui::Window::__init_ext__), Flags::Mutable);
			func_add_param("title", TypeString);
			func_add_param("width", TypeInt32);
			func_add_param("height", TypeInt32);
		class_add_func_virtual(Identifier::func::Delete, TypeVoid, hui_p(&hui::Window::__delete__), Flags::Override | Flags::Mutable);
		//class_add_func("run", TypeVoid, hui_p(&hui::Window::run));
		class_add_func("destroy", TypeVoid, hui_p(&hui::Window::request_destroy), Flags::Mutable);
		class_add_func("show", TypeVoid, hui_p(&hui::Window::show), Flags::Mutable);
		class_add_func("hide", TypeVoid, hui_p(&hui::Window::hide), Flags::Mutable);

		class_add_func("set_menu", TypeVoid, hui_p(&hui::Window::set_menu), Flags::Mutable);
			func_add_param("menu", TypeHuiMenuXfer);
		class_add_func("toolbar", TypeHuiToolbarP, hui_p(&hui::Window::get_toolbar), Flags::Ref);
			func_add_param("index", TypeInt32);
		class_add_func("set_maximized", TypeVoid, hui_p(&hui::Window::set_maximized), Flags::Mutable);
			func_add_param("max", TypeBool);
		class_add_func("is_maximized", TypeBool, hui_p(&hui::Window::is_maximized));
		class_add_func("is_minimized", TypeBool, hui_p(&hui::Window::is_minimized));
		class_add_func("set_id", TypeVoid, hui_p(&hui::Window::set_id), Flags::Mutable);
			func_add_param("id", TypeInt32);
		class_add_func("set_fullscreen", TypeVoid, hui_p(&hui::Window::set_fullscreen), Flags::Mutable);
			func_add_param("fullscreen",TypeBool);
		class_add_func("set_title", TypeVoid, hui_p(&hui::Window::set_title), Flags::Mutable);
			func_add_param("title", TypeString);
		class_add_func("set_position", TypeVoid, hui_p(&hui::Window::set_position), Flags::Mutable);
			func_add_param("x", TypeInt32);
			func_add_param("y", TypeInt32);
		class_add_func("set_size", TypeVoid, hui_p(&hui::Window::set_size), Flags::Mutable);
			func_add_param("x", TypeInt32);
			func_add_param("y", TypeInt32);
		class_add_func("get_size", TypeVoid, hui_p(&hui::Window::get_size));
			func_add_param("x", TypeInt32, Flags::Out);
			func_add_param("y", TypeInt32, Flags::Out);
	//add_func("setOuterior", TypeVoid, 2, TypePointer,"win",
	//																										TypeIRect,"r");
	//add_func("getOuterior", TypeIRect, 1, TypePointer,"win");
	//add_func("setInerior", TypeVoid, 2, TypePointer,"win",
	//																										TypeIRect,"r");
	//add_func("getInterior", TypeIRect, 1, TypePointer,"win");
		class_add_func("set_cursor_pos", TypeVoid, hui_p(&hui::Window::set_cursor_pos), Flags::Mutable);
			func_add_param("x", TypeInt32);
			func_add_param("y", TypeInt32);
		class_add_func("get_mouse", TypeBool, hui_p(&hui::Window::get_mouse));
			func_add_param("x", TypeInt32, Flags::Out);
			func_add_param("y", TypeInt32, Flags::Out);
			func_add_param("button", TypeInt32);
			func_add_param("change", TypeInt32);
		class_add_func("get_key", TypeBool, hui_p(&hui::Window::get_key));
			func_add_param("key", TypeInt32);
		class_add_func_virtual("on_mouse_move", TypeVoid, hui_p(&hui::Window::on_mouse_move), Flags::Mutable); // const or mutable?!?!?
			func_add_param("pos", TypeVec2);
		class_add_func_virtual("on_mouse_wheel", TypeVoid, hui_p(&hui::Window::on_mouse_wheel), Flags::Mutable);
			func_add_param("d", TypeVec2);
		class_add_func_virtual("on_left_button_down", TypeVoid, hui_p(&hui::Window::on_left_button_down), Flags::Mutable);
			func_add_param("pos", TypeVec2);
		class_add_func_virtual("on_middle_button_down", TypeVoid, hui_p(&hui::Window::on_middle_button_down), Flags::Mutable);
			func_add_param("pos", TypeVec2);
		class_add_func_virtual("on_right_button_down", TypeVoid, hui_p(&hui::Window::on_right_button_down), Flags::Mutable);
			func_add_param("pos", TypeVec2);
		class_add_func_virtual("on_left_button_up", TypeVoid, hui_p(&hui::Window::on_left_button_up), Flags::Mutable);
			func_add_param("pos", TypeVec2);
		class_add_func_virtual("on_middle_button_up", TypeVoid, hui_p(&hui::Window::on_middle_button_up), Flags::Mutable);
			func_add_param("pos", TypeVec2);
		class_add_func_virtual("on_right_button_up", TypeVoid, hui_p(&hui::Window::on_right_button_up), Flags::Mutable);
			func_add_param("pos", TypeVec2);
		class_add_func_virtual("on_double_click", TypeVoid, hui_p(&hui::Window::on_double_click), Flags::Mutable);
			func_add_param("pos", TypeVec2);
		class_add_func_virtual("on_close_request", TypeVoid, hui_p(&hui::Window::on_close_request), Flags::Mutable);
		class_add_func_virtual("on_key_down", TypeVoid, hui_p(&hui::Window::on_key_down), Flags::Mutable);
			func_add_param("key", TypeInt32);
		class_add_func_virtual("on_key_up", TypeVoid, hui_p(&hui::Window::on_key_up), Flags::Mutable);
			func_add_param("key", TypeInt32);
		class_add_func_virtual("on_draw", TypeVoid, hui_p(&hui::Window::on_draw), Flags::Mutable);
			func_add_param("p", TypeHuiPainter);
#ifdef KABA_EXPORT_HUI
		class_set_vtable(hui::Window);
#endif

	add_class(TypeHuiGlWindow);
		class_derive_from(TypeHuiWindow);
		class_add_func(Identifier::func::Init, TypeVoid, hui_p(&hui::NixWindow::__init_ext__), Flags::Mutable);
			func_add_param("title", TypeString);
			func_add_param("width", TypeInt32);
			func_add_param("height", TypeInt32);
		class_add_func_virtual(Identifier::func::Delete, TypeVoid, hui_p(&hui::Window::__delete__), Flags::Override | Flags::Mutable);
#ifdef KABA_EXPORT_HUI
		class_set_vtable(hui::Window);
#endif

	add_class(TypeHuiDialog);
		class_derive_from(TypeHuiWindow);
		class_add_func(Identifier::func::Init, TypeVoid, hui_p(&hui::Dialog::__init_ext__), Flags::Mutable);
			func_add_param("title", TypeString);
			func_add_param("width", TypeInt32);
			func_add_param("height", TypeInt32);
			func_add_param("parent", TypeHuiWindowP);
			func_add_param("allow_parent",TypeBool);
		class_add_func_virtual(Identifier::func::Delete, TypeVoid, hui_p(&hui::Window::__delete__), Flags::Override | Flags::Mutable);
#ifdef KABA_EXPORT_HUI
		class_set_vtable(hui::Window);
#endif

	
	add_class(TypeHuiPainter);
		class_derive_from(TypeBasePainter);

	
	// user interface
	add_func("set_idle_function", TypeVoid, hui_p(&hui_set_idle_function_kaba), Flags::Static);
		func_add_param("idle_func", TypeCallback);
	add_func("run_later", TypeInt32, hui_p(&hui_run_later_kaba), Flags::Static);
		func_add_param("dt", TypeFloat32);
		func_add_param("f", TypeCallback);
	add_func("run_repeated", TypeInt32, hui_p(&hui_run_repeated_kaba), Flags::Static);
		func_add_param("dt", TypeFloat32);
		func_add_param("f", TypeCallback);
	add_func("cancel_runner", TypeVoid, hui_p(&hui::cancel_runner), Flags::Static);
		func_add_param("id", TypeInt32);
	add_func("fly", TypeVoidFuture, hui_p(&hui::fly), Flags::Static);
		func_add_param("win", TypeHuiWindowShared);
	add_func("fly_and_wait", TypeVoid, hui_p(&hui::fly_and_wait), Flags::Static);
		func_add_param("win", TypeHuiWindowShared);
	/*add_func("HuiAddKeyCode", TypeVoid, (void*)&hui::AddKeyCode, Flags::STATIC);
		func_add_param("id", TypeString);
		func_add_param("key_code", TypeInt32);
	add_func("HuiAddCommand", TypeVoid, (void*)&hui::AddCommand, Flags::STATIC);
		func_add_param("id", TypeString);
		func_add_param("image", TypeString);
		func_add_param("key_code", TypeInt32);
		func_add_param("func", TypeFunctionP);*/
	add_func("get_event", TypeHuiEventRef, hui_p(&hui::get_event), Flags::Static);
	add_func("do_single_main_loop", TypeVoid, hui_p(&hui::Application::do_single_main_loop), Flags::Static);
	add_func("file_dialog_open", TypePathFuture, hui_p(&hui::file_dialog_open), Flags::Static);
		func_add_param("root", TypeHuiWindowP);
		func_add_param("title", TypeString);
		func_add_param("dir", TypePath);
		func_add_param("params", TypeStringList);
	add_func("file_dialog_save", TypePathFuture, hui_p(&hui::file_dialog_save), Flags::Static);
		func_add_param("root", TypeHuiWindowP);
		func_add_param("title", TypeString);
		func_add_param("dir", TypePath);
		func_add_param("params", TypeStringList);
	add_func("file_dialog_dir", TypePathFuture, hui_p(&hui::file_dialog_dir), Flags::Static);
		func_add_param("root", TypeHuiWindowP);
		func_add_param("title", TypeString);
		func_add_param("dir", TypePath);
		func_add_param("params", TypeStringList);
	add_func("question_box", TypeBoolFuture, hui_p(&hui::question_box), Flags::Static);
		func_add_param("root", TypeHuiWindowP);
		func_add_param("title", TypeString);
		func_add_param("text", TypeString);
		func_add_param("allow_cancel", TypeBool);
	add_func("info_box", TypeVoid, hui_p(&hui::info_box), Flags::Static);
		func_add_param("root", TypeHuiWindowP);
		func_add_param("title", TypeString);
		func_add_param("text", TypeString);
	add_func("error_box", TypeVoid, hui_p(&hui::error_box), Flags::Static);
		func_add_param("root", TypeHuiWindowP);
		func_add_param("title", TypeString);
		func_add_param("text", TypeString);
	add_func("create_menu_from_source", TypeHuiMenuXfer, hui_p(&hui::create_menu_from_source), Flags::Static);
		func_add_param("source", TypeString);
		func_add_param("panel", TypeHuiPanelP);
	add_func("get_key_name", TypeString, hui_p(&hui::get_key_code_name), Flags::Static | Flags::Pure);
		func_add_param("id", TypeInt32);
//	add_func("get_key_char", TypeString, hui_p(&hui::GetKeyChar), Flags::STATIC | Flags::PURE);
//		func_add_param("id", TypeInt32);

	add_func("open_document", TypeVoid, hui_p(&hui::open_document), Flags::Static);
		func_add_param("filename", TypePath);
	add_func("make_gui_image", TypeString, hui_p(&hui::set_image), Flags::Static);
		func_add_param("image", TypeImage);


	add_class(TypeHuiClipboard);
		class_add_func("paste", TypeStringFuture, hui_p(&hui::clipboard::paste), Flags::Static);
		class_add_func("copy", TypeVoid, hui_p(&hui::clipboard::copy), Flags::Static);
			func_add_param("text", TypeString);


	add_class(TypeHuiEvent);
		class_add_element("id", TypeString, GetDAEvent(id));
		class_add_element("message", TypeString, GetDAEvent(message));
		class_add_element("mouse", TypeVec2, GetDAEvent(m));
		class_add_element("dmouse", TypeVec2, GetDAEvent(d));
		class_add_element("pressure", TypeFloat32, GetDAEvent(pressure));
		class_add_element("scroll", TypeVec2, GetDAEvent(scroll));
		class_add_element("key", TypeInt32, GetDAEvent(key_code));
		class_add_element("width", TypeInt32, GetDAEvent(width));
		class_add_element("height", TypeInt32, GetDAEvent(height));
		class_add_element("button_l", TypeBool, GetDAEvent(lbut));
		class_add_element("button_m", TypeBool, GetDAEvent(mbut));
		class_add_element("button_r", TypeBool, GetDAEvent(rbut));
		class_add_element("row", TypeInt32, GetDAEvent(row));
		class_add_element("column", TypeInt32, GetDAEvent(column));

	// key ids (int)
	add_enum("KEY_CONTROL", TypeInt32, hui::KEY_CONTROL);
	add_enum("KEY_LEFT_CONTROL", TypeInt32, hui::KEY_LCONTROL);
	add_enum("KEY_RIGHT_CONTROL", TypeInt32, hui::KEY_RCONTROL);
	add_enum("KEY_SHIFT", TypeInt32, hui::KEY_SHIFT);
	add_enum("KEY_LEFT_SHIFT", TypeInt32, hui::KEY_LSHIFT);
	add_enum("KEY_RIGHT_SHIFT", TypeInt32, hui::KEY_RSHIFT);
	add_enum("KEY_ALT", TypeInt32, hui::KEY_ALT);
	add_enum("KEY_LEFT_ALT", TypeInt32, hui::KEY_LALT);
	add_enum("KEY_RIGHT_ALT", TypeInt32, hui::KEY_RALT);
	add_enum("KEY_PLUS", TypeInt32, hui::KEY_PLUS);
	add_enum("KEY_MINUS", TypeInt32, hui::KEY_MINUS);
	add_enum("KEY_FENCE", TypeInt32, hui::KEY_FENCE);
	add_enum("KEY_END", TypeInt32, hui::KEY_END);
	add_enum("KEY_PAGE_UP", TypeInt32, hui::KEY_PAGE_UP);
	add_enum("KEY_PAGE_DOWN", TypeInt32, hui::KEY_PAGE_DOWN);
	add_enum("KEY_UP", TypeInt32, hui::KEY_UP);
	add_enum("KEY_DOWN", TypeInt32, hui::KEY_DOWN);
	add_enum("KEY_LEFT", TypeInt32, hui::KEY_LEFT);
	add_enum("KEY_RIGHT", TypeInt32, hui::KEY_RIGHT);
	add_enum("KEY_RETURN", TypeInt32, hui::KEY_RETURN);
	add_enum("KEY_ESCAPE", TypeInt32, hui::KEY_ESCAPE);
	add_enum("KEY_INSERT", TypeInt32, hui::KEY_INSERT);
	add_enum("KEY_DELETE", TypeInt32, hui::KEY_DELETE);
	add_enum("KEY_SPACE", TypeInt32, hui::KEY_SPACE);
	add_enum("KEY_F1", TypeInt32, hui::KEY_F1);
	add_enum("KEY_F2", TypeInt32, hui::KEY_F2);
	add_enum("KEY_F3", TypeInt32, hui::KEY_F3);
	add_enum("KEY_F4", TypeInt32, hui::KEY_F4);
	add_enum("KEY_F5", TypeInt32, hui::KEY_F5);
	add_enum("KEY_F6", TypeInt32, hui::KEY_F6);
	add_enum("KEY_F7", TypeInt32, hui::KEY_F7);
	add_enum("KEY_F8", TypeInt32, hui::KEY_F8);
	add_enum("KEY_F9", TypeInt32, hui::KEY_F9);
	add_enum("KEY_F10", TypeInt32, hui::KEY_F10);
	add_enum("KEY_F11", TypeInt32, hui::KEY_F11);
	add_enum("KEY_F12", TypeInt32, hui::KEY_F12);
	add_enum("KEY_0", TypeInt32, hui::KEY_0);
	add_enum("KEY_1", TypeInt32, hui::KEY_1);
	add_enum("KEY_2", TypeInt32, hui::KEY_2);
	add_enum("KEY_3", TypeInt32, hui::KEY_3);
	add_enum("KEY_4", TypeInt32, hui::KEY_4);
	add_enum("KEY_5", TypeInt32, hui::KEY_5);
	add_enum("KEY_6", TypeInt32, hui::KEY_6);
	add_enum("KEY_7", TypeInt32, hui::KEY_7);
	add_enum("KEY_8", TypeInt32, hui::KEY_8);
	add_enum("KEY_9", TypeInt32, hui::KEY_9);
	add_enum("KEY_A", TypeInt32, hui::KEY_A);
	add_enum("KEY_B", TypeInt32, hui::KEY_B);
	add_enum("KEY_C", TypeInt32, hui::KEY_C);
	add_enum("KEY_D", TypeInt32, hui::KEY_D);
	add_enum("KEY_E", TypeInt32, hui::KEY_E);
	add_enum("KEY_F", TypeInt32, hui::KEY_F);
	add_enum("KEY_G", TypeInt32, hui::KEY_G);
	add_enum("KEY_H", TypeInt32, hui::KEY_H);
	add_enum("KEY_I", TypeInt32, hui::KEY_I);
	add_enum("KEY_J", TypeInt32, hui::KEY_J);
	add_enum("KEY_K", TypeInt32, hui::KEY_K);
	add_enum("KEY_L", TypeInt32, hui::KEY_L);
	add_enum("KEY_M", TypeInt32, hui::KEY_M);
	add_enum("KEY_N", TypeInt32, hui::KEY_N);
	add_enum("KEY_O", TypeInt32, hui::KEY_O);
	add_enum("KEY_P", TypeInt32, hui::KEY_P);
	add_enum("KEY_Q", TypeInt32, hui::KEY_Q);
	add_enum("KEY_R", TypeInt32, hui::KEY_R);
	add_enum("KEY_S", TypeInt32, hui::KEY_S);
	add_enum("KEY_T", TypeInt32, hui::KEY_T);
	add_enum("KEY_U", TypeInt32, hui::KEY_U);
	add_enum("KEY_V", TypeInt32, hui::KEY_V);
	add_enum("KEY_W", TypeInt32, hui::KEY_W);
	add_enum("KEY_X", TypeInt32, hui::KEY_X);
	add_enum("KEY_Y", TypeInt32, hui::KEY_Y);
	add_enum("KEY_Z", TypeInt32, hui::KEY_Z);
	add_enum("KEY_BACKSPACE", TypeInt32, hui::KEY_BACKSPACE);
	add_enum("KEY_TAB", TypeInt32, hui::KEY_TAB);
	add_enum("KEY_HOME", TypeInt32, hui::KEY_HOME);
	add_enum("KEY_NUM_0", TypeInt32, hui::KEY_NUM_0);
	add_enum("KEY_NUM_1", TypeInt32, hui::KEY_NUM_1);
	add_enum("KEY_NUM_2", TypeInt32, hui::KEY_NUM_2);
	add_enum("KEY_NUM_3", TypeInt32, hui::KEY_NUM_3);
	add_enum("KEY_NUM_4", TypeInt32, hui::KEY_NUM_4);
	add_enum("KEY_NUM_5", TypeInt32, hui::KEY_NUM_5);
	add_enum("KEY_NUM_6", TypeInt32, hui::KEY_NUM_6);
	add_enum("KEY_NUM_7", TypeInt32, hui::KEY_NUM_7);
	add_enum("KEY_NUM_8", TypeInt32, hui::KEY_NUM_8);
	add_enum("KEY_NUM_9", TypeInt32, hui::KEY_NUM_9);
	add_enum("KEY_NUM_PLUS", TypeInt32, hui::KEY_NUM_ADD);
	add_enum("KEY_NUM_MINUS", TypeInt32, hui::KEY_NUM_SUBTRACT);
	add_enum("KEY_NUM_MULTIPLY", TypeInt32, hui::KEY_NUM_MULTIPLY);
	add_enum("KEY_NUM_DIVIDE", TypeInt32, hui::KEY_NUM_DIVIDE);
	add_enum("KEY_NUM_COMMA", TypeInt32, hui::KEY_NUM_COMMA);
	add_enum("KEY_NUM_ENTER", TypeInt32, hui::KEY_NUM_ENTER);
	add_enum("KEY_COMMA", TypeInt32, hui::KEY_COMMA);
	add_enum("KEY_DOT", TypeInt32, hui::KEY_DOT);
	add_enum("KEY_LESS", TypeInt32, hui::KEY_LESS);
	add_enum("KEY_SZ", TypeInt32, hui::KEY_SZ);
	add_enum("KEY_AE", TypeInt32, hui::KEY_AE);
	add_enum("KEY_OE", TypeInt32, hui::KEY_OE);
	add_enum("KEY_UE", TypeInt32, hui::KEY_UE);
	add_enum("NUM_KEYS", TypeInt32,hui::NUM_KEYS);
	add_enum("KEY_ANY", TypeInt32, hui::KEY_ANY);

	add_ext_var("app_config", TypeOsConfiguration, hui_p(&hui::config));
}

};

#else
namespace kaba {

	void SIAddPackageHui(Context *c) {
		add_internal_package(c, "hui");
	}

};

#endif
