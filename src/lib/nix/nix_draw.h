/*----------------------------------------------------------------------------*\
| Nix draw                                                                     |
| -> drawing functions                                                         |
|                                                                              |
| last update: 2010.03.11 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#if HAS_LIB_GL

#ifndef _NIX_DRAW_EXISTS_
#define _NIX_DRAW_EXISTS_

namespace nix{

class VertexBuffer;
class Texture;

void _cdecl ResetToColor(const color &c);
void _cdecl ResetZ();

void _cdecl DrawTriangles(VertexBuffer *vb);
void _cdecl DrawInstancedTriangles(VertexBuffer *vb, int count);
void _cdecl DrawLines(VertexBuffer *vb, bool contiguous);

};

#endif

#endif
