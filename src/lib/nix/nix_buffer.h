/*
 * nix_buffer.h
 *
 *  Created on: Jun 25, 2021
 *      Author: michi
 */

#if HAS_LIB_GL

#pragma once

namespace nix{


class Buffer {
public:
	enum class Type {
		NONE,
		UNIFORM,
		SSBO
	} type;
	unsigned int buffer;

	Buffer();
	~Buffer();

	void update(void *data, int size);
	void update_array(const DynamicArray &a);

	void read(void *data, int size);
	void read_array(DynamicArray &a);
};

class UniformBuffer : public Buffer {
public:
	UniformBuffer();
};

class ShaderStorageBuffer : public Buffer {
public:
	ShaderStorageBuffer();
};

void bind_uniform_buffer(int binding, UniformBuffer *buf);
void bind_storage_buffer(int binding, ShaderStorageBuffer *buf);

};

#endif
