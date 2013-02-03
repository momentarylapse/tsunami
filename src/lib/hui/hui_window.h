/*----------------------------------------------------------------------------*\
| Hui window                                                                   |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2009.12.05 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#ifndef _HUI_WINDOW_EXISTS_
#define _HUI_WINDOW_EXISTS_

//#include "hui_common.h"

class CHuiMenu;
class HuiEvent;
class CHuiWindow;


struct HuiCompleteWindowMessage
{
	#ifdef HUI_API_WIN
		unsigned int msg,wparam,lparam;
	#endif
};


// user input
struct HuiInputData
{
	// mouse
	float x, y, dx, dy, dz;	// position, change
	float area_x, area_y;
	float mw;					// drection
	bool lb,mb,rb;				// buttons
	int row, column;
	// keyboard
	bool key[256];
	int key_code;
	int KeyBufferDepth;
	int KeyBuffer[HUI_MAX_KEYBUFFER_DEPTH];
	void reset()
	{	memset(this, 0, sizeof(HuiInputData));	}
};

struct irect
{
public:
	int x1,y1,x2,y2;
	irect(){};
	irect(int x1,int x2,int y1,int y2)
	{	this->x1=x1;	this->x2=x2;	this->y1=y1;	this->y2=y2;	}
};


struct HuiControl
{
	int type;
	string id;
	int x, y;
#ifdef HUI_API_WIN
	HWND hWnd,hWnd2,hWnd3;
	Array<HWND> _item_;
	int color[4]; // ColorButton...
#endif
#ifdef HUI_API_GTK
    GtkWidget *widget;
	int selected;
	Array<GtkTreeIter> _item_;
#endif
	bool enabled;
	bool is_button_bar;
	CHuiWindow *win;
};

struct HuiToolbarItem
{
	int type;
	string id;
#ifdef HUI_API_GTK
	GtkToolItem *widget;
#endif
	bool enabled;
	CHuiMenu *menu;
};

struct HuiToolbar
{
	Array<HuiToolbarItem> item;
	bool enabled;
	bool text_enabled;
	bool large_icons;
#ifdef HUI_API_WIN
	HWND hWnd;
#endif
#ifdef HUI_API_GTK
	GtkWidget *widget;
#endif
};

class HuiEventHandler
{
public:
};

struct HuiWinEvent
{
	string id, message;
	hui_callback *function;
	void (HuiEventHandler::*member_function)();
	HuiEventHandler *object;
};

class HuiDrawingContext
{
	public:
#ifdef HUI_API_GTK
	cairo_t *cr;
#endif
	CHuiWindow *win;
	string id;
	void _cdecl End();
	color _cdecl GetThemeColor(int i);
	void _cdecl SetColor(const color &c);
	void _cdecl SetFont(const string &font, float size, bool bold, bool italic);
	void _cdecl SetFontSize(float size);
	void _cdecl SetAntialiasing(bool enabled);
	void _cdecl SetLineWidth(float w);
	void _cdecl DrawPoint(float x, float y);
	void _cdecl DrawLine(float x1, float y1, float x2, float y2);
	void _cdecl DrawLines(float *x, float *y, int num_lines);
	void _cdecl DrawLinesMA(Array<float> &x, Array<float> &y);
	void _cdecl DrawPolygon(float *x, float *y, int num_points);
	void _cdecl DrawPolygonMA(Array<float> &x, Array<float> &y);
	void _cdecl DrawRect(float x1, float y1, float w, float h);
	void _cdecl DrawRect(const rect &r);
	void _cdecl DrawCircle(float x, float y, float radius);
	void _cdecl DrawStr(float x, float y, const string &str);
	float _cdecl GetStrWidth(const string &str);
	void _cdecl DrawImage(float x, float y, const Image &image);
	int width, height;
};

class CHuiWindow : public HuiEventHandler
{
public:
	CHuiWindow(const string &title, int x, int y, int width, int height, CHuiWindow *parent, bool allow_parent, int mode, bool show);
	virtual ~CHuiWindow();
	void _Init_(CHuiWindow *parent, bool allow_parent, int mode);
	void _CleanUp_();

	// the window
	void _cdecl Update();
	void _cdecl Hide(bool hide);
	void _cdecl SetMaximized(bool maximized);
	bool _cdecl IsMaximized();
	bool _cdecl IsMinimized();
	void _cdecl SetID(const string &id);
	void _cdecl SetFullscreen(bool fullscreen);
	void _cdecl SetTitle(const string &title);
	void _cdecl SetPosition(int x, int y);
	void _cdecl SetPositionSpecial(CHuiWindow *win, int mode);
	void _cdecl SetSize(int width, int height);
	void _cdecl SetOuterior(irect rect);
	irect _cdecl GetOuterior();
	void _cdecl SetOuteriorDesired(irect rect);
	irect _cdecl GetOuteriorDesired();
	irect GetInterior();
	void _cdecl Activate(const string &control_id = "");
	bool _cdecl IsActive(bool include_sub_windows=false);
	void _cdecl SetMenu(CHuiMenu *menu);
	CHuiMenu *GetMenu();
	void _cdecl SetBorderWidth(int width);
	CHuiWindow *GetParent();
	void FromResource(const string &id);


	void _cdecl SetCursorPos(int x,int y);
	void _cdecl ShowCursor(bool show);

	// tool bars
	void _cdecl EnableStatusbar(bool enabled);
	//bool _cdecl IsStatusbarEnabled();
	void _cdecl SetStatusText(const string &str);
	void _cdecl EnableToolbar(bool enabled);
	void _cdecl ToolbarSetCurrent(int index);
	void _cdecl ToolbarConfigure(bool text_enabled, bool large_icons);
	void _cdecl ToolbarAddItem(const string &title, const string &tool_tip, const string &image, const string &id);
	void _cdecl ToolbarAddItemCheckable(const string &title, const string &tool_tip, const string &image, const string &id);
	void _cdecl ToolbarAddItemMenu(const string &title, const string &tool_tip, const string &image, CHuiMenu *menu, const string &id);
	void _cdecl ToolbarAddItemMenuByID(const string &title, const string &tool_tip, const string &image, const string &menu_id, const string &id);
	void _cdecl ToolbarAddSeparator();
	void _cdecl ToolbarReset();
	void _cdecl ToolbarSetByID(const string &id);
	// (internal)
	void _ToolbarEnable_(const string &id, bool enabled);
	bool _ToolbarIsEnabled_(const string &id);
	void _ToolbarCheck_(const string &id, bool checked);
	bool _ToolbarIsChecked_(const string &id);

	// events
	void _cdecl AllowEvents(const string &msg);
	void _cdecl Event(const string &id, hui_callback *function);
	void _cdecl EventX(const string &id, const string &msg, hui_callback *function);
	void _EventM(const string &id, HuiEventHandler *handler, void (HuiEventHandler::*function)());
	void _EventMX(const string &id, const string &msg, HuiEventHandler *handler, void (HuiEventHandler::*function)());
	template<typename T>
	void EventM(const string &id, HuiEventHandler* handler, T fun)
	{	_EventM(id, handler, (void(HuiEventHandler::*)())fun);	}
	template<typename T>
	void EventMX(const string &id, const string &msg, HuiEventHandler* handler, T fun)
	{	_EventMX(id, msg, handler, (void(HuiEventHandler::*)())fun);	}
	bool _SendEvent_(HuiEvent *e);

	// creating controls
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

	void _cdecl EmbedDialog(const string &id, int x, int y);

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
	void _cdecl Check(const string &id, bool checked);
	bool _cdecl IsChecked(const string &id);
	void _cdecl SetImage(const string &id, const string &image);
	void _cdecl SetTooltip(const string &id, const string &tip);
	Array<int> _cdecl GetMultiSelection(const string &id);
	void _cdecl SetMultiSelection(const string &id, Array<int> &sel);
	void _cdecl Reset(const string &id);
	void _cdecl RemoveControl(const string &id);

	// edit completion
	void CompletionAdd(const string &id, const string &text);
	void CompletionClear(const string &id);

	// drawing
	void _cdecl Redraw(const string &id);
	void _cdecl RedrawRect(const string &_id, int x, int y, int w, int h);
	HuiDrawingContext _cdecl *BeginDraw(const string &id);

	// input
	bool GetKey(int key);
	bool GetMouse(int &x, int &y, int button);


	// hui internal
	int _GetMainLevel_();
	int _GetUniqueID_();
	HuiControl *_GetControl_(const string &id);
#ifdef HUI_API_GTK
	HuiControl *_GetControlByWidget_(GtkWidget *widget);
	string _GetIDByWidget_(GtkWidget *widget);
#endif
	string _GetCurID_();
	void _SetCurID_(const string &id);
	bool allow_input;
	HuiInputData input;
	int mouse_offset_x, mouse_offset_y;

private:
	int tab_creation_page;


#ifdef OS_WINDOWS
public:
	HWND hWnd;
private:
#endif
#ifdef HUI_API_WIN
	bool ready;
	//hui_callback *NixGetInputFromWindow;
	HWND statusbar, gl_hwnd;
	RECT WindowBounds,WindowClient;
	DWORD WindowStyle;
	int cdx,cdy;
#endif
#ifdef HUI_API_GTK
public:
	GtkWidget *window;
	GtkWidget *gl_widget;
	GtkWidget *plugable;
private:
	GtkWidget *vbox, *hbox, *menubar, *statusbar, *__ttt__, *input_widget;
	Array<GtkWidget*> gtk_menu;
	int gtk_num_menus;
	HuiControl *_InsertControl_(GtkWidget *widget, int x, int y, int width, int height, const string &id, int type, GtkWidget *frame = NULL);
#endif
	
	int num_float_decimals;
	bool used_by_nix;
	bool is_resizable;
	int border_width;
	Array<HuiControl*> control;
	HuiControl *cur_control;
	Array<HuiWinEvent> event;
	HuiToolbar toolbar[4], *cur_toolbar;
	CHuiMenu *menu, *popup;
	bool statusbar_enabled;
	bool allowed, allow_keys;
	CHuiWindow *parent, *terror_child;
	Array<CHuiWindow*> sub_window;

	int unique_id;
	string id;
	bool is_hidden;
	int main_level;
	string cur_id;

	//HuiCompleteWindowMessage CompleteWindowMessage;
};

extern CHuiWindow *HuiCurWindow;

void _cdecl HuiWindowAddControl(CHuiWindow *win, const string &type, const string &title, int x, int y, int width, int height, const string &id);

CHuiWindow *_cdecl HuiCreateWindow(const string &title, int x, int y, int width, int height);
CHuiWindow *_cdecl HuiCreateNixWindow(const string &title, int x, int y, int width, int height);
CHuiWindow *_cdecl HuiCreateControlWindow(const string &title, int x, int y, int width, int height);
CHuiWindow *_cdecl HuiCreateDialog(const string &title, int width, int height, CHuiWindow *root, bool allow_root);
CHuiWindow *_cdecl HuiCreateSizableDialog(const string &title, int width, int height, CHuiWindow *root, bool allow_root);
void _cdecl HuiCloseWindow(CHuiWindow *win);

void HuiFuncIgnore();
void HuiFuncClose();

enum{
	HuiWinModeResizable = 1,
	HuiWinModeNoFrame = 2,
	HuiWinModeNoTitle = 4,
	HuiWinModeControls = 8,
//	HuiWinModeDummy = 16,
	HuiWinModeNix = 32,
};

#define HuiLeft		1
#define HuiRight	2
#define HuiTop		4
#define HuiBottom	8


// tool bar items
enum{
	HuiToolButton,
	HuiToolCheckable,
	HuiToolSeparator,
	HuiToolMenu
};

// which one of the toolbars?
enum{
	HuiToolbarTop,
	HuiToolbarBottom,
	HuiToolbarLeft,
	HuiToolbarRight
};


#endif
