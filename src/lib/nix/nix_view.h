/*----------------------------------------------------------------------------*\
| Nix view                                                                     |
| -> camera etc...                                                             |
|                                                                              |
| last update: 2010.03.11 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#if HAS_LIB_GL

#ifndef _NIX_VIEW_EXISTS_
#define _NIX_VIEW_EXISTS_

namespace nix{

class Texture;
class DepthBuffer;


void _cdecl SetProjectionPerspective();
void _cdecl SetProjectionPerspectiveExt(float center_x, float center_y, float width_1, float height_1, float z_min, float z_max);
void _cdecl SetProjectionOrtho(bool relative);
void _cdecl SetProjectionOrthoExt(float center_x, float center_y, float map_width, float map_height, float z_min, float z_max);
void _cdecl SetProjectionMatrix(const matrix &mat);

void _cdecl SetWorldMatrix(const matrix &mat);
void _cdecl SetViewMatrix(const matrix &view_mat);

void _cdecl SetViewport(const rect &area);

//bool _cdecl StartFrame();
//bool _cdecl StartFrameIntoTexture(Texture *t);
void _cdecl SetScissor(const rect &r);
//void _cdecl EndFrame();

void _cdecl ScreenShotToImage(Image &image);

extern float view_jitter_x, view_jitter_y;

class FrameBuffer {
public:
	FrameBuffer();
	FrameBuffer(const Array<Texture*> &attachments);
	~FrameBuffer();
	void __init__(const Array<Texture*> &attachments);
	void __delete__();
	Array<Texture*> color_attachments;
	DepthBuffer *depth_buffer;
	unsigned int frame_buffer;
	int width, height;
	rect area() const;

	static FrameBuffer *DEFAULT;
};

void BindFrameBuffer(FrameBuffer *fb);


void StartFrameHui();
void EndFrameHui();
#if HAS_LIB_GLFW
void StartFrameGLFW(void *win);
void EndFrameGLFW(void *win);
#endif

};

#endif

#endif
