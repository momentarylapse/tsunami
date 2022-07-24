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
#endif
#if __has_include("../image/image.h")
#include "../image/image.h"
#endif

namespace nix {

extern FrameBuffer *cur_framebuffer;


matrix view_matrix, projection_matrix;
matrix model_matrix, model_view_projection_matrix;



matrix create_pixel_projection_matrix() {
	auto t = matrix::translation(vector(-float(target_width)/2.0f,-float(target_height)/2.0f,-0.5f));
	auto s = matrix::scale(2.0f / float(target_width), -2.0f / float(target_height), 2);
	return s * t;
}


void set_viewport(const rect &area) {
	target_rect = area;
	target_width = max((int)area.width(), 1);
	target_height = max((int)area.height(), 1);

	//glViewport(area.x1, cur_framebuffer->height - area.height() + area.y1, area.width(), area.height());
	glViewport(area.x1, area.y1, area.width(), area.height());
}

void set_model_matrix(const matrix &mat) {
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
	auto t = matrix::translation(
		vector(center.x / float(target_width) * 2.0f - 1,
			1 - center.y / float(target_height) * 2.0f,
			0));
	auto p = matrix::perspective(pi / 2, 1, z_min, z_max, true);
	auto s = matrix::scale(2 * size_1.x / target_width,
			2 * size_1.y / target_height,
			- 1); // z reflection: right/left handedness
	static const float EEE[] = {1,0,0,0, 0,1,0,0, 0,0,-1,0, 0,0,0,1}; // UNDO z-flip... :P

	set_projection_matrix(matrix::translation(vector(0,0,0.5f)) * matrix::scale(1,1,0.5f) * t * p * matrix(EEE) * s);
}

// center_x/y: pixel coordinates of (0,0,0)
// map_width/height: pixel sizes of projected base vectors
void set_projection_ortho_ext(const vec2 &center, const vec2 &map_size, float z_min, float z_max) {
	auto scale = matrix::scale(2.0f / float(target_width) * map_size.x, -2.0f / float(target_height) * map_size.y, 2 / (z_max - z_min));
	auto trans = matrix::translation(vector(2 * center.x / target_width - 1, 1 - 2 * center.y / target_height, -(z_max + z_min) / (z_max - z_min)));
	set_projection_matrix(matrix::translation(vector(0,0,0.5f)) * matrix::scale(1,1,0.5f) * trans * scale);
}

// orthogonal projection (relative [0,1]x[0,1]x[0,1] coordinates)
// y-axis downwards
void set_projection_ortho_relative() {
	auto t = matrix::translation(vector(-0.5f, -0.5f, 0));
	auto s = matrix::scale(2.0f, -2.0f, 1);
	set_projection_matrix(/*matrix::translation(vector(0,0,0.5f)) * matrix::scale(1,1,0.5f) */ s * t);
}

// orthogonal projection (pixel coordinates)
void set_projection_ortho_pixel() {
	set_projection_matrix(matrix::translation(vector(0,0,0.5f)) * matrix::scale(1,1,0.5f) * create_pixel_projection_matrix());
}

void set_projection_matrix(const matrix &m) {
	projection_matrix = m;
	model_view_projection_matrix = projection_matrix * view_matrix * model_matrix;
}

void set_view_matrix(const matrix &m) {
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

#ifdef _X_USE_HUI_

void start_frame_hui() {
	int fb;
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &fb);
	FrameBuffer::DEFAULT->frame_buffer = fb;
	FrameBuffer::DEFAULT->width = hui::get_event()->column;
	FrameBuffer::DEFAULT->height = hui::get_event()->row;
	cur_framebuffer = FrameBuffer::DEFAULT;
	set_viewport(FrameBuffer::DEFAULT->area());
}

void end_frame_hui() {
	FrameBuffer::DEFAULT->frame_buffer = 0;
}

#endif


#if HAS_LIB_GLFW
#include <GLFW/glfw3.h>
#endif

#if HAS_LIB_GLFW
void start_frame_glfw(void *win) {
	GLFWwindow* window = (GLFWwindow*)win;
	glfwMakeContextCurrent(window);
	int w, h;
	glfwGetFramebufferSize(window, &w, &h);
	FrameBuffer::DEFAULT->width = w;
	FrameBuffer::DEFAULT->height = h;
	cur_framebuffer = FrameBuffer::DEFAULT;

	set_viewport(FrameBuffer::DEFAULT->area());
}

void end_frame_glfw(void *win) {
	glFlush();
	GLFWwindow* window = (GLFWwindow*)win;
	glfwSwapBuffers(window);
}
#endif

};

#endif

