/*
 * HuiPanel.h
 *
 *  Created on: 18.03.2014
 *      Author: michi
 */

#ifndef HUIPANEL_H_
#define HUIPANEL_H_

#include "Event.h"
#include "../base/pointer.h"

//#include <gtk/gtk.h>

typedef void* gpointer;
typedef struct _GVariant GVariant;
typedef struct _GtkWidget GtkWidget;
typedef struct _GtkSizeGroup GtkSizeGroup;
typedef struct _GAction GAction;
typedef struct _GSimpleAction GSimpleAction;
typedef struct _GSimpleActionGroup GSimpleActionGroup;


//class Painter;
struct rect;

namespace hui
{

class Menu;
class Resource;
//class Painter;
class Event;
class EventListener;
class EventKeyCode;
class Control;
class ControlRadioButton;

class Panel : public Sharable<EventHandler> {
	friend class Control;
	friend class ControlRadioButton;
	friend class Menu;
public:
	Panel(const string &id, Panel *parent);
	Panel();
	~Panel() override;
	void _ClearPanel_();

	void activate(const string &control_id);
	bool is_active(const string &control_id);
	void from_resource(const string &id);
	void set_from_resource(Resource *res);
	void from_source(const string &source);

	void show();
	void hide();

	virtual void on_show(){}
	virtual void on_hide(){}

	void set_id(const string &id);
	void set_parent(Panel *parent);
	void _set_win(Window *win);

	// events
	int event(const string &id, const Callback &function);
	int event_x(const string &id, const string &msg, const Callback &function);
	int event_xp(const string &id, const string &msg, const CallbackP &function);
	void remove_event_handler(int event_handler_id);
	bool _send_event_(Event *e, bool force_if_not_allowed = false);

	// creating controls

	void add_control(const string &type, const string &title, int x, int y, const string &id);
	void _add_control(const string &ns, Resource &cmd, const string &parent_id);
	void add_button(const string &title, int x, int y,const string &id);
	void add_def_button(const string &title, int x, int y, const string &id);
	void add_color_button(const string &title, int x, int y, const string &id);
	void add_toggle_button(const string &title, int x, int y, const string &id);
	void add_check_box(const string &title, int x, int y, const string &id);
	void add_radio_button(const string &title, int x, int y, const string &id);
	void add_label(const string &title, int x, int y, const string &id);
	void add_edit(const string &title, int x, int y, const string &id);
	void add_multiline_edit(const string &title, int x, int y, const string &id);
	void add_group(const string &title, int x, int y, const string &id);
	void add_combo_box(const string &title, int x, int y, const string &id);
	void add_tab_control(const string &title, int x, int y, const string &id);
	void set_target(const string &id);
	void add_list_view(const string &title, int x, int y, const string &id);
	void add_tree_view(const string &title, int x, int y, const string &id);
	void add_icon_view(const string &title, int x, int y, const string &id);
	void add_list_view__test(const string &title, int x, int y, const string &id);
	void add_progress_bar(const string &title, int x, int y, const string &id);
	void add_slider(const string &title, int x, int y, const string &id);
	void add_image(const string &title, int x, int y, const string &id);
	void add_drawing_area(const string &title, int x, int y, const string &id);
	void add_grid(const string &title, int x, int y, const string &id);
	void add_spin_button(const string &title, int x, int y, const string &id);
	void add_scroller(const string &title, int x, int y, const string &id);
	void add_expander(const string &title, int x, int y, const string &id);
	void add_separator(const string &title, int x, int y, const string &id);
	void add_paned(const string &title, int x, int y, const string &id);
	void add_menu_button(const string &title, int x, int y, const string &id);

	void embed_dialog(const string &id, int x, int y);
	void embed_source(const string &source, const string &parent_id, int x, int y);
	void embed_resource(Resource &c, const string &parent_id, int x, int y);
	void _embed_resource(const string &ns, Resource &c, const string &parent_id, int x, int y);
	void embed(shared<Panel> panel, const string &parent_id, int x, int y);
	void unembed(Panel *p);

// using controls
	// string
	void set_string(const string &id, const string &str);
	void add_string(const string &id, const string &str);
	void add_child_string(const string &id, int parent_row, const string &str);
	void change_string(const string &id, int row, const string &str);
	void remove_string(const string &id, int row);
	string get_string(const string &id);
	string get_cell(const string &id, int row, int column);
	void set_cell(const string &id, int row, int column, const string &str);
	// int
	void set_int(const string &id, int i);
	int get_int(const string &id);
	// float
	void set_decimals(int decimals);
	void set_float(const string &id, float f);
	float get_float(const string &id);
	// color
	void set_color(const string &id, const color &col);
	color get_color(const string &id);
	// tree/expander
	void expand(const string &id, bool expand);
	void expand_row(const string &id, int row, bool expand);
	bool is_expanded(const string &id, int row = -1);
	// stuff
	void enable(const string &id, bool enabled);
	bool is_enabled(const string &id);
	void hide_control(const string &id, bool hide);
	void remove_control(const string &id);
	void check(const string &id, bool checked);
	bool is_checked(const string &id);
	void set_image(const string &id, const string &image);
	void set_tooltip(const string &id, const string &tip);
	Array<int> get_selection(const string &id);
	void set_selection(const string &id, const Array<int> &sel);
	void reset(const string &id);
	void set_options(const string &id, const string &options);

	// drawing
	void redraw(const string &id);
	void redraw_rect(const string &_id, const rect &r);
	Control *_get_control_(const string &id);
	Control *_get_control_by_widget_(GtkWidget *widget);
	string _get_id_by_widget_(GtkWidget *widget);
	string _get_cur_id_();
	void _set_cur_id_(const string &id);
	void set_border_width(int width);
	void set_spacing(int width);


	void _connect_menu_to_panel(Menu *menu);

protected:


public:
	struct SizeGroup {
		GtkSizeGroup *group;
		int mode;
		string name;
	};
	Array<SizeGroup> size_groups;
//protected:
	void _insert_control_(shared<Control> c, int x, int y);
	int desired_width, desired_height;

	Control *target_control;
	shared<Control> root_control;
	void apply_foreach(const string &id, std::function<void(Control*)> f);
public:
	Array<EventListener> event_listeners;
	int current_event_listener_uid;
//protected:

	string id;
protected:
	int unique_id;
	string cur_id;
public:
	int _get_unique_id_();
	int num_float_decimals;
	int border_width;
	int spacing;
	Window *win;
	Panel *parent;
	shared_array<Panel> children;


//#if GTK_CHECK_VERSION(4,0,0)
	static void _on_menu_action_(GSimpleAction *simple, GVariant *parameter, gpointer user_data);
	GSimpleActionGroup *action_group = nullptr;
	void _try_add_action_(const string &id, bool checkable);
public:
	GAction *_get_action(const string &id);
//#endif
};

};

#endif /* HUIPANEL_H_ */
