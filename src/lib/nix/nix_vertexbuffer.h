/*----------------------------------------------------------------------------*\
| Nix vertex buffer                                                            |
| -> handling vertex buffers                                                   |
|                                                                              |
| last update: 2010.03.11 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#if HAS_LIB_GL

#ifndef _NIX_VERTEXBUFFER_EXISTS_
#define _NIX_VERTEXBUFFER_EXISTS_

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
	} attr[MAX_VB_ATTRIBUTES];
	int num_attributes;
	int num_buffers;

	VertexBuffer(const string &f);
	~VertexBuffer();

	void _cdecl __init__(const string &f);
	void _cdecl __delete__();

	void _cdecl update(int index, const DynamicArray &a);
	int count() const;
};

void init_vertex_buffers();

void SetVertexBuffer(VertexBuffer *vb);

};


#endif

#endif
