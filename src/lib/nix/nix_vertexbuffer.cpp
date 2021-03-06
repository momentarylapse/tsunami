/*----------------------------------------------------------------------------*\
| Nix vertex buffer                                                            |
| -> handling vertex buffers                                                   |
|                                                                              |
| last update: 2010.03.11 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#if HAS_LIB_GL

#include "nix.h"
#include "nix_common.h"


namespace nix {

// hmmm, some bugs using GTK:
//  * new vao numbers start over (creating duplicates), after first frame?
//    (but the context seems to be the same)
//  * old vaos loose their data?
bool allow_separate_vertex_arrays = false;
unsigned int global_vao = 0;

VertexBuffer *vb_temp = NULL;
VertexBuffer *vb_temp_i = NULL;

bool gl_is_integer(unsigned int type) {
	if ((type == GL_INT) or (type == GL_UNSIGNED_INT))
		return true;
	if ((type == GL_SHORT) or (type == GL_UNSIGNED_SHORT))
		return true;
	if ((type == GL_BYTE) or (type == GL_UNSIGNED_BYTE))
		return true;
	return false;
}


void _post_config_vertex_buffer(VertexBuffer *vb) {
	for (int i=0; i<vb->num_attributes; i++) {
		auto &a = vb->attr[i];
		glEnableVertexArrayAttrib(vb->vao, i);
		glVertexArrayAttribBinding(vb->vao, i, i);
		glVertexArrayVertexBuffer(vb->vao, i, a.buffer, 0, a.stride);
		if (gl_is_integer(a.type)) // argh!
			glVertexArrayAttribIFormat(vb->vao, i, a.num_components, a.type, 0);
		else
			glVertexArrayAttribFormat(vb->vao, i, a.num_components, a.type, a.normalized, 0);

		// TODO
		if (false)
			glVertexAttribDivisor(i, a.divisor);
	}
	if (vb->index.buffer > 0) {
		glVertexArrayElementBuffer(vb->vao, vb->index.buffer);
	}
}
void ___post_config_vertex_buffer_old(VertexBuffer *vb) {
	glBindVertexArray(vb->vao);
	for (int i=0; i<vb->num_attributes; i++) {
		auto &a = vb->attr[i];
		glEnableVertexAttribArray(i);
		glBindBuffer(GL_ARRAY_BUFFER, a.buffer);
		glVertexAttribPointer(i, a.num_components, a.type, a.normalized, a.stride, (void*)0);//a.stride, (void*)a.offset);
		glVertexAttribDivisor(i, a.divisor);
	}
	glBindVertexArray(0);
}

int gl_component_size(unsigned int type) {
	if ((type == GL_DOUBLE))
		return 8;
	if ((type == GL_INT) or (type == GL_UNSIGNED_INT) or (type == GL_FLOAT))
		return 4;
	if ((type == GL_SHORT) or (type == GL_UNSIGNED_SHORT) or (type == GL_HALF_FLOAT))
		return 2;
	if ((type == GL_BYTE) or (type == GL_UNSIGNED_BYTE))
		return 1;
	return 0;
}

// "2f" "3fn" "4i" etc
bool parse_vb_component(const string &_f, unsigned int &type, int &num) {
	string f = _f.replace("n", "");
	if (f.num < 1)
		return false;

	num = 1;
	if (f[0] >= '0' and f[0] <= '9') {
		num = f.head(1)._int();
		f = f.sub(1);
	}

	if (f == "f" or f == "f32")
		type = GL_FLOAT;
	else if (f == "i" or f == "i32")
		type = GL_INT;
	else if (f == "u" or f == "u32")
		type = GL_UNSIGNED_INT;
	else if (f == "i16")
		type = GL_SHORT;
	else if (f == "u16")
		type = GL_UNSIGNED_SHORT;
	else if (f == "i8")
		type = GL_BYTE;
	else if (f == "u8")
		type = GL_UNSIGNED_BYTE;
	else
		return false;
	return true;
}

// so far, map each attribute to another buffer
VertexBuffer::VertexBuffer(const string &_f) {
	//msg_write("new VertexBuffer " + _f);

	string f = _f;
	index.buffer = 0;
	index.count = 0;
	if (f.find("|") >= 0) {
		string fi = f.sub_ref(f.find("|")+1);
		if ((fi == "i16") or (fi == "u16"))
			index.type = GL_UNSIGNED_SHORT;
		else if ((fi == "i32") or (fi == "i") or (fi == "u32") or (fi == "u"))
			index.type = GL_UNSIGNED_INT;
		else
			throw Exception("VertexBuffer: unhandled index format: " + fi);
		glCreateBuffers(1, &index.buffer);

		f = f.head(f.find("|"));
	}

	auto xx = f.explode(",");
	num_buffers = num_attributes = xx.num;
	if (num_attributes > MAX_VB_ATTRIBUTES) {
		throw Exception("VertexBuffer: too many attributes: " + f);
		num_buffers = num_attributes = 0;
	}

	if (allow_separate_vertex_arrays) {
		glCreateVertexArrays(1, &vao);
	} else {
		vao = global_vao;
	}

	for (int i=0; i<xx.num; i++) {
		string &x = xx[i];
		auto &b = buf[i];
		b.count = 0;

		auto &a = attr[i];
		glCreateBuffers(1, &b.buffer);
		a.buffer = b.buffer;
		a.normalized = false;
		a.divisor = 0;
		a.stride = 0;
		// "2f" "3fn" "4i" etc
		if (!parse_vb_component(x, a.type, a.num_components))
			throw Exception("VertexBuffer: unhandled format: " + x);
		a.normalized = (x.find("n") >= 0);

		a.stride = a.num_components * gl_component_size(a.type);
	}

	_post_config_vertex_buffer(this);
}


VertexBuffer::~VertexBuffer() {
	for (int i=0; i<num_buffers; i++)
		glDeleteBuffers(1, &buf[i].buffer);
	if (allow_separate_vertex_arrays)
		glDeleteVertexArrays(1, &vao);
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
	glNamedBufferData(buf[index].buffer, a.num * a.element_size, a.data, GL_STATIC_DRAW);
	//glNamedBufferStorage
}

void VertexBuffer::update_index(const DynamicArray &a) {
	index.count = a.num;
	glNamedBufferData(index.buffer, a.num * a.element_size, a.data, GL_STATIC_DRAW);
	//glNamedBufferStorage

	if (a.element_size == 1)
		index.type = GL_UNSIGNED_BYTE;
	else if (a.element_size == 2)
		index.type = GL_UNSIGNED_SHORT;
	else
		index.type = GL_UNSIGNED_INT;
}

int VertexBuffer::count() const {
	return buf[0].count;
}

bool VertexBuffer::is_indexed() const {
	return index.buffer > 0;
}

void VertexBuffer::create_rect(const rect &d, const rect &s) {
	if (is_indexed()) {
		Array<vector> p = {{d.x1,d.y1,0}, {d.x1,d.y2,0}, {d.x2,d.y1,0}, {d.x2,d.y2,0}};
		Array<vector> n = {{0,0,1}, {0,0,1}, {0,0,1}, {0,0,1}};
		Array<float> uv = {s.x1,s.y1, s.x1,s.y2, s.x2,s.y1, s.x2,s.y2};
		update(0, p);
		update(1, n);
		update(2, uv);
		Array<unsigned char> index = {0,1,3, 0,3,2};
		update_index(index);
	} else {
		Array<vector> p = {{d.x1,d.y1,0}, {d.x1,d.y2,0}, {d.x2,d.y2,0},  {d.x1,d.y1,0}, {d.x2,d.y2,0}, {d.x2,d.y1,0}};
		Array<vector> n = {{0,0,1}, {0,0,1}, {0,0,1},  {0,0,1}, {0,0,1}, {0,0,1}};
		Array<float> uv = {s.x1,s.y1, s.x1,s.y2, s.x2,s.y2,  s.x1,s.y1, s.x2,s.y2, s.x2,s.y1};
		update(0, p);
		update(1, n);
		update(2, uv);
	}
}

void bind_vertex_buffer(VertexBuffer *vb) {
	if (!allow_separate_vertex_arrays)
		_post_config_vertex_buffer(vb);

#if 0
	for (int i=0; i<vb->num_attributes; i++) {
		auto &a = vb->attr[i];
		glEnableVertexArrayAttrib(vb->vao, i);

		glVertexArrayAttribBinding(vb->vao, i, i);
		glVertexArrayVertexBuffer(vb->vao, i, a.buffer, 0, a.stride);
		glVertexArrayAttribFormat(vb->vao, i, a.num_components, a.type, a.normalized, 0);

		//glEnableVertexAttribArray(i);
//		glBindBuffer(GL_ARRAY_BUFFER, a.buffer);
//		glVertexAttribPointer(i, a.num_components, a.type, a.normalized, a.stride, (void*)0);//a.stride, (void*)a.offset);
		//glVertexAttribDivisor(i, a.divisor);
	}
#endif
	glBindVertexArray(vb->vao);
}

void init_vertex_buffers() {
	if (!allow_separate_vertex_arrays) {
		glCreateVertexArrays(1, &global_vao);
	}

	vb_temp = new VertexBuffer("3f,3fn,2f");
	vb_temp_i = new VertexBuffer("3f,3fn,2f|i");
}

};

#endif
