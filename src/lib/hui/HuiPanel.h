/*
 * HuiPanel.h
 *
 *  Created on: 18.03.2014
 *      Author: michi
 */

#ifndef HUIPANEL_H_
#define HUIPANEL_H_

#include "hui.h"

class HuiMenu;
class HuiResourceNew;
class HuiPainter;
class HuiEvent;
class HuiEventListener;
class HuiControl;
class HuiControlRadioButton;

class HuiPanel : public HuiEventHandler
{
	friend class HuiControl;
	friend class HuiControlRadioButton;
	friend class HuiMenu;
public:
	HuiPanel();
	virtual ~HuiPanel();
	void _cdecl __init__();
	virtual void _cdecl __delete__();
	void _ClearPanel_();

	void _cdecl activate(const string &control_id);
	bool _cdecl isActive(const string &control_id);
	void _cdecl fromResource(const string &id);
	void _cdecl fromSource(const string &source);

	void _cdecl show();
	void _cdecl hide();

	virtual void _cdecl onShow(){}
	virtual void _cdecl onHide(){}

	void set_win(HuiWindow *win);

	// events
	void _cdecl eventS(const string &id, hui_callback *function);
	void _cdecl eventSX(const string &id, const string &msg, hui_callback *function);
	void _cdecl _event(const string &id, HuiEventHandler *handler, void (HuiEventHandler::*function)());
	void _cdecl _eventX(const string &id, const string &msg, HuiEventHandler *handler, void (HuiEventHandler::*function)());
	template<typename T>
	void _cdecl event(const string &id, HuiEventHandler* handler, T fun)
	{	_event(id, handler, (void(HuiEventHandler::*)())fun);	}
	template<typename T>
	void _cdecl eventX(const string &id, const string &msg, HuiEventHandler* handler, T fun)
	{	_eventX(id, msg, handler, (void(HuiEventHandler::*)())fun);	}
	void _cdecl _eventK(const string &id, hui_kaba_callback *function);
	void _cdecl _eventKO(const string &id, HuiEventHandler* handler, hui_kaba_callback *function);
	void _cdecl _eventKX(const string &id, const string &msg, hui_kaba_callback *function);
	void _cdecl _eventKOX(const string &id, const string &msg, HuiEventHandler* handler, hui_kaba_callback *function);
	void removeEventHandlers(HuiEventHandler *handler);
	bool _send_event_(HuiEvent *e);

	// creating controls

	void _cdecl addControl(const string &type, const string &title, int x, int y, int width, int height, const string &id);
	void _cdecl addButton(const string &title,int x,int y,int width,int height,const string &id);
	void _cdecl addDefButton(const string &title,int x,int y,int width,int height,const string &id);
	void _cdecl addColorButton(const string &title,int x,int y,int width,int height,const string &id);
	void _cdecl addToggleButton(const string &title,int x,int y,int width,int height,const string &id);
	void _cdecl addCheckBox(const string &title,int x,int y,int width,int height,const string &id);
	void _cdecl addRadioButton(const string &title,int x,int y,int width,int height,const string &id);
	void _cdecl addText(const string &title,int x,int y,int width,int height,const string &id);
	void _cdecl addEdit(const string &title,int x,int y,int width,int height,const string &id);
	void _cdecl addMultilineEdit(const string &title,int x,int y,int width,int height,const string &id);
	void _cdecl addGroup(const string &title,int x,int y,int width,int height,const string &id);
	void _cdecl addComboBox(const string &title,int x,int y,int width,int height,const string &id);
	void _cdecl addTabControl(const string &title,int x,int y,int width,int height,const string &id);
	void _cdecl setTarget(const string &id, int tab_page);
	void _cdecl addListView(const string &title,int x,int y,int width,int height,const string &id);
	void _cdecl addTreeView(const string &title,int x,int y,int width,int height,const string &id);
	void _cdecl addIconView(const string &title,int x,int y,int width,int height,const string &id);
	void _cdecl addListView_Test(const string &title,int x,int y,int width,int height,const string &id);
	void _cdecl addProgressBar(const string &title,int x,int y,int width,int height,const string &id);
	void _cdecl addSlider(const string &title,int x,int y,int width,int height,const string &id);
	void _cdecl addImage(const string &title,int x,int y,int width,int height,const string &id);
	void _cdecl addDrawingArea(const string &title,int x,int y,int width,int height,const string &id);
	void _cdecl addControlTable(const string &title, int x, int y, int width, int height, const string &id);
	void _cdecl addSpinButton(const string &title, int x, int y, int width, int height, const string &id);
	void _cdecl addScroller(const string &title,int x,int y,int width,int height,const string &id);
	void _cdecl addExpander(const string &title,int x,int y,int width,int height,const string &id);
	void _cdecl addSeparator(const string &title,int x,int y,int width,int height,const string &id);
	void _cdecl addPaned(const string &title,int x,int y,int width,int height,const string &id);

	void _cdecl embedDialog(const string &id, int x, int y);
	void _cdecl embedSource(const string &source, const string &parent_id, int x, int y);
	void embedResource(HuiResourceNew &c, const string &parent_id, int x, int y);
	void _cdecl embed(HuiPanel *panel, const string &parent_id, int x, int y);

// using controls
	// string
	void _cdecl setString(const string &id, const string &str);
	void _cdecl addString(const string &id, const string &str);
	void _cdecl addChildString(const string &id, int parent_row, const string &str);
	void _cdecl changeString(const string &id, int row, const string &str);
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
	void _cdecl setSelection(const string &id, Array<int> &sel);
	void _cdecl reset(const string &id);
	void _cdecl removeControl(const string &id);
	void _cdecl setOptions(const string &id, const string &options);
	void _cdecl setFont(const string &id, const string &font_name);
	void _cdecl setTabSize(const string &id, int tab_size);

	// drawing
	void _cdecl redraw(const string &id);
	void _cdecl redrawRect(const string &_id, int x, int y, int w, int h);
	HuiPainter* _cdecl beginDraw(const string &id);
	HuiControl *_get_control_(const string &id);
#ifdef HUI_API_GTK
	HuiControl *_get_control_by_widget_(GtkWidget *widget);
	string _get_id_by_widget_(GtkWidget *widget);
#endif
	string _get_cur_id_();
	void _set_cur_id_(const string &id);
	void _cdecl setBorderWidth(int width);
	void _cdecl setIndent(int indent);


protected:
	int tab_creation_page;
	bool is_resizable;


#ifdef HUI_API_WIN
#endif
#ifdef HUI_API_GTK
public:
	GtkWidget *plugable;
protected:
	void _insert_control_(HuiControl *c, int x, int y, int width, int height);
	int desired_width, desired_height;
#endif

	Array<HuiControl*> control;
	HuiControl *cur_control;
	HuiControl *root_control;
	Array<HuiEventListener> events;

	string id;
	int unique_id;
	string cur_id;
public:
	int _get_unique_id_();
	int num_float_decimals;
	int border_width;
	int expander_indent;
	HuiWindow *win;
	HuiPanel *parent;
	Array<HuiPanel*> children;
};

#endif /* HUIPANEL_H_ */
