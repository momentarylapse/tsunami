/*----------------------------------------------------------------------------*\
| Nix view                                                                     |
| -> camera etc...                                                             |
|                                                                              |
| last update: 2010.03.11 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#if HAS_LIB_GL


#include "nix.h"
#include "nix_common.h"
#ifdef _X_USE_IMAGE_
#include "../image/image.h"
#endif

namespace nix{

void TestGLError(const char*);


matrix view_matrix, projection_matrix;
matrix world_matrix, world_view_projection_matrix;

Texture *RenderingToTexture = NULL;

#ifdef OS_WINDOWS
	extern HDC hDC;
	extern HGLRC hRC;
#endif


FrameBuffer *FrameBuffer::DEFAULT = new FrameBuffer();

FrameBuffer::FrameBuffer() {
	depth_buffer = nullptr;
	width = height = 0;

	frame_buffer = 0;
}

FrameBuffer::FrameBuffer(const Array<Texture*> &attachments) {
	depth_buffer = nullptr;

	for (auto *a: attachments) {
		if (a->type == a->Type::DEPTH)
			depth_buffer = (DepthBuffer*)a;
		else
			color_attachments.add(a);
		width = a->width;
		height = a->height;
	}

	glGenFramebuffers(1, &frame_buffer);
	TestGLError("FrameBuffer: glGenFramebuffers");
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
	TestGLError("FrameBuffer: glBindFramebuffer");



	if (depth_buffer) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_buffer->texture, 0);
		TestGLError("FrameBuffer: glFramebufferTexture2D");
		glDrawBuffer(GL_NONE);
		TestGLError("DepthTexture: glDrawBuffer");
		glReadBuffer(GL_NONE);
		TestGLError("DepthTexture: glReadBuffer");
	}

	foreachi (Texture *t, color_attachments, i) {

		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, t->texture, 0);
		TestGLError("FrameBuffer: glFramebufferTexture");
		GLenum draw_buffers[1] = {GL_COLOR_ATTACHMENT0 + (unsigned)i};
		glDrawBuffers(1, draw_buffers);
		TestGLError("FrameBuffer: glDrawBuffers");
	}


	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		msg_error("FrameBuffer: framebuffer != complete");

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	TestGLError("FrameBuffer: glBindFramebuffer(0)");
}

FrameBuffer::~FrameBuffer() {
	glDeleteFramebuffers(1, &frame_buffer);
}

void FrameBuffer::__init__(const Array<Texture*> &attachments) {
	new(this) FrameBuffer(attachments);
}

void FrameBuffer::__delete__() {
	this->~FrameBuffer();
}

rect FrameBuffer::area() const {
	return rect(0, width, 0, height);
}

void BindFrameBuffer(FrameBuffer *fb) {
	glBindFramebuffer(GL_FRAMEBUFFER, fb->frame_buffer);
	TestGLError("BindFrameBuffer: glBindFramebuffer()");

	SetViewport(fb->area());
}



matrix create_pixel_projection_matrix() {
	auto t = matrix::translation(vector(-float(target_width)/2.0f,-float(target_height)/2.0f,-0.5f));
	auto s = matrix::scale(2.0f / float(target_width), -2.0f / float(target_height), 2);
	return s * t;
}

void SetViewport(const rect &area) {
	target_rect = area;
	target_width = max((int)area.width(), 1);
	target_height = max((int)area.height(), 1);

	// screen
	glViewport(area.x1, area.y1, area.width(), area.height());
	TestGLError("glViewport");
}

void SetWorldMatrix(const matrix &mat) {
	world_matrix = mat;
	world_view_projection_matrix = projection_matrix * view_matrix * world_matrix;
}


// 3D-Matrizen erstellen (Einstellungen ueber SetPerspectiveMode vor NixStart() zu treffen)
// enable3d: true  -> 3D-Ansicht auf (View3DWidth,View3DHeight) gemapt
//           false -> Pixel-Angaben~~~
// beide Bilder sind um View3DCenterX,View3DCenterY (3D als Fluchtpunkt) verschoben

void SetProjectionPerspective() {
	SetProjectionPerspectiveExt((float)target_width / 2, (float)target_height / 2, (float)target_height, (float)target_height, 0.001f, 10000);
}

// center_x/y: pixel coordinates of perspective center
// height_1/width_1: pixel sizes of 45° frustrum
void SetProjectionPerspectiveExt(float center_x, float center_y, float width_1, float height_1, float z_min, float z_max) {
	// perspective projection
	auto t = matrix::translation(
		vector(center_x / float(target_width) * 2.0f - 1,
			1 - center_y / float(target_height) * 2.0f,
			0));
	auto p = matrix::perspective(pi / 2, 1, z_min, z_max);
	auto s = matrix::scale(2 * width_1 / target_width,
			2 * height_1 / target_height,
			- 1); // z reflection: right/left handedness

	SetProjectionMatrix(t * p * s);
}

// center_x/y: pixel coordinates of (0,0,0)
// map_width/height: pixel sizes of projected base vectors
void SetProjectionOrthoExt(float center_x, float center_y, float map_width, float map_height, float z_min, float z_max) {
	auto scale = matrix::scale(2.0f / float(target_width) * map_width, -2.0f / float(target_height) * map_height, 2 / (z_max - z_min));
	auto trans = matrix::translation(vector(2 * center_x / target_width - 1, 1 - 2 * center_y / target_height, -(z_max + z_min) / (z_max - z_min)));
	SetProjectionMatrix(trans * scale);
}

void SetProjectionOrtho(bool relative) {
	matrix m;
	if (relative) {
		// orthogonal projection (relative [0,1]x[0x1] coordinates)
		auto t = matrix::translation(vector(-0.5f, -0.5f, 0));
		auto s = matrix::scale(2.0f, -2.0f, 1);
		m = s * t;
	} else {
		// orthogonal projection (pixel coordinates)
		//NixSetProjectionOrthoExt(0, 0, 1, 1, )
		m = create_pixel_projection_matrix();
	}

	SetProjectionMatrix(m);
}

void SetProjectionMatrix(const matrix &m) {
	projection_matrix = m;
	world_view_projection_matrix = projection_matrix * view_matrix * world_matrix;
}

void SetViewMatrix(const matrix &m) {
	view_matrix = m;
}


#ifdef OS_WINDOWS
	#ifdef HUI_API_GTK
		#include <gdk/gdkwin32.h>
	#endif
	extern HWND hWndSubWindow;
	extern bool nixDevNeedsUpdate;
#endif


#if 0
//bool StartFrame() {
//	return StartFrameIntoTexture(NULL);
//}

bool StartFrameIntoTexture(Texture *texture) {
	TestGLError("Start prae");


#ifdef OS_WINDOWS
	if (nixDevNeedsUpdate){
		wglDeleteContext(hRC);
	PIXELFORMATDESCRIPTOR pfd={	sizeof(PIXELFORMATDESCRIPTOR),
								1,						// versions nummer
								PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
								PFD_TYPE_RGBA,
								32,//NixFullscreen?depth:NixDesktopDepth,
								//8, 0, 8, 8, 8, 16, 8, 24,
								0, 0, 0, 0, 0, 0, 0, 0, 0,
								0, 0, 0, 0,
								24,						// 24bit Z-Buffer
								8,						// 8bit stencil buffer
								0,						// no "Auxiliary"-buffer
								PFD_MAIN_PLANE,
								0, 0, 0, 0 };
		GtkWidget *gl_widget = NixWindow->_get_control_(NixControlID)->widget;
	
		hDC = GetDC((HWND)GDK_WINDOW_HWND(gtk_widget_get_window(gl_widget)));
		//hDC = GetDC(hWndSubWindow);
		if (!hDC){
			HuiErrorBox(NixWindow, "Fehler", "GetDC..." + i2s(GetLastError()));
			exit(0);
		}
		int OGLPixelFormat = ChoosePixelFormat(hDC, &pfd);
		SetPixelFormat(hDC, OGLPixelFormat, &pfd);
		hRC=wglCreateContext(hDC);
		if (!hRC){
			HuiErrorBox(NixWindow, "Fehler", "wglCreateContext...");
			exit(0);
		}
		int rr=wglMakeCurrent(hDC, hRC);
		if (rr != 1){
			HuiErrorBox(NixWindow, "Fehler", "wglMakeCurrent...");
			exit(0);
		}
		NixSetCull(CullDefault);
		nixDevNeedsUpdate = false;
	}
#endif

	RenderingToTexture=texture;
	//msg_write("Start " + p2s(texture));
	if (!texture){
		#ifdef OS_WINDOWS
	//		if (OGLDynamicTextureSupport)
	//			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
			/*if (!wglMakeCurrent(hDC,hRC)){
				msg_error("wglMakeCurrent");
				msg_write(GetLastError());
				return false;
			}*/
		#endif

	}else{

		glBindFramebuffer(GL_FRAMEBUFFER, texture->frame_buffer);
		//glBindRenderbufferEXT( GL_RENDERBUFFER_EXT, texture->glDepthRenderBuffer );
		/*glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, texture->glTexture, 0 );
		glFramebufferRenderbufferEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, texture->glDepthRenderBuffer );
		GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
		if (status == GL_FRAMEBUFFER_COMPLETE_EXT){
			//msg_write("hurra");
		}else{
			msg_write("we're screwed! (NixStart with dynamic texture target)");
			return false;
		}*/
	}
	TestGLError("Start 1");
	glClearColor(0.0f,0.0f,0.0f,0.0f);
	glDisable(GL_SCISSOR_TEST);
	//glClearStencil(0);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glClear(GL_COLOR_BUFFER_BIT);
	TestGLError("Start 2a");
	glClear(GL_DEPTH_BUFFER_BIT);
	TestGLError("Start 2b");
	glClear(GL_STENCIL_BUFFER_BIT);
	//glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	//glClear(GL_COLOR_BUFFER_BIT);
	TestGLError("Start 2");

	// adjust target size
	if (!texture){
		auto *e = hui::GetEvent();
		SetViewport(e->column, e->row);
	}else{
		// texture
		SetViewport(texture->width, texture->height);
	}

	/*if (texture < 0)
		NixUpdateInput();*/

	//msg_write("-ok?");
	TestGLError("Start post");
	return true;
}
#endif

void SetScissor(const rect &_r)
{
	bool enable_scissors = true;
	rect r = _r;
	if (r.x1 < 0){
		enable_scissors=false;
		r = target_rect;
	}
	if (enable_scissors)
		glEnable(GL_SCISSOR_TEST);
	else
		glDisable(GL_SCISSOR_TEST);
	glScissor((int)r.x1, target_height - (int)r.y2, (int)r.width(), (int)r.height());
	glClearDepth(1.0f);
	TestGLError("Scissor");
}

void EndFrame() {
	TestGLError("End prae");
	glDisable(GL_SCISSOR_TEST);
	if (!RenderingToTexture){
		// auf den Bildschirm
		#ifdef OS_WINDOWS
			if (RenderingToTexture<0)
				SwapBuffers(hDC);
		#endif
		#ifdef OS_LINUX
			#ifdef NIX_ALLOW_FULLSCREEN
				if (NixFullscreen)
					XF86VidModeSetViewPort(x_display,screen,0,NixDesktopHeight-NixScreenHeight);
			#endif
			//glutSwapBuffers();
			/*if (GLDoubleBuffered){
			}*/
		#endif
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	TestGLError("End post");
}



void ScreenShotToImage(Image &image) {
	image.create(target_width, target_height, Black);
	glReadBuffer(GL_FRONT);
	glReadPixels(	0,
					0,
					target_width,
					target_height,
					GL_RGBA, GL_UNSIGNED_BYTE, &image.data[0]);
}

void StartFrameHui() {
	int fb;
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &fb);
	FrameBuffer::DEFAULT->frame_buffer = fb;
	FrameBuffer::DEFAULT->width = hui::GetEvent()->column;
	FrameBuffer::DEFAULT->height = hui::GetEvent()->row;
	SetViewport(FrameBuffer::DEFAULT->area());
}
void EndFrameHui() {
	FrameBuffer::DEFAULT->frame_buffer = 0;
}


#if HAS_LIB_GLFW
#include <GLFW/glfw3.h>
#endif

#if HAS_LIB_GLFW
void StartFrameGLFW(void *win) {
	GLFWwindow* window = (GLFWwindow*)win;
	glfwMakeContextCurrent(window);
	int w, h;
	glfwGetFramebufferSize(window, &w, &h);
	FrameBuffer::DEFAULT->width = w;
	FrameBuffer::DEFAULT->height = h;

	SetViewport(FrameBuffer::DEFAULT->area());
}

void EndFrameGLFW(void *win) {
	GLFWwindow* window = (GLFWwindow*)win;
	glfwSwapBuffers(window);
}
#endif

};

#endif

