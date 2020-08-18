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

namespace nix {

VertexBuffer *vb_temp = NULL;


// so far, map each attribute to another buffer
VertexBuffer::VertexBuffer(const string &f) {
	//msg_write("new VertexBuffer " + f);
	auto xx = f.explode(",");
	num_buffers = num_attributes = xx.num;
	if (num_attributes > MAX_VB_ATTRIBUTES) {
		throw Exception("VertexBuffer: too many attributes: " + f);
		num_buffers = num_attributes = 0;
	}
	for (int i=0; i<xx.num; i++) {
		string &x = xx[i];
		auto &b = buf[i];
		b.count = 0;

		auto &a = attr[i];
		glGenBuffers(1, &b.buffer);
		a.buffer = b.buffer;
		a.normalized = false;
		a.stride = 0;
		a.divisor = 0;
		if (x == "1f") {
			a.type = GL_FLOAT;
			a.num_components = 1;
		} else if (x == "2f") {
			a.type = GL_FLOAT;
			a.num_components = 2;
		} else if (x == "3f") {
			a.type = GL_FLOAT;
			a.num_components = 3;
		} else if (x == "3fn") {
			a.type = GL_FLOAT;
			a.num_components = 3;
			a.normalized = true;
		} else if (x == "4f") {
			a.type = GL_FLOAT;
			a.num_components = 4;
		} else {
			throw Exception("VertexBuffer: unhandled format: " + x);
		}
	}
}


VertexBuffer::~VertexBuffer() {
	for (int i=0; i<num_buffers; i++)
		glDeleteBuffers(1, &buf[i].buffer);
}

void VertexBuffer::__init__(const string &f) {
	new(this) VertexBuffer(f);
}

void VertexBuffer::__delete__() {
	this->~VertexBuffer();
}

void VertexBuffer::set_per_instance(int index) {
	if (index < 0 or index >= num_attributes)
		throw Exception("VertexBuffer: invalid attribute index " + i2s(index));
	attr[index].divisor = 1;
}

void VertexBuffer::update(int index, const DynamicArray &a) {
	if (index < 0 or index >= num_buffers)
		throw Exception("VertexBuffer: invalid index " + i2s(index));
	buf[index].count = a.num;
	glBindBuffer(GL_ARRAY_BUFFER, buf[index].buffer);
	glBufferData(GL_ARRAY_BUFFER, a.num * a.element_size, a.data, GL_STATIC_DRAW);
}

int VertexBuffer::count() const {
	return buf[0].count;
}

void VertexBuffer::create_rect(const rect &d, const rect &s) {
	Array<vector> p = {vector(d.x1,d.y1,0), vector(d.x1,d.y2,0), vector(d.x2,d.y2,0),  vector(d.x1,d.y1,0), vector(d.x2,d.y2,0), vector(d.x2,d.y1,0)};
	Array<float> uv = {s.x1,s.y1, s.x1,s.y2, s.x2,s.y2,  s.x1,s.y1, s.x2,s.y2, s.x2,s.y1};
	update(0, p);
	update(1, p);
	update(2, uv);
}

int _current_vb_attr_ = 0;

void SetVertexBuffer(VertexBuffer *vb) {
	for (int i=0; i<vb->num_attributes; i++) {
		auto &a = vb->attr[i];
		glEnableVertexAttribArray(i);
		TestGLError("set vb 1");
		glBindBuffer(GL_ARRAY_BUFFER, a.buffer);
		TestGLError("set vb 2");
		glVertexAttribPointer(i, a.num_components, a.type, a.normalized, 0, (void*)0);//a.stride, (void*)a.offset);
		TestGLError("set vb 3");
		glVertexAttribDivisor(i, a.divisor);
	}

	for (int i=vb->num_attributes; i<_current_vb_attr_; i++)
		glDisableVertexAttribArray(i);

	_current_vb_attr_ = vb->num_attributes;
}

void init_vertex_buffers() {
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	vb_temp = new VertexBuffer("3f,3fn,2f");
}

};

#endif
