/*----------------------------------------------------------------------------*\
| Nix vertex buffer                                                            |
| -> handling vertex buffers                                                   |
|                                                                              |
| last update: 2010.03.11 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#if HAS_LIB_GL

#include "nix.h"
#include "nix_common.h"


unsigned int VertexArrayID = 0;

namespace nix{

VertexBuffer *vb_temp = NULL;
VertexBuffer *vb_2d = NULL;


VertexBuffer::VertexBuffer(int _num_textures)
{
	num_textures = _num_textures;
	if (num_textures > NIX_MAX_TEXTURELEVELS)
		num_textures = NIX_MAX_TEXTURELEVELS;
	indexed = false;

	num_triangles = 0;

	buffers_created = false;
	dirty = true;

	#ifdef ENABLE_INDEX_BUFFERS
		/*VBIndex[NumVBs]=new ...; // noch zu bearbeiten...
		if (!OGLVBIndex[index]){
			msg_error("IndexBuffer konnte nicht erstellt werden");
			return -1;
		}*/
	#endif
	buf_n = 0;
	buf_v = 0;
	for (int i=0; i<NIX_MAX_TEXTURELEVELS; i++)
		buf_t[i] = 0;
}

VertexBuffer::~VertexBuffer()
{
	clear();
}

void VertexBuffer::__init__(int _num_textures)
{
	new(this) VertexBuffer(_num_textures);
}

void VertexBuffer::__delete__()
{
	this->~VertexBuffer();
}

void VertexBuffer::clear()
{
	vertices.clear();
	normals.clear();
	for (int i=0; i<num_textures; i++)
		tex_coords[i].clear();
	num_triangles = 0;
	dirty = true;
}

void VertexBuffer::update()
{
	if (!buffers_created){
		glGenBuffers(1, &buf_v);
		glGenBuffers(1, &buf_n);
		glGenBuffers(num_textures, buf_t);
		buffers_created = true;
	}
	TestGLError("opt0");
	glBindBuffer(GL_ARRAY_BUFFER, buf_v);
	glBufferData(GL_ARRAY_BUFFER, vertices.num * sizeof(vertices[0]), vertices.data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, buf_n);
	glBufferData(GL_ARRAY_BUFFER, normals.num * sizeof(normals[0]), normals.data, GL_STATIC_DRAW);
	TestGLError("opt1");
	for (int i=0; i<num_textures; i++){
		glBindBuffer(GL_ARRAY_BUFFER, buf_t[i]);
		glBufferData(GL_ARRAY_BUFFER, tex_coords[i].num * sizeof(tex_coords[i][0]), tex_coords[i].data, GL_STATIC_DRAW);
	}
	dirty = false;
	TestGLError("opt2");
}

void VertexBuffer::addTria(const vector &p1,const vector &n1,float tu1,float tv1,
							const vector &p2,const vector &n2,float tu2,float tv2,
							const vector &p3,const vector &n3,float tu3,float tv3)
{
	vertices.add(p1);
	vertices.add(p2);
	vertices.add(p3);
	normals.add(n1);
	normals.add(n2);
	normals.add(n3);
	tex_coords[0].add(tu1);
	tex_coords[0].add(tv1);
	tex_coords[0].add(tu2);
	tex_coords[0].add(tv2);
	tex_coords[0].add(tu3);
	tex_coords[0].add(tv3);
	indexed = false;
	num_triangles ++;
	dirty = true;
}

void VertexBuffer::addTriaM(const vector &p1,const vector &n1,const float *t1,
								const vector &p2,const vector &n2,const float *t2,
								const vector &p3,const vector &n3,const float *t3)
{
	vertices.add(p1);
	vertices.add(p2);
	vertices.add(p3);
	normals.add(n1);
	normals.add(n2);
	normals.add(n3);
	for (int i=0;i<num_textures;i++){
		tex_coords[i].add(t1[i*2  ]);
		tex_coords[i].add(t1[i*2+1]);
		tex_coords[i].add(t2[i*2  ]);
		tex_coords[i].add(t2[i*2+1]);
		tex_coords[i].add(t3[i*2  ]);
		tex_coords[i].add(t3[i*2+1]);
	}
	indexed = false;
	num_triangles ++;
	dirty = true;
}

// for each triangle there have to be 3 vertices (p[i],n[i],t[i*2],t[i*2+1])
void VertexBuffer::addTrias(int num_trias, const vector *p, const vector *n, const float *t)
{
	int nv0 = vertices.num;
	vertices.resize(vertices.num + num_trias * 3);
	normals.resize(normals.num + num_trias * 3);
	memcpy(&vertices[nv0], p, sizeof(vector) * num_trias * 3);
	memcpy(&normals[nv0], n, sizeof(vector) * num_trias * 3);
	//memcpy(OGLVBTexCoords[buffer][0],t,sizeof(float)*num_trias*6);
	int nt0 = tex_coords[0].num;
	tex_coords[0].resize(nt0 + 6 * num_trias);
	for (int i=0;i<num_trias*3;i++){
		tex_coords[0][nt0 + i*2  ] = t[i*2];
		tex_coords[0][nt0 + i*2+1] = t[i*2+1];
	}
	num_triangles += num_trias;
	dirty = true;
}

void VBAddTriasIndexed(int buffer,int num_points,int num_trias,const vector *p,const vector *n,const float *tu,const float *tv,const unsigned short *indices)
{
	#ifdef ENABLE_INDEX_BUFFERS
	#endif
}

void init_vertex_buffers()
{
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	vb_temp = new VertexBuffer(1);
	vb_2d = new VertexBuffer(1);
}

};

#endif
