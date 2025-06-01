/*----------------------------------------------------------------------------*\
| Nix draw                                                                     |
| -> drawing functions                                                         |
|                                                                              |
| last update: 2010.03.11 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#if HAS_LIB_GL

#include "nix.h"
#include "nix_common.h"

namespace nix {






void draw_triangles(VertexBuffer *vb) {
	if (vb->count() == 0)
		return;
	if (default_shader_bindings)
		Context::CURRENT->_current_->set_default_data();

	bind_vertex_buffer(vb);

	if (vb->is_indexed())
		glDrawElements(GL_TRIANGLES, vb->index.count, vb->index.type, (void*)0);
	else
		glDrawArrays(GL_TRIANGLES, 0, vb->count()); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

void draw_instanced_triangles(VertexBuffer *vb, int count) {
	if (vb->count() == 0)
		return;
	if (default_shader_bindings)
		Context::CURRENT->_current_->set_default_data();

	bind_vertex_buffer(vb);

	glDrawArraysInstanced(GL_TRIANGLES, 0, vb->count(), count); // Starting from vertex 0; 3 vertices total -> 1 triangle
}


void draw_lines(VertexBuffer *vb, bool contiguous) {
	if (vb->count() == 0)
		return;
	if (default_shader_bindings)
		Context::CURRENT->_current_->set_default_data();

	bind_vertex_buffer(vb);

	if (contiguous)
		glDrawArrays(GL_LINE_STRIP, 0, vb->count());
	else
		glDrawArrays(GL_LINES, 0, vb->count());
}

void draw_points(VertexBuffer *vb) {
	if (vb->count() == 0)
		return;
	if (default_shader_bindings)
		Context::CURRENT->_current_->set_default_data();

	bind_vertex_buffer(vb);

	glDrawArrays(GL_POINTS, 0, vb->count());
}

void _cdecl draw(PrimitiveTopology topology, VertexBuffer *vb) {
	if (vb->count() == 0)
		return;
	if (default_shader_bindings)
		Context::CURRENT->_current_->set_default_data();

	bind_vertex_buffer(vb);

	glDrawArrays((int)topology, 0, vb->count());
}

void draw_mesh_tasks(int offset, int count) {
#if HAS_LIB_GLEW
	if (Context::CURRENT->supports_mesh_shaders)
		glDrawMeshTasksNV(offset, count);
#endif
}


void clear(const color &c) {
	glClearColor(c.r, c.g, c.b, c.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void clear_color(const color &c) {
	glClearColor(c.r, c.g, c.b, c.a);
	glClear(GL_COLOR_BUFFER_BIT);
}

void clear_z() {
	glClear(GL_DEPTH_BUFFER_BIT);
}

};
#endif
