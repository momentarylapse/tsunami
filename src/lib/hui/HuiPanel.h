/*
 * HuiPanel.h
 *
 *  Created on: 18.03.2014
 *      Author: michi
 */

#ifndef HUIPANEL_H_
#define HUIPANEL_H_

#include "hui.h"

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
public:
	HuiPanel();
	virtual ~HuiPanel();
	void _cdecl __init__();
	virtual void _cdecl __delete__();
	void _ClearPanel_();

	void _cdecl Activate(const string &control_id);
	bool _cdecl IsActive(const string &control_id);
	void _cdecl FromResource(const string &id);
	void _cdecl FromSource(const string &source);

	void _cdecl Show();
	void _cdecl Hide();

	virtual _cdecl void OnShow(){}
	virtual _cdecl void OnHide(){}

	void set_win(HuiWindow *win);

	// events
	void _cdecl Event(const string &id, hui_callback *function);
	void _cdecl EventX(const string &id, const string &msg, hui_callback *function);
	void _cdecl _EventM(const string &id, HuiEventHandler *handler, void (HuiEventHandler::*function)());
	void _cdecl _EventMX(const string &id, const string &msg, HuiEventHandler *handler, void (HuiEventHandler::*function)());
	template<typename T>
	void _cdecl EventM(const string &id, HuiEventHandler* handler, T fun)
	{	_EventM(id, handler, (void(HuiEventHandler::*)())fun);	}
	template<typename T>
	void _cdecl EventMX(const string &id, const string &msg, HuiEventHandler* handler, T fun)
	{	_EventMX(id, msg, handler, (void(HuiEventHandler::*)())fun);	}
	void _cdecl _EventKM(const string &id, HuiEventHandler* handler, hui_kaba_callback *function);
	void _cdecl _EventKMX(const string &id, const string &msg, HuiEventHandler* handler, hui_kaba_callback *function);
	void RemoveEventHandlers(HuiEventHandler *handler);
	bool _SendEvent_(HuiEvent *e);

	// creating controls

	void _cdecl AddControl(const string &type, const string &title, int x, int y, int width, int height, const string &id);
	void _cdecl AddButton(const string &title,int x,int y,int width,int height,const string &id);
	void _cdecl AddDefButton(const string &title,int x,int y,int width,int height,const string &id);
	void _cdecl AddColorButton(const string &title,int x,int y,int width,int height,const string &id);
	void _cdecl AddToggleButton(const string &title,int x,int y,int width,int height,const string &id);
	void _cdecl AddCheckBox(const string &title,int x,int y,int width,int height,const string &id);
	void _cdecl AddRadioButton(const string &title,int x,int y,int width,int height,const string &id);
	void _cdecl AddText(const string &title,int x,int y,int width,int height,const string &id);
	void _cdecl AddEdit(const string &title,int x,int y,int width,int height,const string &id);
	void _cdecl AddMultilineEdit(const string &title,int x,int y,int width,int height,const string &id);
	void _cdecl AddGroup(const string &title,int x,int y,int width,int height,const string &id);
	void _cdecl AddComboBox(const string &title,int x,int y,int width,int height,const string &id);
	void _cdecl AddTabControl(const string &title,int x,int y,int width,int height,const string &id);
	void _cdecl SetTarget(const string &id, int tab_page);
	void _cdecl AddListView(const string &title,int x,int y,int width,int height,const string &id);
	void _cdecl AddTreeView(const string &title,int x,int y,int width,int height,const string &id);
	void _cdecl AddIconView(const string &title,int x,int y,int width,int height,const string &id);
	void _cdecl AddListView_Test(const string &title,int x,int y,int width,int height,const string &id);
	void _cdecl AddProgressBar(const string &title,int x,int y,int width,int height,const string &id);
	void _cdecl AddSlider(const string &title,int x,int y,int width,int height,const string &id);
	void _cdecl AddImage(const string &title,int x,int y,int width,int height,const string &id);
	void _cdecl AddDrawingArea(const string &title,int x,int y,int width,int height,const string &id);
	void _cdecl AddControlTable(const string &title, int x, int y, int width, int height, const string &id);
	void _cdecl AddSpinButton(const string &title, int x, int y, int width, int height, const string &id);
	void _cdecl AddScroller(const string &title,int x,int y,int width,int height,const string &id);
	void _cdecl AddExpander(const string &title,int x,int y,int width,int height,const string &id);
	void _cdecl AddSeparator(const string &title,int x,int y,int width,int height,const string &id);
	void _cdecl AddPaned(const string &title,int x,int y,int width,int height,const string &id);

	void _cdecl EmbedDialog(const string &id, int x, int y);
	void _cdecl EmbedSource(const string &source, const string &parent_id, int x, int y);
	void EmbedResource(HuiResourceNew &c, const string &parent_id, int x, int y);
	void _cdecl Embed(HuiPanel *panel, const string &parent_id, int x, int y);

// using controls
	// string
	void _cdecl SetString(const string &id, const string &str);
	void _cdecl AddString(const string &id, const string &str);
	void _cdecl AddChildString(const string &id, int parent_row, const string &str);
	void _cdecl ChangeString(const string &id, int row, const string &str);
	string _cdecl GetString(const string &id);
	string _cdecl GetCell(const string &id, int row, int column);
	void _cdecl SetCell(const string &id, int row, int column, const string &str);
	// int
	void _cdecl SetInt(const string &id, int i);
	int _cdecl GetInt(const string &id);
	// float
	void _cdecl SetDecimals(int decimals);
	void _cdecl SetFloat(const string &id, float f);
	float _cdecl GetFloat(const string &id);
	// color
	void _cdecl SetColor(const string &id, const color &col);
	color _cdecl GetColor(const string &id);
	// tree
	void _cdecl ExpandAll(const string &id, bool expand);
	void _cdecl Expand(const string &id, int row, bool expand);
	bool _cdecl IsExpanded(const string &id, int row);
	// stuff
	void _cdecl Enable(const string &id, bool enabled);
	bool _cdecl IsEnabled(const string &id);
	void _cdecl HideControl(const string &id, bool hide);
	void _cdecl DeleteControl(const string &id);
	void _cdecl Check(const string &id, bool checked);
	bool _cdecl IsChecked(const string &id);
	void _cdecl SetImage(const string &id, const string &image);
	void _cdecl SetTooltip(const string &id, const string &tip);
	Array<int> _cdecl GetMultiSelection(const string &id);
	void _cdecl SetMultiSelection(const string &id, Array<int> &sel);
	void _cdecl Reset(const string &id);
	void _cdecl RemoveControl(const string &id);
	void _cdecl SetOptions(const string &id, const string &options);

	// drawing
	void _cdecl Redraw(const string &id);
	void _cdecl RedrawRect(const string &_id, int x, int y, int w, int h);
	HuiPainter* _cdecl BeginDraw(const string &id);
	HuiControl *_GetControl_(const string &id);
#ifdef HUI_API_GTK
	HuiControl *_GetControlByWidget_(GtkWidget *widget);
	string _GetIDByWidget_(GtkWidget *widget);
#endif
	string _GetCurID_();
	void _SetCurID_(const string &id);
	void _cdecl SetBorderWidth(int width);
	void _cdecl SetIndent(int indent);


protected:
	int tab_creation_page;
	bool is_resizable;


#ifdef HUI_API_WIN
#endif
#ifdef HUI_API_GTK
public:
	GtkWidget *plugable;
protected:
	void _InsertControl_(HuiControl *c, int x, int y, int width, int height);
	int desired_width, desired_height;
#endif

	Array<HuiControl*> control;
	HuiControl *cur_control;
	HuiControl *root_control;
	Array<HuiEventListener> event;

	string id;
	int unique_id;
	string cur_id;
public:
	int _GetUniqueID_();
	int num_float_decimals;
	int border_width;
	int expander_indent;
	HuiWindow *win;
	HuiPanel *parent;
	Array<HuiPanel*> children;
};

#endif /* HUIPANEL_H_ */
