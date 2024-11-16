/*
 * nix_buffer.cpp
 *
 *  Created on: Jun 25, 2021
 *      Author: michi
 */


#if HAS_LIB_GL

#include "nix.h"
#include "nix_common.h"

namespace nix {


Buffer::Buffer() {
	type = Type::NONE;
	glCreateBuffers(1, &buffer);
}

Buffer::~Buffer() {
	glDeleteBuffers(1, &buffer);
}

void Buffer::update(void *data, int size) {
	glNamedBufferData(buffer, size, data, GL_DYNAMIC_DRAW);
}

void Buffer::update_array(const DynamicArray &a) {
	update(a.data, a.num * a.element_size);
}

void Buffer::read(void *data, int size) {
	auto p = glMapNamedBuffer(buffer, GL_READ_ONLY);
	memcpy(data, p, size);
	glUnmapNamedBuffer(buffer);
}

void Buffer::read_array(DynamicArray &a) {
	read(a.data, a.num * a.element_size);
}


UniformBuffer::UniformBuffer() {
	type = Type::UNIFORM;
}

ShaderStorageBuffer::ShaderStorageBuffer() {
	type = Type::SSBO;
}

void bind_uniform_buffer(int binding, UniformBuffer *buf) {
	glBindBufferBase(GL_UNIFORM_BUFFER, binding, buf->buffer);
}

void bind_storage_buffer(int binding, ShaderStorageBuffer *buf) {
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, buf->buffer);
}

}

#endif



