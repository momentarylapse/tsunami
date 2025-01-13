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

	void update(const void *data, int size);
	void update_array(const DynamicArray &a);

	void read(void *data, int size);
	void read_array(DynamicArray &a);
};

class UniformBuffer : public Buffer {
public:
	explicit UniformBuffer(int size);
};

class ShaderStorageBuffer : public Buffer {
public:
	explicit ShaderStorageBuffer(int size);
};

void bind_uniform_buffer(int binding, UniformBuffer *buf);
void bind_storage_buffer(int binding, ShaderStorageBuffer *buf);

};

#endif
