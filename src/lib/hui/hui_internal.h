
#ifndef _HUI_INTERNAL_EXISTS_
#define _HUI_INTERNAL_EXISTS_



#ifdef OS_WINDOWS
	extern HINSTANCE hui_win_instance;
#endif
#ifdef HUI_API_WIN
	extern unsigned char HuiKeyID[256];
	extern HFONT hui_win_default_font;
	extern HICON hui_win_main_icon;
#else
	extern int HuiKeyID[256];
	extern void *invisible_cursor;
	extern const char *get_stock_id(int image);
	extern void *get_gtk_image(const string &image, bool large);
	extern void *get_gtk_image_pixbuf(const string &image);
#endif
extern int allow_signal_level;


// images
struct sHuiImage
{
	int type;
	string filename;
	Image image;
};
extern Array<sHuiImage> HuiImage;


// window lists...
extern Array<CHuiWindow*> HuiWindow;
struct HuiClosedWindow
{
	CHuiWindow *win;
	int unique_id;
	string last_id;
};
extern Array<HuiClosedWindow> _HuiClosedWindow_;




#endif
