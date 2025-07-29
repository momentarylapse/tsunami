#include "hui.h"
#include "config.h"
#include "../kabaexport/KabaExporter.h"
#include "../base/callable.h"


#define KABA_EXPORT_HUI


namespace hui{
#ifdef KABA_EXPORT_HUI
	xfer<hui::Menu> create_menu_from_source(const string &source, hui::Panel*);
#endif
}


#ifdef KABA_EXPORT_HUI

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
	class KabaPainterWrapper : public hui::Painter {
	public:
		virtual void __delete__() {
			this->Painter::~Painter();
		}
	};
#endif

void _dummy() {}


void export_package_hui(kaba::Exporter* e) {

	e->link_class_func("Menu.__init__", &hui::Menu::__init__);
	e->link_class_func("Menu.popup", &hui::Menu::open_popup);
	e->link_class_func("Menu.add", &hui::Menu::add);
	e->link_class_func("Menu.add_with_image", &hui::Menu::add_with_image);
	e->link_class_func("Menu.add_checkable", &hui::Menu::add_checkable);
	e->link_class_func("Menu.add_separator", &hui::Menu::add_separator);
	e->link_class_func("Menu.add_sub_menu", &hui::Menu::add_sub_menu);
	e->link_class_func("Menu.enable", &hui::Menu::enable);
	e->link_class_func("Menu.check", &hui::Menu::check);


	e->link_class_func("Toolbar.set_by_id", &hui::Toolbar::set_by_id);
	e->link_class_func("Toolbar.from_source", &hui::Toolbar::from_source);
	
	
	// SHARED...
	hui::Panel panel;
	e->declare_class_size("Panel", sizeof(hui::Panel));
	e->declare_class_element("Panel.win", &hui::Panel::win);
	e->link_class_func("Panel.__init__:Panel", &KabaPanelWrapper::__init0__);
	e->link_class_func("Panel.__init__:Panel:Panel*:string", &KabaPanelWrapper::__init2__);
	e->link_virtual("Panel.__delete__", &KabaPanelWrapper::__delete__, &panel);
	e->link_class_func("Panel.set_border_width", &hui::Panel::set_border_width);
	e->link_class_func("Panel.set_decimals", &hui::Panel::set_decimals);
	e->link_class_func("Panel.activate", &hui::Panel::activate);
	e->link_class_func("Panel.is_active", &hui::Panel::is_active);
	e->link_class_func("Panel.from_source", &hui::Panel::from_source);
	e->link_class_func("Panel.add_button", &hui::Panel::add_button);
	e->link_class_func("Panel.add_toggle_button", &hui::Panel::add_toggle_button);
	e->link_class_func("Panel.add_check_box", &hui::Panel::add_check_box);
	e->link_class_func("Panel.add_label", &hui::Panel::add_label);
	e->link_class_func("Panel.add_edit", &hui::Panel::add_edit);
	e->link_class_func("Panel.add_multiline_edit", &hui::Panel::add_multiline_edit);
	e->link_class_func("Panel.add_group", &hui::Panel::add_group);
	e->link_class_func("Panel.add_combo_box", &hui::Panel::add_combo_box);
	e->link_class_func("Panel.add_tab_control", &hui::Panel::add_tab_control);
	e->link_class_func("Panel.set_target", &hui::Panel::set_target);
	e->link_class_func("Panel.add_list_view", &hui::Panel::add_list_view);
	e->link_class_func("Panel.add_tree_view", &hui::Panel::add_tree_view);
	e->link_class_func("Panel.add_icon_view", &hui::Panel::add_icon_view);
	e->link_class_func("Panel.add_progress_bar", &hui::Panel::add_progress_bar);
	e->link_class_func("Panel.add_slider", &hui::Panel::add_slider);
	e->link_class_func("Panel.add_drawing_area", &hui::Panel::add_drawing_area);
	e->link_class_func("Panel.add_grid", &hui::Panel::add_grid);
	e->link_class_func("Panel.add_spin_button", &hui::Panel::add_spin_button);
	e->link_class_func("Panel.add_radio_button", &hui::Panel::add_radio_button);
	e->link_class_func("Panel.add_scroller", &hui::Panel::add_scroller);
	e->link_class_func("Panel.add_expander", &hui::Panel::add_expander);
	e->link_class_func("Panel.add_separator", &hui::Panel::add_separator);
	e->link_class_func("Panel.add_paned", &hui::Panel::add_paned);
	e->link_class_func("Panel.embed", &hui::Panel::embed);
	e->link_class_func("Panel.unembed", &hui::Panel::unembed);
	e->link_class_func("Panel.set_string", &hui::Panel::set_string);
	e->link_class_func("Panel.add_string", &hui::Panel::add_string);
	e->link_class_func("Panel.get_string", &hui::Panel::get_string);
	e->link_class_func("Panel.set_float", &hui::Panel::set_float);
	e->link_class_func("Panel.get_float", &hui::Panel::get_float);
	e->link_class_func("Panel.enable", &hui::Panel::enable);
	e->link_class_func("Panel.is_enabled", &hui::Panel::is_enabled);
	e->link_class_func("Panel.check", &hui::Panel::check);
	e->link_class_func("Panel.is_checked", &hui::Panel::is_checked);
	e->link_class_func("Panel.hide_control", &hui::Panel::hide_control);
	e->link_class_func("Panel.delete_control", &hui::Panel::remove_control);
	e->link_class_func("Panel.set_int", &hui::Panel::set_int);
	e->link_class_func("Panel.get_int", &hui::Panel::get_int);
	e->link_class_func("Panel.set_color", &hui::Panel::set_color);
	e->link_class_func("Panel.get_color", &hui::Panel::get_color);
	e->link_class_func("Panel.set_selection", &hui::Panel::set_selection);
	e->link_class_func("Panel.get_selection", &hui::Panel::get_selection);
	e->link_class_func("Panel.set_image", &hui::Panel::set_image);
	e->link_class_func("Panel.set_cell", &hui::Panel::set_cell);
	e->link_class_func("Panel.get_cell", &hui::Panel::get_cell);
	e->link_class_func("Panel.set_options", &hui::Panel::set_options);
	e->link_class_func("Panel.reset", &hui::Panel::reset);
	e->link_class_func("Panel.redraw", &hui::Panel::redraw);
	e->link_class_func("Panel.expand", &hui::Panel::expand_row);
	e->link_class_func("Panel.expand", &hui::Panel::expand);
	e->link_class_func("Panel.is_expanded", &hui::Panel::is_expanded);
	e->link_class_func("Panel.event", &KabaPanelWrapper::_kaba_event);
	e->link_class_func("Panel.event_x", &KabaPanelWrapper::_kaba_event_x);
	e->link_class_func("Panel.event_x", &KabaPanelWrapper::_kaba_event_x);
	e->link_class_func("Panel.remove_event_handler", &hui::Panel::remove_event_handler);


	hui::Window win;
	e->declare_class_size("Window", sizeof(hui::Window));
	e->link_class_func("Window.__init__", &hui::Window::__init_ext__);
	e->link_virtual("Window.__delete__", &hui::Window::__delete__, &win);
	e->link_class_func("Window.destroy", &hui::Window::request_destroy);
	e->link_class_func("Window.show", &hui::Window::show);
	e->link_class_func("Window.hide", &hui::Window::hide);
	e->link_class_func("Window.set_menu", &hui::Window::set_menu);
	e->link_class_func("Window.toolbar", &hui::Window::get_toolbar);
	e->link_class_func("Window.set_maximized", &hui::Window::set_maximized);
	e->link_class_func("Window.is_maximized", &hui::Window::is_maximized);
	e->link_class_func("Window.is_minimized", &hui::Window::is_minimized);
	e->link_class_func("Window.set_id", &hui::Window::set_id);
	e->link_class_func("Window.set_fullscreen", &hui::Window::set_fullscreen);
	e->link_class_func("Window.set_title", &hui::Window::set_title);
	e->link_class_func("Window.set_position", &hui::Window::set_position);
	e->link_class_func("Window.set_size", &hui::Window::set_size);
	e->link_class_func("Window.get_size", &hui::Window::get_size);
	e->link_class_func("Window.set_cursor_pos", &hui::Window::set_cursor_pos);
	e->link_class_func("Window.get_mouse", &hui::Window::get_mouse);
	e->link_class_func("Window.get_key", &hui::Window::get_key);
	e->link_virtual("Window.on_mouse_move", &hui::Window::on_mouse_move, &win);
	e->link_virtual("Window.on_mouse_wheel", &hui::Window::on_mouse_wheel, &win);
	e->link_virtual("Window.on_mouse_enter", &hui::Window::on_mouse_enter, &win);
	e->link_virtual("Window.on_mouse_leave", &hui::Window::on_mouse_leave, &win);
	e->link_virtual("Window.on_left_button_down", &hui::Window::on_left_button_down, &win);
	e->link_virtual("Window.on_middle_button_down", &hui::Window::on_middle_button_down, &win);
	e->link_virtual("Window.on_right_button_down", &hui::Window::on_right_button_down, &win);
	e->link_virtual("Window.on_left_button_up", &hui::Window::on_left_button_up, &win);
	e->link_virtual("Window.on_middle_button_up", &hui::Window::on_middle_button_up, &win);
	e->link_virtual("Window.on_right_button_up", &hui::Window::on_right_button_up, &win);
	e->link_virtual("Window.on_double_click", &hui::Window::on_double_click, &win);
	e->link_virtual("Window.on_close_request", &hui::Window::on_close_request, &win);
	e->link_virtual("Window.on_key_down", &hui::Window::on_key_down, &win);
	e->link_virtual("Window.on_key_up", &hui::Window::on_key_up, &win);
	e->link_virtual("Window.on_draw", &hui::Window::on_draw, &win);


	e->link_class_func("GlWindow.__init__", &hui::NixWindow::__init_ext__);
	e->link_virtual("GlWindow.__delete__", &hui::NixWindow::__delete__, &win);
	
	e->link_class_func("Dialog.__init__", &hui::Dialog::__init_ext__);
	e->link_virtual("Dialog.__delete__", &hui::Dialog::__delete__, &win);
	

	hui::Painter painter(nullptr, nullptr, 0, 0);
	e->link_func("Painter.__init__", &_dummy); // dummy
	e->link_virtual("Painter.__delete__", &KabaPainterWrapper::__delete__, &painter); // dummy

	
	// user interface
	e->link_func("set_idle_function", &hui_set_idle_function_kaba);
	e->link_func("run_later", &hui_run_later_kaba);
	e->link_func("run_repeated", &hui_run_repeated_kaba);
	e->link_func("cancel_runner", &hui::cancel_runner);
	e->link_func("fly", &hui::fly);
	e->link_func("fly_and_wait", &hui::fly_and_wait);
	/*e->link_func("HuiAddKeyCode", &hui::AddKeyCode);
	e->link_func("HuiAddCommand", &hui::AddCommand);*/
	e->link_func("get_event", &hui::get_event);
	e->link_func("do_single_main_loop", &hui::Application::do_single_main_loop);
	e->link_func("file_dialog_open", &hui::file_dialog_open);
	e->link_func("file_dialog_save", &hui::file_dialog_save);
	e->link_func("file_dialog_dir", &hui::file_dialog_dir);
	e->link_func("question_box", &hui::question_box);
	e->link_func("info_box", &hui::info_box);
	e->link_func("error_box", &hui::error_box);
	e->link_func("create_menu_from_source", &hui::create_menu_from_source);
	e->link_func("get_key_name", &hui::get_key_code_name);
//	e->link_func("get_key_char", &hui::GetKeyChar);

	e->link_func("open_document", &hui::open_document);
	e->link_func("make_gui_image", &hui::set_image);


	e->link_func("clipboard.paste", &hui::clipboard::paste);
	e->link_func("clipboard.copy", &hui::clipboard::copy);

	e->declare_class_size("Event", sizeof(hui::Event));
	e->declare_class_element("Event.id", &hui::Event::id);
	e->declare_class_element("Event.message", &hui::Event::message);
	e->declare_class_element("Event.mouse", &hui::Event::m);
	e->declare_class_element("Event.pressure", &hui::Event::pressure);
	e->declare_class_element("Event.scroll", &hui::Event::scroll);
	e->declare_class_element("Event.key", &hui::Event::key_code);
	e->declare_class_element("Event.width", &hui::Event::width);
	e->declare_class_element("Event.height", &hui::Event::height);
	e->declare_class_element("Event.button_l", &hui::Event::lbut);
	e->declare_class_element("Event.button_m", &hui::Event::mbut);
	e->declare_class_element("Event.button_r", &hui::Event::rbut);
	e->declare_class_element("Event.row", &hui::Event::row);
	e->declare_class_element("Event.column", &hui::Event::column);

#if 0
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
#endif

	e->link("app_config", &hui::config);
}


