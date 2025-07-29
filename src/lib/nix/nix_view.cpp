/*----------------------------------------------------------------------------*\
| Nix view                                                                     |
| -> camera etc...                                                             |
|                                                                              |
| last update: 2010.03.11 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#if HAS_LIB_GL


#include "nix.h"
#include "nix_common.h"
#include "../math/vec2.h"
#if __has_include("../hui/hui.h")
#include "../hui/hui.h"
#define NIX_USE_HUI 1
#endif
#if __has_include("../image/image.h")
#include "../image/image.h"
#endif
#if HAS_LIB_GLFW
#include <GLFW/glfw3.h>
#endif


#ifndef NIX_USE_HUI
namespace hui {
	class Event {
	public:
		void *win;
		string message, id;
		bool is_default;
		vec2 m, d;
		vec2 scroll;
		float pressure;
		int key_code;
		int width, height;
		bool lbut, mbut, rbut;
		int row, column, row_target;
		bool just_focused;
	};
}
#endif

namespace nix {

extern FrameBuffer *cur_framebuffer;


mat4 view_matrix, projection_matrix;
mat4 model_matrix, model_view_projection_matrix;



mat4 create_pixel_projection_matrix() {
	auto t = mat4::translation(vec3(-float(target_width)/2.0f,-float(target_height)/2.0f,-0.5f));
	auto s = mat4::scale(2.0f / float(target_width), -2.0f / float(target_height), 2);
	return s * t;
}


void set_viewport(const rect &area) {
	target_rect = area;
	target_width = max((int)area.width(), 1);
	target_height = max((int)area.height(), 1);

	//glViewport(area.x1, cur_framebuffer->height - area.height() + area.y1, area.width(), area.height());
	glViewport(area.x1, area.y1, area.width(), area.height());
}

void set_model_matrix(const mat4 &mat) {
	model_matrix = mat;
	model_view_projection_matrix = projection_matrix * view_matrix * model_matrix;
}


// 3D-Matrizen erstellen (Einstellungen ueber SetPerspectiveMode vor NixStart() zu treffen)
// enable3d: true  -> 3D-Ansicht auf (View3DWidth,View3DHeight) gemapt
//           false -> Pixel-Angaben~~~
// beide Bilder sind um View3DCenterX,View3DCenterY (3D als Fluchtpunkt) verschoben

void set_projection_perspective() {
	set_projection_perspective_ext({(float)target_width / 2, (float)target_height / 2}, {(float)target_height, (float)target_height}, 0.001f, 10000);
}

// center_x/y: pixel coordinates of perspective center
// height_1/width_1: pixel sizes of 45° frustrum
void set_projection_perspective_ext(const vec2 &center, const vec2 &size_1, float z_min, float z_max) {
	// perspective projection
	auto t = mat4::translation(
		vec3(center.x / float(target_width) * 2.0f - 1,
			1 - center.y / float(target_height) * 2.0f,
			0));
	auto p = mat4::perspective(pi / 2, 1, z_min, z_max, true);
	auto s = mat4::scale(2 * size_1.x / target_width,
			2 * size_1.y / target_height,
			- 1); // z reflection: right/left handedness
	static const float EEE[] = {1,0,0,0, 0,1,0,0, 0,0,-1,0, 0,0,0,1}; // UNDO z-flip... :P

	set_projection_matrix(mat4::translation(vec3(0,0,0.5f)) * mat4::scale(1,1,0.5f) * t * p * mat4(EEE) * s);
}

// center_x/y: pixel coordinates of (0,0,0)
// map_width/height: pixel sizes of projected base vectors
void set_projection_ortho_ext(const vec2 &center, const vec2 &map_size, float z_min, float z_max) {
	auto scale = mat4::scale(2.0f / float(target_width) * map_size.x, -2.0f / float(target_height) * map_size.y, 2 / (z_max - z_min));
	auto trans = mat4::translation(vec3(2 * center.x / target_width - 1, 1 - 2 * center.y / target_height, -(z_max + z_min) / (z_max - z_min)));
	set_projection_matrix(mat4::translation(vec3(0,0,0.5f)) * mat4::scale(1,1,0.5f) * trans * scale);
}

// orthogonal projection (relative [0,1]x[0,1]x[0,1] coordinates)
// y-axis downwards
void set_projection_ortho_relative() {
	auto t = mat4::translation(vec3(-0.5f, -0.5f, 0));
	auto s = mat4::scale(2.0f, -2.0f, 1);
	set_projection_matrix(/*matrix::translation(vector(0,0,0.5f)) * matrix::scale(1,1,0.5f) */ s * t);
}

// orthogonal projection (pixel coordinates)
void set_projection_ortho_pixel() {
	set_projection_matrix(mat4::translation(vec3(0,0,0.5f)) * mat4::scale(1,1,0.5f) * create_pixel_projection_matrix());
}

void set_projection_matrix(const mat4 &m) {
	projection_matrix = m;
	model_view_projection_matrix = projection_matrix * view_matrix * model_matrix;
}

void set_view_matrix(const mat4 &m) {
	view_matrix = m;
}




void set_scissor(const rect &r) {
	if (r.width() > 0) {
		glEnable(GL_SCISSOR_TEST);
		glScissor((int)r.x1, cur_framebuffer->height - (int)r.y2, (int)r.width(), (int)r.height());
	} else {
		glDisable(GL_SCISSOR_TEST);
	}
	//glClearDepth(1.0f);
}



void screen_shot_to_image(Image &image) {
	image.create(target_width, target_height, Black);
	glReadBuffer(GL_FRONT);
	glReadPixels(	0,
					0,
					target_width,
					target_height,
					GL_RGBA, GL_UNSIGNED_BYTE, &image.data[0]);
}

void start_frame_hui(Context *gl, hui::Event* e) {
	Context::CURRENT = gl;
	int fb;
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &fb);
	// FIXME
	gl->default_framebuffer->frame_buffer = fb;
	gl->default_framebuffer->width = e->column;
	gl->default_framebuffer->height = e->row;
	cur_framebuffer = gl->default_framebuffer;
	set_viewport(gl->default_framebuffer->area());
}

void end_frame_hui() {
	Context::CURRENT->default_framebuffer->frame_buffer = 0;
}


#if HAS_LIB_GLFW

static GLFWwindow* nix_glfw_window = nullptr;

void start_frame_glfw(Context *gl, void *win) {
	GLFWwindow* window = (GLFWwindow*)win;
	nix_glfw_window = window;
	glfwMakeContextCurrent(window);
	int w, h;
	glfwGetFramebufferSize(window, &w, &h);
	gl->default_framebuffer->width = w;
	gl->default_framebuffer->height = h;
	cur_framebuffer = gl->default_framebuffer;

	set_viewport(gl->default_framebuffer->area());
}

void end_frame_glfw() {
	glFlush();
	glfwSwapBuffers(nix_glfw_window);
}
#endif

};

#endif

