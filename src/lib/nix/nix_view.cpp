/*----------------------------------------------------------------------------*\
| Nix view                                                                     |
| -> camera etc...                                                             |
|                                                                              |
| last update: 2010.03.11 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#if HAS_LIB_GL


#include "nix.h"
#include "nix_common.h"
#ifdef _X_USE_HUI_
#include "../hui/hui.h"
#endif
#ifdef _X_USE_IMAGE_
#include "../image/image.h"
#endif

namespace nix{


matrix view_matrix, projection_matrix;
matrix model_matrix, model_view_projection_matrix;
FrameBuffer *cur_framebuffer = nullptr;


FrameBuffer *FrameBuffer::DEFAULT = new FrameBuffer();

FrameBuffer::FrameBuffer() {
	depth_buffer = nullptr;
	width = height = 0;

	frame_buffer = 0;
	multi_samples = 0;
}

FrameBuffer::FrameBuffer(const Array<Texture*> &attachments) {
	glCreateFramebuffers(1, &frame_buffer);
	update(attachments);
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

void FrameBuffer::update(const Array<Texture*> &attachments) {
	update_x(attachments, -1);
}

void FrameBuffer::update_x(const Array<Texture*> &attachments, int cube_face) {
	depth_buffer = nullptr;
	color_attachments = {};
	int samples = 0;

	for (auto *a: attachments) {
		if ((a->type == a->Type::DEPTH) or (a->type == a->Type::RENDERBUFFER))
			depth_buffer = (DepthBuffer*)a;
		else
			color_attachments.add(a);
		if (a->width > 0) {
			width = a->width;
			height = a->height;
		}
		if (a->samples > 0)
			samples = a->samples;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);



	if (depth_buffer) {
		if (depth_buffer->type == Texture::Type::RENDERBUFFER) {
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth_buffer->texture);
		} else {
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_buffer->texture, 0);
		}
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}

	Array<GLenum> draw_buffers;
	int target =  GL_TEXTURE_2D;
	if (cube_face >= 0)
		target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + cube_face;
	if (samples > 0)
		target = GL_TEXTURE_2D_MULTISAMPLE;
	foreachi (auto *t, color_attachments, i) {
		//glNamedFramebufferTexture(frame_buffer, GL_COLOR_ATTACHMENT0 + i, t->texture, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, target, t->texture, 0);
		draw_buffers.add(GL_COLOR_ATTACHMENT0 + (unsigned)i);
	}
	glDrawBuffers(draw_buffers.num, &draw_buffers[0]);


	_check();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::_check() {
	auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		msg_error("FrameBuffer: framebuffer != complete");
		if (status == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT)
			msg_write("incomplete att");
		//if (status == GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS)
		//	msg_write("incomplete dim");
		if (status == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT)
			msg_write("missing att");
		if (status == GL_FRAMEBUFFER_UNSUPPORTED)
			msg_write("unsup");
	}
}

rect FrameBuffer::area() const {
	return rect(0, width, 0, height);
}

void FrameBuffer::clear_color(int index, const color &c) {
	glClearNamedFramebufferfv(frame_buffer, GL_COLOR, index, (float*)&c);
}

void FrameBuffer::clear_depth(float depth) {
	glClearNamedFramebufferfv(frame_buffer, GL_DEPTH, 0, &depth);
}

void bind_frame_buffer(FrameBuffer *fb) {
	glBindFramebuffer(GL_FRAMEBUFFER, fb->frame_buffer);
	cur_framebuffer = fb;

	set_viewport(fb->area());
}

void resolve_multisampling(FrameBuffer *target, FrameBuffer *source) {
	//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target->frame_buffer);
	//glBindFramebuffer(GL_READ_FRAMEBUFFER, source->frame_buffer);
	//glBlitFramebuffer(0, 0, source->width, source->height, 0, 0, target->width, target->height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glBlitNamedFramebuffer(source->frame_buffer, target->frame_buffer, 0, 0, source->width, source->height, 0, 0, target->width, target->height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}


matrix create_pixel_projection_matrix() {
	auto t = matrix::translation(vector(-float(target_width)/2.0f,-float(target_height)/2.0f,-0.5f));
	auto s = matrix::scale(2.0f / float(target_width), -2.0f / float(target_height), 2);
	return s * t;
}


void set_viewport(const rect &area) {
	target_rect = area;
	target_width = max((int)area.width(), 1);
	target_height = max((int)area.height(), 1);

	glViewport(area.x1, cur_framebuffer->height - area.height() + area.y1, area.width(), area.height());
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
	set_projection_perspective_ext((float)target_width / 2, (float)target_height / 2, (float)target_height, (float)target_height, 0.001f, 10000);
}

// center_x/y: pixel coordinates of perspective center
// height_1/width_1: pixel sizes of 45° frustrum
void set_projection_perspective_ext(float center_x, float center_y, float width_1, float height_1, float z_min, float z_max) {
	// perspective projection
	auto t = matrix::translation(
		vector(center_x / float(target_width) * 2.0f - 1,
			1 - center_y / float(target_height) * 2.0f,
			0));
	auto p = matrix::perspective(pi / 2, 1, z_min, z_max);
	auto s = matrix::scale(2 * width_1 / target_width,
			2 * height_1 / target_height,
			- 1); // z reflection: right/left handedness

	set_projection_matrix(t * p * s);
}

// center_x/y: pixel coordinates of (0,0,0)
// map_width/height: pixel sizes of projected base vectors
void set_projection_ortho_ext(float center_x, float center_y, float map_width, float map_height, float z_min, float z_max) {
	auto scale = matrix::scale(2.0f / float(target_width) * map_width, -2.0f / float(target_height) * map_height, 2 / (z_max - z_min));
	auto trans = matrix::translation(vector(2 * center_x / target_width - 1, 1 - 2 * center_y / target_height, -(z_max + z_min) / (z_max - z_min)));
	set_projection_matrix(trans * scale);
}

void set_projection_ortho_relative() {
	// orthogonal projection (relative [0,1]x[0x1] coordinates)
	auto t = matrix::translation(vector(-0.5f, -0.5f, 0));
	auto s = matrix::scale(2.0f, -2.0f, 1);
	set_projection_matrix(s * t);
}

// orthogonal projection (pixel coordinates)
void set_projection_ortho_pixel() {
	set_projection_matrix(create_pixel_projection_matrix());
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
	FrameBuffer::DEFAULT->width = hui::GetEvent()->column;
	FrameBuffer::DEFAULT->height = hui::GetEvent()->row;
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

