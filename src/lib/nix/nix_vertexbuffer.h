/*----------------------------------------------------------------------------*\
| Nix vertex buffer                                                            |
| -> handling vertex buffers                                                   |
|                                                                              |
| last update: 2010.03.11 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#if HAS_LIB_GL

#pragma once


#include "../math/rect.h"

#define MAX_VB_ATTRIBUTES 8
#define MAX_VB_BUFFERS 8


namespace nix
{


class VertexBuffer {
public:
	struct Buffer {
		unsigned int buffer;
		int count;
	} buf[MAX_VB_BUFFERS];
	struct Attribute {
		unsigned int buffer;
		int num_components;
		unsigned int type;
		bool normalized;
		int stride;
		int divisor;
	} attr[MAX_VB_ATTRIBUTES];
	int num_attributes;
	int num_buffers;

	struct Index {
		unsigned int buffer;
		unsigned int type;
		int count;
	} index;

	unsigned int vao;

	VertexBuffer(const string &f);
	~VertexBuffer();

	void _cdecl __init__(const string &f);
	void _cdecl __delete__();

	void _cdecl update(int index, const DynamicArray &a);
	void _cdecl set_per_instance(int index);
	void _cdecl update_index(const DynamicArray &a);
	int count() const;
	bool is_indexed() const;

	void create_rect(const rect &dest, const rect &source = rect::ID);
};

void init_vertex_buffers();

void bind_vertex_buffer(VertexBuffer *vb);

};


#endif
