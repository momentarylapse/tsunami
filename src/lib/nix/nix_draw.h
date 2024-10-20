/*----------------------------------------------------------------------------*\
| Nix draw                                                                     |
| -> drawing functions                                                         |
|                                                                              |
| last update: 2010.03.11 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#if HAS_LIB_GL

#pragma once

class color;

namespace nix{

class VertexBuffer;
class Texture;

void _cdecl clear(const color &c);
void _cdecl clear_color(const color &c);
void _cdecl clear_z();

void _cdecl draw_triangles(VertexBuffer *vb);
void _cdecl draw_instanced_triangles(VertexBuffer *vb, int count);
void _cdecl draw_lines(VertexBuffer *vb, bool contiguous);
void _cdecl draw_points(VertexBuffer *vb);
void draw_mesh_tasks(int offset, int count);

};

#endif
