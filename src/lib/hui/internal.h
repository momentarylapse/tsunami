
#ifndef _HUI_INTERNAL_EXISTS_
#define _HUI_INTERNAL_EXISTS_

class Configuration;
class Image;

namespace hui
{

#ifdef OS_WINDOWS
	extern void *hui_win_instance;
#endif
extern int HuiKeyID[256];
extern void *invisible_cursor;
extern void *get_gtk_image(const string &image, IconSize size);
extern void* get_gtk_image_paintable(const string &image);
extern int allow_signal_level;


// images
struct HuiImage {
	int type;
	string filename;
	Image* image;
	void* pix_buf = nullptr;
	void* paintable = nullptr;
};
extern Array<HuiImage> _all_images_;


// window lists...
extern Array<Window*> _all_windows_;


extern Configuration config;

};


#endif
