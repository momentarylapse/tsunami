/*----------------------------------------------------------------------------*\
| Nix draw                                                                     |
| -> drawing functions                                                         |
|                                                                              |
| last update: 2010.03.11 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#if HAS_LIB_GL

#include "nix.h"
#include "nix_common.h"

namespace nix{

extern Shader *current_shader;






void DrawTriangles(VertexBuffer *vb) {
	if (vb->count() == 0)
		return;
	current_shader->set_default_data();

	SetVertexBuffer(vb);

	glDrawArrays(GL_TRIANGLES, 0, vb->count()); // Starting from vertex 0; 3 vertices total -> 1 triangle

	TestGLError("DrawTriangles");
}

void DrawInstancedTriangles(VertexBuffer *vb, int count) {
	if (vb->count() == 0)
		return;
	current_shader->set_default_data();

	SetVertexBuffer(vb);

	glDrawArraysInstanced(GL_TRIANGLES, 0, vb->count(), count); // Starting from vertex 0; 3 vertices total -> 1 triangle

	TestGLError("DrawTriangles");
}


void DrawLines(VertexBuffer *vb, bool contiguous) {
	if (vb->count() == 0)
		return;
	current_shader->set_default_data();

	SetVertexBuffer(vb);

	if (contiguous)
		glDrawArrays(GL_LINE_STRIP, 0, vb->count());
	else
		glDrawArrays(GL_LINES, 0, vb->count());
	TestGLError("DrawLines");
}


void ResetToColor(const color &c) {
	glClearColor(c.r, c.g, c.b, c.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	TestGLError("ResetToColor");
}

void ResetZ() {
	glClear(GL_DEPTH_BUFFER_BIT);
	TestGLError("ResetZ");
}

};
#endif
