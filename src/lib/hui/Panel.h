/*
 * HuiPanel.h
 *
 *  Created on: 18.03.2014
 *      Author: michi
 */

#ifndef HUIPANEL_H_
#define HUIPANEL_H_

#include "hui.h"


class Painter;
class rect;

namespace hui
{

class Menu;
class Resource;
class Painter;
class Event;
class EventListener;
class EventKeyCode;
class Control;
class ControlRadioButton;

class Panel : public EventHandler
{
	friend class Control;
	friend class ControlRadioButton;
	friend class Menu;
public:
	Panel();
	virtual ~Panel();
	void _cdecl __init__();
	virtual void _cdecl __delete__();
	void _ClearPanel_();

	void _cdecl activate(const string &control_id);
	bool _cdecl isActive(const string &control_id);
	void _cdecl fromResource(const string &id);
	void setFromResource(Resource *res);
	void _cdecl fromSource(const string &source);

	void _cdecl show();
	void _cdecl hide();

	virtual void _cdecl onShow(){}
	virtual void _cdecl onHide(){}

	void set_win(Window *win);

	// events
	int _cdecl event(const string &id, const Callback &function);
	int _cdecl eventX(const string &id, const string &msg, const Callback &function);
	int _cdecl eventXP(const string &id, const string &msg, const CallbackP &function);
	void _cdecl removeEventHandler(int event_handler_id);
	void _cdecl setKeyCode(const string &id, int key_code, const string &image = "");
	bool _send_event_(Event *e, bool force_if_not_allowed = false);

	// kaba wrappers
	int _cdecl _kaba_event(const string &id, kaba_member_callback *function);
	int _cdecl _kaba_eventO(const string &id, EventHandler* handler, kaba_member_callback *function);
	int _cdecl _kaba_eventX(const string &id, const string &msg, kaba_member_callback *function);
	int _cdecl _kaba_eventOX(const string &id, const string &msg, EventHandler* handler, kaba_member_callback *function);

	// creating controls

	void _cdecl addControl(const string &type, const string &title, int x, int y, const string &id);
	void _cdecl _addControl(const string &ns, Resource &cmd, const string &parent_id);
	void _cdecl addButton(const string &title, int x, int y,const string &id);
	void _cdecl addDefButton(const string &title, int x, int y, const string &id);
	void _cdecl addColorButton(const string &title, int x, int y, const string &id);
	void _cdecl addToggleButton(const string &title, int x, int y, const string &id);
	void _cdecl addCheckBox(const string &title, int x, int y, const string &id);
	void _cdecl addRadioButton(const string &title, int x, int y, const string &id);
	void _cdecl addLabel(const string &title, int x, int y, const string &id);
	void _cdecl addEdit(const string &title, int x, int y, const string &id);
	void _cdecl addMultilineEdit(const string &title, int x, int y, const string &id);
	void _cdecl addGroup(const string &title, int x, int y, const string &id);
	void _cdecl addComboBox(const string &title, int x, int y, const string &id);
	void _cdecl addTabControl(const string &title, int x, int y, const string &id);
	void _cdecl setTarget(const string &id);
	void _cdecl addListView(const string &title, int x, int y, const string &id);
	void _cdecl addTreeView(const string &title, int x, int y, const string &id);
	void _cdecl addIconView(const string &title, int x, int y, const string &id);
	void _cdecl addListView_Test(const string &title, int x, int y, const string &id);
	void _cdecl addProgressBar(const string &title, int x, int y, const string &id);
	void _cdecl addSlider(const string &title, int x, int y, const string &id);
	void _cdecl addImage(const string &title, int x, int y, const string &id);
	void _cdecl addDrawingArea(const string &title, int x, int y, const string &id);
	void _cdecl addGrid(const string &title, int x, int y, const string &id);
	void _cdecl addSpinButton(const string &title, int x, int y, const string &id);
	void _cdecl addScroller(const string &title, int x, int y, const string &id);
	void _cdecl addExpander(const string &title, int x, int y, const string &id);
	void _cdecl addSeparator(const string &title, int x, int y, const string &id);
	void _cdecl addPaned(const string &title, int x, int y, const string &id);
	void _cdecl addRevealer(const string &title, int x, int y, const string &id);

	void _cdecl embedDialog(const string &id, int x, int y);
	void _cdecl embedSource(const string &source, const string &parent_id, int x, int y);
	void embedResource(Resource &c, const string &parent_id, int x, int y);
	void _embedResource(const string &ns, Resource &c, const string &parent_id, int x, int y);
	void _cdecl embed(hui::Panel *panel, const string &parent_id, int x, int y);

// using controls
	// string
	void _cdecl setString(const string &id, const string &str);
	void _cdecl addString(const string &id, const string &str);
	void _cdecl addChildString(const string &id, int parent_row, const string &str);
	void _cdecl changeString(const string &id, int row, const string &str);
	void _cdecl removeString(const string &id, int row);
	string _cdecl getString(const string &id);
	string _cdecl getCell(const string &id, int row, int column);
	void _cdecl setCell(const string &id, int row, int column, const string &str);
	// int
	void _cdecl setInt(const string &id, int i);
	int _cdecl getInt(const string &id);
	// float
	void _cdecl setDecimals(int decimals);
	void _cdecl setFloat(const string &id, float f);
	float _cdecl getFloat(const string &id);
	// color
	void _cdecl setColor(const string &id, const color &col);
	color _cdecl getColor(const string &id);
	// tree
	void _cdecl expandAll(const string &id, bool expand);
	void _cdecl expand(const string &id, int row, bool expand);
	bool _cdecl isExpanded(const string &id, int row);
	// revealer
	void _cdecl reveal(const string &id, bool reveal);
	bool _cdecl isRevealed(const string &id);
	// stuff
	void _cdecl enable(const string &id, bool enabled);
	bool _cdecl isEnabled(const string &id);
	void _cdecl hideControl(const string &id, bool hide);
	void _cdecl deleteControl(const string &id);
	void _cdecl check(const string &id, bool checked);
	bool _cdecl isChecked(const string &id);
	void _cdecl setImage(const string &id, const string &image);
	void _cdecl setTooltip(const string &id, const string &tip);
	Array<int> _cdecl getSelection(const string &id);
	void _cdecl setSelection(const string &id, const Array<int> &sel);
	void _cdecl reset(const string &id);
	void _cdecl removeControl(const string &id);
	void _cdecl setOptions(const string &id, const string &options);

	// drawing
	void _cdecl redraw(const string &id);
	void _cdecl redrawRect(const string &_id, const rect &r);
	Control *_get_control_(const string &id);
#ifdef HUI_API_GTK
	Control *_get_control_by_widget_(GtkWidget *widget);
	string _get_id_by_widget_(GtkWidget *widget);
#endif
	string _get_cur_id_();
	void _set_cur_id_(const string &id);
	void _cdecl setBorderWidth(int width);


protected:


#ifdef HUI_API_WIN
#endif
#ifdef HUI_API_GTK
public:
	GtkWidget *plugable;
protected:
	void _insert_control_(Control *c, int x, int y);
	int desired_width, desired_height;
#endif

	Control *cur_control;
	Control *root_control;
	void apply_foreach(const string &id, std::function<void(Control*)> f);
public:
	Array<EventListener> event_listeners;
	Array<EventKeyCode> event_key_codes;
	int current_event_listener_uid;
protected:

	string id;
	int unique_id;
	string cur_id;
public:
	int _get_unique_id_();
	int num_float_decimals;
	int border_width;
	Window *win;
	Panel *parent;
	Array<Panel*> children;
};

};

#endif /* HUIPANEL_H_ */
