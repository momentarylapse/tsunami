/*
 * hui_control.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef HUI_CONTROL_H_
#define HUI_CONTROL_H_

#include "../../image/image.h"
#include "../common.h"
#include "../language.h"
#include <functional>

namespace hui
{

class Window;
class Panel;

extern int allow_signal_level; // -> hui_window_control.cpp


Array<string> split_title(const string &title);
string get_option_from_title(const string &title);
bool option_has(const string &options, const string &key);
string option_value(const string &options, const string &key);
bool val_is_positive(const string &val, bool def = false);
Array<std::pair<string, string>> parse_options(const string &options);

class Control {
public:
	Control(int _type, const string &_id);
	virtual ~Control();
	int type;
	string id;

#ifdef HUI_API_WIN
	HWND hWnd, hWnd2;//,hWnd3;
	//Array<HWND> _item_;
	//int color[4]; // ColorButton...
#endif
#ifdef HUI_API_GTK
    GtkWidget *widget;
    GtkWidget *frame;
    GtkWidget *get_frame();
#endif
	bool enabled;
	bool grab_focus;
	int indent;
	Panel *panel;
	Control *parent;
	Array<Control*> children;

	virtual void __reset(){}
	void reset();
	virtual string get_string(){ return ""; }
	virtual int get_int(){ return get_string()._int(); }
	virtual float get_float(){ return get_string()._float(); }
	virtual color get_color(){ return Black; }
	virtual void __set_string(const string &str){}
	void set_string(const string &str);
	virtual void __add_string(const string &str){}
	void add_string(const string &str);
	virtual void __set_int(int i){ __set_string(i2s(i)); }
	void set_int(int i);
	virtual void __set_float(float f){ __set_string(f2s(f, 3)); }
	void set_float(float f);
	virtual void __set_color(const color &c){}
	void set_color(const color &c);
	virtual void set_image(const string &str){}

	virtual void __add_child_string(int parent_row, const string &str){}
	void add_child_string(int parent_row, const string &str);
	virtual void __change_string(int row, const string &str){}
	void change_string(int row, const string &str);
	virtual void __remove_string(int row){}
	void remove_string(int row);
	virtual string get_cell(int row, int column){ return ""; }
	virtual void __set_cell(int row, int column, const string &str){}
	void set_cell(int row, int column, const string &str);
	virtual Array<int> get_selection(){ Array<int> r; return r; }
	virtual void __set_selection(const Array<int> &sel){}
	void set_selection(const Array<int> &sel);
	virtual void expand(int row, bool expand){}
	virtual void expand_all(bool expand){}
	virtual bool is_expanded(int row){ return false; }
	virtual void reveal(bool reveal){}
	virtual bool is_revealed(){ return false; }

	virtual void enable(bool enabled);
	virtual bool is_enabled();
	virtual void hide(bool hidden);
	virtual void __check(bool checked){}
	virtual void check(bool checked);
	virtual bool is_checked(){ return false; }
	virtual void set_tooltip(const string &str);
	virtual void focus();
	virtual bool has_focus();
	virtual void completion_add(const string &text){}
	virtual void completion_clear(){}

	virtual void add(Control *child, int x, int y){}
	void set_options(const string &options);
	virtual void __set_option(const string &op, const string &value){}
	void get_size(int &w, int &h);

	void notify(const string &message = "", bool is_default = true);

	void apply_foreach(const string &id, std::function<void(Control*)> f);

private:
	int min_width, min_height;
};


};

#endif /* HUI_CONTROL_H_ */
