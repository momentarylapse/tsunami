
#ifndef _HUI_INTERNAL_EXISTS_
#define _HUI_INTERNAL_EXISTS_

namespace hui
{

#ifdef OS_WINDOWS
	extern void *hui_win_instance;
#endif
#ifdef HUI_API_WIN
	extern unsigned char HuiKeyID[256];
	extern HFONT hui_win_default_font;
	extern HICON hui_win_main_icon;
#else
	extern int HuiKeyID[256];
	extern void *invisible_cursor;
	extern void *get_gtk_image(const string &image, GtkIconSize size);
	extern void *get_gtk_image_pixbuf(const string &image);
#endif
extern int allow_signal_level;


// images
struct HuiImage {
	int type;
	string filename;
	Image *image;
	void *pix_buf;
};
extern Array<HuiImage> _all_images_;


// window lists...
extern Array<Window*> _all_windows_;

};


#endif
