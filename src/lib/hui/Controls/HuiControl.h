/*
 * hui_control.h
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#ifndef HUI_CONTROL_H_
#define HUI_CONTROL_H_

#include "../hui_common.h"
#include "../hui_language.h"
#include "../../image/image.h"

class HuiWindow;
class HuiPanel;

extern int allow_signal_level; // -> hui_window_control.cpp


void GetPartStrings(const string &id, const string &title);
//string ScanOptions(int id, const string &title);
extern Array<string> PartString;
extern string OptionString, HuiFormatString;

class HuiControl
{
public:
	HuiControl(int _type, const string &_id);
	virtual ~HuiControl();
	int type;
	string id;
	int x, y;
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
	bool is_button_bar;
	bool grab_focus;
	HuiPanel *panel;
	HuiControl *parent;
	Array<HuiControl*> children;

	virtual void __reset(){}
	void reset();
	virtual string getString(){ return ""; }
	virtual int getInt(){ return getString()._int(); }
	virtual float getFloat(){ return getString()._float(); }
	virtual color getColor(){ return Black; }
	virtual void __setString(const string &str){}
	void setString(const string &str);
	virtual void __addString(const string &str){}
	void addString(const string &str);
	virtual void __setInt(int i){ __setString(i2s(i)); }
	void setInt(int i);
	virtual void __setFloat(float f){ __setString(f2s(f, 3)); }
	void setFloat(float f);
	virtual void __setColor(const color &c){}
	void setColor(const color &c);
	virtual void setImage(const string &str){}

	virtual void __addChildString(int parent_row, const string &str){}
	void addChildString(int parent_row, const string &str);
	virtual void __changeString(int row, const string &str){}
	void changeString(int row, const string &str);
	virtual string getCell(int row, int column){ return ""; }
	virtual void __setCell(int row, int column, const string &str){}
	void setCell(int row, int column, const string &str);
	virtual Array<int> getMultiSelection(){ Array<int> r; return r; }
	virtual void __setSelection(Array<int> &sel){}
	void setSelection(Array<int> &sel);
	virtual void expand(int row, bool expand){}
	virtual void expandAll(bool expand){}
	virtual bool isExpanded(int row){ return false; }

	virtual void enable(bool enabled);
	virtual bool isEnabled();
	virtual void hide(bool hidden);
	virtual void __check(bool checked){}
	virtual void check(bool checked);
	virtual bool isChecked(){ return false; }
	virtual void setTooltip(const string &str);
	virtual void focus();
	virtual bool hasFocus();
	virtual void completionAdd(const string &text){}
	virtual void completionClear(){}

	virtual void add(HuiControl *child, int x, int y){}
	void setOptions(const string &options);
	virtual void __setOption(const string &op, const string &value){}
	void getSize(int &w, int &h);

	virtual void setFont(const string &font_name);
	virtual void setTabSize(int tab_size){}

	void notify(const string &message = "", bool is_default = true);
};



#endif /* HUI_CONTROL_H_ */
